# cmsis_i2c_int_b2b_transfer_master

## Overview
CMSIS-Driver defines generic peripheral driver interfaces for middleware making it reusable across a wide 
range of supported microcontroller devices. The API connects microcontroller peripherals with middleware 
that implements for example communication stacks, file systems, or graphic user interfaces. 
More information and usage method please refer to http://www.keil.com/pack/doc/cmsis/Driver/html/index.html.

The i2c_interrupt_b2b_transfer_master example shows how to use CMSIS i2c driver as master to do board to board transfer 
with interrupt:

In this example, one i2c instance as master and another i2c instance on the other board as slave. Master sends a 
piece of data to slave, and receive a piece of data from slave. This example checks if the data received from 
slave is correct.

## Supported Boards
- [FRDM-K22F](../../../../_boards/frdmk22f/cmsis_driver_examples/i2c/int_b2b_transfer/master/example_board_readme.md)
- [FRDM-K32L2B](../../../../_boards/frdmk32l2b/cmsis_driver_examples/i2c/int_b2b_transfer/master/example_board_readme.md)
- [FRDM-KE02Z40M](../../../../_boards/frdmke02z40m/cmsis_driver_examples/i2c/int_b2b_transfer/master/example_board_readme.md)
- [FRDM-MCXC242](../../../../_boards/frdmmcxc242/cmsis_driver_examples/i2c/int_b2b_transfer/master/example_board_readme.md)
- [FRDM-MCXC444](../../../../_boards/frdmmcxc444/cmsis_driver_examples/i2c/int_b2b_transfer/master/example_board_readme.md)
- [TWR-KM35Z75M](../../../../_boards/twrkm35z75m/cmsis_driver_examples/i2c/int_b2b_transfer/master/example_board_readme.md)
