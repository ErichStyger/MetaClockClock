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
 * FreeMASTER Communication Driver - TSA
 */

#ifndef __FREEMASTER_TSA_H
#define __FREEMASTER_TSA_H

#ifndef __FREEMASTER_H
#error Please include "freemaster.h" before "freemaster_tsa.h"
#endif

/*****************************************************************************
 Target-side Address translation structures and macros
******************************************************************************/

/* current TSA version  */
#define FMSTR_TSA_VERSION 3U

/* TSA flags carried in TSA_ENTRY.info (except the first entry in table) */
#define FMSTR_TSA_INFO_ENTRYTYPE_MASK 0x0003U /* flags reserved for TSA_ENTRY use */
#define FMSTR_TSA_INFO_NON_VAR        0x0000U /* ENTRYTYPE: non-variable object. The "type" determines the real type. */
#define FMSTR_TSA_INFO_RO_VAR         0x0001U /* ENTRYTYPE: read-only variable */
#define FMSTR_TSA_INFO_RO_FLASH       0x0002U /* ENTRYTYPE: read-only variable located in flash */
#define FMSTR_TSA_INFO_RW_VAR         0x0003U /* ENTRYTYPE: read-write variable */

#define FMSTR_TSA_INFO_VAR_MASK 0x03U /* R/W flags in info */

/* pointer types used in TSA tables can be overridden in freemaster.h */
/* (this is why macros are used instead of typedefs) */
#ifndef FMSTR_TSATBL_STRPTR
#define FMSTR_TSATBL_STRPTR const char *
#endif
#ifndef FMSTR_TSATBL_VOIDPTR
#define FMSTR_TSATBL_VOIDPTR volatile const void *
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Types definition
 ******************************************************************************/

/* TSA table entry. The unions inside assures variables sized enough to */
/* accommodate both the C-pointer and the user-requested size (FMSTR_ADDR) */
#if defined(__S12Z__)
typedef struct
{
    /* sizeof(FMSTR_ADDR) is 3 on S12Z platform. We need an extra byte to align to 4 bytes.
     * An extra byte is put in front of each entry as S12Z is a big-endian machine. */
    FMSTR_U8 dummy0;
    union
    {
        FMSTR_TSATBL_STRPTR p;
        FMSTR_ADDR n;
    } name;
    FMSTR_U8 dummy1;
    union
    {
        FMSTR_TSATBL_STRPTR p;
        FMSTR_ADDR n;
    } type;
    FMSTR_U8 dummy2;
    union
    {
        FMSTR_TSATBL_VOIDPTR p;
        FMSTR_ADDR n;
    } addr;
    FMSTR_U8 dummy3;
    union
    {
        FMSTR_TSATBL_VOIDPTR p;
        FMSTR_ADDR n;
    } info;
} FMSTR_TSA_ENTRY;

/* dummy member initializers */
#define FMSTR_TSATBL_FILL_PRE 0,
#define FMSTR_TSATBL_FILL_POST

#else /* defined(__S12Z__) */

typedef struct
{
    /* Generic table entry on little-endian systems */
    union
    {
        FMSTR_TSATBL_STRPTR p;
        FMSTR_ADDR n;
    } name;
    union
    {
        FMSTR_TSATBL_STRPTR p;
        FMSTR_ADDR n;
    } type;
    union
    {
        FMSTR_TSATBL_VOIDPTR p;
        FMSTR_ADDR n;
    } addr;
    union
    {
        FMSTR_TSATBL_VOIDPTR p;
        FMSTR_ADDR n;
    } info;
} FMSTR_TSA_ENTRY;

/* no dummy members */
#define FMSTR_TSATBL_FILL_PRE
#define FMSTR_TSATBL_FILL_POST

#endif /* defined(__S12Z__) */

#ifdef __cplusplus
}
#endif

#ifndef FMSTR_TSATBL_STRPTR_CAST
#define FMSTR_TSATBL_STRPTR_CAST(x) ((FMSTR_TSATBL_STRPTR)(x))
#endif
#ifndef FMSTR_TSATBL_STRPTR_ENTRY
#define FMSTR_TSATBL_STRPTR_ENTRY(x) FMSTR_TSATBL_FILL_PRE{FMSTR_TSATBL_STRPTR_CAST(x)} FMSTR_TSATBL_FILL_POST
#endif
#ifndef FMSTR_TSATBL_VOIDPTR_CAST
#define FMSTR_TSATBL_VOIDPTR_CAST(x) ((FMSTR_TSATBL_VOIDPTR)(x))
#endif
#ifndef FMSTR_TSATBL_VOIDPTR_ENTRY
#define FMSTR_TSATBL_VOIDPTR_ENTRY(x) FMSTR_TSATBL_FILL_PRE{FMSTR_TSATBL_VOIDPTR_CAST(x)} FMSTR_TSATBL_FILL_POST
#endif

/* list of available types for TSA table definition */
#define FMSTR_TSA_MEMBER_CFG(parenttype, name, type)                   \
    FMSTR_TSATBL_STRPTR_ENTRY(#name), FMSTR_TSATBL_STRPTR_ENTRY(type), \
        FMSTR_TSATBL_VOIDPTR_ENTRY(&((parenttype *)0)->name),          \
        FMSTR_TSATBL_VOIDPTR_ENTRY(FMSTR_TSA_INFO1(((parenttype *)0)->name, FMSTR_TSA_INFO_NON_VAR)),

#define FMSTR_TSA_RO_VAR_CFG(name, type)                                                                    \
    FMSTR_TSATBL_STRPTR_ENTRY(#name), FMSTR_TSATBL_STRPTR_ENTRY(type), FMSTR_TSATBL_VOIDPTR_ENTRY(&(name)), \
        FMSTR_TSATBL_VOIDPTR_ENTRY(FMSTR_TSA_INFO1(name, FMSTR_TSA_INFO_RO_VAR)),

#define FMSTR_TSA_RW_VAR_CFG(name, type)                                                                    \
    FMSTR_TSATBL_STRPTR_ENTRY(#name), FMSTR_TSATBL_STRPTR_ENTRY(type), FMSTR_TSATBL_VOIDPTR_ENTRY(&(name)), \
        FMSTR_TSATBL_VOIDPTR_ENTRY(FMSTR_TSA_INFO1(name, FMSTR_TSA_INFO_RW_VAR)),

#define FMSTR_TSA_RO_MEM_CFG(name, type, addr, size)                                                     \
    FMSTR_TSATBL_STRPTR_ENTRY(#name), FMSTR_TSATBL_STRPTR_ENTRY(type), FMSTR_TSATBL_VOIDPTR_ENTRY(addr), \
        FMSTR_TSATBL_VOIDPTR_ENTRY(FMSTR_TSA_INFO2(size, FMSTR_TSA_INFO_RO_VAR)),

#define FMSTR_TSA_RW_MEM_CFG(name, type, addr, size)                                                     \
    FMSTR_TSATBL_STRPTR_ENTRY(#name), FMSTR_TSATBL_STRPTR_ENTRY(type), FMSTR_TSATBL_VOIDPTR_ENTRY(addr), \
        FMSTR_TSATBL_VOIDPTR_ENTRY(FMSTR_TSA_INFO2(size, FMSTR_TSA_INFO_RW_VAR)),

#define FMSTR_TSA_RO_ENTRY(name_string, type_string, addr, size)                                                      \
    FMSTR_TSATBL_STRPTR_ENTRY(name_string), FMSTR_TSATBL_STRPTR_ENTRY(type_string), FMSTR_TSATBL_VOIDPTR_ENTRY(addr), \
        FMSTR_TSATBL_VOIDPTR_ENTRY(FMSTR_TSA_INFO2(size, FMSTR_TSA_INFO_RW_VAR)),

#define FMSTR_TSA_NOMEM_SIZE(name_string, type_string, addr, size)                                                    \
    FMSTR_TSATBL_STRPTR_ENTRY(name_string), FMSTR_TSATBL_STRPTR_ENTRY(type_string), FMSTR_TSATBL_VOIDPTR_ENTRY(addr), \
        FMSTR_TSATBL_VOIDPTR_ENTRY(FMSTR_TSA_INFO2(size, FMSTR_TSA_INFO_NON_VAR)),

#define FMSTR_TSA_NOMEM_NOSIZE(name_string, type_string, addr, size)                                                  \
    FMSTR_TSATBL_STRPTR_ENTRY(name_string), FMSTR_TSATBL_STRPTR_ENTRY(type_string), FMSTR_TSATBL_VOIDPTR_ENTRY(addr), \
        FMSTR_TSATBL_VOIDPTR_ENTRY(size),

/*************************************************************************
 * TSA "Base Types", all start with one (non-printable) character.
 * Type is retrieved by PC and parsed according to the binary format of:
 * "111STTZZ" where TT=type[int,frac,fp,special] S=signed ZZ=size[1,2,4,8]
 * "11101100" is a special information block (not a real TSA symbol).
 */

/* S=0 TT=int */
#define FMSTR_TSA_UINT8  "\xE0"
#define FMSTR_TSA_UINT16 "\xE1"
#define FMSTR_TSA_UINT32 "\xE2"
#define FMSTR_TSA_UINT64 "\xE3"
#define FMSTR_TSA_UINT(size) ( \
    (size) == 2 ? FMSTR_TSA_UINT16 : \
    (size) == 4 ? FMSTR_TSA_UINT32 : \
    (size) == 8 ? FMSTR_TSA_UINT64 : FMSTR_TSA_UINT8)

/* S=1 TT=int */
#define FMSTR_TSA_SINT8  "\xF0"
#define FMSTR_TSA_SINT16 "\xF1"
#define FMSTR_TSA_SINT32 "\xF2"
#define FMSTR_TSA_SINT64 "\xF3"
#define FMSTR_TSA_SINT(size) ( \
    (size) == 2 ? FMSTR_TSA_SINT16 : \
    (size) == 4 ? FMSTR_TSA_SINT32 : \
    (size) == 8 ? FMSTR_TSA_SINT64 : FMSTR_TSA_SINT8)

/* S=0 TT=frac */
#define FMSTR_TSA_UFRAC_UQ(m, n) "\xE4:" #m "." #n /* UQm.n fractional m+n=bitsize */
#define FMSTR_TSA_UFRAC16        "\xE5"            /* standard UQ1.15 */
#define FMSTR_TSA_UFRAC32        "\xE6"            /* standard UQ1.31 */
#define FMSTR_TSA_UFRAC64        "\xE7"            /* standard UQ1.63 */
/* S=1 TT=frac */
#define FMSTR_TSA_FRAC_Q(m, n) "\xF4:" #m "." #n /* Qm.n fractional m+n+1=bitsize */
#define FMSTR_TSA_FRAC16       "\xF5"            /* standard Q0.15 */
#define FMSTR_TSA_FRAC32       "\xF6"            /* standard Q0.31 */
#define FMSTR_TSA_FRAC64       "\xF7"            /* standard Q0.63 */
/* S=1 TT=fp */
#define FMSTR_TSA_FLOAT  "\xFA"
#define FMSTR_TSA_DOUBLE "\xFB"
/* TT=special */
#define FMSTR_TSA_SPECIAL_MEM   "\xEC"
#define FMSTR_TSA_SPECIAL_NOMEM "\xED"

/* a pointer type is like UINT with proper size set to sizeof(void*)
 * it is platform-specific, so made as external constant variable and
 * implemented in freemaster_tsa.c */
#if FMSTR_USE_TSA > 0
extern const char FMSTR_TSA_POINTER[];
#else
#define FMSTR_TSA_POINTER ""
#endif

/* macro used to describe "User Type" */
#define FMSTR_TSA_USERTYPE(type) #type

/* macro used to describe pure memory space */
#define FMSTR_TSA_MEMORY NULL

/*****************************/
/* TSA table-building macros */

#if FMSTR_USE_TSA > 0

#define FMSTR_TSA_FUNC(id)       FMSTR_TsaGetTable_##id
#define FMSTR_TSA_FUNC_PROTO(id) FMSTR_ADDR FMSTR_TSA_FUNC(id)(FMSTR_SIZE *tableSize)

#define FMSTR_TSA_TABLE_BEGIN(id) \
    FMSTR_TSA_FUNC_PROTO(id);     \
    FMSTR_TSA_FUNC_PROTO(id)      \
    {                             \
        static FMSTR_TSA_CDECL FMSTR_TSA_ENTRY fmstr_tsatable[] = {
/* entry info  */
#define FMSTR_TSA_INFO1(elem, flags) FMSTR_TSATBL_VOIDPTR_CAST(((sizeof(elem)) << 2) | (flags))
#define FMSTR_TSA_INFO2(size, flags) FMSTR_TSATBL_VOIDPTR_CAST(((size) << 2) | (flags))

/* TSA entry describing the structure/union member (must follow the FMSTR_TSA_STRUCT entry) */
#define FMSTR_TSA_MEMBER(parenttype, name, type) {FMSTR_TSA_MEMBER_CFG(parenttype, name, type)},

/* TSA entry describing read-only variable */
#define FMSTR_TSA_RO_VAR(name, type) {FMSTR_TSA_RO_VAR_CFG(name, type)},

/* TSA entry describing read-write variable */
#define FMSTR_TSA_RW_VAR(name, type) {FMSTR_TSA_RW_VAR_CFG(name, type)},

/* TSA entry describing read-only memory area */
#define FMSTR_TSA_RO_MEM(name, type, addr, size) {FMSTR_TSA_RO_MEM_CFG(name, type, addr, size)},

/* TSA entry describing read-write variable */
#define FMSTR_TSA_RW_MEM(name, type, addr, size) {FMSTR_TSA_RW_MEM_CFG(name, type, addr, size)},

/* TSA entry describing structure/union */
#define FMSTR_TSA_STRUCT(name) {FMSTR_TSA_NOMEM_SIZE(#name, FMSTR_TSA_SPECIAL_NOMEM ":STRUCT", NULL, sizeof(name))},

/* TSA entry describing enum */
#define FMSTR_TSA_ENUM(name) {FMSTR_TSA_NOMEM_SIZE(#name, FMSTR_TSA_SPECIAL_NOMEM ":ENUM", NULL, sizeof(name))},

/* TSA entry describing named constant as part of enumeration type
(entry must follow the ENUM definition) */
#define FMSTR_TSA_CONST(name) {FMSTR_TSA_NOMEM_NOSIZE(#name, FMSTR_TSA_SPECIAL_NOMEM ":CONST", name, 0)},

/* TSA entry describing virtual directory for the subsequent FILE entries
   The 'dirname' is either root-based if it starts with '/' or is relative to parent folder */
#define FMSTR_TSA_DIRECTORY(dirname) {FMSTR_TSA_RO_ENTRY(dirname, FMSTR_TSA_SPECIAL_NOMEM ":DIR", NULL, 0)},

/* TSA entry describing virtual file content statically mapped in memory (static content cached by PC)
   The 'filename' may be specified with relative paths using normal slash */
#define FMSTR_TSA_MEMFILE(filename, filemem, filesize) \
    {FMSTR_TSA_RO_ENTRY(filename, FMSTR_TSA_SPECIAL_MEM ":MEMFILE", filemem, (filesize))},

/* TSA entry describing project to be opened in FreeMASTER
   'project_uri' may be a existing file (one of FILE entries) or a web URI */
#define FMSTR_TSA_PROJECT(projectname, project_uri) \
    {FMSTR_TSA_RO_ENTRY(projectname, FMSTR_TSA_SPECIAL_MEM ":PRJ", project_uri, sizeof(project_uri))},

/* TSA entry describing general WEB link. All web links are offered to be navigated in FreeMASTER
   'link_uri' may be a existing file (one of FILE entries) or a web URI */
#define FMSTR_TSA_HREF(linkname, link_uri) \
    {FMSTR_TSA_RO_ENTRY(linkname, FMSTR_TSA_SPECIAL_MEM ":HREF", link_uri, sizeof(link_uri))},

/* TSA entry describing user defined resource: File.
   The 'function' is an pointer to existing function FMSTR_URES_HANDLER_FUNC.
   The 'param' is a pointer to user params, which is put into in function param.*/
#define FMSTR_TSA_USER_FILE(filename, function, param) \
    {FMSTR_TSA_NOMEM_NOSIZE(filename, FMSTR_TSA_SPECIAL_NOMEM ":U:FILE", (function), param)},

/* TSA entry describing user defined resource: Firmware image.
   The 'function' is an pointer to existing function FMSTR_URES_HANDLER_FUNC.
   The 'param' is a pointer to user params, which is put into in function param.*/
#define FMSTR_TSA_USER_FW(filename, function, param) \
    {FMSTR_TSA_NOMEM_NOSIZE(filename, FMSTR_TSA_SPECIAL_NOMEM ":U:FW", (function), param)},

/* TSA entry describing user defined resource: EEPROM, Flash, or other kind of fixed-size persistent storage.
   The 'function' is an pointer to existing function FMSTR_URES_HANDLER_FUNC.
   The 'param' is a pointer to user params, which is put into in function param.*/
#define FMSTR_TSA_USER_PROM(filename, function, param) \
    {FMSTR_TSA_NOMEM_NOSIZE(filename, FMSTR_TSA_SPECIAL_NOMEM ":U:PROM", (function), param)},

#define FMSTR_TSA_TABLE_END()                \
    }                                        \
    ;                                        \
    if (tableSize != NULL)                   \
    {                                        \
        *tableSize = sizeof(fmstr_tsatable); \
    }                                        \
    return (FMSTR_ADDR)fmstr_tsatable;       \
    }

#else /* FMSTR_USE_TSA */

#define FMSTR_TSA_TABLE_BEGIN(id)
#define FMSTR_TSA_INFO1(elem, flags)
#define FMSTR_TSA_INFO2(size, flags)
#define FMSTR_TSA_STRUCT(name)
#define FMSTR_TSA_MEMBER(parenttype, name, type)
#define FMSTR_TSA_RO_VAR(name, type)
#define FMSTR_TSA_RW_VAR(name, type)
#define FMSTR_TSA_RO_MEM(name, type, addr, size)
#define FMSTR_TSA_RW_MEM(name, type, addr, size)
#define FMSTR_TSA_DIRECTORY(dirname)
#define FMSTR_TSA_MEMFILE(filename, filemem, filesize)
#define FMSTR_TSA_PROJECT(projectname, project_uri)
#define FMSTR_TSA_HREF(linkname, link_uri)
#define FMSTR_TSA_ENUM(name)
#define FMSTR_TSA_CONST(name)
#define FMSTR_TSA_USER_FILE(filename, function, param)
#define FMSTR_TSA_USER_FW(filename, function, param)
#define FMSTR_TSA_USER_PROM(filename, function, param)
#define FMSTR_TSA_TABLE_END()

#endif /* FMSTR_USE_TSA */

/**********************************************/
/* master TSA table-retrieval building macros */

#ifdef __cplusplus
extern "C" {
#endif

#if FMSTR_USE_TSA > 0

#define FMSTR_TSA_TABLE_LIST_BEGIN()                                            \
    FMSTR_ADDR FMSTR_TsaGetTable(FMSTR_INDEX tableIndex, FMSTR_SIZE *tableSize) \
    {                                         \
        if (tableIndex < 0)                   \
        {                                     \
            return FMSTR_TsaInvalidTable();   \
        }                                     \

            
#define FMSTR_TSA_TABLE(id)                   \
    if (tableIndex-- == 0)                    \
    {                                         \
        FMSTR_TSA_FUNC_PROTO(id);             \
        return FMSTR_TSA_FUNC(id)(tableSize); \
    }                                         \
    else

#if FMSTR_USE_TSA_DYNAMIC > 0
#define FMSTR_TSA_TABLE_LIST_END()                         \
    {                                                      \
        FMSTR_TSA_FUNC_PROTO(dynamic_tsa);                 \
        FMSTR_ADDR tableAddr = FMSTR_TSA_FUNC(dynamic_tsa)(tableSize); \
        if (FMSTR_ValidateTsaTable(tableAddr) != FMSTR_FALSE && tableIndex-- == 0) \
        {                                                  \
            return tableAddr;                              \
        }                                                  \
        else                                               \
        {                                                  \
            return FMSTR_TsaGetUserTable(tableIndex, tableSize); \
        }                                                  \
    }                                                      \
    }
#else
#define FMSTR_TSA_TABLE_LIST_END()                         \
    {                                                      \
        return FMSTR_TsaGetUserTable(tableIndex, tableSize); \
    }                                                      \
    }
#endif

/*****************************************************************************
 Target-side Address translation functions
******************************************************************************/

/* master TSA table-retrieval function */
FMSTR_ADDR FMSTR_TsaGetTable(FMSTR_INDEX tableIndex, FMSTR_SIZE *tableSize);
FMSTR_ADDR FMSTR_TsaGetUserTable(FMSTR_INDEX tableIndex, FMSTR_SIZE *tableSize);
FMSTR_ADDR FMSTR_TsaInvalidTable(void);
FMSTR_BOOL FMSTR_ValidateTsaTable(FMSTR_ADDR tableAddr);
FMSTR_ADDR FMSTR_FindUresInTsa(FMSTR_ADDR resourceId);
FMSTR_TSA_FUNC_PROTO(dynamic_tsa);

#else /* FMSTR_USE_TSA */

#define FMSTR_TSA_TABLE_LIST_BEGIN()
#define FMSTR_TSA_TABLE(id)
#define FMSTR_TSA_TABLE_LIST_END()

#endif /* FMSTR_USE_TSA */

#ifdef __cplusplus
}
#endif

#endif /* __FREEMASTER_TSA_H */
