/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2019-2020, 2024 NXP
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
#if defined(FMSTR_CAN_DRV) && (FMSTR_MK_IDSTR(FMSTR_CAN_DRV) == FMSTR_CAN_56F800E_FLEXCAN_ID)
#include "freemaster_can.h"
#include "freemaster_56f800e_flexcan.h"

#if (!(FMSTR_DISABLE))

/******************************************************************************
* Local types
******************************************************************************/

typedef struct _FMSTR_FlexCAN_FRAME
{
    FMSTR_U16 len;
    FMSTR_U32 idr;        /* top bit is set for EXT frame */
    FMSTR_U32 data[2];
} FMSTR_FlexCAN_FRAME;

/******************************************************************************
* Local functions
******************************************************************************/

static FMSTR_BOOL _FMSTR_56F800E_FlexCAN_Init(FMSTR_U32 idRx, FMSTR_U32 idTx);  /* Initialize CAN module on a given base address. */
static void _FMSTR_56F800E_FlexCAN_EnableTxInterrupt(FMSTR_BOOL enable);     /* Enable CAN Transmit interrupt. */
static void _FMSTR_56F800E_FlexCAN_EnableRxInterrupt(FMSTR_BOOL enable);     /* Enable CAN Receive interrupt. */
static void _FMSTR_56F800E_FlexCAN_EnableRx(void);                           /* Enable/re-initialize Receiver buffer. */
static FMSTR_SIZE8 _FMSTR_56F800E_FlexCAN_GetRxFrameLen(void);               /* Return size of received CAN frame, or 0 if no Rx frame is available. */
static FMSTR_BCHR _FMSTR_56F800E_FlexCAN_GetRxFrameByte(FMSTR_SIZE8 index);  /* Get data byte at index (0..8). */
static void _FMSTR_56F800E_FlexCAN_AckRxFrame(void);                         /* Discard received frame and enable receiving a next one. */
static FMSTR_BOOL _FMSTR_56F800E_FlexCAN_PrepareTxFrame(void);               /* Initialize transmit buffer; return false when Tx buffer is not available. */
static void _FMSTR_56F800E_FlexCAN_PutTxFrameByte(FMSTR_SIZE8 index, FMSTR_BCHR data);   /* Fill one byte of transmit data. */
static void _FMSTR_56F800E_FlexCAN_SendTxFrame(FMSTR_SIZE8 len);             /* Send the Tx buffer. */

/******************************************************************************
* Local variables
******************************************************************************/

/* FlexCAN base address as word pointer on DSC platform */
#ifdef FMSTR_CAN_BASE
    #define _FMSTR_CAN_BASE FMSTR_CAN_BASE
#else
    static FMSTR_U16 fmstr_canBaseAddr = 0;
    #define _FMSTR_CAN_BASE fmstr_canBaseAddr
#endif

static FMSTR_U32 _fmstr_canIdrRx;
static FMSTR_U32 _fmstr_canIdrTx;

static FMSTR_FlexCAN_FRAME _fmstr_canRxFrame;
static FMSTR_FlexCAN_FRAME _fmstr_canTxFrame;

/******************************************************************************
* Driver interface
******************************************************************************/

const FMSTR_CAN_DRV_INTF FMSTR_CAN_56F800E_FLEXCAN =
{
    FMSTR_C99_INIT(Init              ) _FMSTR_56F800E_FlexCAN_Init,
    FMSTR_C99_INIT(EnableTxInterrupt ) _FMSTR_56F800E_FlexCAN_EnableTxInterrupt,
    FMSTR_C99_INIT(EnableRxInterrupt ) _FMSTR_56F800E_FlexCAN_EnableRxInterrupt,
    FMSTR_C99_INIT(EnableRx          ) _FMSTR_56F800E_FlexCAN_EnableRx,
    FMSTR_C99_INIT(GetRxFrameLen     ) _FMSTR_56F800E_FlexCAN_GetRxFrameLen,
    FMSTR_C99_INIT(GetRxFrameByte    ) _FMSTR_56F800E_FlexCAN_GetRxFrameByte,
    FMSTR_C99_INIT(AckRxFrame        ) _FMSTR_56F800E_FlexCAN_AckRxFrame,
    FMSTR_C99_INIT(PrepareTxFrame    ) _FMSTR_56F800E_FlexCAN_PrepareTxFrame,
    FMSTR_C99_INIT(PutTxFrameByte    ) _FMSTR_56F800E_FlexCAN_PutTxFrameByte,
    FMSTR_C99_INIT(SendTxFrame       ) _FMSTR_56F800E_FlexCAN_SendTxFrame,
};

/****************************************************************************************
* General peripheral space access macros
*****************************************************************************************/

/* FCAN module registers offsets */
#define FMSTR_FCANTMR_OFFSET    (0x04)
#define FMSTR_FCANESR1_OFFSET   (0x10)
#define FMSTR_FCANIMASK_OFFSET  (0x14)
#define FMSTR_FCANIFLAG_OFFSET  (0x18)
#define FMSTR_FCANRXFG_OFFSET   (0x40 + ((FMSTR_FLEXCAN_RXMB) * 0x08)) //*0x08 because this is WORD pointer
#define FMSTR_FCANTXFG_OFFSET   (0x40 + ((FMSTR_FLEXCAN_TXMB) * 0x08))

/* FCAN MB registers offsets (must also add FCANxxFG_OFFSET) */
#define FMSTR_FCMBCSR   0x00
#define FMSTR_FCMBIDR   0x02
#define FMSTR_FCMBDATA0 0x04
#define FMSTR_FCMBDATA1 0x06

/* FCAN32 module MB CODEs */
#define FMSTR_FCMBCSR_CODE_SHIFT    24
#define FMSTR_FCANMB_CODE_MASK      (0xfUL << FMSTR_FCMBCSR_CODE_SHIFT)

#define FMSTR_FCANMB_BUSY           (0x1UL << FMSTR_FCMBCSR_CODE_SHIFT)
#define FMSTR_FCANMB_CRXVOID        (0x0UL << FMSTR_FCMBCSR_CODE_SHIFT)   /* buffer void after received data read-out */
#define FMSTR_FCANMB_CRXEMPTY       (0x4UL << FMSTR_FCMBCSR_CODE_SHIFT)   /* active and empty */
#define FMSTR_FCANMB_CRXFULL        (0x2UL << FMSTR_FCMBCSR_CODE_SHIFT)
#define FMSTR_FCANMB_CRXOVERRUN     (0x6UL << FMSTR_FCMBCSR_CODE_SHIFT)

#define FMSTR_FCANMB_CTXTRANS_ONCE  (0xcUL << FMSTR_FCMBCSR_CODE_SHIFT)   /* Initialise transmitting data from buffer */
#define FMSTR_FCANMB_CTXEMPTY       (0x8UL << FMSTR_FCMBCSR_CODE_SHIFT)    /* Message buffer not ready for transmit */

#define FMSTR_FCMBCSR_IDE           (1UL<<21)
#define FMSTR_FCMBCSR_SRR           (1UL<<22)
#define FMSTR_FCMBCSR_DLC_SHIFT     16
#define FMSTR_FCMBCSR_DLC_MASK      (0xfUL<<FMSTR_FCMBCSR_DLC_SHIFT)

#define FMSTR_FCMBIDR_IDR_MASK      0x1fffffffUL

/* FCAN IDR flags */
#define FMSTR_FCANIDR_EXT_FLG       0x80000000UL

/* FCAN: id to idr translation */
#define FMSTR_FCAN_MAKEIDR(id) ((FMSTR_U32)( ((id)&FMSTR_CAN_EXTID) ? ((id)&0x1fffffff) | FMSTR_FCANIDR_EXT_FLG : (((id)<<18)&(0x7ffUL<<18)) ))

#define FMSTR_FLEXCAN_RXMB_FLAG (1UL << (FMSTR_FLEXCAN_RXMB))
#define FMSTR_FLEXCAN_TXMB_FLAG (1UL << (FMSTR_FLEXCAN_TXMB))

/* Register access inline functions */
#define _FMSTR_DECL_ADDR(addr, off) \
    FMSTR_U16 _tmpa = ((FMSTR_U16)_FMSTR_CAN_BASE) + (off); \
    volatile FMSTR_U32* addr = (FMSTR_U32*)_tmpa;

static inline void FMSTR_SETBIT32(FMSTR_U16 offset, FMSTR_U32 bit)
{
    _FMSTR_DECL_ADDR(addr, offset);
    *addr |= bit;
}

static inline void FMSTR_CLRBIT32(FMSTR_U16 offset, FMSTR_U32 bit)
{
    _FMSTR_DECL_ADDR(addr, offset);
    *addr &= (FMSTR_U32)~bit;
}

static inline FMSTR_U32 FMSTR_TSTBIT32(FMSTR_U16 offset, FMSTR_U32 bit)
{
    _FMSTR_DECL_ADDR(addr, offset);
    return (*addr) & bit;
}

static inline void FMSTR_SETREG32(FMSTR_U16 offset, FMSTR_U32 value)
{
    _FMSTR_DECL_ADDR(addr, offset);
    *addr = value;
}

static inline FMSTR_U32 FMSTR_GETREG32(FMSTR_U16 offset)
{
    _FMSTR_DECL_ADDR(addr, offset);
    return *addr;
}

/******************************************************************************
*
* @brief    FlexCAN communication initialization
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_56F800E_FlexCAN_Init(FMSTR_U32 idRx, FMSTR_U32 idTx)
{
    _fmstr_canIdrRx = FMSTR_FCAN_MAKEIDR(idRx);
    _fmstr_canIdrTx = FMSTR_FCAN_MAKEIDR(idTx);
    _fmstr_canRxFrame.len = 0;
    _fmstr_canTxFrame.len = 0;

    /* initialize RX buffer */
    FMSTR_SETREG32(FMSTR_FCANRXFG_OFFSET + FMSTR_FCMBCSR, FMSTR_FCANMB_CRXVOID);

    /* initialize TX buffer if separate one is used */
    if(FMSTR_FLEXCAN_RXMB != FMSTR_FLEXCAN_TXMB)
        FMSTR_SETREG32(FMSTR_FCANTXFG_OFFSET + FMSTR_FCMBCSR, FMSTR_FCANMB_CTXEMPTY);

    return FMSTR_TRUE;
}

static void _FMSTR_56F800E_FlexCAN_EnableTxInterrupt(FMSTR_BOOL enable)
{
    if(enable)
        FMSTR_SETBIT32(FMSTR_FCANIMASK_OFFSET, FMSTR_FLEXCAN_TXMB_FLAG);
    else
        FMSTR_CLRBIT32(FMSTR_FCANIMASK_OFFSET, FMSTR_FLEXCAN_TXMB_FLAG);
}

static void _FMSTR_56F800E_FlexCAN_EnableRxInterrupt(FMSTR_BOOL enable)
{
    if(enable)
        FMSTR_SETBIT32(FMSTR_FCANIMASK_OFFSET, FMSTR_FLEXCAN_RXMB_FLAG);
    else
        FMSTR_CLRBIT32(FMSTR_FCANIMASK_OFFSET, FMSTR_FLEXCAN_RXMB_FLAG);
}

static void _FMSTR_56F800E_FlexCAN_EnableRx(void)
{
    FMSTR_SETREG32(FMSTR_FCANRXFG_OFFSET + FMSTR_FCMBIDR, _fmstr_canIdrRx & FMSTR_FCMBIDR_IDR_MASK);
    FMSTR_SETREG32(FMSTR_FCANRXFG_OFFSET + FMSTR_FCMBCSR, FMSTR_FCANMB_CRXEMPTY |
            ((_fmstr_canIdrRx & FMSTR_FCANIDR_EXT_FLG) ? FMSTR_FCMBCSR_IDE : 0));
}

static FMSTR_SIZE8 _FMSTR_56F800E_FlexCAN_GetRxFrameLen(void)
{
    if(_fmstr_canRxFrame.len == 0)
    {
        FMSTR_U32 sr;
        FMSTR_U32 code;

#if FMSTR_POLL_DRIVEN > 0
        /* Any data received? */
        FMSTR_U32 iflag = FMSTR_GETREG32(FMSTR_FCANIFLAG_OFFSET);

        if((iflag & FMSTR_FLEXCAN_RXMB_FLAG) == 0)
            return 0;

        FMSTR_SETREG32(FMSTR_FCANIFLAG_OFFSET, FMSTR_FLEXCAN_RXMB_FLAG);
#endif /* FMSTR_POLL_DRIVEN */

        do
        {
            sr = FMSTR_GETREG32(FMSTR_FCANRXFG_OFFSET + FMSTR_FCMBCSR);
        } while(sr & FMSTR_FCANMB_BUSY);

        code = sr & FMSTR_FCANMB_CODE_MASK;

        if(code == FMSTR_FCANMB_CRXOVERRUN)
            _fmstr_canRxFrame.len = 0;

        if(code == FMSTR_FCANMB_CRXFULL || code == FMSTR_FCANMB_CRXOVERRUN)
        {
            _fmstr_canRxFrame.len = (FMSTR_U16)((sr & FMSTR_FCMBCSR_DLC_MASK) >> FMSTR_FCMBCSR_DLC_SHIFT);
            _fmstr_canRxFrame.idr = FMSTR_GETREG32(FMSTR_FCANRXFG_OFFSET + FMSTR_FCMBIDR);
            _fmstr_canRxFrame.data[0] = FMSTR_GETREG32(FMSTR_FCANRXFG_OFFSET + FMSTR_FCMBDATA0);
            _fmstr_canRxFrame.data[1] = FMSTR_GETREG32(FMSTR_FCANRXFG_OFFSET + FMSTR_FCMBDATA1);

            if(sr & FMSTR_FCMBCSR_IDE)
                _fmstr_canRxFrame.idr |= FMSTR_FCANIDR_EXT_FLG;

            /* discard frames that are not matching our required ID */
            if(_fmstr_canRxFrame.idr != _fmstr_canIdrRx)
                _fmstr_canRxFrame.len = 0;
        }

        /* dummy read of timer register to unlock MB*/
        FMSTR_GETREG32(FMSTR_FCANTMR_OFFSET);
    }

    return _fmstr_canRxFrame.len;
}

static FMSTR_BCHR _FMSTR_56F800E_FlexCAN_GetRxFrameByte(FMSTR_SIZE8 index)
{
    FMSTR_INDEX i = (index>>2)&1;
    FMSTR_U32 d32 = _fmstr_canRxFrame.data[i];
    d32 >>= 8*(3-(index&3));
    return (FMSTR_BCHR)(d32 & 0xff);
}

static void _FMSTR_56F800E_FlexCAN_AckRxFrame(void)
{
    _fmstr_canRxFrame.len = 0;
}

static FMSTR_BOOL _FMSTR_56F800E_FlexCAN_PrepareTxFrame(void)
{
#if FMSTR_POLL_DRIVEN > 0
    FMSTR_U32 iflag = FMSTR_GETREG32(FMSTR_FCANIFLAG_OFFSET);

    /* All data sent? */
    if(_fmstr_canTxFrame.len && (iflag & FMSTR_FLEXCAN_TXMB_FLAG) == 0)
        return FMSTR_FALSE; /* not yet */

    /* Clear TX status flag */
    FMSTR_SETREG32(FMSTR_FCANIFLAG_OFFSET, FMSTR_FLEXCAN_TXMB_FLAG);

    /* Acknowledge frame was transmitted */
    _fmstr_canTxFrame.len = 0;
#else
    /* transmission in progress */
    if(_fmstr_canTxFrame.len)
        return FMSTR_FALSE;
#endif /* FMSTR_POLL_DRIVEN */

    /* set as transmit-emtpy MB */
    FMSTR_SETREG32(FMSTR_FCANTXFG_OFFSET + FMSTR_FCMBIDR, _fmstr_canIdrTx & FMSTR_FCMBIDR_IDR_MASK);
    FMSTR_SETREG32(FMSTR_FCANTXFG_OFFSET + FMSTR_FCMBCSR, FMSTR_FCANMB_CTXEMPTY);

    return FMSTR_TRUE;
}

static void _FMSTR_56F800E_FlexCAN_PutTxFrameByte(FMSTR_SIZE8 index, FMSTR_BCHR data)
{
    FMSTR_INDEX i = (index>>2)&1;
    FMSTR_U32 d32 = _fmstr_canTxFrame.data[i];
    FMSTR_SIZE shift = 8*(3-(index&3));

    d32 &= ~(0xffUL << shift);
    d32 |= ((FMSTR_U32)data) << shift;

    _fmstr_canTxFrame.data[i] = d32;
}

static void _FMSTR_56F800E_FlexCAN_SendTxFrame(FMSTR_SIZE8 len)
{
    FMSTR_U32 cr = ((((FMSTR_U32)len) << FMSTR_FCMBCSR_DLC_SHIFT) & FMSTR_FCMBCSR_DLC_MASK);
    cr |= (_fmstr_canIdrTx & FMSTR_FCANIDR_EXT_FLG) ? (FMSTR_FCMBCSR_IDE | FMSTR_FCMBCSR_SRR) : 0;

    _fmstr_canTxFrame.len = len;

    FMSTR_SETREG32(FMSTR_FCANTXFG_OFFSET+FMSTR_FCMBDATA0, _fmstr_canTxFrame.data[0]);
    FMSTR_SETREG32(FMSTR_FCANTXFG_OFFSET+FMSTR_FCMBDATA1, _fmstr_canTxFrame.data[1]);
    FMSTR_SETREG32(FMSTR_FCANTXFG_OFFSET + FMSTR_FCMBCSR, FMSTR_FCANMB_CTXTRANS_ONCE | cr);
}

/******************************************************************************
*
* @brief    Assigning FreeMASTER communication module base address
*
******************************************************************************/
void FMSTR_CanSetBaseAddress(FMSTR_U16 base)
{
    fmstr_canBaseAddr = base;
}

void FMSTR_CanIsr(void)
{
#if FMSTR_LONG_INTR || FMSTR_SHORT_INTR
    FMSTR_U32 esr = FMSTR_GETREG32(FMSTR_FCANESR1_OFFSET);
    FMSTR_U32 iflag = FMSTR_GETREG32(FMSTR_FCANIFLAG_OFFSET);

    /* Acknowledge (ignore) error and wake up flags in ESR */
    FMSTR_SETREG32(FMSTR_FCANESR1_OFFSET, esr);

    if(iflag)
    {
        if(iflag & FMSTR_FLEXCAN_RXMB_FLAG)
        {
            FMSTR_ProcessCanRx();
        }

        if(iflag & FMSTR_FLEXCAN_TXMB_FLAG)
        {
            /* Acknowledge frame transmission */
            _fmstr_canTxFrame.len = 0;
            /* Send next frame, if needed */
            FMSTR_ProcessCanTx();
        }

        /* Clear all MB interrupt flags */
        FMSTR_SETREG32(FMSTR_FCANIFLAG_OFFSET, iflag);
    }
#endif
}

#else /* (!(FMSTR_DISABLE)) */

void FMSTR_CanSetBaseAddress(FMSTR_U16 base)
{
    FMSTR_UNUSED(base);
}

void FMSTR_CanIsr(void)
{
}

#endif /* (!(FMSTR_DISABLE)) */
#endif /* defined(FMSTR_CAN_DRV) && (FMSTR_MK_IDSTR(FMSTR_CAN_DRV) == FMSTR_CAN_56F800E_FLEXCAN) */
