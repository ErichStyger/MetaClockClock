Hardware requirements
===================
- Mini/micro USB cable
- FRDM-K22F
- Personal Computer

Board settings
============
No special settings are required.

Prepare the Demo
===============
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
============
The ADC PDB demo application will print the following message to the terminal:

~~~~~~~~~~~~~~~~~
Run pdb trig adc with flextimer demo.
ADC Full Range: XXXX
Input any character to start demo.
~~~~~~~~~~~~~~~~~~
Input a character to the serial console to start the ADC PDB demo.
The demo will then display 256 lines of information for the ADC conversion result.
