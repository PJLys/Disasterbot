/*
 * DriverInterrupt.h
 *
 * Created: 24/10/2023 14:48:32
 *  Author: pjlys
 */ 

#include "hwconfig.h"
#include <stdbool.h>
#include <avr/sleep.h>


volatile bool timerExpired;


/************************************************************************/
/* Set the right bits to enable interrupts.                             */
/************************************************************************/
void InitInterrupts();

/************************************************************************/
/* Used in the main loop to check if the timer overflow is reached.     */
/************************************************************************/
bool getSleepFlag(void);

void clearSleepFlag(void);
