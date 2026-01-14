/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __FSL_RAMDISK_H__
#define __FSL_RAMDISK_H__

#include "ff.h"
#include "diskio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*
 * The following macros (FSL_FF_RAMDISK_SECTOR_SIZE, FSL_FF_RAMDISK_DISK_SIZE)
 * can be redefined from the application, to facilitate a RAM disk with a
 * different size. If so desired, it is recommended to firstly define these
 * macros in "app_definitions.h" and then include this file, like so:
 *
 * #define FSL_FF_RAMDISK_DISK_SIZE mySize
 * #include "fsl_ram_disk.h"
 */

#ifndef FSL_FF_RAMDISK_SECTOR_SIZE
#define FSL_FF_RAMDISK_SECTOR_SIZE FF_MIN_SS     /* usually 512 B */
#endif

#ifndef FSL_FF_RAMDISK_DISK_SIZE
/* Minimal volume size (128 sectors) required in file ff.c in f_mkfs function,
   Minimal sectors per track required in file ff.c in f_mkfs function,
   N_SEC_TRACK (63 sectors) defined in file ff.c */
#define FSL_FF_RAMDISK_DISK_SIZE  (128 + 63) * FF_MIN_SS
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
 * API
 ******************************************************************************/
DSTATUS ram_disk_initialize(BYTE pdrv);
DSTATUS ram_disk_status(BYTE pdrv);
DRESULT ram_disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count);
DRESULT ram_disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count);
DRESULT ram_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff);

#if defined(__cplusplus)
}
#endif

#endif /* __FSL_RAMDISK_H__ */
