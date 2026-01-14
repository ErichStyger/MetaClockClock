# erpc_server_matrix_multiply_spi

## Overview

This example demonstrates usage of eRPC between two boards using SPI transport layer.
One board acts like a server and the second as client.
When client starts, it generates two random matrixes and sends them to server.
Server then performs matrix multiplication and sends result data back to client.
Client then prints the result matrix.

Optional HW setup is to use the PC as the eRPC client and the board as the eRPC server.
In this case the LIBUSBSIOSPITransport (Python) is used on the PC side and the SPI slave transport on the board side.

The libusbsio Python module is used for accessing the NXP libusbsio APIs and thus allowing to exercise SPI, I2C bus and GPIO pins
from Python application (over USBSIO interface of NXP LPCLink2 and MCUlink Pro devices).

This configuration is supported for selected boards only. Note, that when more than one board with LPCLink2/MCUlink devices is
connected to the PC the Python application is not able to automatically detect which one to use to establish the communication.
Thus, please ensure only one board with running eRPC server application is connected to the PC.
When starting the Python application on the PC side or use the -d command line argument of the Python application to specify
the targeted LPCLink2/MCUlink device index.

## eRPC documentation

eRPC specific files are stored in: middleware\multicore\erpc
eRPC documentation is stored in: middleware\multicore\erpc\doc
eRPC is open-source project stored on github: https://github.com/EmbeddedRPC/erpc
eRPC documentation can be also found in: http://embeddedrpc.github.io

## PC Side Setup (Optional for PC-to-board communication)
1. Make sure you have Python installed on your PC
2. Install serial module by executing following command in command line: "python -m pip install pyserial"
3. Install eRPC module to Python by executing setup.py located in: middleware\multicore\erpc\erpc_python - "python setup.py install"
4. Install libusbsio module to Python by executing following command in command line: "python -m pip install libusbsio"

usage: run_spi.py [-h] [-b BD] [-g CS_PORT] [-p CS_PIN] [-d DEVIDX]

eRPC Matrix Multiply example

optional arguments:
  -h, --help                    show this help message and exit
  -b BD, --bd BD                Baud rate (default value is 500000)
  -g CS_PORT, --csport CS_PORT  Chip select GPIO port number (see the board schematics)
  -p CS_PIN, --cspin CS_PIN     Chip select GPIO pin number (see the board schematics)
  -d DEVIDX, --devidx DEVIDX    LIBUSBSIO device index (default value is 0 - only one board connected to the host PC)

Example:
To run PC side as a client with a board connected as a server:
"run_spi.py --bd 500000 --csport 0 --cspin 15"

## Building the application

This shows example how to build application for `frdmmcxn947` board with `cm33_core0` core_id.
Change the `-b <board>` parameter based on board you want to build.
Change the `-Dcore_id=<core_id>` parameter based on board core you want to build.
For these parameters please see attribute `boards:` in `primary/example.yml`.

```
west build examples/multiprocessor_examples/erpc_server_matrix_multiply_spi --toolchain armgcc --config debug -b frdmmcxn947 -Dcore_id=cm33_core0
```

## Supported Boards
- [FRDM-K22F](../../_boards/frdmk22f/multiprocessor_examples/erpc_server_matrix_multiply_spi/example_board_readme.md)
- [FRDM-K32L2B](../../_boards/frdmk32l2b/multiprocessor_examples/erpc_server_matrix_multiply_spi/example_board_readme.md)
- [FRDM-MCXN236](../../_boards/frdmmcxn236/multiprocessor_examples/erpc_server_matrix_multiply_spi/example_board_readme.md)
- [FRDM-MCXN947](../../_boards/frdmmcxn947/multiprocessor_examples/erpc_server_matrix_multiply_spi/example_board_readme.md)
- [LPCXpresso55S36](../../_boards/lpcxpresso55s36/multiprocessor_examples/erpc_server_matrix_multiply_spi/example_board_readme.md)
- [MCX-N9XX-EVK](../../_boards/mcxn9xxevk/multiprocessor_examples/erpc_server_matrix_multiply_spi/example_board_readme.md)
- [MIMXRT700-EVK](../../_boards/mimxrt700evk/multiprocessor_examples/erpc_server_matrix_multiply_spi/example_board_readme.md)
