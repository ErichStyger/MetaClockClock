/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
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
#define EXAMPLE_DSPI_MASTER_BASE (SPI0_BASE)
#define EXAMPLE_DSPI_MASTER_IRQN (SPI0_IRQn)
#define DSPI_MASTER_CLK_SRC      (DSPI0_CLK_SRC)
#define DSPI_MASTER_CLK_FREQ     CLOCK_GetFreq((DSPI0_CLK_SRC))

#define EXAMPLE_DSPI_SLAVE_BASE (SPI1_BASE)
#define EXAMPLE_DSPI_SLAVE_IRQN (SPI1_IRQn)

#define SINGLE_BOARD   0
#define BOARD_TO_BOARD 1

#define EXAMPLE_CONNECT_DSPI SINGLE_BOARD
#if (EXAMPLE_CONNECT_DSPI == BOARD_TO_BOARD)
#define isMASTER         0
#define isSLAVE          1
#define SPI_MASTER_SLAVE isMASTER
#endif
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
/*${prototype:end}*/

#endif /* _APP_H_ */
