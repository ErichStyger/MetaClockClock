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
 * FreeMASTER Communication Driver - MCAN low-level driver
 */

#include "freemaster.h"
#include "freemaster_private.h"

/* Compile this code only if the CAN driver is selected in freemaster_cfg.h. */
#if defined(FMSTR_CAN_DRV) && (FMSTR_MK_IDSTR(FMSTR_CAN_DRV) == FMSTR_CAN_MCUX_MCAN_ID)

#include "freemaster_mcan.h"

#if FMSTR_DISABLE == 0

#include "freemaster_can.h"
#include "fsl_mcan.h"

/******************************************************************************
 * Configuration
 ******************************************************************************/

/* MCAN  needs to know offset of the mcan shared memory, offsets of the buffers into shared memory,
 * transmit/receive MB numbers */
#ifndef FMSTR_MCAN_TXMB_OFFSET
#error "MCAN transmit buffers offset in shared memory must be defined"
#endif

#ifndef FMSTR_MCAN_RXMB_OFFSET
#error "MCAN receive buffers offset in shared memory must be defined"
#endif

#ifndef FMSTR_MCAN_SHAREDMEMORY_OFFSET
#error "MCAN shared memory address must be defined"
#endif

/******************************************************************************
 * Local macros
 ******************************************************************************/

/* FCAN: id to id-raw (idr) translation */
#define FMSTR_FCAN_ID2IDR(id) (((id)&FMSTR_CAN_EXTID) != 0U ? MCAN_ID_EXT(id) : MCAN_ID_STD(id))

#define FMSTR_MCAN_STD_FILTER_OFS 0x0U
#define FMSTR_MCAN_STDID_OFFSET   18U

/* Data size of mCAN (8 bytes is for normal CAN, 32 bytes is for CAN FD) */
#define FMSTR_MCAN_CAN_DATASIZE 8U

/* Select Rx FiFo */
#if defined(FMSTR_MCAN_RX_FIFO) && (FMSTR_MCAN_RX_FIFO == 1)
#define FMSTR_MCAN_RX_FIFO1
#else
#define FMSTR_MCAN_RX_FIFO0
#endif

/******************************************************************************
 * Local functions
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_MCAN_Init(FMSTR_U32 idRx, FMSTR_U32 idTx); /* Initialize CAN module on a given base address. */
static void _FMSTR_MCAN_EnableTxInterrupt(FMSTR_BOOL enable);       /* Enable CAN Transmit interrupt. */
static void _FMSTR_MCAN_EnableRxInterrupt(FMSTR_BOOL enable);       /* Enable CAN Receive interrupt. */
static void _FMSTR_MCAN_EnableRx(void);                             /* Enable/re-initialize Receiver buffer. */
static FMSTR_SIZE8 _FMSTR_MCAN_GetRxFrameLen(void);                 /* Return size of received CAN frame. */
static FMSTR_BCHR _FMSTR_MCAN_GetRxFrameByte(FMSTR_SIZE8 index);    /* Get data byte at index (0..8). */
static void _FMSTR_MCAN_AckRxFrame(void);           /* Discard received frame and enable receiving a next one. */
static FMSTR_BOOL _FMSTR_MCAN_PrepareTxFrame(void); /* Initialize transmit buffer. */
static void _FMSTR_MCAN_PutTxFrameByte(FMSTR_SIZE8 index, FMSTR_BCHR data); /* Fill one byte of transmit data. */
static void _FMSTR_MCAN_SendTxFrame(FMSTR_SIZE8 len);                       /* Send the Tx buffer. */

/******************************************************************************
 * Local variables
 ******************************************************************************/

/* Serial base address */
#ifdef FMSTR_CAN_BASE
static CAN_Type *fmstr_canBaseAddr = FMSTR_CAN_BASE;
#else
static CAN_Type *fmstr_canBaseAddr = NULL;
#endif

static mcan_frame_filter_config_t rxFilter;        /* RX filter config */
static mcan_std_filter_element_config_t stdFilter; /* RX message filter config */

static mcan_rx_buffer_frame_t fmstr_rxmsg; /* Received frame buffer, valid when length>0 */
static mcan_tx_buffer_frame_t fmstr_txmsg; /* Buffer to prepare transmission */

static uint8_t txData[FMSTR_MCAN_CAN_DATASIZE];

/******************************************************************************
 * Driver interface
 ******************************************************************************/

const FMSTR_CAN_DRV_INTF FMSTR_CAN_MCUX_MCAN = {
    .Init              = _FMSTR_MCAN_Init,
    .EnableTxInterrupt = _FMSTR_MCAN_EnableTxInterrupt,
    .EnableRxInterrupt = _FMSTR_MCAN_EnableRxInterrupt,
    .EnableRx          = _FMSTR_MCAN_EnableRx,
    .GetRxFrameLen     = _FMSTR_MCAN_GetRxFrameLen,
    .GetRxFrameByte    = _FMSTR_MCAN_GetRxFrameByte,
    .AckRxFrame        = _FMSTR_MCAN_AckRxFrame,
    .PrepareTxFrame    = _FMSTR_MCAN_PrepareTxFrame,
    .PutTxFrameByte    = _FMSTR_MCAN_PutTxFrameByte,
    .SendTxFrame       = _FMSTR_MCAN_SendTxFrame,
};

/******************************************************************************
 * Implementation
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_MCAN_Init(FMSTR_U32 idRx, FMSTR_U32 idTx)
{
    mcan_rx_fifo_config_t rxFifo;
    mcan_tx_buffer_config_t txBuffer;

    /* Set Message RAM base address and clear to avoid BEU/BEC error. */
    MCAN_SetMsgRAMBase(fmstr_canBaseAddr, FMSTR_MCAN_SHAREDMEMORY_OFFSET);
    uint32_t *p = (uint32_t *)(FMSTR_MCAN_SHAREDMEMORY_OFFSET);
    FMSTR_MemSet(p, 0, (8U + FMSTR_MCAN_CAN_DATASIZE) * sizeof(uint8_t));

    /* STD filter config. */
    rxFilter.address  = FMSTR_MCAN_STD_FILTER_OFS;
    rxFilter.idFormat = (idRx & FMSTR_CAN_EXTID) ? kMCAN_FrameIDExtend : kMCAN_FrameIDStandard;
    rxFilter.listSize = 1U;
    rxFilter.nmFrame  = kMCAN_reject0;
    rxFilter.remFrame = kMCAN_rejectFrame;
    MCAN_SetFilterConfig(fmstr_canBaseAddr, &rxFilter);

#ifdef FMSTR_MCAN_RX_FIFO0
    stdFilter.sfec = kMCAN_storeinFifo0;
#else
    stdFilter.sfec = kMCAN_storeinFifo1;
#endif
    stdFilter.sft   = kMCAN_classic; /* Classic filter mode, only filter matching ID. */
    stdFilter.sfid1 = idRx;          /* RX Filter ID */
    stdFilter.sfid2 = 0;             /* IT MUST BE NULL FOR USED RX BUFFER! */

    /* Prepare RX buffer */
    FMSTR_MemSet(&fmstr_rxmsg, 0, sizeof(fmstr_rxmsg));

    /* Prepare TX buffer */
    FMSTR_MemSet(&fmstr_txmsg, 0, sizeof(fmstr_txmsg));
    fmstr_txmsg.xtd  = (idTx & FMSTR_CAN_EXTID) ? kMCAN_FrameIDExtend : kMCAN_FrameIDStandard;
    fmstr_txmsg.rtr  = kMCAN_FrameTypeData;
    fmstr_txmsg.id   = idTx << FMSTR_MCAN_STDID_OFFSET;
    fmstr_txmsg.data = txData;
#if (defined(USE_CANFD) && USE_CANFD)
    fmstr_txmsg.fdf = 1;
    fmstr_txmsg.dlc = 13U;
#endif

    /* RX fifo config. */
    FMSTR_MemSet(&rxFifo, 0, sizeof(rxFifo));
    rxFifo.address       = FMSTR_MCAN_RXMB_OFFSET;
    rxFifo.elementSize   = 1U;
    rxFifo.watermark     = 0;
    rxFifo.opmode        = kMCAN_FifoBlocking;
    rxFifo.datafieldSize = kMCAN_8ByteDatafield;
#if (defined(USE_CANFD) && USE_CANFD)
    rxFifo.datafieldSize = kMCAN_32ByteDatafield;
#endif

#ifdef FMSTR_MCAN_RX_FIFO0
    MCAN_SetRxFifo0Config(fmstr_canBaseAddr, &rxFifo);
#else
    MCAN_SetRxFifo1Config(fmstr_canBaseAddr, &rxFifo);
#endif

    /* TX buffer config. */
    FMSTR_MemSet(&txBuffer, 0, sizeof(txBuffer));
    txBuffer.mode          = kMCAN_txQueue;
    txBuffer.address       = FMSTR_MCAN_TXMB_OFFSET;
    txBuffer.dedicatedSize = 1U;
    txBuffer.fqSize        = 0;
    txBuffer.datafieldSize = kMCAN_8ByteDatafield;
#if (defined(USE_CANFD) && USE_CANFD)
    txBuffer.datafieldSize = kMCAN_32ByteDatafield;
#endif
    MCAN_SetTxBufferConfig(fmstr_canBaseAddr, &txBuffer);

    /* Enter normal mode. */
    MCAN_EnterNormalMode(fmstr_canBaseAddr);

    return FMSTR_TRUE;
}

static void _FMSTR_MCAN_EnableTxInterrupt(FMSTR_BOOL enable)
{
    if (enable != FMSTR_FALSE)
    {
        MCAN_EnableInterrupts(fmstr_canBaseAddr, 0, CAN_IE_TCE_MASK);
        MCAN_EnableTransmitBufferInterrupts(fmstr_canBaseAddr, 0);
    }
    else
    {
        MCAN_DisableInterrupts(fmstr_canBaseAddr, CAN_IE_TCE_MASK);
        MCAN_DisableTransmitBufferInterrupts(fmstr_canBaseAddr, 0);
    }
}

static void _FMSTR_MCAN_EnableRxInterrupt(FMSTR_BOOL enable)
{
    if (enable != FMSTR_FALSE)
    {
        MCAN_EnableInterrupts(fmstr_canBaseAddr, 0, CAN_IE_RF0NE_MASK);
    }
    else
    {
        MCAN_DisableInterrupts(fmstr_canBaseAddr, CAN_IE_RF0NE_MASK);
    }
}

static void _FMSTR_MCAN_EnableRx(void)
{
    MCAN_SetSTDFilterElement(fmstr_canBaseAddr, &rxFilter, &stdFilter, 0);
}

static FMSTR_SIZE8 _FMSTR_MCAN_GetRxFrameLen(void)
{
#if FMSTR_POLL_DRIVEN > 0
    /* Is any data received? */
    if (MCAN_GetStatusFlag(fmstr_canBaseAddr, CAN_IR_RF0N_MASK) == 0U)
    {
        return 0;
    }

    /* Clear RX status flag */
    MCAN_ClearStatusFlag(fmstr_canBaseAddr, CAN_IR_RF0N_MASK);
#endif /* FMSTR_POLL_DRIVEN */

    /* Current cache still valid? */
    if (fmstr_rxmsg.dlc == 0U)
    {
        /* Try to read, when successful, the MB is acknowledged and set for next receive */
        if (MCAN_ReadRxFifo(fmstr_canBaseAddr, 0, &fmstr_rxmsg) == kStatus_Fail)
        {
            return 0; /* no frame available */
        }
    }

    /* we have got some frame, return its size */
    return fmstr_rxmsg.dlc;
}

static FMSTR_BCHR _FMSTR_MCAN_GetRxFrameByte(FMSTR_SIZE8 index)
{
    FMSTR_ASSERT(fmstr_rxmsg.data != NULL);
    FMSTR_ASSERT(index < fmstr_rxmsg.dlc);

    return fmstr_rxmsg.data[index];
}

static void _FMSTR_MCAN_AckRxFrame(void)
{
    /* The frame is already acknowledged in registers by calling MCAN_ReadRxFifo before. */
    /* We only clear the local cached buffer so it appears as if we have acknowledged it. */
    fmstr_rxmsg.dlc = 0U;
}

static FMSTR_BOOL _FMSTR_MCAN_PrepareTxFrame(void)
{
    if (MCAN_IsTransmitOccurred(fmstr_canBaseAddr, 0) != 0U)
    {
        /* Acknowledge frame was transmitted */
        fmstr_txmsg.dlc = 0U;
    }

    if (fmstr_txmsg.dlc != 0U)
    {
        return FMSTR_FALSE;
    }

    fmstr_txmsg.size = 0U;
    return FMSTR_TRUE;
}

static void _FMSTR_MCAN_PutTxFrameByte(FMSTR_SIZE8 index, FMSTR_BCHR data)
{
    FMSTR_ASSERT(fmstr_txmsg.data != NULL);
    FMSTR_ASSERT(index < FMSTR_MCAN_CAN_DATASIZE);

    fmstr_txmsg.data[index] = data;
}

static void _FMSTR_MCAN_SendTxFrame(FMSTR_SIZE8 len)
{
    fmstr_txmsg.dlc  = len;
    fmstr_txmsg.size = len;

    if (MCAN_WriteTxBuffer(fmstr_canBaseAddr, 0, &fmstr_txmsg) == kStatus_Success)
        MCAN_TransmitAddRequest(fmstr_canBaseAddr, 0);
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
    uint32_t status = MCAN_GetStatusFlag(fmstr_canBaseAddr, CAN_IR_RF0N_MASK | CAN_IR_TC_MASK);

    /* Rx interrupt */
    if ((status & CAN_IR_RF0N_MASK) != 0U)
    {
        FMSTR_ProcessCanRx();
    }

    /* Tx done interrupt */
    if ((status & CAN_IR_TC_MASK) != 0U)
    {
        /* Acknowledge frame transmission */
        fmstr_txmsg.dlc = 0U;
        /* Send next frame, if needed */
        FMSTR_ProcessCanTx();
    }

    if (status != 0U)
    {
        MCAN_ClearStatusFlag(fmstr_canBaseAddr, status);
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
#endif /* defined(FMSTR_CAN_DRV) && (FMSTR_MK_IDSTR(FMSTR_CAN_DRV) == FMSTR_CAN_MCUX_MCAN_ID) */
