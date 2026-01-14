# power_manager

## Overview
The power manager demo application demonstrates how to change power modes in the KSDK. The difference between this demo
and power_mode_switch is, this demo uses a notification framework to inform application about the mode change.
Application could register callback to the notification framework, when power mode changes, the callback
function is called and user can do something, such as closing debug console before entering low power mode, and
opening debug console after exiting low power mode.

When this demo runs, the power mode menu is shown in the debug console, where the user can set the MCU to a specific power mode.
User can also set the wakeup source following the debug console prompts.

 Tips:
 This demo is to show how the various power mode can switch to each other. However, in actual low power use case, to save energy and reduce the consumption even more, many things can be done including:
 - Disable the clock for unnecessary modules during low power mode. That means, programmer can disable the clocks before entering the low power mode and re-enable them after exiting the low power mode when necessary.
 - Disable the function for unnecessary part of a module when other part would keep working in low power mode. At the most time, more powerful function means more power consumption. For example, disable the digital function for the unnecessary pin mux, and so on.
 - Set the proper pin state (direction and logic level) according to the actual application hardware. Otherwise, there would be current leakage on the pin, which will increase the power consumption.
 - Other low power consideration based on the actual application hardware.

## Supported Boards
- [EVK-MIMXRT595](../../_boards/evkmimxrt595/demo_apps/power_manager/example_board_readme.md)
- [EVK-MIMXRT685](../../_boards/evkmimxrt685/demo_apps/power_manager/example_board_readme.md)
- [FRDM-K22F](../../_boards/frdmk22f/demo_apps/power_manager/example_board_readme.md)
- [FRDM-K32L2A4S](../../_boards/frdmk32l2a4s/demo_apps/power_manager/example_board_readme.md)
- [FRDM-K32L2B](../../_boards/frdmk32l2b/demo_apps/power_manager/example_board_readme.md)
- [FRDM-KE15Z](../../_boards/frdmke15z/demo_apps/power_manager/example_board_readme.md)
- [FRDM-KE16Z](../../_boards/frdmke16z/demo_apps/power_manager/example_board_readme.md)
- [FRDM-KE17Z](../../_boards/frdmke17z/demo_apps/power_manager/example_board_readme.md)
- [FRDM-KE17Z512](../../_boards/frdmke17z512/demo_apps/power_manager/example_board_readme.md)
- [FRDM-MCXC041](../../_boards/frdmmcxc041/demo_apps/power_manager/example_board_readme.md)
- [FRDM-MCXC242](../../_boards/frdmmcxc242/demo_apps/power_manager/example_board_readme.md)
- [FRDM-MCXC444](../../_boards/frdmmcxc444/demo_apps/power_manager/example_board_readme.md)
- [FRDM-MCXE247](../../_boards/frdmmcxe247/demo_apps/power_manager/example_board_readme.md)
- [MIMXRT685-AUD-EVK](../../_boards/mimxrt685audevk/demo_apps/power_manager/example_board_readme.md)
- [TWR-KM34Z50MV3](../../_boards/twrkm34z50mv3/demo_apps/power_manager/example_board_readme.md)
- [TWR-KM34Z75M](../../_boards/twrkm34z75m/demo_apps/power_manager/example_board_readme.md)
- [TWR-KM35Z75M](../../_boards/twrkm35z75m/demo_apps/power_manager/example_board_readme.md)
