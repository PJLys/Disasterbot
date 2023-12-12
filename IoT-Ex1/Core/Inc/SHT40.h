#ifndef SHT40_H
#define SHT40_H

#include "stm32l4xx_hal.h"

// SHT40 I2C Address
#define SHT40_I2C_ADDRESS 0x44 << 1 // 7-bit address, shift

// Mode: high or low precision
#define SHT40_MEAS_HIGH_PRECISION 0xFD // typ 1.3, max 8.3 ms
#define SHT40_MEAS_MEDIUM_PRECISION 0xF6 // typ 3.7, max 4.5 ms
#define SHT40_MEAS_LOW_PRECISION 0xE0 // typ 6.9, max 1.6 ms

// Functions
void SHT40_Init(void);
void SHT40_Read(float *t, float *rh, uint8_t mode);

#endif // SHT40_H
