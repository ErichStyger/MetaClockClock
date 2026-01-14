Hardware requirements
=====================
- Mini USB cable
- FRDM-K22F board
- Personal Computer

Board settings
==============
Connect J4-11(DAC_OUT) to J4-2(PTB0/LLWU_P5/ADC0_SE8/ADC1_SE8)


Prepare the Demo
===============
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Download the program to the target board.
3.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
4.  Run "run.py" on your PC with -p (--port) parameter to select COM port (baud rate can be also set through -b (--bd) parameter).

For detailed instructions, see the appropriate board User's Guide.

Running the demo
================

A. Server on the board, client on the PC:

Selected UART port: com15, 115200 bd
eRPC server has been run
eRPC client has been created
Board configuration:
    DAC: True
    LED: Red=True, Green=True, Blue=True
ADC configuration:
    Vref=3.29999995232 V
    Atomic steps=4096.0

---------------------------
eRPC Remote Control example
---------------------------
-> Press '1' for DAC-ADC conversion
-> Press '2' for GPIO LED
-> Press '3' for Accelerometer and Magnetometer values
2
Select which LED should be turned on:
--> Press '1' for red
--> Press '2' for green
--> Press '3' for blue
1

---------------------------
eRPC Remote Control example
---------------------------
-> Press '1' for DAC-ADC conversion
-> Press '2' for GPIO LED
-> Press '3' for Accelerometer and Magnetometer values
3
Read value from Accelerometer and Magnetometer:
    Accelerometer axis: x=7608, y=16736, z=-512
    Magnetometer axis: x=0, y=0, z=0

---------------------------
eRPC Remote Control example
---------------------------
-> Press '1' for DAC-ADC conversion
-> Press '2' for GPIO LED
-> Press '3' for Accelerometer and Magnetometer values
1
Enter voltage <0V - 3.3V>: 1.5
DAC is set for 1.5 V which is 1861 of atomic steps
ADC value: 1.50095212675 V


B. Server on the PC, client on the board:

[Message from board] Button SW2 was pressed
[Message from board] Button SW3 was pressed
