# freertos_tickless

## Overview
This document explains the freertos_tickless example. It shows how the CPU enters the sleep mode and then
it is woken up either by expired time delay using low power timer module or by external interrupt caused by a
user defined button.

## Prepare the Demo
Connect a serial cable from the debug UART port of the board to the PC. Start Tera Term
(http://ttssh2.osdn.jp) and make a connection to the virtual serial port.

1. Start Tera Term
2. New connection -> Serial
3. Set appropriate COMx port (x is port number) in Port context menu. Number is provided by operation
   system and could be different from computer to computer. Select COM number related to virtual
   serial port. Confirm selected port by OK button.
4. Set following connection parameters in menu Setup->Serial port...
        Baud rate:    115200
        Data:         8
        Parity:       none
        Stop:         1
        Flow control: none
5.  Confirm selected parameters by OK button.

## Running the demo
After flashing the example to the board the console will start printing the number of tick count periodically
when the CPU is running.
To wake up the CPU by external interrupt, press the button, that is specified at the beginning of the
example (SWx where x is the number of the user defined button). When the button is pressed, the console prints
out the "CPU woken up by external interrupt" message.

Example output:

Press any key to start the example
Tickless Demo example
Press or turn on SWx to wake up the CPU

Tick count :
0
5000
10000
CPU woken up by external interrupt
15000
20000
25000

## Explanation of the example
The example application prints the actual tick count number every time after the specified
delay. When the vTaskDelay() is called, the CPU enters the sleep mode for that defined period
of time.

While the CPU is in sleep mode and the user defined button is pressed, the CPU is woken up
by the external interrupt and continues to sleep after the interrupt is handled.
The period of time delay is not changed after the external interrupt occurs.

## Supported Boards
- [EVKB-IMXRT1050](../../_boards/evkbimxrt1050/freertos_examples/freertos_tickless/example_board_readme.md)
- [MIMXRT1060-EVKB](../../_boards/evkbmimxrt1060/freertos_examples/freertos_tickless/example_board_readme.md)
- [MIMXRT1170-EVKB](../../_boards/evkbmimxrt1170/freertos_examples/freertos_tickless/example_board_readme.md)
- [MIMXRT1060-EVKC](../../_boards/evkcmimxrt1060/freertos_examples/freertos_tickless/example_board_readme.md)
- [EVK-MCIMX7ULP](../../_boards/evkmcimx7ulp/freertos_examples/freertos_tickless/example_board_readme.md)
- [EVK-MIMX8MM](../../_boards/evkmimx8mm/freertos_examples/freertos_tickless/example_board_readme.md)
- [EVK-MIMX8MN](../../_boards/evkmimx8mn/freertos_examples/freertos_tickless/example_board_readme.md)
- [EVK-MIMX8MNDDR3L](../../_boards/evkmimx8mnddr3l/freertos_examples/freertos_tickless/example_board_readme.md)
- [EVK-MIMX8MP](../../_boards/evkmimx8mp/freertos_examples/freertos_tickless/example_board_readme.md)
- [EVK-MIMX8MQ](../../_boards/evkmimx8mq/freertos_examples/freertos_tickless/example_board_readme.md)
- [EVK-MIMXRT1010](../../_boards/evkmimxrt1010/freertos_examples/freertos_tickless/example_board_readme.md)
- [EVK-MIMXRT1015](../../_boards/evkmimxrt1015/freertos_examples/freertos_tickless/example_board_readme.md)
- [EVK-MIMXRT1020](../../_boards/evkmimxrt1020/freertos_examples/freertos_tickless/example_board_readme.md)
- [MIMXRT1024-EVK](../../_boards/evkmimxrt1024/freertos_examples/freertos_tickless/example_board_readme.md)
- [MIMXRT1040-EVK](../../_boards/evkmimxrt1040/freertos_examples/freertos_tickless/example_board_readme.md)
- [EVK-MIMXRT1064](../../_boards/evkmimxrt1064/freertos_examples/freertos_tickless/example_board_readme.md)
- [MIMXRT1160-EVK](../../_boards/evkmimxrt1160/freertos_examples/freertos_tickless/example_board_readme.md)
- [FRDM-K22F](../../_boards/frdmk22f/freertos_examples/freertos_tickless/example_board_readme.md)
- [FRDM-K32L2A4S](../../_boards/frdmk32l2a4s/freertos_examples/freertos_tickless/example_board_readme.md)
- [FRDM-K32L2B](../../_boards/frdmk32l2b/freertos_examples/freertos_tickless/example_board_readme.md)
- [FRDM-K32L3A6](../../_boards/frdmk32l3a6/freertos_examples/freertos_tickless/example_board_readme.md)
- [FRDM-KE15Z](../../_boards/frdmke15z/freertos_examples/freertos_tickless/example_board_readme.md)
- [FRDM-KE16Z](../../_boards/frdmke16z/freertos_examples/freertos_tickless/example_board_readme.md)
- [FRDM-KE17Z](../../_boards/frdmke17z/freertos_examples/freertos_tickless/example_board_readme.md)
- [FRDM-KE17Z512](../../_boards/frdmke17z512/freertos_examples/freertos_tickless/example_board_readme.md)
- [FRDM-MCXA346](../../_boards/frdmmcxa346/freertos_examples/freertos_tickless/example_board_readme.md)
- [FRDM-MCXC242](../../_boards/frdmmcxc242/freertos_examples/freertos_tickless/example_board_readme.md)
- [FRDM-MCXC444](../../_boards/frdmmcxc444/freertos_examples/freertos_tickless/example_board_readme.md)
- [FRDM-MCXN236](../../_boards/frdmmcxn236/freertos_examples/freertos_tickless/example_board_readme.md)
- [FRDM-MCXN947](../../_boards/frdmmcxn947/freertos_examples/freertos_tickless/example_board_readme.md)
- [FRDM-MCXW71](../../_boards/frdmmcxw71/freertos_examples/freertos_tickless/example_board_readme.md)
- [MCX-W71-EVK](../../_boards/mcxw71evk/freertos_examples/freertos_tickless/example_board_readme.md)
- [K32W148-EVK](../../_boards/k32w148evk/freertos_examples/freertos_tickless/example_board_readme.md)
- [KW45B41Z-EVK](../../_boards/kw45b41zevk/freertos_examples/freertos_tickless/example_board_readme.md)
- [KW45B41Z-LOC](../../_boards/kw45b41zloc/freertos_examples/freertos_tickless/example_board_readme.md)
- [KW47-EVK](../../_boards/kw47evk/freertos_examples/freertos_tickless/example_board_readme.md)
- [MCX-N5XX-EVK](../../_boards/mcxn5xxevk/freertos_examples/freertos_tickless/example_board_readme.md)
- [MCX-N9XX-EVK](../../_boards/mcxn9xxevk/freertos_examples/freertos_tickless/example_board_readme.md)
- [MCX-W72-EVK](../../_boards/mcxw72evk/freertos_examples/freertos_tickless/example_board_readme.md)
- [TWR-KM34Z50MV3](../../_boards/twrkm34z50mv3/freertos_examples/freertos_tickless/example_board_readme.md)
- [TWR-KM34Z75M](../../_boards/twrkm34z75m/freertos_examples/freertos_tickless/example_board_readme.md)
- [TWR-KM35Z75M](../../_boards/twrkm35z75m/freertos_examples/freertos_tickless/example_board_readme.md)
- [FRDM-MCXL255](../../_boards/frdmmcxl255/freertos_examples/freertos_tickless/example_board_readme.md)
- [FRDM-MCXE247](../../_boards/frdmmcxe247/freertos_examples/freertos_tickless/example_board_readme.md)
