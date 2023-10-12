#include "DriverLed.h"

void DriverLedInit(void)
{
	PORTB.DIRSET=0b00000111;   //LED1-3 zijn outputs op PB[0:2]
	PORTB.PIN0CTRL=0b01011000;	//LOW --> Stroom vloeit
	PORTB.PIN1CTRL=0b01011000;		//Invert bit hoog, dus ik kan 1 schrijven
	PORTB.PIN2CTRL=0b01011000;		//Pullup weerstand nodig
	
	PORTA.DIRSET=0b10000000;	//LED4 is output on PA[7]
	PORTA.PIN7CTRL=0b01011000;
}

void DriverLedWrite(uint8_t LedData)
{
	PORTB.OUT = LedData&0b00000111;		 //ik schrijf alleen maar de eerste 3 bits van LedData naar poortB
	PORTA.OUT = (LedData&0b00001000)<<4; //ik schrijf enkel bit 3 naar PA7 voor led 4
}

void DriverLedSet(uint8_t LedData)
{
	PORTB.OUTSET = LedData&0b0111;
	PORTA.OUTSET = (LedData&0b1000)<<4;
}

void DriverLedClear(uint8_t LedData)
{
	PORTB.OUTCLR = LedData&0b0111;
	PORTA.OUTCLR = (LedData&0b1000)<<4; //read A and shift to the right, clear on the 1 bits (AND), mask, shift to left

}

void DriverLedToggle(uint8_t LedData)
{
	PORTB.OUTTGL = LedData&0b0111;
	PORTA.OUTTGL = (LedData&0b1000)<<4;
}