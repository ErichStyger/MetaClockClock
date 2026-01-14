# cmsis_dspi_int_b2b_transfer_slave

## Overview
The dspi_int_b2b_transfer example shows how to use DSPI CMSIS driver in interrupt way:

In this example , we need two boards, one board used as DSPI master and another board used as DSPI slave.
The file 'dspi_int_b2b_transfer_slave.c' includes the DSPI slave code.
This example uses the transactional API in DSPI driver.

1. DSPI master send/received data to/from DSPI slave in interrupt .

## Supported Boards
- [FRDM-K22F](../../../../_boards/frdmk22f/cmsis_driver_examples/dspi/int_b2b_transfer/slave/example_board_readme.md)
