readme.txt
----------
Firmware for the different clocks, using NXP MCUXpresso IDE and SDK.
The firmware includes the master controlling all clocks and the 'slaves' with one or up to 4 clocks on board

Common Library: place this folder on the same level as the projects
- ClockCommon: directory with the common files for all projects
- McuLib: Mcu library, used by all projects

Master:
- LPC845_Master: Master using the NXP LPC845-BRK board with RTC and RS-485 breakout, possible to use a breakout board
- tinyK22_Master: Master using the tinyK22 and WS2812B interface plus RTC on a custom board

Clock Boards:
- LPC845_Clock4: original board with LPC845 driving 4 clocks
- K02_128_Clock2: Dual-Clock board (detached) with the NXP K02FN128, able to drive two LED rings
- K02_64_Clock2: same as above, but usinig the K02FN64 with 64KByte Flash
