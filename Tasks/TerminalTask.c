#include "TerminalTask.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "DriverPower.h"
#include "DriverMotor.h"
#include "MotionTask.h"
#include "GyroTask.h"
#include "RGBTask.h"
#include "ADCTask.h"
#include "DriverAdps9960.h"
#include "memmap.h"

#include <avr/pgmspace.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_PARS 2

//Private function prototypes
static void WorkerTerminal(void *pvParameters);
static void SplitCmd(char *Data,char *Cmd,float *Pars);

//Function definitions
void InitTerminalTask()
{
	xTaskCreate( WorkerTerminal, "term", 1024, NULL, tskIDLE_PRIORITY+1, NULL );	
}


static void SplitCmd(char *Data,char *Cmd,float *Pars)
{
	uint8_t a;
	char *ss;
	
	ss=strtok(Data," ");
	if (ss!=NULL) strcpy(Cmd,ss);
	for (a=0;a<MAX_PARS;a++) 
	{
		ss=strtok(NULL," ");
		if (ss!=NULL)
		{
			Pars[a]=atof(ss);
		}
		else
			Pars[a]=0.0;
	}
	
	/*
	printf ("Cmd:%s",Cmd);
	for (a=0;a<MAX_PARS;a++)
	{
		printf ("Par %d:%f ",a,Pars[a]);		
	}
	printf ("\r\n");
	*/

}

static void WorkerTerminal(void *pvParameters)
{
	char sbuf[200];
	float Pars[MAX_PARS];
	char Cmd[64];
	uint16_t c,r,g,b;
	
	while (1)
	{
		DriveSegment(Pars[0]);

		printf ("C>\r\n");
		fgets(sbuf,199,stdin);
		SplitCmd(sbuf,Cmd,Pars);
		

		
		//Task list command
		if (strstr(Cmd,"help"))
		{
			printf_P (PSTR("Command list:\r\n"));
			printf_P (PSTR("help :this help page\r\n"));
			printf_P (PSTR("tsklst :list FreeRTOS tasks\r\n"));
			printf_P (PSTR("memmap :show memory map\r\n"));
			printf_P (PSTR("drvstr distance speed :Drive straight over 'distance' mm at a speed of 'speed' mm/s\r\n"));
			printf_P (PSTR("rotctr angle speed :rotate 'angle' degrees around center of robot at a speed of 'speed' mm/s\r\n"));
			printf_P (PSTR("drvseg speed :follow line segment until end at a speed of 'speed' mm/s\r\n"));
			printf_P (PSTR("setled effect :set RGB led effect (see RGBTask.h)\r\n"));
			printf_P (PSTR("setmot leftmotor_pwm rightmotor_pwm :directly control motor pwm signal. Pwm is in a range of -4095 to 4095\r\n"));
			printf_P (PSTR("getenc :get motor encoder values\r\n"));
			printf_P (PSTR("getrgb :returns RGB light sensor values\r\n"));
			printf_P (PSTR("getgyr :returns gyroscope info in format 'yawrate (deg/s) yaw(deg)\r\n"));
			printf_P (PSTR("getadc :returns Analog channels in format 'left_line_sensor mid_line_sensor right_line_sensor potentiometer\r\n"));
			printf_P (PSTR("auxpwr state:'state'=1: turn on aux power net, 'state'=0: turn off aux power net\r\n"));
		}
		else if (strstr(Cmd,"tsklst"))
		{
			vTaskGetRunTimeStats(sbuf);
			puts(sbuf);
		}
		else if (strstr(Cmd,"memmap"))
		{
			MemMap();
		}
		else if (strstr(Cmd,"drvstr"))
		{
			DriveStraight(Pars[0],Pars[1]);
			printf ("OK\r\n");
		}
		else if (strstr(Cmd,"drvseg"))
		{
			DriveSegment(Pars[0]);
			printf ("OK\r\n");
		}
		else if (strstr(Cmd,"rotctr"))
		{
			RotateCenter(Pars[0],Pars[1]);
			printf ("OK\r\n");
		}
		else if (strstr(Cmd,"setled"))
		{
			SetRGB(Pars[0]);
			printf ("OK\r\n");
		}
		else if (strstr(Cmd,"setmot"))
		{
			DriverMotorSet((int16_t) Pars[0],(int16_t) Pars[1]);
			printf ("OK\r\n");
		}
		else if (strstr(Cmd,"getenc"))
		{	
			EncoderStruct Encoder;
			Encoder=DriverMotorGetEncoder();
			printf ("OK %d %d\r\n",Encoder.Cnt1,Encoder.Cnt2);
		}		
		else if (strstr(Cmd,"getrgb"))
		{
			DriverAdps9960Get(&c,&r,&g,&b);
			printf ("OK %d %d %d %d\r\n",c,r,g,b);
		}
		else if (strstr(Cmd,"getgyr"))
		{
			float YawRate,Yaw;
			GyroGet(&YawRate,&Yaw);
			printf ("OK %f %f\r\n",YawRate,Yaw);
		}
		else if (strstr(Cmd,"getadc"))
		{
			ADCStruct ADCData;
			ADCData=GetADCData();
			printf ("OK %d %d %d %d\r\n",ADCData.PhotoL,ADCData.PhotoM,ADCData.PhotoR,ADCData.Potmeter);
		}
		else if (strstr(Cmd,"auxpwr"))
		{
			DriverPowerVccAuxSet((uint8_t) Pars[0]);
			printf ("OK\r\n");
		}
		else
		printf ("Unknown command\r\n");
	}
}
