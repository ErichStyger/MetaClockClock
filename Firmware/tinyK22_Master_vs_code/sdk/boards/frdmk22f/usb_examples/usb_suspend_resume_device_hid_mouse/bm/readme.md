# dev_suspend_resume_hid_mouse




## Overview

The Suspend/Resume USB HID mouse application is a simple demonstration program based on the MCUXpresso SDK.
It is enumerated as a mouse. Users can see the mouse arrow moving on the PC screen according in a rectangular fashion.
When host releases the USB Bus, the device will enter into low power mode. The device can be waked up when resume signal is detected on the bus.
Or the device can remote wake-up the host by delivering the resume signal when the remote wake-up is enabled by the host.

## System Requirements

### Hardware requirements

- Mini/micro USB cable
- USB A to micro AB cable
- Hardware (Tower module/base board, and so on) for a specific device
- Personal Computer (PC)


### Software requirements

- The project files are in:
<br> <MCUXpresso_SDK_Install>/boards/<board>/usb_examples/usb_suspend_resume_device_hid_mouse/<rtos>/<toolchain>.
<br> For lite version, the project files are in:
<br> <MCUXpresso_SDK_Install>/boards/<board>/usb_examples/usb_suspend_resume_device_hid_mouse_lite/<rtos>/
<toolchain>.
> The <rtos> is Bare Metal or FreeRTOS OS.


## Getting Started

### Hardware Settings

> Set the hardware jumpers (Tower system/base module) to default settings.


### Prepare the example

> For detailed instructions, see the appropriate board User's Guide.

## Run the example

1.  Plug-in the device, which is running Suspend/Resume HID mouse example, into the PC. A HID-compliant mouse is enumerated in the Device Manager.
2.  The mouse arrow is moving on PC screen in the rectangular rotation.
3.  Make sure your PC enable the USB remote wake support in BIOS.Press the sleep button to make your PC go to sleep and the USB bus becomes idle. Then the device enters into low power mode.
4.  If the remote wake-up feature is enabled by the host, the device is waked up and delivers the resume signal when the wakeup switch is pressed.
    The name of wakeup switch is showed in debug console when the remote wake-up feature is enabled.
    For example, if the name of wakeup switch is "SW3", the debug console output is as following. 
    ```
        USB device remote wakeup state: 1
        USB device start suspend
        USB device suspended.
        Please Press wakeup switch(SW3) to remote wakeup the host.
        Remote wakeup the host.
        USB device start resume
        USB device resumed.
    ```
    <br> Or the device is waked up when the resume signal is detected on the bus.
    ```
        USB device remote wakeup state: 0
        USB device start suspend
        USB device suspended.
        USB device start resume
        USB device resumed.
    ```



*/


## Supported Boards
- MIMXRT1170-EVKB
- FRDM-K22F
- LPCXpresso55S69
- EVK-MIMXRT1064
- MIMXRT685-AUD-EVK
- [LPCXpresso54S018](../../_boards/lpcxpresso54s018/usb_examples/usb_suspend_resume_device_hid_mouse/example_board_readme.md)
- [LPCXpresso55S16](../../_boards/lpcxpresso55s16/usb_examples/usb_suspend_resume_device_hid_mouse/example_board_readme.md)
- [LPCXpresso54S018M](../../_boards/lpcxpresso54s018m/usb_examples/usb_suspend_resume_device_hid_mouse/example_board_readme.md)
- MIMXRT1060-EVKB
- EVK-MIMXRT1010
- MIMXRT1040-EVK
- [FRDM-MCXN947](../../_boards/frdmmcxn947/usb_examples/usb_suspend_resume_device_hid_mouse/example_board_readme.md)
- MIMXRT1024-EVK
- LPCXpresso55S28
- [LPCXpresso54628](../../_boards/lpcxpresso54628/usb_examples/usb_suspend_resume_device_hid_mouse/example_board_readme.md)
- LPCXpresso55S36
- [MCX-N5XX-EVK](../../_boards/mcxn5xxevk/usb_examples/usb_suspend_resume_device_hid_mouse/example_board_readme.md)
- MIMXRT1060-EVKC
- MIMXRT1160-EVK
- [MIMXRT1180-EVK](../../_boards/evkmimxrt1180/usb_examples/usb_suspend_resume_device_hid_mouse/example_board_readme.md)
- FRDM-K32L2A4S
- EVK-MIMXRT1020
- MIMXRT700-EVK
- FRDM-MCXA156
- EVK-MIMXRT595
- EVK-MIMXRT685
- [MCX-N9XX-EVK](../../_boards/mcxn9xxevk/usb_examples/usb_suspend_resume_device_hid_mouse/example_board_readme.md)
- EVKB-IMXRT1050
- [RD-RW612-BGA](../../_boards/rdrw612bga/usb_examples/usb_suspend_resume_device_hid_mouse/example_board_readme.md)
- FRDM-K32L3A6
- EVK-MIMXRT1015
- FRDM-MCXN236
- [FRDM-RW612](../../_boards/frdmrw612/usb_examples/usb_suspend_resume_device_hid_mouse/example_board_readme.md)
