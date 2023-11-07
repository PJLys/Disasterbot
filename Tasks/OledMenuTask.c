#include "OledMenuTask.h"

#include "DriverOled.h"
#include "DriverCursorstick.h"
#include "DriverLed.h"
#include "DriverAdps9960.h"
#include "DriverMotor.h"
#include "DriverMPU6050.h"
#include "DriverVL53L0X.h"

#include "MotorPosTask.h"
#include "MotorSpeedTask.h"
#include "LineFollowerSpeedTask.h"
#include "LineFollowerDirectTask.h"
#include "ADCTask.h"
#include "RGBTask.h"
#include "GyroTask.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>


#define NUM_LINES 10
#define DISPLAYED_LINES 6
#define MAX_TASKS 20 //Max number of tasks to display in CPU stats page

char MenuLines[NUM_LINES][20]=
{
	"  CPU status",
	"  Motor speed",
	"  ADPS9960",
	"  ADC",
	"  Follow Line speed",
	"  Follow Line dir",
	"  RGB",
	"  Gyro",
	"  Ranger",
	"  HALT"
};

static uint8_t TopItem=0;
static uint8_t SelItem=0;

//Private function prototypes
void WorkerOLEDMenu(void *pvParameters);
void DrawMenu();
void DownMenu();
void UpMenu();
void PageCPUStatus();
void PageMotorSpeed();
void PageADPS9960();
void PageADC();
void PageLineFollowSpeed();
void PageLineFollowDirect();
void PageRGB();
void PageGyro();
void PageRanger();

//Function definitions
void InitOLEDMenuTask()
{
	xTaskCreate( WorkerOLEDMenu, "oledmen", 1024, NULL, tskIDLE_PRIORITY+2, NULL );	
}

void WorkerOLEDMenu(void *pvParameters)
{
	uint8_t Stick;
	
	while(1)
	{
		Stick=DriverCursorStickGetFifo(300);
		if (Stick & CURSOR_DOWN) DownMenu();
		else if (Stick & CURSOR_UP) UpMenu();
		else if (Stick & CURSOR_PRESS)
		{
			if (SelItem==0) PageCPUStatus();
			else if (SelItem==1) PageMotorSpeed();
			else if (SelItem==2) PageADPS9960();
			else if (SelItem==3) PageADC();
			else if (SelItem==4) PageLineFollowSpeed();
			else if (SelItem==5) PageLineFollowDirect();
			else if (SelItem==6) PageRGB();
			else if (SelItem==7) PageGyro();	
			else if (SelItem==8) PageRanger();
			else if (SelItem==9) configASSERT(false); //Force halt
		}
		
		DrawMenu(TopItem,SelItem);
	}
}

void DrawMenu()
{
	uint8_t i1=TopItem,i2=TopItem+DISPLAYED_LINES-1;
	uint8_t a;
	
	DriverOLEDClearScreen();
	//Print menu lines
	for (a=i1;a<=i2;a++)
	{
		DriverOLEDPrintSmText(a-i1,MenuLines[a],0);
	}

	
	//Print cursor
	DriverOLEDPrintSmChar(0,SelItem-TopItem,'*',0);
	DriverOLEDUpdate();
	
}


void DownMenu()
{
	if (SelItem<NUM_LINES-1) SelItem++;
	if (SelItem>=TopItem+DISPLAYED_LINES) TopItem++;
}

void UpMenu()
{
	if (SelItem>0) SelItem--;
	if (SelItem<TopItem) TopItem=SelItem;
}

void PageCPUStatus()
{
	uint8_t Stick;
	char s[32];
	TaskStatus_t TaskStatusArray[MAX_TASKS];
	TaskStatus_t IdleTaskStatus;
	uint32_t TotalRunTime;
	int NumTasks;
	float Load;
	
	uint32_t IdleTime;
	uint32_t OldTotalRunTime=0,OldIdleTime=0;
	
	vTaskDelay(300);
	while (1)
	{
		Stick=DriverCursorStickGetFifo(300);
		if (Stick & CURSOR_PRESS) break;
		
		
		DriverOLEDClearScreen();
		
		DriverOLEDPrintSmText(0,"Mem free:",0);
		sprintf (s,"%6u/%6u",xPortGetFreeHeapSize(),configTOTAL_HEAP_SIZE);
		DriverOLEDPrintSmText(1,s,0);
		
		
		NumTasks=uxTaskGetSystemState(TaskStatusArray,MAX_TASKS,&TotalRunTime);
		sprintf (s,"Number of tasks:%d",NumTasks);
		DriverOLEDPrintSmText(2,s,0);
		
		//Calculate system load
		vTaskGetTaskInfo(xTaskGetIdleTaskHandle(),&IdleTaskStatus,pdFALSE,eInvalid);
		IdleTime=IdleTaskStatus.ulRunTimeCounter;
		Load=(1-((float) (IdleTime-OldIdleTime)/ (float)(TotalRunTime-OldTotalRunTime)))*100;
		sprintf(s,"CPU Load:%2.2f",Load);
		DriverOLEDPrintSmText(3,s,0);
		
		DriverOLEDUpdate();
		OldIdleTime=IdleTime;
		OldTotalRunTime=TotalRunTime;
		
	}
	
}

void PageMotorSpeed()
{
	uint8_t Stick;
	int16_t Speed1=50,Speed2=50;
	char s[32];
	
	//Phase calculation for encoder 1
	uint32_t EdgeList1[3];	//Edge list: rising edge on A, rising edge on B, rising edge on A
	uint8_t EdgeIdx1;
	float Phase1;
	
	//Phase calculation for encoder 2
	uint32_t EdgeList2[3];	//Edge list: rising edge on A, rising edge on B, rising edge on A
	uint8_t EdgeIdx2;
	float Phase2;
	
	EncoderStruct EncoderInfo;
	EncoderEventStruct Event;
			
	EnableMotorPosTask();	
	EnableMotorSpeedTask();	
	while (1)
	{
		EncoderInfo=DriverMotorGetEncoder();

		EdgeIdx1=0; EdgeIdx2=0;
		while(1) 
		{
			Event=DriverMotorGetEncoderEvent();
			if (Event.Time==0) break; //No more events
			
			//Parse events for encoder 1
			if (EdgeIdx1==0)
			{
				if (Event.Event==RISING_1A)
				{
					EdgeList1[0]=Event.Time;
					EdgeIdx1++;
				}
			}
			else if (EdgeIdx1==1)
			{
				if (Event.Event==RISING_1B)
				{
					EdgeList1[1]=Event.Time;
					EdgeIdx1++;
				}
			}
			else if (EdgeIdx1==2)
			{
				if (Event.Event==RISING_1A)
				{
					EdgeList1[2]=Event.Time;
					EdgeIdx1++;
				}
			}

			//Parse events for encoder 2
			if (EdgeIdx2==0)
			{
				if (Event.Event==RISING_2A)
				{
					EdgeList2[0]=Event.Time;
					EdgeIdx2++;
				}
			}
			else if (EdgeIdx2==1)
			{
				if (Event.Event==RISING_2B)
				{
					EdgeList2[1]=Event.Time;
					EdgeIdx2++;
				}
			}
			else if (EdgeIdx2==2)
			{
				if (Event.Event==RISING_2A)
				{
					EdgeList2[2]=Event.Time;
					EdgeIdx2++;
				}
			}
		}
		if (EdgeIdx1==3) //All events needed are detected
		{
			Phase1=(float) ((EdgeList1[1]-EdgeList1[0])/ (float) (EdgeList1[2]-EdgeList1[0]))*180;
		}
		if (EdgeIdx2==3) //All events needed are detected
		{
			Phase2=(float) ((EdgeList2[1]-EdgeList2[0])/ (float) (EdgeList2[2]-EdgeList2[0]))*180;
		}
		
		
		Stick=DriverCursorStickGetFifo(300);
		
		DriverOLEDClearScreen();
		
		sprintf(s,"  S1:%d",Speed1);
		DriverOLEDPrintSmText(0,s,0);
		sprintf(s,"  S2:%d",Speed2);
		DriverOLEDPrintSmText(1,s,0);

		sprintf(s,"  A1:%d",EncoderInfo.Cnt1);
		DriverOLEDPrintSmText(2,s,0);
		sprintf(s,"  A2:%d",EncoderInfo.Cnt2);
		DriverOLEDPrintSmText(3,s,0);
		
		sprintf(s,"  P1:%f",Phase1);
		DriverOLEDPrintSmText(4,s,0);
		sprintf(s,"  P2:%f",Phase2);
		DriverOLEDPrintSmText(5,s,0);

		
		if (Stick & CURSOR_UP) Speed1+=5;
		if (Stick & CURSOR_DOWN) Speed1-=5;
		if (Stick & CURSOR_RIGHT) Speed2+=5;
		if (Stick & CURSOR_LEFT) Speed2-=5;	
		if (Stick & CURSOR_PRESS)
		{
			MotorSpeedSet(0,0);
			vTaskDelay(300);
			DisableMotorSpeedTask();
			DisableMotorPosTask();
			
			return;
		}	
		MotorSpeedSet(Speed1,Speed2);
		
		
		DriverOLEDUpdate();
		
	}
		
}

void PageADPS9960()
{
	uint8_t Stick;
	uint16_t C,R,G,B;
	char s[32];
	
	DriverLedSet(0b0010);
	vTaskDelay(300);
	while (1)
	{
		Stick=DriverCursorStickGetFifo(300);
		
		
		DriverOLEDClearScreen();
		
		DriverAdps9960Get(&C,&R,&G,&B);
		
		sprintf(s,"C:%u",C);
		DriverOLEDPrintSmText(0,s,0);

		sprintf(s,"R:%u",R);
		DriverOLEDPrintSmText(1,s,0);
		
		sprintf(s,"G:%u",G);
		DriverOLEDPrintSmText(2,s,0);
		
		sprintf(s,"B:%u",B);
		DriverOLEDPrintSmText(3,s,0);

		
		if (Stick & CURSOR_UP);
		if (Stick & CURSOR_DOWN);
		if (Stick & CURSOR_RIGHT);
		if (Stick & CURSOR_LEFT);
		if (Stick & CURSOR_PRESS)
		{
			vTaskDelay(300);
			DriverLedClear(0b0010);
			return;
		}
	
		DriverOLEDUpdate();
		
	}	
}

void PageADC()
{
	uint8_t Stick;
	char s[32];
	ADCStruct ADCData;

	vTaskDelay(300);
	while (1)
	{
		Stick=DriverCursorStickGetFifo(300);
		
		
		DriverOLEDClearScreen();
		ADCData=GetADCData();
		

		sprintf(s,"PHOTO1:%d",ADCData.PhotoL);
		DriverOLEDPrintSmText(0,s,0);

		sprintf(s,"PHOTO2:%d",ADCData.PhotoM);
		DriverOLEDPrintSmText(1,s,0);
		
		sprintf(s,"PHOTO3:%d",ADCData.PhotoR);
		DriverOLEDPrintSmText(2,s,0);
		
		sprintf(s,"POT:%d",ADCData.Potmeter);
		DriverOLEDPrintSmText(3,s,0);
		
		sprintf(s,"DIFF:%d",ADCData.PhotoL-ADCData.PhotoR);	
		DriverOLEDPrintSmText(5,s,0);
		
		if (Stick & CURSOR_UP);
		if (Stick & CURSOR_DOWN);
		if (Stick & CURSOR_RIGHT);
		if (Stick & CURSOR_LEFT);
		if (Stick & CURSOR_PRESS)
		{
			vTaskDelay(300);
			return;
		}
		
		DriverOLEDUpdate();
		
	}
}

void PageLineFollowSpeed()
{
		uint8_t Stick;
		char s[32];
		
		EnableMotorPosTask();
		EnableMotorSpeedTask();
		StartLineFollower(40.0);
		vTaskDelay(300);
		while (1)
		{
			Stick=DriverCursorStickGetFifo(300);
					
			DriverOLEDClearScreen();
		
			DriverOLEDPrintSmText(0,"Line follower active",0);
			
			if (Stick & CURSOR_UP);
			if (Stick & CURSOR_DOWN);
			if (Stick & CURSOR_RIGHT);
			if (Stick & CURSOR_LEFT);
			if (Stick & CURSOR_PRESS)
			{
				StopLineFollower();
				DisableMotorSpeedTask();
				DisableMotorPosTask();
						
				vTaskDelay(300);
				return;
			}
			
			DriverOLEDUpdate();
		}	
}

void PageLineFollowDirect()
{
	uint8_t Stick;
	char s[32];

	EnableLineFollowerDirectTask();
	vTaskDelay(300);
	while (1)
	{
		Stick=DriverCursorStickGetFifo(300);
		
		DriverOLEDClearScreen();
		
		DriverOLEDPrintSmText(0,"Line follower active",0);
		
		if (Stick & CURSOR_UP);
		if (Stick & CURSOR_DOWN);
		if (Stick & CURSOR_RIGHT);
		if (Stick & CURSOR_LEFT);
		if (Stick & CURSOR_PRESS)
		{
			vTaskDelay(300);
			DisableLineFollowerDirectTask();
			return;
		}
		
		DriverOLEDUpdate();
	}
}

void PageRGB()
{
	uint8_t Stick;
	uint8_t Light=0;
	uint8_t BlinkLeft=0;
	uint8_t BlinkRight=0;
	uint8_t Brake=0;
	uint8_t Effect=0;

	vTaskDelay(300);
	while (1)
	{
		Stick=DriverCursorStickGetFifo(300);
		
		DriverOLEDClearScreen();
		
		DriverOLEDPrintSmText(0,"Left:toggle blink",0);
		DriverOLEDPrintSmText(1,"Right:toggle blink",0);
		DriverOLEDPrintSmText(2,"Down:toggle brake",0);
		DriverOLEDPrintSmText(3,"Up:toggle light",0);		


		
		if (Stick & CURSOR_UP)
		{
			if (Light==0) Light=1;
			else if (Light==1) Light=2;
			else if (Light==2) Light=0;
		}
		if (Stick & CURSOR_DOWN)
		{
			if (Brake==0) Brake=1;
			else Brake=0;
		}
		if (Stick & CURSOR_RIGHT)
		{
			if (BlinkRight==0) BlinkRight=1;
			else BlinkRight=0;
		}
		if (Stick & CURSOR_LEFT)
		{
			if (BlinkLeft==0) BlinkLeft=1;
			else BlinkLeft=0;
		}
		if (Stick & CURSOR_PRESS)
		{
			vTaskDelay(300);
			return;
		}
		
		if (Light==0) Effect=RGB_NONE;
		if (Light==1) Effect=RGB_HEADLIGHT_LOW;
		if (Light==2) Effect=RGB_HEADLIGHT_HIGH;
		if (Brake==1) Effect|=RGB_BRAKE;
		if (BlinkLeft) Effect|=RGB_BLINK_LEFT;
		if (BlinkRight) Effect|=RGB_BLINK_RIGHT;
		SetRGB(Effect);
		
		DriverOLEDUpdate();

		
	}	
}

void PageGyro()
{
	uint8_t Stick;
	int16_t x,y,z;
	char s[32];
	float Yaw,YawRate;
	
	vTaskDelay(300);
	while (1)
	{
		Stick=DriverCursorStickGetFifo(300);
		
		
		DriverOLEDClearScreen();
		
		GyroGet(&YawRate,&Yaw);
		
		sprintf(s,"Yaw:%f",Yaw);
		DriverOLEDPrintSmText(0,s,0);

		sprintf(s,"YawRate:%f",YawRate);
		DriverOLEDPrintSmText(1,s,0);
		
		if (Stick & CURSOR_UP);
		if (Stick & CURSOR_DOWN);
		if (Stick & CURSOR_RIGHT);
		if (Stick & CURSOR_LEFT);
		if (Stick & CURSOR_PRESS)
		{
			vTaskDelay(300);
			return;
		}
		
		DriverOLEDUpdate();
		
	}
}

void PageRanger()
{
	uint8_t Stick;
	uint16_t Distance;
	char s[32];
	
	vTaskDelay(300);
	while (1)
	{
		Stick=DriverCursorStickGetFifo(300);
		
		
		DriverOLEDClearScreen();
		
		Distance=DriverVL53L0XReadSingle();
		
		sprintf(s,"Range:%u",Distance);
		DriverOLEDPrintSmText(0,s,0);

		
		if (Stick & CURSOR_UP);
		if (Stick & CURSOR_DOWN);
		if (Stick & CURSOR_RIGHT);
		if (Stick & CURSOR_LEFT);
		if (Stick & CURSOR_PRESS)
		{
			vTaskDelay(300);
			return;
		}
		
		DriverOLEDUpdate();
		
	}
}