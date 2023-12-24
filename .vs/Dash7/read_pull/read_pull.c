
#include "hwuart.h"
#include "scheduler.h"
//#include "read_push.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hwleds.h"
#include "hwsystem.h"
#include "hwlcd.h"

#include "scheduler.h"
#include "timer.h"
#include "debug.h"
#include "d7ap_fs.h"
#include "log.h"

#include "d7ap.h"
#include "alp_layer.h"
#include "dae.h"

#include "platform.h"

#ifdef MODULE_LORAWAN
  #error "sensor_pull app is not compatible with LoRaWAN, so disable MODULE_LORAWAN in cmake"
#endif

#if !defined(USE_SX127X) && !defined(USE_NETDEV_DRIVER)
  #error "background frames are only supported by the sx127x driver for now"
#endif

#ifndef READ_PULL_H
#define READ_PULL_H

#include <stdint.h>

#define SENSOR_FILE_SIZE 4
#define START_DELIMITER 0x73
#define END_DELIMITER 0x65

#define TEMPERATURE_FILE_ID 0x3C
#define LIGHT_FILE_ID 0x3D
#define HUMIDITY_FILE_ID 0x3E
#define RADIO_FILE_ID 0x3F //radioactivity sensor
#define DUST_FILE_ID 0x45
#define TEST_FILE_ID 0x43

#include <time.h>  // for srand and rand functions



typedef enum {
    DATA_REV = 0x00,
    DATA_REQ = 0x01,
    TEMPERATURE_SENSOR = 0x02,
    HUMIDITY_SENSOR = 0x03,
    LIGHT_SENSOR = 0x04,
    RADIO_SENSOR = 0x05,
    DUST_SENSOR = 0x06

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

#endif // CUSTOM_PROTOCOL_H

#define UART_PORT_IDX 0 // Adjust based on your hardware setup
#define UART_BAUDRATE 115200 // Adjust based on your requirements
#define UART_PINS 0 // Adjust based on your hardware setup
#define DATA_BUFFER_SIZE 7 // Adjust based on expected data size 2*char + data = 24 bit, + sensortype is 32 bit?

uint8_t data_buffer[DATA_BUFFER_SIZE];
uint16_t data_buffer_index = 0;
uint8_t transmissionStarted = 0;

#define SENSOR_INTERVAL_SEC	TIMER_TICKS_PER_SEC

uart_handle_t* uarthandler;

// Define the D7 interface configuration used for sending the ALP command on
static alp_interface_config_d7ap_t itf_config = (alp_interface_config_d7ap_t){
  .itf_id = ALP_ITF_ID_D7ASP,
  .d7ap_session_config = {
    .qos = {
        .qos_resp_mode = SESSION_RESP_MODE_PREFERRED,
        .qos_retry_mode = SESSION_RETRY_MODE_NO
    },
    .dormant_timeout = 0,
    .addressee = {
        .ctrl = {
            .nls_method = AES_NONE,
            .id_type = ID_TYPE_NOID,
        },
        .access_class = 0x01,
        .id = { 0 }
    }
  }
};

void wakeup_STM(){

    //uart_send_string(uarthandler, "Sensors");//sens "Sensors" over uart so STM knows it has to wake up
}

void ask_data(){
    //send request
    uint8_t payload[3];
    payload[0] = START_DELIMITER;
    payload[1] = DATA_REQ;
    payload[2] = END_DELIMITER;
    uart_send_bytes(uarthandler,payload,sizeof(payload));
}

void startTask(){
    //wakeup_STM();
    timer_post_task_delay(&ask_data, SENSOR_INTERVAL_SEC);
    timer_post_task_delay(&startTask, SENSOR_INTERVAL_SEC*10);
}

/*void push_data(uint8_t* buffer, uint16_t size, CustomMessage message){
    uint8_t payload[4];
    payload[0] = START_DELIMITER;
    payload[1] = (message.sensorData >> 8) & 0xFF;
    payload[2] = message.sensorData & 0xFF;
    payload[3] = END_DELIMITER;

        // alloc command. This will be freed when the command completes
    alp_command_t* command = alp_layer_command_alloc(false, false);
    
    // forward to the D7 interface
    alp_append_forward_action(command, (alp_interface_config_t*)&itf_config, d7ap_session_config_length(&itf_config.d7ap_session_config));

    // add the return file data action
    alp_append_return_file_data_action(command, TEMPERATURE_FILE_ID, 0, sizeof(payload), payload);

    // and finally execute this
    alp_layer_process(command);
}*/

void process_received_data(uint8_t* buffer, uint16_t size){
    CustomMessage message;
    size_t offset = 0;
    // Manually copy data from buffer to struct fields
    message.startDelimiter = buffer[offset++];
    message.sensorType = buffer[offset++];
    //message.timestamp = (uint32_t)buffer[offset] | ((uint32_t)buffer[offset + 1] << 8) | ((uint32_t)buffer[offset + 2] << 16) | ((uint32_t)buffer[offset + 3] << 24);
    //offset += 4;
    message.sensorData = (uint32_t)buffer[offset] | ((uint32_t)buffer[offset + 1] << 8) | ((uint32_t)buffer[offset + 2] << 16) | ((uint32_t)buffer[offset + 3] << 24);;
    offset += 4;
    //message.checksum = buffer[offset++];
    message.endDelimiter = buffer[offset];

    static uint32_t data = 0;
    static uint32_t temperature = 2383946474;
    //check delimiters
    if (message.sensorType == TEMPERATURE_SENSOR){ //temperature
        data = __builtin_bswap16(message.sensorData); // need to store in big endian in fs
        int rc = d7ap_fs_write_file(TEMPERATURE_FILE_ID, 0, (uint8_t*)&data, SENSOR_FILE_SIZE, ROOT_AUTH);
        assert(rc == 0);
        data = __builtin_bswap16(data); // revert to make sure we're working with the right value(?)
        //uart_send_string("temperature processed \n\r");
    } else if (message.sensorType == HUMIDITY_SENSOR){
        data = __builtin_bswap16(message.sensorData); // need to store in big endian in fs
        int rc = d7ap_fs_write_file(HUMIDITY_FILE_ID, 0, (uint8_t*)&data, SENSOR_FILE_SIZE, ROOT_AUTH);
        assert(rc == 0);
        data = __builtin_bswap16(data); // revert to make sure we're working with the right value(?)
    }else if (message.sensorType == LIGHT_SENSOR){
        data = __builtin_bswap16(message.sensorData); // need to store in big endian in fs
        int rc = d7ap_fs_write_file(LIGHT_FILE_ID, 0, (uint8_t*)&data, SENSOR_FILE_SIZE, ROOT_AUTH);
        assert(rc == 0);
        data = __builtin_bswap16(data); // revert to make sure we're working with the right value(?)
    }else if (message.sensorType == RADIO_SENSOR){
        data = __builtin_bswap16(message.sensorData); // need to store in big endian in fs
        int rc = d7ap_fs_write_file(RADIO_FILE_ID, 0, (uint8_t*)&data, SENSOR_FILE_SIZE, ROOT_AUTH);
        assert(rc == 0);
        data = __builtin_bswap16(data); // revert to make sure we're working with the right value(?)
    }else if (message.sensorType == DUST_SENSOR){
        data = __builtin_bswap16(message.sensorData); // need to store in big endian in fs
        int rc = d7ap_fs_write_file(DUST_FILE_ID, 0, (uint8_t*)&data, SENSOR_FILE_SIZE, ROOT_AUTH);
        assert(rc == 0);
        data = __builtin_bswap16(data); // revert to make sure we're working with the right value(?)
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
        }//room for buffer overflow handler
        
    }
}

void write_random_temperature(){
    static uint32_t data = 2383946474;
    data = __builtin_bswap32(data); // need to store in big endian in fs
    int rc = d7ap_fs_write_file(TEMPERATURE_FILE_ID, 0, (uint8_t*)&data, SENSOR_FILE_SIZE, ROOT_AUTH);
    assert(rc == 0);
    data = __builtin_bswap32(data); // revert to make sure we're working with the right value(?)

}

void init_user_files(){
    //temperature
    write_random_temperature();
    d7ap_fs_file_header_t default_header = (d7ap_fs_file_header_t) {
      .file_permissions = (file_permission_t) { .user_read = true, .guest_read = true },
      .file_properties.action_protocol_enabled = 0,
      .file_properties.storage_class = FS_STORAGE_PERMANENT,
      .length = SENSOR_FILE_SIZE,
      .allocated_length = SENSOR_FILE_SIZE,
    };
    d7ap_fs_init_file(TEMPERATURE_FILE_ID, &default_header, NULL);
    d7ap_fs_init_file(HUMIDITY_FILE_ID, &default_header, NULL);
    d7ap_fs_init_file(LIGHT_FILE_ID, &default_header, NULL);
    d7ap_fs_init_file(RADIO_FILE_ID, &default_header, NULL);
    d7ap_fs_init_file(DUST_FILE_ID, &default_header, NULL);
    d7ap_fs_init_file(TEST_FILE_ID, &default_header, NULL);
    
    timer_post_task_delay(&startTask, SENSOR_INTERVAL_SEC);
}

void on_alp_command_completed_cb(uint8_t tag_id, bool success)
{
    if(success)
      log_print_string("Command (%i) completed successfully", tag_id);
    else
      log_print_string("Command failed, no ack received");
}

void on_alp_command_result_cb(alp_command_t *alp_command, alp_interface_status_t* origin_itf_status)
{
  if(origin_itf_status && (origin_itf_status->itf_id == ALP_ITF_ID_D7ASP) && (origin_itf_status->len > 0)) {
      d7ap_session_result_t* d7_result = ((d7ap_session_result_t*)origin_itf_status->itf_status);
      log_print_string("recv response @ %i dB link budget from:", d7_result->rx_level);
      log_print_data(d7_result->addressee.id, d7ap_addressee_id_length(d7_result->addressee.ctrl.id_type));
  }
  log_print_string("response payload:");
  log_print_data(alp_command->alp_command, fifo_get_size(&alp_command->alp_command_fifo));
  fifo_skip(&alp_command->alp_command_fifo, fifo_get_size(&alp_command->alp_command_fifo));
  
  //timer_post_task_delay(&wakeup,amountoftimeinms)
}



static alp_init_args_t alp_init_args;

void bootstrap(){
    d7ap_fs_init();
    d7ap_init();
    log_print_string("Device booted\n");
    alp_init_args.alp_command_completed_cb = &on_alp_command_completed_cb;
    alp_init_args.alp_command_result_cb = &on_alp_command_result_cb;
    alp_layer_init(&alp_init_args, false);
    
    d7ap_fs_write_dll_conf_active_access_class(0x11); // use scanning AC, visible in d7ap_fs_data.c
    uarthandler = uart_init(UART_PORT_IDX, UART_BAUDRATE, UART_PINS);
    if(uarthandler == NULL) {
        // Initialization failed, handle the error
    }

    init_user_files();
    //uart_send_string(uarthandler, "UART Initialized Successfully\r\n");

    uart_set_rx_interrupt_callback(uarthandler, uart_rx_callback);
    uart_rx_interrupt_enable(uarthandler);

    sched_register_task(&startTask);
    sched_register_task(&ask_data);
}