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

from d7a.alp.command import Command
from d7a.alp.interface import InterfaceType
from d7a.d7anp.addressee import Addressee, IdType
from d7a.sp.configuration import Configuration
from d7a.sp.qos import QoS, ResponseMode
from d7a.types.ct import CT
from modem.modem import Modem



# This example can be used with a node running the gateway app included in Sub-IoT.
# The gateway is continuously listening for foreground frames.
# Messages pushed by other nodes (running for example the sensor_push app) will be received by the gateway node,
# transmitted over serial and the received_command_callback() function below will be called.
# It will then transmit a dormant session to the configured UID or to the first transmitter of the first received message.
# This dormant session has a timeout of 5 minutes and will write file 0x42 (which is not yet defined in most devices but should raise an error visible in RTTLog)
from util.logger import configure_default_logger


def received_command_callback(cmd):
  global addressee_id, try_dormant_session
  try:
    logging.info("gotten message {}".format(cmd))
    addressee_id = cmd.interface_status.operand.interface_status.addressee.id
    try_dormant_session = True
  except:
    return

def rebooted_callback(cmd):
  logging.info("rebooted with reason: {}".format(cmd))

def send_dormant_session():
  global once, addressee_id, try_dormant_session, modem
  try_dormant_session = False
  if once and (addressee_id != 0):
    logging.info("transmitting the dormant session")
    once = False
    data = [0]

    cmd = Command.create_with_write_file_action(
      file_id=0x42,
      offset=0,
      data=data,
      interface_type=InterfaceType.D7ASP,
      interface_configuration=Configuration(
        qos=QoS(resp_mod=ResponseMode.RESP_MODE_ANY),
        addressee=Addressee(
          id_type=IdType.UID,
          id=addressee_id,
          access_class=0x11
        ),
        dorm_to=CT.compress(60 * 5)
      )
    )

    modem.execute_command(
      timeout_seconds=0,
      alp_command=cmd
    )

argparser = argparse.ArgumentParser()
argparser.add_argument("-d", "--device", help="serial device /dev file modem",
                            default="/dev/ttyUSB0")
argparser.add_argument("-r", "--rate", help="baudrate for serial device", type=int, default=115200)
argparser.add_argument("-v", "--verbose", help="verbose", default=False, action="store_true")
argparser.add_argument("-u", "--uid", help="UID of the receiver of the dormant session, if not given, it will wait for the first message it receives and respond on that UID. (in hexstring, for example 0xb57000009151d)", default=None)
config = argparser.parse_args()

configure_default_logger(config.verbose)

modem = Modem(config.device, config.rate, unsolicited_response_received_callback=received_command_callback, rebooted_callback=rebooted_callback)
modem.connect()
once = True

if config.uid is not None:
  try_dormant_session = True
  addressee_id = int(config.uid, 16)
else:
  try_dormant_session = False
  addressee_id = 0

send_dormant_session()

try:
  while True:
    if try_dormant_session:
      send_dormant_session()
    time.sleep(0.1)
    pass
except KeyboardInterrupt:
  sys.exit(0)


