# led_control_bm

## Overview
The LED demo is used to demonstrate how to use new components (serial manager, LED, Button, GPIO adapter, Shell, Debug console, Uart adapter, Timer, and so on).
The main function of the demo is to control the led by using the shell or button.
For shell, please enter \"help\" to get the help information firstly. Turn on LED by using command \"led on\". And turn off LED by using command \"led off\".
If your board supports dimming, enhanch led brightness by using command \"dim brighten 5000\".And dim the brightness by using command \"dim darken 5000\".
For button, please press the button corresponding to control LED. Turn on LED when the button is pressed with long press or double click event.
And turn off LED when the button is pressed with short press or one click event.
Note: Please input one character at a time. If you input too many characters each time, the receiver may overflow
because the low level UART uses simple polling way for receiving. If you want to try inputting many characters each time,
just define DEBUG_CONSOLE_TRANSFER_NON_BLOCKING in your project to use the advanced debug console utility.

## Supported Boards
- [FRDM-K22F](../../../_boards/frdmk22f/component_examples/led_control/bm/example_board_readme.md)
- [FRDM-K32L2A4S](../../../_boards/frdmk32l2a4s/component_examples/led_control/bm/example_board_readme.md)
- [FRDM-K32L2B](../../../_boards/frdmk32l2b/component_examples/led_control/bm/example_board_readme.md)
- [FRDM-MCXC444](../../../_boards/frdmmcxc444/component_examples/led_control/bm/example_board_readme.md)
- [LPCXpresso54628](../../../_boards/lpcxpresso54628/component_examples/led_control/bm/example_board_readme.md)
