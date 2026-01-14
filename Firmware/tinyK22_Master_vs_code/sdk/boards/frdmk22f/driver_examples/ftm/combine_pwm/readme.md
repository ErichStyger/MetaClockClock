# ftm_combine_pwm

## Overview
The FTM project is a demonstration program of generating a combined PWM signal by the SDK FTM driver. It sets up the FTM
hardware block to output PWM signals(24kHZ) on two TPM channels. The example also shows the complementary mode of operation
and deadtime insertion, updates deadTimePrescale based on counter clock and tries to insert a dead time of about 650 nsec.
This example updates the pwm duty cycle with an interrupt, meanwhile, disables the interrupt to retain the current duty cycle
for a few seconds and disables the output on each channel before updating.
On boards that have 2 LEDs connected to the FTM pins, the user will see a change in LED brightness.
And if the board do not support LEDs to show, the outputs can be observed by oscilloscope.

## Supported Boards
- [FRDM-K22F](../../../_boards/frdmk22f/driver_examples/ftm/combine_pwm/example_board_readme.md)
- [FRDM-KE02Z40M](../../../_boards/frdmke02z40m/driver_examples/ftm/combine_pwm/example_board_readme.md)
- [FRDM-KE15Z](../../../_boards/frdmke15z/driver_examples/ftm/combine_pwm/example_board_readme.md)
- [FRDM-KE16Z](../../../_boards/frdmke16z/driver_examples/ftm/combine_pwm/example_board_readme.md)
- [FRDM-KE17Z](../../../_boards/frdmke17z/driver_examples/ftm/combine_pwm/example_board_readme.md)
- [FRDM-KE17Z512](../../../_boards/frdmke17z512/driver_examples/ftm/combine_pwm/example_board_readme.md)
- [FRDM-MCXE247](../../../_boards/frdmmcxe247/driver_examples/ftm/combine_pwm/example_board_readme.md)
- [LPCXpresso860MAX](../../../_boards/lpcxpresso860max/driver_examples/ftm/combine_pwm/example_board_readme.md)
