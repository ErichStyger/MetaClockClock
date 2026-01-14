# dev_hid_mouse




## Overview

The USB HID mouse application is a simple demonstration program based on the MCUXpresso SDK.
It is enumerated as a mouse. Users can see the mouse arrow moving on the PC screen according in a rectangular fashion.
## System Requirements

### Hardware requirements

- Mini/micro USB cable
- USB A to micro AB cable
- Hardware (Tower module/base board, and so on) for a specific device
- Personal Computer (PC)


### Software requirements

- The project files are in:
<br> <MCUXpresso_SDK_Install>/boards/<board>/usb_examples/usb_device_hid_mouse/<rtos>/<toolchain>.
<br> For lite version, the project files are in:
<br> <MCUXpresso_SDK_Install>/boards/<board>/usb_examples/usb_device_hid_mouse_lite/<rtos>/<toolchain>.
> The <rtos> is Bare Metal or FreeRTOS OS.


## Getting Started

### Hardware Settings

> Set the hardware jumpers (Tower system/base module) to default settings.


### Prepare the example

1.  Download the program to the target board.
2.  Connect the target board to the external power source (the example is self-powered).
3.  Power off the target board. Then power on again.
4.  Connect a USB cable between the PC and the USB device port of the board.

> For detailed instructions, see the appropriate board User's Guide.

## Run the example

1.  Plug-in the device, which is running HID mouse example, into the PC. A HID-compliant mouse is enumerated in the Device Manager.
2.  The mouse arrow is moving on PC screen in the rectangular rotation.

*/


## Supported Boards
- [LPCXpresso55S16](../../_boards/lpcxpresso55s16/usb_examples/usb_device_hid_mouse/example_board_readme.md)
- [MCX-N5XX-EVK](../../_boards/mcxn5xxevk/usb_examples/usb_device_hid_mouse/example_board_readme.md)
- FRDM-K22F
- LPCXpresso55S69
- [MCX-N9XX-EVK](../../_boards/mcxn9xxevk/usb_examples/usb_device_hid_mouse/example_board_readme.md)
- [FRDM-MCXN947](../../_boards/frdmmcxn947/usb_examples/usb_device_hid_mouse/example_board_readme.md)
- LPCXpresso55S28
- FRDM-MCXA153
- [FRDM-K32L2A4S](../../_boards/frdmk32l2a4s/usb_examples/usb_device_hid_mouse/example_board_readme.md)
- LPCXpresso54S018M
- FRDM-K32L3A6
- LPCXpresso51U68
- LPCXpresso54S018
- FRDM-MCXA156
- FRDM-MCXN236
- LPCXpresso54628
- LPCXpresso55S36
