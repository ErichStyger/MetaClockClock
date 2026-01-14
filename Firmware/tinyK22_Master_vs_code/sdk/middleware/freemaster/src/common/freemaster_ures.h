/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2018-2020, 2024 NXP
 *
 * License: NXP LA_OPT_Online Code Hosting NXP_Software_License
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may
 * only be used strictly in accordance with the applicable license terms.
 * By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that
 * you have read, and that you agree to comply with and are bound by,
 * such license terms.  If you do not agree to be bound by the applicable
 * license terms, then you may not retain, install, activate or otherwise
 * use the software.
 *
 * FreeMASTER Communication Driver - User Resource handling
 */

#ifndef __FREEMASTER_URES_H
#define __FREEMASTER_URES_H

/******************************************************************************
 * Macro definitions
 ******************************************************************************/

/* User resources operation code */
#define FMSTR_URES_OP_CODE    FMSTR_SIZE
#define FMSTR_URES_IOCTL_CODE FMSTR_SIZE

/* Operation codes */
#define FMSTR_URES_OP_READ  0x00U
#define FMSTR_URES_OP_WRITE 0x01U
#define FMSTR_URES_OP_IOCTL 0x02U

/* IOCTL codes */
#define FMSTR_URES_IOCTL_GET_BUSY     0x00U
#define FMSTR_URES_IOCTL_WRITE_FLUSH  0x01U
#define FMSTR_URES_IOCTL_GET_ACCESS   0x02U
#define FMSTR_URES_IOCTL_GET_SIZE     0x04U
#define FMSTR_URES_IOCTL_SET_SIZE     0x05U
#define FMSTR_URES_IOCTL_GET_MAX_SIZE 0x06U
#define FMSTR_URES_IOCTL_ERASE        0x07U
#define FMSTR_URES_IOCTL_BLANK_CHECK  0x08U
#define FMSTR_URES_IOCTL_HASH         0x0AU
#define FMSTR_URES_IOCTL_GET_BLKINFO  0x0CU

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Types definitions
 ******************************************************************************/

/* In/Out buffer structure for user resources function */
typedef struct
{
    FMSTR_BPTR buff;         /* In/out buffer */
    FMSTR_SIZE sizeIn;       /* Input size */
    FMSTR_SIZE sizeConsumed; /* Consumed size */
    FMSTR_SIZE sizeOut;      /* Output size */
    FMSTR_SIZE sizeOutMax;   /* Maximum output size */

    FMSTR_ADDR offset;               /* read/write offset */
    FMSTR_URES_IOCTL_CODE ioctlCode; /* IOCTL code */
} FMSTR_RWI_BUFF;

/* User resources function, which is used in TSA table in special entry. */
typedef FMSTR_BOOL (*FMSTR_URES_HANDLER_FUNC)(FMSTR_URES_OP_CODE opCode, FMSTR_RWI_BUFF *buffer, void *param);

/******************************************************************************
 * Global API functions
 ******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __FREEMASTER_URES_H */
