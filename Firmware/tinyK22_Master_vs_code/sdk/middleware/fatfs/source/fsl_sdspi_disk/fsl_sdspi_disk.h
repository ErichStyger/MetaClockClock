/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_SDSPI_DISK_H_
#define _FSL_SDSPI_DISK_H_

#include <stdint.h>
#include "ff.h"
#include "diskio.h"
#include "fsl_common.h"
#include "board.h"

/*!
 * @addtogroup SD Disk over SPI
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* DSPI clock source */
#if (BOARD_SDSPI_SPI_BASE == SPI0_BASE)
#define DSPI_MASTER_CLK_SRC (DSPI0_CLK_SRC)
#elif(BOARD_SDSPI_SPI_BASE == SPI1_BASE)
#define DSPI_MASTER_CLK_SRC (DSPI0_CLK_SRC)
#elif(BOARD_SDSPI_SPI_BASE == SPI2_BASE)
#define DSPI_MASTER_CLK_SRC (DSPI2_CLK_SRC)
#elif(BOARD_SDSPI_SPI_BASE == SPI3_BASE)
#define DSPI_MASTER_CLK_SRC (DSPI3_CLK_SRC)
#elif(BOARD_SDSPI_SPI_BASE == SPI4_BASE)
#define DSPI_MASTER_CLK_SRC (DSPI4_CLK_SRC)
#else
#error Should define the DSPI_MASTER_CLK_SRC!
#endif

/* Which PCS used to select the slave */
#if (BOARD_SDSPI_SPI_PCS_NUMBER == 0U)
#define DSPI_MASTER_PCS_CONFIG kDSPI_Pcs0
#define DSPI_MASTER_PCS_TRANSFER kDSPI_MasterPcs0
#define DSPI_MASTER_PCS_ACTIVE_LOW kDSPI_Pcs0ActiveLow
#elif(BOARD_SDSPI_SPI_PCS_NUMBER == 1U)
#define DSPI_MASTER_PCS_CONFIG kDSPI_Pcs1
#define DSPI_MASTER_PCS_TRANSFER kDSPI_MasterPcs1
#define DSPI_MASTER_PCS_ACTIVE_LOW kDSPI_Pcs1ActiveLow
#elif(BOARD_SDSPI_SPI_PCS_NUMBER == 2U)
#define DSPI_MASTER_PCS_CONFIG kDSPI_Pcs2
#define DSPI_MASTER_PCS_TRANSFER kDSPI_MasterPcs2
#define DSPI_MASTER_PCS_ACTIVE_LOW kDSPI_Pcs2ActiveLow
#elif(BOARD_SDSPI_SPI_PCS_NUMBER == 3U)
#define DSPI_MASTER_PCS_CONFIG kDSPI_Pcs3
#define DSPI_MASTER_PCS_TRANSFER kDSPI_MasterPcs3
#define DSPI_MASTER_PCS_ACTIVE_LOW kDSPI_Pcs3ActiveLow
#elif(BOARD_SDSPI_SPI_PCS_NUMBER == 4U)
#define DSPI_MASTER_PCS_CONFIG kDSPI_Pcs4
#define DSPI_MASTER_PCS_TRANSFER kDSPI_MasterPcs4
#define DSPI_MASTER_PCS_ACTIVE_LOW kDSPI_Pcs4ActiveLow
#elif(BOARD_SDSPI_SPI_PCS_NUMBER == 5U)
#define DSPI_MASTER_PCS_CONFIG kDSPI_Pcs5
#define DSPI_MASTER_PCS_TRANSFER kDSPI_MasterPcs5
#define DSPI_MASTER_PCS_ACTIVE_LOW kDSPI_Pcs5ActiveLow
#endif

#define DSPI_MASTER_CTAR (kDSPI_Ctar0) /* The CTAR to describle the transfer attribute */
#define DSPI_BUS_BAUDRATE (500000U)    /* Transfer baudrate - 500k */
/*************************************************************************************************
 * API - SD disk interface
 ************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name SD over SPI Disk Function
 * @{
 */

/*!
 * @brief Initializes SD disk over SPI.
 *
 * @param pdrv Physical drive number.
 * @retval STA_NOINIT Failed.
 * @retval RES_OK Success.
 */
DSTATUS sdspi_disk_initialize(BYTE pdrv);

/*!
 * Gets SD over SPI disk status
 *
 * @param pdrv Physical drive number.
 * @retval STA_NOINIT Failed.
 * @retval RES_OK Success.
 */
DSTATUS sdspi_disk_status(BYTE pdrv);

/*!
 * @brief Reads SD disk over SPI.
 *
 * @param pdrv Physical drive number.
 * @param buff The data buffer pointer to store read content.
 * @param sector The start sector number to be read.
 * @param count The sector count to be read.
 * @retval RES_PARERR Failed.
 * @retval RES_OK Success.
 */
DRESULT sdspi_disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count);

/*!
 * @brief Writes to SD disk over SPI.
 *
 * @param pdrv Physical drive number.
 * @param buff The data buffer pointer to store write content.
 * @param sector The start sector number to be written.
 * @param count The sector count to be written.
 * @retval RES_PARERR Failed.
 * @retval RES_OK Success.
 */
DRESULT sdspi_disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count);

/*!
 * @brief SD over SPI disk IO operation.
 *
 * @param pdrv Physical drive number.
 * @param cmd The command to be set.
 * @param buff The buffer to store command result.
 * @retval RES_PARERR Failed.
 * @retval RES_OK Success.
 */
DRESULT sdspi_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff);

/*************************************************************************************************
 * API - SPI interface
 ************************************************************************************************/
/*!
 * @brief Initializes the SPI.
 */
void spi_init(void);

/*!
 * @brief Sets the SPI bus frequency.
 *
 * @param frequency The frequency to set.
 * @retval kStatus_Success Success.
 * @retval kStatus_Fail Failed.
 */
status_t spi_set_frequency(uint32_t frequency);

/*!
 * @brief Transfers data over SPI bus in full-duplex way.
 *
 * @param in The buffer to save the data to be sent.
 * @param out The buffer to save the data to be read.
 * @param size The transfer data size.
 * @return The status of the function DSPI_MasterTransferPolling().
 */
status_t spi_exchange(uint8_t *in, uint8_t *out, uint32_t size);

/*!
 * @brief Initializes the timer to generator 1ms interrupt used to get current time in milliseconds.
 */
void timer_init(void);

/*!
 * @brief Gets current time in milliseconds.
 *
 * @return Current time in milliseconds.
 */
uint32_t timer_get_current_milliseconds(void);

/*!
 * @brief Initializes the host descriptor.
 */
void sdspi_host_init(void);

/* @} */
#if defined(__cplusplus)
}
#endif

#endif /* _FSL_SDSPI_DISK_H_ */
