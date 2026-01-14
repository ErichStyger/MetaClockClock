# freertos_lpuart

## Overview
This example is showing data send and receive via FreeRTOS adapted LPUART driver. Program initially send
string into serial terminal via virtual COM. After that, user may send some custom input and
application will return every 4 characters back to console. If delay from last user input exceed
10s (receive timeout) notification about exceeded timeout appear and application will finish.
Example need only single LPUART instance.

## Supported Boards
- [EVKB-IMXRT1050](../../_boards/evkbimxrt1050/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [MIMXRT1060-EVKB](../../_boards/evkbmimxrt1060/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [MIMXRT1170-EVKB](../../_boards/evkbmimxrt1170/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [EVK-MIMXRT1010](../../_boards/evkmimxrt1010/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [EVK-MIMXRT1015](../../_boards/evkmimxrt1015/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [EVK-MIMXRT1020](../../_boards/evkmimxrt1020/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [MIMXRT1024-EVK](../../_boards/evkmimxrt1024/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [MIMXRT1040-EVK](../../_boards/evkmimxrt1040/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [EVK-MIMXRT1064](../../_boards/evkmimxrt1064/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [MIMXRT1160-EVK](../../_boards/evkmimxrt1160/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [MIMXRT1180-EVK](../../_boards/evkmimxrt1180/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [FRDM-K22F](../../_boards/frdmk22f/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [FRDM-K32L2A4S](../../_boards/frdmk32l2a4s/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [FRDM-K32L2B](../../_boards/frdmk32l2b/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [FRDM-K32L3A6](../../_boards/frdmk32l3a6/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [FRDM-KE15Z](../../_boards/frdmke15z/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [FRDM-KE17Z](../../_boards/frdmke17z/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [KW45B41Z-LOC](../../_boards/kw45b41zloc/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [KW47-EVK](../../_boards/kw47evk/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [MCX-N9XX-EVK](../../_boards/mcxn9xxevk/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [MCX-W72-EVK](../../_boards/mcxw72evk/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [TWR-KM34Z75M](../../_boards/twrkm34z75m/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
- [TWR-KM35Z75M](../../_boards/twrkm35z75m/freertos_driver_examples/freertos_lpuart/example_board_readme.md)
