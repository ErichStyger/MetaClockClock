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
 * FreeMASTER Communication Driver - private API
 */

#ifndef __FREEMASTER_PRIVATE_H
#define __FREEMASTER_PRIVATE_H

#ifndef __FREEMASTER_H
#error Please include the freemaster.h master header file before the freemaster_private.h
#endif

#ifndef __FREEMASTER_TSA_H
#error Please include the freemaster_tsa.h header file before the freemaster_private.h
#endif

/******************************************************************************
 * Inline functions
 ******************************************************************************/

/* Inline function declaration */
#ifndef FMSTR_INLINE
#ifdef __CSMC__ /* Cosmic */
#define FMSTR_INLINE static
#else
#define FMSTR_INLINE static inline
#endif
#endif

/* Weak function declaration */
#ifndef FMSTR_WEAK
#if defined(__ICCARM) && __ICCARM && !__ICCARM_V8
#define FMSTR_WEAK _Pragma("__weak")
#elif defined(__S12Z__) || defined(__CSMC__)
#define FMSTR_WEAK
#else
#define FMSTR_WEAK __attribute__((weak))
#endif
#endif

/* C99 structure member initialization */
#if __STDC_VERSION__ >= 199901L
#define FMSTR_C99_INIT(member) .member =
#else
#define FMSTR_C99_INIT(member)
#endif

/* Default address NULL checking as inline function for assert/MISRA compatibility */
#ifndef FMSTR_ADDR_VALID
#define FMSTR_ADDR_VALID FMSTR_ADDR_VALID_Func
FMSTR_INLINE FMSTR_BOOL FMSTR_ADDR_VALID_Func(FMSTR_ADDR addr)
{
    /* This function shall be overriden if FMSTR_ADDR is not a pointer type */
    return addr != NULL ? FMSTR_TRUE : FMSTR_FALSE;
}
#endif

/* Default address casting as inline function for MISRA isolation */
#ifndef FMSTR_CAST_PTR_TO_ADDR
#define FMSTR_CAST_PTR_TO_ADDR FMSTR_CAST_PTR_TO_ADDR_Func
FMSTR_INLINE FMSTR_ADDR FMSTR_CAST_PTR_TO_ADDR_Func(const void *ptr)
{
    /* Coverity: Intentional cast of a const pointer to generic address type */
    /* coverity[misra_c_2012_rule_11_8_violation:FALSE] */
    return (FMSTR_ADDR)ptr;
}
#endif

/* Default address casting as inline function for MISRA isolation */
#ifndef FMSTR_CAST_ADDR_TO_PTR
#define FMSTR_CAST_ADDR_TO_PTR FMSTR_CAST_ADDR_TO_PTR_Func
FMSTR_INLINE void *FMSTR_CAST_ADDR_TO_PTR_Func(FMSTR_ADDR addr)
{
    /* Coverity: Intentional cast of a generic address type to a void pointer */
    /* coverity[misra_c_2012_rule_11_8_violation:FALSE] */
    return (void *)addr;
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Long pointer type declarations. By default, all FMSTR_LP_xx types are just
 * regular pointers, but platforms like 56F800E may require to use __far
 * declaration. Note that LP pointers are only used to access user variables
 * and Recorder memory. The rest of FreeMASTER driver uses conventional pointer
 * types to internal data structures.
 ******************************************************************************/

#ifndef FMSTR_TYPEDEF_LPTR
#define FMSTR_TYPEDEF_LPTR(type, ptrtype) typedef type *ptrtype
#endif

#ifndef FMSTR_LP_U8
FMSTR_TYPEDEF_LPTR(FMSTR_U8, _FMSTR_LP_U8);
#define FMSTR_LP_U8 _FMSTR_LP_U8
#endif

#ifndef FMSTR_LP_U16
FMSTR_TYPEDEF_LPTR(FMSTR_U16, _FMSTR_LP_U16);
#define FMSTR_LP_U16 _FMSTR_LP_U16
#endif

#ifndef FMSTR_LP_U32
FMSTR_TYPEDEF_LPTR(FMSTR_U32, _FMSTR_LP_U32);
#define FMSTR_LP_U32 _FMSTR_LP_U32
#endif

#ifndef FMSTR_LP_U64
FMSTR_TYPEDEF_LPTR(FMSTR_U64, _FMSTR_LP_U64);
#define FMSTR_LP_U64 _FMSTR_LP_U64
#endif

/* pointer to FMSTR_TSA_ENTRY (also potentially far on some platforms) */
#ifndef FMSTR_LP_TSA_ENTRY
FMSTR_TYPEDEF_LPTR(FMSTR_TSA_ENTRY, _FMSTR_LP_TSA_ENTRY);
#define FMSTR_LP_TSA_ENTRY _FMSTR_LP_TSA_ENTRY
#endif

/******************************************************************************
 * Internal data types used
 ******************************************************************************/

#define SHA1_BLOCK_SIZE 20

typedef struct fmstr_sha1_ctx
{
    FMSTR_U8 data[64];
    FMSTR_U32 datalen;
    FMSTR_U64 bitlen;
    FMSTR_U32 state[5];
    FMSTR_U32 k[4];
} FMSTR_SHA1_CTX;

/* Transport protocol interface between the Protocol layer and communication subsystem */
typedef struct FMSTR_TRANSPORT_INTF_S
{
    FMSTR_BOOL (*Init)(void);
    void (*Poll)(void);
    void (*SendResponse)(FMSTR_BPTR pResponse, FMSTR_SIZE nLength, FMSTR_U8 statusCode, void *identification);
} FMSTR_TRANSPORT_INTF;

typedef struct FMSTR_SESSION_S
{
    void *identification; /* Identification for transport (address/socket/...) */

#if FMSTR_CFG_F1_RESTRICTED_ACCESS != 0
    struct
    {
        /* Salt challenge value used between AUTH1 and AUTH2 steps of password authentication. */
        FMSTR_U8 authSalt[FMSTR_AUTHENT_PRTCL_SHA1_SALT_LEN];
        /* Currently granted access (one of FMSTR_RESTRICTED_ACCESS_xxx). */
        FMSTR_U8 grantedAccess;
        FMSTR_U8 accessKey[SHA1_BLOCK_SIZE];
        FMSTR_U8 localKey[SHA1_BLOCK_SIZE];
    } restr; /* Restricted access */
#endif       /* FMSTR_CFG_F1_RESTRICTED_ACCESS */

} FMSTR_SESSION;

/* There are multiple global instances of different transports. User selects one in
   the application configuration file (or defines it indirectly by macros like
   FMSTR_USE_UART). */

extern const FMSTR_TRANSPORT_INTF FMSTR_TRANSPORT;

/******************************************************************************
 * Feature locking API
 ******************************************************************************/

#if FMSTR_SESSION_COUNT > 1
FMSTR_BOOL FMSTR_IsFeatureOwned(FMSTR_SESSION *session, FMSTR_U8 featureType, FMSTR_U8 instance);
#endif

/******************************************************************************
 * Global non-API functions (used internally in FreeMASTER driver)
 ******************************************************************************/

void FMSTR_FreeSession(void *identification);

void FMSTR_SendResponse(FMSTR_BPTR response, FMSTR_SIZE length, FMSTR_U8 statusCode, FMSTR_SESSION *session);
FMSTR_BOOL FMSTR_ProtocolDecoder(FMSTR_BPTR msgBuffIO, FMSTR_SIZE msgSize, FMSTR_U8 cmdCode, void *identification);
FMSTR_BOOL FMSTR_SendTestFrame(FMSTR_BPTR msgBuffIO, FMSTR_SESSION *session);

#if FMSTR_USE_APPCMD > 0
FMSTR_BOOL FMSTR_InitAppCmds(void);
FMSTR_BPTR FMSTR_StoreAppCmd(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_SIZE msgSize, FMSTR_U8 *retStatus);
FMSTR_BPTR FMSTR_GetAppCmdStatus(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus);
FMSTR_BPTR FMSTR_GetAppCmdRespData(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus);
#endif

#if FMSTR_USE_SCOPE > 0
FMSTR_BOOL FMSTR_InitScope(void);
FMSTR_BPTR FMSTR_SetScope(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_SIZE inputLen, FMSTR_U8 *retStatus);
FMSTR_BPTR FMSTR_ReadScope(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus, FMSTR_SIZE maxOutSize);
#endif

#if FMSTR_USE_RECORDER > 0
FMSTR_BOOL FMSTR_InitRec(void);
FMSTR_BPTR FMSTR_SetRecCmd(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_SIZE inputLen, FMSTR_U8 *retStatus);
FMSTR_BPTR FMSTR_GetRecCmd(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus);
FMSTR_BOOL FMSTR_IsInRecBuffer(FMSTR_ADDR addr, FMSTR_SIZE size);
#endif

#if FMSTR_USE_TSA > 0
FMSTR_BOOL FMSTR_InitTsa(void);
FMSTR_BPTR FMSTR_GetTsaInfo(FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus);
FMSTR_BPTR FMSTR_GetStringLen(FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus);
FMSTR_BOOL FMSTR_CheckTsaSpace(FMSTR_ADDR varAddr, FMSTR_SIZE varSize, FMSTR_BOOL writeAccess);
#if FMSTR_USE_URES > 0
FMSTR_BPTR FMSTR_UresControl(FMSTR_BPTR msgBuffIO, FMSTR_SIZE msgSize, FMSTR_U8 *retStatus);
#endif
#endif

#if FMSTR_USE_PIPES > 0
FMSTR_BOOL FMSTR_InitPipes(void);
FMSTR_BPTR FMSTR_PipeFrame(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_SIZE msgSize, FMSTR_U8 *retStatus);
FMSTR_BPTR FMSTR_GetPipe(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_SIZE msgSize, FMSTR_U8 *retStatus);
FMSTR_INDEX FMSTR_FindPipeIndex(FMSTR_PIPE_PORT pipePort);
#endif

void FMSTR_Sha1Init(FMSTR_SHA1_CTX *ctx);
void FMSTR_Sha1Update(FMSTR_SHA1_CTX *ctx, const FMSTR_U8 *data, FMSTR_SIZE len);
void FMSTR_Sha1Final(FMSTR_SHA1_CTX *ctx, FMSTR_U8 *hash);

/******************************************************************************
 * aligned memory and buffer memory operations
 ******************************************************************************/

void FMSTR_MemCpyTo(FMSTR_ADDR destAddr, FMSTR_ADDR srcAddr, FMSTR_SIZE size);
void FMSTR_MemCpyFrom(FMSTR_ADDR destAddr, FMSTR_ADDR srcAddr, FMSTR_SIZE size);
void FMSTR_MemCpyToMasked(FMSTR_ADDR destAddr, FMSTR_ADDR srcAddr, FMSTR_ADDR maskAddr, FMSTR_SIZE size);

FMSTR_BPTR FMSTR_CopyToBuffer(FMSTR_BPTR destBuff, FMSTR_ADDR srcAddr, FMSTR_SIZE size);
FMSTR_BPTR FMSTR_CopyFromBuffer(FMSTR_ADDR destAddr, FMSTR_BPTR srcBuff, FMSTR_SIZE size);
void FMSTR_CopyFromBufferWithMask(FMSTR_ADDR destAddr, FMSTR_BPTR srcBuff, FMSTR_SIZE size);

#define FMSTR_ValueFromBuffer8(dest, src) \
    ((*((FMSTR_U8 *)(dest)) = *(FMSTR_U8 *)(src)), (FMSTR_BPTR)(((FMSTR_BPTR)(src)) + 1))

#define FMSTR_ValueToBuffer8(dest, src) \
    ((*((FMSTR_U8 *)(dest)) = (FMSTR_U8)(src)), (FMSTR_BPTR)(((FMSTR_BPTR)(dest)) + 1))

#define FMSTR_SkipInBuffer(dest, size) ((FMSTR_BPTR)(((FMSTR_BPTR)(dest)) + (size)))

FMSTR_BPTR FMSTR_ValueFromBuffer16BE(FMSTR_U16 *pnum, FMSTR_BPTR src);
FMSTR_BPTR FMSTR_ValueToBuffer16BE(FMSTR_BPTR dest, FMSTR_U16 num);
FMSTR_BPTR FMSTR_AddressFromBuffer(FMSTR_ADDR *paddr, FMSTR_BPTR src);
FMSTR_BPTR FMSTR_AddressToBuffer(FMSTR_BPTR dest, FMSTR_ADDR addr);
FMSTR_BPTR FMSTR_SizeFromBuffer(FMSTR_SIZE *psize, FMSTR_BPTR src);
FMSTR_BPTR FMSTR_SizeToBuffer(FMSTR_BPTR dest, FMSTR_SIZE size);
FMSTR_BPTR FMSTR_IndexFromBuffer(FMSTR_INDEX *pindex, FMSTR_BPTR src);
FMSTR_BPTR FMSTR_ULebToBuffer(FMSTR_BPTR dest, FMSTR_U32 num);
FMSTR_BPTR FMSTR_ULebFromBuffer(FMSTR_U32 *pnum, FMSTR_BPTR src);
FMSTR_SIZE FMSTR_GetAlignmentCorrection(FMSTR_ADDR addr, FMSTR_SIZE size);

/******************************************************************************
 * Memory operations
 ******************************************************************************/

FMSTR_INLINE FMSTR_S8 _FMSTR_GetS8(FMSTR_ADDR addr)
{
    /* coverity[misra_c_2012_rule_11_3_violation:FALSE] */
    FMSTR_S8* a = (FMSTR_S8*)addr;
    return *a;
}

FMSTR_INLINE FMSTR_U8 _FMSTR_GetU8(FMSTR_ADDR addr)
{
    /* coverity[misra_c_2012_rule_11_3_violation:FALSE] */
    FMSTR_U8* a = (FMSTR_U8*)addr;
    return *a;
}

FMSTR_INLINE FMSTR_S16 _FMSTR_GetS16(FMSTR_ADDR addr)
{
    /* coverity[misra_c_2012_rule_11_3_violation:FALSE] */
    FMSTR_S16* a = (FMSTR_S16*)addr;
    return *a;
}

FMSTR_INLINE FMSTR_U16 _FMSTR_GetU16(FMSTR_ADDR addr)
{
    /* coverity[misra_c_2012_rule_11_3_violation:FALSE] */
    FMSTR_U16* a = (FMSTR_U16*)addr;
    return *a;
}

FMSTR_INLINE FMSTR_S32 _FMSTR_GetS32(FMSTR_ADDR addr)
{
    /* coverity[misra_c_2012_rule_11_3_violation:FALSE] */
    FMSTR_S32* a = (FMSTR_S32*)addr;
    return *a;
}

FMSTR_INLINE FMSTR_U32 _FMSTR_GetU32(FMSTR_ADDR addr)
{
    /* coverity[misra_c_2012_rule_11_3_violation:FALSE] */
    FMSTR_U32* a = (FMSTR_U32*)addr;
    return *a;
}

FMSTR_INLINE FMSTR_S64 _FMSTR_GetS64(FMSTR_ADDR addr)
{
    /* coverity[misra_c_2012_rule_11_3_violation:FALSE] */
    FMSTR_S64* a = (FMSTR_S64*)addr;
    return *a;
}

FMSTR_INLINE FMSTR_U64 _FMSTR_GetU64(FMSTR_ADDR addr)
{
    /* coverity[misra_c_2012_rule_11_3_violation:FALSE] */
    FMSTR_U64* a = (FMSTR_U64*)addr;
    return *a;
}

#ifndef FMSTR_GetS8
#define FMSTR_GetS8(addr)  _FMSTR_GetS8(addr)
#endif
#ifndef FMSTR_GetU8
#define FMSTR_GetU8(addr)  _FMSTR_GetU8(addr)
#endif
#ifndef FMSTR_GetS16
#define FMSTR_GetS16(addr)  _FMSTR_GetS16(addr)
#endif
#ifndef FMSTR_GetU16
#define FMSTR_GetU16(addr)  _FMSTR_GetU16(addr)
#endif
#ifndef FMSTR_GetS32
#define FMSTR_GetS32(addr)  _FMSTR_GetS32(addr)
#endif
#ifndef FMSTR_GetU32
#define FMSTR_GetU32(addr)  _FMSTR_GetU32(addr)
#endif
#ifndef FMSTR_GetS64
#define FMSTR_GetS64(addr)  _FMSTR_GetS64(addr)
#endif
#ifndef FMSTR_GetU64
#define FMSTR_GetU64(addr)  _FMSTR_GetU64(addr)
#endif

#if FMSTR_REC_FLOAT_TRIG > 0

FMSTR_INLINE FMSTR_FLOAT _FMSTR_GetFloat(FMSTR_ADDR addr)
{
    /* coverity[misra_c_2012_rule_11_3_violation:FALSE] */
    FMSTR_FLOAT* a = (FMSTR_FLOAT*)addr;
    return *a;
}

FMSTR_INLINE FMSTR_DOUBLE _FMSTR_GetDouble(FMSTR_ADDR addr)
{
    /* coverity[misra_c_2012_rule_11_3_violation:FALSE] */
    FMSTR_DOUBLE* a = (FMSTR_DOUBLE*)addr;
    return *a;
}

#ifndef FMSTR_GetFloat
#define FMSTR_GetFloat(addr)  _FMSTR_GetFloat(addr)
#endif
#ifndef FMSTR_GetDouble
#define FMSTR_GetDouble(addr)  _FMSTR_GetDouble(addr)
#endif

#endif

#ifdef __cplusplus
}
#endif

/****************************************************************************************
 * Potentially unused variable declaration
 *****************************************************************************************/

#if defined(_lint) || defined(__IAR_SYSTEMS_ICC__) || defined(__ARMCC_VERSION)
#define FMSTR_UNUSED(sym) (void)(sym)
#elif defined(__GNUC__)
#define FMSTR_UNUSED(sym) (void)(sym)
#else
#define FMSTR_UNUSED(sym) ((sym), 0)
#endif

/****************************************************************************************
 * Potentially unused variable declaration
 *****************************************************************************************/

/** Get count of elements in a static array. */
#define FMSTR_COUNTOF(arr) (sizeof(arr) / sizeof(arr[0]))

/****************************************************************************************
 * Configuration checking
 *****************************************************************************************/

/* obsolete transport and driver macros */
#if defined(FMSTR_USE_PDBDM)
#warning Obsolete configuration option detected. Define FMSTR_TRANSPORT as FMSTR_PDBDM instead.
#endif

#if (FMSTR_LONG_INTR && (FMSTR_SHORT_INTR || FMSTR_POLL_DRIVEN)) || \
    (FMSTR_SHORT_INTR && (FMSTR_LONG_INTR || FMSTR_POLL_DRIVEN)) || \
    (FMSTR_POLL_DRIVEN && (FMSTR_LONG_INTR || FMSTR_SHORT_INTR)) || \
    !(FMSTR_POLL_DRIVEN || FMSTR_LONG_INTR || FMSTR_SHORT_INTR)
/* mismatch in interrupt modes, only one can be selected */
#error You have to enable exctly one of FMSTR_LONG_INTR or FMSTR_SHORT_INTR or FMSTR_POLL_DRIVEN
#endif

/* check scope settings */
#if FMSTR_USE_SCOPE > 0
#if FMSTR_MAX_SCOPE_VARS > 32 || FMSTR_MAX_SCOPE_VARS < 2
#error Error in FMSTR_MAX_SCOPE_VARS value. Use a value in range 2..32
#endif
#endif

/* check recorder settings */
#if FMSTR_USE_RECORDER > 0
#if FMSTR_USE_READMEM == 0
#error Recorder needs the FMSTR_USE_READMEM feature
#endif

#if defined(FMSTR_REC_OWNBUFF)
#warning The FMSTR_REC_OWNBUFF is obsolete. Use FMSTR_REC_BUFF_SIZE for Recorder 0 and/or FMSTR_RecorderCreate for other Recorders.
#endif
#endif

#if FMSTR_USE_TSA > 0
#if FMSTR_USE_READMEM == 0
#error TSA needs the FMSTR_USE_READMEM feature
#endif
#endif

#if FMSTR_USE_PIPES > 0

#if defined(FMSTR_PIPES_EXPERIMENTAL)
#warning The "pipes" feature is now in experimental code phase. Not yet tested on this platform.
#endif

/* must enable printf for vararg printf */
#if FMSTR_USE_PIPE_PRINTF == 0 && FMSTR_USE_PIPE_PRINTF_VARG > 0
#error You must enable pipe printf for vararg printf (see FMSTR_USE_PIPE_PRINTF)
#endif

/* printf buffer should accept one integer printed */
#if FMSTR_PIPES_PRINTF_BUFF_SIZE < 8
#error Pipe printf buffer should be at least 8 (see FMSTR_PIPES_PRINTF_BUFF_SIZE)
#endif
#if FMSTR_PIPES_PRINTF_BUFF_SIZE > 255
#error Pipe printf buffer should not exceed 255 (see FMSTR_PIPES_PRINTF_BUFF_SIZE)
#endif

/* obsolete configuration options */
#if defined(FMSTR_MAX_PIPES_COUNT)
#warning The FMSTR_MAX_PIPES_COUNT option is obsolete. Specify number of used pipes as FMSTR_USE_PIPES
#endif

#endif

#if FMSTR_DEBUG_TX > 0
/* When communication debugging mode is requested, this global variable is used to
   turn the debugging off once a valid connection is detected */
extern FMSTR_BOOL fmstr_doDebugTx;
#endif

/* maximum data item width in bytes when performing optimized/aligned memory copy */
#ifndef FMSTR_MEMCPY_MAX_SIZE
/* 32-bit transfers maximum by default */
#define FMSTR_MEMCPY_MAX_SIZE 4
#endif

#endif /* __FREEMASTER_PRIVATE_H */
