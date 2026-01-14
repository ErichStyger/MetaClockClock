# i2c_interrupt

## Overview
The i2c_functional_interrupt example shows how to use I2C functional driver to build a interrupt based application:

In this example , one i2c instance used as I2C master and another i2c instance used as I2C slave .
1. I2C master send data to I2C slave in interrupt . (I2C Slave using interrupt to receive the data)
2. I2C master read data from I2C slave in interrupt . (I2C Slave using interrupt to send the data)
3. The example assumes that the connection is OK between master and slave, so there's NO error handling code.

## Supported Boards
- [FRDM-K22F](../../../_boards/frdmk22f/driver_examples/i2c/interrupt/example_board_readme.md)
- [FRDM-K32L2B](../../../_boards/frdmk32l2b/driver_examples/i2c/interrupt/example_board_readme.md)
- [FRDM-MCXC242](../../../_boards/frdmmcxc242/driver_examples/i2c/interrupt/example_board_readme.md)
- [FRDM-MCXC444](../../../_boards/frdmmcxc444/driver_examples/i2c/interrupt/example_board_readme.md)
