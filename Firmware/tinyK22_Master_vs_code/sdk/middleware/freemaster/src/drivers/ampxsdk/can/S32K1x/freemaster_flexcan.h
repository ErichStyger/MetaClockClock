/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2018-2019, 2024 NXP
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
 * FreeMASTER Communication Driver - FlexCAN low-level driver
 */

#ifndef __FREEMASTER_FLEXCAN_H
#define __FREEMASTER_FLEXCAN_H

/******************************************************************************
* Required header files include check
******************************************************************************/
#ifndef __FREEMASTER_H
#error Please include the freemaster.h master header file before the freemaster_flexcan.h
#endif


#include "flexcan_driver.h"
#include "flexcan_hw_access.h"
#include "can_pal1.h"
#include "flexcan_hw_access.h"
#include "can_pal.h"

/******************************************************************************
* Adapter configuration
******************************************************************************/
/* flexcan needs to know the transmit and receive MB number */
#if ((FMSTR_USE_FLEXCAN) || (FMSTR_USE_FLEXCAN32))
    /* Flexcan TX message buffer must be defined */
    #ifndef FMSTR_FLEXCAN_TXMB
    /* #error FlexCAN transmit buffer needs to be specified (use FMSTR_FLEXCAN_TXMB) */
    #warning "FlexCAN Message Buffer 0 is used for transmitting messages"
    #define FMSTR_FLEXCAN_TXMB 0
    #endif
    /* Flexcan RX message buffer must be defined */
    #ifndef FMSTR_FLEXCAN_RXMB
    /* #error FlexCAN receive buffer needs to be specified (use FMSTR_FLEXCAN_RXMB) */
    #warning "FlexCAN Message Buffer 1 is used for receiving messages"
    #define FMSTR_FLEXCAN_RXMB 1
    #endif
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
      
void FMSTR_CanSetBaseAddress(CAN_Type *base);
void FMSTR_CanSetInstance(can_instance_t *instance);
void FMSTR_CanIsr(void);
void FMSTR_CanIsrReceive(void);
void FMSTR_CanIsrSend(void);

void can_pal_callback(uint32_t instance, can_event_t eventType, uint32_t buffIdx, void *driverState);

#ifdef __cplusplus
  }
#endif

#endif /* __FREEMASTER_FLEXCAN_H */

