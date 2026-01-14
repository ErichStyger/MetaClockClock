# cmsis_lpuart_interrupt_transfer

## Overview
CMSIS-Driver defines generic peripheral driver interfaces for middleware making it reusable across a wide
range of supported microcontroller devices. The API connects microcontroller peripherals with middleware
that implements for example communication stacks, file systems, or graphic user interfaces.
More information and usage method please refer to http://www.keil.com/pack/doc/cmsis/Driver/html/index.html.

The cmsis_uart_interrupt_transfer example shows how to use uart cmsis driver in interrupt way:

In this example, one uart instance connect to PC through uart, the board will
send back all characters that PC send to the board.

Note: The example echo every 8 characters, so input 8 characters every time.

## Supported Boards
- [EVK9-MIMX8ULP](../../../_boards/evk9mimx8ulp/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [EVKB-IMXRT1050](../../../_boards/evkbimxrt1050/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [MIMXRT1060-EVKB](../../../_boards/evkbmimxrt1060/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [MIMXRT1170-EVKB](../../../_boards/evkbmimxrt1170/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [MIMXRT1060-EVKC](../../../_boards/evkcmimxrt1060/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [EVK-MCIMX7ULP](../../../_boards/evkmcimx7ulp/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [EVK-MIMX8ULP](../../../_boards/evkmimx8ulp/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [EVK-MIMXRT1010](../../../_boards/evkmimxrt1010/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [EVK-MIMXRT1015](../../../_boards/evkmimxrt1015/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [EVK-MIMXRT1020](../../../_boards/evkmimxrt1020/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [MIMXRT1024-EVK](../../../_boards/evkmimxrt1024/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [MIMXRT1040-EVK](../../../_boards/evkmimxrt1040/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [EVK-MIMXRT1064](../../../_boards/evkmimxrt1064/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [MIMXRT1160-EVK](../../../_boards/evkmimxrt1160/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [MIMXRT1180-EVK](../../../_boards/evkmimxrt1180/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [FRDM-K22F](../../../_boards/frdmk22f/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [FRDM-K32L2A4S](../../../_boards/frdmk32l2a4s/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [FRDM-K32L2B](../../../_boards/frdmk32l2b/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [FRDM-K32L3A6](../../../_boards/frdmk32l3a6/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [FRDM-KE15Z](../../../_boards/frdmke15z/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [FRDM-KE16Z](../../../_boards/frdmke16z/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [FRDM-KE17Z](../../../_boards/frdmke17z/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [FRDM-KE17Z512](../../../_boards/frdmke17z512/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [FRDM-MCXA153](../../../_boards/frdmmcxa153/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [FRDM-MCXA156](../../../_boards/frdmmcxa156/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [FRDM-MCXA346](../../../_boards/frdmmcxa346/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [FRDM-MCXC041](../../../_boards/frdmmcxc041/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [FRDM-MCXC242](../../../_boards/frdmmcxc242/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [FRDM-MCXC444](../../../_boards/frdmmcxc444/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [FRDM-MCXE247](../../../_boards/frdmmcxe247/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [FRDM-MCXN236](../../../_boards/frdmmcxn236/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [FRDM-MCXN947](../../../_boards/frdmmcxn947/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [FRDM-MCXW71](../../../_boards/frdmmcxw71/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [MCX-W71-EVK](../../../_boards/mcxw71evk/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [IMX95LP4XEVK-15](../../../_boards/imx95lp4xevk15/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [IMX95LPD5EVK-19](../../../_boards/imx95lpd5evk19/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [imx95verdinevk](../../../_boards/imx95verdinevk/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [K32W148-EVK](../../../_boards/k32w148evk/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [KW45B41Z-EVK](../../../_boards/kw45b41zevk/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [KW47-EVK](../../../_boards/kw47evk/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [MCIMX93AUTO-EVK](../../../_boards/mcimx93autoevk/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [MCIMX93-EVK](../../../_boards/mcimx93evk/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [MCIMX93-QSB](../../../_boards/mcimx93qsb/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [MCX-N5XX-EVK](../../../_boards/mcxn5xxevk/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [MCX-N9XX-EVK](../../../_boards/mcxn9xxevk/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [MCX-W72-EVK](../../../_boards/mcxw72evk/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
- [MIMXRT700-EVK](../../../_boards/mimxrt700evk/cmsis_driver_examples/lpuart/interrupt_transfer/example_board_readme.md)
