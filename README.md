# Disasterbot

### About
This part of the project includes the code for the LineBot.

### Capabilities
- Follow Line
- Receive UART commands
- Sleep

### Working

![LineBot_SM](https://github.com/PJLys/Disasterbot/assets/67599688/1bf721d9-40b7-4bc6-9a61-78de263fdaf3)

The Linebot will be asleep most of the time, until woken up by a GPIO interrupt originating from the IoT module. While asleep, the UART module will still be able to receive messages. When woken up, the Linebot will
execute the tasks from UART. If none, it will just follow the line before going to sleep and sending the encoder data to the IoT module.

