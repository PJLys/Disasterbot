/**
 * MPU6050 IMU I²C driver. Currently only supports gyroscope function
 * \file DriverMPU6050.h
 * \brief MPU6050 I²C driver
*/


#ifndef DRIVER_MPU6050_H
#define DRIVER_MPU6050_H

#include "DriverTWIMaster.h"
#include <stdint.h>


/**
 * \brief Initialize MPU6050 driver. DriverTWIMaster should be initialized beforehand.
*/
void DriverMPU6050Init(void);

/**
 * \brief Requests gyroscope data, blocks until data is returned.
 * \param Gx: X gyroscope data. Pass NULL if not used. (1/131)°/s per unit
 * \param Gy: Y gyroscope data. Pass NULL if not used. (1/131)°/s per unit
 * \param Gz: Z gyroscope data. Pass NULL if not used. (1/131)°/s per unit
*/
void DriverMPU6050GyroGet(int16_t *Gx,int16_t *Gy,int16_t *Gz);

#endif