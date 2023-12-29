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

from pprint import pprint
from bitstring import ConstBitStream
from d7a.dll.parser import Parser, FrameType

part1 = [
    0xd7,                                           # interface start
    0x04, 0x00, 0x00, 0x00,                         # fifo config
    0x20,                                           # addressee
    0x24, 0x8a, 0xb6 
]

part2 = [
    0x01, 0x51, 0xc7, 0x96, 0x6d,  # ID
    0x20,                                           # action=32/ReturnFileData
    0x40,                                           # File ID
    0x00,                                           # offset
    0x04,                                           # length
    0x00, 0xf3, 0x00, 0x00,                         # data
    0xd7,                                           # interface start
    0x04, 0x00, 0x00, 0x00                          # fifo config
]

part3 = [
    0x20,                                           # addressee
    0x24, 0x8a, 0xb6, 0x00, 0x52, 0x0b, 0x35, 0x2c, # ID
    0x20,                                           # action=32/ReturnFileData
    0x40,                                           # File ID
    0x00,                                           # offset
    0x00                                            # length
]

parser = Parser(frame_type=FrameType.BACKGROUND)


cmds, _ = parser.parse(ConstBitStream(bytes=part1))
for cmd in cmds: pprint(cmd.as_dict())

cmds, _ = parser.parse(ConstBitStream(bytes=part2))
for cmd in cmds: pprint(cmd.as_dict())

cmds, _ = parser.parse(ConstBitStream(bytes=part2))
for cmd in cmds: pprint(cmd.as_dict())
