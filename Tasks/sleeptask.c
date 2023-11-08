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
#include "DriverMotor.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdio.h>
#include <avr/interrupt.h>

volatile bool sleep;

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
	PORTF.INTCTRL = 0b11;
	
	xTaskCreate( workerSleepTask , "slp", 256, NULL, tskIDLE_PRIORITY, NULL );
}

void ClearSleepFlag() {
	sleep = false;
}

bool GetSleepFlag() {
	return sleep;
}

void workerSleepTask(void *pvParameters){
	while(1) {
		// Do nothing
		vTaskDelay(2000);
		/*
		Stop moving
		*/
		PORTF.DIRCLR = 0b00111111;

		/*
		Wake up STM with interrupt on RE of PA7 (LED4 for debugging)
		*/
		DriverLedSet(0b1000);
		
		/*
		SEND UART
		*/
		EncoderStruct encData;
		encData = DriverMotorGetEncoder();
		printf("Encoder: %d %d\n\r", encData.Cnt1, encData.Cnt2);
		vTaskDelay(25);
		
		DriverMotorResetEncoder();
		DriverLedClear(0b1000);

		
		DriverPowerVccAuxSet(0);
		PMIC.CTRL &= 0b11111100; // Only allow H-lvl interrupts
		sleep = true;
		vTaskDelay(10);
		sleep_mode();
		sleep = false;
	}
}


ISR (PORTF_INT0_vect)
{
	if (GetSleepFlag()) {
		PMIC.CTRL |= 0b00000111;
		PORTF.DIRSET = 0b00111111;
		DriverPowerVccAuxSet(1);
	}
	//ClearSleepFlag();
}

