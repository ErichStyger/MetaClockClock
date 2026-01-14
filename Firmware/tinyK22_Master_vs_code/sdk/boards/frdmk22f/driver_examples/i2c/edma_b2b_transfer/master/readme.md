# i2c_edma_b2b_transfer_master

## Overview
The i2c_edma_b2b_transfer_master example shows how to use i2c driver as master to do board to board transfer 
with EDMA:

In this example, one i2c instance as master and another i2c instance on the other board as slave. Master sends a 
piece of data to slave, and receive a piece of data from slave. This example checks if the data received from 
slave is correct.

## Supported Boards
- [FRDM-K22F](../../../../_boards/frdmk22f/driver_examples/i2c/edma_b2b_transfer/master/example_board_readme.md)
