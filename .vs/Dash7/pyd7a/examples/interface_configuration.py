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

from d7a.system_files.interface_configuration import InterfaceConfigurationFile
from d7a.alp.interface import InterfaceType
from d7a.alp.operands.interface_configuration import InterfaceConfiguration
from d7a.sp.configuration import Configuration
from d7a.sp.qos import QoS, ResponseMode, RetryMode
from d7a.d7anp.addressee import Addressee, IdType, NlsMethod
from d7a.types.ct import CT


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
argparser.add_argument("-f", "--file_id", help="File where we're writing the interface configuration", type=int, default=0x1D)
argparser.add_argument("-t", "--timeout", help="timeout", type=int, default=0)
config = argparser.parse_args()
configure_default_logger(config.verbose)

modem = Modem(config.device, config.rate, unsolicited_response_received_callback=received_command_callback)
modem.connect()
# D7 Example
interface_file = InterfaceConfigurationFile(
    interface_configuration=InterfaceConfiguration(
        interface_id=InterfaceType.D7ASP,
        interface_configuration=Configuration(
            qos=QoS(
                resp_mod=ResponseMode.RESP_MODE_PREFERRED,
                retry_mod=RetryMode.RETRY_MODE_NO,
                stop_on_err=False,
                record=False
            ),
            dorm_to=CT(),
            addressee=Addressee(
                nls_method=NlsMethod.NONE,
                id_type=IdType.NBID,
                access_class=0x01,
                id=CT(mant=3, exp=0)
            )
        )
    )
)

# LORAWAN OTAA Example
# interface_file = InterfaceConfigurationFile(
#     interface_configuration=InterfaceConfiguration(
#         interface_id=InterfaceType.LORAWAN_OTAA,
#         interface_configuration=LoRaWANInterfaceConfigurationOTAA(
#             adr_enabled=True,
#             request_ack=True,
#             app_port=2,
#             data_rate=0,
#             device_eui=[0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07],
#             app_eui=[0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01],
#             app_key=[0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F]
#         )
#     )
# )

modem.execute_command(
  alp_command=Command.create_with_write_file_action(
    file_id=config.file_id,
    data=list(interface_file),
  )
)

try:
    while True:
        pass
except KeyboardInterrupt:
    sys.exit(0)
