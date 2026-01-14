# uart_9bit_interrupt_transfer

## Overview
The uart_9bit_interrupt_transfer example shows how to use uart driver in 9-bit mode in multi-slave system.
Master can send data to slave with certain address specifically, and slave can only receive data when it is addressed.

In this example, one uart instance is used with address configured. Its TX and RX pins are connected together.
First it sends a piece of data out, then addresses itself, after that sends the other piece of data. Only data
sent after the address can be received by itself.

## Supported Boards
- [FRDM-K22F](../../../_boards/frdmk22f/driver_examples/uart/9bit_interrupt_transfer/example_board_readme.md)
- [FRDM-K32L2B](../../../_boards/frdmk32l2b/driver_examples/uart/9bit_interrupt_transfer/example_board_readme.md)
- [TWR-KM34Z75M](../../../_boards/twrkm34z75m/driver_examples/uart/9bit_interrupt_transfer/example_board_readme.md)
- [TWR-KM35Z75M](../../../_boards/twrkm35z75m/driver_examples/uart/9bit_interrupt_transfer/example_board_readme.md)
