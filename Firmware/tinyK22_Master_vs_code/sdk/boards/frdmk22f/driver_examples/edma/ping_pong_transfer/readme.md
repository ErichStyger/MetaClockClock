# edma_ping_pong_transfer

## Overview
The EDMA ping pong transfer example is a simple demonstration program that uses the SDK software.
It excuates ping pong transfer from source buffer to destination buffer using the SDK EDMA drivers.
The purpose of this example is to show how to use ping pong buffer by the EDMA and to provide a simple example for
debugging and further development.
The example demostrate the ping pong transfer by the EDMA scatter gather feature.
Please reference user manual for the detail of the feature.
                                                                                                                                                                     minor loop finished
                                                                                                                                 +--------------------------------------------------------------+
                                                                                                                                 v                                                              |
+-------+     +---------------------------------------------------------------+     +---------------------------+  load tcd1   +-----------------------------------+  major loop not finish   +------------------+
| start | --> |                       tcd 0 major loop                        | --> | tcd 0 major loop finished | -----------> |         tcd 1 major loop          | -----------------------> | tcd 1 minor loop |
+-------+     +---------------------------------------------------------------+     +---------------------------+              +-----------------------------------+                          +------------------+
                |                         ^                       ^                                                              |
                | major loop not finish   | minor loop finished   | load tcd 0                                                   |
                v                         |                       |                                                              v
              +------------------------+  |                       |                                                            +-----------------------------------+
              |    tcd 0 minor loop    | -+                       +----------------------------------------------------------- |     tcd 1 major loop finished     |
              +------------------------+                                                                                       +-----------------------------------+
                                                                                                                                 |
                                                                                                                                 |
                                                                                                                                 v
                                                                                                                               +-----------------------------------+
                                                                                                                               |        transfer counter ++        |
                                                                                                                               +-----------------------------------+
                                                                                                                                 |
                                                                                                                                 | transfer counter not less than 5
                                                                                                                                 v
                                                                                                                               +-----------------------------------+
                                                                                                                               |         example complete          |
                                                                                                                               +-----------------------------------+

## Supported Boards
- [EVKB-IMXRT1050](../../../_boards/evkbimxrt1050/driver_examples/edma/ping_pong_transfer/example_board_readme.md)
- [MIMXRT1060-EVKB](../../../_boards/evkbmimxrt1060/driver_examples/edma/ping_pong_transfer/example_board_readme.md)
- [MIMXRT1170-EVKB](../../../_boards/evkbmimxrt1170/driver_examples/edma/ping_pong_transfer/example_board_readme.md)
- [MIMXRT1060-EVKC](../../../_boards/evkcmimxrt1060/driver_examples/edma/ping_pong_transfer/example_board_readme.md)
- [EVK-MIMXRT1010](../../../_boards/evkmimxrt1010/driver_examples/edma/ping_pong_transfer/example_board_readme.md)
- [EVK-MIMXRT1015](../../../_boards/evkmimxrt1015/driver_examples/edma/ping_pong_transfer/example_board_readme.md)
- [EVK-MIMXRT1020](../../../_boards/evkmimxrt1020/driver_examples/edma/ping_pong_transfer/example_board_readme.md)
- [MIMXRT1024-EVK](../../../_boards/evkmimxrt1024/driver_examples/edma/ping_pong_transfer/example_board_readme.md)
- [MIMXRT1040-EVK](../../../_boards/evkmimxrt1040/driver_examples/edma/ping_pong_transfer/example_board_readme.md)
- [EVK-MIMXRT1064](../../../_boards/evkmimxrt1064/driver_examples/edma/ping_pong_transfer/example_board_readme.md)
- [MIMXRT1160-EVK](../../../_boards/evkmimxrt1160/driver_examples/edma/ping_pong_transfer/example_board_readme.md)
- [FRDM-K22F](../../../_boards/frdmk22f/driver_examples/edma/ping_pong_transfer/example_board_readme.md)
- [FRDM-K32L2A4S](../../../_boards/frdmk32l2a4s/driver_examples/edma/ping_pong_transfer/example_board_readme.md)
- [FRDM-K32L3A6](../../../_boards/frdmk32l3a6/driver_examples/edma/ping_pong_transfer/example_board_readme.md)
- [FRDM-KE15Z](../../../_boards/frdmke15z/driver_examples/edma/ping_pong_transfer/example_board_readme.md)
- [FRDM-KE17Z](../../../_boards/frdmke17z/driver_examples/edma/ping_pong_transfer/example_board_readme.md)
- [FRDM-KE17Z512](../../../_boards/frdmke17z512/driver_examples/edma/ping_pong_transfer/example_board_readme.md)
- [FRDM-MCXE247](../../../_boards/frdmmcxe247/driver_examples/edma/ping_pong_transfer/example_board_readme.md)
