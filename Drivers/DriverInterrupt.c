#include "DriverInterrupt.h";
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdint.h>

/*
 * DriverInterrupt.c
 *
 * Created: 24/10/2023 14:49:33
 *  Author: pjlys
 */ 

void InitInterrupts(void) {
	// Wake up
	PORTF.INTCTRL = 0b0011;			//High level interrupts on INT0
	PORTF.INT0MASK = 0b10000000;	//Set interrupt on SWC (pin7)
	PORTF.PIN7CTRL = 0b00000001;	//No inv; Totem; Rising
	//Interrupt handling
	PMIC.CTRL = 0b10000111;			//round robin scheduling + all interrupts enabled
	SREG = 0b10000000;				//enable global interrupt
	
	// Start timer
	TCC0.CTRLA = 0b0111;	//Prescaler = 1024 --> f = fclk /1024 = 
	TCC0.CTRLB = 0;			//Normal mode
}

ISR(PORTF_INT0_vect){
	// Wake up
}
