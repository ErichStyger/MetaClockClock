# i2c_edma_b2b_transfer_slave

## Overview
The i2c_edma_b2b_transfer_slave example shows how to use i2c driver as slave to do board to board transfer 
with a EDMA master:

In this example, one i2c instance as slave and another i2c instance on the other board as master. Master sends a 
piece of data to slave, and receive a piece of data from slave. This example checks if the data received from 
slave is correct. I2C slave works as supporting board to help demonstrate master EDMA trasnfer, actually uses interrupt
way.

## Supported Boards
- [FRDM-K22F](../../../../_boards/frdmk22f/driver_examples/i2c/edma_b2b_transfer/slave/example_board_readme.md)
