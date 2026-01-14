# cmsis_i2c_read_accel_value_transfer

## Overview
CMSIS-Driver defines generic peripheral driver interfaces for middleware making it reusable across a wide 
range of supported microcontroller devices. The API connects microcontroller peripherals with middleware 
that implements for example communication stacks, file systems, or graphic user interfaces. 
More information and usage method please refer to http://www.keil.com/pack/doc/cmsis/Driver/html/index.html.

The cmsis_i2c_read_accel_value example shows how to use CMSIS I2C driver to communicate with an i2c device:

 1. How to use the i2c driver to read a i2c device who_am_I register.
 2. How to use the i2c driver to write/read the device registers.

In this example, the values of three-axis accelerometer print to the serial terminal on PC through
the virtual serial port on board.

## Supported Boards
- [FRDM-K22F](../../../_boards/frdmk22f/cmsis_driver_examples/i2c/read_accel_value_transfer/example_board_readme.md)
- [FRDM-K32L2B](../../../_boards/frdmk32l2b/cmsis_driver_examples/i2c/read_accel_value_transfer/example_board_readme.md)
- [FRDM-KE02Z40M](../../../_boards/frdmke02z40m/cmsis_driver_examples/i2c/read_accel_value_transfer/example_board_readme.md)
- [TWR-KM34Z50MV3](../../../_boards/twrkm34z50mv3/cmsis_driver_examples/i2c/read_accel_value_transfer/example_board_readme.md)
- [TWR-KM34Z75M](../../../_boards/twrkm34z75m/cmsis_driver_examples/i2c/read_accel_value_transfer/example_board_readme.md)
- [TWR-KM35Z75M](../../../_boards/twrkm35z75m/cmsis_driver_examples/i2c/read_accel_value_transfer/example_board_readme.md)
