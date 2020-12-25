readme.txt
----------
Project for a tinyK22 board (NXP K22FN512VLH12) with with FreeRTOS running a ring of WS2812B.

WS2818B data: GPIOD (PTD0...PTD7)
RS-485:       EN (PTB19=> UART0 RTS/PTB2, high active for tx), UART0 TX (PTB17), RX (PTB16)
OpenSDA UART: LPUART Rx PTC3, Tx PTC4

Bootloader via USB and SD card: https://community.nxp.com/docs/DOC-347572?et=watches.email.document

Example commands
================
In terminal with RS-485-2-USB converter (virtual address 0x2):
----------------------------------------
@<TO> <FROM> <CRC> <string> 
(TO, FROM and CRC are two characters in hex)
@02 05 14 cmd rs status

@20 02 xx cmd matrix q 0 0 1 a 90 10 sh
@20 02 xx cmd matrix q 0 0 0 a 270 10 sh
@00 02 xx cmd matrix exq

From Master (tinyK22) (address 0x1):
-----------------
rs sendcmd 0x20 rs help

Example hand immediate move (without leds):
rs sendcmd 0x20 matrix r 0 0 0 180 10 cw

rs sendcmd 0x20 matrix q 0 0 0 r 90 8 cc
rs sendcmd 0x20 matrix exq

matrix R 0 0 0 90 2 cw
matrix R 0 0 0 90 2 CC
matrix R 0 0 0 360 1 CW

- measure current: 'standard' with hands on: 4 A, ring random color: 4.2 A, ring+hands random color moving: 5.6A

Issues/open points:
-------------------
- flash programming K22
- watchdog K22
- matrix R 0 4 0 360 3 CW
  led is behind, while
  matrix R 0 4 0 360 3 CC
  is ahead?
- RS485UartResponseQueue gets filled even if not sending commands
- line wrap on master side:
CMD> rs sendcmd 0x27 matrix help
matrix                    ; Group of matrix commands
  help|status             ; Print help or status information
  test                    ; Test the stepper on the board
  12                      ; Set matrix to 12:00 position
                          ; <xyz>: coordinate, separated by space, e.g. 0 0 1
                          ; <md>: mode (cc, cw, sh), lowercase mode letter is with accel contro
l for start/stop, e.g. Cw
- add relative move map mode
- Problem if clock somehow does not receive commands and this is not detected?
Time: 12:10
CRC_ERR 22: 04 expected 74
‰R@01 22 F5 NOK
r=-)@01 27 F9 OK
Time: 12:11
- reduce RS-485 errors on clocks
- reduce the number of stepper queues? just having one with 'wait' queue entry? allows to send new commands while executing
- add/use brightness for hands and rings
- lower brightness for rings: only use every second led
- implement Adafruit NeoPixel app mode
- implement Adafruit accelerometer mode
- Bug: ColorPicker: if any value is zero, nothing is sent (reported to adafruit forum)
- intermezzo with temperature
- changing scenes/numbers: dimm lights out instead just switch them off
- stadium wave: follow hands with a rgb dot (ahead?)
- clock: second dots sometimes skips one dot?
- clock: seconds off should turn off the dot (send signal)
- clock border off should remove border

Slaves:
- reduce stepper backlash?
- following gives an error (timeout)?
        rs sendcmd 0x28 matrix hour 1
        @28 01 37 cmd matrix hour 1
        *** Failed or unknown command: rs sendcmd 0x28 matrix hour 1
        *** Type help to get a list of available commands
        CMD> @01 28 96 OK
    Better: send back OK after CRC check!
    
    
 @60 01 BB cmd matrix q 0 0 0 rc 0x010304 ,0 0 1 rc 0x050103
