Hardware requirements
===================
- Mini USB cable
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
When the example runs successfully, the following message is displayed in the terminal:

~~~~~~~~~~~~~~~~~~~~~
SHELL build: Sep 27 2020
Copyright  2020  NXP

LOG SHELL>> installbackend debugconsole
LOG SHELL>> log error
       22350: ERROR log_main.c:104:This is "error" log message

LOG SHELL>> log fatal
       27900: FATAL log_main.c:100:This is "fatal" log message

LOG SHELL>> log warning
       35430: WARN  log_main.c:108:This is "warning" log message

LOG SHELL>> log info
       38585: INFO  log_main.c:112:This is "info" log message

LOG SHELL>> log debug
       41330: DEBUG log_main.c:116:This is "debug" log message

LOG SHELL>> log trace
       45090: TRACE log_main.c:120:This is "trace" log message

LOG SHELL>> log test
       49835: ERROR log_main.c:124:The input arguement "test" is not valid

LOG SHELL>> uninstallbackend debugconsole
LOG SHELL>> 
~~~~~~~~~~~~~~~~~~~~~
Note: The shell information "SHELL build: Sep 27 2020" may be different, which depends on the compile date.
