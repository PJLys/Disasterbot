/**
 * Generic ADC driver, enabled reading arbitrary ADC channels in single ended or differential mode
 * \file DriverAdc.h
 * \brief ADC driver
*/


#ifndef DRIVERADC_H
#define DRIVERADC_H

#include <stdint.h>

/**
 * \brief Initialize ADC driver
*/
void DriverAdcInit(void);			

/**
 * \brief Reads designated ADC pins, blocks until a value has been read
 * \param ChannelPos: 0 to 15
 * \param ChannelNeg: -1 for single ended, 0 to 7 for differential mode
 * \Return value: -2048 to 2047 (signed) or 0 to 4095 (unsigned)
 *                10000: Invalid channel settings
*/
int16_t DriverAdcGetCh(int8_t PinPos,int8_t PinNeg);


#endif