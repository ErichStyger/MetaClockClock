# erpc_server_matrix_multiply_uart

## Overview

This example demonstrates usage of eRPC between PC and board using UART transport layer.
Board acts like a server and the PC as client.

When client starts, it generates two random matrixes and sends them to server.

Server then performs matrix multiplication and sends result data back to client.

Result matrix is then printed on the PC side.

## eRPC documentation

eRPC specific files are stored in: middleware\multicore\erpc
eRPC documentation is stored in: middleware\multicore\erpc\doc
eRPC is open-source project stored on github: https://github.com/EmbeddedRPC/erpc
eRPC documentation can be also found in: http://embeddedrpc.github.io

## PC Side Setup (Python)

1. Make sure you have Python installed on your PC
2. Install serial module by executing following command in command line: "python -m pip install pyserial"
3. Install eRPC module to Python by executing setup.py located in: middleware\multicore\erpc\erpc_python - "python setup.py install"

## Usage of run_uart.py

usage: run_uart.py [-h] [-c] [-s] [-p PORT] [-b BD]

eRPC Matrix Multiply example

optional arguments:
  -h, --help            show this help message and exit
  -c, --client          Run client
  -s, --server          Run server
  -p PORT, --port PORT  Serial port
  -b BD, --bd BD        Baud rate (default value is 115200)

Either server or client has to be selected to run

Example:
To run PC side as a client with a board connected as a server to COM3 execute:
"run_uart.py --client --port COM3"

## Building the application

This shows example how to build application for `evkbmimxrt1170` board with `cm7` core_id.
Change the `-b <board>` parameter based on board you want to build.
Change the `-Dcore_id=<core_id>` parameter based on board core you want to build.
For these parameters please see attribute `boards:` in `primary/example.yml`.

```
west build examples/multiprocessor_examples/erpc_server_matrix_multiply_uart --toolchain armgcc --config debug -b evkbmimxrt1170 -Dcore_id=cm7
```

## Supported Boards

- [MIMXRT1170-EVKB](../../_boards/evkbmimxrt1170/multiprocessor_examples/erpc_server_matrix_multiply_uart/example_board_readme.md)
- [MIMXRT1180-EVK](../../_boards/evkmimxrt1180/multiprocessor_examples/erpc_server_matrix_multiply_uart/example_board_readme.md)
- [EVK-MIMXRT685](../../_boards/evkmimxrt685/multiprocessor_examples/erpc_server_matrix_multiply_uart/example_board_readme.md)
- [FRDM-K22F](../../_boards/frdmk22f/multiprocessor_examples/erpc_server_matrix_multiply_uart/example_board_readme.md)
- [FRDM-K32L2B](../../_boards/frdmk32l2b/multiprocessor_examples/erpc_server_matrix_multiply_uart/example_board_readme.md)
- [FRDM-MCXC242](../../_boards/frdmmcxc242/multiprocessor_examples/erpc_server_matrix_multiply_uart/example_board_readme.md)
- [FRDM-MCXC444](../../_boards/frdmmcxc444/multiprocessor_examples/erpc_server_matrix_multiply_uart/example_board_readme.md)
- [FRDM-MCXN236](../../_boards/frdmmcxn236/multiprocessor_examples/erpc_server_matrix_multiply_uart/example_board_readme.md)
- [FRDM-MCXN947](../../_boards/frdmmcxn947/multiprocessor_examples/erpc_server_matrix_multiply_uart/example_board_readme.md)
- [LPCXpresso55S36](../../_boards/lpcxpresso55s36/multiprocessor_examples/erpc_server_matrix_multiply_uart/example_board_readme.md)
- [MCX-N9XX-EVK](../../_boards/mcxn9xxevk/multiprocessor_examples/erpc_server_matrix_multiply_uart/example_board_readme.md)
- [MIMXRT700-EVK](../../_boards/mimxrt700evk/multiprocessor_examples/erpc_server_matrix_multiply_uart/example_board_readme.md)
