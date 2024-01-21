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
from time import sleep
import sys
import json
import time
from queue import Queue
import struct
import mysql.connector


import logging

from d7a.alp.command import Command
from d7a.alp.interface import InterfaceType
from d7a.d7anp.addressee import Addressee, IdType
from d7a.sp.configuration import Configuration
from d7a.sp.qos import QoS, ResponseMode
from modem.modem import Modem

from d7a.alp.operands.length import Length
from d7a.alp.operations.requests import ReadFileData
from d7a.alp.operations.responses import ReturnFileData

# This example can be used with a node running the gateway app included in Sub-IoT, which is connect using the supplied serial device.
# It will query the sensor file (file 0x40) from other nodes running sensor_pull, using adhoc synchronization and print the results.
from util.logger import configure_default_logger

processing_queue = Queue()
class CustomJSONEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, Length):
            return obj.value
        return super().default(obj)

processing = False

def transform_incoming_data(data):
    bytestream = data
    # Convert to Big-Endian float
    be_val = 0
    #se_val = 0
    for i, byte in enumerate(bytestream):    
        be_val += byte * (256 ** (3 - i))    
        #se_val += byte * (256 ** i)
        # Interpret as float
        be_float = struct.unpack('>f', struct.pack('>I', be_val))[0]
        #se_float = struct.unpack('<f', struct.pack('<I', se_val))[0]
    return be_float
        
def handle_read_file_action(action, cmd):
    logging.info(cmd)

    read_file_data_operation = action.operation
    data = read_file_data_operation.operand.data
    file_id = read_file_data_operation.operand.offset.id
    offset = read_file_data_operation.operand.offset.offset
    logging.info(f"Received data: {data} from file {file_id} at offset {offset}")
    data2 = (transform_incoming_data(data),)
    # Create a dictionary to store data
    data_dict = {'data': data2, 'file_id': file_id, 'offset': offset}
    
     # Log data to the console
    print(f"Received data: {data} from file {file_id} at offset {offset}")
    # Write to file
    # with open("output.txt", 'a') as txt_file:
        # json.dump(data_dict, txt_file, indent=2, cls=CustomJSONEncoder)
    try:
        if file_id == 60:
            insert_query = """
                INSERT INTO sensordata (Temperature)
                VALUES (%s)
            """
                    # Execute the query
            cursor.execute(insert_query, data2)
                    # Commit the changes to the database
            connection.commit()
            logging.info("Pused the Temperature data to database")
        elif file_id == 61:
            insert_query = """
                INSERT INTO sensordata (Light)
                VALUES (%s)
            """
                    # Execute the query
            cursor.execute(insert_query, data2)
                    # Commit the changes to the database
            connection.commit()
          
        elif file_id == 62:
            insert_query = """
                INSERT INTO sensordata (Humidity)
                VALUES (%s)
            """
                    # Execute the query
            cursor.execute(insert_query, data2)
                    # Commit the changes to the database
            connection.commit()
        elif file_id == 63:
            insert_query = """
                INSERT INTO sensordata (Radiation)
                VALUES (%s)
            """
                    # Execute the query
            cursor.execute(insert_query, data2)
                    # Commit the changes to the database
            connection.commit()
        elif file_id == 65:
            insert_query = """
                INSERT INTO sensordata (Dust)
                VALUES (%s)
            """
                    # Execute the query
            cursor.execute(insert_query, data2)
                    # Commit the changes to the database
            connection.commit()
    except mysql.connector.Error as err:
        print(f"Database Error: {err}")
        # Rollback changes in case of an error
        connection.rollback()
            
    
        

def received_command_callback(cmd):
  #logging.info(cmd)
  #for action in cmd.actions:
  global processing
  if cmd.actions:
     handle_read_file_action(cmd.actions[0], cmd)
  else:
     logging.warning("Received command with no actions")
  processing = False
  #os._exit(0)

argparser = argparse.ArgumentParser()
argparser.add_argument("-d", "--device", help="serial device /dev file modem",
                            default="/dev/ttyUSB0")
argparser.add_argument("-r", "--rate", help="baudrate for serial device", type=int, default=115200)
argparser.add_argument("-v", "--verbose", help="verbose", default=False, action="store_true")
config = argparser.parse_args()

configure_default_logger(config.verbose)

modem = Modem(config.device, config.rate, unsolicited_response_received_callback=received_command_callback)
modem.connect()

# Establish a connection to the MySQL server
connection = mysql.connector.connect(
    host='192.168.1.57',
    user='root',
    password='jens',
    database='disasterbot'
)
cursor = connection.cursor()

logging.info("Executing query...")
while True :
    processing_queue.put((0x3C, 4))
    processing_queue.put((0x3D, 4))
    processing_queue.put((0x3E, 4))
    processing_queue.put((0x3F, 4))
    processing_queue.put((0x41, 4))
    while not processing_queue.empty():  
        if not processing:
            time.sleep(10)
            file_id, length = processing_queue.get()
           # modem.execute_command_async(
            alp_command=Command.create_with_read_file_action(
                file_id=file_id,
                length=length,
                interface_type=InterfaceType.D7ASP,
                interface_configuration=Configuration(
                  qos=QoS(resp_mod=ResponseMode.RESP_MODE_ALL),
                  addressee=Addressee(
                    access_class=0x11,
                    id_type=IdType.NOID
                  )
                )
            )
            logging.info(alp_command)
            modem.execute_command_async(
                alp_command
            )
            processing = True
            
    #if processing_queue.empty():
        #os._exit(0)
    
try:
  while True:
    sleep(5)
except KeyboardInterrupt:
  sys.exit(0)
