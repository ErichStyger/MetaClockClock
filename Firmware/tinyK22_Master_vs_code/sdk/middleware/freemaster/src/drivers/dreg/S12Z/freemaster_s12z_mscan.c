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
 * FreeMASTER Communication Driver - msCAN low-level driver
 */

#include "freemaster.h"
#include "freemaster_private.h"

/* Compile this code only if the CAN driver is selected in freemaster_cfg.h. */
#if defined(FMSTR_CAN_DRV) && (FMSTR_MK_IDSTR(FMSTR_CAN_DRV) == FMSTR_CAN_S12Z_MSCAN_ID)
#if !(FMSTR_DISABLE)

#include "freemaster_can.h"
#include "freemaster_s12z_mscan.h"

/******************************************************************************
* Local variables
******************************************************************************/

/* Serial base address */
#ifdef FMSTR_CAN_BASE
    static FMSTR_ADDR fmstr_canBaseAddr = (FMSTR_ADDR)FMSTR_CAN_BASE;
#else
    static FMSTR_ADDR fmstr_canBaseAddr = (FMSTR_ADDR)0;
#endif

static FMSTR_U8 fmstr_canRxIDR[4];  /* ID RAW to compare with incoming frames */    
static FMSTR_U8 fmstr_canTxIDR[4];  /* ID RAW to send in outgoing frames */
static FMSTR_U8 fmstr_canTxBufSel;  /* selected transmit buffer index */

static FMSTR_U8 fmstr_canTxPrio;    /* transmit priority to ensure sequential transmission */

/******************************************************************************
* Local functions
******************************************************************************/

static FMSTR_BOOL _FMSTR_S12zCan_Init(FMSTR_U32 idRx, FMSTR_U32 idTx);  /* Initialize CAN module on a given base address. */
static void _FMSTR_S12zCan_EnableTxInterrupt(FMSTR_BOOL enable);     /* Enable CAN Transmit interrupt. */
static void _FMSTR_S12zCan_EnableRxInterrupt(FMSTR_BOOL enable);     /* Enable CAN Receive interrupt. */
static void _FMSTR_S12zCan_EnableRx(void);                           /* Enable/re-initialize Receiver buffer. */
static FMSTR_SIZE8 _FMSTR_S12zCan_GetRxFrameLen(void);               /* Return size of received CAN frame, or 0 if no Rx frame is available. */
static FMSTR_BCHR _FMSTR_S12zCan_GetRxFrameByte(FMSTR_SIZE8 index);  /* Get data byte at index (0..8). */
static void _FMSTR_S12zCan_AckRxFrame(void);                         /* Discard received frame and enable receiving a next one. */
static FMSTR_BOOL _FMSTR_S12zCan_PrepareTxFrame(void);               /* Initialize transmit buffer; return false when Tx buffer is not available. */
static void _FMSTR_S12zCan_PutTxFrameByte(FMSTR_SIZE8 index, FMSTR_BCHR data);   /* Fill one byte of transmit data. */
static void _FMSTR_S12zCan_SendTxFrame(FMSTR_SIZE8 len);             /* Send the Tx buffer. */

/******************************************************************************
* Driver interface
******************************************************************************/

const FMSTR_CAN_DRV_INTF FMSTR_CAN_S12Z_MSCAN =
{
    FMSTR_C99_INIT(Init              ) _FMSTR_S12zCan_Init,
    FMSTR_C99_INIT(EnableTxInterrupt ) _FMSTR_S12zCan_EnableTxInterrupt,
    FMSTR_C99_INIT(EnableRxInterrupt ) _FMSTR_S12zCan_EnableRxInterrupt,
    FMSTR_C99_INIT(EnableRx          ) _FMSTR_S12zCan_EnableRx,
    FMSTR_C99_INIT(GetRxFrameLen     ) _FMSTR_S12zCan_GetRxFrameLen,
    FMSTR_C99_INIT(GetRxFrameByte    ) _FMSTR_S12zCan_GetRxFrameByte,
    FMSTR_C99_INIT(AckRxFrame        ) _FMSTR_S12zCan_AckRxFrame,
    FMSTR_C99_INIT(PrepareTxFrame    ) _FMSTR_S12zCan_PrepareTxFrame,
    FMSTR_C99_INIT(PutTxFrameByte    ) _FMSTR_S12zCan_PutTxFrameByte,
    FMSTR_C99_INIT(SendTxFrame       ) _FMSTR_S12zCan_SendTxFrame,
};

/****************************************************************************************
* General peripheral space access macros
*****************************************************************************************/

#define FMSTR_SETBIT(base, offset, bit)         (*(((volatile FMSTR_U8*)(base))+(offset)) |= (bit))
#define FMSTR_CLRBIT(base, offset, bit)         (*(((volatile FMSTR_U8*)(base))+(offset)) &= (FMSTR_U8)(~(bit)))
#define FMSTR_TSTBIT(base, offset, bit)         (*(((volatile FMSTR_U8*)(base))+(offset)) & (bit))
#define FMSTR_SETREG(base, offset, value)       (*(((volatile FMSTR_U8*)(base))+(offset)) = (value))
#define FMSTR_GETREG(base, offset)              (*(((volatile FMSTR_U8*)(base))+(offset)))

/****************************************************************************************
* MSCAN module constants
*****************************************************************************************/

/* MSCAN module registers */
#define FMSTR_MSCANRFLG_OFFSET  0x04
#define FMSTR_MSCANRIER_OFFSET  0x05
#define FMSTR_MSCANTFLG_OFFSET  0x06
#define FMSTR_MSCANTIER_OFFSET  0x07
#define FMSTR_MSCANTBSEL_OFFSET 0x0a
#define FMSTR_MSCANRXFG_OFFSET  0x20
#define FMSTR_MSCANTXFG_OFFSET  0x30

/* MSCAN MB registers offsets (must also add CANxxFG_OFFSET) */
#define FMSTR_MSCMBIDR0  0
#define FMSTR_MSCMBIDR1  1
#define FMSTR_MSCMBIDR2  2
#define FMSTR_MSCMBIDR3  3
#define FMSTR_MSCMBDSR0  4
#define FMSTR_MSCMBDSR1  5
#define FMSTR_MSCMBDSR2  6
#define FMSTR_MSCMBDSR3  7
#define FMSTR_MSCMBDSR4  8
#define FMSTR_MSCMBDSR5  9
#define FMSTR_MSCMBDSR6 10
#define FMSTR_MSCMBDSR7 11
#define FMSTR_MSCMBDLR  12
#define FMSTR_MSCMBTBPR 13
#define FMSTR_MSCMBTSRH 14
#define FMSTR_MSCMBTSRL 15

/* MSCAN CANRFLG */
#define FMSTR_MSCANRFLG_RXF     0x01

/* MSCAN CANRIER */
#define FMSTR_MSCANRFLG_RXFIE   0x01

/* MSCAN CANTFLG, TIER, TARQ, TBSEL */
#define FMSTR_MSCANTBUF_TX2     0x04
#define FMSTR_MSCANTBUF_TX1     0x02
#define FMSTR_MSCANTBUF_TX0     0x01
#define FMSTR_MSCANTBUF_ALL     0x07

/* TX buffers used by this driver. Facing strange race/overrun condition when using
 * multiple buffers. Works well when using single buffer. */
#define FMSTR_MSCANTBUF_USED    0x01 /* FMSTR_MSCANTBUF_ALL */  

/* MSCAN CANIDRx */
#define FMSTR_MSCANIDR1_IDE     0x08
#define FMSTR_MSCANIDR1_STD_RTR 0x10
#define FMSTR_MSCANIDR3_EXT_RTR 0x08

/* MSCAN: id to idr translation */
#define FMSTR_MSCAN_MAKEIDR0(id) ((FMSTR_U8)(((id)&FMSTR_CAN_EXTID) ? (id)>>21 : (id)>>3))
#define FMSTR_MSCAN_MAKEIDR1(id) ((FMSTR_U8)(((id)&FMSTR_CAN_EXTID) ? ((((id)>>13) & 0xe0U) | 0x18 | (((id)>>15) & 0x07U)) : (((id)<<5)&0xe0)))
#define FMSTR_MSCAN_MAKEIDR2(id) ((FMSTR_U8)(((id)&FMSTR_CAN_EXTID) ? (id)>>7 : 0))
#define FMSTR_MSCAN_MAKEIDR3(id) ((FMSTR_U8)(((id)&FMSTR_CAN_EXTID) ? (((id)<<1) & 0xfeU) : 0))

/******************************************************************************
* Implementation
******************************************************************************/

static FMSTR_BOOL _FMSTR_S12zCan_Init(FMSTR_U32 idRx, FMSTR_U32 idTx)
{
    fmstr_canTxBufSel = 0;
    fmstr_canTxPrio = 0;
    
    fmstr_canRxIDR[0] = FMSTR_MSCAN_MAKEIDR0(idRx);
    fmstr_canRxIDR[1] = FMSTR_MSCAN_MAKEIDR1(idRx);
    fmstr_canRxIDR[2] = FMSTR_MSCAN_MAKEIDR2(idRx);
    fmstr_canRxIDR[3] = FMSTR_MSCAN_MAKEIDR3(idRx);

    fmstr_canTxIDR[0] = FMSTR_MSCAN_MAKEIDR0(idTx);
    fmstr_canTxIDR[1] = FMSTR_MSCAN_MAKEIDR1(idTx);
    fmstr_canTxIDR[2] = FMSTR_MSCAN_MAKEIDR2(idTx);
    fmstr_canTxIDR[3] = FMSTR_MSCAN_MAKEIDR3(idTx);
    
    return FMSTR_TRUE;
}

static void _FMSTR_S12zCan_EnableTxInterrupt(FMSTR_BOOL enable)
{
    /* enable/disable CAN TX interrupts */
    if(enable)
    {
        FMSTR_SETBIT(fmstr_canBaseAddr, FMSTR_MSCANTIER_OFFSET, FMSTR_MSCANTBUF_USED);
    }
    else
    {
        FMSTR_CLRBIT(fmstr_canBaseAddr, FMSTR_MSCANTIER_OFFSET, FMSTR_MSCANTBUF_USED);
    }
}

static void _FMSTR_S12zCan_EnableRxInterrupt(FMSTR_BOOL enable)
{
    /* enable/disable CAN RX interrupts */
    if(enable)
    {
        FMSTR_SETBIT(fmstr_canBaseAddr, FMSTR_MSCANRIER_OFFSET, FMSTR_MSCANRFLG_RXFIE);
    }
    else
    {
        FMSTR_CLRBIT(fmstr_canBaseAddr, FMSTR_MSCANRIER_OFFSET, FMSTR_MSCANRFLG_RXFIE);
    }
}

static void _FMSTR_S12zCan_EnableRx(void)
{
    /* nothing to do, msCAN is always ready to receive (ID filtering is not under our control) */
}

static FMSTR_SIZE8 _FMSTR_S12zCan_GetRxFrameLen(void)
{
    /* Return 0 when no frame received */
    FMSTR_SIZE8 size = 0;
    
    /* Test RX Full in RX status register */
    if(FMSTR_TSTBIT(fmstr_canBaseAddr, FMSTR_MSCANRFLG_OFFSET, FMSTR_MSCANRFLG_RXF))
    {
        FMSTR_BOOL received = FMSTR_FALSE;
        
        /* Test received ID */
        if((fmstr_canRxIDR[1]) & FMSTR_MSCANIDR1_IDE)
        {
            /* ext id compare */
            if((fmstr_canRxIDR[0] == FMSTR_GETREG(fmstr_canBaseAddr, FMSTR_MSCANRXFG_OFFSET+FMSTR_MSCMBIDR0)) &&
               (fmstr_canRxIDR[1] == FMSTR_GETREG(fmstr_canBaseAddr, FMSTR_MSCANRXFG_OFFSET+FMSTR_MSCMBIDR1)) &&                    
               (fmstr_canRxIDR[2] == FMSTR_GETREG(fmstr_canBaseAddr, FMSTR_MSCANRXFG_OFFSET+FMSTR_MSCMBIDR2)) &&                    
               (fmstr_canRxIDR[3] == FMSTR_GETREG(fmstr_canBaseAddr, FMSTR_MSCANRXFG_OFFSET+FMSTR_MSCMBIDR3)))
            {
                /* matches! */
                received = FMSTR_TRUE;
            }
        }
        else
        {
            /* std id compare */
            if((fmstr_canRxIDR[0] == FMSTR_GETREG(fmstr_canBaseAddr, FMSTR_MSCANRXFG_OFFSET+FMSTR_MSCMBIDR0)) &&
              ((fmstr_canRxIDR[1] ^ FMSTR_GETREG(fmstr_canBaseAddr, FMSTR_MSCANRXFG_OFFSET+FMSTR_MSCMBIDR1)) & 0xf8U) == 0)
            {
                /* matches! */
                received = FMSTR_TRUE;
            }
        }
        
        if(received)
        {
            /* Matching frame was received, read length */
            size = FMSTR_GETREG(fmstr_canBaseAddr, FMSTR_MSCANRXFG_OFFSET+FMSTR_MSCMBDLR);
            
            /* Discard frames with 0 length */
            if(!size)
                FMSTR_SETREG(fmstr_canBaseAddr, FMSTR_MSCANRFLG_OFFSET, FMSTR_MSCANRFLG_RXF);
        }
    }
        
    return size;
}

static FMSTR_BCHR _FMSTR_S12zCan_GetRxFrameByte(FMSTR_SIZE8 index)
{
    /* CAN reception, get one received byte */
    return FMSTR_GETREG(fmstr_canBaseAddr, FMSTR_MSCANRXFG_OFFSET+FMSTR_MSCMBDSR0+index);
}

static void _FMSTR_S12zCan_AckRxFrame(void)
{
    /* Clear RX Full flag */
    FMSTR_SETREG(fmstr_canBaseAddr, FMSTR_MSCANRFLG_OFFSET, FMSTR_MSCANRFLG_RXF);
    
    /* Reset the priority counter for the next transmission sequence */
    fmstr_canTxPrio = 0;
}

static FMSTR_BOOL _FMSTR_S12zCan_PrepareTxFrame(void)
{
    /* read TX status register to see if any tx buffer is free */
    FMSTR_U8 tbsel = FMSTR_GETREG(fmstr_canBaseAddr, FMSTR_MSCANTFLG_OFFSET) & FMSTR_MSCANTBUF_USED;
    
    if(!tbsel)
    {
        /* no free Tx buffer found */
        fmstr_canTxBufSel = 0;
        return FMSTR_FALSE; 
    }

    /* select one of the free buffers */
    FMSTR_SETREG(fmstr_canBaseAddr, FMSTR_MSCANTBSEL_OFFSET, tbsel); \

    /* which one was finally selected? */
    fmstr_canTxBufSel = FMSTR_GETREG(fmstr_canBaseAddr, FMSTR_MSCANTBSEL_OFFSET);
    
    /* final sanity check */
    return (FMSTR_BOOL)(fmstr_canTxBufSel ? FMSTR_TRUE : FMSTR_FALSE);
}

static void _FMSTR_S12zCan_PutTxFrameByte(FMSTR_SIZE8 index, FMSTR_BCHR data)
{
    /* put one data byte for transmission */
    FMSTR_SETREG(fmstr_canBaseAddr, FMSTR_MSCANTXFG_OFFSET+FMSTR_MSCMBDSR0+index, data);
}

static void _FMSTR_S12zCan_SendTxFrame(FMSTR_SIZE8 len)
{
    /* write transmission IDR  */
    FMSTR_SETREG(fmstr_canBaseAddr, FMSTR_MSCANTXFG_OFFSET+FMSTR_MSCMBIDR0, fmstr_canTxIDR[0]);
    FMSTR_SETREG(fmstr_canBaseAddr, FMSTR_MSCANTXFG_OFFSET+FMSTR_MSCMBIDR1, fmstr_canTxIDR[1]);
    FMSTR_SETREG(fmstr_canBaseAddr, FMSTR_MSCANTXFG_OFFSET+FMSTR_MSCMBIDR2, fmstr_canTxIDR[2]);
    FMSTR_SETREG(fmstr_canBaseAddr, FMSTR_MSCANTXFG_OFFSET+FMSTR_MSCMBIDR3, fmstr_canTxIDR[3]);

    /* frame length */
    FMSTR_SETREG(fmstr_canBaseAddr, FMSTR_MSCANTXFG_OFFSET+FMSTR_MSCMBDLR, len);

    /* set transmit priority */
    FMSTR_SETREG(fmstr_canBaseAddr, FMSTR_MSCANTXFG_OFFSET+FMSTR_MSCMBTBPR, fmstr_canTxPrio);
    fmstr_canTxPrio++;

    /* do send */
    FMSTR_SETREG(fmstr_canBaseAddr, FMSTR_MSCANTFLG_OFFSET, fmstr_canTxBufSel);
}

/******************************************************************************
*
* @brief    Assigning FreeMASTER communication module base address
*
******************************************************************************/

void FMSTR_CanSetBaseAddress(FMSTR_ADDR base)
{
    fmstr_canBaseAddr = base;
}

/******************************************************************************
*
* @brief    Process FreeMASTER CAN interrupt (call this function from msCAN ISR)
*
******************************************************************************/

void FMSTR_CanIsr(void)
{
#if FMSTR_LONG_INTR || FMSTR_SHORT_INTR
    if(FMSTR_TSTBIT(fmstr_canBaseAddr, FMSTR_MSCANRFLG_OFFSET, FMSTR_MSCANRFLG_RXF))
    {
        /* Process received frame */
        FMSTR_ProcessCanRx();
    }

    if(FMSTR_TSTBIT(fmstr_canBaseAddr, FMSTR_MSCANTFLG_OFFSET, FMSTR_MSCANTBUF_USED))
    {
        /* Send next frame, if needed */
        FMSTR_ProcessCanTx();
    }
#endif
}

#else /* (!(FMSTR_DISABLE)) */

/* Empty API functions when FMSTR_DISABLE is set */
void FMSTR_CanSetBaseAddress(CAN_Type *base)
{
    FMSTR_UNUSED(base);
}

void FMSTR_CanIsr(void)
{
}

#endif /* !(FMSTR_DISABLE) */ 
#endif /* defined(FMSTR_CAN_DRV) && (FMSTR_MK_IDSTR(FMSTR_CAN_DRV) == FMSTR_CAN_MCUX_MSCAN_ID) */
