#ifndef HWCONFIG_H
#define HWCONFIG_H

#include <avr/io.h>

#define F_CPU 32000000L							//Peripheral frequency
#define F_XTAL 16000000L						//Crystal frequency
#define CLK_PER_MS (F_CPU/1000)

//Cursor stick settings
#define CURSOR_FIFO_LENGTH 10
#define CURSOR_MIN_INTERVAL 200*CLK_PER_MS		//Minimum interval between actuations (in CPU ticks)

//Motor and drive settings
#define ROBOT_MG_6_120							//Define if robot with Olimex MG_6_120 motors is used
//#define ROBOT_NXT_MOTOR						//Define if robot with NXT motors is used


#ifdef ROBOT_MG_6_120
	#define ENCODER_RIGHT_INVERT 0
	#define ENCODER_LEFT_INVERT 0
	#define MOTOR_RIGHT_INVERT 1
	#define MOTOR_LEFT_INVERT 0				
	#define WHEEL_CIRC 207.3					//Wheel circumference (mm)
	#define WHEEL_DISTANCE 111					//Distance between the wheels (mm)
	#define DEG_PER_CNT 3						//Number of degrees per encoder count
	//#define ENCODER_FILTER_ENABLE				//Define if encoder inputs should be filtered in SW
	#define ENCODER_FILTER_TIME	32000			//Minimum number of CPU ticks between encoder line edges
	#define MOTPOS_KP 300						//Motor angular position controller: KP
	#define MOTPOS_FF_OFFSET 1500				//Motor drive signal: output offset (signal level when motor shaft starts to rotate)
#endif

#ifdef ROBOT_NXT_MOTOR
	#define ENCODER_RIGHT_INVERT 0
	#define ENCODER_LEFT_INVERT 0
	#define MOTOR_RIGHT_INVERT 1
	#define MOTOR_LEFT_INVERT 1
	#define WHEEL_DISTANCE 111					//Distance between the wheels (mm)
	#define WHEEL_CIRC 260.0					//Wheel circumference (mm)
	#define DEG_PER_CNT 0.5						//Number of degrees per encoder count
	//#define ENCODER_FILTER_ENABLE				//Define if encoder inputs should be filtered in SW
	#define ENCODER_FILTER_TIME	32000			//Minimum number of CPU ticks between encoder line edges
	#define MOTPOS_KP 110						//Motor angular position controller: KP
	#define MOTPOS_FF_OFFSET 1500				//Motor drive signal: output offset (signal level when motor shaft starts to rotate)
#endif

	#define LINEFOLLOW_SPEED_KP 0.1			    //Speed mode line follow KP
	#define LINEFOLLOW_THRESHOLD 2500			//End of segment luminosity threshold
	#define LINEFOLLOW_THRESHOLD_CNT 20			//Number of ticks needed above threshold

//DriverUSART definitions
//Define either UART_RPI or UART_USB
//#define UART_RPI
#define UART_USB

#define UART_QUEUE_LENGTH 128
#ifdef UART_RPI
	#define USART USARTC0						//USART used by the driver
	#define USART_PORT PORTC					//Corresponding USART port
	#define USART_RXC_vect USARTC0_RXC_vect		//Corresponding USART RXC ISR vector
	#define USART_TXC_vect USARTC0_TXC_vect		//Corresponding USART TXC ISR vector
#endif
#ifdef UART_USB
	#define USART USARTE0						//USART used by the driver
	#define USART_PORT PORTE					//Corresponding USART port
	#define USART_RXC_vect USARTE0_RXC_vect		//Corresponding USART RXC ISR vector
	#define USART_TXC_vect USARTE0_TXC_vect		//Corresponding USART TXC ISR vector
#endif

//DriverTWIMaster definitions
#define TWIM_BUS TWIE							//TWI module used by the driver
#define TWIM_PORT PORTE							//Corresponding TWI port
#define TWIM_BUS_vect TWIE_TWIM_vect			//Corresponding TWI ISR vector 
#define TWIM_WRITE_BUFFER_SIZE 32				//TWI write buffer size
#define TWIM_READ_BUFFER_SIZE  32				//TWI read buffer size
#define TWIM_INTLEVEL 1							//TWI interrupt priority
#define TWIM_BAUDRATE 400000					//TWI Bitrate (bps)

#endif
