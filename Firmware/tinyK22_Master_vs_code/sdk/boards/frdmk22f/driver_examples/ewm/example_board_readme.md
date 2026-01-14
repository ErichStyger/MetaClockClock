Hardware requirements
=====================
- J-Link ARM
- FRDM-K22F board
- Mini/micro USB cable
- Personal Computer

Board settings
============
No special is needed.

Prepare the Demo
================
1. Connect a USB cable between the PC host and the OpenSDA USB port on the board.
2. Open a serial terminal with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================

When the example runs successfully, you can see the similar information from the terminal as below.

~~~~~~~~~~~~~~~~~~~~~
Press anykey to start the example...

EWM example
Press SW3 to expire EWM
EWM interrupt is occurred
Press SW3 to expire EWM again
~~~~~~~~~~~~~~~~~~~~~