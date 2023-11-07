#include <avr/io.h>
#include "Driverpl9823.h"

void DriverPL9823BitbangSet(uint32_t FrontLeft,uint32_t FrontRight,uint32_t RearRight,uint32_t RearLeft);

void DriverPL9823Init()
{
	PORTB.DIRSET=1<<3;
	PORTB.OUTSET=1<<3;
}

void DriverPL9823Set(uint32_t FrontLeft,uint32_t FrontRight,uint32_t RearRight,uint32_t RearLeft)
{
	
	DriverPL9823BitbangSet(FrontLeft, FrontRight, RearRight, RearLeft);
}