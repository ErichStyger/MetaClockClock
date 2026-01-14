/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FREERTOSCONFIG_FRAG_H_
#define _FREERTOSCONFIG_FRAG_H_


/* Do not include if processing assembly file */
#if (!defined(__IAR_SYSTEMS_ASM__) && !defined(__ASSEMBLER__))
    #if defined(__ICCARM__)||defined(__CC_ARM)||defined(__GNUC__)
    extern void traceTaskSwitchedOut(void);
    extern void traceTaskSwitchedIn(void);
    #endif
    #define traceTASK_SWITCHED_IN()  traceTaskSwitchedIn()
    #define traceTASK_SWITCHED_OUT() traceTaskSwitchedOut()
#endif

#endif /*_FREERTOSCONFIG_FRAG_H_*/
