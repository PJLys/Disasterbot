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
from d7a.alp.interface import InterfaceType
from d7a.d7anp.addressee import Addressee, IdType

from d7a.sp.configuration import Configuration
from d7a.sp.qos import QoS, ResponseMode, RetryMode
from d7a.system_files.factory_settings import FactorySettingsFile
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
argparser.add_argument("-o", "--offset", help="offset of gain", default=0, type=int, required=False)
argparser.add_argument("-lr", "--low_rx", help="rx bandwidth for low rate", default=10468, type=int, required=False)
argparser.add_argument("-lb", "--low_bitrate", help="bitrate for low rate", default=9600, type=int, required=False)
argparser.add_argument("-lf", "--low_fdev", help="frequency deviation for low rate", default=4800, type=int, required=False)
argparser.add_argument("-lp", "--low_preamble_size", help="preamble size for low rate", default=4, type=int, required=False)
argparser.add_argument("-nr", "--normal_rx", help="rx bandwidth for normal rate", default=78646, type=int, required=False)
argparser.add_argument("-nb", "--normal_bitrate", help="bitrate for normal rate", default=55555, type=int, required=False)
argparser.add_argument("-nf", "--normal_fdev", help="frequency deviation for normal rate", default=50000, type=int, required=False)
argparser.add_argument("-np", "--normal_preamble_size", help="preamble size for normal rate", default=5, type=int, required=False)
argparser.add_argument("-hr", "--high_rx", help="rx bandwidth for high rate", default=125868, type=int, required=False)
argparser.add_argument("-hb", "--high_bitrate", help="bitrate for high rate", default=166667, type=int, required=False)
argparser.add_argument("-hf", "--high_fdev", help="frequency deviation for high rate", default=41667, type=int, required=False)
argparser.add_argument("-hp", "--high_preamble_size", help="preamble size for high rate", default=7, type=int, required=False)
argparser.add_argument("-pdl", "--detector_preamble_size_lo_rate", help="preamble detector size for low rate", default=3, type=int, required=False)
argparser.add_argument("-pdn", "--detector_preamble_size_normal_rate", help="preamble detector size for normal rate", default=3, type=int, required=False)
argparser.add_argument("-pdh", "--detector_preamble_size_hi_rate", help="preamble detector size for high rate", default=3, type=int, required=False)
argparser.add_argument("-ptl", "--preamble_tol_lo_rate", help="preamble tolerance of low rate", default=15, type=int, required=False)
argparser.add_argument("-ptn", "--preamble_tol_normal_rate", help="preamble tolerance of normal rate", default=10, type=int, required=False)
argparser.add_argument("-pth", "--preamble_tol_hi_rate", help="preamble tolerance of high rate", default=10, type=int, required=False)
argparser.add_argument("-rs", "--rssi_smoothing", help="rssi_smoothing (average of X samples)", default=8, type=int, required=False)
argparser.add_argument("-ro", "--rssi_offset", help="rssi_offset", default=0, type=int, required=False)
argparser.add_argument("-lbw", "--lora_bw", help="bandwidth for lora", default=125000, type=int, required=False)
argparser.add_argument("-lsf", "--lora_SF", help="Spreading factor for lora", default=9, type=int, required=False)
argparser.add_argument("-g", "--gaussian", help="gaussian filter for radio chip. BT=1/gaussian", default=2, type=int, required=False)
argparser.add_argument("-p", "--paramp", help="paramp in microseconds used for sending packages", default=40, type=int, required=False)
argparser.add_argument("-f", "--forward", help="forward write file to interface", default=False, action="store_true")
config = argparser.parse_args()
configure_default_logger(config.verbose)


modem = Modem(config.device, config.rate, unsolicited_response_received_callback=received_command_callback)
modem.connect()

print("gain set to {}, rx low {}, normal {} and high {}".format(config.offset, config.low_rx, config.normal_rx, config.high_rx))

fsFile = FactorySettingsFile(gain=config.offset, rx_bw_low_rate=config.low_rx, rx_bw_normal_rate=config.normal_rx,
                             rx_bw_high_rate=config.high_rx, bitrate_lo_rate=config.low_bitrate,
                             bitrate_normal_rate=config.normal_bitrate, bitrate_hi_rate=config.high_bitrate,
                             fdev_lo_rate=config.low_fdev, fdev_normal_rate=config.normal_fdev,
                             fdev_hi_rate=config.high_fdev,
                             preamble_size_lo_rate=config.low_preamble_size,
                             preamble_size_normal_rate=config.normal_preamble_size,
                             preamble_size_hi_rate=config.high_preamble_size,
                             preamble_detector_size_lo_rate=config.detector_preamble_size_lo_rate,
                             preamble_detector_size_normal_rate=config.detector_preamble_size_normal_rate,
                             preamble_detector_size_hi_rate=config.detector_preamble_size_hi_rate,
                             preamble_tol_lo_rate=config.preamble_tol_lo_rate,
                             preamble_tol_normal_rate=config.preamble_tol_normal_rate,
                             preamble_tol_hi_rate=config.preamble_tol_hi_rate,
                             rssi_smoothing=config.rssi_smoothing, rssi_offset=config.rssi_offset,
                             lora_bw=config.lora_bw, lora_SF=config.lora_SF,
                             paramp=config.paramp, gaussian=config.gaussian)

print(fsFile.__str__())

print( '[{}]'.format(', '.join(hex(byte) for byte in list(fsFile))))

interface_type = InterfaceType.HOST
interface_configuration = None

if config.forward:
  interface_type = InterfaceType.D7ASP
  interface_configuration = Configuration(
    qos=QoS(resp_mod=ResponseMode.RESP_MODE_PREFERRED, retry_mod=RetryMode.RETRY_MODE_NO),
    addressee=Addressee(
      access_class=0x11,
      id_type=IdType.NBID,
      id=CT.compress(2)
    )
  )

modem.execute_command(
  alp_command=Command.create_with_write_file_action(
    file_id=1,
    data=list(fsFile),
    interface_type=interface_type,
    interface_configuration=interface_configuration
  )
)

# try:
#     while True:
#         pass
# except KeyboardInterrupt:
#     sys.exit(0)
