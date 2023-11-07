/**
 * \brief Initialize USART module as defined in hwconfig.h, using interrupts, hardcoded to 19200bps. Binds USART to stdio functions.
*/


#ifndef DRIVER_USART_H
#define DRIVER_USART_H

/**
 * \brief Initialize USART defined in hwconfig.h, bind to stdin, stdout
*/
void DriverUSARTInit(void); 

#endif