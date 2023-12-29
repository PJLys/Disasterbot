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

import logging
import sys
import time

from modem.modem import Modem



# This example can be used with a node running the gateway app included in Sub-IoT.
# The gateway is continuously listening for foreground frames.
# Messages pushed by other nodes (running for example the sensor_push app) will be received by the gateway node,
# transmitted over serial and the received_command_callback() function below will be called.
from util.logger import configure_default_logger


def received_command_callback(cmd):
  logging.info(cmd)

def rebooted_callback(cmd):
  logging.info("rebooted with reason: {}".format(cmd))

argparser = argparse.ArgumentParser()
argparser.add_argument("-d", "--device", help="serial device /dev file modem",
                            default="/dev/ttyUSB0")
argparser.add_argument("-r", "--rate", help="baudrate for serial device", type=int, default=115200)
argparser.add_argument("-v", "--verbose", help="verbose", default=False, action="store_true")
config = argparser.parse_args()

configure_default_logger(config.verbose)

modem = Modem(config.device, config.rate, unsolicited_response_received_callback=received_command_callback, rebooted_callback=rebooted_callback)
modem.connect()

try:
  while True:
    time.sleep(0.1)
    pass
except KeyboardInterrupt:
  sys.exit(0)
