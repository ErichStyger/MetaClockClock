/*
 * Copyright 2025 NXP
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
 * FreeMASTER Communication Driver - FlexCAN low-level driver for CAN-FD
 */

#include "freemaster.h"
#include "freemaster_private.h"

/* Compile this code only if the CAN FD driver is selected in freemaster_cfg.h */
#if defined(FMSTR_CAN_DRV) && (FMSTR_MK_IDSTR(FMSTR_CAN_DRV) == FMSTR_CAN_MCUX_FLEXCANFD_ID)

#include "freemaster_flexcanfd.h"

/* Flexible data rate supported */
#if !(defined(FSL_FEATURE_FLEXCAN_HAS_FLEXIBLE_DATA_RATE) && FSL_FEATURE_FLEXCAN_HAS_FLEXIBLE_DATA_RATE != 0)
#error CAN flexible data rate not supported!
#endif

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

static FMSTR_BOOL _FMSTR_FlexCANFD_Init(FMSTR_U32 idRx,
                                        FMSTR_U32 idTx);              /* Initialize CAN module on a given base address. */
static void _FMSTR_FlexCANFD_EnableTxInterrupt(FMSTR_BOOL enable);    /* Enable CAN Transmit interrupt. */
static void _FMSTR_FlexCANFD_EnableRxInterrupt(FMSTR_BOOL enable);    /* Enable CAN Receive interrupt. */
static void _FMSTR_FlexCANFD_EnableRx(void);                          /* Enable/re-initialize Receiver buffer. */
static FMSTR_SIZE8 _FMSTR_FlexCANFD_GetRxFrameLen(void);              /* Return size of received CAN frame. */
static FMSTR_BCHR _FMSTR_FlexCANFD_GetRxFrameByte(FMSTR_SIZE8 index); /* Get data byte at index (0..8). */
static void _FMSTR_FlexCANFD_AckRxFrame(void);                        /* Discard received frame and enable receiving a next one. */
static FMSTR_BOOL _FMSTR_FlexCANFD_PrepareTxFrame(void);              /* Initialize transmit buffer. */
static void _FMSTR_FlexCANFD_PutTxFrameByte(FMSTR_SIZE8 index, FMSTR_BCHR data); /* Fill one byte of transmit data. */
static void _FMSTR_FlexCANFD_SendTxFrame(FMSTR_SIZE8 len);            /* Send the Tx buffer. */
static void _FMSTR_FlexCANFD_GetCaps(FMSTR_CAN_IF_CAPS *caps);        /* Get driver capabilities */

/******************************************************************************
 * Local variables
 ******************************************************************************/

/* Serial base address */
#ifdef FMSTR_CAN_BASE
static CAN_Type *fmstr_canBaseAddr = FMSTR_CAN_BASE;
#else
static CAN_Type *fmstr_canBaseAddr = NULL;
#endif

static flexcan_rx_mb_config_t fmstr_rxcfg;    /* MB configuration with Raw ID */
static flexcan_fd_frame_t fmstr_rxmsg;        /* Received frame buffer, valid when length>0 */
static flexcan_fd_frame_t fmstr_txmsg;        /* Buffer to prepare transmission */

/******************************************************************************
 * Driver interface
 ******************************************************************************/

const FMSTR_CAN_DRV_INTF FMSTR_CAN_MCUX_FLEXCANFD = {
    .Init               = _FMSTR_FlexCANFD_Init,
    .EnableTxInterrupt  = _FMSTR_FlexCANFD_EnableTxInterrupt,
    .EnableRxInterrupt  = _FMSTR_FlexCANFD_EnableRxInterrupt,
    .EnableRx           = _FMSTR_FlexCANFD_EnableRx,
    .GetRxFrameLen      = _FMSTR_FlexCANFD_GetRxFrameLen,
    .GetRxFrameByte     = _FMSTR_FlexCANFD_GetRxFrameByte,
    .AckRxFrame         = _FMSTR_FlexCANFD_AckRxFrame,
    .PrepareTxFrame     = _FMSTR_FlexCANFD_PrepareTxFrame,
    .PutTxFrameByte     = _FMSTR_FlexCANFD_PutTxFrameByte,
    .SendTxFrame        = _FMSTR_FlexCANFD_SendTxFrame,
    .GetCaps            = _FMSTR_FlexCANFD_GetCaps
};

/******************************************************************************
 * Implementation
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_FlexCANFD_Init(FMSTR_U32 idRx, FMSTR_U32 idTx)
{
    /* initialize Rx MB */
    FMSTR_MemSet(&fmstr_rxmsg, 0, sizeof(fmstr_rxmsg));
    FMSTR_MemSet(&fmstr_rxcfg, 0, sizeof(fmstr_rxcfg));
    fmstr_rxcfg.id     = FMSTR_FCAN_ID2IDR(idRx);
    fmstr_rxcfg.format = (idRx & FMSTR_CAN_EXTID) != 0U ? kFLEXCAN_FrameFormatExtend : kFLEXCAN_FrameFormatStandard;
    fmstr_rxcfg.type   = kFLEXCAN_FrameTypeData;
    FLEXCAN_SetFDRxMbConfig(fmstr_canBaseAddr, FMSTR_FLEXCAN_RXMB, &fmstr_rxcfg, false);

    /* initialize Tx MB */
    FMSTR_MemSet(&fmstr_txmsg, 0, sizeof(fmstr_txmsg));
    fmstr_txmsg.id = FMSTR_FCAN_ID2IDR(idTx);
    fmstr_txmsg.format =
        (FMSTR_U8)((idTx & FMSTR_CAN_EXTID) != 0U ? kFLEXCAN_FrameFormatExtend : kFLEXCAN_FrameFormatStandard);
    fmstr_txmsg.type = (FMSTR_U8)kFLEXCAN_FrameTypeData;
    FLEXCAN_SetFDTxMbConfig(fmstr_canBaseAddr, FMSTR_FLEXCAN_TXMB, false);

    {
        /* Make sure the RX Message Buffer is unlocked. */
        volatile uint32_t dummy = fmstr_canBaseAddr->TIMER;
        FMSTR_UNUSED(dummy);
    }

    return FMSTR_TRUE;
}

static void _FMSTR_FlexCANFD_EnableTxInterrupt(FMSTR_BOOL enable)
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

static void _FMSTR_FlexCANFD_EnableRxInterrupt(FMSTR_BOOL enable)
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

static void _FMSTR_FlexCANFD_EnableRx(void)
{
    /* enbale receiving into the MB */
    FLEXCAN_SetFDRxMbConfig(fmstr_canBaseAddr, FMSTR_FLEXCAN_RXMB, &fmstr_rxcfg, true);
}

static FMSTR_SIZE8 _FMSTR_FlexCANFD_GetRxFrameLen(void)
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
    if (fmstr_rxmsg.length == 0U)
    {
        /* Try to read data, when successful, the MB is acknowledged and set for next receive */
        status_t s = FLEXCAN_ReadFDRxMb(fmstr_canBaseAddr, FMSTR_FLEXCAN_RXMB, &fmstr_rxmsg);

        /* Make sure the RX Message Buffer is unlocked. */
        volatile uint32_t dummy = fmstr_canBaseAddr->TIMER;
        FMSTR_UNUSED(dummy);

        if (s == kStatus_Fail)
        {
            fmstr_rxmsg.length = 0U; /* no frame available */
        }
    }

    /* we have got some frame, return its length */
    return (FMSTR_SIZE8)DLC_LENGTH_DECODE(fmstr_rxmsg.length);
}

static FMSTR_BCHR _FMSTR_FlexCANFD_GetRxFrameByte(FMSTR_SIZE8 index)
{
    FMSTR_BCHR dataByte;

    /* Get word index */
    FMSTR_U8 wordIdx = index / 4;
    /* Get byte index */
    FMSTR_U8 byteIdx = index % 4;
    /* Get data word */
    uint32_t dataWord = fmstr_rxmsg.dataWord[wordIdx];
    /* Get byte from data word */
    dataByte = (dataWord >> ((3 - byteIdx) * 8)) & 0xFF;

    return dataByte;
}

static void _FMSTR_FlexCANFD_AckRxFrame(void)
{
    /* The frame is already acknowledged in registers by calling FLEXCAN_ReadFDRxMb before. */
    /* We only clear the local cached buffer so it appears as if we have acknowledged it. */
    fmstr_rxmsg.length = 0U;
}

static FMSTR_BOOL _FMSTR_FlexCANFD_PrepareTxFrame(void)
{
#if FMSTR_POLL_DRIVEN > 0
    /* Was all data sent? */
    if (fmstr_txmsg.length > 0U && FLEXCAN_GetMbStatusFlags(fmstr_canBaseAddr, FMSTR_FLEXCAN_TXMB_FLAG) == 0U)
    {
        return FMSTR_FALSE;
    }

    /* Clear TX status flag */
    FLEXCAN_ClearMbStatusFlags(fmstr_canBaseAddr, FMSTR_FLEXCAN_TXMB_FLAG);

    /* Acknowledge frame was transmitted */
    fmstr_txmsg.length = 0U;
#else
    if (fmstr_txmsg.length > 0U)
    {
        return FMSTR_FALSE;
    }
#endif /* FMSTR_POLL_DRIVEN */

    /* set as transmit-emtpy MB */
    FLEXCAN_SetFDTxMbConfig(fmstr_canBaseAddr, FMSTR_FLEXCAN_TXMB, true);

    return FMSTR_TRUE;
}

static void _FMSTR_FlexCANFD_PutTxFrameByte(FMSTR_SIZE8 index, FMSTR_BCHR data)
{
    /* calculate data byte position within the data frame word */
    FMSTR_U8 wordIdx = index / 4;
    FMSTR_U8 byteIdx = index % 4;
    uint32_t dataWord = fmstr_txmsg.dataWord[wordIdx];
    FMSTR_U8 shift = (3 - byteIdx) * 8;
    uint32_t byteMask = 0xFF << shift;
    dataWord &= ~byteMask;

    /* Set byte in data word */
    fmstr_txmsg.dataWord[wordIdx] = dataWord | (data << shift);
}

static void _FMSTR_FlexCANFD_SendTxFrame(FMSTR_SIZE8 len)
{
    status_t status;
    FMSTR_SIZE8 dlc = len;

#if defined(FMSTR_CAN_USE_CANFD) && FMSTR_CAN_USE_CANFD !=0
    /* set right data length code */
    if (len > 8U)
    {
        switch (len)
        {
        case 12U:
            dlc = kFLEXCAN_12BperFrame;
            break;
        case 16U:
            dlc = kFLEXCAN_16BperFrame;
            break;
        case 20U:
            dlc = kFLEXCAN_20BperFrame;
            break;
        case 24U:
            dlc = kFLEXCAN_24BperFrame;
            break;
        case 32U:
            dlc = kFLEXCAN_32BperFrame;
            break;
        case 48U:
            dlc = kFLEXCAN_48BperFrame;
            break;
        case 64U:
            dlc = kFLEXCAN_64BperFrame;
            break;

        default:
            FMSTR_ASSERT(FMSTR_FALSE);
            dlc = kFLEXCAN_8BperFrame;
            break;
        }
    }

    /* set "Extended Data Length" flag */
    fmstr_txmsg.edl = 1U;

    /* set "Bit Rate Switch" flag */
#if defined(FMSTR_CANFD_USE_BRS) && FMSTR_CANFD_USE_BRS != 0
    fmstr_txmsg.brs = 1U;
#else
    fmstr_txmsg.brs = 0U;
#endif

#else /* FMSTR_CAN_USE_CANFD */

    /* normal CAN mode */
    if (dlc > 8U)
    {
        FMSTR_ASSERT(FMSTR_FALSE);
        dlc = 8U;
    }

     /* clear flags */
    fmstr_txmsg.edl = 0U;
    fmstr_txmsg.brs = 0U;

#endif /* FMSTR_CAN_USE_CANFD */

    /* set length */
    fmstr_txmsg.length = dlc;

    /* send frame */
    status = FLEXCAN_WriteFDTxMb(fmstr_canBaseAddr, FMSTR_FLEXCAN_TXMB, &fmstr_txmsg);
    FMSTR_UNUSED(status);
}

static void _FMSTR_FlexCANFD_GetCaps(FMSTR_CAN_IF_CAPS *caps)
{
    FMSTR_ASSERT(caps != NULL);

    caps->flags |= FMSTR_CAN_IF_CAPS_FLAG_FD;
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
            fmstr_txmsg.length = 0U;
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
#endif /* defined(FMSTR_CAN_DRV) && (FMSTR_MK_IDSTR(FMSTR_CAN_DRV) == FMSTR_CAN_MCUX_FLEXCANFD_ID) */
