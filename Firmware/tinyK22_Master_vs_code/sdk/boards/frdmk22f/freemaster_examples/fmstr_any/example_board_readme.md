Hardware requirements
=====================
- FRDMK22F board
- Personal Computer
- FreeMASTER 3.0 or later installed on PC
- Serial line or USB equipment as needed to connect over selected interface.

Board settings
============
By default, the demo is configured for standard serial (UART) communication. It may
be changed to other interface in MCUXpresso ConfigTools. Run the MCUXpresso ConfigTools, 
select the Peripherals tool and open the Middleware/FreeMASTER component configuration.
Refer to the list of recommended communication options below and make the selection in 
the FreeMASTER configuration page. Note that the other communication interfaces may require
to use different physical connections, such as serial or USB ports. 

Recommended communication options 
=================================
- 'SERIAL' at UART1 module connected to virtual serial port of the debugger USB interface.
- 'Packet-driven BDM' using the debugger interface (use flash-based build targets only).

Prepare the demo
===============
1.  Interconnect the host PC and the target microcontroller board using the selected 
    interface cable.
2.  Compile and download the program to the target microcontroller.
3.  Run and resume application execution when debugger stops in the main() function.

If using the Packet-driven BDM interface
========================================
4.  Terminate the debugger session to release the JTAG interface for FreeMASTER use.
5.  Reset the target board to make sure it runs after detaching the debugger.

Connect with FreeMASTER
=======================
6.  Run FreeMASTER, use the Connection Wizard or open Project Options.
7.  Configure Serial or other communication as required by this demo setup.
8.  Start communication, FreeMASTER loads the initial TSA Active Content links in the Welcome page.
9.  Click the "FreeMASTER Demonstration Project (embedded in device)" in the Welcome page.
10. The demo is now running, you should be able to watch variable values and graphs.

More information
================
Read more information about FreeMASTER tool at http://www.nxp.com/freemaster.
Feel free to ask questions and report issues at FreeMASTER's 
community page at https://community.nxp.com/community/freemaster
