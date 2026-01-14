# dspi_interrupt

## Overview
The dspi_interrupt example shows how to use DSPI driver in interrupt way:

In this example , one dspi instance used as DSPI master and another dspi instance used as DSPI slave in the same board.
This example does not use the transactional API in DSPI driver. It's a demonstration that how to use the interrupt in KSDK driver.

1. DSPI master send/received data to/from DSPI slave in interrupt . (DSPI Slave using interrupt to receive/send the data)

## Supported Boards
- [FRDM-K22F](../../../_boards/frdmk22f/driver_examples/dspi/interrupt/example_board_readme.md)
