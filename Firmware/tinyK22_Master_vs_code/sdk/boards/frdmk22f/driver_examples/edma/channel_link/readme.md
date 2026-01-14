# edma_channel_link

## Overview
The EDMA channel link example is a simple demonstration program that uses the SDK software.
It excuates channel link transfer using the SDK EDMA drivers.
The purpose of this example is to show how to use the EDMA and to provide a simple example for
debugging and further development, it demostrates how to use the minor loop link and major loop link:
Since each transfer request can be divided into MAJOR_LOOPS_COUNTS * MINOR_LOOPS_BYTES,
such as you want to request DMA transfer 8 bytes total, 4 bytes each request, then MAJOR_LOOPS_COUNTS = 2, MINOR_LOOPS_BYTES = 4.
The minor loop channel linking occurs at the completion of the minor loop 4 byte transferred.
The major loop channel linking is occurs at the major loop exhausted.
The example demostrate the channel link transfer by the feature of edma:
1. minor loop channel link
                                                                                                                   +-----------------------------+                          +----------------------+
                                                                                                                   | channel 1 transfer complete | -----------------------> |   example complete   |
                                                                                                                   +-----------------------------+                          +----------------------+
                                                                                                                     ^
                                                                                                                     | major loop finished
                                                                                                                     |
                        +-----------------------------+     +-------------------------------+  trigger channel 1   +-----------------------------+  major loop not finish   +----------------------+     +-------------------------------+
                        |    channel 0 minor loop     | --> | channel 0 minor loop finished | -------------------> |    channel 1 major loop     | -----------------------> | channel 1 minor loop | --> | channel 1 minor loop finished |
                        +-----------------------------+     +-------------------------------+                      +-----------------------------+                          +----------------------+     +-------------------------------+
                          ^                                                                                                                                                                                |
                          | major loop not finish                                                                                                                                                          |
                          |                                                                                                                                                                                |
+-----------------+     +-----------------------------+      trigger channel 0                                                                                                                             |
| channel 0 start | --> |    channel 0 major loop     | <--------------------------------------------------------------------------------------------------------------------------------------------------+
+-----------------+     +-----------------------------+
                          |
                          | major loop finished
                          v
                        +-----------------------------+
                        | channel 0 transfer complete |
                        +-----------------------------+

2. major loop channel link

                         channel 1 minor loop finished
  +---------------------------------------------------------+
  |                                                         v
+----------------------+  major loop not finish           +-------------------------------+  major loop finished     +-----------------------------+     +-------------------------------+
| channel 1 minor loop | <------------------------------- |     channel 1 major loop      | -----------------------> | channel 1 transfer complete | --> |       example complete        |
+----------------------+                                  +-------------------------------+                          +-----------------------------+     +-------------------------------+
                                                            ^
                                                            | trigger channel 1
                                                            |
                                                          +-------------------------------+                          +-----------------------------+
                                                          | channel 0 major loop finished | -----------------------> | channel 0 transfer complete |
                                                          +-------------------------------+                          +-----------------------------+
                                                            ^
                                                            |
                                                            |
+----------------------+                                  +-------------------------------+  major loop not finish   +-----------------------------+     +-------------------------------+
|   channel 0 start    | -------------------------------> |     channel 0 major loop      | -----------------------> |    channel 0 minor loop     | --> | channel 0 minor loop finished |
+----------------------+                                  +-------------------------------+                          +-----------------------------+     +-------------------------------+
                                                            ^                                                                                              |
                                                            +----------------------------------------------------------------------------------------------+


Please reference user manual for the detail of the feature.

## Supported Boards
- [EVKB-IMXRT1050](../../../_boards/evkbimxrt1050/driver_examples/edma/channel_link/example_board_readme.md)
- [MIMXRT1060-EVKB](../../../_boards/evkbmimxrt1060/driver_examples/edma/channel_link/example_board_readme.md)
- [MIMXRT1170-EVKB](../../../_boards/evkbmimxrt1170/driver_examples/edma/channel_link/example_board_readme.md)
- [MIMXRT1060-EVKC](../../../_boards/evkcmimxrt1060/driver_examples/edma/channel_link/example_board_readme.md)
- [EVK-MIMXRT1010](../../../_boards/evkmimxrt1010/driver_examples/edma/channel_link/example_board_readme.md)
- [EVK-MIMXRT1015](../../../_boards/evkmimxrt1015/driver_examples/edma/channel_link/example_board_readme.md)
- [EVK-MIMXRT1020](../../../_boards/evkmimxrt1020/driver_examples/edma/channel_link/example_board_readme.md)
- [MIMXRT1024-EVK](../../../_boards/evkmimxrt1024/driver_examples/edma/channel_link/example_board_readme.md)
- [MIMXRT1040-EVK](../../../_boards/evkmimxrt1040/driver_examples/edma/channel_link/example_board_readme.md)
- [EVK-MIMXRT1064](../../../_boards/evkmimxrt1064/driver_examples/edma/channel_link/example_board_readme.md)
- [MIMXRT1160-EVK](../../../_boards/evkmimxrt1160/driver_examples/edma/channel_link/example_board_readme.md)
- [FRDM-K22F](../../../_boards/frdmk22f/driver_examples/edma/channel_link/example_board_readme.md)
- [FRDM-K32L2A4S](../../../_boards/frdmk32l2a4s/driver_examples/edma/channel_link/example_board_readme.md)
- [FRDM-K32L3A6](../../../_boards/frdmk32l3a6/driver_examples/edma/channel_link/example_board_readme.md)
- [FRDM-KE15Z](../../../_boards/frdmke15z/driver_examples/edma/channel_link/example_board_readme.md)
- [FRDM-KE17Z](../../../_boards/frdmke17z/driver_examples/edma/channel_link/example_board_readme.md)
- [FRDM-KE17Z512](../../../_boards/frdmke17z512/driver_examples/edma/channel_link/example_board_readme.md)
- [FRDM-MCXE247](../../../_boards/frdmmcxe247/driver_examples/edma/channel_link/example_board_readme.md)
