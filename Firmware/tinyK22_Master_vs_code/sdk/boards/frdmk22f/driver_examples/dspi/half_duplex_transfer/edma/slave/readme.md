# dspi_half_duplex_edma_slave

## Overview
The dspi_half_duplex_edma_transfer_slave example shows how to receive and transmit data to master board.

Notice: The SPI slave of this example uses edma mode. The data transfer size(not buffer size) is twice of
the master's transfer size. The first half of the transmission will receive data, and the second half will send
data back to master, so the begain address of the receive buffer is &rxDataPolling[64].

In this example, one spi instance as slave and another spi instance on the other board as master. Master sends a 
piece of data to slave, and receive a piece of data from slave. This example checks if the data received from 
master is correct. And slave will print what it received.

## Supported Boards
- [FRDM-K22F](../../../../../_boards/frdmk22f/driver_examples/dspi/half_duplex_transfer/edma/slave/example_board_readme.md)
