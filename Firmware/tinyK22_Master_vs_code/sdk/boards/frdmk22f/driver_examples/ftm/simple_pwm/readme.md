# ftm_simple_pwm

## Overview
The FTM project is a simple demonstration program of the SDK FTM driver. It sets up the FTM
hardware block to output a center-aligned PWM signal. The PWM dutycycle is periodically updated.
On boards that have an LED connected to the FTM pins, the user will see a change in LED brightness.

## Note
If the FTM instance has only TPM features, please use the TPM driver.
Can be judged by FSL_FEATURE_FTM_IS_TPM_ONLY_INSTANCE(base) macro.

## Supported Boards
- [FRDM-K22F](../../../_boards/frdmk22f/driver_examples/ftm/simple_pwm/example_board_readme.md)
- [FRDM-KE02Z40M](../../../_boards/frdmke02z40m/driver_examples/ftm/simple_pwm/example_board_readme.md)
- [FRDM-KE15Z](../../../_boards/frdmke15z/driver_examples/ftm/simple_pwm/example_board_readme.md)
- [FRDM-KE16Z](../../../_boards/frdmke16z/driver_examples/ftm/simple_pwm/example_board_readme.md)
- [FRDM-KE17Z](../../../_boards/frdmke17z/driver_examples/ftm/simple_pwm/example_board_readme.md)
- [FRDM-KE17Z512](../../../_boards/frdmke17z512/driver_examples/ftm/simple_pwm/example_board_readme.md)
- [FRDM-MCXE247](../../../_boards/frdmmcxe247/driver_examples/ftm/simple_pwm/example_board_readme.md)
- [LPCXpresso860MAX](../../../_boards/lpcxpresso860max/driver_examples/ftm/simple_pwm/example_board_readme.md)
