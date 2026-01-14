# dev_video_virtual_camera




## Overview

The USB video virtual camera application is a simple demonstration program based on the MCUXpresso SDK.
It is enumerated as a camera and users can see the video of the device by using a PC test tool.

## System Requirement

### Hardware requirements

- Mini/micro USB cable
- USB A to micro AB cable
- Hardware (Tower module/base board, and so on) for a specific device
- Personal Computer (PC)


### Software requirements

- The project files for lite version example are in:
<br> <MCUXpresso_SDK_Install>/boards/<board>/usb_examples/usb_device_video_virtual_camera_lite/<rtos>/<toolchain>.
<br>  For non-lite version example, the path is:
<br> <MCUXpresso_SDK_Install>/boards/<board>/usb_examples/usb_device_video_virtual_camera/<rtos>/<toolchain>.
note<br>
<br> - The <rtos> is Bare Metal or FreeRTOS OS. 
<br> - The video virtual camera example is tested in Microsoft Windows 10.


## Getting Started

### Hardware Settings

> Set the hardware jumpers (Tower system/base module) to default settings.


### Prepare the example

1.  Download the program to the target board.
2.  Connect the target board to the external power source (the example is self-powered).
3.  Power off the target board. And then power on again.
4.  Connect a USB cable between the PC and the USB device port of the board.

> For detailed instructions, see the appropriate board User's Guide.

## Run the example

1.  Plug in the device, which is running video virtual camera application, into the PC. An imaging device "Virtual Camera Demo" is enumerated in the Device Manager.\n
<br>![The video device is attached](usb_device_video_test_tool_attached.jpg "The video device is attached")
2.  Open the camera tool Windows provides, switch to use the target camera by clicking the button in the red circle.
<br>![Windows camera test tool](usb_device_video_test_tool_windows.jpg "Windows camera test tool")
    Or use online tool https://webcammictest.com/, select "Virtual Camera Device".
<br>![Online camera test tool](usb_device_video_test_tool_web.jpg "Online camera test tool")

> The resolution is 176 * 144 and the video image format is MJPEG. The frame interval includes 30, 25, 20, 15, 10, and 5 and the default frame interval is 15.


*/


## Supported Boards
- MIMXRT1170-EVKB
- FRDM-K22F
- LPCXpresso55S69
- EVK-MIMXRT1064
- FRDM-MCXA153
- MIMXRT685-AUD-EVK
- LPCXpresso51U68
- LPCXpresso54S018
- [LPCXpresso55S16](../../_boards/lpcxpresso55s16/usb_examples/usb_device_video_virtual_camera/example_board_readme.md)
- LPCXpresso54S018M
- MIMXRT1060-EVKB
- EVK-MIMXRT1010
- MIMXRT1040-EVK
- FRDM-MCXN947
- MIMXRT1024-EVK
- LPCXpresso55S28
- LPCXpresso54628
- LPCXpresso55S36
- MCX-N5XX-EVK
- MIMXRT1060-EVKC
- MIMXRT1160-EVK
- MIMXRT1180-EVK
- [FRDM-K32L2A4S](../../_boards/frdmk32l2a4s/usb_examples/usb_device_video_virtual_camera/example_board_readme.md)
- EVK-MIMXRT1020
- MIMXRT700-EVK
- FRDM-MCXA156
- EVK-MIMXRT595
- EVK-MIMXRT685
- MCX-N9XX-EVK
- EVKB-IMXRT1050
- RD-RW612-BGA
- FRDM-K32L3A6
- EVK-MIMXRT1015
- FRDM-MCXN236
- FRDM-RW612
