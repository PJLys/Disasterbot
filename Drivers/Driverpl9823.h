/**
 * PL9823 RGB LED driver. Hardcoded for 4 LEDs in series connected to data line PD3, MUX line PA6, data transfer using USARTD0
 * \file Driverpl9823.h
 * \brief PL9823 RGB LED driver
*/

#ifndef DRIVER_PL9823_H
#define DRIVER_PL9823_H

#include <stdint.h>

#define PL9823_RGB(r,g,b) (((uint32_t) r<<0)|((uint32_t)g<<8)|((uint32_t)b<<16)) 

#define PL9823_RED PL9823_RGB(255,0,0)
#define PL9823_GREEN PL9823_RGB(0,255,0)
#define PL9823_BLUE PL9823_RGB(0,0,255)
#define PL9823_WHITE PL9823_RGB(255,255,255)
#define PL9823_YELLOW PL9823_RGB(255,255,0)
#define PL9823_CYAN PL9823_RGB(0,255,255)
#define PL9823_PURPLE PL9823_RGB(255,0,255)
#define PL9823_BLANK PL9823_RGB(0,0,0)

/**
 * \brief Initialize PL9823 driver
*/
void DriverPL9823Init();

/**
 * \brief Set LED status. Color is in format 0bBBBBBBBBGGGGGGGGRRRRRRRR
 * \param FrontLeft: Respective LED color. 
 * \param FrontRight: Respective LED color. 
 * \param RearRight: Respective LED color. 
 * \param RearLeft: Respective LED color. 
*/
void DriverPL9823Set(uint32_t FrontLeft,uint32_t FrontRight,uint32_t RearRight,uint32_t RearLeft);

#endif