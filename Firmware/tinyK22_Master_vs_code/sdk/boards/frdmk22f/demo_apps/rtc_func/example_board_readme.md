Hardware requirements
=====================
- Mini/micro USB cable
- FRDM-K22F board
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
================
The following menu is displayed in the serial terminal:

~~~~~~~~~~~~~~~~~~~~~
RTC Demo running...

Please choose the sub demo to run:

1) Get current date time.

2) Set current date time.

3) Alarm trigger show.

4) Second interrupt show (demo for 20s).

Select:
~~~~~~~~~~~~~~~~~~~~~

1. Get current date time:

This command gets the current date time in the format of YYYY-MM-DD HH:MM:SS.
~~~~~~~~~~~~~~~~~~~~~
Current datetime: 2015-11-11 11:11:11
~~~~~~~~~~~~~~~~~~~~~

2. Set current date time

Sets the date time as YYYY-MM-DD HH:MM:SS.
~~~~~~~~~~~~~~~~~~~~~
Input date time like: "2010-10-10 10:10:10"
2009-10-10 10:10:10
~~~~~~~~~~~~~~~~~~~~~

3. Show the alarm function

Sets the alarm time: 1~9 seconds from the time you input and waits for the alarm event arrival.
~~~~~~~~~~~~~~~~~~~~~
Input the alarm seconds from the current moment (1s~9s):3
Triggered Alarm: 10:10:31
~~~~~~~~~~~~~~~~~~~~~

4. Seconds interrupt show

Shows the digital time animation by seconds-interrupt function.
~~~~~~~~~~~~~~~~~~~~~
10:10:51
10:10:51
...
10:11:11
~~~~~~~~~~~~~~~~~~~~~

