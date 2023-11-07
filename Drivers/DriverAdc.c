#include "DriverAdc.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

SemaphoreHandle_t AdcBusySema;					//Semaphore is taken when an ADC acquisition is performed
SemaphoreHandle_t ConversionCompleteSema;	//Semaphore used to signal completion of an ADC conversion

void DriverAdcInit(void)
{
	ADCA.CTRLA=0b00000001; //Enable ADC
	ADCA.CTRLB=0b00000110; //Manual mode, 12 bit right justified
	ADCA.REFCTRL=0b00000010; //1.00V internal reference, enable bandgap
	ADCA.PRESCALER=0b00000011; //DIV32; 1MHz ADCClk. Should be lower than 1,8MHz per spec
	ADCA.CH0.INTCTRL=0b01; //Lo pri int
	
	AdcBusySema=xSemaphoreCreateBinary();
	ConversionCompleteSema=xSemaphoreCreateBinary();
	xSemaphoreGive(AdcBusySema);		
}

int16_t DriverAdcGetCh(int8_t PinPos,int8_t PinNeg)
{
	int16_t Res;
	
	xSemaphoreTake(AdcBusySema,portMAX_DELAY);
	
	//Configure channels
	if (PinPos<16 && PinPos>=0)
		ADCA.CH0.MUXCTRL=PinPos<<3;
	else
		return 10000; //Invalid settings

	if (PinNeg==-1) 
		{			
		ADCA.CH0.CTRL=0b00000001; //Single ended mode, no gain
		ADCA.CTRLB&=~0b00010000;  //Unsigned mode		
		}		
	else if (PinNeg>=0 && PinNeg<4)
		{			
		ADCA.CH0.CTRL=0b00000010; //Differential mode, no gain
		ADCA.CTRLB|=0b00010000;   //Signed mode
		ADCA.CH0.MUXCTRL|=PinNeg;
		}						
	else if (PinNeg>=4 && PinNeg<8)
		{
		ADCA.CH0.CTRL=0b00011111; //Differential mode, div 2
		ADCA.CTRLB|=0b00010000;
		ADCA.CH0.MUXCTRL|=(PinNeg-4);	
		}
	else return 10000; //Invalid settings
	
	//Start measurement
	ADCA.CH0.CTRL|=0b10000000; //Start conversion
	
	xSemaphoreTake(ConversionCompleteSema,portMAX_DELAY);
	Res=ADCA.CH0.RES;
	xSemaphoreGive(AdcBusySema);
	return Res;
}

ISR(ADCA_CH0_vect)
{
	BaseType_t xHigherPriorityTaskWoken=pdFALSE;
	xSemaphoreGiveFromISR(ConversionCompleteSema,&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}