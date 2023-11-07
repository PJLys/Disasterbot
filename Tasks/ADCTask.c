#include "hwconfig.h"

#include "ADCTask.h"
#include "DriverLed.h"
#include "DriverAdc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

QueueHandle_t ADCQueue;

void WorkerADC(void *pvParameters);

//Function definitions
void InitADCTask()
{
	ADCQueue=xQueueCreate(1,sizeof(ADCStruct));
	xTaskCreate( WorkerADC, "ADC", 512, NULL, tskIDLE_PRIORITY+2, NULL );
}

void WorkerADC(void *pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xPeriod = 10;
	ADCStruct ADCData;
	
	DriverLedSet(0b0111);
	
	xLastWakeTime=xTaskGetTickCount();
	while(1)
	{
		ADCData.PhotoL=DriverAdcGetCh(0,4);
		ADCData.PhotoM=DriverAdcGetCh(1,4);
		ADCData.PhotoR=DriverAdcGetCh(2,4);
		ADCData.Potmeter=DriverAdcGetCh(3,4);
		xQueueOverwrite(ADCQueue,&ADCData);
		vTaskDelayUntil( &xLastWakeTime, xPeriod );
	}
}

ADCStruct GetADCData(void)
{
	ADCStruct ADCData;
	xQueuePeek(ADCQueue,&ADCData,portMAX_DELAY);
	return ADCData;
}