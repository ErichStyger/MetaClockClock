# dspi_interrupt_b2b_transfer_master

## Overview
The dspi_interrupt_b2b_transfer example shows how to use DSPI driver in interrupt way:

In this example , we need two boards, one board used as DSPI master and another board used as DSPI slave.
The file 'dspi_interrupt_b2b_transfer_master.c' includes the DSPI master code.
This example uses the transactional API in DSPI driver.

1. DSPI master send/received data to/from DSPI slave in interrupt . (DSPI Slave using interrupt to receive/send the data)

## Supported Boards
- [FRDM-K22F](../../../../_boards/frdmk22f/driver_examples/dspi/interrupt_b2b_transfer/master/example_board_readme.md)
