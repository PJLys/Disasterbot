/*
 * dash7.c
 *
 *  Created on: Nov 30, 2023
 *      Author: jeoffrey.canters
 */

#include <dash7.h>
#include <string.h>

volatile uint8_t uart_rx_buffer[BUFFER_SIZE];
static volatile uint8_t uart_rx_buffer_i = 0;
volatile bool uart_rx_done_flag;

uint8_t create_payload(message msg, uint8_t *payload){
    uint8_t payload_i = 0;

    payload[payload_i++] = msg.msg_start_char;
    memcpy(payload+payload_i, &msg.msg_type, sizeof(msg.msg_type));
    payload_i += sizeof(msg.msg_type);

    if (msg.msg_type == RESPONSE_TEMPERATURE){
        memcpy(payload+payload_i, &msg.msg_data.temperature, sizeof(msg.msg_data.temperature));
        payload_i += sizeof(msg.msg_data.temperature);
    }
    else if (msg.msg_type == RESPONSE_HUMIDITY){
        memcpy(payload+payload_i, &msg.msg_data.humidity, sizeof(msg.msg_data.humidity));
        payload_i += sizeof(msg.msg_data.humidity);
    }
    else if (msg.msg_type == RESPONSE_LIGHT){
        memcpy(payload+payload_i, msg.msg_data.light, sizeof(msg.msg_data.light));
        payload_i += sizeof(msg.msg_data.light);
    }
    else if (msg.msg_type == RESPONSE_RADIATION){
    	memcpy(payload+payload_i, &msg.msg_data.radiation, sizeof(msg.msg_data.radiation));
    	payload_i += sizeof(msg.msg_data.radiation);
    }
    else if (msg.msg_type == RESPONSE_DUST) {
    	memcpy(payload+payload_i, &msg.msg_data.dust, sizeof(msg.msg_data.dust));
    	payload_i += sizeof(msg.msg_data.dust);
    }

    payload[payload_i++] = msg.msg_end_char;

    return payload_i;
}

void uart_rx_buffer_handler(uint8_t input_byte){
    uart_rx_done_flag = false;

    if (input_byte == 's')
    {
      uart_rx_buffer_i = 0;
    }
    else if (input_byte == 'e')
    {
      uart_rx_done_flag = true;
    }

    uart_rx_buffer[uart_rx_buffer_i] = input_byte;

    if (uart_rx_buffer_i < (BUFFER_SIZE - 1))
    {
      uart_rx_buffer_i++;
    }
}

void uart_rx_buffer_clear(){
	for (int i=0; i<(BUFFER_SIZE); i++) {
		uart_rx_buffer[i] = 0xF;
	}
}

