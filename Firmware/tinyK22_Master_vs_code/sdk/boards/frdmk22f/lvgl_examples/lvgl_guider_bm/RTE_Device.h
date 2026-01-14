/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void DSPI1_InitPins();
extern void DSPI1_DeinitPins();
extern void I2C1_InitPins();
extern void I2C1_DeinitPins();
extern void DSPI1_InitPins();
extern void DSPI1_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of DSPIX_GetFreq/DSPIX_InitPins/DSPIX_DeinitPins for the enabled DSPI
 * instance. */
#define RTE_SPI1            1
#define RTE_SPI1_PIN_INIT   DSPI1_InitPins
#define RTE_SPI1_PIN_DEINIT DSPI1_DeinitPins
#define RTE_SPI1_DMA_EN     1

/* User needs to provide the implementation of I2CX_GetFreq/I2CX_InitPins/I2CX_DeinitPins for the enabled I2C instance.
 */
#define RTE_I2C1            1
#define RTE_I2C1_PIN_INIT   I2C1_InitPins
#define RTE_I2C1_PIN_DEINIT I2C1_DeinitPins
#define RTE_I2C1_DMA_EN     0

/* DSPI configuration. */
#define RTE_SPI1_PCS_TO_SCK_DELAY       0
#define RTE_SPI1_SCK_TO_PSC_DELAY       0
#define RTE_SPI1_BETWEEN_TRANSFER_DELAY 0
#define RTE_SPI1_MASTER_PCS_PIN_SEL     kDSPI_MasterPcs0
#define RTE_SPI1_PIN_INIT               DSPI1_InitPins
#define RTE_SPI1_PIN_DEINIT             DSPI1_DeinitPins
#define RTE_SPI1_DMA_TX_CH              3
#define RTE_SPI1_DMA_TX_PERI_SEL        (uint8_t) kDmaRequestMux0SPI1
#define RTE_SPI1_DMA_TX_DMAMUX_BASE     DMAMUX0
#define RTE_SPI1_DMA_TX_DMA_BASE        DMA0
#define RTE_SPI1_DMA_RX_CH              4
#define RTE_SPI1_DMA_RX_PERI_SEL        (uint8_t) kDmaRequestMux0SPI1
#define RTE_SPI1_DMA_RX_DMAMUX_BASE     DMAMUX0
#define RTE_SPI1_DMA_RX_DMA_BASE        DMA0
#define RTE_SPI1_DMA_LINK_DMA_BASE      DMA0
#define RTE_SPI1_DMA_LINK_CH            5

#endif /* _RTE_DEVICE_H */
