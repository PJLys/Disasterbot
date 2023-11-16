#include "LineFollowerDirectTask.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "DriverMotor.h"
#include "ADCTask.h"

#include <stdio.h>

static int16_t SensorOffset=0;
static float ReqSpeed;
static const float LineFollowKp=0.2;
static float Speed=3200.0;

static TaskHandle_t LineFollowerDirectTaskHandle;

//Private function prototypes
static void WorkerLineFollowerDirect(void *pvParameters);

//Function definitions
void InitLineFollowerDirectTask()
{
	xTaskCreate( WorkerLineFollowerDirect, "LFdirect", 256, NULL, tskIDLE_PRIORITY+3, &LineFollowerDirectTaskHandle );	
	DisableLineFollowerDirectTask();
}

void EnableLineFollowerDirectTask()
{
	vTaskResume(LineFollowerDirectTaskHandle);
}

void DisableLineFollowerDirectTask()
{
	vTaskSuspend(LineFollowerDirectTaskHandle);
}

static void WorkerLineFollowerDirect(void *pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xPeriod = 10;
	float Diff;
	float OutL,OutR;
	ADCStruct ADCData;

	
	while(1)
	{
		xLastWakeTime=xTaskGetTickCount();

		ADCData=GetADCData();
		Diff=(float) (ADCData.PhotoL-ADCData.PhotoR+SensorOffset);
		printf ("Diff:%f\r\n",Diff);
		OutL=Speed+(Diff*LineFollowKp);
		OutR=Speed-(Diff*LineFollowKp);
		
		if (OutL<0) OutL=0;
		if (OutR<0) OutR=0;
		DriverMotorSet((int16_t) OutL, (int16_t) OutR);
			

		vTaskDelayUntil( &xLastWakeTime, xPeriod );
	}

}
