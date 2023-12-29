#!/usr/bin/env python
#
# Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
#
# This file is part of pyd7a.
# See https://github.com/Sub-IoT/pyd7a for further info.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import argparse
import os
from time import sleep
import sys

import logging

from d7a.alp.command import Command
from d7a.alp.interface import InterfaceType
from d7a.d7anp.addressee import Addressee, IdType
from d7a.sp.configuration import Configuration
from d7a.sp.qos import QoS, ResponseMode
from modem.modem import Modem

# This example can be used with a node running the gateway app included in Sub-IoT, which is connect using the supplied serial device.
# It will query the sensor file (file 0x40) from other nodes running sensor_pull, using adhoc synchronization and print the results.
from util.logger import configure_default_logger


def received_command_callback(cmd):
      try:
      transmitter = cmd.interface_status.operand.interface_status.addressee.id
      transmitterHexString = hex(transmitter)[2:-1]
      operation = cmd.actions[0].operation
      if operation.file_type is None or operation.file_data_parsed is None:
        logging.info("received random data: {} from {}".format(operation.operand.data, transmitterHexString))
        return
      fileType = operation.file_type
      parsedData = operation.file_data_parsed
      logging.info("Received {} content: {} from {}".format(fileType.__class__.__name__,
                                              parsedData, transmitterHexString))

      if fileType.__class__ in [ButtonFile, PirFile]:
        device = {
            'manufacturer': 'Kwiam',
            'name': 'Push7_{}'.format(transmitterHexString),
            'identifiers': [transmitterHexString],
            # 'sw_version' : could read from version file
        }

        self.gather_and_send_voltage(transmitterHexString, device, parsedData)

        if fileType.__class__ is ButtonFile:
          unique_id = '{}_button{}'.format(transmitterHexString, parsedData.button_id)
          state_topic = 'homeassistant/{}/{}/state'.format(parsedData.component, unique_id)
          config_topic = 'homeassistant/{}/{}/config'.format(parsedData.component, unique_id)
          
          config = {
            'device': device,
            # 'icon': we could choose a custom icon
            # 'json_attributes_topic': ?
            'name': 'Button_{}'.format(parsedData.button_id),
            'qos': 1,
            'unique_id': unique_id,
            'state_topic': state_topic
          }
          self.mq.publish(config_topic, json.dumps(config), retain=True)
          self.mq.publish(state_topic, 'ON' if (1 << parsedData.button_id) & parsedData.state.value else 'OFF', retain=True)

          logging.info("published Button state: {} to topic {}".format('ON' if (1 << parsedData.button_id) & parsedData.state.value else 'OFF', state_topic))

        elif fileType.__class__ is PirFile:
          unique_id = '{}_pir'.format(transmitterHexString)
          state_topic = 'homeassistant/{}/{}/state'.format(parsedData.component, unique_id)
          config_topic = 'homeassistant/{}/{}/config'.format(parsedData.component, unique_id)

          config = {
            'device': device,
            # 'icon': we could choose a custom icon
            # 'json_attributes_topic': ?
            'name': 'Pir_state',
            'qos': 1,
            'unique_id': unique_id,
            'state_topic': state_topic
          }
          self.mq.publish(config_topic, json.dumps(config), retain=True)
          self.mq.publish(state_topic, 'ON' if (parsedData.pir_state) else 'OFF', retain=True)

          logging.info("published Pir state: {} to topic {}".format('ON' if (parsedData.pir_state) else 'OFF', state_topic))

    except (AttributeError, IndexError):
      # probably an answer on downlink we don't care about right now
      return
    except:
      exc_type, exc_value, exc_traceback = sys.exc_info()
      lines = traceback.format_exception(exc_type, exc_value, exc_traceback)
      trace = "".join(lines)
      logging.error("Exception while processing command: \n{}".format(trace))

argparser = argparse.ArgumentParser()
argparser.add_argument("-d", "--device", help="serial device /dev file modem",
                            default="/dev/ttyUSB0")
argparser.add_argument("-r", "--rate", help="baudrate for serial device", type=int, default=115200)
argparser.add_argument("-v", "--verbose", help="verbose", default=False, action="store_true")
config = argparser.parse_args()

configure_default_logger(config.verbose)

modem = Modem(config.device, config.rate, unsolicited_response_received_callback=received_command_callback)
modem.connect()
logging.info("Executing query...")
modem.execute_command_async(
  alp_command=Command.create_with_read_file_action(
    file_id=0x41,
    length=2,
    interface_type=InterfaceType.D7ASP,
    interface_configuration=Configuration(
      qos=QoS(resp_mod=ResponseMode.RESP_MODE_ALL),
      addressee=Addressee(
        access_class=0x11,
        id_type=IdType.NOID
      )
    )
  )
)
try:
  while True:
    sleep(5)
except KeyboardInterrupt:
  sys.exit(0)
