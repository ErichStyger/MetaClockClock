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
#if defined(MMC_DISK_ENABLE) && (MMC_DISK_ENABLE == 1)

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "fsl_mmc.h"
#include "fsl_mmc_disk.h"
#include "diskio.h"
/*******************************************************************************
 * Definitons
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief Card descriptor */
mmc_card_t g_mmc;

/*******************************************************************************
 * Code
 ******************************************************************************/

DRESULT mmc_disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count)
{
    if (pdrv != MMCDISK)
    {
        return RES_PARERR;
    }

    if (kStatus_Success != MMC_WriteBlocks(&g_mmc, buff, sector, count))
    {
        return RES_ERROR;
    }
    return RES_OK;
}

DRESULT mmc_disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count)
{
    if (pdrv != MMCDISK)
    {
        return RES_PARERR;
    }

    if (kStatus_Success != MMC_ReadBlocks(&g_mmc, buff, sector, count))
    {
        return RES_ERROR;
    }
    return RES_OK;
}

DRESULT mmc_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff)
{
    DRESULT res = RES_OK;

    if (pdrv != MMCDISK)
    {
        return RES_PARERR;
    }

    switch (cmd)
    {
        case GET_SECTOR_COUNT:
            if (buff)
            {
                *(uint32_t *)buff = g_mmc.userPartitionBlocks;
            }
            else
            {
                res = RES_PARERR;
            }
            break;
        case GET_SECTOR_SIZE:
            if (buff)
            {
                *(WORD *)buff = g_mmc.blockSize;
            }
            else
            {
                res = RES_PARERR;
            }
            break;
        case GET_BLOCK_SIZE:
            if (buff)
            {
                *(uint32_t *)buff = g_mmc.eraseGroupBlocks;
            }
            else
            {
                res = RES_PARERR;
            }
            break;
        case CTRL_SYNC:
            res = RES_OK;
            break;
        default:
            res = RES_PARERR;
            break;
    }

    return res;
}

DSTATUS mmc_disk_status(BYTE pdrv)
{
    if (pdrv != MMCDISK)
    {
        return STA_NOINIT;
    }

    return RES_OK;
}

DSTATUS mmc_disk_initialize(BYTE pdrv)
{
    static bool isCardInitialized = false;

    if (pdrv != MMCDISK)
    {
        return STA_NOINIT;
    }

    /* demostrate the normal flow of card re-initialization. If re-initialization is not neccessary, return RES_OK directly will be fine */
    if(isCardInitialized)
    {
        MMC_Deinit(&g_mmc);
    }

    if (kStatus_Success != MMC_Init(&g_mmc))
    {
        MMC_Deinit(&g_mmc);
        memset(&g_mmc, 0U, sizeof(g_mmc));
        return STA_NOINIT;
    }

    isCardInitialized = true;

    return RES_OK;
}
#endif /* defined(MMC_DISK_ENABLE) && (MMC_DISK_ENABLE == 1) */
