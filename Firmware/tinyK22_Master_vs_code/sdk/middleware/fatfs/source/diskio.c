/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include "ffconf.h"     /* FatFs configuration options */
#include "ff.h"         /* Obtains integer types */
#include "diskio.h"     /* Declarations of disk functions */

#if defined(RAM_DISK_ENABLE) && (RAM_DISK_ENABLE == 1)
#include "fsl_ram_disk.h"
#endif

#if defined(USB_DISK_ENABLE) && (USB_DISK_ENABLE == 1)
#include "fsl_usb_disk.h"
#endif

#if defined(SD_DISK_ENABLE) && (SD_DISK_ENABLE == 1)
#include "fsl_sd_disk.h"
#endif

#if defined(MMC_DISK_ENABLE) && (MMC_DISK_ENABLE == 1)
#include "fsl_mmc_disk.h"
#endif

#if defined(SDSPI_DISK_ENABLE) && (SDSPI_DISK_ENABLE == 1)
#include "fsl_sdspi_disk.h"
#endif

#if defined(NAND_DISK_ENABLE) && (NAND_DISK_ENABLE == 1)
#include "fsl_nand_disk.h"
#endif

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv        /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS stat;
    switch (pdrv)
    {
#if defined(RAM_DISK_ENABLE) && (RAM_DISK_ENABLE == 1)
        case RAMDISK:
            stat = ram_disk_status(pdrv);
            return stat;
#endif
#if defined(USB_DISK_ENABLE) && (USB_DISK_ENABLE == 1)
        case USBDISK:
            stat = USB_HostMsdGetDiskStatus(pdrv);
            return stat;
#endif
#if defined(SD_DISK_ENABLE) && (SD_DISK_ENABLE == 1)
        case SDDISK:
            stat = sd_disk_status(pdrv);
            return stat;
#endif
#if defined(MMC_DISK_ENABLE) && (MMC_DISK_ENABLE == 1)
        case MMCDISK:
            stat = mmc_disk_status(pdrv);
            return stat;
#endif
#if defined(SDSPI_DISK_ENABLE) && (SDSPI_DISK_ENABLE == 1)
        case SDSPIDISK:
            stat = sdspi_disk_status(pdrv);
            return stat;
#endif
#if defined(NAND_DISK_ENABLE) && (NAND_DISK_ENABLE == 1)
        case NANDDISK:
            stat = nand_disk_status(pdrv);
            return stat;
#endif
        default:
            break;
    }
    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv                /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS stat;
    switch (pdrv)
    {
#if defined(RAM_DISK_ENABLE) && (RAM_DISK_ENABLE == 1)
        case RAMDISK:
            stat = ram_disk_initialize(pdrv);
            return stat;
#endif
#if defined(USB_DISK_ENABLE) && (USB_DISK_ENABLE == 1)
        case USBDISK:
            stat = USB_HostMsdInitializeDisk(pdrv);
            return stat;
#endif
#if defined(SD_DISK_ENABLE) && (SD_DISK_ENABLE == 1)
        case SDDISK:
            stat = sd_disk_initialize(pdrv);
            return stat;
#endif
#if defined(MMC_DISK_ENABLE) && (MMC_DISK_ENABLE == 1)
        case MMCDISK:
            stat = mmc_disk_initialize(pdrv);
            return stat;
#endif
#if defined(SDSPI_DISK_ENABLE) && (SDSPI_DISK_ENABLE == 1)
        case SDSPIDISK:
            stat = sdspi_disk_initialize(pdrv);
            return stat;
#endif

#if defined(NAND_DISK_ENABLE) && (NAND_DISK_ENABLE == 1)
        case NANDDISK:
            stat = nand_disk_initialize(pdrv);
            return stat;
#endif
        default:
            break;
    }
    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,        /* Physical drive nmuber to identify the drive */
    BYTE *buff,        /* Data buffer to store read data */
    LBA_t sector,    /* Start sector in LBA */
    UINT count        /* Number of sectors to read */
)
{
    DRESULT res;
    switch (pdrv)
    {
#if defined(RAM_DISK_ENABLE) && (RAM_DISK_ENABLE == 1)
        case RAMDISK:
            res = ram_disk_read(pdrv, buff, sector, count);
            return res;
#endif
#if defined(USB_DISK_ENABLE) && (USB_DISK_ENABLE == 1)
        case USBDISK:
            res = USB_HostMsdReadDisk(pdrv, buff, sector, count);
            return res;
#endif
#if defined(SD_DISK_ENABLE) && (SD_DISK_ENABLE == 1)
        case SDDISK:
            res = sd_disk_read(pdrv, buff, sector, count);
            return res;
#endif
#if defined(MMC_DISK_ENABLE) && (MMC_DISK_ENABLE == 1)
        case MMCDISK:
            res = mmc_disk_read(pdrv, buff, sector, count);
            return res;
#endif
#if defined(SDSPI_DISK_ENABLE) && (SDSPI_DISK_ENABLE == 1)
        case SDSPIDISK:
            res = sdspi_disk_read(pdrv, buff, sector, count);
            return res;
#endif

#if defined(NAND_DISK_ENABLE) && (NAND_DISK_ENABLE == 1)
        case NANDDISK:
            res = nand_disk_read(pdrv, buff, sector, count);
            return res;
#endif
        default:
            break;
    }

    return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0
DRESULT disk_write (
    BYTE pdrv,            /* Physical drive nmuber to identify the drive */
    const BYTE *buff,    /* Data to be written */
    LBA_t sector,        /* Start sector in LBA */
    UINT count            /* Number of sectors to write */
)
{
    DRESULT res;
    switch (pdrv)
    {
#if defined(RAM_DISK_ENABLE) && (RAM_DISK_ENABLE == 1)
        case RAMDISK:
            res = ram_disk_write(pdrv, buff, sector, count);
            return res;
#endif
#if defined(USB_DISK_ENABLE) && (USB_DISK_ENABLE == 1)
        case USBDISK:
            res = USB_HostMsdWriteDisk(pdrv, buff, sector, count);
            return res;
#endif
#if defined(SD_DISK_ENABLE) && (SD_DISK_ENABLE == 1)
        case SDDISK:
            res = sd_disk_write(pdrv, buff, sector, count);
            return res;
#endif
#if defined(MMC_DISK_ENABLE) && (MMC_DISK_ENABLE == 1)
        case MMCDISK:
            res = mmc_disk_write(pdrv, buff, sector, count);
            return res;
#endif
#if defined(SDSPI_DISK_ENABLE) && (SDSPI_DISK_ENABLE == 1)
        case SDSPIDISK:
            res = sdspi_disk_write(pdrv, buff, sector, count);
            return res;
#endif

#if defined(NAND_DISK_ENABLE) && (NAND_DISK_ENABLE == 1)
        case NANDDISK:
            res = nand_disk_write(pdrv, buff, sector, count);
            return res;
#endif
        default:
            break;
    }
    return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE pdrv,        /* Physical drive nmuber (0..) */
    BYTE cmd,        /* Control code */
    void *buff        /* Buffer to send/receive control data */
)
{
    DRESULT res;
    switch (pdrv)
    {
#if defined(RAM_DISK_ENABLE) && (RAM_DISK_ENABLE == 1)
        case RAMDISK:
            res = ram_disk_ioctl(pdrv, cmd, buff);
            return res;
#endif
#if defined(USB_DISK_ENABLE) && (USB_DISK_ENABLE == 1)
        case USBDISK:
            res = USB_HostMsdIoctlDisk(pdrv, cmd, buff);
            return res;
#endif
#if defined(SD_DISK_ENABLE) && (SD_DISK_ENABLE == 1)
        case SDDISK:
            res = sd_disk_ioctl(pdrv, cmd, buff);
            return res;
#endif
#if defined(MMC_DISK_ENABLE) && (MMC_DISK_ENABLE == 1)
        case MMCDISK:
            res = mmc_disk_ioctl(pdrv, cmd, buff);
            return res;
#endif
#if defined(SDSPI_DISK_ENABLE) && (SDSPI_DISK_ENABLE == 1)
        case SDSPIDISK:
            res = sdspi_disk_ioctl(pdrv, cmd, buff);
            return res;
#endif

#if defined(NAND_DISK_ENABLE) && (NAND_DISK_ENABLE == 1)
        case NANDDISK:
            res = nand_disk_ioctl(pdrv, cmd, buff);
            return res;
#endif
        default:
            break;
    }
    return RES_PARERR;
}

