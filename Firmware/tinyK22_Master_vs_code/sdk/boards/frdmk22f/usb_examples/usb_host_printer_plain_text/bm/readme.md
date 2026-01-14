# host_printer_plain_text



## Overview

The host printer example demonstrates how to get the status of the printer device
and how to print a certain test string. Currently, it supports the virtual printer
(see device_printer_virtual_plain_text applicatioin) and the real printer, which supports the PJL and postscript (for example, HP LaserJet P2055dn).
PJL is one printer control protocol (PCP) from HP, postscript is one page description language (PDL) from Adobe.
For the virtual printer, the test string is shown on the virtual printer side so that
it is easy for the customer to verify the basic function of the printer. For the real printer,
it has to be installed correctly to make sure that the content can be printed from the printer.
Installing the printer is beyond the scope of this document.
For more information, see the appropriate manuals.

## System Requirement

### Hardware requirements

- Mini/micro USB cable
- USB A to micro AB cable
- Hardware (Tower module/base board, and so on) for a specific device
- Personal Computer(PC)


### Software requirements

- The project path is:
<br> <MCUXpresso_SDK_Install>/boards/<board>/usb_examples/usb_host_printer_plain_text/<rtos>/<toolchain>.
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
2.  Make sure to use a USB HUB or an adapter with OTG functionality firstly. Plug in the printer device to the board. The attached information prints out in the terminal.
> Two types of printer devices are supported: device_printer_virtual_plain_text demo device and
the real printer that support PJL and postscript.
3.  The terminal log is as follows:
<br>![debug console output](usb_host_printer_output.jpg "debug console output")
4.  If the device is a real printer which supports both PJL and postscript,
The "NXP host printer test" string is printed.
<br>![real print](usb_host_printer_print.jpg "real print")
If the device is a real printer which does not support PJL and postscript,
the "unsupported printer language" is output in the terminal log.
If the device is device_printer_virtual_plain_text virtual printer, the following is output in the device's debug console
terminal.
<br> ```
                file name
        NXP host printer test```
```



## Supported Boards
- MIMXRT1170-EVKB
- [FRDM-K22F](../../_boards/frdmk22f/usb_examples/usb_host_printer_plain_text/example_board_readme.md)
- [LPCXpresso55S69](../../_boards/lpcxpresso55s69/usb_examples/usb_host_printer_plain_text/example_board_readme.md)
- EVK-MIMXRT1064
- MIMXRT685-AUD-EVK
- LPCXpresso54S018
- [LPCXpresso55S16](../../_boards/lpcxpresso55s16/usb_examples/usb_host_printer_plain_text/example_board_readme.md)
- LPCXpresso54S018M
- MIMXRT1060-EVKB
- EVK-MIMXRT1010
- MIMXRT1040-EVK
- FRDM-MCXN947
- MIMXRT1024-EVK
- [LPCXpresso55S28](../../_boards/lpcxpresso55s28/usb_examples/usb_host_printer_plain_text/example_board_readme.md)
- LPCXpresso54628
- LPCXpresso55S36
- [MCX-N5XX-EVK](../../_boards/mcxn5xxevk/usb_examples/usb_host_printer_plain_text/example_board_readme.md)
- MIMXRT1060-EVKC
- MIMXRT1160-EVK
- MIMXRT1180-EVK
- FRDM-K32L2A4S
- EVK-MIMXRT1020
- MIMXRT700-EVK
- EVK-MIMXRT595
- EVK-MIMXRT685
- [MCX-N9XX-EVK](../../_boards/mcxn9xxevk/usb_examples/usb_host_printer_plain_text/example_board_readme.md)
- EVKB-IMXRT1050
- RD-RW612-BGA
- EVK-MIMXRT1015
- FRDM-MCXN236
- FRDM-RW612
