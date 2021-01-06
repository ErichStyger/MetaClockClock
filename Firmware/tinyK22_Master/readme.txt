readme.txt
----------
Project for a tinyK22 board (NXP K22FN512VLH12) with with FreeRTOS.

Links:
- https://github.com/ErichStyger/MetaClockClock

Notes:
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

- current data: 'standard' with hands on: 4 A, ring random color: 4.2 A, ring+hands random color moving: 5.6A
 
Demo sequences:
------------------------ 
demo hands random pos
demo time large 14:34
intermezzo 17
demo temperature large 23
demo time large 14:35
demo text large 0 0 TEST
demo text large 0 0 DONE


matrix hc all 0x500;demo text large 0 0 "    ";demo text large 1 0 "YES "
demo text large 0 0 "    ";matrix hc all 0x50000;demo text large 0 0 " NO "


demo hands random pos
matrix hc all 0x50500
demo square
demo hands random pos
matrix hc all 0x500
demo time large 20:54
intermezzo 18
matrix hc all 0x50008
demo time 20:55
intermezzo 16
demo time 20:56

Issues/open points:
-------------------
- watchdog K22
- matrix R 0 4 0 360 3 CW
  led is behind, while
  matrix R 0 4 0 360 3 CC
  is ahead?
- reduce the number of stepper queues? just having one with 'wait' queue entry? allows to send new commands while executing
- add/use brightness for hands and rings
- lower brightness for rings: only use every second led
- implement Adafruit NeoPixel app mode
- implement Adafruit accelerometer mode
- Bug: ColorPicker: if any value is zero, nothing is sent (reported to adafruit forum)
- changing scenes/numbers: dimm lights out instead just switch them off
- stadium wave: follow hands with a rgb dot (ahead?)
- clock: second dots sometimes skips one dot?
- clock border off should remove border
- new intermezzo: clock in the middle with analog hands
- new intermezzo: ring, clock in the middle, then wave with clock
- new round clock: center, 12 around it
- reduce stepper backlash?
- add letters to 4x5 font
- intermezzo with month (JAN 2021) with larger font

