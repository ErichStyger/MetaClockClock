Hardware requirements
=====================
- 2x Mini/micro USB cables
- Target microcontroller board
- Personal Computer
- FreeMASTER 2.5 or later installed on PC

Board settings
==============
No special settings are required. The demo communicates over a virtual serial 
line (CDC) interface implemented by the microcontroller application and available
at the device USB port. Two USB cables are required, one for the debugger session
and the other for the microcontroller device USB port connection.

Prepare the demo
===============
1.  Connect a USB cable between the host PC and the Debug USB port on the target board.
2.  Compile and download the program to the target microcontroller.
3.  Run and resume application execution when debugger stops in the main() function.

Connect with FreeMASTER
=======================
4.  Connect the 2nd USB cable between host PC and the microcontroller USB port on the target board.
5.  Wait for the operating system to configure the new USB serial device and assigns the COMx port.
6.  Run FreeMASTER, use the Connection Wizard or open Project Options.
7.  Select serial communication, choose the newly installed COMx port and connect at speed 115200 bps.
8.  Start communication, FreeMASTER loads the initial TSA Active Content links in the Welcome page.
9.  Click the "FreeMASTER Demonstration Project (embedded in device)" in the Welcome page
10. The demo is now running, you should be able to watch variable values and graphs.

More information
================
Read more information about FreeMASTER tool at http://www.nxp.com/freemaster.
Feel free to ask questions and report issues at FreeMASTER's 
community page at https://community.nxp.com/community/freemaster
