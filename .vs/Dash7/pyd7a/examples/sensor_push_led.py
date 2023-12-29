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
from d7a.types.ct import CT
from d7a.d7anp.addressee import Addressee, IdType
from d7a.sp.qos import QoS, ResponseMode
from d7a.sp.configuration import Configuration
from d7a.alp.interface import InterfaceType
from d7a.alp.command import Command

import logging
import platform
import signal
import sys
import traceback

import time
import json

import paho.mqtt.client as mqtt

from modem.modem import Modem
from util.logger import configure_default_logger


class Modem2Mqtt():

  def __init__(self):
    argparser = argparse.ArgumentParser()
    argparser.add_argument("-d", "--device", help="serial device /dev file modem",
                           default="/dev/ttyACM0")
    argparser.add_argument("-r", "--rate", help="baudrate for serial device", type=int, default=115200)
    argparser.add_argument("-v", "--verbose", help="verbose", default=False, action="store_true")
    argparser.add_argument("-b", "--broker", help="mqtt broker hostname",
                             default="localhost")

    self.last_transmitter_id = 0

    self.config = argparser.parse_args()
    configure_default_logger(self.config.verbose)

    self.modem = Modem(self.config.device, self.config.rate, self.on_command_received)
    self.modem.connect()
    self.connect_to_mqtt()


  def connect_to_mqtt(self):
    self.connected_to_mqtt = False

    self.mq = mqtt.Client("", True, None, mqtt.MQTTv31)
    self.mq.on_connect = self.on_mqtt_connect
    self.mq.on_message = self.on_mqtt_message
    self.mq.username_pw_set("mqtt", "Be&FX&Rw9sFdr@8W")
    self.mqtt_topic_temperature = "home/temperature"
    self.mqtt_topic_led_status = "home/led-status"
    self.mqtt_topic_outgoing = "home/led-cmd"

    self.mq.connect(self.config.broker, 1883, 60)
    self.mq.loop_start()
    while not self.connected_to_mqtt: pass  # busy wait until connected
    logging.info("Connected to MQTT broker on {}, sending to topics {} and {}. Subscribing on topic {}".format(
      self.config.broker,
      self.mqtt_topic_temperature,
      self.mqtt_topic_led_status,
      self.mqtt_topic_outgoing
    ))

  def on_mqtt_connect(self, client, config, flags, rc):
    self.mq.subscribe(self.mqtt_topic_outgoing)
    self.connected_to_mqtt = True

  def on_mqtt_message(self, client, config, msg):
    # downlink can only mean we want to set the led to a new state for now
    logging.info("gotten downlink {} to {}".format(msg.payload, self.last_transmitter_id)) # TODO

    # we can only send downlink if we already got a message from a sensor
    if self.last_transmitter_id == 0:
      return

    data = [1 if msg.payload[1:] == "ON" else 0]
    logging.info("sending payload {}".format(data))

    logging.info("over {}".format("dormant session" if msg.payload[0] == "D" else "low power listening"))
    cmd = Command.create_with_write_file_action(
      file_id=0x42,
      offset=2,
      data=data,
      interface_type=InterfaceType.D7ASP,
      interface_configuration=Configuration(
        qos=QoS(resp_mod=ResponseMode.RESP_MODE_ANY),
        addressee=Addressee(
          id_type=IdType.UID,
          id=self.last_transmitter_id,
          access_class=0x11
        ),
        dorm_to=CT.compress(60 * 5) if msg.payload[0] == "D" else CT()
      )
    )

    self.modem.execute_command(
      timeout_seconds=0,
      alp_command=cmd
    )
    

  def __del__(self): # pragma: no cover
    try:
      self.mq.loop_stop()
      self.mq.disconnect()
    except: pass

  def on_command_received(self, cmd):
    try:
      self.last_transmitter_id = cmd.interface_status.operand.interface_status.addressee.id
      data = cmd.actions[0].operation.operand.data
      logging.info("Command received: binary ALP (size {})".format(len(data)))

      temperature = (data[0] * 0x100 + data[1]) / 10.0
      result_temperature = {'active': 'ON', 'Temperature': temperature}
      result_json_temperature = json.dumps(result_temperature)

      led_status = data[2] != 0
      result_led_status = {'state': 'ON' if led_status else 'OFF'}
      result_json_led_status = json.dumps(result_led_status)

      logging.info("Gotten temperature {} and led is {}".format(temperature, "on" if led_status else "off"))

      #pass temperature and led status to seperate topics. This can also be done to the same topic
      self.mq.publish(self.mqtt_topic_temperature, result_json_temperature)
      self.mq.publish(self.mqtt_topic_led_status, result_json_led_status)
    except (AttributeError, IndexError):
      # probably an answer on downlink we don't care about right now
      return
    except:
      exc_type, exc_value, exc_traceback = sys.exc_info()
      lines = traceback.format_exception(exc_type, exc_value, exc_traceback)
      trace = "".join(lines)
      logging.error("Exception while processing command: \n{}".format(trace))

  def run(self):
    logging.info("Started")
    keep_running = True
    while keep_running:
      try:
        if platform.system() == "Windows":
          time.sleep(1)
        else:
          signal.pause()
      except KeyboardInterrupt:
        logging.info("received KeyboardInterrupt... stopping processing")
        keep_running = False

if __name__ == "__main__":
  Modem2Mqtt().run()
