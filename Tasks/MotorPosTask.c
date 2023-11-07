#include "hwconfig.h"
#include "MotorPosTask.h"

#include "DriverMotor.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#include <stdio.h>



typedef struct
{
	int16_t IncSet1,IncSet2;
} IncSetStruct;

static SemaphoreHandle_t ResetSetpointSema;
static QueueHandle_t IncSetQueue;
static TaskHandle_t MotorPosTaskHandle;

static TickType_t xLastWakeTime;

//Private function prototypes
void WorkerMotorPos(void *pvParameters);

//Function definitions
void InitMotorPosTask()
{
	DriverMotorInit();
	xTaskCreate( WorkerMotorPos, "motpos", 256, NULL, tskIDLE_PRIORITY+3, &MotorPosTaskHandle );
	DisableMotorPosTask(); 	
	
	IncSetQueue=xQueueCreate(1,sizeof(IncSetStruct));
	vSemaphoreCreateBinary(ResetSetpointSema);
}

void EnableMotorPosTask()
{
	DriverMotorResetEncoder();
	xSemaphoreGive(ResetSetpointSema);
	xLastWakeTime = xTaskGetTickCount();
	vTaskResume(MotorPosTaskHandle);
}

void DisableMotorPosTask()
{
	vTaskSuspend(MotorPosTaskHandle);
	DriverMotorSet(0,0);
}


void MotorPosIncSet(int16_t IncSet1,int16_t IncSet2)
{
	IncSetStruct IncSet;
	IncSet.IncSet1=IncSet1;
	IncSet.IncSet2=IncSet2;
	xQueueSend(IncSetQueue,&IncSet,portMAX_DELAY);
}

void WorkerMotorPos(void *pvParameters)
{
	
	const TickType_t xPeriod = 10;
	
	float Set1=0,Set2=0;
	float Out1,Out2;
	float Err1=0,Err2=0;
	
	IncSetStruct IncSet;
	EncoderStruct EncoderInfo;
	
	xLastWakeTime = xTaskGetTickCount();
	
	while (1)
	{		
			EncoderInfo=DriverMotorGetEncoder();

			//Process reset setpoint command
			if (xSemaphoreTake(ResetSetpointSema,0)==pdPASS)
			{
				Set1=EncoderInfo.Cnt1;
				Set2=EncoderInfo.Cnt2;
			}
			
			//Process setpoint increment data
			if (xQueueReceive(IncSetQueue,&IncSet,0)==pdPASS)
			{
				Set1+=((float) IncSet.IncSet1)/100;
				Set2+=((float) IncSet.IncSet2)/100;
			}
			
			//PID 1
			Err1=Set1-EncoderInfo.Cnt1;
			Out1=Err1*MOTPOS_KP;
			if (Out1>0) Out1+=MOTPOS_FF_OFFSET;
			if (Out1>4095) Out1=4095;
			if (Out1<0) Out1-=MOTPOS_FF_OFFSET;			
			if (Out1<-4095) Out1=-4095;

			//PID 2
			Err2=Set2-EncoderInfo.Cnt2;
			Out2=Err2*MOTPOS_KP;
			if (Out2>0) Out2+=MOTPOS_FF_OFFSET;
			if (Out2>4095) Out2=4095;
			if (Out2<0) Out2-=MOTPOS_FF_OFFSET;
			if (Out2<-4095) Out2=-4095;
			
			//printf ("Err:%f   Out:%f\r\n",Err2 ,Out2);			
			
			DriverMotorSet(Out1,Out2);
			
			vTaskDelayUntil( &xLastWakeTime, xPeriod );
	}
}
