/*
 * Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
 *
 * This file is part of Sub-IoT.
 * See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


// This examples does not push sensor data to gateway(s) continuously, but instead writes the sensor value to a local file,
// which can then be fetched on request.
// Temperature data is used as a sensor value, when a HTS221 is available, otherwise value 0 is used.
// Contrary to the sensor_push example we are now defining an Access Profile which has a periodic scan automation enabled.
// The sensor will sniff the channel every second for background adhoc synchronization frames, to be able to receive requests from other nodes.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hwleds.h"
#include "hwsystem.h"
#include "hwlcd.h"
#include "hwuart.h"


#include "scheduler.h"
#include "timer.h"
#include "debug.h"
#include "d7ap_fs.h"
#include "log.h"
#include "compress.h"

#include "d7ap.h"
#include "alp_layer.h"
#include "dae.h"
#include "platform_defs.h"
#include "modules_defs.h"
#include "platform.h"

#include <unistd.h>

#include <time.h>  // for srand and rand functions
#include <stdint.h>

#ifdef USE_HTS221
  #include "HTS221_Driver.h"
  #include "hwi2c.h"
#endif

#ifdef MODULE_LORAWAN
  #error "sensor_pull app is not compatible with LoRaWAN, so disable MODULE_LORAWAN in cmake"
#endif

#if !defined(USE_SX127X) && !defined(USE_NETDEV_DRIVER)
  #error "background frames are only supported by the sx127x driver for now"
#endif


#define SENSOR_FILE_SIZE 4
#define START_DELIMITER 0x73
#define END_DELIMITER 0x65

#define TEMPERATURE_FILE_ID 0x3C
#define LIGHT_FILE_ID 0x3D
#define HUMIDITY_FILE_ID 0x3E
#define RADIO_FILE_ID 0x3F //radioactivity sensor
#define DUST_FILE_ID 0x41
#define TEST_FILE_ID 0x43

#define SENSOR_INTERVAL_SEC	TIMER_TICKS_PER_SEC 

#define UART_PORT_IDX 0 // Adjust based on your hardware setup
#define UART_BAUDRATE 115200 // Adjust based on your requirements
#define UART_PINS 0 // Adjust based on your hardware setup
#define DATA_BUFFER_SIZE 7 // Adjust based on expected data size 2*char + data = 24 bit, + sensortype is 32 bit?

uint8_t data_buffer[DATA_BUFFER_SIZE];
uint16_t data_buffer_index = 0;
uint8_t transmissionStarted = 0;

typedef enum {
    DATA_REV = 0x00,
    DATA_REQ = 0x01,
    TEMPERATURE_SENSOR = 0x02,
    HUMIDITY_SENSOR = 0x03,
    LIGHT_SENSOR = 0x04,
    RADIO_SENSOR = 0x05,
    //DUST_SENSOR = 0x06,
    DUST_SENSOR = 0x07

    // Add other sensor types as needed
} SensorType;

typedef struct {
    uint8_t startDelimiter;
    SensorType sensorType;
    //uint32_t timestamp; //though perhaps important
    uint32_t sensorData;
    //uint8_t checksum;
    uint8_t endDelimiter;
} CustomMessage;

typedef struct {
    uint8_t startDelimiter;
    SensorType sensorType;
    uint8_t endDelimiter;
} Request;


uart_handle_t* uarthandler;

void ask_data(){
    //send request
    uint8_t payload[3];
    payload[0] = START_DELIMITER;
    payload[1] = DATA_REQ;
    payload[2] = END_DELIMITER;
    uart_send_bytes(uarthandler,payload,sizeof(payload));
    //non blocking delay ot allow reception
    int i = 30000;//no clue how much delay this is but it's delay..
    while(i!=0){
        i--;
    }
}

void startTask(){
    //wakeup_STM();
    timer_post_task_delay(&ask_data, SENSOR_INTERVAL_SEC);
    timer_post_task_delay(&startTask, SENSOR_INTERVAL_SEC*10);
}

void process_received_data(uint8_t* buffer, uint8_t size){
    //uart_send_string(uarthandler,"received");
    CustomMessage message;
    size_t offset = 0;
    // Manually copy data from buffer to struct fields
    message.startDelimiter = buffer[offset++];
    message.sensorType = buffer[offset++];
    //message.timestamp = (uint32_t)buffer[offset] | ((uint32_t)buffer[offset + 1] << 8) | ((uint32_t)buffer[offset + 2] << 16) | ((uint32_t)buffer[offset + 3] << 24);
    //offset += 4;
    message.sensorData = (uint32_t)buffer[offset] | ((uint32_t)buffer[offset + 1] << 8) | ((uint32_t)buffer[offset + 2] << 16) | ((uint32_t)buffer[offset +3] << 24);
    offset += 4;
    //message.checksum = buffer[offset++];
    message.endDelimiter = buffer[offset];

    static uint32_t data = 0;
    static uint32_t temperature = 40;


    //check delimiters
    if (message.sensorType == TEMPERATURE_SENSOR){ //temperature
        data = __builtin_bswap32(message.sensorData); // need to store in big endian in fs
        int rc = d7ap_fs_write_file(TEMPERATURE_FILE_ID, 0, (uint8_t*)&data, SENSOR_FILE_SIZE, ROOT_AUTH);
        assert(rc == 0);
        data = __builtin_bswap32(data); // revert to make sure we're working with the right value(?)
        //uart_send_string("temperature processed \n\r");
    } else if (message.sensorType == HUMIDITY_SENSOR){
        data = __builtin_bswap32(message.sensorData); // need to store in big endian in fs
        int rc = d7ap_fs_write_file(HUMIDITY_FILE_ID, 0, (uint8_t*)&data, SENSOR_FILE_SIZE, ROOT_AUTH);
        assert(rc == 0);
        data = __builtin_bswap32(data); // revert to make sure we're working with the right value(?)
    }else if (message.sensorType == LIGHT_SENSOR){
        data = __builtin_bswap32(message.sensorData); // need to store in big endian in fs
        int rc = d7ap_fs_write_file(LIGHT_FILE_ID, 0, (uint8_t*)&data, SENSOR_FILE_SIZE, ROOT_AUTH);
        assert(rc == 0);
        data = __builtin_bswap32(data); // revert to make sure we're working with the right value(?)
    }else if (message.sensorType == RADIO_SENSOR){
        data = __builtin_bswap32(message.sensorData); // need to store in big endian in fs
        int rc = d7ap_fs_write_file(RADIO_FILE_ID, 0, (uint8_t*)&data, SENSOR_FILE_SIZE, ROOT_AUTH);
        assert(rc == 0);
        data = __builtin_bswap32(data); // revert to make sure we're working with the right value(?)
    }else if (message.sensorType == DUST_SENSOR){
        data = __builtin_bswap32(message.sensorData); // need to store in big endian in fs
        int rc = d7ap_fs_write_file(DUST_FILE_ID, 0, (uint8_t*)&data, SENSOR_FILE_SIZE, ROOT_AUTH);
        assert(rc == 0);
        data = __builtin_bswap32(data); // revert to make sure we're working with the right value(?)
    }else{
        data = __builtin_bswap32(temperature); // need to store in big endian in fs
        int rc = d7ap_fs_write_file(TEMPERATURE_FILE_ID, 0, (uint8_t*)&data, SENSOR_FILE_SIZE, ROOT_AUTH);
        assert(rc == 0);
        data = __builtin_bswap32(temperature); // revert to make sure we're working with the right value(?)
    }
   // push_data(buffer,size, message);
}

void uart_rx_callback(uart_handle_t* uart_handler, uint8_t byte){
    //accumulate bytes in the buffer
    if (byte == START_DELIMITER) transmissionStarted=1;
    if(transmissionStarted){
        if(data_buffer_index < DATA_BUFFER_SIZE){
            data_buffer[data_buffer_index++] = byte;
            if(byte == END_DELIMITER){
                transmissionStarted = 0;
                uint8_t tempIndex = data_buffer_index;
                data_buffer_index=0;
                process_received_data(data_buffer,tempIndex);
            }
        }
        
    }  
}

void execute_sensor_measurement()
{
  static int32_t temperature = 27; // in decicelsius. When there is no sensor, we transmit a rising number



  temperature = __builtin_bswap32(temperature); // need to store in big endian in fs
  int rc = d7ap_fs_write_file(TEMPERATURE_FILE_ID, 0, (uint8_t*)&temperature, SENSOR_FILE_SIZE, ROOT_AUTH);
  assert(rc == 0);
  temperature = __builtin_bswap32(temperature); // revert to make sure we're working with the right value

 // timer_post_task_delay(&execute_sensor_measurement, SENSOR_INTERVAL_SEC);
}

void init_user_files()
{
  // file 0x40: contains our sensor data
  d7ap_fs_file_header_t sensor_file_header = (d7ap_fs_file_header_t) {
      .file_permissions = (file_permission_t) { .user_read = true, .guest_read = true },
      .file_properties.action_protocol_enabled = 0,
      .file_properties.storage_class = FS_STORAGE_PERMANENT,
      .length = SENSOR_FILE_SIZE,
      .allocated_length = SENSOR_FILE_SIZE,
  };

  d7ap_fs_init_file(TEMPERATURE_FILE_ID, &sensor_file_header, NULL);
  d7ap_fs_init_file(HUMIDITY_FILE_ID, &sensor_file_header, NULL);
  d7ap_fs_init_file(LIGHT_FILE_ID, &sensor_file_header, NULL);
  d7ap_fs_init_file(RADIO_FILE_ID, &sensor_file_header, NULL);
  d7ap_fs_init_file(DUST_FILE_ID, &sensor_file_header, NULL);
  d7ap_fs_init_file(TEST_FILE_ID, &sensor_file_header, NULL);
  
  timer_post_task_delay(&startTask, SENSOR_INTERVAL_SEC);
}

static alp_init_args_t alp_init_args;

void bootstrap()
{
    log_print_string("Device booted\n");
    
    alp_layer_init(NULL, false);
    
    d7ap_fs_write_dll_conf_active_access_class(0x11); // use scanning AC, visible in d7ap_fs_data.c
    init_user_files();

    uarthandler = uart_init(UART_PORT_IDX, UART_BAUDRATE, UART_PINS);
    if(uarthandler == NULL) {
        // Initialization failed, handle the error
    }
    
    uart_set_rx_interrupt_callback(uarthandler, uart_rx_callback);
    uart_rx_interrupt_enable(uarthandler);

    //sched_register_task(&execute_sensor_measurement);
    //sched_post_task(&execute_sensor_measurement);
    sched_register_task(&startTask);
    sched_register_task(&ask_data);
    //uart_send_string(uarthandler, "Test");
}
