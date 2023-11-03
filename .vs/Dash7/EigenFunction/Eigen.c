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

#define SENSOR_FILE_ID           0x40
#define SENSOR_FILE_SIZE         2
#define SENSOR_INTERVAL_SEC	TIMER_TICKS_PER_SEC * 10

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

void receive_sensor_measurment()
{

  //console/uart code to receive sensor data

  // Generate ALP command.
  // We will be sending a return file data action, without a preceding file read request.
  // This is an unsolicited message, where we push the sensor data to the gateway(s).
  
  // alloc command. This will be freed when the command completes
  alp_command_t* command = alp_layer_command_alloc(false, false);
  
  // forward to the D7 interface
  alp_append_forward_action(command, (alp_interface_config_t*)&itf_config, d7ap_session_config_length(&itf_config.d7ap_session_config));

  // add the return file data action
  alp_append_return_file_data_action(command, SENSOR_FILE_ID, 0, SENSOR_FILE_SIZE, (uint8_t*)&temperature);

  // and finally execute this
  alp_layer_process(command);
}

void on_alp_command_completed_cb(uint8_t tag_id, bool success)
{
    if(success)
      log_print_string("Command (%i) completed successfully", tag_id);
    else
      log_print_string("Command failed, no ack received");

    // reschedule sensor measurement
    timer_post_task_delay(&receive_sensor_measurment, SENSOR_INTERVAL_SEC);
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
}

void bootstrap()//main function I suppose
{
    log_print_string("Device booted\n");
    d7ap_fs_init();
    d7ap_init();

    alp_init_args.alp_command_completed_cb = &on_alp_command_completed_cb;
    alp_init_args.alp_command_result_cb = &on_alp_command_result_cb;
    alp_layer_init(&alp_init_args, false);

}