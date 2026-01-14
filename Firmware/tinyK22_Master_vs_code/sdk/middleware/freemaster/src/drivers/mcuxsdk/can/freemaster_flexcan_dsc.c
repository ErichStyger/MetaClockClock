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
 * FreeMASTER Communication Driver - DSC FlexCAN low-level driver
 */

#include "freemaster.h"
#include "freemaster_private.h"

/* Compile this code only if the CAN driver is selected in freemaster_cfg.h. */
#if defined(FMSTR_CAN_DRV) &&(FMSTR_MK_IDSTR(FMSTR_CAN_DRV) == FMSTR_CAN_MCUX_DSCFLEXCAN_ID)

#include "freemaster_flexcan_dsc.h"

#if FMSTR_DISABLE == 0

#include "freemaster_can.h"
#include "fsl_flexcan.h"

/******************************************************************************
 * Configuration
 ******************************************************************************/

/* Flexcan TX message buffer must be defined */
#ifndef FMSTR_FLEXCAN_TXMB
#warning "FlexCAN Message Buffer 1 is used for transmitting messages"
#define FMSTR_FLEXCAN_TXMB 1U
#endif

/* Flexcan RX message buffer must be defined */
#ifndef FMSTR_FLEXCAN_RXMB
#warning "FlexCAN Message Buffer 2 is used for receiving messages"
#define FMSTR_FLEXCAN_RXMB 2U
#endif

/******************************************************************************
 * Local macros
 ******************************************************************************/

/* FCAN: id to id-raw (idr) translation */
#define FMSTR_FCAN_ID2IDR(id) (((id)&FMSTR_CAN_EXTID) != 0UL ? FLEXCAN_ID_EXT(id) : FLEXCAN_ID_STD(id))

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
#define FMSTR_FLEXCAN_FLAG_ALL 0xFFFFFFFFFFFFFFFFULL
#else
#define FMSTR_FLEXCAN_FLAG_ALL 0xFFFFFFFFUL
#endif

/******************************************************************************
 * Local functions
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_FlexCAN_Init(FMSTR_U32 idRx,
                                      FMSTR_U32 idTx);              /* Initialize CAN module on a given base address. */
static void _FMSTR_FlexCAN_EnableTxInterrupt(FMSTR_BOOL enable);    /* Enable CAN Transmit interrupt. */
static void _FMSTR_FlexCAN_EnableRxInterrupt(FMSTR_BOOL enable);    /* Enable CAN Receive interrupt. */
static void _FMSTR_FlexCAN_EnableRx(void);                          /* Enable/re-initialize Receiver buffer. */
static FMSTR_SIZE8 _FMSTR_FlexCAN_GetRxFrameLen(void);              /* Return size of received CAN frame. */
static FMSTR_BCHR _FMSTR_FlexCAN_GetRxFrameByte(FMSTR_SIZE8 index); /* Get data byte at index (0..8). */
static void _FMSTR_FlexCAN_AckRxFrame(void);           /* Discard received frame and enable receiving a next one. */
static FMSTR_BOOL _FMSTR_FlexCAN_PrepareTxFrame(void); /* Initialize transmit buffer. */
static void _FMSTR_FlexCAN_PutTxFrameByte(FMSTR_SIZE8 index, FMSTR_BCHR data); /* Fill one byte of transmit data. */
static void _FMSTR_FlexCAN_SendTxFrame(FMSTR_SIZE8 len);                       /* Send the Tx buffer. */

/******************************************************************************
 * Local variables
 ******************************************************************************/

/* Serial base address */
#ifdef FMSTR_CAN_BASE
static CAN_Type *fmstr_canBaseAddr = FMSTR_CAN_BASE;
#else
static CAN_Type *fmstr_canBaseAddr = NULL;
#endif

static flexcan_rx_mb_config_t fmstr_rxcfg; /* MB configuration with Raw ID */
static flexcan_frame_t fmstr_rxmsg;        /* Received frame buffer, valid when length>0 */
static flexcan_frame_t fmstr_txmsg;        /* Buffer to prepare transmission */

/******************************************************************************
 * Driver interface
 ******************************************************************************/

const FMSTR_CAN_DRV_INTF FMSTR_CAN_MCUX_DSCFLEXCAN = {
    FMSTR_C99_INIT(Init) _FMSTR_FlexCAN_Init,
    FMSTR_C99_INIT(EnableTxInterrupt) _FMSTR_FlexCAN_EnableTxInterrupt,
    FMSTR_C99_INIT(EnableRxInterrupt) _FMSTR_FlexCAN_EnableRxInterrupt,
    FMSTR_C99_INIT(EnableRx) _FMSTR_FlexCAN_EnableRx,
    FMSTR_C99_INIT(GetRxFrameLen) _FMSTR_FlexCAN_GetRxFrameLen,
    FMSTR_C99_INIT(GetRxFrameByte) _FMSTR_FlexCAN_GetRxFrameByte,
    FMSTR_C99_INIT(AckRxFrame) _FMSTR_FlexCAN_AckRxFrame,
    FMSTR_C99_INIT(PrepareTxFrame) _FMSTR_FlexCAN_PrepareTxFrame,
    FMSTR_C99_INIT(PutTxFrameByte) _FMSTR_FlexCAN_PutTxFrameByte,
    FMSTR_C99_INIT(SendTxFrame) _FMSTR_FlexCAN_SendTxFrame,
};

/******************************************************************************
 * Implementation
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_FlexCAN_Init(FMSTR_U32 idRx, FMSTR_U32 idTx)
{
    /* initialize Rx MB */
    FMSTR_MemSet(&fmstr_rxmsg, 0, sizeof(fmstr_rxmsg));
    FMSTR_MemSet(&fmstr_rxcfg, 0, sizeof(fmstr_rxcfg));
    fmstr_rxcfg.u32Id   = FMSTR_FCAN_ID2IDR(idRx);
    fmstr_rxcfg.eFormat = (idRx & FMSTR_CAN_EXTID) != 0U ? kFLEXCAN_FrameFormatExtend : kFLEXCAN_FrameFormatStandard;
    fmstr_rxcfg.eType   = kFLEXCAN_FrameTypeData;
    FLEXCAN_SetRxMbConfig(fmstr_canBaseAddr, FMSTR_FLEXCAN_RXMB, &fmstr_rxcfg, false);

    /* initialize Tx MB */
    FMSTR_MemSet(&fmstr_txmsg, 0, sizeof(fmstr_txmsg));
    fmstr_txmsg.bitsId    = FMSTR_FCAN_ID2IDR(idTx);
    fmstr_txmsg.bitFormat = (uint32_t)((idTx & FMSTR_CAN_EXTID) != 0U ? kFLEXCAN_FrameFormatExtend : kFLEXCAN_FrameFormatStandard);
    fmstr_txmsg.bitType   = (uint32_t)kFLEXCAN_FrameTypeData;
    FLEXCAN_SetTxMbConfig(fmstr_canBaseAddr, FMSTR_FLEXCAN_TXMB, true);

    {
        /* Make sure the RX Message Buffer is unlocked. */
        volatile uint32_t dummy = fmstr_canBaseAddr->TIMER;
        FMSTR_UNUSED(dummy);
    }

    return FMSTR_TRUE;
}

static void _FMSTR_FlexCAN_EnableTxInterrupt(FMSTR_BOOL enable)
{
    if (enable != FMSTR_FALSE)
    {
        FLEXCAN_EnableMbInterrupts(fmstr_canBaseAddr, FMSTR_FLEXCAN_TXMB_FLAG);
    }
    else
    {
        FLEXCAN_DisableMbInterrupts(fmstr_canBaseAddr, FMSTR_FLEXCAN_TXMB_FLAG);
    }
}

static void _FMSTR_FlexCAN_EnableRxInterrupt(FMSTR_BOOL enable)
{
    if (enable != FMSTR_FALSE)
    {
        FLEXCAN_EnableMbInterrupts(fmstr_canBaseAddr, FMSTR_FLEXCAN_RXMB_FLAG);
    }
    else
    {
        FLEXCAN_DisableMbInterrupts(fmstr_canBaseAddr, FMSTR_FLEXCAN_RXMB_FLAG);
    }
}

static void _FMSTR_FlexCAN_EnableRx(void)
{
    /* enbale receiving into the MB */
    FLEXCAN_SetRxMbConfig(fmstr_canBaseAddr, FMSTR_FLEXCAN_RXMB, &fmstr_rxcfg, true);
}

static FMSTR_SIZE8 _FMSTR_FlexCAN_GetRxFrameLen(void)
{
#if FMSTR_POLL_DRIVEN > 0
    /* Is any data received? */
    if (FLEXCAN_GetMbStatusFlags(fmstr_canBaseAddr, FMSTR_FLEXCAN_RXMB_FLAG) == 0U)
    {
        return 0U;
    }

    /* Clear RX status flag */
    FLEXCAN_ClearMbStatusFlags(fmstr_canBaseAddr, FMSTR_FLEXCAN_RXMB_FLAG);
#endif /* FMSTR_POLL_DRIVEN */

    /* Current cache still valid? */
    if (fmstr_rxmsg.bitsLength == 0U)
    {
        /* Try to read data, when successful, the MB is acknowledged and set for next receive */
        status_t s = FLEXCAN_ReadRxMb(fmstr_canBaseAddr, FMSTR_FLEXCAN_RXMB, &fmstr_rxmsg);

        /* Make sure the RX Message Buffer is unlocked. */
        volatile uint32_t dummy = fmstr_canBaseAddr->TIMER;
        FMSTR_UNUSED(dummy);

        if (s == kStatus_Fail)
        {
            fmstr_rxmsg.bitsLength = 0U; /* no frame available */
        }
    }

    /* we have got some frame, return its length */
    return (FMSTR_SIZE8)fmstr_rxmsg.bitsLength;
}

static FMSTR_BCHR _FMSTR_FlexCAN_GetRxFrameByte(FMSTR_SIZE8 index)
{
    FMSTR_BCHR data;

    /* need switch as data bytes are not necessarily ordered in the frame */
    switch (index)
    {
        case 0:
            data = fmstr_rxmsg.u8DataByte0;
            break;
        case 1:
            data = fmstr_rxmsg.u8DataByte1;
            break;
        case 2:
            data = fmstr_rxmsg.u8DataByte2;
            break;
        case 3:
            data = fmstr_rxmsg.u8DataByte3;
            break;
        case 4:
            data = fmstr_rxmsg.u8DataByte4;
            break;
        case 5:
            data = fmstr_rxmsg.u8DataByte5;
            break;
        case 6:
            data = fmstr_rxmsg.u8DataByte6;
            break;
        default:
            data = fmstr_rxmsg.u8DataByte7;
            break;
    }

    return data;
}

static void _FMSTR_FlexCAN_AckRxFrame(void)
{
    /* The frame is already acknowledged in registers by calling FLEXCAN_ReadRxMb before. */
    /* We only clear the local cached buffer so it appears as if we have acknowledged it. */
    fmstr_rxmsg.bitsLength = 0U;
}

static FMSTR_BOOL _FMSTR_FlexCAN_PrepareTxFrame(void)
{
#if FMSTR_POLL_DRIVEN > 0
    /* Was all data sent? */
    if (fmstr_txmsg.bitsLength > 0U && FLEXCAN_GetMbStatusFlags(fmstr_canBaseAddr, FMSTR_FLEXCAN_TXMB_FLAG) == 0U)
    {
        return FMSTR_FALSE;
    }

    /* Clear TX status flag */
    FLEXCAN_ClearMbStatusFlags(fmstr_canBaseAddr, FMSTR_FLEXCAN_TXMB_FLAG);

    /* Acknowledge frame was transmitted */
    fmstr_txmsg.bitsLength = 0U;
#else
    if (fmstr_txmsg.bitsLength > 0U)
    {
        return FMSTR_FALSE;
    }
#endif /* FMSTR_POLL_DRIVEN */

    /* set as transmit-emtpy MB */
    FLEXCAN_SetTxMbConfig(fmstr_canBaseAddr, FMSTR_FLEXCAN_TXMB, true);

    return FMSTR_TRUE;
}

static void _FMSTR_FlexCAN_PutTxFrameByte(FMSTR_SIZE8 index, FMSTR_BCHR data)
{
    /* need switch as data bytes are not necessarily ordered in the frame */
    switch (index)
    {
        case 0:
            fmstr_txmsg.u8DataByte0 = data;
            break;
        case 1:
            fmstr_txmsg.u8DataByte1 = data;
            break;
        case 2:
            fmstr_txmsg.u8DataByte2 = data;
            break;
        case 3:
            fmstr_txmsg.u8DataByte3 = data;
            break;
        case 4:
            fmstr_txmsg.u8DataByte4 = data;
            break;
        case 5:
            fmstr_txmsg.u8DataByte5 = data;
            break;
        case 6:
            fmstr_txmsg.u8DataByte6 = data;
            break;
        default:
            fmstr_txmsg.u8DataByte7 = data;
            break;
    }
}

static void _FMSTR_FlexCAN_SendTxFrame(FMSTR_SIZE8 len)
{
    status_t status;

    fmstr_txmsg.bitsLength = len;

    status = FLEXCAN_WriteTxMb(fmstr_canBaseAddr, FMSTR_FLEXCAN_TXMB, &fmstr_txmsg);
    FMSTR_UNUSED(status);
}

/******************************************************************************
 *
 * @brief    Assigning FreeMASTER
 *communication module base address
 *
 ******************************************************************************/

void FMSTR_CanSetBaseAddress(CAN_Type *base)
{
    fmstr_canBaseAddr = base;
}

void FMSTR_CanIsr(void)
{
#if FMSTR_LONG_INTR > 0 || FMSTR_SHORT_INTR > 0
    uint32_t isrFlags = FLEXCAN_GetStatusFlags(fmstr_canBaseAddr);
#ifdef FMSTR_FLEXCAN_REG_SIZE64
    uint64_t isrFlagsMB;
#else
    uint32_t isrFlagsMB;
#endif

    isrFlagsMB = FLEXCAN_GetMbStatusFlags(fmstr_canBaseAddr, FMSTR_FLEXCAN_FLAG_ALL);

    /* Ignoring error and wake up flags */
    if (isrFlags != 0U)
    {
        FLEXCAN_ClearStatusFlags(fmstr_canBaseAddr, isrFlags);
    }

    if (isrFlagsMB != 0U)
    {
        /* Read or Write */
        if ((isrFlagsMB & FMSTR_FLEXCAN_RXMB_FLAG) != 0U)
        {
            FMSTR_ProcessCanRx();
        }
        if ((isrFlagsMB & FMSTR_FLEXCAN_TXMB_FLAG) != 0U)
        {
            /* Acknowledge frame transmission */
            fmstr_txmsg.bitsLength = 0U;
            /* Send next frame, if needed */
            FMSTR_ProcessCanTx();
        }

        /* Clear all interrupt flags */
        FLEXCAN_ClearMbStatusFlags(fmstr_canBaseAddr, isrFlagsMB);
    }
#endif
}

#else /* (!(FMSTR_DISABLE)) */

void FMSTR_CanSetBaseAddress(CAN_Type *base)
{
    FMSTR_UNUSED(base);
}

void FMSTR_CanIsr(void)
{
}

#endif /* (!(FMSTR_DISABLE)) */
#endif /* defined(FMSTR_CAN_DRV) && (FMSTR_MK_IDSTR(FMSTR_CAN_DRV) == FMSTR_CAN_MCUX_DSCFLEXCAN_ID) */
