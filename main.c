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
#include <stdbool.h>
#include <stdio.h>


char text[10];
uint16_t clear, red, green, blue;

int init() {
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
	DriverPL9823Init();						//RGB Driver
	DriverPowerVccAuxSet(1);				//Enable Auxillary power line
	_delay_ms(1);
	DriverAdps9960Init();					//Photo sensor
}


int run(void) {
	printf("\n\rStart run\n\n\r");
	DriverMotorSet(1500,1500);						//Drive
	DriverLedWrite(0b111);							//Enable leds
	DriverAdps9960Get(&clear, &red, &green, &blue); //Measure photo sensor data

	while(1) {
		_delay_ms(500);
		printf ("C:%d\t R:%d\t G:%d\t B:%d\t\n\r", clear, red, green, blue);	//Print photo data
		DriverAdps9960Get(&clear, &red, &green, &blue); //Measure photo sensor data

		if (getSleepFlag()){							//Check for sleep flag set
			clearSleepFlag();
			DriverPowerVccAuxSet(0);
			DriverMotorSet(0,0);
			sei();
			sleep_cpu();
			printf("\n\rSleep\n\r");
			break;	
		}
	}
	return 0;
}


int main(void)
{
	init();
	run();
	
	return 0;
}

/************************************************************************/
/* Takes in a value / multiple values and tells if the sensor is still on the line.                                                                     */
/************************************************************************/
int8_t is_on_line(int16_t* value){
	return 1;
}




