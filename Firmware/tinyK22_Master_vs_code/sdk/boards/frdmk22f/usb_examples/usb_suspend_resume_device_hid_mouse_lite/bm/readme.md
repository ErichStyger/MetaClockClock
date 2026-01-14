# dev_suspend_resume_hid_mouse_lite_bm




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
- FRDM-MCXC444
- FRDM-K22F
- LPCXpresso55S69
- EVK-MIMXRT1064
- FRDM-MCXA153
- MIMXRT685-AUD-EVK
- LPCXpresso54S018
- LPCXpresso55S16
- FRDM-K32L2B
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
- FRDM-K32L2A4S
- EVK-MIMXRT1020
- MIMXRT700-EVK
- FRDM-MCXA156
- EVK-MIMXRT595
- EVK-MIMXRT685
- FRDM-MCXC242
- MCX-N9XX-EVK
- EVKB-IMXRT1050
- RD-RW612-BGA
- FRDM-K32L3A6
- EVK-MIMXRT1015
- FRDM-MCXN236
