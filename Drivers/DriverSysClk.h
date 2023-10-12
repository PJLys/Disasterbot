/**
 * XMEGA System clock (SYSCLK) driver
 * \file DriverSysClk.h
 * \brief XMEGA System clock (SYSCLK) driver
*/

//System clock module
//Following macro's should be present in hwconfig.h: F_CPU, F_XTAL

#ifndef DRIVER_SYSCLK_H
#define DRIVER_SYSCLK_H

#include "hwconfig.h"
#include "stdint.h"


/**
 * \brief Set system clock according to settings in hwconfig.h. F_CPU and F_XTAL should be defined.
 * \return 0: success, -1: error
*/
int8_t DriverSysClkXtalInit(void);


#endif