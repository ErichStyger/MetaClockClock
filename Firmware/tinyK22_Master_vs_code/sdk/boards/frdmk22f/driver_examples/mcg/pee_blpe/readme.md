# mcg_pee_blpe

## Overview
The pee_bple example shows how to use MCG driver to change from PEE mode to BLPE mode:

 1. How to use the mode functions for MCG mode switch.
 2. How to use the frequency functions to get current MCG frequency.
 3. Work flow
    Boot to PEE mode from default reset mode
    Change mode PEE -> PBE -> BLPE
    Change back BLPE -> PBE -> PEE
    Get System clock in PEE mode to blink LED

In this example, because the debug console's clock frequency may change,
so the example running information is not output from debug console. Here the
LED blinks to show that the example finished successfully.

## Supported Boards
- [FRDM-K22F](../../../_boards/frdmk22f/driver_examples/mcg/pee_blpe/example_board_readme.md)
- [TWR-KM34Z50MV3](../../../_boards/twrkm34z50mv3/driver_examples/mcg/pee_blpe/example_board_readme.md)
- [TWR-KM34Z75M](../../../_boards/twrkm34z75m/driver_examples/mcg/pee_blpe/example_board_readme.md)
- [TWR-KM35Z75M](../../../_boards/twrkm35z75m/driver_examples/mcg/pee_blpe/example_board_readme.md)
