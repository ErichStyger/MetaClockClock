/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2018-2021, 2024 NXP
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
 * FreeMASTER Communication Driver - Recorder API
 */

#ifndef __FREEMASTER_REC_H
#define __FREEMASTER_REC_H

#ifndef __FREEMASTER_H
#error Please include freemaster_rec.h after freemaster.h main header file.
#endif

/***********************************
*  global recorder types
***********************************/
/* Recorder variable trigger mode settings */
#define FMSTR_REC_TRG_TYPE_MASK         0x03U        /* Recorder trigger type - bit mask */
#define FMSTR_REC_TRG_TYPE_NO_TRIGGER   0x00U        /* Not used as trigger */
#define FMSTR_REC_TRG_TYPE_UINT         0x01U        /* Recorder trigger type - unsigned integer */
#define FMSTR_REC_TRG_TYPE_SINT         0x02U        /* Recorder trigger type - signed integer */
#define FMSTR_REC_TRG_TYPE_FLOAT        0x03U        /* Recorder trigger type - float */

#define FMSTR_REC_TRG_F_TRGONLY         0x04U    /* Recorder trigger configuration - variable is used just for triggering */
#define FMSTR_REC_TRG_F_ABOVE           0x10U    /* Recorder trigger configuration - trigger is active for threshold above results */
#define FMSTR_REC_TRG_F_BELOW           0x20U    /* Recorder trigger configuration - trigger is active for threshold below results */
#define FMSTR_REC_TRG_F_LEVEL           0x40U    /* Recorder trigger configuration - trigger is level active, otherwise edge */
#define FMSTR_REC_TRG_F_VARTHR          0x80U    /* Recorder trigger configuration - trigger has fixed threshold value */

#ifdef __cplusplus
  extern "C" {
#endif

/* Description of recorder instance buffer */
typedef struct
{
    FMSTR_ADDR  addr;           /* address of buffer for recorder instance */
    FMSTR_U32   basePeriod_ns;  /* Base speed of recorder sampling in nanoseconds. Client may request to sample at multiples of this value. */
    FMSTR_SIZE  size;           /* size of buffer for recorder instance */
    FMSTR_CHAR *name;           /* String description of recorder sampling point etc. (e.g. "PWM Reload Interrupt", or "Timer interrupt"). */
} FMSTR_REC_BUFF;

/* Recorder description of one variable */
typedef struct
{
    FMSTR_ADDR  addr;           /* address of recorded variable */
    FMSTR_ADDR  trgAddr;        /* address of trigger variable to compare thresholds */
    FMSTR_SIZE8 size;           /* size of recorded variable */
    FMSTR_SIZE8 triggerMode;    /* trigger mode */
} FMSTR_REC_VAR;

/* configuration variables */
typedef struct
{
    FMSTR_SIZE  totalSmps;      /* number of samples to measure */
    FMSTR_SIZE  preTrigger;     /* number of pre-trigger samples to keep */
    FMSTR_SIZE  timeDiv;        /* divisor of recorder "clock" */
    FMSTR_SIZE8 varCount;       /* number of active recorder variables */
} FMSTR_REC_CFG;

#ifdef __cplusplus
  }
#endif

/***********************************
*  global recorder variables
***********************************/


#endif /* __FREEMASTER_REC_H */
