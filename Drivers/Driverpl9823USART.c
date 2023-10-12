#include "hwconfig.h"
#include "Driverpl9823.h"
#include "util/delay.h"
#include "avr/interrupt.h"
#include "FreeRTOS.h"
#include "semphr.h"

#define BUFFER_SIZE 78
//PL9823 coding:
//Reset: 133 bit periods 1
//0: 01111
//1: 00001


void PortSetup(void);
void SendByte(uint8_t Data);
void ShiftInBit(uint8_t Bit);
void ShiftReset();
void ShiftInReset();
void ShiftIn0();
void ShiftIn1();
void ShiftInByte(uint8_t Data);
void SendBuffer();

static uint8_t BitIdx=0,ByteIdx=0;
static uint8_t DataBuffer[BUFFER_SIZE];
static volatile uint8_t TxIdx;


void SendByte(uint8_t Data)
{
	while (!(USARTD0.STATUS&(1<<5)));
	USARTD0.STATUS=(1<<6); //Clear TXCIF
	USARTD0.DATA=Data;
}

void ShiftInBit(uint8_t Bit)
{
	configASSERT(ByteIdx<BUFFER_SIZE);
	if (BitIdx==0) DataBuffer[ByteIdx]=255; //On first bit, clear the byte 
	
	
	if (ByteIdx<BUFFER_SIZE && !Bit) DataBuffer[ByteIdx]&=~(0x80>>BitIdx);
	BitIdx++;
	if (BitIdx==8)
	{
		BitIdx=0;
		ByteIdx++;
	}
}

void ShiftReset()
{
	uint8_t a;
	for (a=0;a<BUFFER_SIZE;a++)
		DataBuffer[a]=255;	
	ByteIdx=0;	
}
void ShiftInReset()
{
	uint8_t a;
	for (a=0;a<133;a++) ShiftInBit(1);
}

void ShiftIn0()
{
	ShiftInBit(0);
	ShiftInBit(1);
	ShiftInBit(1);
	ShiftInBit(1);
	ShiftInBit(1);
}

void ShiftIn1()
{
	ShiftInBit(0);
	ShiftInBit(0);
	ShiftInBit(0);
	ShiftInBit(0);
	ShiftInBit(1);
}

void ShiftInByte(uint8_t Data)
{
	uint8_t a;
	for (a=0;a<8;a++)
	{
		if (Data & 0x80) ShiftIn1(); else ShiftIn0();
		Data<<=1;
	}
}


void SendBuffer()
{
	uint8_t a;
	TxIdx=0;
	
	PORTA.OUTSET=1<<6; //CS enable
	for (a=0;a<BUFFER_SIZE;a++)
		SendByte(DataBuffer[a]);
	_delay_us(30);	
	PORTA.OUTCLR=1<<6; //CS disable
	
}

void PortSetup(void)
{
	//Data init
	PORTD.DIRSET=1<<3;
	PORTD.PIN3CTRL=0b01000000;
	USARTD0.CTRLA=0b00000000;
	USARTD0.CTRLB=0b1000;
	USARTD0.CTRLC=0b11000000;
	USARTD0.BAUDCTRLA=5; //0.375µs per bit, 
}

void DriverPL9823Init()
{
	//CS init
	PORTA.DIRSET=1<<6;
	PORTA.OUTCLR=1<<6;
}

void DriverPL9823Set(uint32_t FrontLeft,uint32_t FrontRight,uint32_t RearRight,uint32_t RearLeft)
{
	uint8_t a;
	PortSetup();
	
	ShiftReset();

	ShiftInReset();
	
	ShiftInByte((uint8_t) ((FrontLeft>>0) & 0xFF));
	ShiftInByte((uint8_t) ((FrontLeft>>8) & 0xFF));
	ShiftInByte((uint8_t) ((FrontLeft>>16) & 0xFF));

	ShiftInByte((uint8_t) ((FrontRight>>0) & 0xFF));
	ShiftInByte((uint8_t) ((FrontRight>>8) & 0xFF));
	ShiftInByte((uint8_t) ((FrontRight>>16) & 0xFF));
	
	ShiftInByte((uint8_t) ((RearRight>>0) & 0xFF));
	ShiftInByte((uint8_t) ((RearRight>>8) & 0xFF));
	ShiftInByte((uint8_t) ((RearRight>>16) & 0xFF));
	
	ShiftInByte((uint8_t) ((RearLeft>>0) & 0xFF));
	ShiftInByte((uint8_t) ((RearLeft>>8) & 0xFF));
	ShiftInByte((uint8_t) ((RearLeft>>16) & 0xFF));
	
	portENTER_CRITICAL();
	SendBuffer(); //Approx 260 µs for entire buffer
	portEXIT_CRITICAL();
}

