/*
 * Copyright 2021, 2024-2025 NXP
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
 * FreeMASTER Communication Driver - RTT (virtual network) driver
 */

#include "freemaster.h"
#include "freemaster_private.h"

/* Compile this code only if the NET driver is selected in freemaster_cfg.h. */
#if defined(FMSTR_NET_DRV) && (FMSTR_MK_IDSTR(FMSTR_NET_DRV) == FMSTR_NET_SEGGER_RTT_ID)

#include "freemaster_net_segger_rtt.h"

#if FMSTR_DISABLE == 0

#include "freemaster_protocol.h"
#include "freemaster_net.h"
#include "SEGGER_RTT.h"

/******************************************************************************
 * Adapter configuration
 ******************************************************************************/
#if (defined(FMSTR_SHORT_INTR) && FMSTR_SHORT_INTR) || (defined(FMSTR_LONG_INTR) && FMSTR_LONG_INTR)
#warning "The FreeMASTER RTT driver does not support interrupt mode."
#endif

#if FMSTR_NET_AUTODISCOVERY != 0
#warning The FreeMASTER RTT driver does not support auto discovery.
#endif

#if FMSTR_SESSION_COUNT != 1
/* SEGGER RTT transport only supports a single session */
#warning Please set FMSTR_SESSION_COUNT to 1.
#endif

#ifndef FMSTR_NET_SEGGER_RTT_BUFFER_INDEX
#define FMSTR_NET_SEGGER_RTT_BUFFER_INDEX 0
#endif

/******************************************************************************
 * Local types
 ******************************************************************************/

/******************************************************************************
 * Local functions
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_RttInit(void);
static void _FMSTR_RttPoll(void);
static FMSTR_S32 _FMSTR_RttRecv(FMSTR_BPTR msgBuff,
                                FMSTR_SIZE msgMaxSize,
                                FMSTR_NET_ADDR *recvAddr,
                                FMSTR_BOOL *isBroadcast);
static FMSTR_S32 _FMSTR_RttSend(FMSTR_NET_ADDR *sendAddr, FMSTR_BPTR msgBuff, FMSTR_SIZE msgSize);
static void _FMSTR_RttClose(FMSTR_NET_ADDR *addr);
static void _FMSTR_RttGetCaps(FMSTR_NET_IF_CAPS *caps);

/******************************************************************************
 * Local variables
 ******************************************************************************/

static const FMSTR_NET_ADDR fmstr_rttDummyAddress = {
    .type = FMSTR_NET_ADDR_TYPE_V4, .port = 0, .addr.v4 = {0, 0, 0, 0}};

/******************************************************************************
 * Driver interface
 ******************************************************************************/
/* Interface of this network TCP driver */
const FMSTR_NET_DRV_INTF FMSTR_NET_SEGGER_RTT = {
    .Init    = _FMSTR_RttInit,
    .Poll    = _FMSTR_RttPoll,
    .Recv    = _FMSTR_RttRecv,
    .Send    = _FMSTR_RttSend,
    .Close   = _FMSTR_RttClose,
    .GetCaps = _FMSTR_RttGetCaps,
};

#if FMSTR_NET_SEGGER_RTT_BUFFER_INDEX > 0
#define FMSTR_RTT_BUFF_SIZE (FMSTR_COMM_BUFFER_SIZE + 7)
static FMSTR_U8 fmstr_rttBuffer[FMSTR_RTT_BUFF_SIZE];
#endif

/******************************************************************************
 * Implementation
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_RttInit(void)
{
    SEGGER_RTT_Init();
    
    if(SEGGER_RTT_SetTerminal(FMSTR_NET_SEGGER_RTT_BUFFER_INDEX) < 0)
    {
        return FMSTR_FALSE;
    }

#if FMSTR_NET_SEGGER_RTT_BUFFER_INDEX > 0
    if(SEGGER_RTT_ConfigUpBuffer(FMSTR_NET_SEGGER_RTT_BUFFER_INDEX, NULL, fmstr_rttBuffer, FMSTR_RTT_BUFF_SIZE, SEGGER_RTT_MODE_NO_BLOCK_SKIP) < 0)
    {
        return FMSTR_FALSE;
    }
    if(SEGGER_RTT_ConfigDownBuffer(FMSTR_NET_SEGGER_RTT_BUFFER_INDEX, NULL, fmstr_rttBuffer, FMSTR_RTT_BUFF_SIZE, 0) < 0)
    {
        return FMSTR_FALSE;
    }
#else
    if(SEGGER_RTT_ConfigUpBuffer(FMSTR_NET_SEGGER_RTT_BUFFER_INDEX, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP) < 0)
    {
        return FMSTR_FALSE;
    }
    if(SEGGER_RTT_ConfigDownBuffer(FMSTR_NET_SEGGER_RTT_BUFFER_INDEX, NULL, NULL, 0, 0) < 0)
    {
        return FMSTR_FALSE;
    }
#endif
    return FMSTR_TRUE;
}

static void _FMSTR_RttPoll(void)
{
}

static FMSTR_S32 _FMSTR_RttRecv(FMSTR_BPTR msgBuff,
                                FMSTR_SIZE msgMaxSize,
                                FMSTR_NET_ADDR *recvAddr,
                                FMSTR_BOOL *isBroadcast)
{
    unsigned res;
    
    FMSTR_ASSERT(msgBuff != NULL);
    FMSTR_ASSERT(recvAddr != NULL);

    FMSTR_MemCpy(recvAddr, &fmstr_rttDummyAddress, sizeof(FMSTR_NET_ADDR));

    res = SEGGER_RTT_Read(FMSTR_NET_SEGGER_RTT_BUFFER_INDEX, msgBuff, msgMaxSize);
    return res <= (unsigned)msgMaxSize ? (FMSTR_S32)res : -1;
}

static FMSTR_S32 _FMSTR_RttSend(FMSTR_NET_ADDR *sendAddr, FMSTR_BPTR msgBuff, FMSTR_SIZE msgSize)
{
    unsigned res;

    FMSTR_ASSERT(msgBuff != NULL);
    FMSTR_ASSERT(sendAddr != NULL);

    res = SEGGER_RTT_Write(FMSTR_NET_SEGGER_RTT_BUFFER_INDEX, msgBuff, msgSize);
    return res <= (unsigned)msgSize ? (FMSTR_S32)res : -1;
}

static void _FMSTR_RttClose(FMSTR_NET_ADDR *addr)
{
}

static void _FMSTR_RttGetCaps(FMSTR_NET_IF_CAPS *caps)
{
    FMSTR_ASSERT(caps != NULL);

    caps->flags |= FMSTR_NET_IF_CAPS_FLAG_RTT;
}

#endif /* (!(FMSTR_DISABLE)) */
#endif /* defined(FMSTR_NET_DRV) && (FMSTR_MK_IDSTR(FMSTR_NET_DRV) == FMSTR_NET_RTT_ID) */
