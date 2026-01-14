Hardware requirements
=====================
- Mini/micro USB cable
- FRDM-K22F board
- Personal Computer

Board settings
==============
The EDMA memory to memory demo does not call for any special hardware configurations.

Prepare the Demo
================
1. Connect a mini USB cable between the PC host and the OpenSDA USB on the board.
2. Open a serial terminal on PC for OpenSDA serial device with these settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo.

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.
~~~~~~~~~~~~~~~~~~~~~
EDMA interleave transfer example.



edma signle transfer interleave start

0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	

edma signle transfer interleave finished

1	0	2	0	3	0	4	0	5	0	6	0	7	0	8	0	

edma minor loop interleave start

0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	0	

edma minor loop interleave finished

1	2	0	4	5	0	7	8	0	10	11	0	13	14	0	0	
~~~~~~~~~~~~~~~~~~~~~

