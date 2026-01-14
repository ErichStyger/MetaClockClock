Hardware requirements
===================
- Mini/micro USB cable
- FRDM-K22F board
- Personal Computer

Board settings
============
No special is needed.

Prepare the Demo
===============
1.  Connect a mini USB cable between the PC host and the OpenSDA USB port on the board.
2.  Connect the analog signal source's output to the input of used-defined comparator's channel (defined as 
"DEMO_CMP_USER_CHANNEL" in source code.)
3.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Reset the SoC and run the project.

Running the demo
===============
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~~~
CMP interrupt Example
~~~~~~~~~~~~~~~~~~~~~

Then change CMP analog input, and watch the change of LED.
    - CMP0_IN0 (Jump J1-16) connected to VCC=3.3V(Jump J25-4): LED GREEN on
    - CMP0_IN0 (Jump J1-16) connected to GND(Jump J2-14): LED GREEN off
