#include "hwconfig.h"

#include "DriverSysClk.h"
#include "DriverUSART.h"
#include "DriverCursorStick.h"
#include "DriverPower.h"
#include "DriverTwiMaster.h"
#include "DriverAdc.h"
#include "DriverLed.h"
#include "DriverMotor.h"
#include "DriverAdps9960.h"
#include "DriverInterrupt.h"

#include <util/delay.h>
#include <avr/sleep.h>

#include <stdio.h>

char text[10];
char a = 0b111;

int main(void)
{
	//###1###
	
	//Initialize drivers
	DriverSysClkXtalInit();					//Clock init
	DriverUSARTInit();						//USART init and link to stdio
	printf("Initialising Drivers\n\r");		//(Check if terminal is working)
	DriverTWIMInit();						//Initialize TWI in master mode
	DriverLedInit();						//Initialize LED's
	DriverPowerInit();						//Initialize aux power driver
	DriverAdcInit();						//Initialize ADC driver
	DriverMotorInit();						//Initialize motor driver
	InitInterrupts();						//Global interrupts and GPIO wake up
	DriverPowerVccAuxSet(0);				//Enable Auxillary power line
	DriverAdps9960Init();					//Photo sensor

	
	//Enable sleep
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	
	//DriverMotorSet(1500,1500);
	
	uint16_t clear, red, green, blue;
	uint16_t *cptr = &clear;
	uint16_t *rptr = &red;
	uint16_t *gptr = &green;
	uint16_t *bptr = &blue;
	
	DriverLedWrite(a); // White LEDs --> ON
	a=1;
	while(1)
	{
		a = a<<1;
		if (!(a&0x0F))
		{
			DriverMotorSet(0,0);
			a = 0b01;
		}
		DriverAdps9960Get(cptr, rptr, gptr, bptr);
		_delay_ms(1000);
		printf ("C:%d\t R:%d\t G:%d\t B:%d\t\n\r", clear, red, green, blue);
	}
	return 0;
}

/************************************************************************/
/* Takes in a value / multiple values and tells if the sensor is still on the line.                                                                     */
/************************************************************************/
int8_t is_on_line(int16_t* value){
	return 1;
}


