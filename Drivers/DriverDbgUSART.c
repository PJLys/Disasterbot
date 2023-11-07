#include "DriverDbgUSART.h"


void DbgPrint(char *Text)
{
	DbgPrintn(Text,0);	
}

void DbgPrintn(char *Text,int n)
{
	static int i;
	i=0;
	while (*Text!=0 && (i<n || n==0))
	{
		USART.DATA = *Text;
		while (!(USART.STATUS & 0b01000000));
		USART.STATUS=0b01000000;
		Text++;
		i++;
	}
}

void DbgPrintInt(uint16_t Data)
{
	//Use statics to minimize stack usage
	static int i;
	static uint8_t pf;
	
	i=10000;
	pf=0;
	while (i>0)
	{
		if (((Data/i)%10)!=0 || pf) //Trim leading 0's
		{
			USART.DATA = '0'+ ((Data/i)%10);
			while (!(USART.STATUS & 0b01000000));
			USART.STATUS=0b01000000;
			pf=1;		
		}
		i/=10;
	}
}

