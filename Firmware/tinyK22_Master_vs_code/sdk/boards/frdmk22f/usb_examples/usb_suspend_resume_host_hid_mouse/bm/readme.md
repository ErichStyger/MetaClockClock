# host_suspend_resume_hid_mouse



## Overview

The Suspend/Resume USB Host HID example is a simple demonstration program based on the MCUXpresso SDK.
The host will release the USB bus and enter into the low power mode when user needs to suspend the USB bus.
The host can be waked up by a switch or the resume signal delivered by the device if the remote wake-up feature is enabled.
The host will wake up the device by delivering the resume signal if it is waked up by a switch.

<br> The application supports the mouse device.

## System Requirement

### Hardware requirements

- Mini/micro USB cable
- USB A to micro AB cable
- Hardware (Tower module/base board, and so on) for a specific device
- Personal Computer (PC)


### Software requirements

- The project path is:
<br> <MCUXpresso_SDK_Install>/boards/<board>/usb_examples/usb_suspend_resume_host_hid_mouse/<rtos>/<toolchain>.
> The <rtos> is Bare Metal or FreeRTOS OS.


## Getting Started

### Hardware Settings

> Set the hardware jumpers (Tower system/base module) to default settings.


### Prepare the example 

1.  Download the program to the target board.
2.  Power off the target board and power on again.
3.  Connect devices to the board.

> For detailed instructions, see the appropriate board User's Guide.
> this example can only work with SDK_DEBUGCONSOLE = 1, setting this macro to 0 will lead to build error.
> Host hid example doesn't support HID report descriptor analysis, this example assume that the device data are sent by specific order. 
      <br> For more detail, please refer to the code. For the device list we tested,
      <br> please refer to chapter "Peripheral devices tested with the USB Host stack" in "SDK Release Notes xxxx(board name)".

## Run the example

1.  Connect the board UART to the PC and open the COM port in a terminal tool.
2.  Make sure to use a USB HUB or an adapter with OTG functionality firstly. Plug in the mouse device to the board. The attached information prints out in the terminal.
3.  The mouse operation information prints in the terminal when you operate the mouse.
    <br> The application prints the mouse operation information in one line. Each line contains the following sequential string:
    "Left Click", "Middle Click", "Right Click" and "Wheel Down"/"Wheel Up" movement. For "Right"/"Left" movement and "UP"/"Down" movement, to print less log, the related log is printed after movement count reaches 100.
    <br> For example, when the mouse moves right for some time,
    <br> ``` "                                  Right move events: 100 times             "```
    <br> prints in the terminal.

4.  Suspend<br>
    Start to suspend the bus by entering character 's'. And then select whether enable the remote wake-up feature and set/clear the feature if the device supports the feature.
    And then release the USB bus and enter into the low power mode.
    For example, if the name of wakeup switch is "SW3", the debug console output is as following. 
    <br>
    ```
        host init done, the host stack version is 1.0.0.
        Please Enter 's' to start suspend test
        hid mouse attached:pid=0x7cvid=0x1fc9 address=1
        mouse attached
                                                Right move events: 100 times
                                                Down move events: 100 times
        ...
                                                Left move events: 100 times
        Start suspend USB BUS...

        Please Enter:
            1. Enable remote wakeup feature.
            2. Disable remote wakeup feature.
                                                Up move events: 100 times
        ...
                                                Up move events: 100 times
                                                Right move events: 100 times
        1Remote wakeup feature set.
        BUS has been suspended.
        Please Press wakeup switch(SW3) to start resume test.
        Or, wait for device sends resume signal.
    ```
    <br>
    <br>ote The name of wakeup switch is showed in debug console.

5.  Resume<br> 
    The host is waked up and starts to deliver the resume signal when the wake up source is the wakeup switch.
    For example, if the name of wakeup switch is "SW3", the debug console output is as following. 
    <br>
    ```
        BUS has been suspended.
        Please Press wakeup switch(SW3) to start resume test.
        Start resume the device.
        BUS has been resumed.
                                                Up move events: 100 times
                                                Right move events: 100 times
    ```
    Or the host is waked up when the resume signal is detected on the bus.
    ```
        BUS has been suspended.
        Please Press wakeup switch(SW3) to start resume test.
        Or, wait for device sends resume signal.
        BUS has been resumed.
                                                Down move events: 100 times
                                                Left move events: 100 times
    ```
    <br>
    <br>ote The name of wakeup switch is showed in debug console.



*/


## Supported Boards
- MIMXRT1170-EVKB
- FRDM-K22F
- [LPCXpresso55S69](../../_boards/lpcxpresso55s69/usb_examples/usb_suspend_resume_host_hid_mouse/example_board_readme.md)
- EVK-MIMXRT1064
- MIMXRT685-AUD-EVK
- [LPCXpresso54S018](../../_boards/lpcxpresso54s018/usb_examples/usb_suspend_resume_host_hid_mouse/example_board_readme.md)
- [LPCXpresso55S16](../../_boards/lpcxpresso55s16/usb_examples/usb_suspend_resume_host_hid_mouse/example_board_readme.md)
- [LPCXpresso54S018M](../../_boards/lpcxpresso54s018m/usb_examples/usb_suspend_resume_host_hid_mouse/example_board_readme.md)
- MIMXRT1060-EVKB
- EVK-MIMXRT1010
- MIMXRT1040-EVK
- FRDM-MCXN947
- MIMXRT1024-EVK
- [LPCXpresso55S28](../../_boards/lpcxpresso55s28/usb_examples/usb_suspend_resume_host_hid_mouse/example_board_readme.md)
- [LPCXpresso54628](../../_boards/lpcxpresso54628/usb_examples/usb_suspend_resume_host_hid_mouse/example_board_readme.md)
- LPCXpresso55S36
- [MCX-N5XX-EVK](../../_boards/mcxn5xxevk/usb_examples/usb_suspend_resume_host_hid_mouse/example_board_readme.md)
- MIMXRT1060-EVKC
- MIMXRT1160-EVK
- [MIMXRT1180-EVK](../../_boards/evkmimxrt1180/usb_examples/usb_suspend_resume_host_hid_mouse/example_board_readme.md)
- FRDM-K32L2A4S
- EVK-MIMXRT1020
- MIMXRT700-EVK
- EVK-MIMXRT595
- EVK-MIMXRT685
- [MCX-N9XX-EVK](../../_boards/mcxn9xxevk/usb_examples/usb_suspend_resume_host_hid_mouse/example_board_readme.md)
- EVKB-IMXRT1050
- [RD-RW612-BGA](../../_boards/rdrw612bga/usb_examples/usb_suspend_resume_host_hid_mouse/example_board_readme.md)
- [FRDM-RW612](../../_boards/frdmrw612/usb_examples/usb_suspend_resume_host_hid_mouse/example_board_readme.md)
- EVK-MIMXRT1015
- FRDM-MCXN236
