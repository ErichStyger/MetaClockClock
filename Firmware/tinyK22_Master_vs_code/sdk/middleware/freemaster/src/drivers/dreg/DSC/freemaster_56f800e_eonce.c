/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2020-2021, 2024 NXP
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
 * FreeMASTER Communication Driver - DSC JTAG EOnCE RTD Unit Communication Interface
 */
#include "freemaster.h"
#include "freemaster_private.h"

/* Compile this code only if the SERIAL driver is selected in freemaster_cfg.h. */
#if defined(FMSTR_SERIAL_DRV) && (FMSTR_MK_IDSTR(FMSTR_SERIAL_DRV) == FMSTR_SERIAL_56F800E_EONCE_ID)
#include "freemaster_serial.h"
#include "freemaster_56f800e_eonce.h"

#if !(FMSTR_DISABLE)

/***********************************
*  configuration check
***********************************/

#if FMSTR_USE_EONCE_TDF_WORKAROUND && (!FMSTR_POLL_DRIVEN)
#error FreeMASTER EOnCE communication with active TDF big workaround only works in polled mode.
#endif

/***********************************
*  local variables
***********************************/

/* EOnCE bas address is the same for all DSC platforms */
#define FMSTR_EONCE_BASE 0xFFFF00

/* EOnCE base address kept as U32 word, but still it needs the LDM to work. */
#if defined(FMSTR_SERIAL_BASE) && ((FMSTR_SERIAL_BASE) != FMSTR_EONCE_BASE)
    #warning FMSTR_SERIAL_BASE should not be specified when using JTAG EOnCE RTD Unit (undefine to use default 0xFFFF00)
    static FMSTR_U32 fmstr_eonceBaseAddr = (FMSTR_U32)FMSTR_SERIAL_BASE;
#else
    static FMSTR_U32 fmstr_eonceBaseAddr = 0;
#endif

struct
{
    FMSTR_U32 txData;    /* Cached 4 bytes of transmit data */
    FMSTR_U32 rxData;    /* Cached 4 bytes of receive data */
    FMSTR_SIZE8 txSize;  /* Number of bytes in transmit data cache */
    FMSTR_SIZE8 rxSize;  /* Number of bytes in receive data cache */

    union
    {
        FMSTR_FLAGS all;
        struct
        {
            unsigned riePending : 1;   /* RX interrupt is pending to be enabled */
            unsigned tdfNeedRead : 1;  /* used with FMSTR_USE_EONCE_TDF_WORKAROUND logic */
        } bit;
    } flags;

} fmstr_eonceCtx;

/***********************************
*  local function prototypes
***********************************/

/* Interface function - Initialization of EONCE driver adapter */
static FMSTR_BOOL _FMSTR_56F800E_EOnCE_Init(void);
static void _FMSTR_56F800E_EOnCE_EnableTransmit(FMSTR_BOOL enable);
static void _FMSTR_56F800E_EOnCE_EnableReceive(FMSTR_BOOL enable);
static void _FMSTR_56F800E_EOnCE_EnableTransmitInterrupt(FMSTR_BOOL enable);
static void _FMSTR_56F800E_EOnCE_EnableReceiveInterrupt(FMSTR_BOOL enable);
static void _FMSTR_56F800E_EOnCE_EnableTransmitCompleteInterrupt(FMSTR_BOOL enable);
static FMSTR_BOOL _FMSTR_56F800E_EOnCE_IsTransmitRegEmpty(void);
static FMSTR_BOOL _FMSTR_56F800E_EOnCE_IsReceiveRegFull(void);
static FMSTR_BOOL _FMSTR_56F800E_EOnCE_IsTransmitterActive(void);
static void _FMSTR_56F800E_EOnCE_PutChar(FMSTR_BCHR  ch);
static FMSTR_BCHR _FMSTR_56F800E_EOnCE_GetChar(void);
static void _FMSTR_56F800E_EOnCE_Flush(void);
static void _FMSTR_56F800E_EOnCE_Poll(void);

static FMSTR_BOOL _FMSTR_56F800E_EOnCE_ReadTDF(void);
static void _FMSTR_56F800E_EOnCE_TryReceiveData(void);
static void _FMSTR_56F800E_EOnCE_TryTransmitData(void);

/***********************************
*  global variables
***********************************/
/* Interface of this EONCE driver */

const FMSTR_SERIAL_DRV_INTF FMSTR_SERIAL_56F800E_EONCE =
{
    FMSTR_C99_INIT(Init                       ) _FMSTR_56F800E_EOnCE_Init,
    FMSTR_C99_INIT(EnableTransmit             ) _FMSTR_56F800E_EOnCE_EnableTransmit,
    FMSTR_C99_INIT(EnableReceive              ) _FMSTR_56F800E_EOnCE_EnableReceive,
    FMSTR_C99_INIT(EnableTransmitInterrupt    ) _FMSTR_56F800E_EOnCE_EnableTransmitInterrupt,
    FMSTR_C99_INIT(EnableTransmitCompleteInterrupt ) _FMSTR_56F800E_EOnCE_EnableTransmitCompleteInterrupt,
    FMSTR_C99_INIT(EnableReceiveInterrupt     ) _FMSTR_56F800E_EOnCE_EnableReceiveInterrupt,
    FMSTR_C99_INIT(IsTransmitRegEmpty         ) _FMSTR_56F800E_EOnCE_IsTransmitRegEmpty,
    FMSTR_C99_INIT(IsReceiveRegFull           ) _FMSTR_56F800E_EOnCE_IsReceiveRegFull,
    FMSTR_C99_INIT(IsTransmitterActive        ) _FMSTR_56F800E_EOnCE_IsTransmitterActive,
    FMSTR_C99_INIT(PutChar                    ) _FMSTR_56F800E_EOnCE_PutChar,
    FMSTR_C99_INIT(GetChar                    ) _FMSTR_56F800E_EOnCE_GetChar,
    FMSTR_C99_INIT(Flush                      ) _FMSTR_56F800E_EOnCE_Flush,
    FMSTR_C99_INIT(Poll                       ) _FMSTR_56F800E_EOnCE_Poll,
};

/****************************************************************************************
* General peripheral space access macros
*****************************************************************************************/

/* When enabled, assembler macros will be used to access the EOnCE registers. This code
 * is able to access the registers at 'far' address at base 0xffff00 even in SDM model. */
#if 1

#define FMSTR_SETBIT(baseAddr, regOffset, bitField) do { \
        register FMSTR_U16* rX; \
        asm ( move.l baseAddr, rX); \
        asm ( bfset bitField, X:(rX+regOffset) ); \
    } while(0)

#define FMSTR_CLRBIT(baseAddr, regOffset, bitField) do { \
        register FMSTR_U16* rX; \
        asm ( move.l baseAddr, rX); \
        asm ( bfclr bitField, X:(rX+regOffset) ); \
    } while(0)

#define FMSTR_TSTBIT(baseAddr, regOffset, bitField) \
    (FMSTR_GETREG((baseAddr), (regOffset)) & (bitField))

static inline void FMSTR_SETREG(register FMSTR_U32 baseAddr, register FMSTR_U32 regOffset, register FMSTR_U16 newValue)
{
    register FMSTR_U16* rX;
    baseAddr += regOffset;
    asm ( move.l baseAddr, rX);
    asm ( move.w newValue, X:(rX+0) );
}

static inline FMSTR_U16 FMSTR_GETREG(register FMSTR_U32 baseAddr, register FMSTR_U32 regOffset)
{
    register FMSTR_U16* rX;
    register FMSTR_U16 val;
    baseAddr += regOffset;
    asm ( move.l baseAddr, rX);
    asm ( move.w X:(rX+0), val );
    return val;
}

static inline void FMSTR_SETREG32(register FMSTR_U32 baseAddr, register FMSTR_U32 regOffset, register FMSTR_U32 newValue)
{
    register FMSTR_U32* rX;
    baseAddr += regOffset;
    asm ( move.l baseAddr, rX);
    asm ( move.l newValue, X:(rX+0) );
}

static inline FMSTR_U32 FMSTR_GETREG32(register FMSTR_U32 baseAddr, register FMSTR_U32 regOffset)
{
    register FMSTR_U32* rX;
    register FMSTR_U32 val;
    baseAddr += regOffset;
    asm ( move.l baseAddr, rX);
    asm ( move.l X:(rX+0), val );
    return val;
}

#else /* When assembler macros are disabled above, this is an equivalent pure C code which will only work in LDM. */ 

#define FMSTR_SETBIT(base, offset, bit)     (*(volatile FMSTR_U16*)(((FMSTR_U32)(base))+(offset)) |= bit)
#define FMSTR_CLRBIT(base, offset, bit)     (*(volatile FMSTR_U16*)(((FMSTR_U32)(base))+(offset)) &= (FMSTR_U16)~((FMSTR_U16)(bit)))
#define FMSTR_TSTBIT(base, offset, bit)     (*(volatile FMSTR_U16*)(((FMSTR_U32)(base))+(offset)) & (bit))
#define FMSTR_SETREG(base, offset, value)   (*(volatile FMSTR_U16*)(((FMSTR_U32)(base))+(offset)) = value)
#define FMSTR_GETREG(base, offset)          (*(volatile FMSTR_U16*)(((FMSTR_U32)(base))+(offset)))
#define FMSTR_SETREG32(base, offset, value) (*(volatile FMSTR_U32*)(((FMSTR_U32)(base))+(offset)) = value)
#define FMSTR_GETREG32(base, offset)        (*(volatile FMSTR_U32*)(((FMSTR_U32)(base))+(offset)))

#endif


/****************************************************************************************
* EONCE module constants
*****************************************************************************************/

/* EONCE module registers */
#define FMSTR_EONCE_OTXRXSR_OFFSET 0xfdU
#define FMSTR_EONCE_ORX_OFFSET     0xfeU
#define FMSTR_EONCE_OTX_OFFSET     0xfeU
#define FMSTR_EONCE_ORX1_OFFSET    0xffU
#define FMSTR_EONCE_OTX1_OFFSET    0xffU

/* OTXRXSR register */
#define FMSTR_EONCE_OTXRXSR_RDF 0x01U
#define FMSTR_EONCE_OTXRXSR_TDF 0x02U
#define FMSTR_EONCE_OTXRXSR_RIE 0x04U
#define FMSTR_EONCE_OTXRXSR_TIE 0x08U

/******************************************************************************
*
* EONCE communication initialization
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_56F800E_EOnCE_Init(void)
{
    /* Default EOnCE address is used */
    if(fmstr_eonceBaseAddr == 0)
        fmstr_eonceBaseAddr = FMSTR_EONCE_BASE;

    fmstr_eonceCtx.flags.all = 0;
    fmstr_eonceCtx.txSize = 0;
    fmstr_eonceCtx.rxSize = 0;
    return FMSTR_TRUE;
}

/******************************************************************************
*
* EONCE polling
*
******************************************************************************/

static void _FMSTR_56F800E_EOnCE_Poll(void)
{
    /* In the JTAG interrupt-driven mode (both short and long), this function periodically checks
     * if setting the JTAG RIE bit failed recently. This may happen because of the
     * RIE is held low by the EONCE hardware until the EONCE is first accessed from host.
     * The FMSTR_Init (->FMSTR_Listen) is often called while the PC-side FreeMASTER is still
     * turned off. So the JTAG is not enabled at this time and RIE bit is not set.
     * This problem is detected (see how riePending is set) and it is tried to be fixed
     * periodically here in the poll call.
     */
#if FMSTR_SHORT_INTR || FMSTR_LONG_INTR
    if(fmstr_eonceCtx.flags.bit.riePending)
    {
#if FMSTR_DEBUG_LEVEL >= 3
        FMSTR_DEBUG_PRINTF("FMSTR EOnCE Late RIE attempt\n");
#endif
        _FMSTR_56F800E_EOnCE_EnableReceiveInterrupt(FMSTR_TRUE);
    }
#endif
}

/******************************************************************************
*
* Enable/Disable EONCE transmitter
*
******************************************************************************/

static void _FMSTR_56F800E_EOnCE_EnableTransmit(FMSTR_BOOL enable)
{
    if(!enable)
    {
        fmstr_eonceCtx.txSize = 0;
#if FMSTR_USE_EONCE_TDF_WORKAROUND
        fmstr_eonceCtx.flags.bit.tdfNeedRead = 0;
#endif
    }
}

/******************************************************************************
*
* Enable/Disable EONCE receiver
*
******************************************************************************/

static void _FMSTR_56F800E_EOnCE_EnableReceive(FMSTR_BOOL enable)
{
    if(!enable)
        fmstr_eonceCtx.rxSize = 0;
}

/******************************************************************************
*
* Enable/Disable interrupt from transmit register empty event
*
******************************************************************************/

static void _FMSTR_56F800E_EOnCE_EnableTransmitInterrupt(FMSTR_BOOL enable)
{
    if(enable)
    {
        /* Enable interrupt */
        FMSTR_SETBIT(fmstr_eonceBaseAddr, FMSTR_EONCE_OTXRXSR_OFFSET, FMSTR_EONCE_OTXRXSR_TIE);
    }
    else
    {
        /* Disable interrupt */
        FMSTR_CLRBIT(fmstr_eonceBaseAddr, FMSTR_EONCE_OTXRXSR_OFFSET, FMSTR_EONCE_OTXRXSR_TIE);
    }
}

/******************************************************************************
*
* Enable/Disable interrupt when transmission is complete (=idle)
*
******************************************************************************/

static void _FMSTR_56F800E_EOnCE_EnableTransmitCompleteInterrupt(FMSTR_BOOL enable)
{
    /* no such interrupt, see how TDF is tested. */
}

/******************************************************************************
*
* Enable/Disable interrupt from receive register full event
*
******************************************************************************/

static void _FMSTR_56F800E_EOnCE_EnableReceiveInterrupt(FMSTR_BOOL enable)
{
    /* this bit will be set eventually if enabling the interrupt fails */
    fmstr_eonceCtx.flags.bit.riePending = 0;

    if(enable)
    {
        /* Enable interrupt */
        FMSTR_SETBIT(fmstr_eonceBaseAddr, FMSTR_EONCE_OTXRXSR_OFFSET, FMSTR_EONCE_OTXRXSR_RIE);

        /* On older core versions, enabling the interrupt may fail if JTAG module is not
         * yet initialized by an external access. See the Poll call for more information. */
        if(!FMSTR_TSTBIT(fmstr_eonceBaseAddr, FMSTR_EONCE_OTXRXSR_OFFSET, FMSTR_EONCE_OTXRXSR_RIE))
            fmstr_eonceCtx.flags.bit.riePending = 1;
    }
    else
    {
        /* Disable interrupt */
        FMSTR_CLRBIT(fmstr_eonceBaseAddr, FMSTR_EONCE_OTXRXSR_OFFSET, FMSTR_EONCE_OTXRXSR_RIE);
    }
}

/******************************************************************************
*
* Returns TRUE if the transmit register is empty, and it's possible to put next char
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_56F800E_EOnCE_IsTransmitRegEmpty(void)
{
    /* Any space in transmit buffer? */
    if(fmstr_eonceCtx.txSize < 4)
        return FMSTR_TRUE;

    /* Try to flush 4 bytes which are pending in our output buffer now */
    _FMSTR_56F800E_EOnCE_TryTransmitData();

    /* Any free space by now? */
    return fmstr_eonceCtx.txSize < 4;
}

/******************************************************************************
*
* Try to receive some EOnCE data from the remote peer into the local buffer
*
******************************************************************************/

static void _FMSTR_56F800E_EOnCE_TryReceiveData(void)
{
    if(FMSTR_TSTBIT(fmstr_eonceBaseAddr, FMSTR_EONCE_OTXRXSR_OFFSET, FMSTR_EONCE_OTXRXSR_RDF))
    {
        fmstr_eonceCtx.rxData = FMSTR_GETREG32(fmstr_eonceBaseAddr, FMSTR_EONCE_ORX_OFFSET);
        fmstr_eonceCtx.rxSize = 4;

#if FMSTR_DEBUG_LEVEL >= 3
        FMSTR_DEBUG_PRINTF("FMSTR EOnCE RX %04lx\n", fmstr_eonceCtx.rxData);
#endif
    }
}

/******************************************************************************
*
* Returns TRUE if the receive register is full, and it's possible to get received char
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_56F800E_EOnCE_IsReceiveRegFull(void)
{
    /* Still some characters in the receive buffer? */
    if(fmstr_eonceCtx.rxSize > 0)
        return FMSTR_TRUE;

    /* Try to get next 4 bytes */
    _FMSTR_56F800E_EOnCE_TryReceiveData();

    /* Any data by now? */
    return fmstr_eonceCtx.rxSize > 0;
}

/******************************************************************************
*
* Low-level testing of TDF flag
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_56F800E_EOnCE_ReadTDF(void)
{
    FMSTR_U16 statusReg = FMSTR_GETREG(fmstr_eonceBaseAddr, FMSTR_EONCE_OTXRXSR_OFFSET);

    /* The TDF=0 indicates the transmitter has completed the physical transmission. */
    if(!(statusReg & FMSTR_EONCE_OTXRXSR_TDF))
    {
#if FMSTR_USE_EONCE_TDF_WORKAROUND
        if(fmstr_eonceCtx.flags.bit.tdfNeedRead)
        {
            /* Silicon bug in older DSC parts makes TDF bit unusable (TDF gets reset too early).
               The FreeMASTER tool sends a dummy confirmation word whenever it receives
               word transmitted by us. This means we can use the RDF as an indication of !TDF */
            if(statusReg & FMSTR_EONCE_OTXRXSR_RDF)
            {
                /* Read and discard the data. */
                FMSTR_U32 dummyRx = FMSTR_GETREG32(fmstr_eonceBaseAddr, FMSTR_EONCE_ORX_OFFSET);
#if FMSTR_DEBUG_LEVEL >= 3
                FMSTR_DEBUG_PRINTF("FMSTR EOnCE Dummy RX for !TDF %04lx\n", dummyRx);
#endif
                fmstr_eonceCtx.flags.bit.tdfNeedRead = 0;
                return FMSTR_FALSE;
            }
            else
            {
                return FMSTR_TRUE;  /* Not received anything, transmitter is still active. */
            }
        }
        else
        {
            /* We only check the RDF once per each transmitted word */
            return FMSTR_FALSE;
        }
#else
        /* Modern DSC parts should have the bug fixed, so we can rely on the TDF bit. */
        return FMSTR_FALSE;
#endif
    }

    /* Transmitter is still active. */
    return FMSTR_TRUE;
}

/******************************************************************************
*
* Returns TRUE if the transmitter is still active
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_56F800E_EOnCE_IsTransmitterActive(void)
{
    /* this call should only be used after flush */
    FMSTR_ASSERT(fmstr_eonceCtx.txSize == 0 || fmstr_eonceCtx.txSize == 4);

    /* Try physical flushing if word is still pending */
    if(fmstr_eonceCtx.txSize == 4)
        _FMSTR_56F800E_EOnCE_TryTransmitData();

    return fmstr_eonceCtx.txSize > 0 || _FMSTR_56F800E_EOnCE_ReadTDF();
}

/******************************************************************************
*
* The function puts the char for transmit
*
******************************************************************************/

static void _FMSTR_56F800E_EOnCE_PutChar(FMSTR_BCHR ch)
{
    /* There should be space */
    if(fmstr_eonceCtx.txSize < 4)
    {
        fmstr_eonceCtx.txData = (fmstr_eonceCtx.txData << 8) | (ch & 0xff);
        fmstr_eonceCtx.txSize++;

        if(fmstr_eonceCtx.txSize == 4)
            _FMSTR_56F800E_EOnCE_TryTransmitData();
    }
    else
    {
        /* Caller hasn't checked if IsTransmitRegEmpty */
        FMSTR_ASSERT(0);
    }
}

/******************************************************************************
*
* The function gets the received char
*
******************************************************************************/
static FMSTR_BCHR _FMSTR_56F800E_EOnCE_GetChar(void)
{
    FMSTR_BCHR ch = 0;

    if(fmstr_eonceCtx.rxSize > 0)
    {
        ch = (FMSTR_BCHR)((fmstr_eonceCtx.rxData >> 24) & 0xff);
        fmstr_eonceCtx.rxData <<= 8;
        fmstr_eonceCtx.rxSize--;
    }
    else
    {
        /* Caller hasn't checked if IsReceiveRegFull */
        FMSTR_ASSERT(0);
    }

    return ch;
}

/******************************************************************************
*
* Sends buffered 32bit word
*
******************************************************************************/

static void _FMSTR_56F800E_EOnCE_TryTransmitData(void)
{
    /* This should only be used when full word is ready */
    FMSTR_ASSERT(fmstr_eonceCtx.txSize == 4);

    /* Is it possible to transmit physically now? */
    if(!_FMSTR_56F800E_EOnCE_ReadTDF())
    {
#if FMSTR_DEBUG_LEVEL >= 3
        FMSTR_DEBUG_PRINTF("FMSTR EOnCE TX %04lx\n", fmstr_eonceCtx.txData);
#endif
        /* OK put to physical transmit buffer */
        FMSTR_SETREG32(fmstr_eonceBaseAddr, FMSTR_EONCE_OTX_OFFSET, fmstr_eonceCtx.txData);
        fmstr_eonceCtx.txSize = 0;

        /* To fix the TDF behavior, we will perform dummy reading. */
#if FMSTR_USE_EONCE_TDF_WORKAROUND
        fmstr_eonceCtx.flags.bit.tdfNeedRead = 1;
#endif
    }
}

/******************************************************************************
*
* Send buffered data
*
******************************************************************************/

static void _FMSTR_56F800E_EOnCE_Flush(void)
{
    /* If anything pending to be transmitted. */
    if(fmstr_eonceCtx.txSize)
    {
        /* Send dummy 0xff bytes to wrap up to 4 bytes */
        if(fmstr_eonceCtx.txSize < 4)
        {
            FMSTR_SIZE8 shift = 8*(4-fmstr_eonceCtx.txSize);
            fmstr_eonceCtx.txData = (fmstr_eonceCtx.txData << shift) | ((1<<shift)-1);
            fmstr_eonceCtx.txSize = 4;
        }

        /* Physical flush if possible. */
        _FMSTR_56F800E_EOnCE_TryTransmitData();
    }
}

/******************************************************************************
*
* Assign FreeMASTER communication module base address
*
******************************************************************************/

void FMSTR_SerialSetBaseAddress(FMSTR_U32 base)
{
    fmstr_eonceBaseAddr = base;
}

/******************************************************************************
*
* Process FreeMASTER serial interrupt (call this function from EONCE ISR)
*
******************************************************************************/

void FMSTR_SerialIsr(void)
{
    /* process incoming or just transmitted byte */
    #if (FMSTR_LONG_INTR) || (FMSTR_SHORT_INTR)
    FMSTR_ProcessSerial();
    #endif
}

#else /* !(FMSTR_DISABLE) */

/* Empty API functions when FMSTR_DISABLE is set */
void FMSTR_SerialSetBaseAddress(FMSTR_U32 base)
{
    FMSTR_UNUSED(base);
}

void FMSTR_SerialIsr(void)
{
}

#endif /* !(FMSTR_DISABLE) */
#endif /* defined(FMSTR_SERIAL_DRV) && (FMSTR_MK_IDSTR(FMSTR_SERIAL_DRV) == FMSTR_SERIAL_56F800E_EONCE_ID) */
