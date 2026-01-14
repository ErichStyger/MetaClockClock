 /*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_SD_DISK_H_
#define _FSL_SD_DISK_H_

#include <stdint.h>
#include "ff.h"
#include "diskio.h"
#include "fsl_sd.h"

/*!
 * @addtogroup SD Disk
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define CD_USING_GPIO

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern sd_card_t g_sd; /* sd card descriptor */

/*************************************************************************************************
 * API
 ************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name SD Disk Function
 * @{
 */

/*!
 * @brief Initializes SD disk.
 *
 * @param pdrv Physical drive number.
 * @retval STA_NOINIT Failed.
 * @retval RES_OK Success.
 */
DSTATUS sd_disk_initialize(BYTE pdrv);

/*!
 * Gets SD disk status
 *
 * @param pdrv Physical drive number.
 * @retval STA_NOINIT Failed.
 * @retval RES_OK Success.
 */
DSTATUS sd_disk_status(BYTE pdrv);

/*!
 * @brief Reads SD disk.
 *
 * @param pdrv Physical drive number.
 * @param buff The data buffer pointer to store read content.
 * @param sector The start sector number to be read.
 * @param count The sector count to be read.
 * @retval RES_PARERR Failed.
 * @retval RES_OK Success.
 */
DRESULT sd_disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count);

/*!
 * @brief Writes SD disk.
 *
 * @param pdrv Physical drive number.
 * @param buff The data buffer pointer to store write content.
 * @param sector The start sector number to be written.
 * @param count The sector count to be written.
 * @retval RES_PARERR Failed.
 * @retval RES_OK Success.
 */
DRESULT sd_disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count);

/*!
 * @brief SD disk IO operation.
 *
 * @param pdrv Physical drive number.
 * @param cmd The command to be set.
 * @param buff The buffer to store command result.
 * @retval RES_PARERR Failed.
 * @retval RES_OK Success.
 */
DRESULT sd_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff);

/* @} */
#if defined(__cplusplus)
}
#endif

#endif /* _FSL_SD_DISK_H_ */
