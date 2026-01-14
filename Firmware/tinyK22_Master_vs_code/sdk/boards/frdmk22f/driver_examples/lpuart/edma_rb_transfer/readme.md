# lpuart_edma_rb_transfer

## Overview
The lpuart_edma Ring Buffer Example project is to demonstrate usage of the KSDK lpuart driver.
In the example, a ring buffer will be used with EDMA enabled, data received will be stored in ring 
buffer first, while IDLE line was detected, routine will read out the received characters, and echo back
them to the terminal by using EDMA mode. so, you can see the your pressed characters on the terminal.

## Supported Boards
- [EVKB-IMXRT1050](../../../_boards/evkbimxrt1050/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [MIMXRT1060-EVKB](../../../_boards/evkbmimxrt1060/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [MIMXRT1060-EVKC](../../../_boards/evkcmimxrt1060/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [EVK-MIMXRT1015](../../../_boards/evkmimxrt1015/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [EVK-MIMXRT1020](../../../_boards/evkmimxrt1020/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [MIMXRT1024-EVK](../../../_boards/evkmimxrt1024/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [MIMXRT1040-EVK](../../../_boards/evkmimxrt1040/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [EVK-MIMXRT1064](../../../_boards/evkmimxrt1064/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [FRDM-K22F](../../../_boards/frdmk22f/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [FRDM-K32L2A4S](../../../_boards/frdmk32l2a4s/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [FRDM-KE15Z](../../../_boards/frdmke15z/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [FRDM-KE17Z](../../../_boards/frdmke17z/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [FRDM-KE17Z512](../../../_boards/frdmke17z512/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [FRDM-MCXE247](../../../_boards/frdmmcxe247/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [FRDM-MCXW71](../../../_boards/frdmmcxw71/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [MCX-W71-EVK](../../../_boards/mcxw71evk/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [K32W148-EVK](../../../_boards/k32w148evk/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [KW45B41Z-EVK](../../../_boards/kw45b41zevk/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [KW47-EVK](../../../_boards/kw47evk/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [MCX-W72-EVK](../../../_boards/mcxw72evk/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
- [MIMXRT700-EVK](../../../_boards/mimxrt700evk/driver_examples/lpuart/edma_rb_transfer/example_board_readme.md)
