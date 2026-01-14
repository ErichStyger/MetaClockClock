# ewm

## Overview
The EWM Example project is to demonstrate usage of the KSDK EWM driver.
In the example, EWM counter is continuously refreshed until button is pressed.
Once the button is pressed, EWM counter will expire and interrupt will be generated.
After the first pressing, another interrupt can be triggered by pressing button again.

Note: In flash boot mode, if you want to use the IDE reset program, please use systemReset.

## Supported Boards
- [EVK9-MIMX8ULP](../../_boards/evk9mimx8ulp/driver_examples/ewm/example_board_readme.md)
- [EVKB-IMXRT1050](../../_boards/evkbimxrt1050/driver_examples/ewm/example_board_readme.md)
- [MIMXRT1060-EVKB](../../_boards/evkbmimxrt1060/driver_examples/ewm/example_board_readme.md)
- [MIMXRT1170-EVKB](../../_boards/evkbmimxrt1170/driver_examples/ewm/example_board_readme.md)
- [MIMXRT1060-EVKC](../../_boards/evkcmimxrt1060/driver_examples/ewm/example_board_readme.md)
- [EVK-MCIMX7ULP](../../_boards/evkmcimx7ulp/driver_examples/ewm/example_board_readme.md)
- [EVK-MIMX8ULP](../../_boards/evkmimx8ulp/driver_examples/ewm/example_board_readme.md)
- [EVK-MIMXRT1010](../../_boards/evkmimxrt1010/driver_examples/ewm/example_board_readme.md)
- [EVK-MIMXRT1015](../../_boards/evkmimxrt1015/driver_examples/ewm/example_board_readme.md)
- [EVK-MIMXRT1020](../../_boards/evkmimxrt1020/driver_examples/ewm/example_board_readme.md)
- [MIMXRT1024-EVK](../../_boards/evkmimxrt1024/driver_examples/ewm/example_board_readme.md)
- [MIMXRT1040-EVK](../../_boards/evkmimxrt1040/driver_examples/ewm/example_board_readme.md)
- [EVK-MIMXRT1064](../../_boards/evkmimxrt1064/driver_examples/ewm/example_board_readme.md)
- [MIMXRT1160-EVK](../../_boards/evkmimxrt1160/driver_examples/ewm/example_board_readme.md)
- [MIMXRT1180-EVK](../../_boards/evkmimxrt1180/driver_examples/ewm/example_board_readme.md)
- [FRDM-K22F](../../_boards/frdmk22f/driver_examples/ewm/example_board_readme.md)
- [FRDM-K32L3A6](../../_boards/frdmk32l3a6/driver_examples/ewm/example_board_readme.md)
- [FRDM-KE15Z](../../_boards/frdmke15z/driver_examples/ewm/example_board_readme.md)
- [FRDM-KE16Z](../../_boards/frdmke16z/driver_examples/ewm/example_board_readme.md)
- [FRDM-KE17Z](../../_boards/frdmke17z/driver_examples/ewm/example_board_readme.md)
- [FRDM-KE17Z512](../../_boards/frdmke17z512/driver_examples/ewm/example_board_readme.md)
- [FRDM-MCXE247](../../_boards/frdmmcxe247/driver_examples/ewm/example_board_readme.md)
- [FRDM-MCXN236](../../_boards/frdmmcxn236/driver_examples/ewm/example_board_readme.md)
- [FRDM-MCXN947](../../_boards/frdmmcxn947/driver_examples/ewm/example_board_readme.md)
- [FRDM-MCXW71](../../_boards/frdmmcxw71/driver_examples/ewm/example_board_readme.md)
- [MCX-W71-EVK](../../_boards/mcxw71evk/driver_examples/ewm/example_board_readme.md)
- [K32W148-EVK](../../_boards/k32w148evk/driver_examples/ewm/example_board_readme.md)
- [KW45B41Z-EVK](../../_boards/kw45b41zevk/driver_examples/ewm/example_board_readme.md)
- [KW47-EVK](../../_boards/kw47evk/driver_examples/ewm/example_board_readme.md)
- [MCX-N5XX-EVK](../../_boards/mcxn5xxevk/driver_examples/ewm/example_board_readme.md)
- [MCX-N9XX-EVK](../../_boards/mcxn9xxevk/driver_examples/ewm/example_board_readme.md)
- [MCX-W72-EVK](../../_boards/mcxw72evk/driver_examples/ewm/example_board_readme.md)
- [TWR-KM34Z50MV3](../../_boards/twrkm34z50mv3/driver_examples/ewm/example_board_readme.md)
- [TWR-KM34Z75M](../../_boards/twrkm34z75m/driver_examples/ewm/example_board_readme.md)
- [TWR-KM35Z75M](../../_boards/twrkm35z75m/driver_examples/ewm/example_board_readme.md)
