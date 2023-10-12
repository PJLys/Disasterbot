/**
 * ADPS9960 color sensor I²C driver
 * \file DriverAdps9960.h
 * \brief ADPS9960 color sensor I²C driver
*/


#ifndef DRIVER_ADPS9960_H
#define DRIVER_ADPS9960_H

#include "DriverTWIMaster.h"
#include <stdint.h>


/**
 * \brief Initialize ADPS9960 driver. DriverTWIMaster should be initialized beforehand.
*/
void DriverAdps9960Init(void);

/**
 * \brief Requests color sensor data, blocks until data has been returned
 * \param Clear: data for the unfiltered color channel will be stored here. Pass NULL if not used
 * \param Red: data for the red color channel will be stored here. Pass NULL if not used
 * \param Green: data for the green color channel will be stored here. Pass NULL if not used
 * \param Blue: data for the blue color channel will be stored here. Pass NULL if not used
*/
void DriverAdps9960Get(uint16_t *Clear,uint16_t *Red,uint16_t *Green, uint16_t *Blue);

#endif