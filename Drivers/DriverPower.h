/**
 * Linebot Power management driver
 * \file DriverPower.h
 * \brief Linebot Power management driver
*/

#include "hwconfig.h"

/**
 * \brief Initialize power management driver
*/
void DriverPowerInit(void);


/**
 * \brief Set auxillary Vcc state
 * \param State: 0=OFF, 1=ON
*/
void DriverPowerVccAuxSet(uint8_t State);