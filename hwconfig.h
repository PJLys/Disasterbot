#ifndef HWCONFIG_H
#define HWCONFIG_H

#include <avr/io.h>

#define F_CPU 32000000L						//Peripheral frequency
#define F_XTAL 16000000L					//Crystal frequency

//DriverUSART definitions
#define USART USARTE0						//USART used by the driver
#define USART_PORT PORTE					//Corresponding USART port
#define USART_RXC_vect USARTE0_RXC_vect		//Corresponding USART RXC ISR vector
#define USART_TXC_vect USARTE0_TXC_vect		//Corresponding USART TXC ISR vector

//DriverTWIMaster definitions
#define TWIM_BUS TWIE						//TWI module used by the driver
#define TWIM_PORT PORTE						//Corresponding TWI port
#define TWIM_BUS_vect TWIE_TWIM_vect		//Corresponding TWI ISR vector 
#define TWIM_WRITE_BUFFER_SIZE 32			//TWI write buffer size
#define TWIM_READ_BUFFER_SIZE  32			//TWI read buffer size
#define TWIM_INTLEVEL 1						//TWI interrupt priority
#define TWIM_BAUDRATE 400000				//TWI Bitrate (bps)

#endif
