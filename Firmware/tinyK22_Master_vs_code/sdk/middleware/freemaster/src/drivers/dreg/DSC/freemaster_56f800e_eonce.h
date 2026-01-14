/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2020, 2024 NXP
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
 * FreeMASTER Communication Driver - 56F800E JTAG/EONCE RTD Unit low-level driver
 */

#ifndef __FREEMASTER_56F800E_EONCE_H
#define __FREEMASTER_56F800E_EONCE_H

/******************************************************************************
* Required header files include check
******************************************************************************/
#ifndef __FREEMASTER_H
#error Please include the freemaster.h master header file before the freemaster_56f800e_eonce.h
#endif

/******************************************************************************
* Adapter configuration
******************************************************************************/

/* Silicon bug in older DSC cores makes the TDF bit unusable (TDF gets reset too early).
   The FreeMASTER tool may send a dummy confirmation word whenever it receives
   word transmitted by us. This means we can use the RDF as an indication of !TDF.
   Note that if this option is set, the same workaround MUST be activated on the
   FreeMASTER tool side (in the JTAG EOnCE communication plug-in settings). */
#ifndef FMSTR_USE_EONCE_TDF_WORKAROUND
#define FMSTR_USE_EONCE_TDF_WORKAROUND 0
#endif

#ifdef __cplusplus
  extern "C" {
#endif

/******************************************************************************
* Types definition
******************************************************************************/

/******************************************************************************
* inline functions
******************************************************************************/

/******************************************************************************
* Global API functions
******************************************************************************/

/* This call is ignored as EOnCE has a fixed core address */
void FMSTR_SerialSetBaseAddress(FMSTR_U32 base);
void FMSTR_SerialIsr(void);

#ifdef __cplusplus
  }
#endif

#endif /* __FREEMASTER_56F800E_EONCE_H */

