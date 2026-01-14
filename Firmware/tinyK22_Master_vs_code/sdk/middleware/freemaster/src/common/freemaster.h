/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2018-2021, 2024-2025 NXP
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
 * FreeMASTER Communication Driver - master header file
 */

#ifndef __FREEMASTER_H
#define __FREEMASTER_H

/******************************************************************************
 * Include user configuration file and define default configuration parameters
 ******************************************************************************/

#include "freemaster_defcfg.h"

/******************************************************************************
 * Include Platform-dependent file
 ******************************************************************************/

#if FMSTR_PLATFORM_CORTEX_M
/* CortexM is a generic 32bit little-endian platform */
#define FMSTR_PLATFORM "Cortex-M"
#include "freemaster_gen32le.h"
#elif FMSTR_PLATFORM_S12Z
/* CortexM is a generic 32bit little-endian platform */
#define FMSTR_PLATFORM "S12Z"
#include "freemaster_s12z.h"
#elif FMSTR_PLATFORM_S32
#define FMSTR_PLATFORM "S32"
#include "freemaster_gen32le.h"
#elif FMSTR_PLATFORM_PA32
#define FMSTR_PLATFORM "PA32"
#include "freemaster_gen32be.h"
#elif FMSTR_PLATFORM_56F800E
#define FMSTR_PLATFORM "56F800E"
#include "freemaster_56f800e.h"
#elif FMSTR_PLATFORM_ZEPHYR
/* Zephyr platform uses CPU settings from Zephyr configuration */
#define FMSTR_PLATFORM "ZEPHYR"
#include "freemaster_zephyr.h"
#else
/* If you are looking for legacy Motorola and Freescale platforms like 56F800,
   HC08, HC12, ColdFire, please refer to FreeMASTER driver v2.0 available at
   www.nxp.com/freemaster */
#warning Unknown FreeMASTER driver platform, using default settings for generic 32bit platform

/* Use default settings */
#define FMSTR_PLATFORM "Generic-32LE"
#include "freemaster_gen32le.h"
#endif

/*****************************************************************************
 * Global types, platform independent
 ******************************************************************************/

#ifndef FMSTR_TRUE
#define FMSTR_TRUE (1U)
#endif

#ifndef FMSTR_FALSE
#define FMSTR_FALSE (0U)
#endif

/* Pipe usage modes */
#define FMSTR_PIPE_MODE_CONSOLE 0x00U
#define FMSTR_PIPE_MODE_UINT    0x04U
#define FMSTR_PIPE_MODE_INT     0x08U
#define FMSTR_PIPE_MODE_REAL    0x0CU

/* Pipe element size */
#define FMSTR_PIPE_SIZE_1B 0x00U
#define FMSTR_PIPE_SIZE_2B 0x01U
#define FMSTR_PIPE_SIZE_4B 0x02U
#define FMSTR_PIPE_SIZE_8B 0x03U

/* Pipe type constants to be used with FMSTR_PipeOpen() */
#define FMSTR_PIPE_TYPE_ANSI_TERMINAL    (FMSTR_PIPE_MODE_CONSOLE | FMSTR_PIPE_SIZE_1B)
#define FMSTR_PIPE_TYPE_UNICODE_TERMINAL (FMSTR_PIPE_MODE_CONSOLE | FMSTR_PIPE_SIZE_2B)

/******************************************************************************
 * Types definition
 ******************************************************************************/

 #ifdef __cplusplus
extern "C" {
#endif

/* application command-specific types */
typedef unsigned char FMSTR_APPCMD_CODE;
typedef unsigned char FMSTR_APPCMD_DATA, *FMSTR_APPCMD_PDATA;
typedef unsigned char FMSTR_APPCMD_RESULT;

/* pointer to application command callback handler */
typedef FMSTR_APPCMD_RESULT (*FMSTR_PAPPCMDFUNC)(FMSTR_APPCMD_CODE code, FMSTR_APPCMD_PDATA pdata, FMSTR_SIZE size);

/* pipe-related types */
typedef void *FMSTR_HPIPE;              /* pipe handle */
typedef unsigned short FMSTR_PIPE_PORT; /* pipe port identifier (unsigned, 7 bits used) */
typedef unsigned short FMSTR_PIPE_SIZE; /* pipe buffer size type (unsigned, at least 16 bits) */

/* pointer to pipe event handler */
typedef void (*FMSTR_PPIPEFUNC)(FMSTR_HPIPE pipeHandle);

/* PDBDM buffer type, it is in a public header as user may want to define a custom buffer instance
  (when FMSTR_PDBDM_USER_BUFFER is defined) */
typedef struct
{
    FMSTR_U32   ctxStart;                   /* Border code of Packet driven BDM communication buffer */
    FMSTR_U8    bdmState;                   /* State of the Packet driven BDM communication */
    FMSTR_U8    pcktSize;                   /* Size of current buffer. */
    FMSTR_U8    padding0;                   /* Padding to align buffer by 4. */
    FMSTR_U8    cmdStatus;                  /* Command status byte */
    FMSTR_BCHR  commBuffer[FMSTR_COMM_BUFFER_SIZE]; /*FreeMASTER Packet driven BDM communication buffer (in/out) plus the STS */
    FMSTR_U32   reservedForCrc;             /* Reserved to store additional checking informations in communication in case that full length of buffer is used. */
    FMSTR_U32   ctxEnd;                     /* Border code of Packet driven BDM communication buffer */
} FMSTR_PDBDM_COMBUFF;

#ifdef __cplusplus
}
#endif

/*****************************************************************************
 * FreeMASTER user types and macros
 ******************************************************************************/

#include "freemaster_tsa.h"
#include "freemaster_rec.h"
#include "freemaster_ures.h"

/*****************************************************************************
 * Constants
 ******************************************************************************/

/* Application command status information  */
#define FMSTR_APPCMDRESULT_NOCMD     0xffU
#define FMSTR_APPCMDRESULT_RUNNING   0xfeU
#define MFSTR_APPCMDRESULT_LASTVALID 0xf7U /* F8-FF are reserved  */

/* Recorder time base is reported in nanoseconds in protocol V4 */
#define FMSTR_REC_BASE_SECONDS(x)  (((FMSTR_U32)(x)) * 1000000000UL)
#define FMSTR_REC_BASE_MILLISEC(x) (((FMSTR_U32)(x)) * 1000000UL)
#define FMSTR_REC_BASE_MICROSEC(x) (((FMSTR_U32)(x)) * 1000UL)
#define FMSTR_REC_BASE_NANOSEC(x)  ((FMSTR_U32)(x))

/* The Access Protection levels */
#define FMSTR_RESTRICTED_ACCESS_NO  0U
#define FMSTR_RESTRICTED_ACCESS_R   1U
#define FMSTR_RESTRICTED_ACCESS_RW  2U
#define FMSTR_RESTRICTED_ACCESS_RWF 3U

/* The Authentication protocols supported */
#define FMSTR_AUTHENT_PRTCL_SHA1 0x01U

#define FMSTR_AUTHENT_PRTCL_SHA1_SALT_LEN 16U

/******************************************************************************
 * NULL needed
 ******************************************************************************/

#ifndef NULL
#ifdef __cplusplus
#define NULL (0)
#else
#define NULL ((void *)0)
#endif
#endif

/*****************************************************************************
 * ASSERT functionality
 ******************************************************************************/

#if defined(DEBUG) || (FMSTR_DEBUG_LEVEL > 0)
#include <assert.h>
#define FMSTR_ASSERT(condition) assert(condition)
#else
#define FMSTR_ASSERT(condition)
#endif

#define FMSTR_ASSERT_RETURN(condition, ret) \
    do                                      \
    {                                       \
        FMSTR_ASSERT(condition);            \
        if (!(condition))                   \
        {                                   \
            return ret;                     \
        }                                   \
    } while (0 == 1)

/******************************************************************************
 * standard library functions; redirected to internal implementations by default.
 ******************************************************************************/

/* Platform header may override the defines and use custom implementations */
#ifndef FMSTR_StrCmp
#define FMSTR_StrCmp _FMSTR_StrCmp
#endif

#ifndef FMSTR_StrLen
#define FMSTR_StrLen _FMSTR_StrLen
#endif

#ifndef FMSTR_Rand
#define FMSTR_Rand _FMSTR_Rand
#endif

#ifndef FMSTR_MemSet
#define FMSTR_MemSet _FMSTR_MemSet
#endif

#ifndef FMSTR_MemCmp
#define FMSTR_MemCmp _FMSTR_MemCmp
#endif

#ifndef FMSTR_MemCpy
#define FMSTR_MemCpy _FMSTR_MemCpy
#endif

/*****************************************************************************
 * Global functions
 ******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/* FreeMASTER serial communication API */
FMSTR_BOOL FMSTR_Init(void); /* general initialization */
void FMSTR_Poll(void);       /* polling call, use in SHORT_INTR and POLL_DRIVEN modes */

/* Recorder API */
FMSTR_BOOL FMSTR_RecorderCreate(FMSTR_INDEX recIndex, FMSTR_REC_BUFF *buffCfg);
FMSTR_BOOL FMSTR_RecorderSetTimeBase(FMSTR_INDEX recIndex, FMSTR_U32 timeBase_ns);
FMSTR_BOOL FMSTR_RecorderConfigure(FMSTR_INDEX recIndex, FMSTR_REC_CFG *recCfg);
FMSTR_BOOL FMSTR_RecorderAddVariable(FMSTR_INDEX recIndex, FMSTR_INDEX recVarIx, FMSTR_REC_VAR *recVarCfg);
FMSTR_BOOL FMSTR_RecorderStart(FMSTR_INDEX recIndex);
FMSTR_BOOL FMSTR_RecorderTrigger(FMSTR_INDEX recIndex);
FMSTR_BOOL FMSTR_RecorderAbort(FMSTR_INDEX recIndex);
void FMSTR_Recorder(FMSTR_INDEX recIndex);

/* Application commands API */
FMSTR_APPCMD_CODE FMSTR_GetAppCmd(void);
FMSTR_APPCMD_PDATA FMSTR_GetAppCmdData(FMSTR_SIZE *dataLen);
FMSTR_BOOL FMSTR_RegisterAppCmdCall(FMSTR_APPCMD_CODE appCmdCode, FMSTR_PAPPCMDFUNC callbackFunc);

void FMSTR_AppCmdAck(FMSTR_APPCMD_RESULT resultCode);
void FMSTR_AppCmdSetResponseData(FMSTR_ADDR resultDataAddr, FMSTR_SIZE resultDataLen);

/* Dynamic TSA API */
FMSTR_BOOL FMSTR_SetUpTsaBuff(FMSTR_ADDR buffAddr, FMSTR_SIZE buffSize);
FMSTR_BOOL FMSTR_TsaAddVar(FMSTR_TSATBL_STRPTR tsaName,
                           FMSTR_TSATBL_STRPTR tsaType,
                           FMSTR_TSATBL_VOIDPTR varAddr,
                           FMSTR_SIZE32 varSize,
                           FMSTR_SIZE flags);

/* Loss-less Communication Pipes API */
FMSTR_HPIPE FMSTR_PipeOpen(FMSTR_PIPE_PORT pipePort,
                           FMSTR_PPIPEFUNC pipeCallback,
                           FMSTR_ADDR pipeRxBuff,
                           FMSTR_PIPE_SIZE pipeRxSize,
                           FMSTR_ADDR pipeTxBuff,
                           FMSTR_PIPE_SIZE pipeTxSize,
                           FMSTR_U8 type,
                           const FMSTR_CHAR *name);
void FMSTR_PipeClose(FMSTR_HPIPE pipeHandle);
FMSTR_PIPE_SIZE FMSTR_PipeWrite(FMSTR_HPIPE pipeHandle,
                                FMSTR_ADDR pipeData,
                                FMSTR_PIPE_SIZE pipeDataLen,
                                FMSTR_PIPE_SIZE writeGranularity);
FMSTR_PIPE_SIZE FMSTR_PipeRead(FMSTR_HPIPE pipeHandle,
                               FMSTR_ADDR pipeData,
                               FMSTR_PIPE_SIZE pipeDataLen,
                               FMSTR_PIPE_SIZE readGranularity);

/* Pipe printing and formatting */
FMSTR_BOOL FMSTR_PipePuts(FMSTR_HPIPE pipeHandle, const char *text);
FMSTR_BOOL FMSTR_PipePrintf(FMSTR_HPIPE pipeHandle, const char *format, ...);
FMSTR_BOOL FMSTR_PipePrintfU8(FMSTR_HPIPE pipeHandle, const char *format, FMSTR_U8 arg);
FMSTR_BOOL FMSTR_PipePrintfS8(FMSTR_HPIPE pipeHandle, const char *format, FMSTR_S8 arg);
FMSTR_BOOL FMSTR_PipePrintfU16(FMSTR_HPIPE pipeHandle, const char *format, FMSTR_U16 arg);
FMSTR_BOOL FMSTR_PipePrintfS16(FMSTR_HPIPE pipeHandle, const char *format, FMSTR_S16 arg);
FMSTR_BOOL FMSTR_PipePrintfU32(FMSTR_HPIPE pipeHandle, const char *format, FMSTR_U32 arg);
FMSTR_BOOL FMSTR_PipePrintfS32(FMSTR_HPIPE pipeHandle, const char *format, FMSTR_S32 arg);

/* Standard library internal implementations. Platform header makes them available using macro aliases. */
FMSTR_INDEX _FMSTR_StrCmp(const FMSTR_CHAR *str1, const FMSTR_CHAR *str2);
FMSTR_INDEX _FMSTR_MemCmp(const void *b1, const void *b2, FMSTR_SIZE size);
FMSTR_SIZE _FMSTR_StrLen(const FMSTR_CHAR *str);
void _FMSTR_MemCpy(void *dest, const void *src, FMSTR_SIZE size);
void _FMSTR_MemSet(void *dest, FMSTR_U8 fill, FMSTR_SIZE size);
FMSTR_U32 _FMSTR_Rand(void);

#ifdef __cplusplus
}
#endif

#endif /* __FREEMASTER_H */
