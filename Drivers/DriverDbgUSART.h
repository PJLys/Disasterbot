
/**
 * \brief Polled USART string print function. Can be used before FreeRTOS init.
*/

#include "hwconfig.h"

#ifndef DRIVER_DBG_USART_H
#define DRIVER_DBG_USART_H

/**
 * \brief Print text to 'USART' defined in hwconfig.h in polled mode 
 * \param Text : NULL terminated string that will be sent over the USART TX line
*/
void DbgPrint(char *Text);

/**
 * \brief Print text to 'USART' defined in hwconfig.h in polled mode with a limited number of characters.
 * \param Text : NULL terminated string that will be sent over the USART TX line
 * \param n : max number of characters to print. If 0, unlimited.
*/
void DbgPrintn(char *Text,int n);

/**
 * \brief Print unsigned integer to 'USART' defined in hwconfig.h in polled mode. 
 * \param Data : unsigned integer that will be printed in ASCII over the USART
*/
void DbgPrintInt(uint16_t Data);

#endif