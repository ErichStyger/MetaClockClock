# ftm_dual_edge_capture

## Overview
The FTM project is a demonstration program of the SDK FTM driver's dual-edge capture feature.
This feature is available only on certain SoC's.
The example sets up a FTM channel-pair for dual-edge capture. Meanwhile, the capture mode is set up one-shot, 
using API FTM_SetupDualEdgeCapture(). Once the input signal is received and the second edge is detected,
the interrupt flag will be cleared and the interrupt overflow will be disabled.
The example gets the capture value of the input signal using API FTM_GetInputCaptureValue().
And this example will print the capture values and period of the input signal on the terminal window.

If high level input is detected on the channel when FTM timer starts, FTM will generate a capture event incorrectly
although there is no rising edge. So this example ignore first pluse and measure second pluse. Need to ensure to
input least two pluse into the channel, a pwm signal is recommended.

## Supported Boards
- [FRDM-K22F](../../../_boards/frdmk22f/driver_examples/ftm/dual_edge_capture/example_board_readme.md)
- [FRDM-KE02Z40M](../../../_boards/frdmke02z40m/driver_examples/ftm/dual_edge_capture/example_board_readme.md)
- [FRDM-KE15Z](../../../_boards/frdmke15z/driver_examples/ftm/dual_edge_capture/example_board_readme.md)
- [FRDM-KE16Z](../../../_boards/frdmke16z/driver_examples/ftm/dual_edge_capture/example_board_readme.md)
- [FRDM-KE17Z](../../../_boards/frdmke17z/driver_examples/ftm/dual_edge_capture/example_board_readme.md)
- [FRDM-KE17Z512](../../../_boards/frdmke17z512/driver_examples/ftm/dual_edge_capture/example_board_readme.md)
- [FRDM-MCXE247](../../../_boards/frdmmcxe247/driver_examples/ftm/dual_edge_capture/example_board_readme.md)
- [LPCXpresso860MAX](../../../_boards/lpcxpresso860max/driver_examples/ftm/dual_edge_capture/example_board_readme.md)
