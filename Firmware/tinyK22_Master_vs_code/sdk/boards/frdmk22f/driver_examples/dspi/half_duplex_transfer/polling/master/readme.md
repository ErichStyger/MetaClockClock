# dspi_half_duplex_polling_master

## Overview
The dspi_half_duplex_polling_transfer_master example shows how to use driver API to transfer in half-duplex way.  

In this example, one spi instance as master and another spi instance on the other board as slave. Master sends a
piece of data to slave, and receive a piece of data from slave. This example checks if the data received from 
slave is correct. 
Besides, master will transfer in polling way.

## Supported Boards
- [FRDM-K22F](../../../../../_boards/frdmk22f/driver_examples/dspi/half_duplex_transfer/polling/master/example_board_readme.md)
