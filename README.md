# ATXMEGA_Rover
This is the movement part of a Mars Rover prototype.

The bot will follow a line in a test environment. It will interact with an IoT module on top.

This schematic shows the state machine of the bot:

![LineBot_SM drawio](https://github.com/PJLys/ATXMEGA_Rover/assets/67599688/85f3a0ea-f915-4c22-a50f-bb811d936fbe)

Since it shares the power source with the IoT module, it has to be able to sleep when the IoT module is sensing/communicating.
It gets woken up by the STM32 IoT chip when it's finished doing its measurements. Then moves further, to eventually stop and send an interrupt in the opposite 
direction to wake up the STM32 again. (Whilst going to sleep itself).
