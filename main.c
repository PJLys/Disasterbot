#include "hwconfig.h"

#include "FreeRTOS.h"
#include "task.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#include "DriverSysClk.h"

#include "StartupTask.h"

#include <util/delay.h>

#include <stdio.h>
#include <stdlib.h>

uint8_t *ucHeap;

int main(void)
{
	DriverSysClkXtalInit();	//Clock init
	//Allocate FreeRTOS heap
	ucHeap=malloc(configTOTAL_HEAP_SIZE);
	if (ucHeap==NULL) while(1);
	
	//Enable interrupts
	PMIC.CTRL=0b111;
	sei();
	
	//Init startup task
	InitStartupTask();
	
	//Start scheduler loop
	vTaskStartScheduler();	

	return 0;
}


