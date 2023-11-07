#include "MotorSpeedTask.h"

#include "MotorPosTask.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdio.h>

typedef struct
{
	int16_t Speed1;
	int16_t Speed2;
} MotorSpeedStruct;

static MotorSpeedStruct MotorSpeed;

static QueueHandle_t MotorSpeedQueue;
static TaskHandle_t MotorSpeedTaskHandle;

static TickType_t xLastWakeTime;

//Private function prototypes
void WorkerMotorSpeed(void *pvParameters);

//Function definitions
void InitMotorSpeedTask()
{
	xTaskCreate( WorkerMotorSpeed, "motspd", 256, NULL, tskIDLE_PRIORITY+3, &MotorSpeedTaskHandle );	
	DisableMotorSpeedTask();
	
	MotorSpeedQueue=xQueueCreate(1,sizeof(MotorSpeedStruct));
	MotorSpeedSet(0,0);
}

void EnableMotorSpeedTask()
{
	xLastWakeTime = xTaskGetTickCount();
	vTaskResume(MotorSpeedTaskHandle);
}

void DisableMotorSpeedTask()
{
	vTaskSuspend(MotorSpeedTaskHandle);
}

void MotorSpeedSet(int16_t Speed1,int16_t Speed2)
{
	
	MotorSpeed.Speed1=Speed1;
	MotorSpeed.Speed2=Speed2;
	xQueueOverwrite(MotorSpeedQueue,&MotorSpeed);
	
}

void WorkerMotorSpeed(void *pvParameters)
{
	const TickType_t xPeriod = 10;
	MotorSpeedStruct MotorSpeed;

	xLastWakeTime = xTaskGetTickCount();
	
	while (1)
	{
			xQueuePeek(MotorSpeedQueue,&MotorSpeed,portMAX_DELAY);
			MotorPosIncSet(MotorSpeed.Speed1,MotorSpeed.Speed2);
			
			vTaskDelayUntil( &xLastWakeTime, xPeriod );
	}

}
