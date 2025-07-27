readme.txt
----------

Clock project with the NXP LPC845.

Issues:
- Board with ID 0x26: triggering MAG3 also pulls down MAG2?
- Hint: only connect RTT if debugger is paused
- With -Os, hardfault in MinINI code (works with -O1)
- rs sendcmd 0x26 mag status returns random pin status. Ok with mag get or executed locally
- magnet 0x04 always on with board 0x25?

----------------------------------------------------------------------------------------------------------
Board bring-up:
- Resistance 5V-GND: ~100k
- Resistance 3V-BND: ~360k

Power:
- V0.1: power consumption: 5V @ 80 mA
- V1.0: no motors (only LPC): ~20mA (5V), limit at 30mA
- V1.0: with motors: 205mA (4V), limit at 250 mA. Motors initialized and operating: all 6 boards: ~85mA


Links:
- https://github.com/ErichStyger/MetaClockClock
- https://mcuoneclipse.com/2020/05/24/60-billion-lights-2400-rgb-leds-and-120-stepper-motors-hiding-behind-canvas-art/
- https://mcuoneclipse.com/2020/06/07/behind-the-canvas-making-of-60-billion-lights/
- https://mcuoneclipse.com/2019/11/24/world-stepper-clock-with-nxp-lpc845/


CMD> matrix a 0 0 0 0 2 sh,0 0 1 180 2 sh,1 0 0 0 2 sh,1 0 1 180 2 sh,2 0 0 0 2 sh,2 0 1 180 2 sh,3 0 0 0 2 sh,3 0 1 180 2 sh

CMD> matrix a 0 0 0 180 2 sh,0 0 1 0 2 sh,1 0 0 180 2 sh,1 0 1 0 2 sh,2 0 0 180 2 sh,2 0 1 0 2 sh,3 0 0 180 2 sh,3 0 1 0 2 sh

matrix q 4 1 0 a 180 2 sh;matrix q 4 1 1 a 0 2 sh;matrix q 5 1 0 a 180 2 sh;matrix q 5 1 1 a 0 2 sh

matrix q 4 1 0 a 0 2 sh;matrix q 4 1 1 a 180 2 sh;matrix q 5 1 0 a 0 2 sh;matrix q 5 1 1 a 180 2 sh
matrix exq


rs sendcmd 0x26 matrix q 0 0 0 a 180 2 cw,0 0 1 a 0 2 cc,1 0 0 a 180 2 cw,1 0 1 a 0 2 cc,2 0 0 a 180 2 cw,2 0 1 a 0 2 cc,3 0 0 a 180 2 cw,3 0 1 a 0 2 cc
rs sendcmd 0x0 matrix exq

rs sendcmd 0x26 matrix q 0 0 0 a 0 2 sh,0 0 1 a 180 2 sh,1 0 0 a 0 2 sh,1 0 1 a 180 2 sh,2 0 0 a 0 2 sh,2 0 1 a 180 2 sh,3 0 0 a 0 2 sh,3 0 1 a 180 2 sh
rs sendcmd 0x0 matrix exq