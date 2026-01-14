/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ffconf.h"

/* This fatfs subcomponent is disabled by default
 * To enable it, define following macro in ffconf.h */
#if defined(RAM_DISK_ENABLE) && (RAM_DISK_ENABLE == 1)

#include "fsl_common.h"
#include "fsl_ram_disk.h"

/*******************************************************************************
 * Globals
 ******************************************************************************/
static uint8_t disk_space[FSL_FF_RAMDISK_DISK_SIZE];

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Get RAM disk status.
 */
DSTATUS ram_disk_status(BYTE pdrv)
{
    if (pdrv != RAMDISK)
    {
        return STA_NOINIT;
    }
    return 0;
}

/*!
 * @brief Inidialize a RAM disk.
 */
DSTATUS ram_disk_initialize(BYTE pdrv)
{
    if (pdrv != RAMDISK)
    {
        return STA_NOINIT;
    }
    return 0;
}

/*!
 * @brief Read Sector(s) from RAM disk.
 */
DRESULT ram_disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count)
{
    if (pdrv != RAMDISK)
    {
        return RES_PARERR;
    }
    memcpy(buff, disk_space + sector * FSL_FF_RAMDISK_SECTOR_SIZE, FSL_FF_RAMDISK_SECTOR_SIZE * count);
    return RES_OK;
}

/*!
 * @brief Write Sector(s) to RAM disk.
 */
DRESULT ram_disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count)
{
    if (pdrv != RAMDISK)
    {
        return RES_PARERR;
    }
    memcpy(disk_space + sector * FSL_FF_RAMDISK_SECTOR_SIZE, buff, FSL_FF_RAMDISK_SECTOR_SIZE * count);
    return RES_OK;
}

/*!
 * @brief Miscellaneous RAM disk Functions.
 */
DRESULT ram_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff)
{
    if (pdrv != RAMDISK)
    {
        return RES_PARERR;
    }
    switch (cmd)
    {
        case GET_SECTOR_COUNT:
            *(uint32_t *)buff = FSL_FF_RAMDISK_DISK_SIZE / FSL_FF_RAMDISK_SECTOR_SIZE;
            return RES_OK;
            break;
        case GET_SECTOR_SIZE:
            *(WORD *)buff = FSL_FF_RAMDISK_SECTOR_SIZE;
            return RES_OK;
            break;
        case CTRL_SYNC:
            return RES_OK;
            break;
        default:
            break;
    }
    return RES_PARERR;
}
#endif /* defined(RAM_DISK_ENABLE) && (RAM_DISK_ENABLE == 1) */
