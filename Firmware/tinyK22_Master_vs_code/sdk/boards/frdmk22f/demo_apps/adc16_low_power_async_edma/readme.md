# adc16_low_power_async_edma

## Overview
The ADC Low Power Async DMA demo application demonstrates the usage of the ADC and DMA peripheral while in a low power mode. The
microcontroller is first set to very low power stop (VLPS) mode. Every 100 ms, low power timer trigger the ADC module convert
value on ADC channel. After 16 times(1,6s) the DMA transfer finish interrupt wake up the CPU to process sampled data, print result to
user and toggle LED.

## Supported Boards
- [FRDM-K22F](../../../_boards/frdmk22f/demo_apps/adc16_low_power_async_edma/example_board_readme.md)
