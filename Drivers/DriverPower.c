#include "DriverPower.h"

void DriverPowerInit(void)
{
	//Aux Vcc GPIO setup
	PORTC.DIRSET=1<<5;
	
	
}


void DriverPowerVccAuxSet(uint8_t State)
{
	if (State)
		PORTC.OUTSET=1<<5;
	else
		PORTC.OUTCLR=1<<5;
	
}