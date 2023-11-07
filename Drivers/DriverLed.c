#include "DriverLed.h"

void DriverLedInit(void)
{
	PORTB.DIRSET=0b00000111;
	PORTB.PIN0CTRL=0b01000000; //Totem pole out, inverted
	PORTB.PIN1CTRL=0b01000000; //Totem pole out, inverted
	PORTB.PIN2CTRL=0b01000000; //Totem pole out, inverted
	
	PORTA.DIRSET=0b10000000;
	PORTA.PIN7CTRL=0b01000000;	
}

void DriverLedWrite(uint8_t LedData)
{
	PORTB.OUT=(PORTB.OUT & 0b11111000) | (LedData & 0b00000111);
	PORTA.OUT=(PORTA.OUT & 0b01111111) | ((LedData & 0b00001000) << 4); 
}

void DriverLedSet(uint8_t LedData)
{
	PORTB.OUT=PORTB.OUT | (LedData & 0b00000111);
	PORTA.OUT=PORTA.OUT | ((LedData & 0b00001000) << 4);
}

void DriverLedClear(uint8_t LedData)
{
	PORTB.OUT=PORTB.OUT & ~(LedData & 0b00000111);
	PORTA.OUT=PORTA.OUT & ~((LedData & 0b00001000) << 4);
}

void DriverLedToggle(uint8_t LedData)
{
	PORTB.OUT=PORTB.OUT ^ (LedData & 0b00000111);
	PORTA.OUT=PORTA.OUT ^ ((LedData & 0b00001000) << 4);
}