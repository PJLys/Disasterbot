/*
 * sleeptask.c
 *
 * Created: 07/11/2023 11:15:08
 *  Author: pjlys
 */ 
#include "sleeptask.h"
#include "LineFollowerDirectTask.h"
#include "hwconfig.h"
#include "DriverLed.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdio.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/io.h>


volatile bool sleepflag;


static void workerSleepTask(void *pvParameters);

void InitSleepTask(void) {
	//Enable interrupts
	PMIC.CTRL |= 0b10000111;
	sei();
	
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	
	// Enable interrupt on pin 7
	PORTF.DIRCLR = 0b10000000;
	PORTF.INT0MASK = 0b10000000;
	PORTF.PIN7CTRL = 0b01011001;
	
	xTaskCreate( workerSleepTask , "slp", 256, NULL, tskIDLE_PRIORITY, NULL );
}


void workerSleepTask(void *pvParameters){
	while(1) {
		// Do nothing
		vTaskDelay(2000);
		uint8_t portf = PORTF.IN;
		printf("PORTF: %d\r", portf);

		DriverPowerVccAuxSet(0);
		PORTF.DIRCLR = 0b00111111; 
		PMIC.CTRL &= 0b11111110; // Disable Low level interrupts
		vTaskDelay(10);
		sleep_mode();
	}
}

ISR(PORTF_INT0_vect) {
	DriverLedToggle(0b1000);
	BaseType_t xHigherPriorityTaskWoken=pdFALSE;
	// Reset sleep settings
	PMIC.CTRL |= 0b00000111;
	PORTF.DIRSET = 0b00111111;
	DriverPowerVccAuxSet(1);
	printf("INTERRUPT!");
	
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
