#ifndef DRIVER_VL53L0X_H
#define DRIVER_VL53L0X_H

/**
 * Hardware: VL53L0X laser rangefinder (I²C)
 * Software: The driver supports initialization and readout of the rangefinder. An initialized DriverTwiMaster module is required. 
 * CURRENTLY NOT OPERATIONAL
 * \file vl53l0x.h
 * \brief VL53L0X laser rangefinder driver
*/

#include "DriverTwiMaster.h"
#include "avr/pgmspace.h"
#include <string.h>
#include <stdlib.h>
#include <util/delay.h>
#include <stddef.h>

#define false 0
#define true 1

/**
 * \brief Initialize rangefinder module
*/
uint8_t DriverVL53L0XInit(void);

/**
 * \brief Perform one measurement
 * \return distance in mm
*/
uint16_t DriverVL53L0XReadSingle();

uint16_t DriverVL53L0XReadContinuous();
void DriverVL53L0XStartContinuous(uint32_t period_ms);
void DriverVL53L0XStopContinuous();


#endif