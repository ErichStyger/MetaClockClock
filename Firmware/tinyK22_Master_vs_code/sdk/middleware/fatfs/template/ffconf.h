/*----------------------------------------------------------------------------/
/  FatFs - Generic FAT Filesystem module  R0.15                               /
/-----------------------------------------------------------------------------/
/
/ Copyright (C) 2022, ChaN, all right reserved.
/
/ FatFs module is an open source software. Redistribution and use of FatFs in
/ source and binary forms, with or without modification, are permitted provided
/ that the following condition is met:

/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/
/----------------------------------------------------------------------------*/

#ifndef _FFCONF_H_
#define _FFCONF_H_

#include "ffconf_gen.h"

/* Handle macros required to be defined.
 * If these macros are not defined set them to 0.
 */

#if !defined(RAM_DISK_ENABLE)
#define RAM_DISK_ENABLE 0
#endif

#if !defined(USB_DISK_ENABLE)
#define USB_DISK_ENABLE 0
#endif

#if !defined(SD_DISK_ENABLE)
#define SD_DISK_ENABLE 0
#endif

#if !defined(MMC_DISK_ENABLE)
#define MMC_DISK_ENABLE 0
#endif

#if !defined(SDSPI_DISK_ENABLE)
#define SDSPI_DISK_ENABLE 0
#endif

#if !defined(NAND_DISK_ENABLE)
#define NAND_DISK_ENABLE 0
#endif

#if !defined(FF_FS_READONLY)
#define FF_FS_READONLY 0
#endif

#if !defined(FF_FS_MINIMIZE)
#define FF_FS_MINIMIZE 0
#endif

#if !defined(FF_USE_FIND)
#define FF_USE_FIND 0
#endif

#if !defined(FF_USE_MKFS)
#define FF_USE_MKFS 0
#endif

#if !defined(FF_USE_FASTSEEK)
#define FF_USE_FASTSEEK 0
#endif

#if !defined(FF_USE_EXPAND)
#define FF_USE_EXPAND 0
#endif

#if !defined(FF_USE_CHMOD)
#define FF_USE_CHMOD 0
#endif

#if !defined(FF_USE_LABEL)
#define FF_USE_LABEL 0
#endif

#if !defined(FF_USE_FORWARD)
#define FF_USE_FORWARD 0
#endif

#if !defined(FF_USE_STRFUNC)
#define FF_USE_STRFUNC 0
#endif

#if !defined(FF_PRINT_LLI)
#define FF_PRINT_LLI 0
#endif

#if !defined(FF_PRINT_FLOAT)
#define FF_PRINT_FLOAT 0
#endif

#if !defined(FF_STRF_ENCODE)
#define FF_STRF_ENCODE 0
#endif

#if !defined(FF_USE_LFN)
#define FF_USE_LFN 0
#endif

#if !defined(FF_LFN_UNICODE)
#define FF_LFN_UNICODE 0
#endif

#if !defined(FF_STR_VOLUME_ID)
#define FF_STR_VOLUME_ID 0
#endif

#if !defined(FF_MULTI_PARTITION)
#define FF_MULTI_PARTITION 0
#endif

#if !defined(FF_LBA64)
#define FF_LBA64 0
#endif

#if !defined(FF_USE_TRIM)
#define FF_USE_TRIM 0
#endif

#if !defined(FF_FS_TINY)
#define FF_FS_TINY 0
#endif

#if !defined(FF_FS_EXFAT)
#define FF_FS_EXFAT 0
#endif

#if !defined(FF_FS_NORTC)
#define FF_FS_NORTC 0
#endif

#if !defined(FF_NORTC_MON)
#define FF_NORTC_MON 0
#endif

#if !defined(FF_FS_REENTRANT)
#define FF_FS_REENTRANT 0
#endif

#if FF_FS_REENTRANT
#include "FreeRTOS.h"
#include "semphr.h"
#define FF_SYNC_t       SemaphoreHandle_t
#endif

#endif /* _FFCONF_H_ */
