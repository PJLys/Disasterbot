/**
 * Linebot LED driver
 * \file DriverLed.h
 * \brief Linebot LED driver
*/

#include "hwconfig.h"

/**
 * \brief Initialize LED driver
*/
void DriverLedInit(void);


/**
 * \brief Set LED status
 * \param LedData: B0 --> B3: LED1 --> LED4. 0=OFF, 1=ON
*/
void DriverLedWrite(uint8_t LedData);

/**
 * \brief Turn designated LEDs on
 * \param LedData: B0 --> B3: LED1 --> LED4. 0=unchanged, 1=ON
*/
void DriverLedSet(uint8_t LedData);

/**
 * \brief Turn designated LEDs off
 * \param LedData: B0 --> B3: LED1 --> LED4. 0=unchanged, 1=OFF
*/
void DriverLedClear(uint8_t LedData);

/**
 * \brief Toggle designated LEDs 
 * \param LedData: B0 --> B3: LED1 --> LED4. 0=unchanged, 1=toggle
*/
void DriverLedToggle(uint8_t LedData);