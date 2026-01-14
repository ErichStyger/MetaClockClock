# ftm_pwm_twochannel

## Overview
The FTM pwm two channel Example project is a demonstration program that uses the KSDK software to generate a square 
pulse PWM(24kHZ) on 2 channel to control the LED brightness. It sets up the FTM hardware block to output two edge-aligned PWM signals.
The PWM dutycycles are periodically updated.
- FTM generates a PWM with the increasing and decreasing duty cycle.
- LED brightness is increasing and then dimming. This is a continuous process.
The user should probe the FTM output with a oscilloscope to see the PWM.

## Supported Boards
- [FRDM-K22F](../../../_boards/frdmk22f/driver_examples/ftm/pwm_twochannel/example_board_readme.md)
- [FRDM-KE02Z40M](../../../_boards/frdmke02z40m/driver_examples/ftm/pwm_twochannel/example_board_readme.md)
- [FRDM-KE15Z](../../../_boards/frdmke15z/driver_examples/ftm/pwm_twochannel/example_board_readme.md)
- [FRDM-KE16Z](../../../_boards/frdmke16z/driver_examples/ftm/pwm_twochannel/example_board_readme.md)
- [FRDM-KE17Z](../../../_boards/frdmke17z/driver_examples/ftm/pwm_twochannel/example_board_readme.md)
- [FRDM-KE17Z512](../../../_boards/frdmke17z512/driver_examples/ftm/pwm_twochannel/example_board_readme.md)
- [FRDM-MCXE247](../../../_boards/frdmmcxe247/driver_examples/ftm/pwm_twochannel/example_board_readme.md)
- [LPCXpresso860MAX](../../../_boards/lpcxpresso860max/driver_examples/ftm/pwm_twochannel/example_board_readme.md)
