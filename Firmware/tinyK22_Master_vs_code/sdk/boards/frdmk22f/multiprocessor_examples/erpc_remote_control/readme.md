# erpc_remote_control

## Overview

This example demonstrates usage of eRPC between PC and board using UART transport layer.
Thanks to eRPC arbitrator both the board and the PC act like server and client.

When both servers start they wait for data being sent from client over UART.
A. Server on the board performs action (DAC/ADC conversion, turn on selected LEDs, read data from magnetometer and accelerometer sensor)
and sends result data back to client (or lights LED).
B. Server on the PC displays which SW button was pressed on the board.

## eRPC documentation

eRPC specific files are stored in: middleware\multicore\erpc
eRPC documentation is stored in: middleware\multicore\erpc\doc
eRPC is open-source project stored on github: https://github.com/EmbeddedRPC/erpc
eRPC documentation can be also found in: http://embeddedrpc.github.io

## Building the application

This shows example how to build application for `frdmk32l2b` board.
Change the `-b <board>` parameter based on board you want to build.
For these parameters please see attribute `boards:` in `primary/example.yml`.

```
west build examples/multiprocessor_examples/erpc_server_dac_adc --toolchain armgcc --config debug -b frdmk32l2b
```

## Supported Boards

- [FRDM-K22F](../../_boards/frdmk22f/multiprocessor_examples/erpc_remote_control/example_board_readme.md)
- [FRDM-K32L2B](../../_boards/frdmk32l2b/multiprocessor_examples/erpc_remote_control/example_board_readme.md)
