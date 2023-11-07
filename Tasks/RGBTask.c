#include "RGBTask.h"

#include "Driverpl9823.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdio.h>


QueueHandle_t EffectQueue;

//Private function prototypes
void WorkerRGB(void *pvParameters);


//Function definitions
void InitRGBTask()
{
	EffectQueue=xQueueCreate(1,sizeof(uint8_t));
	SetRGB(RGB_NONE);
	xTaskCreate( WorkerRGB, "rgb", 512, NULL, tskIDLE_PRIORITY+4, NULL );	
}

void SetRGB(uint8_t Effect)
{
	xQueueOverwrite(EffectQueue,&Effect);
	
}

void WorkerRGB(void *pvParameters)
{
	uint8_t Effect;
	uint32_t FrontLeft,FrontRight,RearRight,RearLeft;
	
	while(1)
	{
		FrontLeft=PL9823_BLANK;
		FrontRight=PL9823_BLANK;
		RearLeft=PL9823_BLANK;
		RearRight=PL9823_BLANK;	
		xQueuePeek(EffectQueue,&Effect,0);
		
		
		
		//Time slot 1
		if (Effect & RGB_HEADLIGHT_HIGH)
		{
			FrontRight=PL9823_RGB(255,255,255);
			FrontLeft=PL9823_RGB(255,255,255);
		}
		if (Effect & RGB_HEADLIGHT_LOW)
		{
			FrontRight=PL9823_RGB(128,128,128);
			FrontLeft=PL9823_RGB(128,128,128);
		}
		if (Effect & RGB_BRAKE)
		{
			RearRight=PL9823_RED;
			RearLeft=PL9823_RED;
		}

		DriverPL9823Set(FrontLeft,FrontRight,RearRight,RearLeft);
		vTaskDelay(250); 
		
		//Time slot 2
		if (Effect & RGB_BLINK_LEFT)
		{
			FrontLeft=PL9823_YELLOW;
			RearLeft=PL9823_YELLOW;
		}
		if (Effect & RGB_BLINK_RIGHT)
		{
			FrontRight=PL9823_YELLOW;
			RearRight=PL9823_YELLOW;
		}
		DriverPL9823Set(FrontLeft,FrontRight,RearRight,RearLeft);
		vTaskDelay(250);
	}
}
