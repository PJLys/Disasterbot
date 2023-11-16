
#include <avr/interrupt.h>
#include "DriverTWIMaster.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>

#define false 0
#define true 1

/* Transaction status defines. */
#define TWIM_STATUS_READY              0
#define TWIM_STATUS_BUSY               1

/* Transaction result enumeration. */
typedef enum TWIM_RESULT_enum {
	TWIM_RESULT_UNKNOWN          = (0x00<<0),
	TWIM_RESULT_OK               = (0x01<<0),
	TWIM_RESULT_BUFFER_OVERFLOW  = (0x02<<0),
	TWIM_RESULT_ARBITRATION_LOST = (0x03<<0),
	TWIM_RESULT_BUS_ERROR        = (0x04<<0),
	TWIM_RESULT_NACK_RECEIVED    = (0x05<<0),
	TWIM_RESULT_FAIL             = (0x06<<0),
} TWIM_RESULT_t;



//Private function prototypes
void TWIMTransactionFinished(uint8_t result);
void TWIMArbitrationLostBusErrorHandler();
void TWIMWriteHandler();
void TWIMReadHandler();

//Private variables
static volatile uint8_t Twim_address;								//Slave address
static volatile uint8_t *Twim_writeData;							//Data to write
static volatile uint8_t *Twim_readData;								//Read data
static volatile uint8_t Twim_bytesToWrite;							//Number of bytes to write
static volatile uint8_t Twim_bytesToRead;							//Number of bytes to read
static volatile uint8_t Twim_bytesWritten;							//Number of bytes written
static volatile uint8_t Twim_bytesRead;								//Number of bytes read
static volatile uint8_t Twim_status;							    //Status of transaction
static uint8_t Twim_result;											//Result of transaction

static SemaphoreHandle_t SemaBus = NULL;
static SemaphoreHandle_t SemaRx = NULL;

//Public function definitions
void DriverTWIMInit()
{
	Twim_status=0;
	TWIM_PORT.PIN0CTRL=0b00011000;
	TWIM_PORT.PIN1CTRL=0b00011000;
	TWIM_BUS.MASTER.CTRLA = (TWIM_INTLEVEL << 6) |
	                               TWI_MASTER_RIEN_bm |
	                               TWI_MASTER_WIEN_bm |
	                               TWI_MASTER_ENABLE_bm;
	TWIM_BUS.MASTER.BAUD =  ((F_CPU / (2 * TWIM_BAUDRATE)) - 5);
	TWIM_BUS.MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
	
	SemaBus=xSemaphoreCreateMutex();
	SemaRx=xSemaphoreCreateBinary();

	return;
}

uint8_t TWIMWrite(uint8_t address, uint8_t *writeData,uint8_t bytesToWrite)
{
	uint8_t twi_status = TWIMWriteRead(address, writeData, bytesToWrite, NULL,0);
	return twi_status;
}

uint8_t TWIMRead(uint8_t address,uint8_t *readData,uint8_t bytesToRead)
{
	uint8_t twi_status = TWIMWriteRead(address, NULL, 0,readData, bytesToRead);
	return twi_status;
}

uint8_t TWIMWriteRead(uint8_t address, uint8_t *writeData, uint8_t bytesToWrite, uint8_t *readData, uint8_t bytesToRead)
{
	xSemaphoreTake(SemaBus, portMAX_DELAY);
	Twim_writeData=writeData;
	Twim_readData=readData;

	/*Initiate transaction if bus is ready. */
	if (Twim_status == TWIM_STATUS_READY) 
	{
		Twim_status = TWIM_STATUS_BUSY;
		Twim_result = TWIM_RESULT_UNKNOWN;

		Twim_address = address<<1;

		Twim_bytesToWrite = bytesToWrite;
		Twim_bytesToRead = bytesToRead;
		Twim_bytesWritten = 0;
		Twim_bytesRead = 0;

		/* If write command, send the START condition + Address +
		 * 'R/_W = 0'
		 */
		if (Twim_bytesToWrite > 0) {
			uint8_t writeAddress = Twim_address & ~0x01;
			TWIM_BUS.MASTER.ADDR = writeAddress;
		}

		/* If read command, send the START condition + Address +
		 * 'R/_W = 1'
		 */
		else if (Twim_bytesToRead > 0) 
		{
			uint8_t readAddress = Twim_address | 0x01;
			TWIM_BUS.MASTER.ADDR = readAddress;
		}
		xSemaphoreTake(SemaRx,portMAX_DELAY); //Block until RX complete
		
		if (Twim_result==TWIM_RESULT_OK)
		{
			xSemaphoreGive(SemaBus);
			return true;
		}
		else
		{
			printf ("Result for addr %x:%d\r\n",address,Twim_result);
			xSemaphoreGive(SemaBus);
			return false;
		}
	} 
	else 
	{
		xSemaphoreGive(SemaBus);
		return false;
	}
}

void TWIMArbitrationLostBusErrorHandler()
{
	uint8_t currentStatus = TWIM_BUS.MASTER.STATUS;

	/* If bus error. */
	if (currentStatus & TWI_MASTER_BUSERR_bm) {
		Twim_result = TWIM_RESULT_BUS_ERROR;
	}
	/* If arbitration lost. */
	else {
		Twim_result = TWIM_RESULT_ARBITRATION_LOST;
	}

	/* Clear interrupt flag. */
	TWIM_BUS.MASTER.STATUS = currentStatus | TWI_MASTER_ARBLOST_bm;

	Twim_status = TWIM_STATUS_READY;
}

void TWIMWriteHandler()
{
	/* Local variables used in if tests to avoid compiler warning. */
	uint8_t bytesToWrite  = Twim_bytesToWrite;
	uint8_t bytesToRead   = Twim_bytesToRead;

	/* If NOT acknowledged (NACK) by slave cancel the transaction. */
	if (TWIM_BUS.MASTER.STATUS & TWI_MASTER_RXACK_bm) {
		TWIM_BUS.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
		Twim_result = TWIM_RESULT_NACK_RECEIVED;
		Twim_status = TWIM_STATUS_READY;
	}

	/* If more bytes to write, send data. */
	else if (Twim_bytesWritten < bytesToWrite) {
		uint8_t data = Twim_writeData[Twim_bytesWritten];
			TWIM_BUS.MASTER.DATA = data;
		++Twim_bytesWritten;
	}

	/* If bytes to read, send repeated START condition + Address +
	 * 'R/_W = 1'
	 */
	else if (Twim_bytesRead < bytesToRead) {
		uint8_t readAddress = Twim_address | 0x01;
		TWIM_BUS.MASTER.ADDR = readAddress;
	}

	/* If transaction finished, send STOP condition and set RESULT OK. */
	else {
		TWIM_BUS.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
		TWIMTransactionFinished(TWIM_RESULT_OK);
	}
}

void TWIMReadHandler()
{
	/* Fetch data if bytes to be read. */
	if (Twim_bytesRead < TWIM_READ_BUFFER_SIZE) {
		uint8_t data = TWIM_BUS.MASTER.DATA;
		Twim_readData[Twim_bytesRead] = data;
		Twim_bytesRead++;
	}

	/* If buffer overflow, issue STOP and BUFFER_OVERFLOW condition. */
	else {
		TWIM_BUS.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
		TWIMTransactionFinished(TWIM_RESULT_BUFFER_OVERFLOW);
	}

	/* Local variable used in if test to avoid compiler warning. */
	uint8_t bytesToRead = Twim_bytesToRead;

	/* If more bytes to read, issue ACK and start a byte read. */
	if (Twim_bytesRead < bytesToRead) {
		TWIM_BUS.MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
	}

	/* If transaction finished, issue NACK and STOP condition. */
	else {
		TWIM_BUS.MASTER.CTRLC = TWI_MASTER_ACKACT_bm |
		                               TWI_MASTER_CMD_STOP_gc;
		TWIMTransactionFinished(TWIM_RESULT_OK);
	}
}

void TWIMTransactionFinished( uint8_t result)
{
	Twim_result = result;
	Twim_status = TWIM_STATUS_READY;
}

ISR (TWIM_BUS_vect)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	uint8_t currentStatus = TWIM_BUS.MASTER.STATUS;

	/* If arbitration lost or bus error. */
	if ((currentStatus & TWI_MASTER_ARBLOST_bm) ||
	(currentStatus & TWI_MASTER_BUSERR_bm)) {

		TWIMArbitrationLostBusErrorHandler();
	}

	/* If master write interrupt. */
	else if (currentStatus & TWI_MASTER_WIF_bm) {
		TWIMWriteHandler();
	}

	/* If master read interrupt. */
	else if (currentStatus & TWI_MASTER_RIF_bm) {
		TWIMReadHandler();
	}

	/* If unexpected state. */
	else {
		TWIMTransactionFinished(TWIM_RESULT_FAIL);
	}
	
	if (Twim_status == TWIM_STATUS_READY) //Transaction finished
	{
		BaseType_t res=xSemaphoreGiveFromISR( SemaRx, &xHigherPriorityTaskWoken );

		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );		
	}
}




