#include "DriverDbgUSART.h"
#include "hwconfig.h"

void DbgPrint(char *Text)
{
	uint8_t CtrlABack;
	uint8_t CtrlBBack;
	uint8_t CtrlCBack;
	
	CtrlABack=USART.CTRLA;
	CtrlBBack=USART.CTRLB;
	CtrlCBack=USART.CTRLC;
	
	while (*Text!=0)
	{
		USART.DATA = *Text;
		while (!(USART.STATUS & 0b01000000));
		USART.STATUS=0b01000000;
		Text++;
	}
	
	USART.CTRLA=CtrlABack;
	USART.CTRLB=CtrlBBack;
	USART.CTRLC=CtrlCBack;
}

