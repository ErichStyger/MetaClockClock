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
 * FreeMASTER Communication Driver - msCAN low-level driver
 */

#include "freemaster.h"
#include "freemaster_private.h"

/* Compile this code only if the CAN driver is selected in freemaster_cfg.h. */
#if defined(FMSTR_CAN_DRV) && (FMSTR_MK_IDSTR(FMSTR_CAN_DRV) == FMSTR_CAN_MCUX_DSCMSCAN_ID)

#include "freemaster_mscan_dsc.h"

#if FMSTR_DISABLE == 0

#include "freemaster_can.h"
#include "fsl_mscan.h"

/******************************************************************************
 * Local macros
 ******************************************************************************/

/******************************************************************************
 * Local functions
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_MsCAN_Init(FMSTR_U32 idRx, FMSTR_U32 idTx); /* Initialize CAN module. */
static void _FMSTR_MsCAN_EnableTxInterrupt(FMSTR_BOOL enable);       /* Enable CAN Transmit interrupt. */
static void _FMSTR_MsCAN_EnableRxInterrupt(FMSTR_BOOL enable);       /* Enable CAN Receive interrupt. */
static void _FMSTR_MsCAN_EnableRx(void);                             /* Enable/re-initialize Receiver buffer. */
static FMSTR_SIZE8 _FMSTR_MsCAN_GetRxFrameLen(void);                 /* Return size of received CAN frame. */
static FMSTR_BCHR _FMSTR_MsCAN_GetRxFrameByte(FMSTR_SIZE8 index);    /* Get data byte at index (0..8). */
static void _FMSTR_MsCAN_AckRxFrame(void);           /* Discard received frame and enable receiving a next one. */
static FMSTR_BOOL _FMSTR_MsCAN_PrepareTxFrame(void); /* Initialize transmit buffer. */
static void _FMSTR_MsCAN_PutTxFrameByte(FMSTR_SIZE8 index, FMSTR_BCHR data); /* Fill one byte of transmit data. */
static void _FMSTR_MsCAN_SendTxFrame(FMSTR_SIZE8 len);                       /* Send the Tx buffer. */

/******************************************************************************
 * Local variables
 ******************************************************************************/

/* Serial base address */
#ifdef FMSTR_CAN_BASE
static CAN_Type *fmstr_canBaseAddr = FMSTR_CAN_BASE;
#else
static CAN_Type *fmstr_canBaseAddr = NULL;
#endif

static mscan_frame_t fmstr_rxmsg; /* Received frame buffer, valid when length>0 */
static mscan_frame_t fmstr_txmsg; /* Buffer to prepare transmission */
static uint8_t fmstr_txPrio;      /* transmit priority to ensure sequential transmission */

/******************************************************************************
 * Driver interface
 ******************************************************************************/

const FMSTR_CAN_DRV_INTF FMSTR_CAN_MCUX_DSCMSCAN = {
    .Init              = _FMSTR_MsCAN_Init,
    .EnableTxInterrupt = _FMSTR_MsCAN_EnableTxInterrupt,
    .EnableRxInterrupt = _FMSTR_MsCAN_EnableRxInterrupt,
    .EnableRx          = _FMSTR_MsCAN_EnableRx,
    .GetRxFrameLen     = _FMSTR_MsCAN_GetRxFrameLen,
    .GetRxFrameByte    = _FMSTR_MsCAN_GetRxFrameByte,
    .AckRxFrame        = _FMSTR_MsCAN_AckRxFrame,
    .PrepareTxFrame    = _FMSTR_MsCAN_PrepareTxFrame,
    .PutTxFrameByte    = _FMSTR_MsCAN_PutTxFrameByte,
    .SendTxFrame       = _FMSTR_MsCAN_SendTxFrame,
};

/******************************************************************************
 * Implementation
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_MsCAN_Init(FMSTR_U32 idRx, FMSTR_U32 idTx)
{
    /* initialize Rx MB */
    FMSTR_MemSet(&fmstr_rxmsg, 0, sizeof(fmstr_rxmsg));
    fmstr_rxmsg.ID_Type.ID = MSCAN_RX_MB_STD_MASK(idRx);
    fmstr_txmsg.format     = (idRx & FMSTR_CAN_EXTID) != 0U ? kMSCAN_FrameFormatExtend : kMSCAN_FrameFormatStandard;
    fmstr_txmsg.type       = kMSCAN_FrameTypeData;

    /* Configure ID acceptance filter setting. */
    uint32_t rxIdentifier = MSCAN_RX_MB_STD_MASK(idRx);
    MSCAN_WriteIDAR0(fmstr_canBaseAddr, (uint8_t *)(&rxIdentifier));
    MSCAN_WriteIDAR1(fmstr_canBaseAddr, (uint8_t *)(&rxIdentifier));

    /* initialize Tx MB */
    FMSTR_MemSet(&fmstr_txmsg, 0, sizeof(fmstr_txmsg));
    fmstr_txmsg.ID_Type.ID = idTx;
    fmstr_txmsg.format     = (idTx & FMSTR_CAN_EXTID) != 0U ? kMSCAN_FrameFormatExtend : kMSCAN_FrameFormatStandard;
    fmstr_txmsg.type       = kMSCAN_FrameTypeData;

    fmstr_txPrio = 0U;

#if FMSTR_LONG_INTR > 0 || FMSTR_SHORT_INTR > 0
    /* Disable all interrupts */
    MSCAN_DisableRxInterrupts(fmstr_canBaseAddr, 0xFF);
    MSCAN_DisableTxInterrupts(fmstr_canBaseAddr, 0xFF);
#endif /* FMSTR_LONG_INTR || FMSTR_SHORT_INTR */

    return FMSTR_TRUE;
}

static void _FMSTR_MsCAN_EnableTxInterrupt(FMSTR_BOOL enable)
{
    if (enable != FMSTR_FALSE)
    {
        MSCAN_EnableTxInterrupts(fmstr_canBaseAddr, (uint8_t)kMSCAN_TxEmptyInterruptEnable);
    }
    else
    {
        MSCAN_DisableTxInterrupts(fmstr_canBaseAddr, (uint8_t)kMSCAN_TxEmptyInterruptEnable);
    }
}

static void _FMSTR_MsCAN_EnableRxInterrupt(FMSTR_BOOL enable)
{
    if (enable != FMSTR_FALSE)
    {
        MSCAN_EnableRxInterrupts(fmstr_canBaseAddr, (uint8_t)kMSCAN_RxFullInterruptEnable);
    }
    else
    {
        MSCAN_DisableRxInterrupts(fmstr_canBaseAddr, (uint8_t)kMSCAN_RxFullInterruptEnable);
    }
}

static void _FMSTR_MsCAN_EnableRx(void)
{
    uint32_t rxMask = 0x00;
    MSCAN_WriteIDMR0(fmstr_canBaseAddr, (uint8_t *)(&rxMask));
    MSCAN_WriteIDMR1(fmstr_canBaseAddr, (uint8_t *)(&rxMask));
}

static FMSTR_SIZE8 _FMSTR_MsCAN_GetRxFrameLen(void)
{
#if FMSTR_POLL_DRIVEN > 0
    /* Is any data received? */
    if (MSCAN_GetRxBufferFullFlag(fmstr_canBaseAddr) == 0U)
    {
        return 0;
    }
#endif /* FMSTR_POLL_DRIVEN */

    /* Current cache still valid? */
    if (fmstr_rxmsg.DLR == 0U)
    {
        /* Try to read, when successfull, the MB is acknowledged and set for next receive */
        if (MSCAN_ReadRxMb(fmstr_canBaseAddr, &fmstr_rxmsg) == kStatus_Fail)
        {
            return 0; /* no frame available */
        }
    }

#if FMSTR_POLL_DRIVEN > 0
    /* Clear RX status flag */
    MSCAN_ClearRxBufferFullFlag(fmstr_canBaseAddr);
#endif /* FMSTR_POLL_DRIVEN */

    /* we have got some frame, return its length */
    return fmstr_rxmsg.DLR;
}

static FMSTR_BCHR _FMSTR_MsCAN_GetRxFrameByte(FMSTR_SIZE8 index)
{
    FMSTR_BCHR data;

    /* need switch as data bytes are not necessarily ordered in the frame */
    switch (index)
    {
        case 0:
            data = fmstr_rxmsg.dataByte0;
            break;
        case 1:
            data = fmstr_rxmsg.dataByte1;
            break;
        case 2:
            data = fmstr_rxmsg.dataByte2;
            break;
        case 3:
            data = fmstr_rxmsg.dataByte3;
            break;
        case 4:
            data = fmstr_rxmsg.dataByte4;
            break;
        case 5:
            data = fmstr_rxmsg.dataByte5;
            break;
        case 6:
            data = fmstr_rxmsg.dataByte6;
            break;
        default:
            data = fmstr_rxmsg.dataByte7;
            break;
    }

    return data;
}

static void _FMSTR_MsCAN_AckRxFrame(void)
{
    /* The frame is already acknowledged in registers by calling _FMSTR_MsCAN_GetRxFrameLen before. */
    /* We only clear the local cached buffer so it appears as if we have acknowledged it. */
    fmstr_rxmsg.DLR = 0U;

    /* Reset the priority counter for the next transmission sequence */
    fmstr_txPrio = 0U;
}

static FMSTR_BOOL _FMSTR_MsCAN_PrepareTxFrame(void)
{
#if FMSTR_POLL_DRIVEN > 0
    /* Was all data sent? */
    if (fmstr_txmsg.DLR > 0U &&
        MSCAN_GetTxBufferStatusFlags(fmstr_canBaseAddr, (uint8_t)kMSCAN_TxEmptyInterruptEnable) == 0U)
    {
        return FMSTR_FALSE;
    }

    /* Acknowledge frame was transmitted */
    fmstr_txmsg.DLR = 0U;
#else
    if (fmstr_txmsg.DLR != 0U)
    {
        return FMSTR_FALSE;
    }
#endif /* FMSTR_POLL_DRIVEN */

    return FMSTR_TRUE;
}

static void _FMSTR_MsCAN_PutTxFrameByte(FMSTR_SIZE8 index, FMSTR_BCHR data)
{
    /* need switch as data bytes are not necessarily ordered in the frame */
    switch (index)
    {
        case 0:
            fmstr_txmsg.dataByte0 = data;
            break;
        case 1:
            fmstr_txmsg.dataByte1 = data;
            break;
        case 2:
            fmstr_txmsg.dataByte2 = data;
            break;
        case 3:
            fmstr_txmsg.dataByte3 = data;
            break;
        case 4:
            fmstr_txmsg.dataByte4 = data;
            break;
        case 5:
            fmstr_txmsg.dataByte5 = data;
            break;
        case 6:
            fmstr_txmsg.dataByte6 = data;
            break;
        default:
            fmstr_txmsg.dataByte7 = data;
            break;
    }
}

static void _FMSTR_MsCAN_SendTxFrame(FMSTR_SIZE8 len)
{
    fmstr_txmsg.DLR = (uint8_t)len;

    /* Set priority for TX buffer */
    fmstr_txmsg.BPR = fmstr_txPrio++;
    /* Write data, ignore errors, we will re-transmit eventually */
    (void)MSCAN_WriteTxMb(fmstr_canBaseAddr, &fmstr_txmsg);
}

/******************************************************************************
 *
 * @brief    Assigning FreeMASTER communication module base address
 *
 ******************************************************************************/
void FMSTR_CanSetBaseAddress(CAN_Type *base)
{
    fmstr_canBaseAddr = base;
}

void FMSTR_CanIsr(void)
{
#if FMSTR_LONG_INTR > 0 || FMSTR_SHORT_INTR > 0
    /* Rx interrupt */
    if (MSCAN_GetRxBufferFullFlag(fmstr_canBaseAddr) != 0U)
    {
        FMSTR_ProcessCanRx();
        /* Clear RX status flag */
        MSCAN_ClearRxBufferFullFlag(fmstr_canBaseAddr);
    }
    /* Tx done interrupt */
    if (MSCAN_GetTxBufferStatusFlags(fmstr_canBaseAddr, kMSCAN_TxEmptyInterruptEnable) != 0U)
    {
        /* Acknowledge frame transmission */
        fmstr_txmsg.DLR = 0;
        /* Send next frame, if needed */
        FMSTR_ProcessCanTx();
    }
#endif
}

#else /* (!(FMSTR_DISABLE)) */

void FMSTR_CanSetBaseAddress(MSCAN_Type *base)
{
    FMSTR_UNUSED(base);
}

void FMSTR_CanIsr(void)
{
}

#endif /* (!(FMSTR_DISABLE)) */
#endif /* defined(FMSTR_CAN_DRV) && (FMSTR_MK_IDSTR(FMSTR_CAN_DRV) == FMSTR_CAN_MCUX_MSCAN_ID) */
