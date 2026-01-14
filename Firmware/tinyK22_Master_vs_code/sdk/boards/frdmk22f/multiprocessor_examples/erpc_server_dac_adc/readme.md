# erpc_server_dac_adc

## Overview

This example demonstrates usage of eRPC between PC and board using UART transport layer.
Board acts like a server and the PC as client.
When server starts, it waits for data being send from client over UART.
Server then performs action (DAC/ADC conversion, light LEDs, read data from
magnetometer sensor) and sends result data back to client (or lights LED).

## eRPC documentation

eRPC specific files are stored in: middleware\multicore\erpc
eRPC documentation is stored in: middleware\multicore\erpc\doc
eRPC is open-source project stored on github: https://github.com/EmbeddedRPC/erpc
eRPC documentation can be also found in: http://embeddedrpc.github.io

## Running with i.MX7D Sabre SD board

- First, download L4.1.15_2.0.0_iMX6UL7D board support package from nxp.com
- Then uncompress it and locate file fsl-image-validation-imx-imx7dsabresd.sdcard
- Insert an SD card and run: sudo dd if=fsl-image-validation-imx-imx7dsabresd.sdcard
of=/dev/mmcblk0 bs=1M
- Flush disk IO operations, run command: sync
- Download enum34, erpc and pyserial packages from pypi, uncompress them and copy to
/home/root, respective links are:
		- enum34: https://pypi.python.org/packages/bf/3e/31d502c25302814a7c2f1d3959d2a3b3f78e509002ba91aea64993936876/enum34-1.1.6.tar.gz#md5=5f13a0841a61f7fc295c514490d120d0
		- erpc: https://pypi.python.org/packages/fd/0d/64425e9d5258fa5ea9cef7de879e3ae8be6c0d50a7e215ba791d0a32d77e/erpc--1.4.0.tar.gz#md5=1387082dab3af4067fdfe01c98b09dee
		- pyserial: https://pypi.python.org/packages/df/c9/d9da7fafaf2a2b323d20eee050503ab08237c16b0119c7bbf1597d53f793/pyserial-2.7.tar.gz#md5=794506184df83ef2290de0d18803dd11
- Insert SD card in the board, connect to USB debug console
- Once you boot and login (user root), enter enum34, erpc and pyserial directories
and run in each of them: python setup.py install
- now you can use: python run.py -p <PORT> -b <BAUDRATE>, to run the example

## Building the application

This shows example how to build application for `frdmk32l2b` board.
Change the `-b <board>` parameter based on board you want to build.
For these parameters please see attribute `boards:` in `primary/example.yml`.

```
west build examples/multiprocessor_examples/erpc_server_dac_adc --toolchain armgcc --config debug -b frdmk32l2b
```

## Supported Boards

- [FRDM-K22F](../../_boards/frdmk22f/multiprocessor_examples/erpc_server_dac_adc/example_board_readme.md)
- [FRDM-K32L2B](../../_boards/frdmk32l2b/multiprocessor_examples/erpc_server_dac_adc/example_board_readme.md)
