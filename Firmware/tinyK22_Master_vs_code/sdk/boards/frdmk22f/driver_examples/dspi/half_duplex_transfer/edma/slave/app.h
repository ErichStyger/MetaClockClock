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
#define EXAMPLE_DSPI_SLAVE_BASEADDR SPI0

#define EXAMPLE_DSPI_SLAVE_DMA_MUX_BASEADDR      DMAMUX
#define EXAMPLE_DSPI_SLAVE_DMA_BASEADDR          DMA0
#define EXAMPLE_DSPI_SLAVE_DMA_RX_REQUEST_SOURCE kDmaRequestMux0SPI0Rx
#define EXAMPLE_DSPI_SLAVE_DMA_TX_REQUEST_SOURCE kDmaRequestMux0SPI0Tx
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
/*${prototype:end}*/

#endif /* _APP_H_ */
