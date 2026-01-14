# power_mode_switch

## Overview
The Power mode switch demo application demonstrates the use of power modes in the KSDK. The demo prints the power mode menu
through the debug console, where the user can set the MCU to a specific power mode. The user can also set the wakeup
source by following the debug console prompts. The purpose of this demo is to show how to switch between different power
 modes, and how to configure a wakeup source and wakeup the MCU from low power modes.

 Tips:
 This demo is to show how the various power mode can switch to each other. However, in actual low power use case, to save energy and reduce the consumption even more, many things can be done including:
 - Disable the clock for unnecessary modules during low power mode. That means, programmer can disable the clocks before entering the low power mode and re-enable them after exiting the low power mode when necessary.
 - Disable the function for unnecessary part of a module when other part would keep working in low power mode. At the most time, more powerful function means more power consumption. For example, disable the digital function for the unnecessary pin mux, and so on.
 - Set the proper pin state (direction and logic level) according to the actual application hardware. Otherwise, there would be current leakage on the pin, which will increase the power consumption.
 - Other low power consideration based on the actual application hardware.
 - Debug pins(e.g SWD_DIO) would consume addtional power, had better to disable related pins or disconnect them.

## Supported Boards
- [FRDM-K22F](../../_boards/frdmk22f/demo_apps/power_mode_switch/example_board_readme.md)
- [FRDM-K32L2A4S](../../_boards/frdmk32l2a4s/demo_apps/power_mode_switch/example_board_readme.md)
- [FRDM-K32L2B](../../_boards/frdmk32l2b/demo_apps/power_mode_switch/example_board_readme.md)
- [FRDM-KE15Z](../../_boards/frdmke15z/demo_apps/power_mode_switch/example_board_readme.md)
- [FRDM-KE16Z](../../_boards/frdmke16z/demo_apps/power_mode_switch/example_board_readme.md)
- [FRDM-KE17Z](../../_boards/frdmke17z/demo_apps/power_mode_switch/example_board_readme.md)
- [FRDM-KE17Z512](../../_boards/frdmke17z512/demo_apps/power_mode_switch/example_board_readme.md)
- [FRDM-MCXC041](../../_boards/frdmmcxc041/demo_apps/power_mode_switch/example_board_readme.md)
- [FRDM-MCXC242](../../_boards/frdmmcxc242/demo_apps/power_mode_switch/example_board_readme.md)
- [FRDM-MCXC444](../../_boards/frdmmcxc444/demo_apps/power_mode_switch/example_board_readme.md)
- [FRDM-MCXE247](../../_boards/frdmmcxe247/demo_apps/power_mode_switch/example_board_readme.md)
- [FRDM-RW612](../../_boards/frdmrw612/demo_apps/power_mode_switch/example_board_readme.md)
- [RD-RW612-BGA](../../_boards/rdrw612bga/demo_apps/power_mode_switch/example_board_readme.md)
- [TWR-KM34Z50MV3](../../_boards/twrkm34z50mv3/demo_apps/power_mode_switch/example_board_readme.md)
- [TWR-KM34Z75M](../../_boards/twrkm34z75m/demo_apps/power_mode_switch/example_board_readme.md)
- [TWR-KM35Z75M](../../_boards/twrkm35z75m/demo_apps/power_mode_switch/example_board_readme.md)
