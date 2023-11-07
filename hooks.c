#include "hwconfig.h"
#include "DriverDbgUSART.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include <avr/io.h>
#include <util/delay.h>

#define CFG_ERR_LED() PORTA.DIRSET=1<<7
#define SET_ERR_LED() PORTA.OUTSET=1<<7
#define CLR_ERR_LED() PORTA.OUTCLR=1<<7


void vApplicationIdleHook( void )
{
	
}

void vApplicationMallocFailedHook(void)
{
	taskDISABLE_INTERRUPTS();
	DbgPrint ("ERROR: memory allocation failed\r\n");
	CFG_ERR_LED();
	while (1)
	{
		SET_ERR_LED();
		_delay_ms(100);
		CLR_ERR_LED();
		_delay_ms(100);
	}
}



void vApplicationStackOverflowHook( TaskHandle_t xTask,signed char *pcTaskName )
{
	taskDISABLE_INTERRUPTS();
	CFG_ERR_LED();
	DbgPrint("STACK overflow in task ");
	DbgPrintn(pcTaskName,10);
	DbgPrint("\r\n");
	while (1)
	{
		SET_ERR_LED();
		_delay_ms(500);
		CLR_ERR_LED();
		_delay_ms(500);
	}
}

void vAssertCalled( char *File, int Line)
{
	taskDISABLE_INTERRUPTS();	
	CFG_ERR_LED();
	DbgPrint("Assert in file ");
	DbgPrintn(File,100);
	DbgPrint(", line nr ");
	DbgPrintInt(Line);
	DbgPrint("\r\n");

	while (1)
	{
		SET_ERR_LED();
		_delay_ms(100);
		CLR_ERR_LED();
		_delay_ms(500);
	}
}