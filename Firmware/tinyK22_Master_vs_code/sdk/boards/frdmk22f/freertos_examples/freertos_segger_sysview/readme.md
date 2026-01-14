# freertos_segger_sysview

## Overview

Segger Sysview demo shows the basic capabilities of Segger System Viewer tool.
Required software:
    - J-Link Software and Documentation Pack
    - SystemView - Real-time analysis and visualization
    - J-Link OpenSDA - Board-Specific Firmwares
can be downloaded from https://www.segger.com/downloads/jlink

Demo requires JLink connected. The JLink can be external probe or on board SDA debugger.
In case of SDA debugger you have to change firmware to J-Link OpenSDA by holding board RESET 
button while power up board. The board is then detected as USB mass storage device and it's 
required to copy the .bin firmware to the device. When it's done you have to reset the board 
once more.

## Running the demo

1) Connect board default console with PC. Set the JLink as current debugger in your IDE and build the demo.
When demo is running, you should see on the terminal the address of "RTT block".
The RTT block address is location of '_SEGGER_RTT' variable.

2) Launch System Viewer tool and click "Target - Start Recording". In modal window set the target 
interface. Automatic detection might not work so switch "RTT Control Block Detection" to "address"
and put the address from step (1) into the text field.

## Supported Boards
- [EVK-MIMXRT1010](../../../_boards/evkmimxrt1010/freertos_examples/visualization/freertos_segger_sysview/example_board_readme.md)
- [EVK-MIMXRT1015](../../../_boards/evkmimxrt1015/freertos_examples/visualization/freertos_segger_sysview/example_board_readme.md)
- [FRDM-K22F](../../../_boards/frdmk22f/freertos_examples/visualization/freertos_segger_sysview/example_board_readme.md)
- [FRDM-K32L2B](../../../_boards/frdmk32l2b/freertos_examples/visualization/freertos_segger_sysview/example_board_readme.md)
- [FRDM-K32L3A6](../../../_boards/frdmk32l3a6/freertos_examples/visualization/freertos_segger_sysview/example_board_readme.md)
- [FRDM-MCXC242](../../../_boards/frdmmcxc242/freertos_examples/visualization/freertos_segger_sysview/example_board_readme.md)
- [FRDM-MCXC444](../../../_boards/frdmmcxc444/freertos_examples/visualization/freertos_segger_sysview/example_board_readme.md)
