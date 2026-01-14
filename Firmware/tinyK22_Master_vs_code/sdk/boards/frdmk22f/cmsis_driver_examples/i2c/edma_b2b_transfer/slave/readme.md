# cmsis_i2c_edma_b2b_transfer_slave

## Overview
CMSIS-Driver defines generic peripheral driver interfaces for middleware making it reusable across a wide 
range of supported microcontroller devices. The API connects microcontroller peripherals with middleware 
that implements for example communication stacks, file systems, or graphic user interfaces. 
More information and usage method please refer to http://www.keil.com/pack/doc/cmsis/Driver/html/index.html.

The cmsis_i2c_edma_b2b_transfer_slave example shows how to use i2c driver as slave to do board to board transfer 
with a EDMA master:

In this example, one i2c instance as slave and another i2c instance on the other board as master. Master sends a 
piece of data to slave, and receive a piece of data from slave. This example checks if the data received from 
slave is correct.

## Supported Boards
- [FRDM-K22F](../../../../_boards/frdmk22f/cmsis_driver_examples/i2c/edma_b2b_transfer/slave/example_board_readme.md)
