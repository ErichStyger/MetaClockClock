# dspi_interrupt_b2b_slave

## Overview
The dspi_interrupt_b2b example shows how to use DSPI driver in interrupt way:

In this example , we need two boards , one board used as DSPI master and another board used as DSPI slave.
The file 'dspi_interrupt_b2b_slave.c' includes the DSPI slave code.
This example does not use the transactional API in DSPI driver. It's a demonstration that how to use the interrupt in KSDK driver.

1. DSPI master send/received data to/from DSPI slave in interrupt . (DSPI Slave using interrupt to receive/send the data)

## Supported Boards
- [FRDM-K22F](../../../../_boards/frdmk22f/driver_examples/dspi/interrupt_b2b/slave/example_board_readme.md)
