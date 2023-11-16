#include "FreeRTOS.h"
#include "DriverAdps9960.h"


#include <stdio.h>
#include <stdint.h>

#define ADPS9960_ADDR 0x39

#define REG_ENABLE 0x80
#define REG_ATIME 0x81
#define REG_CTRL1 0x8F
#define REG_CTRL1_AGAIN 0
#define REG_ENABLE_PON 0
#define REG_ENABLE_AEN 1
#define REG_STATUS 0x93
#define REG_STATUX_AVALID 0
#define REG_CDATA 0x94
#define REG_RDATA 0x96
#define REG_GDATA 0x98
#define REG_BDATA 0x9B

#define REG_ID 0x92
#define ID 0xAB

void DriverAdps9960Init(void)
{
	uint8_t res;
	uint8_t Buffer[2];
	Buffer[0]=REG_ENABLE;
	Buffer[1]=(1<<REG_ENABLE_PON);
	res=TWIMWrite(ADPS9960_ADDR,Buffer,2);
	//printf ("TwimWrite:%d\r\n",res);
	
	Buffer[0]=REG_ID;
	res=TWIMWriteRead(ADPS9960_ADDR,Buffer,1,Buffer,1);
	if (Buffer[0]!=ID)
		printf ("ADPS9960 ID readback fail: %d read\r\n",Buffer[0]);

	Buffer[0]=REG_ATIME;
	Buffer[1]=219; //100ms integration time
	res=TWIMWrite(ADPS9960_ADDR,Buffer,2);

	Buffer[0]=REG_CTRL1;
	Buffer[1]=0b11<<REG_CTRL1_AGAIN; //max gain
	res=TWIMWrite(ADPS9960_ADDR,Buffer,2);

}

void DriverAdps9960Get(uint16_t *Clear,uint16_t *Red,uint16_t *Green, uint16_t *Blue)
{
	uint8_t res;
	uint8_t Buffer[9];
	uint16_t *C=(uint16_t *) &(Buffer[0]),*R=(uint16_t *) &(Buffer[2]),*G=(uint16_t *) &(Buffer[4]),*B=(uint16_t *) &(Buffer[6]);
	uint8_t *Status=&(Buffer[0]);


	Buffer[0]=REG_ENABLE;
	Buffer[1]=(1<<REG_ENABLE_PON) | (1<<REG_ENABLE_AEN);
	res=TWIMWrite(ADPS9960_ADDR,Buffer,2);


	do 
	{
		Buffer[0]=REG_STATUS;
		res=TWIMWriteRead(ADPS9960_ADDR,Buffer,1,Buffer,1);
		configASSERT(res);
		//printf ("STATUS:%d\r\n",Buffer[0]);
	} while (!(Buffer[0] & (1<<REG_STATUX_AVALID)) );


	Buffer[0]=REG_CDATA;
	res=TWIMWriteRead(ADPS9960_ADDR,Buffer,1,Buffer,8);
	configASSERT(res);
	//printf ("TwimWriteRead:%d\r\n",res);

	if (Clear!=NULL) *Clear=*C;	
	if (Red!=NULL) *Red=*R;
	if (Green!=NULL) *Green=*G;
	if (Blue!=NULL) *Blue=*B;

}