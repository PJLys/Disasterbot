/*
 * dash7.h
 *
 *  Created on: Nov 30, 2023
 *      Author: jeoffrey.canters
 */

#ifndef INC_DASH7_H_
#define INC_DASH7_H_

#include <stdbool.h>
#include <stdint.h>

#define MSG_START_CHARACTER 's'
#define MSG_END_CHARACTER 'e'

#define BUFFER_SIZE 30

enum msg_type_e {
    REQUEST_DATA,
    RESPONSE_TEMPERATURE,
    RESPONSE_HUMIDITY,
    RESPONSE_LIGHT,
	RESPONSE_RADIATION,
	RESPONSE_DUST,
};

extern volatile uint8_t uart_rx_buffer[BUFFER_SIZE];
extern volatile bool uart_rx_done_flag;

uint8_t* create_payload_f(uint8_t msg_type, float data);
uint8_t* create_payload(uint8_t msg_type, uint8_t* data);
void uart_rx_buffer_handler(uint8_t input_byte);
void uart_rx_buffer_clear();
uint8_t* floatToUint8Vector(float val);


#endif /* INC_DASH7_H_ */
