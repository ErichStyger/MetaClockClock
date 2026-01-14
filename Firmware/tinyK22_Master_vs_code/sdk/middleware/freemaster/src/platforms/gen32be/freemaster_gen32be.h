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
 * FreeMASTER Communication Driver - Generic 32-bit big-endian platform specific code
 */

#ifndef _FREEMASTER_GEN32BE_H
#define _FREEMASTER_GEN32BE_H

#include <string.h>
#include <stdlib.h>

/******************************************************************************
 * platform-specific default configuration
 ******************************************************************************/

#define FMSTR_CFG_BUS_WIDTH       1U
#define FMSTR_TSA_FLAGS           0U
#define FMSTR_PLATFORM_BIG_ENDIAN 1U
#define FMSTR_MEMCPY_MAX_SIZE     4U

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Types definition
 ******************************************************************************/

/* FreeMASTER types used */
typedef unsigned char *FMSTR_ADDR;  /* CPU address type (4bytes) */
typedef unsigned int FMSTR_SIZE;    /* general size type (at least 16 bits) */
typedef unsigned char FMSTR_SIZE8;  /* one-byte size value */
typedef unsigned long FMSTR_SIZE32; /* general size type (at least size of address (typicaly 32bits)) */
typedef unsigned int FMSTR_BOOL;    /* general boolean type  */

typedef unsigned char FMSTR_U8;       /* smallest memory entity */
typedef unsigned short FMSTR_U16;     /* 16bit value */
typedef unsigned long FMSTR_U32;      /* 32bit value */
typedef unsigned long long FMSTR_U64; /* 64bit value */

typedef signed char FMSTR_S8;       /* signed 8bit value */
typedef signed short FMSTR_S16;     /* signed 16bit value */
typedef signed long FMSTR_S32;      /* signed 32bit value */
typedef signed long long FMSTR_S64; /* signed 64bit value */

typedef float FMSTR_FLOAT;   /* float value */
typedef double FMSTR_DOUBLE; /* double value */

typedef unsigned char FMSTR_FLAGS; /* type to be union-ed with flags (at least 8 bits) */
typedef signed int FMSTR_INDEX;    /* general for-loop index (must be signed) */

typedef unsigned char FMSTR_BCHR;  /* type of a single character in comm.buffer */
typedef unsigned char *FMSTR_BPTR; /* pointer within a communication buffer */

typedef char FMSTR_CHAR; /* regular character, part of string */

#ifdef __cplusplus
}
#endif

/*********************************************************************************
 * Platform depending functionalities
 *********************************************************************************/

#if 0 /* Uncomment to take standard C functions taken from stdlib */
#define FMSTR_StrLen(str)              ((FMSTR_SIZE)strlen(str))
#define FMSTR_StrCmp(str1, str2)       ((FMSTR_INDEX)strcmp(str1, str2))
#define FMSTR_MemCmp(b1, b2, size)     ((FMSTR_INDEX)memcmp(b1, b2, size))
#define FMSTR_MemSet(dest, mask, size) memset(dest, mask, size)
#endif

/* Rand is taken from stdlib */
#define FMSTR_Rand() rand()

#endif /* _FREEMASTER_GEN32BE_H */
