/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_H_
#define _APP_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define EXAMPLE_DSPI_MASTER_BASEADDR   SPI0
#define EXAMPLE_DSPI_MASTER_CLK_SRC    DSPI0_CLK_SRC
#define EXAMPLE_DSPI_MASTER_CLK_FREQ   CLOCK_GetFreq(DSPI0_CLK_SRC)
#define EXAMPLE_DSPI_MASTER_PCS        kDSPI_Pcs0
#define EXAMPLE_DSPI_MASTER_IRQ        SPI0_IRQn
#define EXAMPLE_DSPI_MASTER_IRQHandler SPI0_IRQHandler

#define EXAMPLE_DSPI_DEALY_COUNT 0XFFFFFU
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
/*${prototype:end}*/

#endif /* _APP_H_ */
