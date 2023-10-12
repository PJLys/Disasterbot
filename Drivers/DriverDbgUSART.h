/**
 * \brief Polled USART string print function
*/


#ifndef DRIVER_DBG_USART_H
#define DRIVER_DBG_USART_H

/**
 * \brief Print text to 'USART' defined in hwconfig.h. First reconfigures USART to polled mode. Before returning, the function restores the USART settings
 * \param Text : NULL terminated string that will be sent over the USART TX line
*/
void DbgPrint(char *Text);

#endif