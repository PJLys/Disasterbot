#include "DriverMotor.h"
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdint.h>

volatile int motor1Pos, motor2Pos;

void DriverMotorInit(void)
{
	//GPIO INIT
	PORTF.DIRSET = 0b00111111;
	PORTF.PIN5CTRL=0b01011000; //Fault
	PORTF.PIN4CTRL=0b01011000; //Sleep
	PORTF.PIN3CTRL=0b00010000; //M2
	PORTF.PIN2CTRL=0b00010000;
	PORTF.PIN1CTRL=0b00010000; //M1
	PORTF.PIN0CTRL=0b00010000;

	//Timer init, hbridge
	TCF0.CTRLA = 0b111;			//CA has factor /8
	TCF0.CTRLB = 0b11110011;	//Enable CCx and use single slope pwm
	TCF0.PER = 4095;			//value can vary from -4095 to 4095
	
	//Encoder 1
	PORTC.INTCTRL  = 0b0101;		//Low level interrupts
	PORTC.INT0MASK = 0b10000000;	//geeft interrupt wanneer C7 verandert
	PORTC.INT1MASK = 0b01000000;	//geeft interrupt wanneer C6 verandert
	PORTC.PIN7CTRL = 0x00;			//no inv, output conf nvt,Bothedges
	PORTC.PIN6CTRL = 0x00;			//no inv, output conf nvt,Bothedges
	//Encoder 2
	PORTE.INTCTRL  = 0b0101;		//Low level interrupts
	PORTE.INT0MASK = 0b10000000;	//geeft interrupt wanneer E7 verandert
	PORTE.INT1MASK = 0b01000000;	//geeft interrupt wanneer E6 verandert
	PORTE.PIN7CTRL = 0x00;			//no inv, output conf nvt,Bothedges
	PORTE.PIN6CTRL = 0x00;			//no inv, output conf nvt,Bothedges
}



void DriverMotorSet(int16_t MotorLeft,int16_t MotorRight)
{
	printf("MotorSet %d %d\n\r", MotorLeft, MotorRight);
	if (MotorLeft<0) {
		TCF0.CCA = -1*MotorLeft;
		TCF0.CCB = 0;
	} else {
		TCF0.CCA = 0;
		TCF0.CCB = MotorLeft;
	}
	
	if (MotorRight>0) {
		TCF0.CCC = MotorRight;
		TCF0.CCD = 0;
		} else {
		TCF0.CCC = 0;
		TCF0.CCD = -1*MotorRight;
	}
}

EncoderStruct DriverMotorGetEncoder(void){
	EncoderStruct st;
	st.Cnt1 = motor1Pos;
	st.Cnt2 = motor2Pos;
	return st;
}


//###ISRs###
ISR(PORTC_INT0_vect){ //interrupt op pin C7
	char input = PORTC.IN>>6;
	if (input&0b1)				//als pin 6 hoog is
	{
		if (input&0b10)				//als rising edge op pin 7
		{
			motor1Pos--;					//teller--
			} else{						//als falling edge op pin7
			motor1Pos++;					//teller++
		}
		} else {					//als pin 6 laag is
		if (input&0b10)				//als rising edge op pin 7
		{
			motor1Pos++;					//teller++
			} else {					//als falling edge op pin7
			motor1Pos--;					//teller--
		}
	}
}

ISR(PORTC_INT1_vect){	//interrupt op pin C6
	char input = PORTC.IN>>6;
	if (input&0b10)			//als C7 hoog is
	{
		if (input&0b1)			//als RE C6	
		{
			motor1Pos++;				//teller++
		} else {				//als FE C6
			motor1Pos--;				//teller--
		}
	} else {				//als C7 laag is
		if (input&0b10)			//als RE C6
		{					
			motor1Pos--;				//teller++
		} else {				//als FE C6
			motor1Pos++;				//teller--
		}
	}
}

ISR(PORTE_INT0_vect){ //interrupt op pin E7
	char input = PORTE.IN>>6;
	if (input&0b1)				//als pin 6 hoog is
	{
		if (input&0b10)				//als rising edge op pin 7
		{
			motor2Pos--;					//teller--
			} else{						//als falling edge op pin7
			motor2Pos++;					//teller++
		}
		} else {				//als pin 6 laag is
		if (input&0b10)				//als rising edge op pin 7
		{
			motor2Pos++;					//teller++
			} else {					//als falling edge op pin7
			motor2Pos--;					//teller--
		}
	}
}

ISR(PORTE_INT1_vect){	//Interrupt op E6
	char input = PORTE.IN>>6;
	if (input&0b10)			//als E7 hoog is
	{
		if (input&0b1)			//als RE E6
		{
			motor2Pos++;			//teller++
			} else {			//als FE E6
			motor2Pos--;			//teller--
		}
		} else {			//als E7 laag is
		if (input&0b10)			//als RE E6
		{
			motor2Pos--;			//teller++
			} else {			//als FE E6
			motor2Pos++;			//teller--
		}
	}
}
