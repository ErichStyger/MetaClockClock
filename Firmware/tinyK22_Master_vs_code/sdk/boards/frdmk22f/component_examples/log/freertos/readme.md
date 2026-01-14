# log_freertos

## Overview
The log Demo application demonstrates to prompt LOG with level by commands.
Note: Please input one character at a time. If you input too many characters each time, the receiver may overflow
because the low level UART uses simple polling way for receiving. If you want to try inputting many characters each time,
just define DEBUG_CONSOLE_TRANSFER_NON_BLOCKING in your project to use the advanced debug console utility.
Besides, the demo does not support semihosting mode. The log demo is based on shell, debug console and
serial manager. When semihosting is used, debug console and serial manager are bypassed. So the log demo cannot
work with semihosting.

## Supported Boards
- [EVKB-IMXRT1050](../../../_boards/evkbimxrt1050/component_examples/log/freertos/example_board_readme.md)
- [MIMXRT1060-EVKB](../../../_boards/evkbmimxrt1060/component_examples/log/freertos/example_board_readme.md)
- [MIMXRT1170-EVKB](../../../_boards/evkbmimxrt1170/component_examples/log/freertos/example_board_readme.md)
- [MIMXRT1060-EVKC](../../../_boards/evkcmimxrt1060/component_examples/log/freertos/example_board_readme.md)
- [EVK-MIMXRT1010](../../../_boards/evkmimxrt1010/component_examples/log/freertos/example_board_readme.md)
- [EVK-MIMXRT1015](../../../_boards/evkmimxrt1015/component_examples/log/freertos/example_board_readme.md)
- [EVK-MIMXRT1020](../../../_boards/evkmimxrt1020/component_examples/log/freertos/example_board_readme.md)
- [MIMXRT1024-EVK](../../../_boards/evkmimxrt1024/component_examples/log/freertos/example_board_readme.md)
- [MIMXRT1040-EVK](../../../_boards/evkmimxrt1040/component_examples/log/freertos/example_board_readme.md)
- [EVK-MIMXRT1064](../../../_boards/evkmimxrt1064/component_examples/log/freertos/example_board_readme.md)
- [MIMXRT1160-EVK](../../../_boards/evkmimxrt1160/component_examples/log/freertos/example_board_readme.md)
- [MIMXRT1180-EVK](../../../_boards/evkmimxrt1180/component_examples/log/freertos/example_board_readme.md)
- [EVK-MIMXRT595](../../../_boards/evkmimxrt595/component_examples/log/freertos/example_board_readme.md)
- [EVK-MIMXRT685](../../../_boards/evkmimxrt685/component_examples/log/freertos/example_board_readme.md)
- [FRDM-K22F](../../../_boards/frdmk22f/component_examples/log/freertos/example_board_readme.md)
- [FRDM-K32L2A4S](../../../_boards/frdmk32l2a4s/component_examples/log/freertos/example_board_readme.md)
- [FRDM-K32L2B](../../../_boards/frdmk32l2b/component_examples/log/freertos/example_board_readme.md)
- [FRDM-K32L3A6](../../../_boards/frdmk32l3a6/component_examples/log/freertos/example_board_readme.md)
- [FRDM-KE15Z](../../../_boards/frdmke15z/component_examples/log/freertos/example_board_readme.md)
- [FRDM-KE17Z](../../../_boards/frdmke17z/component_examples/log/freertos/example_board_readme.md)
- [FRDM-KE17Z512](../../../_boards/frdmke17z512/component_examples/log/freertos/example_board_readme.md)
- [FRDM-MCXC242](../../../_boards/frdmmcxc242/component_examples/log/freertos/example_board_readme.md)
- [FRDM-MCXC444](../../../_boards/frdmmcxc444/component_examples/log/freertos/example_board_readme.md)
- [FRDM-MCXW71](../../../_boards/frdmmcxw71/component_examples/log/freertos/example_board_readme.md)
- [MCX-W71-EVK](../../../_boards/mcxw71evk/component_examples/log/freertos/example_board_readme.md)
- [K32W148-EVK](../../../_boards/k32w148evk/component_examples/log/freertos/example_board_readme.md)
- [KW45B41Z-EVK](../../../_boards/kw45b41zevk/component_examples/log/freertos/example_board_readme.md)
- [KW45B41Z-LOC](../../../_boards/kw45b41zloc/component_examples/log/freertos/example_board_readme.md)
- [KW47-EVK](../../../_boards/kw47evk/component_examples/log/freertos/example_board_readme.md)
- [LPCXpresso51U68](../../../_boards/lpcxpresso51u68/component_examples/log/freertos/example_board_readme.md)
- [LPCXpresso54628](../../../_boards/lpcxpresso54628/component_examples/log/freertos/example_board_readme.md)
- [LPCXpresso54S018](../../../_boards/lpcxpresso54s018/component_examples/log/freertos/example_board_readme.md)
- [LPCXpresso54S018M](../../../_boards/lpcxpresso54s018m/component_examples/log/freertos/example_board_readme.md)
- [LPCXpresso55S16](../../../_boards/lpcxpresso55s16/component_examples/log/freertos/example_board_readme.md)
- [LPCXpresso55S28](../../../_boards/lpcxpresso55s28/component_examples/log/freertos/example_board_readme.md)
- [LPCXpresso55S69](../../../_boards/lpcxpresso55s69/component_examples/log/freertos/example_board_readme.md)
- [MCX-W72-EVK](../../../_boards/mcxw72evk/component_examples/log/freertos/example_board_readme.md)
- [MIMXRT685-AUD-EVK](../../../_boards/mimxrt685audevk/component_examples/log/freertos/example_board_readme.md)
- [MIMXRT700-EVK](../../../_boards/mimxrt700evk/component_examples/log/freertos/example_board_readme.md)
- [TWR-KM34Z50MV3](../../../_boards/twrkm34z50mv3/component_examples/log/freertos/example_board_readme.md)
- [TWR-KM34Z75M](../../../_boards/twrkm34z75m/component_examples/log/freertos/example_board_readme.md)
- [TWR-KM35Z75M](../../../_boards/twrkm35z75m/component_examples/log/freertos/example_board_readme.md)
