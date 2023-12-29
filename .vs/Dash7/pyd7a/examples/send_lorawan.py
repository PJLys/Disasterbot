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

import logging

from d7a.alp.command import Command
from d7a.alp.interface import InterfaceType
from d7a.alp.operands.lorawan_interface_configuration_otaa import LoRaWANInterfaceConfigurationOTAA

from modem.modem import Modem

from util.logger import configure_default_logger

# This example can be used with a node running the mode app included in Sub-IoT, which is connect using the supplied serial device.
# It will send a LoRaWAN message print the result.

# in Sub-IoT-Stack:
# - UID file defines the deviceEUI
# - USER_FILE_LORAWAN_KEYS_FILE_ID (0x41) defines the appEUI and appKey


def received_command_callback(cmd):
  logging.info(cmd)
  if cmd.execution_completed:
    os._exit(0)

argparser = argparse.ArgumentParser()
argparser.add_argument("-d", "--device", help="serial device /dev file modem",
                            default="/dev/ttyUSB0")
argparser.add_argument("-r", "--rate", help="baudrate for serial device", type=int, default=115200)
argparser.add_argument("-v", "--verbose", help="verbose", default=False, action="store_true")
config = argparser.parse_args()

configure_default_logger(config.verbose)

modem = Modem(config.device, config.rate, )
modem.connect()
logging.info("Executing query...")
result = modem.execute_command(
    alp_command=Command.create_with_read_file_action(
    file_id=0x40,
    length=8,
    interface_type=InterfaceType.LORAWAN_OTAA,
    interface_configuration=LoRaWANInterfaceConfigurationOTAA(
        request_ack=False,
        app_port=0x01,
        adr_enabled=False,
        data_rate=0
    )
    ),
    timeout_seconds=100
)

try:
	while True:
		pass
except KeyboardInterrupt:
	os._exit(0)
