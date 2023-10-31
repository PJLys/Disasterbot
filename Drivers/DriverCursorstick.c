#include "DriverCursorstick.h"

void DriverCursorstickInit(void)
{
	PORTB.DIRCLR=0b11111000;
	PORTB.PIN3CTRL=0b01011000;
	PORTB.PIN4CTRL=0b01011000;
	PORTB.PIN5CTRL=0b01011000;
	PORTB.PIN6CTRL=0b01011000;
	PORTB.PIN7CTRL=0b01011000;
}

char inreg;
char outreg;

uint8_t DriverCursorstickGet(void)
{
	inreg = PORTB.IN>>3;				//READ Bin
	outreg=0;							//set outreg 0
	outreg = outreg|(inreg&0b1);		//Place the LSB of inreg on outreg(0)
	for (char i=0;i<4;i++){				//repeat this another 4 times
		inreg = inreg>>1;				//Shift inreg to the right
		outreg= outreg<<1;				//Shift the outreg to the left
		outreg = outreg|(inreg&0b1);	//The new LSB of inreg is the new LSB of outreg
	}
	return (outreg);
}