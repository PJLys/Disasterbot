#ifndef RGB_TASK_H
#define RGB_TASK_H

/**
 * Implements control of the 4 RGB LEDs on the linebot. Enables a number of effects as defined in RGBTask.h. 
 * \file RGBTask.h
 * \brief linebot RGB LED effect task
*/


#include <stdint.h>

#define RGB_NONE 0
#define RGB_HEADLIGHT_LOW 1
#define RGB_HEADLIGHT_HIGH 2
#define RGB_BLINK_LEFT 4
#define RGB_BLINK_RIGHT 8
#define RGB_BRAKE 16


/**
 * \brief Initializes RGB LED control task
*/
void InitRGBTask(void);

/**
 * \brief Set current RGB LED effect. 
 * \param Effect: RGB LED effect. Effects are bit position based and can be combined (example RGB_BLINK_LEFT | RGB_BLINK_RIGHT)
*/
void SetRGB(uint8_t Effect);

#endif