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

uint8_t* create_payload_f(uint8_t msg_type, float data) {
    // Assuming data is a 4-byte uint8_t array
	uint8_t* data_array=floatToUint8Vector(data);
	return create_payload(msg_type, data_array);
}

uint8_t* create_payload(uint8_t msg_type, uint8_t* data_array){
    // Allocate memory for the payload (6 bytes)
    uint8_t* payload = malloc(7);

    if (payload != NULL) {
        // Place 'e' in the first byte
        payload[0] = MSG_START_CHARACTER;
        payload[1] = msg_type;

        // Copy the 4-byte data into bytes 1-4
        for (int i = 0; i < 4; ++i) {
            payload[i + 2] = data_array[i];
        }

        // Place 's' in the last byte
        payload[6] = MSG_END_CHARACTER;
    }

    return payload;
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
		uart_rx_buffer[i] = 0x0;
	}
}

uint8_t* floatToUint8Vector(float val) {
	char data[sizeof(float)];

	memcpy(data, &val, sizeof(val));

	return data;
}

