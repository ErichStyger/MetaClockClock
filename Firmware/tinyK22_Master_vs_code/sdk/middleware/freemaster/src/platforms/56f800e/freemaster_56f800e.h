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
 * FreeMASTER Communication Driver - DSC 56F800E platform specific code
 */

#ifndef _FREEMASTER_56F800E_H
#define _FREEMASTER_56F800E_H

#include <string.h>
#include <stdlib.h>

/******************************************************************************
 * platform-specific default configuration
 ******************************************************************************/

#define FMSTR_CFG_BUS_WIDTH       1U /* all addressing uses .bp byte-addressing mode on 56F800e family */
#define FMSTR_TSA_FLAGS           FMSTR_TSA_INFO_HV2BA
#define FMSTR_PLATFORM_BIG_ENDIAN 0U
#define FMSTR_MEMCPY_MAX_SIZE     4U
#define FMSTR_REC_STRUCT_ALIGN    8U

#ifndef FMSTR_PLATFORM_BASE_ADDRESS
/* #define FMSTR_PLATFORM_BASE_ADDRESS 0 */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Types definition
 ******************************************************************************/

/* FreeMASTER types used */
#if __has_feature(FE_POINTER_MODIFIERS)
typedef unsigned char *__attribute__((far)) FMSTR_ADDR; /* Use far pointer in all models */
#else
typedef unsigned char *FMSTR_ADDR; /* Use pointers according to model */
#endif

typedef unsigned int FMSTR_SIZE;    /* general size type (at least 16 bits) */
typedef unsigned int FMSTR_SIZE8;   /* one-byte size value */
typedef unsigned long FMSTR_SIZE32; /* general size type (at least size of address) */
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

#if __has_feature(FE_POINTER_MODIFIERS)
/* TSA object pointer will also use far if possible */
#define FMSTR_TSATBL_VOIDPTR volatile const void * __attribute__((far))
#endif

#ifdef __cplusplus
}
#endif

/******************************************************************************
 * If possible we use 'far' declaration of long pointer type so this driver
 * is able to access far variables (beyond word address 0x8000 (= .bp 0x10000).
 * Also the Recorder buffers may be located in the far memory.
 ******************************************************************************/

#if __has_feature(FE_POINTER_MODIFIERS)
/* The compiler supports the 'far' attribute. We use it in our FMSTR_LP_xx types. */
#define FMSTR_TYPEDEF_LPTR(type, ptrtype) typedef type *__attribute__((far)) ptrtype
#else
/* Default pointer type will be used according to LDM or SDM model. */
#if defined(__SDM__)
/* In SDM, be aware that the FreeMASTER driver will only be able to access a memory
 * in word address range 0x0000..0x8000. Update the CodeWarrior compiler to the latest
 * version in order to get a 'far' attribute support. */
/* #warning Compiler does not support 'far' pointers. Address range visible to FreeMASTER is limited in SDM. */
#endif
#endif

#if 0 /* Uncomment to take standard C functions taken from stdlib */
#define FMSTR_StrLen(str)              ((FMSTR_SIZE)strlen(str))
#define FMSTR_StrCmp(str1, str2)       ((FMSTR_INDEX)strcmp(str1, str2))
#define FMSTR_MemCmp(b1, b2, size)     ((FMSTR_INDEX)memcmp(b1, b2, size))
#define FMSTR_MemSet(dest, mask, size) memset(dest, mask, size)
#endif

/* Rand is taken from stdlib */
#define FMSTR_Rand() rand()

#endif /* _FREEMASTER_56F800E_H */
