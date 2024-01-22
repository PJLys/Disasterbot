
#ifndef INC_LTR329_H_
#define INC_LTR329_H_

#include "stm32l4xx_hal.h"
#include <stdint.h>

// LTR-329ALS-01 I2C Address
#define LTR329_I2C_ADDRESS 0x29 << 1  // Shifted for 8-bit format

// Register Addresses
#define LTR329_ALS_CONTR 0x80
#define LTR329_ALS_MEAS_RATE 0x85
#define LTR329_PART_ID 0x86
#define LTR329_MANUFAC_ID 0x87
#define LTR329_ALS_DATA_CH1_0 0x88
#define LTR329_ALS_DATA_CH1_1 0x89
#define LTR329_ALS_DATA_CH0_0 0x8A
#define LTR329_ALS_DATA_CH0_1 0x8B

// Configuration Values
#define LTR329_ACTIVE_MODE 0x01
#define LTR329_MEAS_RATE 0x03  // Example: 100ms integration time

// Function Prototypes
void LTR329_Init(void);
void LTR329_Read_Light(uint16_t *ch0, uint16_t *ch1);


#endif /* INC_LTR329_H_ */
