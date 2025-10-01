TODO:
- Special modes:
6:00 Einschalten

6:00-10:00 Uhr Block
Normalfunktion Uhr (13min) 
                flow wechsel zu Temperatur (1min) 
                flow wechsel zu Datum (1min)
                flow wechsel zurück zur Normalfunktion Uhr

10:00-10:30 Pausen Block
                flow (7 min)
                wechsel zu Uhr (1min)
                wechsel zu Temperatur (1min)
                wechsel zu Datum (1min)

10:30-12:00 Uhr Block
Normalfunktion Uhr (13min) 
                flow wechsel zu Temperatur (1min) 
                flow wechsel zu Datum (1min)
                flow wechsel zurück zur Normalfunktion Uhr

12:00-13:00 Mittags Block
                flow (7 min)
                wechsel zu Uhr (1min)
                wechsel zu Temperatur (1min)
                wechsel zu Datum (1min)

13:00-15:00 Uhr Block
Normalfunktion Uhr (13min) 
                flow wechsel zu Temperatur (1min) 
                flow wechsel zu Datum (1min)
                flow wechsel zurück zur Normalfunktion Uhr

15:00-15:30 Pausen Block
                flow (7 min)
                wechsel zu Uhr (1min)
                wechsel zu Temperatur (1min)
                wechsel zu Datum (1min)

15:30-22:00 Uhr Block
Normalfunktion Uhr (13min) 
                flow wechsel zu Temperatur (1min) 
                flow wechsel zu Datum (1min)
                flow wechsel zurück zur Normalfunktion Uhr

22:00 Ausschalten

***flow: wellenartiges intermezzo, wie hier https://youtu.be/0pdKkobyyyI ab Minute

 



- HardFault on tinyk22
	demo clap
	demo square rotate
	matrix hour 12

- Intermezzo 17,21,19,20 too fast not in sync with hands? All with relative moves? Problem with calculation errors using PL_CONFIG_USE_VIRTUAL_STEPPER? Disabled for now in intermezzo.
- issue with LPC board 0x26: alle 4 steppers seems to clack/jump sometimes. Replace drivers?
  MATRIX_SendToRemoteQueueExecuteAndWait(false);  <<= false fixed it?
   ==> problems are the RS-485 interrupts on the LPC845, causing the stepper motor timer to interrupt or delay?
     Unclear why it only showed up on this board?
   ==> the 100 Ohm termination resistor on the last board seems to cause this. Termination resistor was not enabled on the tinyK22 master.
   Setting/enabling it did not improve, but removing (unjumpering the solder jumper) on the last board let the problem disappear.
   Experimented with different values, but a 300 Ohm resistor instead of the 100 Ohm solved the issue too. Not clear why it had such an effect.

- If intermezzo stopped or not running: clear pixels (but not every time)!
#if PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_SMARTWALL_16x5
  } else {
    clearUnusedPixel(); /*clear the unused pixels in the background */
#endif


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

From Master (tinyK22) (address 0x1), examples assume RS-485 target address as 0x20
-----------------
rs sendcmd 0x20 rs help
rs sendcmd 0x20 matrix help

Example hand immediate move immediate relativ 'r' at (0,0,0) local coordinate, 180 degree with a delay of 10, clockwise (cw):
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

Issues/open points/wish list:
-----------------------------
- add ability to change hour/minute using buttons (2-4 buttons)
- watchdog for K22
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
- reduce stepper backlash
- add letters to 4x5 font
- intermezzo with month (JAN 2021) with larger font
- have 'intermezzo only' mode
- Booster to 5V: 
  https://hackaday.com/2021/01/12/smart-power-delivery-for-long-led-strips/
  https://roboticsbrno.github.io/RB0004-NeopixelBooster/eval_v2/index.html
  https://cadlab.io/project/23122/master/files
  https://github.com/RoboticsBrno/RB0004-NeopixelBooster
  https://hackaday.com/2021/01/13/stepping-down-voltage-with-reliability/
  
