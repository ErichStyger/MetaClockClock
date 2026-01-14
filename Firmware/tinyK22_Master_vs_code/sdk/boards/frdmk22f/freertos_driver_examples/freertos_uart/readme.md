# freertos_uart

## Overview
The UART example for FreeRTOS demonstrates the possibility to use the UART driver in the RTOS.
The example uses single instance of UART IP and writes string into, then reads back chars.
After every 4B received, these are sent back on UART.

## Supported Boards
- [FRDM-K22F](../../_boards/frdmk22f/freertos_driver_examples/freertos_uart/example_board_readme.md)
- [FRDM-K32L2B](../../_boards/frdmk32l2b/freertos_driver_examples/freertos_uart/example_board_readme.md)
- [MIMXRT685-AUD-EVK](../../_boards/mimxrt685audevk/freertos_driver_examples/freertos_uart/example_board_readme.md)
