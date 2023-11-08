#include "DriverCursorstick.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "sleeptask.h"

#include <avr/interrupt.h>
#include <stdio.h>

static QueueHandle_t CursorstickQueue;

void DriverCursorstickInit(void)
{
	PORTB.DIRCLR=0b11111000;
	PORTB.PIN3CTRL=0b01011001; //Pull up, inverted
	PORTB.PIN4CTRL=0b01011001; //Pull up, inverted	
	PORTB.PIN5CTRL=0b01011001; //Pull up, inverted
	PORTB.PIN6CTRL=0b01011001; //Pull up, inverted
	PORTB.PIN7CTRL=0b01011001; //Pull up, inverted
	PORTB.INT0MASK=0b11111000; //Interrupt on all cursor stick lines
	PORTB.INTCTRL=0b11;		   //Enable interrupt0
	
	CursorstickQueue=xQueueCreate(CURSOR_FIFO_LENGTH,1);
}

uint8_t DriverCursorstickGet(void)
{
	uint8_t ret=0;
	if (PORTB.IN & (1<<3)) ret|=(1<<4);
	if (PORTB.IN & (1<<4)) ret|=(1<<3);
	if (PORTB.IN & (1<<5)) ret|=(1<<2);
	if (PORTB.IN & (1<<6)) ret|=(1<<1);
	if (PORTB.IN & (1<<7)) ret|=(1<<0);

	return ret;
}

uint8_t DriverCursorStickGetFifo(TickType_t BlockTime)
{
	uint8_t ButtonState;
	BaseType_t res;
	res=xQueueReceive(CursorstickQueue,&ButtonState,BlockTime);
	if (res==pdTRUE) return ButtonState;
	else return 0;

}


ISR (PORTB_INT0_vect)
{
	if (GetSleepFlag()) {
		PMIC.CTRL |= 0b00000111;
		PORTF.DIRSET = 0b00111111;
		DriverPowerVccAuxSet(1);
	}
	//ClearSleepFlag();
}
