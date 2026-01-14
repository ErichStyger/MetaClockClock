Hardware requirements
=====================
- Mini/micro USB cable
- FRDM-K22F board
- Personal Computer
- Encoder with PHA and PHB singal.

Board settings
==============
1. Connect the "J24-2" (PTB0, FTM1_QD_PHA) to encoder's PHA singal.
2. Connect the "J24-4" (PTB1, FTM1_QD_PHB) to encoder's PHB singal.

Prepare the Demo
================
1. Connect a USB cable between the PC host and the OpenSDA USB port on the board.
2. Connect the board with encoder.
3. Open a serial terminal with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4. Download the program to the target board.
5. Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
Scroll forward and backword the encoder and watch the log on the terminal window:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

FTM quad decoder example
Encoder direction: -
Get current counter: 0
Encoder direction: -
Get current counter: 0
Encoder direction: -
Get current counter: 0
Encoder direction: -
Get current counter: 0
Encoder direction: -
Get current counter: 0
Encoder direction: -
Get current counter: 1995
Encoder direction: +
Get current counter: 1998
Encoder direction: +
Get current counter: 11
Encoder direction: +
Get current counter: 7
Encoder direction: +
Get current counter: 13
Encoder direction: +
Get current counter: 0
Encoder direction: +
Get current counter: 0
Encoder direction: +
Get current counter: 0
Encoder direction: -
Get current counter: 1995

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
