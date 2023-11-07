#include "FreeRTOS.h"
#include "hwconfig.h"
#include "DriverMPU6050.h"

#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>

#define MPU6050_ADDR 0b1101000
#define MPU6050_WHO_AM_I 0x75
#define MPU6050_GX 0x43
#define MPU6050_GY 0x45
#define MPU6050_GZ 0x47
#define MPU6050_CONFIG 0x1A
#define MPU6050_GYRO_CONFIG 0x1B
#define MPU6050_ACC_CONFIG 0x1C
#define MPU6050_SMPRT_DIV 0x19
#define MPU6050_INT_PIN_CFG 0x37
#define MPU6050_INT_ENABLE 0x38
#define MPU6050_INT_STATUS 0x3A
#define MPU6050_PWR_MGMT_1 0x6B



void DriverMPU6050Init(void)
{
	uint8_t res;
	uint8_t Buffer[2];
	int a;
	
	Buffer[0]=MPU6050_WHO_AM_I;
	res=TWIMWriteRead(MPU6050_ADDR,Buffer,1,Buffer,1);
	if (!res)
	{
		printf ("MPU6050: comms fail\r\n");
		return;
	}
	if (Buffer[0]!=MPU6050_ADDR)
	{
		printf ("MPU6050 WHO_AM_I readback fail: %x read, %x expected\r\n",Buffer[0],MPU6050_ADDR);
		return;
	}

	//Setup sample rate
	Buffer[0]=MPU6050_SMPRT_DIV;
	Buffer[1]=0;	//Output rate/1 --> 1kHz
	res=TWIMWrite(MPU6050_ADDR,Buffer,2);
	
	//Setup CONFIG
	Buffer[0]=MPU6050_CONFIG;
	Buffer[1]=1;	//Fs=1kHz, 188Hz BW
	res=TWIMWrite(MPU6050_ADDR,Buffer,2);	
	
	//Setup GYRO_CONFIG
	Buffer[0]=MPU6050_GYRO_CONFIG;
	Buffer[1]=0;	//250°/s max rate
	res=TWIMWrite(MPU6050_ADDR,Buffer,2);	

	//Setup INT_PIN_CFG
	Buffer[0]=MPU6050_INT_PIN_CFG;
	Buffer[1]=1<<4;	//Clear INT on any read
	res=TWIMWrite(MPU6050_ADDR,Buffer,2);

	//Setup INT_ENABLE
	Buffer[0]=MPU6050_INT_ENABLE;
	Buffer[1]=1;	//Set INT on data ready
	res=TWIMWrite(MPU6050_ADDR,Buffer,2);
	
	//Setup PWR_MGMT1
	Buffer[0]=MPU6050_PWR_MGMT_1;
	Buffer[1]=1;	//Gyro X as clock
	res=TWIMWrite(MPU6050_ADDR,Buffer,2);

}

void DriverMPU6050GyroGet(int16_t *Gx,int16_t *Gy,int16_t *Gz)
{
	uint8_t res;
	uint16_t x,y,z;
	uint8_t Buffer[6];

	//Read Gyro data
	Buffer[0]=MPU6050_GX;
	res=TWIMWriteRead(MPU6050_ADDR,Buffer,1,Buffer,6);
	configASSERT(res);
	((char *) (&x))[0]=Buffer[1];
	((char *) (&x))[1]=Buffer[0];
	
	((char *) (&y))[0]=Buffer[3];
	((char *) (&y))[1]=Buffer[2];
		
	((char *) (&z))[0]=Buffer[5];
	((char *) (&z))[1]=Buffer[4];

	if (Gx!=NULL) *Gx=x;	
	if (Gy!=NULL) *Gy=y;	
	if (Gz!=NULL) *Gz=z;	
}
