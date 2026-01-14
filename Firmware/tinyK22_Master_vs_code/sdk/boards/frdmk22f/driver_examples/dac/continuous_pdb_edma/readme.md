# dac_continuous_pdb_edma

## Overview

The demo shows how to use the PDB to generate a DAC trigger and use the DMA to transfer data into DAC buffer.

In this example, DAC is first set to normal buffer mode. PDB is as DAC hardware trigger source and DMA would work 
when DAC read pointer is zero. When run the example, the DAC is triggered by PDB and the read pointer increases by one,
every time the trigger occurs. When the read pointer reaches the upper limit, it goes to zero directly in the next trigger event.
while read pointer goes to zero, DMA request will be triggered and transfer data into DAC buffer. The user should probe
the DAC output with a oscilloscope to see the Half-sine signal.

## Supported Boards
- [FRDM-K22F](../../../_boards/frdmk22f/driver_examples/dac/continuous_pdb_edma/example_board_readme.md)
