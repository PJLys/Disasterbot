/**
 * Linebot cursor stick driver
 * \file DriverCursorstick.h
 * \brief Linebot cursor stick driver
*/

#include "hwconfig.h"
#include <stdint.h>

#define CURSOR_UP    0b00001
#define CURSOR_LEFT  0b00010
#define CURSOR_DOWN  0b00100
#define CURSOR_RIGHT 0b01000
#define CURSOR_PRESS 0b10000


/**
 * \brief Initialize cursor stick driver
*/
void DriverCursorstickInit(void);


/**
 * \brief Get cursor stick state
 * \return B0-->B4: U L D R C (0=depressed, 1=pressed)
*/
uint8_t DriverCursorstickGet(void);