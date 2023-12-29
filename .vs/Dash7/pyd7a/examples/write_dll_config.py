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
import sys
import logging

from d7a.alp.command import Command
from d7a.system_files.system_file_ids import SystemFileIds

from modem.modem import Modem

from util.logger import configure_default_logger


def received_command_callback(cmd):
  logging.info(cmd)
  if cmd.execution_completed:
      sys.exit(0)

argparser = argparse.ArgumentParser()
argparser.add_argument("-d", "--device", help="serial device /dev file modem",
                            default="/dev/ttyUSB0")
argparser.add_argument("-r", "--rate", help="baudrate for serial device", type=int, default=115200)
argparser.add_argument("-v", "--verbose", help="verbose", default=False, action="store_true")
argparser.add_argument("-ac", "--access-class", help="specifier for access profile. Default 0x01 is continuous scan, 0x11 is bg scan, 0x21 is no scan", default="21")
config = argparser.parse_args()
configure_default_logger(config.verbose)

data = [int(config.access_class, 16)]

modem = Modem(config.device, config.rate, unsolicited_response_received_callback=received_command_callback)
modem.connect()



modem.execute_command(
  alp_command=Command.create_with_write_file_action(SystemFileIds.DLL_CONFIG.value, data)
)

