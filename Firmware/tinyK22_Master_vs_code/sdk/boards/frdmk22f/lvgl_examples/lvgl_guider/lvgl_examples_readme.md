Hardware requirements
===================
- Micro USB cable
- FRDM-K22F board
- Adafruit TFT LCD shield w/Cap Touch
- Personal Computer

Board settings
============
There are different versions of Adafruit 2.8" TFT LCD shields. The shields marked
v2.0 works directly with this project. For the other shields, please solder
the center pin of IOREF pads to the 3.3V pad, and solder the pads 11, 12, and 13.
See the link for details:
https://community.nxp.com/t5/MCUXpresso-Community-Articles/Modifying-the-latest-Adafruit-2-8-quot-LCD-for-SDK-graphics/ba-p/1131104

Attach the LCD shield to the FRDM board.

Prepare the Demo
===============
1.  Connect a micro USB cable between the PC host and the OpenSDA USB port on the board.
2.  Build the project.
3.  Download the program to the target board.
4.  Reset the SoC and run the project.

Running the demo
===============
Known issue:
The MCU is not able to generate hardware reset of the LCD due to hardware limitation of the boards interconnection.
Because of this the LCD may not get initialized (stays blank) following a cold start (power on) of the board even if the demo was
correctly programmed to the FLASH memory. In such a case simply press the reset button to restart the demo.
In Rev D, the debug port UART and I2C for touch screen connect to the same pins, in this demo these two pins are used by I2C, PRINTF
function in the code can't work normally.
Supported board revisions:
FRDM-K22F REV A is not supported, the example is validated on REV C and D.
