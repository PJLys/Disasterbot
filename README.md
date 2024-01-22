# Disasterbot: Bluetooth module

### About

<div align="center">

Code for the Bluetooth Central and Peripheral devices, to allow for bi-directional communication of UART messages.

![BT](https://github.com/PJLys/Disasterbot/assets/67599688/1cedc0a9-75b4-4592-98c3-7cd3b1aae40b)

</div>

### How to use

Use Arduino IDE in order to deploy the code on the seed studio. In order to use Arduino IDE for the Seeed Studio chip, add the following link to the additional
boards manager URLs: https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json

This allows you to download the Seeed nRF52 Boards and select the Seeed XIAO nRF52840 sense board with the Board Manager.

Choose the Bluetooth role by running either Peripheral or Central. Make sure they're different from eachother. Communicate to the boards via UART (terminal or other) and they will relay these messages over Bluetooth.

### Hardware

- Seeed studio nRF52540
- [Tutorial](https://wiki.seeedstudio.com/XIAO_BLE/)
