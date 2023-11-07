#include "MotionTask.h"

#include "hwconfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "DriverMotor.h"
#include "LineFollowerSpeedTask.h"
#include "MotorSpeedTask.h"
#include "MotorPosTask.h"
#include "ADCTask.h"

#include <stdio.h>

#define CMD_DRIVE_STRAIGHT 1
#define CMD_DRIVE_SEGMENT 2
#define CMD_ROTATE_CENTER 3


#define MM_TO_CNT(x) (x/WHEEL_CIRC*360/DEG_PER_CNT)

typedef struct 
{
	uint8_t Cmd;
	float Params[2];
} MotionCmdStruct;

static QueueHandle_t CmdQueue;
static SemaphoreHandle_t CmdCompleteSema;

//Private function prototypes
static void WorkerMotion(void *pvParameters);
static void CmdDriveStraight(float Distance, float Speed);
static void CmdDriveSegment(float Speed);

//Function definitions
void InitMotionTask()
{
	xTaskCreate( WorkerMotion, "motion", 256, NULL, tskIDLE_PRIORITY+3, NULL );	
	CmdQueue=xQueueCreate(1,sizeof(MotionCmdStruct));
	CmdCompleteSema=xSemaphoreCreateBinary();
}

static void CmdDriveStraight(float Distance, float Speed)
{
	TickType_t xLastWakeTime;
	const TickType_t xPeriod = 10;
	EncoderStruct Encoder,EncoderTarget;
	ADCStruct ADCData;
	int Cnt;
		
	float fa;
		
	xLastWakeTime = xTaskGetTickCount();
	EnableMotorPosTask();
	EnableMotorSpeedTask();
	Encoder=DriverMotorGetEncoder();
	if (Distance>0)
	{
		//Fwd
		EncoderTarget.Cnt1=Encoder.Cnt1+MM_TO_CNT(Distance);
				
		MotorSpeedSet(MM_TO_CNT(Speed),MM_TO_CNT(Speed));
	}
	else
	{
		//Reverse
		EncoderTarget.Cnt1=Encoder.Cnt1+MM_TO_CNT(Distance);
		MotorSpeedSet(-MM_TO_CNT(Speed),-MM_TO_CNT(Speed));
	}
	while (1)
	{
		Encoder=DriverMotorGetEncoder();
				
		//Stop if target distance is reached
		if (Distance>0)
		{//Fwd
			if (Encoder.Cnt1>EncoderTarget.Cnt1) break;
		}
		else
		{//Reverse
			if (Encoder.Cnt1<EncoderTarget.Cnt1) break;
		}
				
		vTaskDelayUntil( &xLastWakeTime, xPeriod );
	}
	DisableMotorPosTask();
	DisableMotorSpeedTask();	
	
}

static void CmdDriveSegment(float Speed)
{
	TickType_t xLastWakeTime;
	const TickType_t xPeriod = 10;
	EncoderStruct Encoder,EncoderTarget;
	ADCStruct ADCData;
	int Cnt;
			
	float fa;
			
	xLastWakeTime = xTaskGetTickCount();
	EnableMotorPosTask();
	EnableMotorSpeedTask();
	Encoder=DriverMotorGetEncoder();
				
	StartLineFollower(MM_TO_CNT(Speed));

	Cnt=0;
	while (1)
	{
		ADCData=GetADCData();
		if ((ADCData.PhotoL+ADCData.PhotoR)/2>LINEFOLLOW_THRESHOLD)
		{
			Cnt++;
			if (Cnt>LINEFOLLOW_THRESHOLD_CNT) break;
		}
		else
		Cnt=0;
					
		vTaskDelayUntil( &xLastWakeTime, xPeriod );
	}
	StopLineFollower();
	DisableMotorPosTask();
	DisableMotorSpeedTask();
}

static void CmdRotateCenter(float Angle, float Speed)
{
	TickType_t xLastWakeTime;
	const TickType_t xPeriod = 10;
	EncoderStruct Encoder,EncoderTarget;
	ADCStruct ADCData;
	int Cnt;
		
	float fa;
	
	xLastWakeTime = xTaskGetTickCount();
	EnableMotorPosTask();
	EnableMotorSpeedTask();
	Encoder=DriverMotorGetEncoder();
	fa=WHEEL_DISTANCE*3.14/360*Angle; //Calculate wheel distance to travel
	if (fa>0)
	{
		//Fwd
		EncoderTarget.Cnt1=(float) Encoder.Cnt1-MM_TO_CNT(fa);
		MotorSpeedSet(-MM_TO_CNT(Speed),MM_TO_CNT(Speed));
	}
	else
	{
		//Reverse
		EncoderTarget.Cnt1=(float) Encoder.Cnt1-MM_TO_CNT(fa);
		MotorSpeedSet(MM_TO_CNT(Speed),-MM_TO_CNT(Speed));
	}
	while (1)
	{
		Encoder=DriverMotorGetEncoder();
		
		//Stop if target distance is reached
		if (fa>0)
		{//Fwd
			if (Encoder.Cnt1<EncoderTarget.Cnt1) break;
		}
		else
		{//Reverse
			if (Encoder.Cnt1>EncoderTarget.Cnt1) break;
		}
		
		vTaskDelayUntil( &xLastWakeTime, xPeriod );
	}
	DisableMotorPosTask();
	DisableMotorSpeedTask();
	
}

static void WorkerMotion(void *pvParameters)
{
	TickType_t xLastWakeTime;
	const TickType_t xPeriod = 10;
	MotionCmdStruct MotionCmd;
	EncoderStruct Encoder,EncoderTarget;
	ADCStruct ADCData;
	int Cnt;
	
	float fa;
	
	while (1)
	{
		xQueueReceive(CmdQueue,&MotionCmd,portMAX_DELAY);

		if (MotionCmd.Cmd==CMD_DRIVE_STRAIGHT)
		{
			CmdDriveStraight(MotionCmd.Params[0],MotionCmd.Params[1]);
		}	
		else if (MotionCmd.Cmd==CMD_DRIVE_SEGMENT)
		{
			CmdDriveSegment(MotionCmd.Params[0]);
		}
		else if (MotionCmd.Cmd==CMD_ROTATE_CENTER)
		{
			CmdRotateCenter(MotionCmd.Params[0],MotionCmd.Params[1]);	
		}			
		if (uxQueueMessagesWaiting(CmdQueue)==0) xSemaphoreGive(CmdCompleteSema);
		
	}
}

void DriveStraight(float Distance, float Speed)
{
	MotionCmdStruct MotionCmd;
	MotionCmd.Cmd=CMD_DRIVE_STRAIGHT;
	MotionCmd.Params[0]=Distance;
	MotionCmd.Params[1]=Speed;
	xQueueSendToBack(CmdQueue,&MotionCmd,portMAX_DELAY);	//Issue command
	xSemaphoreTake(CmdCompleteSema,portMAX_DELAY);			//Block until command has been executed
}
void DriveSegment(float Speed)
{
	MotionCmdStruct MotionCmd;
	MotionCmd.Cmd=CMD_DRIVE_SEGMENT;
	MotionCmd.Params[0]=Speed;
	xQueueSendToBack(CmdQueue,&MotionCmd,portMAX_DELAY);	//Issue command
	xSemaphoreTake(CmdCompleteSema,portMAX_DELAY);			//Block until command has been executed
}
void RotateCenter(float Angle, float Speed)
{
	MotionCmdStruct MotionCmd;
	MotionCmd.Cmd=CMD_ROTATE_CENTER;
	MotionCmd.Params[0]=Angle;
	MotionCmd.Params[1]=Speed;
	xQueueSendToBack(CmdQueue,&MotionCmd,portMAX_DELAY);	//Issue command
	xSemaphoreTake(CmdCompleteSema,portMAX_DELAY);			//Block until command has been executed
}