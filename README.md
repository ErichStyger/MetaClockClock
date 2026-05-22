# MetaClockClock
Project to build a clock with clocks: the Meta Clock Clock!
The idea is to use many 'clocks' to display information with text, digits and color. The design is very modular, and it can be customized in many ways:
- number of clocks, There are builds with 24, 60 or even 120 clocks
- with or without RGB LEDs

![60 clock version with time](/Hardware/images/1946.jpg?raw=true "60 clock version with time")

# Ordering from PCBWay
PCBWay sponsored the setup fee and assembly for 80 LED rings for my second [MetaClockClock78](https://mcuoneclipse.com/2026/05/03/building-the-metaclockclock78-my-largest-clock-project-yet/#led-ring-pcbs-and-assembly).
Total costs for PCBs plus assembly was $1091, and PCBWay sponsored $430 of it.
Their service was amazing with smooth communication. I really appreciated that their engineers asked apropriate questions and shared their progress.
I received the boards earlier than expected, and 100% of the PCBs worked which is amazing. See links in next section if you want to order either just the PCBs or fully assembled versions from [PCBWay](https://www.pcbway.com).

# PCBs and Assembly
Read https://github.com/ErichStyger/MetaClockClock/wiki about building our own clock version.

All of my PCBs have been produced by [PCBWay](https://www.pcbway.com/).
I highly recommend using **PCBWay** for assembly services too, see [PCBs and Assembly Service](https://mcuoneclipse.com/2026/05/03/building-the-metaclockclock78-my-largest-clock-project-yet/#led-ring-pcbs-and-assembly). You order PCBs and fully assembled PCBs from PCBWay with Gerbers and BOM:
- [MCU Board]( https://www.pcbway.com/project/shareproject/MetaClockClock_MCU_Board.html)
- [Motor Board](https://www.pcbway.com/project/shareproject/MetaClockClock_Motor_Board__Satellite_.html)
- [LED Ring](https://www.pcbway.com/project/shareproject/MetaClockClock_LED_Ring.html)
- [tinyK22](https://www.pcbway.com/project/shareproject/TinyK22_V1_3.html)
- [Controller Board](https://www.pcbway.com/project/shareproject/MetaClockClock_Master_tinyK22.html)

## Articles
For more information about this project, see the following articles:
- [Building the MetaClockClock78: My Largest Clock Project Yet](https://mcuoneclipse.com/2026/05/03/building-the-metaclockclock78-my-largest-clock-project-yet/)
- [Hackster.io: Erich Styger's MetaClockClock78 Clock-of-Clocks Display Is the Biggest Yet](https://www.hackster.io/news/erich-styger-s-metaclockclock78-clock-of-clocks-display-is-the-biggest-yet-a1265e577950)
- [New MetaClockClock: Combining Art and Technology in Clocks](https://mcuoneclipse.com/2025/08/03/new-metaclockclock-combining-art-and-technology-in-clocks/)
- [“World Stepper Clock with NXP LPC845”](https://mcuoneclipse.com/2019/11/24/world-stepper-clock-with-nxp-lpc845/)
- [“60 Billion Lights”: 2400 RGB LEDs and 120 Stepper Motors hiding behind Canvas Art”](https://mcuoneclipse.com/2020/05/24/60-billion-lights-2400-rgb-leds-and-120-stepper-motors-hiding-behind-canvas-art/)
- [Behind the Canvas: Making of “60 Billion Lights”](https://mcuoneclipse.com/2020/06/07/behind-the-canvas-making-of-60-billion-lights/)
- [New Version](https://mcuoneclipse.com/2020/07/19/new-version/) using a modular design
- 60 Clock version built with the round modular PCBs: [New MetaClockClock V3 finished with 60 Clocks](https://mcuoneclipse.com/2020/12/26/new-metaclockclock-v3-finished-with-60-clocks/)
- V4 version using walnut veneer: [MetaClockClock V4 for the Year 2021](https://mcuoneclipse.com/2021/01/01/metaclockclock-v4-for-the-year-2021/)


## Directory Structure
- **Firmware**: contains the firmware for the different master and clock versions
- **Hardware**: contains PCB files
- **3D**: laser cutter and 3D printer files
