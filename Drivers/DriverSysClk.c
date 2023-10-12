#include "DriverSysClk.h"

#pragma GCC push_options
#pragma GCC optimize ("O2") //This function must be optimized to avoid too long time interval betwee CCP write and CLK.CTRL write (4 CLK CYC max)


int8_t DriverSysClkXtalInit(void)
{
    uint8_t Mult;
	         
    // Clock setup          
    if (F_XTAL<400E3) return -1; //Invalid frequency
    if (F_XTAL<=2E6) OSC.XOSCCTRL=0b00001011; //Select <2 Mhz XTAL osc, max startup time
    else if(F_XTAL<=9E6) OSC.XOSCCTRL=0b01001011; //Select <2 Mhz XTAL osc, max startup time 
    else if(F_XTAL<=12E6) OSC.XOSCCTRL=0b10001011; //Select <2 Mhz XTAL osc, max startup time 
    else if(F_XTAL<=16E6) OSC.XOSCCTRL=0b11001011; //Select <2 Mhz XTAL osc, max startup time 
    else return -1; //Invalid frequency
    OSC.CTRL=0b01000; //Enable oscillator
    
	//Wait until XTAL osc is ready
    while (!(OSC.STATUS&0b1000)); 

	
	if (F_XTAL==F_CPU)
	{
		//Select XTAL osc as system CLK source
		CCP=0xd8;
		CLK.CTRL=0b0011;
	}	
	else if (F_CPU>F_XTAL)
	{
		//Configure PLL
		Mult=F_CPU/F_XTAL;
		if (Mult>31) return -1; //Invalid frequency; too high
		if (F_XTAL * (uint32_t) Mult !=F_CPU) return -1; //Only supports whole dividers
		OSC.PLLCTRL=0b11000000 | (Mult);
		OSC.CTRL=0b00011000; //Enable PLL
		
		//Wait until PLL is ready
		while (!(OSC.STATUS & 0b00010000));
		
		//Select PLL as system CLK source
		CCP=0xd8;
		CLK.CTRL=0b100;
	}
	return 0;
}
#pragma GCC pop_options
