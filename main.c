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
char a = 0b1;

void SimpleFunction(void);	//A simple function: print a counter (0 to 9) to the terminal

int main(void)
{
	//###1###
	
	//Initialize drivers
	DriverSysClkXtalInit();	//Clock init
	DriverUSARTInit();		//USART init and link to stdio
	DriverTWIMInit();		//Initialize TWI in master mode
	DriverCursorstickInit();//Initialize cursor stick
	DriverLedInit();		//Initialize LED's
	DriverPowerInit();		//Initialize aux power driver
	DriverAdcInit();		//Initialize ADC driver
	DriverMotorInit();		//Initialize motor driver

	DriverPowerVccAuxSet(1);//Enable Auxillary power line
	
	_delay_ms(500);
	
		
		
	DriverMotorSet(4095,4095);
		
	while(1)
	{
		DriverLedWrite(a);
		a = a<<1;
		if (!(a&0x0F))
		{
			DriverMotorSet(0,0);
			a = 0b01;
		}
		_delay_ms(500);
	}

	return 0;
}

