#include "hwconfig.h"

#include "DriverSysClk.h"
#include "DriverUSART.h"
#include "DriverCursorStick.h"
#include "DriverPower.h"
#include "DriverTwiMaster.h"
#include "DriverAdc.h"
#include "DriverLed.h"

#include <util/delay.h>

#include <stdio.h>

char text[10];
char a = 0b111;

int main(void)
{
	//###1###
	
	//Initialize drivers
	DriverSysClkXtalInit();	//Clock init
	DriverUSARTInit();		//USART init and link to stdio
	DriverTWIMInit();		//Initialize TWI in master mode
	//DriverCursorstickInit();//Initialize cursor stick
	DriverLedInit();		//Initialize LED's
	//DriverPowerInit();		//Initialize aux power driver
	//DriverAdcInit();		//Initialize ADC driver
	DriverMotorInit();		//Initialize motor driver
	DriverAdps9960Init();

	DriverPowerVccAuxSet(1);//Enable Auxillary power line
	
	//_delay_ms(500);
	
		
	DriverMotorSet(4095,4095);
	uint16_t Clear, Red, Green, Blue;
	DriverLedWrite(a);
	a=1;
	while(1)
	{
		a = a<<1;
		if (!(a&0x0F))
		{
			DriverMotorSet(0,0);
			a = 0b01;
		}
		DriverAdps9960Get(&Clear, &Red, &Green, &Blue);
		printf ("Clear:%d\t Red:%d\t Green:%d\t Blue:%d\t\n\r", Clear, Red, Green, Blue);
		_delay_ms(1000);
	}

	return 0;
}

