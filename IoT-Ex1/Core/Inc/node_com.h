/*
 * node_com.h
 *
 *  Created on: Nov 30, 2023
 *      Author: jeoffrey.canters
 */

#ifndef INC_NODE_COM_H_
#define INC_NODE_COM_H_

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

union msg_data_u {
    float temperature;
    float humidity;
    float radiation;
    float dust;
    uint8_t light[4];
};

typedef struct {
    const char msg_start_char;
    enum msg_type_e msg_type;
    union msg_data_u msg_data;
    const char msg_end_char;
} message;

extern volatile uint8_t uart_rx_buffer[BUFFER_SIZE];
extern volatile bool uart_rx_done_flag;

uint8_t create_payload(message msg, uint8_t* payload);
void uart_rx_buffer_handler(uint8_t input_byte);


#endif /* INC_NODE_COM_H_ */
