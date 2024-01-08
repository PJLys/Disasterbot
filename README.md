# ATXMEGA_Rover

## About the project
This is the movement part of a Mars Rover prototype.
The bot will follow a line in a test environment. It will interact with an IoT module on top (STM32).

![Full project](https://github.com/PJLys/Disasterbot/assets/67599688/01fcf0c4-ca07-42f6-8573-27b129ecc090)


## Branches

### xmega

This is the code for the linebot. It allows the linebot to follow a line or move in a square depending on what is preferred.
The linebot will go to sleep and can be woken up by an interrupt at the cursorstick pins (PORTB 3-7).

### nucleo-32

The nucleo-32 works as the IoT chip, it performs measurements of light, temperature and humidity. It also simulates dust and radiation measurements.

### dash7

This is the code for the dash7 module. It connects with the nucleo-32 via UART and will wake it up when it wants to communicate the latest measurements. Then it pushes the data to the gateway.

### BT

This is code for the nRF52840 powered Seeed Studio chip. The code implements a bluetooth central-peripheral connection that allows us to send data over UART. 
