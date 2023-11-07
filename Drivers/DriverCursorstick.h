/**
 * Linebot cursor stick driver.
 * \file DriverCursorstick.h
 * \brief Linebot cursor stick driver
*/

#include "hwconfig.h"
#include <stdint.h>
#include <FreeRTOS.h>



#define CURSOR_UP    0b10000
#define CURSOR_LEFT  0b01000
#define CURSOR_DOWN  0b00100
#define CURSOR_RIGHT 0b00010
#define CURSOR_PRESS 0b00001


/**
 * \brief Initialize cursor stick driver
*/
void DriverCursorstickInit(void);


/**
 * \brief Get cursor stick state
 * \return B4-->B0: U L D R C (0=depressed, 1=pressed)
*/
uint8_t DriverCursorstickGet(void);


/**
 * \brief Get debounced cursor stick event from FIFO
 * \param BlockTime: time to block before returning (in ms)
 * \return B4-->B0: U L D R C (0=depressed, 1=pressed)
*/
uint8_t DriverCursorStickGetFifo(TickType_t BlockTime);
