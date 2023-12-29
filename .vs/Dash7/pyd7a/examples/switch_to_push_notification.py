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
import struct

from d7a.alp.command import Command
from d7a.alp.interface import InterfaceType
from d7a.alp.operands.file import Data, DataRequest
from d7a.alp.operands.file_header import FileHeaderOperand
from d7a.alp.operands.interface_configuration import InterfaceConfiguration
from d7a.alp.operands.length import Length
from d7a.alp.operands.offset import Offset
from d7a.alp.operands.query import QueryOperand, QueryType, ArithQueryParams, ArithComparisonType
from d7a.alp.operations.break_query import BreakQuery
from d7a.alp.operations.requests import ReadFileData
from d7a.alp.operations.write_operations import WriteFileData, WriteFileHeader
from d7a.alp.regular_action import RegularAction
from d7a.d7anp.addressee import Addressee, IdType, NlsMethod
from d7a.fs.file_header import FileHeader
from d7a.fs.file_permissions import FilePermissions
from d7a.fs.file_properties import FileProperties, ActionCondition, StorageClass
from d7a.sp.configuration import Configuration
from d7a.sp.qos import QoS, ResponseMode, RetryMode
from d7a.types.ct import CT
from modem.modem import Modem
from util.logger import configure_default_logger


# This example can be used with a node running the modem app included in Sub-IoT.
# It will remotely reconfigure other nodes running sensor_pull so that they will push (using D7AActpP),
# the temperature value when it exceeds the supplied threshold value

def received_command_callback(cmd):
  logging.info(cmd)

argparser = argparse.ArgumentParser()
argparser.add_argument("-d", "--device", help="serial device /dev file modem",
                            default="/dev/ttyUSB0")
argparser.add_argument("-r", "--rate", help="baudrate for serial device", type=int, default=115200)
argparser.add_argument("-v", "--verbose", help="verbose", default=False, action="store_true")
argparser.add_argument("-t", "--temperature", help="temperature threshold value (in degrees C)", type=float, default=0)
config = argparser.parse_args()

configure_default_logger(config.verbose)

modem = Modem(config.device, config.rate, unsolicited_response_received_callback=received_command_callback)
modem.connect()

# command to be executed as an action: first do an arithmetic comparison of the sensor value with the supplied value...
query_sensor_file_cmd = Command(generate_tag_request_action=False)
query_sensor_file_cmd.add_action(
  RegularAction(operation=BreakQuery(operand=QueryOperand(
      type=QueryType.ARITH_COMP_WITH_VALUE,
      mask_present=False,
      params=ArithQueryParams(signed_data_type=False, comp_type=ArithComparisonType.GREATER_THAN),
      compare_length = Length(2),
      compare_value=struct.pack(">H", int(config.temperature * 10)),
      file_a_offset=Offset(id=0x40, offset=Length(0))))
  )
)

# ...if the query succeeds, read the file
query_sensor_file_cmd.add_action(RegularAction(operation=ReadFileData(operand=DataRequest(
  offset=Offset(id=0x40),
  length=Length(2)
))))

# the interface config to send the result of above action to
interface_config = InterfaceConfiguration(InterfaceType.D7ASP, Configuration(
  qos=QoS(resp_mod=ResponseMode.RESP_MODE_NO, retry_mod=RetryMode.RETRY_MODE_NO, stop_on_err=False, record=False),
  dorm_to=CT(),
  addressee=Addressee(access_class=0x01, id_type=IdType.NOID, id=None, nls_method=NlsMethod.NONE)
))

# create the command to write the action file and interface configuration file,
# adapt the properties on the sensor file and forward using the downlink access profile
cmd = Command()
cmd.add_forward_action(InterfaceType.D7ASP, Configuration(
  qos=QoS(resp_mod=ResponseMode.RESP_MODE_ALL, retry_mod=RetryMode.RETRY_MODE_NO, stop_on_err=False, record=False),
  dorm_to=CT(),
  addressee=Addressee(access_class=0x11, id_type=IdType.NBID, id=CT.compress(3), nls_method=NlsMethod.NONE)
))

# the action
cmd.add_action(RegularAction(operation=WriteFileData(operand=Data(
  offset=Offset(id=0x41, offset=Length(0)),
  data=list(query_sensor_file_cmd))))
)

# the interface config
cmd.add_action(RegularAction(operation=WriteFileData(operand=Data(
  offset=Offset(id=0x42, offset=Length(0)),
  data=list(interface_config))))
)

# enable action on sensor file
file_header = FileHeader(
  properties=FileProperties(act_enabled=True, act_condition=ActionCondition.WRITE, storage_class=StorageClass.TRANSIENT),
  alp_command_file_id=0x41,
  interface_file_id=0x42,
  file_size=2,
  allocated_size=2,
  permissions=FilePermissions()
)

cmd.add_action(RegularAction(operation=WriteFileHeader(operand=FileHeaderOperand(file_id=0x40, file_header=file_header))))

print("Executing ...")
responses = modem.execute_command(alp_command=cmd, timeout_seconds=30)

for resp in responses:
  if not resp.execution_completed:
    print("{} reconfigured to use D7AActP".format(resp.get_d7asp_interface_status().addressee.id))


print("Done")