# host_msd_fatfs



## Overview

This Host FatFs example supports UFI and SCSI U-disk device. 
<br> The application prints the attached device information when U-disk device is attached.
The application executes some FatFs APIs to test the attached device.

## System Requirement

### Hardware requirements

- Mini/micro USB cable
- USB A to micro AB cable
- Hardware (Tower module/base board, and so on) for a specific device
- Personal Computer (PC)


### Software requirements

- The project path is: 
<br> <MCUXpresso_SDK_Install>/boards/<board>/usb_examples/usb_host_msd_fatfs/<rtos>/<toolchain>.
> The <rtos> is Bare Metal or FreeRTOS OS.


## Getting Started

### Hardware Settings

> Set the hardware jumpers (Tower system/base module) to default settings.


### Prepare the example 

1.  Download the program to the target board.
2.  Power off the target board and power on again.
3.  Connect devices to the board.

> For detailed instructions, see the appropriate board User's Guide.

## Run the example

1.  Connect the board UART to the PC and open the COM port in a terminal tool.
2.  Make sure to use a USB HUB or an adapter with OTG functionality firstly. Plug in a U-disk device to the board. The attached information prints out in the terminal.
3.  The test information prints in the terminal. The "success" message prints when a FatFs API succeeds. The "fail" message prints when a FatFs API fails.
    The test completes when either the FatFs API fails or all the tests are done.
<br> The following figure is an example for attaching one U-disk device.
<br>![Attach U-disk device](host_msd_fatfs_output.jpg "Attach U-disk device")
4.  To test the throughput, set the MSD_FATFS_THROUGHPUT_TEST_ENABLE to (1) in the file host_msd_fatfs.h. 
    An additional 64 K RAM is required to test the throughput. The macro is only supported on TWR-K65F180M Tower System module and IAR IDE.
    <br> The throughput test process is as follows:
    - Enable MSD_FATFS_THROUGHPUT_TEST_ENABLE.
    - Format the U-disk in the PC. Select the "Allocation unit size" 32 K.
    - Insert the U-disk. The throughput test starts.
<br> The following image shows an example.
<br>![Throughput test](host_msd_fatfs_throughput_output.jpg "Throughput test")

> 1. The throughput test only supports the TWR-K65F180M Tower System module.
> 2. LFN (long file name) can be enabled by changing the macro FF_USE_LFN as 1.



## Supported Boards
- MIMXRT1170-EVKB
- [FRDM-K22F](../../_boards/frdmk22f/usb_examples/usb_host_msd_fatfs/example_board_readme.md)
- [LPCXpresso55S69](../../_boards/lpcxpresso55s69/usb_examples/usb_host_msd_fatfs/example_board_readme.md)
- EVK-MIMXRT1064
- MIMXRT685-AUD-EVK
- [LPCXpresso54S018](../../_boards/lpcxpresso54s018/usb_examples/usb_host_msd_fatfs/example_board_readme.md)
- [LPCXpresso55S16](../../_boards/lpcxpresso55s16/usb_examples/usb_host_msd_fatfs/example_board_readme.md)
- [LPCXpresso54S018M](../../_boards/lpcxpresso54s018m/usb_examples/usb_host_msd_fatfs/example_board_readme.md)
- MIMXRT1060-EVKB
- EVK-MIMXRT1010
- MIMXRT1040-EVK
- FRDM-MCXN947
- MIMXRT1024-EVK
- [LPCXpresso55S28](../../_boards/lpcxpresso55s28/usb_examples/usb_host_msd_fatfs/example_board_readme.md)
- [LPCXpresso54628](../../_boards/lpcxpresso54628/usb_examples/usb_host_msd_fatfs/example_board_readme.md)
- LPCXpresso55S36
- [MCX-N5XX-EVK](../../_boards/mcxn5xxevk/usb_examples/usb_host_msd_fatfs/example_board_readme.md)
- MIMXRT1060-EVKC
- MIMXRT1160-EVK
- MIMXRT1180-EVK
- [FRDM-K32L2A4S](../../_boards/frdmk32l2a4s/usb_examples/usb_host_msd_fatfs/example_board_readme.md)
- EVK-MIMXRT1020
- MIMXRT700-EVK
- EVK-MIMXRT595
- EVK-MIMXRT685
- [MCX-N9XX-EVK](../../_boards/mcxn9xxevk/usb_examples/usb_host_msd_fatfs/example_board_readme.md)
- EVKB-IMXRT1050
- RD-RW612-BGA
- EVK-MIMXRT1015
- FRDM-MCXN236
- FRDM-RW612
