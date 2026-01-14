/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_MMC_DISK_H_
#define _FSL_MMC_DISK_H_

#include <stdint.h>
#include "ff.h"
#include "diskio.h"

/*!
 * @addtogroup MMC Disk
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define CD_USING_GPIO

/*************************************************************************************************
 * API
 ************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name MMC Disk Function
 * @{
 */

/*!
 * @brief Initializes MMC disk.
 *
 * @param pdrv Physical drive number.
 * @retval STA_NOINIT Failed.
 * @retval RES_OK Success.
 */
DSTATUS mmc_disk_initialize(BYTE pdrv);

/*!
 * @brief Gets MMC disk status
 *
 * @param pdrv Physical drive number.
 * @retval STA_NOINIT Failed.
 * @retval RES_OK Success.
 */
DSTATUS mmc_disk_status(BYTE pdrv);

/*!
 * @brief Reads MMC disk.
 *
 * @param pdrv Physical drive number.
 * @param buff The data buffer pointer to store read content.
 * @param sector The start sector number to be read.
 * @param count The sector count to be read.
 * @retval RES_PARERR Failed.
 * @retval RES_OK Success.
 */
DRESULT mmc_disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count);

/*!
 * @brief Writes MMC disk.
 *
 * @param pdrv Physical drive number.
 * @param buff The data buffer pointer to store write content.
 * @param sector The start sector number to be written.
 * @param count The sector count to be written.
 * @retval RES_PARERR Failed.
 * @retval RES_OK Success.
 */
DRESULT mmc_disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count);

/*!
 * @brief MMC disk IO operation.
 *
 * @param pdrv Physical drive number.
 * @param cmd The command to be set.
 * @param buff The buffer to store command result.
 * @retval RES_PARERR Failed.
 * @retval RES_OK Success.
 */
DRESULT mmc_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff);

/* @} */

#if defined(__cplusplus)
}
#endif

#endif /* _FSL_MMC_DISK_H_ */
