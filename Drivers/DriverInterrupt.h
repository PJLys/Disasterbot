/*
 * DriverInterrupt.h
 *
 * Created: 24/10/2023 14:48:32
 *  Author: pjlys
 */ 

#include "hwconfig.h"

/************************************************************************/
/* Set the right bits to enable interrupts.                                                                     */
/************************************************************************/
void InitInterrupts(void);


extern void PORTF_INT0_vect_ISR(void);

