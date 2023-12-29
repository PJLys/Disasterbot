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
import os

from d7a.alp.command import Command
from d7a.alp.operands.file import Data
from d7a.alp.operands.length import Length
from d7a.alp.operands.offset import Offset
from d7a.alp.operations.write_operations import WriteFileData
from d7a.alp.regular_action import RegularAction

from modem.modem import Modem

from util.logger import configure_default_logger


def received_command_callback(cmd):
  logging.info(cmd)
  if cmd.execution_completed:
      sys.exit(0)

def rebooted_callback(cmd):
  global stop
  logging.warn("modem rebooted with reason {}".format(cmd))
  stop = True

argparser = argparse.ArgumentParser()
argparser.add_argument("-d", "--device", help="serial device /dev file modem",
                            default="/dev/ttyUSB0")
argparser.add_argument("-r", "--rate", help="baudrate for serial device", type=int, default=115200)
argparser.add_argument("-v", "--verbose", help="verbose", default=False, action="store_true")
argparser.add_argument("-f", "--file", help="file for OTA", required=True)
config = argparser.parse_args()
configure_default_logger(config.verbose, logging_level=logging.WARNING)

modem = Modem(config.device, config.rate, unsolicited_response_received_callback=received_command_callback,
              rebooted_callback=rebooted_callback)
modem.connect()

data = [0]
print(data)

cmd = Command.create_with_write_file_action(file_id=199, data=data)
answ = modem.execute_command(cmd)
cmd = Command()


f=open(config.file, "rb")
total_length = os.path.getsize(config.file)

amount_of_bytes = 239
length = 0
try:
  bytes = list(bytearray(f.read(amount_of_bytes)))
  while len(bytes) != 0:
    cmd.add_action(
      RegularAction(
        operation=WriteFileData(
          operand=Data(
            offset=Offset(id=200, offset=Length(length)),
            data=bytes
          )
        )
      )
    )
    length += amount_of_bytes

    answ = modem.execute_command(cmd)
    worked = False
    for answer in answ:
      if answer.execution_completed and not answer.completed_with_error:
        worked = True
    if not worked:
      print("write failed")
      break
    cmd = Command()
    print(chr(27) + "[2J\n"+str(length)+"/"+str(total_length)+": "+str(length * 100 / total_length) + "%")

    bytes = list(bytearray(f.read(amount_of_bytes)))
finally:
  f.close()



