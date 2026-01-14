# pdb_dac_trigger

## Overview

The pdb_dac_trigger example shows how to use the PDB to generate a DAC trigger.

Based on the basic counter, to use the DAC trigger, just to enable the DAC trigger's "milestone" and set the user-
defined value for it.
The DAC's "milestone" is called as "interval". Multiple DAC trigger intervals can be included into one PDB counter's cycle.
DAC trigger's counter would reset after the trigger is created and start counting again to the interval value.

In this example, the DAC is configured with hardware buffer enabled in normal work mode. Once it gets the trigger from 
the PDB, the buffer read pointer increases.

## Supported Boards
- [FRDM-K22F](../../../_boards/frdmk22f/driver_examples/pdb/dac_trigger/example_board_readme.md)
