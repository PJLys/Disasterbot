/*
 * sleeptask.c
 *
 * Created: 07/11/2023 11:15:08
 *  Author: pjlys
 */ 
#include "sleeptask.h"
#include "LineFollowerDirectTask.h"
#include "hwconfig.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdio.h>
#include <stdbool.h>

volatile bool sleepflag;


static void workerSleepTask(void *pvParameters);

void InitSleepTask(void) {
	//Enable interrupts
	PMIC.CTRL |= 0b1000111;
	
	// Sleep mode
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	
	// Enable interrupt on pin 7
	PORTF.DIRCLR = 0b10000000;
	PORTF.INT1MASK = 0b10000000;
	PORTF.PIN7CTRL = 0b1;
	
	xTaskCreate( workerSleepTask , "slp", 256, NULL, tskIDLE_PRIORITY, NULL );
}


void workerSleepTask(void *pvParameters){
	while(1) {
		vTaskDelay(2000);
		DriverPowerVccAuxSet(0);
		PORTF.DIRCLR = 0b00111111;
		PMIC.CTRL |= 0b100;
		sleep_cpu();
		PORTF.DIRSET = 0b00111111;
	}
}

ISR(PORTF_INT1_vect) {
	printf_P("Woken up\n\r");
}





