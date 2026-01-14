# adc16_low_power_peripheral

## Overview
The advanced Low Power ADC project is designed to work with the Tower System or in a stand alone setting.
The code of this demo has been prepared and updated for use with the MCUXpresso Configuration Tools (Pins/Clocks/Peripherals).
The ADC Low Power demo application demonstrates the usage of the ADC peripheral while in a low power mode. The
microcontroller is first set to very low power stop (VLPS) mode. Every 500 ms, an interrupt wakes up the ADC module and
reads the current temperature of the microcontroller. Increment or decrement the temperature to see the changes, red lights
for higher and blue one for lower than average counted temperatures. You can open the mex file with MCUXpresso Config Tool to 
do further configuration of pin, clock and peripheral.

## Supported Boards
- [FRDM-K22F](../../../_boards/frdmk22f/demo_apps/adc16_low_power_peripheral/example_board_readme.md)
- [FRDM-MCXC242](../../../_boards/frdmmcxc242/demo_apps/adc16_low_power_peripheral/example_board_readme.md)
- [TWR-KM35Z75M](../../../_boards/twrkm35z75m/demo_apps/adc16_low_power_peripheral/example_board_readme.md)
