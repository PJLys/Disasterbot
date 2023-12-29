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
import time
import signal

from d7a.alp.command import Command
from d7a.alp.interface import InterfaceType
from d7a.alp.operands.file import Data
from d7a.alp.operands.offset import Offset
from d7a.alp.operations.write_operations import WriteFileData
from d7a.alp.regular_action import RegularAction
from d7a.d7anp.addressee import Addressee, IdType

from d7a.phy.channel_id import ChannelID
from d7a.sp.configuration import Configuration
from d7a.sp.qos import ResponseMode, QoS, RetryMode
from d7a.system_files.engineering_mode import EngineeringModeFile, EngineeringModeMode
from d7a.system_files.factory_settings import FactorySettingsFile
from d7a.types.ct import CT

from modem.modem import Modem

from util.logger import configure_default_logger


def received_command_callback(cmd):
  logging.info(cmd)
  if cmd.execution_completed:
      sys.exit(0)

def rebooted_callback(cmd):
  global stop
  logging.info("modem rebooted with reason {}".format(cmd))
  stop = True

def cleanup(sig, frame):
  cmd = Command()
  emFile.mode = EngineeringModeMode.ENGINEERING_MODE_MODE_OFF
  cmd.add_action(
    RegularAction(
      operation=WriteFileData(
        operand=Data(
          offset=Offset(id=emFile.id),
          data=list(emFile)
        )
      )
    )
  )
  modem.execute_command(cmd)
  sys.exit(0)

argparser = argparse.ArgumentParser()
argparser.add_argument("-d", "--device", help="serial device /dev file modem",
                            default="/dev/ttyUSB0")
argparser.add_argument("-r", "--rate", help="baudrate for serial device", type=int, default=115200)
argparser.add_argument("-v", "--verbose", help="verbose", default=False, action="store_true")
argparser.add_argument("-c", "--channel-id", help="for example 868LP000 ; format FFFRCIII where FFF={433, 868, 915}, R={L, N, H, R (LORA)}, C={P (PN9), F (FEC), C (CW)} III=000...280", default="868LP000")
modes = ["OFF", "CONT_TX", "TRANSIENT_TX", "PER_RX", "PER_TX"]
argparser.add_argument("-m", "--mode", choices=modes, required=True)
argparser.add_argument("-e", "--eirp", help="EIRP in dBm", type=int, default=0)
argparser.add_argument("-t", "--timeout", help="timeout", type=int, default=0)
argparser.add_argument("-x", "--not_exe", help="Don't execute the command on the modem, just print the resulting bytes", default=False, action="store_true")
argparser.add_argument("-f", "--forward", help="forward write to engineering mode on access profile 0x01 using broadcast", default=False, action="store_true")
argparser.add_argument("-fp", "--factory-paramp", help="first set factory settings paramp", type=int, default=0xFF)
argparser.add_argument("-fg", "--factory-gaussian", help="first set factory settings gaussian", type=int, default=0xFF)
config = argparser.parse_args()
configure_default_logger(config.verbose)

ch = ChannelID.from_string(config.channel_id)
print("Using mode {} for channel {} with TX EIRP {} dBm".format(config.mode, config.channel_id, config.eirp))
mode = EngineeringModeMode.from_string(config.mode)

if (mode == EngineeringModeMode.ENGINEERING_MODE_MODE_PER_TX) and (config.timeout == 0):
  config.timeout = 255

emFile = EngineeringModeFile(mode=mode, flags=0, timeout=config.timeout, channel_id=ch, eirp=config.eirp)

stop = False

signal.signal(signal.SIGINT, cleanup)

print(list(emFile))

if not config.not_exe:
  modem = Modem(config.device, config.rate, unsolicited_response_received_callback=received_command_callback,
                rebooted_callback=rebooted_callback)
  modem.connect()

  cmd = Command()

  if config.forward:
    cmd.add_forward_action(
      interface_type=InterfaceType.D7ASP,
      interface_configuration=Configuration(
        qos=QoS(resp_mod=ResponseMode.RESP_MODE_PREFERRED, retry_mod=RetryMode.RETRY_MODE_NO),
        addressee=Addressee(
          access_class=0x11,
          id_type=IdType.NBID,
          id=CT.compress(2)
        )
      )
    )

  if (config.factory_gaussian != 0xFF) or (config.factory_paramp != 0xFF):
    if (config.factory_gaussian != 0xFF) or (config.factory_paramp != 0xFF):
      fact = FactorySettingsFile(gaussian=config.factory_gaussian, paramp=config.factory_paramp)
    elif config.factory_gaussian != 0xFF:
      fact = FactorySettingsFile(gaussian=config.factory_gaussian)
    else:
      fact = FactorySettingsFile(paramp=config.factory_paramp)
    cmd.add_action(
      RegularAction(
        operation=WriteFileData(
          operand=Data(
            offset=Offset(id=fact.id),
            data=list(fact)
          )
        )
      )
    )

  cmd.add_action(
    RegularAction(
      operation=WriteFileData(
        operand=Data(
          offset=Offset(id=emFile.id),
          data=list(emFile)
        )
      )
    )
  )

  done = False

  while not done:

    answ = modem.execute_command(
      alp_command=cmd
    )

    for answer in answ:
      if answer.execution_completed and not answer.completed_with_error:
        done = True
      else:
        logging.info("command {}completed with{} error".format('not ' if not answer.execution_completed else '', 'out' if not answer.completed_with_error else ''))

if mode != EngineeringModeMode.ENGINEERING_MODE_MODE_OFF:
  while not stop:
    time.sleep(0.1)
    pass
