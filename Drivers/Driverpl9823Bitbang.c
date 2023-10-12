#include <avr/io.h>
#include "Driverpl9823.h"
#include <FreeRTOS.h>
#include <task.h>

void DriverPL9823BitbangSet(uint32_t FrontLeft,uint32_t FrontRight,uint32_t RearRight,uint32_t RearLeft);

void DriverPL9823Init()
{
	//CS init
	PORTA.DIRSET=1<<6;
	PORTA.OUTCLR=1<<6;
	//Data init
	PORTD.DIRSET=1<<3;
	PORTD.OUTSET=1<<3;
}

void DriverPL9823Set(uint32_t FrontLeft,uint32_t FrontRight,uint32_t RearRight,uint32_t RearLeft)
{
	PORTA.OUTSET=1<<6; //CS enable
	DriverPL9823BitbangSet(FrontLeft, FrontRight, RearRight, RearLeft);
	PORTA.OUTCLR=1<<6; //CS disable
}