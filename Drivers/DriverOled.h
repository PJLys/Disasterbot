#ifndef DRIVER_OLED_H
#define DRIVER_OLED_H

/**
 * Hardware: OLED 128x64 module (SSD1306)
 * Software: The driver supports basic drawing and text commands. An initialized DriverTwiMaster module is required.
 * \file oled.h
 * \brief OLED 128x64 module driver
*/

#include "DriverTwiMaster.h"
#include "avr/pgmspace.h"
#include <string.h>
#include <stdlib.h>
#include <util/delay.h>
#include <stddef.h>


/**
 * \brief Initialize OLED module
 * \param Orientation : 0:Landscape, 1: Portrait, 2:Landscape inverted
*/
void DriverOLEDInit(uint8_t Orientation);

/**
 * \brief Write screen buffer to module
*/
void DriverOLEDUpdate();

/**
 * \brief Print large font text (16 pixels height)
 * \param row :Text row, 0=first row
 * \param text :text string (null terminated)
*/
void DriverOLEDPrintText(uint8_t row, char *text);

/**
 * \brief Print one character in small font (8 pixels height)
 * \param x :X pixel position
 * \param y :Y pixel position
 * \param ch :ASCII code
 * \param scr :flag, if trues, wraps around text
*/
void DriverOLEDPrintSmChar(unsigned char x, unsigned char y, unsigned char ch, uint8_t scr);

/**
 * \brief Print small font text (8 pixels height)
 * \param row :Text row, 0=first row
 * \param dataPtr :text string (null terminated)
 * \param scr :flag, if trues, wraps around text
*/
void DriverOLEDPrintSmText(unsigned char row, char *dataPtr, uint8_t scr);

/**
 * \brief Clear screen buffer
*/
void DriverOLEDClearScreen();

/**
 * \brief Set inverted color screen mode
*/
void DriverOLEDInvertScreen();

/**
 * \brief Set normal color screen mode
*/
void DriverOLEDNormalScreen();

/**
 * \brief Draw one pixel
 * \param x :X coordinate
 * \param y :Y coordinate
*/
void DriverOLEDDrawPixel(unsigned char x, unsigned char y);

/**
 * \brief Clear one pixel
 * \param x :X coordinate
 * \param y :Y coordinate
*/
void DriverOLEDClearPixel(unsigned char x, unsigned char y);

/**
 * \brief Draw a line between two coordinates
 * \param x1 :X coordinate for point 1
 * \param y1 :Y coordinate for point 1
 * \param x2 :X coordinate for point 2
 * \param y2 :Y coordinate for point 2
*/
void DriverOLEDDrawLine (int x1, int y1, int x2, int y2);

/**
 * \brief Draw a rectangle (only contours)
 * \param x1 :X coordinate for point 1
 * \param y1 :Y coordinate for point 1
 * \param x2 :X coordinate for point 2
 * \param y2 :Y coordinate for point 2
*/
void DriverOLEDDrawRectangle (int x1, int y1, int x2, int y2);

/**
 * \brief Draw a filled rectangle
 * \param x1 :X coordinate for point 1
 * \param y1 :Y coordinate for point 1
 * \param x2 :X coordinate for point 2
 * \param y2 :Y coordinate for point 2
*/
void DriverOLEDDrawSolidRectangle (int x1, int y1, int x2, int y2);

/**
 * \brief Draw an ellipse
 * \param CX :center X coordinate
 * \param CY :center Y coordinate
 * \param XRadius :X radius
 * \param YRadius :Y radius
*/
void DriverOLEDDrawEllipse (int CX, int CY, int XRadius, int YRadius);

/**
 * \brief Draw a circle
 * \param x :center X coordinate
 * \param y :center Y coordinate
 * \param r :circle radius
*/
void DriverOLEDDrawCircle (int x, int y, int r);

/**
 * \brief Draw a triangle (contour)
 * \param x1 :X coordinate for point 1
 * \param y1 :Y coordinate for point 1
 * \param x2 :X coordinate for point 2
 * \param y2 :Y coordinate for point 2
 * \param x3 :X coordinate for point 3
 * \param y3 :Y coordinate for point 3
*/
void DriverOLEDDrawTriangle (int x1, int y1, int x2, int y2, int x3, int y3);

/**
 * \brief Set display in sleep mode (energy saving mode)
*/
void DriverOLEDSleep();

/**
 * \brief Wake up display (turn off energy saving mode)
*/
void DriverOLEDWake();

#endif