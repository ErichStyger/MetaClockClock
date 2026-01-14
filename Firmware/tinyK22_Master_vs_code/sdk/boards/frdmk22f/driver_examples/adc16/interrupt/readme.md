# adc16_interrupt

## Overview

The adc16_interrupt example shows how to use interrupt with ADC16 driver.

In this example, user should indicate a channel to provide a voltage signal (can be controlled by user) as the ADC16's
sample input. When running the project, typing any key into debug console would trigger the conversion. ADC interrupt 
would be asserted once the conversion is completed. In ADC ISR, the conversion completed flag would be cleared by 
reading the conversion result value. Also, the conversion result value is stored, and the ISR counter is increased. 
These information would be printed when the execution return to the main loop.

The point is that the ADC16 interrupt configuration is set when configuring the ADC16's conversion channel. When in 
software trigger mode, the conversion would be launched by the operation of configuring channel, just like writing a
conversion command. So if user wants to generate the interrupt every time the conversion is completed, the channel's 
configuration with enabling interrupt setting would be used for each conversion.

## Supported Boards
- [FRDM-K22F](../../../_boards/frdmk22f/driver_examples/adc16/interrupt/example_board_readme.md)
- [FRDM-K32L2A4S](../../../_boards/frdmk32l2a4s/driver_examples/adc16/interrupt/example_board_readme.md)
- [FRDM-K32L2B](../../../_boards/frdmk32l2b/driver_examples/adc16/interrupt/example_board_readme.md)
- [FRDM-MCXC041](../../../_boards/frdmmcxc041/driver_examples/adc16/interrupt/example_board_readme.md)
- [FRDM-MCXC242](../../../_boards/frdmmcxc242/driver_examples/adc16/interrupt/example_board_readme.md)
- [FRDM-MCXC444](../../../_boards/frdmmcxc444/driver_examples/adc16/interrupt/example_board_readme.md)
- [TWR-KM34Z50MV3](../../../_boards/twrkm34z50mv3/driver_examples/adc16/interrupt/example_board_readme.md)
- [TWR-KM34Z75M](../../../_boards/twrkm34z75m/driver_examples/adc16/interrupt/example_board_readme.md)
- [TWR-KM35Z75M](../../../_boards/twrkm35z75m/driver_examples/adc16/interrupt/example_board_readme.md)
