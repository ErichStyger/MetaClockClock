# vref_example

## Overview
The vref example shows how to use the vref driver.

In this example, the adc16 module is initialized and used to measure the VREF output voltage. So, it cannot use internal
VREF as the reference voltage.

Then, user should configure the VREF output pin as the ADC16's sample input. When running the project, it will firstly
measure the VREF output voltage within the default (factory) trim value. Then, it will measure the VREF output voltage
under different trim values.

## Supported Boards
- [FRDM-K22F](../../_boards/frdmk22f/driver_examples/vref/example_board_readme.md)
- [TWR-KM34Z50MV3](../../_boards/twrkm34z50mv3/driver_examples/vref/example_board_readme.md)
- [TWR-KM35Z75M](../../_boards/twrkm35z75m/driver_examples/vref/example_board_readme.md)
