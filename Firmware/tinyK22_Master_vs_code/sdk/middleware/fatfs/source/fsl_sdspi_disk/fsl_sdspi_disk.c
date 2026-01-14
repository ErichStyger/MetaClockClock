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
#if defined(SDSPI_DISK_ENABLE) && (SDSPI_DISK_ENABLE == 1)

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "fsl_dspi.h"
#include "fsl_sdspi.h"
#include "fsl_gpio.h"
#include "fsl_sdspi_disk.h"


/* New project wizard guide note. */
#ifndef BOARD_SDSPI_SPI_BASE
#warning Undefined macro. Define BOARD_SDSPI_SPI_BASE in board.h
#endif

/*******************************************************************************
 * Definitons
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* SDSPI driver state. */
sdspi_card_t g_card;
sdspi_host_t g_host;
/*******************************************************************************
 * Code - SD disk interface
 ******************************************************************************/

DRESULT sdspi_disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count)
{
    if (pdrv != SDSPIDISK)
    {
        return RES_PARERR;
    }

    if (kStatus_Success != SDSPI_WriteBlocks(&g_card, (uint8_t *)buff, sector, count))
    {
        return RES_ERROR;
    }
    return RES_OK;
}

DRESULT sdspi_disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count)
{
    if (pdrv != SDSPIDISK)
    {
        return RES_PARERR;
    }

    if (kStatus_Success != SDSPI_ReadBlocks(&g_card, buff, sector, count))
    {
        return RES_ERROR;
    }
    return RES_OK;
}

DRESULT sdspi_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff)
{
    DRESULT result = RES_OK;

    if (pdrv != SDSPIDISK)
    {
        return RES_PARERR;
    }

    switch (cmd)
    {
        case GET_SECTOR_COUNT:
            if (buff)
            {
                *(uint32_t *)buff = g_card.blockCount;
            }
            else
            {
                result = RES_PARERR;
            }
            break;
        case GET_SECTOR_SIZE:
            if (buff)
            {
                *(WORD *)buff = g_card.blockSize;
            }
            else
            {
                result = RES_PARERR;
            }
            break;
        case GET_BLOCK_SIZE:
            if (buff)
            {
                *(uint32_t *)buff = g_card.csd.eraseSectorSize;
            }
            else
            {
                result = RES_PARERR;
            }
            break;
        case CTRL_SYNC:
            result = RES_OK;
            break;
        default:
            result = RES_PARERR;
            break;
    }
    return result;
}

DSTATUS sdspi_disk_status(BYTE pdrv)
{
    if (pdrv != SDSPIDISK)
    {
        return STA_NOINIT;
    }
    return 0;
}

DSTATUS sdspi_disk_initialize(BYTE pdrv)
{
    if (pdrv == SDSPIDISK)
    {
        sdspi_host_init();
        SDSPI_Init(&g_card);
        g_card.host = &g_host;
        return 0;
    }
    return STA_NOINIT;
}

/*******************************************************************************
 * Code - SPI interface
 ******************************************************************************/

void spi_init(void)
{
    uint32_t sourceClock;

    dspi_master_config_t masterConfig;

    /*Master config*/
    masterConfig.whichCtar = DSPI_MASTER_CTAR;
    masterConfig.ctarConfig.baudRate = DSPI_BUS_BAUDRATE;
    masterConfig.ctarConfig.bitsPerFrame = 8;
    masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveHigh;
    masterConfig.ctarConfig.cpha = kDSPI_ClockPhaseFirstEdge;
    masterConfig.ctarConfig.direction = kDSPI_MsbFirst;
    masterConfig.ctarConfig.pcsToSckDelayInNanoSec = 0;
    masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec = 0;
    masterConfig.ctarConfig.betweenTransferDelayInNanoSec = 0;

    masterConfig.whichPcs = DSPI_MASTER_PCS_CONFIG;
    masterConfig.pcsActiveHighOrLow = kDSPI_PcsActiveLow;

    masterConfig.enableContinuousSCK = false;
    masterConfig.enableRxFifoOverWrite = false;
    masterConfig.enableModifiedTimingFormat = false;
    masterConfig.samplePoint = kDSPI_SckToSin0Clock;

    sourceClock = CLOCK_GetFreq(DSPI_MASTER_CLK_SRC);
    DSPI_MasterInit((SPI_Type *)BOARD_SDSPI_SPI_BASE, &masterConfig, sourceClock);
}

status_t spi_set_frequency(uint32_t frequency)
{
    uint32_t sourceClock;

    sourceClock = CLOCK_GetFreq(DSPI_MASTER_CLK_SRC);
    /* If returns 0, indicates failed. */
    if (DSPI_MasterSetBaudRate((SPI_Type *)BOARD_SDSPI_SPI_BASE, DSPI_MASTER_CTAR, frequency, sourceClock))
    {
        return kStatus_Success;
    }

    return kStatus_Fail;
}

void spi_csActivePolarity(sdspi_cs_active_polarity_t polarity)
{
    if(polarity == kSDSPI_CsActivePolarityHigh)
    {
        DSPI_SetAllPcsPolarity((SPI_Type *)BOARD_SDSPI_SPI_BASE, 0U);
    }
    else
    {
        DSPI_SetAllPcsPolarity((SPI_Type *)BOARD_SDSPI_SPI_BASE, DSPI_MASTER_PCS_ACTIVE_LOW);
    }
}

status_t spi_exchange(uint8_t *in, uint8_t *out, uint32_t size)
{
    dspi_transfer_t masterTransfer;

    masterTransfer.txData = in;
    masterTransfer.rxData = out;
    masterTransfer.dataSize = size;
    masterTransfer.configFlags = (kDSPI_MasterCtar0 | DSPI_MASTER_PCS_TRANSFER | kDSPI_MasterPcsContinuous);
    return DSPI_MasterTransferBlocking((SPI_Type *)BOARD_SDSPI_SPI_BASE, &masterTransfer);
}


void sdspi_host_init(void)
{
    /* Saves host state and callback. */
    g_host.busBaudRate = DSPI_BUS_BAUDRATE;
    g_host.setFrequency = spi_set_frequency;
    g_host.exchange = spi_exchange;
    g_host.init     = spi_init;
    g_host.csActivePolarity         = spi_csActivePolarity;

    /* Saves card state. */
    g_card.host = &g_host;
}
#endif /* defined(SDSPI_DISK_ENABLE) && (SDSPI_DISK_ENABLE == 1 */
