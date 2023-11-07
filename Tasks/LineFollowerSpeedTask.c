#include "LineFollowerSpeedTask.h"

#include "hwconfig.h"

#include "DriverLed.h"
#include "DriverAdc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "MotorSpeedTask.h"
#include "ADCTask.h"
#include <stdio.h>

static int16_t SensorOffset=-100;
static float ReqSpeed;
static float Speed=0.0;

static SemaphoreHandle_t StartSema,StopSema; 

void WorkerLineFollowerSpeed(void *pvParameters);


//Function definitions
void InitLineFollowerSpeedTask()
{
	StartSema=xSemaphoreCreateBinary();
	StopSema=xSemaphoreCreateBinary();
	xTaskCreate( WorkerLineFollowerSpeed, "LFSpeed", 512, NULL, tskIDLE_PRIORITY+2, NULL );
}

void WorkerLineFollowerSpeed(void *pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xPeriod = 10;
	float Diff;
	float OutL,OutR;
	ADCStruct ADCData;
	
	while(1)
	{
		xSemaphoreTake(StartSema,portMAX_DELAY);
		Speed=ReqSpeed;
		xLastWakeTime=xTaskGetTickCount();
		while (1)
		{
			
			ADCData=GetADCData();
			Diff=(float) (ADCData.PhotoL-ADCData.PhotoR+SensorOffset);
			//printf ("Diff:%f\r\n",Diff);
			OutL=Speed+(Diff*(float) (LINEFOLLOW_SPEED_KP));
			OutR=Speed-(Diff*(float) (LINEFOLLOW_SPEED_KP));
			if (OutL<0) OutL=0;
			if (OutR<0) OutR=0;
			MotorSpeedSet((int16_t) OutL, (int16_t) OutR);	
			
			if (xSemaphoreTake(StopSema,0)==pdPASS)
			{
				MotorSpeedSet(0,0);	
				break;
			}
			vTaskDelayUntil( &xLastWakeTime, xPeriod );
		}

		
		
	}
}

void StartLineFollower(float Speed)
{
	ReqSpeed=Speed;
	xSemaphoreGive(StartSema);
	
}
void StopLineFollower()
{
	xSemaphoreGive(StopSema);
	
}