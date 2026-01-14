# ftm_pdb_adc16

## Overview

The FTM / PDB demo application demonstrates how to use the FTM external trigger to start the ADC conversion using the
PDB. FTM0 is configured as a complementary combined mode, and each channel output frequency is 16 KHz. The complementary
channel dead time is 1 µs. The PDB pre-trigger works in back-to-back mode. The ADC0 and ADC1 work in single-end mode.
The ADC0 uses channel 1 and channel 5, and ADC1 uses channel 1 and channel 7.

## Supported Boards
- [FRDM-K22F](../../../_boards/frdmk22f/demo_apps/ftm_pdb_adc16/example_board_readme.md)
