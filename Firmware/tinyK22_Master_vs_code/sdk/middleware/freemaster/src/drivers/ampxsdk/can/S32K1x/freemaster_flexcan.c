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

#include "freemaster.h"
#include "freemaster_private.h"

/* Compile this code only if the CAN driver is selected in freemaster_cfg.h. */
#if defined(FMSTR_CAN_DRV) && (FMSTR_MK_IDSTR(FMSTR_CAN_DRV) == FMSTR_CAN_S32K1x_FLEXCAN_ID)

#include "freemaster_flexcan.h"

#if (!(FMSTR_DISABLE))

#include "freemaster_can.h"
/******************************************************************************
* Local macros
******************************************************************************/

/* FCAN: id to id-raw (idr) translation */
#define FMSTR_FCAN_ID2IDR(id) (((id)&FMSTR_CAN_EXTID) ? FLEXCAN_ID_EXT(id) : FLEXCAN_ID_STD(id))

/* user-selected MB as a 32bit or 64bit flag */
#if FMSTR_FLEXCAN_RXMB < 32
#define FMSTR_FLEXCAN_RXMB_FLAG (1UL << (FMSTR_FLEXCAN_RXMB))
#else
#define FMSTR_FLEXCAN_RXMB_FLAG (1ULL << (FMSTR_FLEXCAN_RXMB))
#endif
#if FMSTR_FLEXCAN_TXMB < 32
#define FMSTR_FLEXCAN_TXMB_FLAG (1UL << (FMSTR_FLEXCAN_TXMB))
#else
#define FMSTR_FLEXCAN_TXMB_FLAG (1ULL << (FMSTR_FLEXCAN_TXMB))
#endif

/* Using 64bit registers */
#if (defined(FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER)) && (FSL_FEATURE_FLEXCAN_HAS_EXTENDED_FLAG_REGISTER > 0)
#define FMSTR_FLEXCAN_REG_SIZE64
#define FMSTR_FLEXCAN_FLAG_ALL  0xFFFFFFFFFFFFFFFF
#else
#define FMSTR_FLEXCAN_FLAG_ALL  0xFFFFFFFF
#endif

/******************************************************************************
* Local functions
******************************************************************************/

static FMSTR_BOOL _FMSTR_FlexCAN_Init(FMSTR_U32 idRx, FMSTR_U32 idTx);  /* Initialize CAN module on a given base address. */
static void _FMSTR_FlexCAN_EnableTxInterrupt(FMSTR_BOOL enable);     /* Enable CAN Transmit interrupt. */
static void _FMSTR_FlexCAN_EnableRxInterrupt(FMSTR_BOOL enable);     /* Enable CAN Receive interrupt. */
static void _FMSTR_FlexCAN_EnableRx(void);                           /* Enable/re-initialize Receiver buffer. */
static FMSTR_SIZE8 _FMSTR_FlexCAN_GetRxFrameLen(void);               /* Return size of received CAN frame, or 0 if no Rx frame is available. */
static FMSTR_BCHR _FMSTR_FlexCAN_GetRxFrameByte(FMSTR_SIZE8 index);  /* Get data byte at index (0..8). */
static void _FMSTR_FlexCAN_AckRxFrame(void);                         /* Discard received frame and enable receiving a next one. */
static FMSTR_BOOL _FMSTR_FlexCAN_PrepareTxFrame(void);               /* Initialize transmit buffer; return false when Tx buffer is not available. */
static void _FMSTR_FlexCAN_PutTxFrameByte(FMSTR_SIZE8 index, FMSTR_BCHR data);   /* Fill one byte of transmit data. */
static void _FMSTR_FlexCAN_SendTxFrame(FMSTR_SIZE8 len);             /* Send the Tx buffer. */
/******************************************************************************
* Local variables
******************************************************************************/

/* Serial base address */
#ifdef FMSTR_CAN_BASE
    static CAN_Type *fmstr_canBaseAddr = FMSTR_CAN_BASE;
#else
    static CAN_Type *fmstr_canBaseAddr = NULL;
#endif

/* Serial instance */
#ifdef FMSTR_CAN_INSTANCE
    static can_instance_t *fmstr_canInstance = FMSTR_CAN_INSTANCE;
#else
    static can_instance_t *fmstr_canInstance = NULL;
#endif

static can_buff_config_t fmstr_rxcfg; /* MB configuration with Raw ID */
static can_buff_config_t fmstr_txcfg;

static can_message_t fmstr_rxmsg;/* Received frame buffer, valid when length>0 */
static can_message_t fmstr_txmsg;/* Buffer to prepare transmission */
/******************************************************************************
* Driver interface
******************************************************************************/

const FMSTR_CAN_DRV_INTF FMSTR_CAN_S32K1x_FLEXCAN =
{
    .Init = _FMSTR_FlexCAN_Init,
    .EnableTxInterrupt = _FMSTR_FlexCAN_EnableTxInterrupt,
    .EnableRxInterrupt = _FMSTR_FlexCAN_EnableRxInterrupt,
    .EnableRx =        _FMSTR_FlexCAN_EnableRx,
    .GetRxFrameLen =   _FMSTR_FlexCAN_GetRxFrameLen,
    .GetRxFrameByte =  _FMSTR_FlexCAN_GetRxFrameByte,
    .AckRxFrame =      _FMSTR_FlexCAN_AckRxFrame,
    .PrepareTxFrame =  _FMSTR_FlexCAN_PrepareTxFrame,
    .PutTxFrameByte =  _FMSTR_FlexCAN_PutTxFrameByte,
    .SendTxFrame =     _FMSTR_FlexCAN_SendTxFrame,
};

/******************************************************************************
* Implementation
******************************************************************************/
#if FMSTR_LONG_INTR || FMSTR_SHORT_INTR
void can_pal_callback(uint32_t instance, can_event_t eventType, uint32_t buffIdx, void *driverState)
{
    (void)driverState;
    (void)instance;
    switch(eventType)
    {
    case CAN_EVENT_RX_COMPLETE:
        FMSTR_ProcessCanRx();
        CAN_Receive(fmstr_canInstance, FMSTR_FLEXCAN_RXMB, &fmstr_rxmsg);
        break;
    case CAN_EVENT_TX_COMPLETE:
        /* Acknowledge frame transmission */
        fmstr_txmsg.length = 0;
        /* Send next frame, if needed */
        FMSTR_ProcessCanTx();
        break;
    default:
        break;
    }
}
#endif

static FMSTR_BOOL _FMSTR_FlexCAN_Init(FMSTR_U32 idRx, FMSTR_U32 idTx)
{
    /* initialize Rx MB */
    FMSTR_MemSet(&fmstr_rxmsg, 0, sizeof(fmstr_rxmsg));
    FMSTR_MemSet(&fmstr_rxcfg, 0, sizeof(fmstr_rxcfg));
    fmstr_rxcfg.idType = (idRx & FMSTR_CAN_EXTID) ? CAN_MSG_ID_EXT : CAN_MSG_ID_STD;
    fmstr_rxcfg.isRemote = 0u;//mean it's frame data type
    fmstr_rxmsg.id = idRx;
    CAN_ConfigRxBuff(fmstr_canInstance, FMSTR_FLEXCAN_RXMB, &fmstr_rxcfg, fmstr_rxmsg.id);
    
    /* initialize Tx MB */
    FMSTR_MemSet(&fmstr_txmsg, 0, sizeof(fmstr_txmsg));
    FMSTR_MemSet(&fmstr_txcfg, 0, sizeof(fmstr_txcfg));
    fmstr_txcfg.idType = (idTx & FMSTR_CAN_EXTID) ? CAN_MSG_ID_EXT : CAN_MSG_ID_STD;
    fmstr_txcfg.isRemote = 0u;//mean it's frame data type
    fmstr_txmsg.id = idTx;
    CAN_ConfigTxBuff(fmstr_canInstance, FMSTR_FLEXCAN_TXMB, &fmstr_txcfg);
   
#if FMSTR_LONG_INTR || FMSTR_SHORT_INTR
    CAN_InstallEventCallback(fmstr_canInstance, can_pal_callback, NULL);
#endif

    return FMSTR_TRUE;
}

static void _FMSTR_FlexCAN_EnableTxInterrupt(FMSTR_BOOL enable)
{
}

static void _FMSTR_FlexCAN_EnableRxInterrupt(FMSTR_BOOL enable)
{
    if (enable)
    {
        CAN_Receive(fmstr_canInstance, FMSTR_FLEXCAN_RXMB, &fmstr_rxmsg);
    }
}

static void _FMSTR_FlexCAN_EnableRx(void)
{
    CAN_ConfigRxBuff(fmstr_canInstance, FMSTR_FLEXCAN_RXMB, &fmstr_rxcfg, fmstr_rxmsg.id);
}

static FMSTR_SIZE8 _FMSTR_FlexCAN_GetRxFrameLen(void)
{
    if(!fmstr_rxmsg.length)
    {
        /* Try to read, when successfull, the MB is acknowledged and set for next receive */
        CAN_Receive(fmstr_canInstance, FMSTR_FLEXCAN_RXMB, &fmstr_rxmsg);
    }
    /* we have got some frame, return its length */
    return fmstr_rxmsg.length;
}

static FMSTR_BCHR _FMSTR_FlexCAN_GetRxFrameByte(FMSTR_SIZE8 index)
{
    FMSTR_BCHR data;
    data = fmstr_rxmsg.data[index];
    return data;
}

static void _FMSTR_FlexCAN_AckRxFrame(void)
{
    /* The frame is already acknowledged in registers by calling FLEXCAN_ReadRxMb before. */
    /* We only clear the local cached buffer so it appears as if we have acknowledged it. */
    fmstr_rxmsg.length = 0;
}

static FMSTR_BOOL _FMSTR_FlexCAN_PrepareTxFrame(void)
{
#if FMSTR_POLL_DRIVEN
    /* Acknowledge frame was transmitted */
    fmstr_txmsg.length = 0;
#else
    if(fmstr_txmsg.length)
        return FMSTR_FALSE;
#endif /* FMSTR_POLL_DRIVEN */
    
    /* set as transmit-emtpy MB */
    CAN_ConfigTxBuff(fmstr_canInstance, FMSTR_FLEXCAN_TXMB, &fmstr_txcfg);
    return FMSTR_TRUE;
}

static void _FMSTR_FlexCAN_PutTxFrameByte(FMSTR_SIZE8 index, FMSTR_BCHR data)
{
    /* need switch as data bytes are not necessarily ordered in the frame */
    fmstr_txmsg.data[index] = data;
}

static void _FMSTR_FlexCAN_SendTxFrame(FMSTR_SIZE8 len)
{
    fmstr_txmsg.length = len;
    CAN_Send(fmstr_canInstance, FMSTR_FLEXCAN_TXMB, &fmstr_txmsg);
#if FMSTR_POLL_DRIVEN
    while(CAN_GetTransferStatus(fmstr_canInstance, FMSTR_FLEXCAN_TXMB) != STATUS_SUCCESS);
#endif
}

/******************************************************************************
*
* @brief    Assigning FreeMASTER communication module base address
*
******************************************************************************/
void FMSTR_CanSetBaseAddress(CAN_Type *base)
{
    /*this function has not been used due to use of PAL functions for Freemaster comunication,
    and has been replaced with FMSTR_CanSetInstance(can_instance_t *instance) */
    FMSTR_ASSERT(FMSTR_FALSE);
    fmstr_canBaseAddr = base;
}

/******************************************************************************
*
* @brief    Assigning FreeMASTER communication module instance
*
******************************************************************************/
void FMSTR_CanSetInstance(can_instance_t *instance)
{
    /*this function replaces FMSTR_CanSetBaseAddress(CAN_Type *base) */
    fmstr_canInstance = instance;
}

#else /* (!(FMSTR_DISABLE)) */

void FMSTR_CanSetBaseAddress(CAN_Type *base)
{
    FMSTR_UNUSED(base);
}

#endif /* (!(FMSTR_DISABLE)) */
#endif /* defined(FMSTR_CAN_DRV) && (FMSTR_MK_IDSTR(FMSTR_CAN_DRV) == FMSTR_CAN_S32K1x_FLEXCAN_ID) */
