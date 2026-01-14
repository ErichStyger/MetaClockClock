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
 * FreeMASTER Communication Driver - Serial Communication Interface
 */
#include "freemaster.h"
#include "freemaster_private.h"

/* Compile this code only if the SERIAL driver is selected in freemaster_cfg.h. */
#if defined(FMSTR_SERIAL_DRV) && (FMSTR_MK_IDSTR(FMSTR_SERIAL_DRV) == FMSTR_SERIAL_56F800E_QSCI_ID)
#include "freemaster_serial.h"
#include "freemaster_56f800e_qsci.h"

#if !(FMSTR_DISABLE)

/***********************************
*  local variables
***********************************/

/* SCI base address kept as word pointer on DSC platform */
#ifdef FMSTR_SERIAL_BASE
    static FMSTR_U16 fmstr_sciBaseAddr = (FMSTR_U16)FMSTR_SERIAL_BASE;
#else
    static FMSTR_U16 fmstr_sciBaseAddr = 0;
#endif

/***********************************
*  local function prototypes
***********************************/

/* Interface function - Initialization of SCI driver adapter */
static FMSTR_BOOL _FMSTR_56F800E_Sci_Init(void);
static void _FMSTR_56F800E_Sci_EnableTransmit(FMSTR_BOOL enable);
static void _FMSTR_56F800E_Sci_EnableReceive(FMSTR_BOOL enable);
static void _FMSTR_56F800E_Sci_EnableTransmitInterrupt(FMSTR_BOOL enable);
static void _FMSTR_56F800E_Sci_EnableReceiveInterrupt(FMSTR_BOOL enable);
static void _FMSTR_56F800E_Sci_EnableTransmitCompleteInterrupt(FMSTR_BOOL enable);
static FMSTR_BOOL _FMSTR_56F800E_Sci_IsTransmitRegEmpty(void);
static FMSTR_BOOL _FMSTR_56F800E_Sci_IsReceiveRegFull(void);
static FMSTR_BOOL _FMSTR_56F800E_Sci_IsTransmitterActive(void);
static void _FMSTR_56F800E_Sci_PutChar(FMSTR_BCHR  ch);
static FMSTR_BCHR _FMSTR_56F800E_Sci_GetChar(void);
static void _FMSTR_56F800E_Sci_Flush(void);

/***********************************
*  global variables
***********************************/
/* Interface of this SCI driver */

const FMSTR_SERIAL_DRV_INTF FMSTR_SERIAL_56F800E_QSCI =
{
    FMSTR_C99_INIT(Init                       ) _FMSTR_56F800E_Sci_Init,
    FMSTR_C99_INIT(EnableTransmit             ) _FMSTR_56F800E_Sci_EnableTransmit,
    FMSTR_C99_INIT(EnableReceive              ) _FMSTR_56F800E_Sci_EnableReceive,
    FMSTR_C99_INIT(EnableTransmitInterrupt    ) _FMSTR_56F800E_Sci_EnableTransmitInterrupt,
    FMSTR_C99_INIT(EnableTransmitCompleteInterrupt ) _FMSTR_56F800E_Sci_EnableTransmitCompleteInterrupt,
    FMSTR_C99_INIT(EnableReceiveInterrupt     ) _FMSTR_56F800E_Sci_EnableReceiveInterrupt,
    FMSTR_C99_INIT(IsTransmitRegEmpty         ) _FMSTR_56F800E_Sci_IsTransmitRegEmpty,
    FMSTR_C99_INIT(IsReceiveRegFull           ) _FMSTR_56F800E_Sci_IsReceiveRegFull,
    FMSTR_C99_INIT(IsTransmitterActive        ) _FMSTR_56F800E_Sci_IsTransmitterActive,
    FMSTR_C99_INIT(PutChar                    ) _FMSTR_56F800E_Sci_PutChar,
    FMSTR_C99_INIT(GetChar                    ) _FMSTR_56F800E_Sci_GetChar,
    FMSTR_C99_INIT(Flush                      ) _FMSTR_56F800E_Sci_Flush,
};

/****************************************************************************************
* General peripheral space access macros
*****************************************************************************************/

#define FMSTR_SETBIT(base, offset, bit)     (*(volatile FMSTR_U16*)(((FMSTR_U16)(base))+(offset)) |= bit)
#define FMSTR_CLRBIT(base, offset, bit)     (*(volatile FMSTR_U16*)(((FMSTR_U16)(base))+(offset)) &= (FMSTR_U16)~((FMSTR_U16)(bit)))
#define FMSTR_TSTBIT(base, offset, bit)     (*(volatile FMSTR_U16*)(((FMSTR_U16)(base))+(offset)) & (bit))
#define FMSTR_SETREG(base, offset, value)   (*(volatile FMSTR_U16*)(((FMSTR_U16)(base))+(offset)) = value)
#define FMSTR_GETREG(base, offset)          (*(volatile FMSTR_U16*)(((FMSTR_U16)(base))+(offset)))

/****************************************************************************************
* SCI module constants
*****************************************************************************************/

/* SCI module registers */
#define FMSTR_SCIBR_OFFSET 0U
#define FMSTR_SCICR_OFFSET 1U
#define FMSTR_SCISR_OFFSET 3U
#define FMSTR_SCIDR_OFFSET 4U

/* SCI Control Register bits */
#define FMSTR_SCICR_LOOP      0x8000U
#define FMSTR_SCICR_SWAI      0x4000U
#define FMSTR_SCICR_RSRC      0x2000U
#define FMSTR_SCICR_M         0x1000U
#define FMSTR_SCICR_WAKE      0x0800U
#define FMSTR_SCICR_POL       0x0400U
#define FMSTR_SCICR_PE        0x0200U
#define FMSTR_SCICR_PT        0x0100U
#define FMSTR_SCICR_TEIE      0x0080U
#define FMSTR_SCICR_TIIE      0x0040U
#define FMSTR_SCICR_RFIE      0x0020U
#define FMSTR_SCICR_REIE      0x0010U
#define FMSTR_SCICR_TE        0x0008U
#define FMSTR_SCICR_RE        0x0004U
#define FMSTR_SCICR_RWU       0x0002U
#define FMSTR_SCICR_SBK       0x0001U

/* SCI Status registers bits */
#define FMSTR_SCISR_TDRE      0x8000U
#define FMSTR_SCISR_TIDLE     0x4000U
#define FMSTR_SCISR_RDRF      0x2000U
#define FMSTR_SCISR_RIDLE     0x1000U
#define FMSTR_SCISR_OR        0x0800U
#define FMSTR_SCISR_NF        0x0400U
#define FMSTR_SCISR_FE        0x0200U
#define FMSTR_SCISR_PF        0x0100U
#define FMSTR_SCISR_RAF       0x0001U

/******************************************************************************
*
* @brief    SCI communication initialization
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_56F800E_Sci_Init(void)
{
#if FMSTR_SERIAL_SINGLEWIRE
    #error Internal single wire mode is not supported.
    return FMSTR_FALSE;
#endif

    /* valid runtime module address must be assigned */
    if(fmstr_sciBaseAddr != 0)
        return FMSTR_TRUE;
    else
        return FMSTR_FALSE;
}

/******************************************************************************
*
* @brief    Enable/Disable SCI transmitter
*
******************************************************************************/

static void _FMSTR_56F800E_Sci_EnableTransmit(FMSTR_BOOL enable)
{
    if(enable)
    {
        /* Enable transmitter */
        FMSTR_SETBIT(fmstr_sciBaseAddr, FMSTR_SCICR_OFFSET, FMSTR_SCICR_TE);
    }
    else
    {
        /* Disable transmitter */
        FMSTR_CLRBIT(fmstr_sciBaseAddr, FMSTR_SCICR_OFFSET, FMSTR_SCICR_TE);
    }
}

/******************************************************************************
*
* @brief    Enable/Disable SCI receiver
*
******************************************************************************/

static void _FMSTR_56F800E_Sci_EnableReceive(FMSTR_BOOL enable)
{
    if(enable)
    {
        /* Enable receiver */
        FMSTR_SETBIT(fmstr_sciBaseAddr, FMSTR_SCICR_OFFSET, FMSTR_SCICR_RE);
    }
    else
    {
        /* Disable receiver */
        FMSTR_CLRBIT(fmstr_sciBaseAddr, FMSTR_SCICR_OFFSET, FMSTR_SCICR_RE);
    }
}

/******************************************************************************
*
* @brief    Enable/Disable interrupt from transmit register empty event
*
******************************************************************************/

static void _FMSTR_56F800E_Sci_EnableTransmitInterrupt(FMSTR_BOOL enable)
{
    if(enable)
    {
        /* Enable interrupt */
        FMSTR_SETBIT(fmstr_sciBaseAddr, FMSTR_SCICR_OFFSET, FMSTR_SCICR_TEIE);
    }
    else
    {
        /* Disable interrupt */
        FMSTR_CLRBIT(fmstr_sciBaseAddr, FMSTR_SCICR_OFFSET, FMSTR_SCICR_TEIE);
    }
}

/******************************************************************************
*
* @brief    Enable/Disable interrupt when transmission is complete (=idle)
*
******************************************************************************/

static void _FMSTR_56F800E_Sci_EnableTransmitCompleteInterrupt(FMSTR_BOOL enable)
{
    if(enable)
    {
        /* Enable interrupt */
        FMSTR_SETBIT(fmstr_sciBaseAddr, FMSTR_SCICR_OFFSET, FMSTR_SCICR_TIIE);
    }
    else
    {
        /* Disable interrupt */
        FMSTR_CLRBIT(fmstr_sciBaseAddr, FMSTR_SCICR_OFFSET, FMSTR_SCICR_TIIE);
    }
}

/******************************************************************************
*
* @brief    Enable/Disable interrupt from receive register full event
*
******************************************************************************/

static void _FMSTR_56F800E_Sci_EnableReceiveInterrupt(FMSTR_BOOL enable)
{
    if(enable)
    {
        /* Enable interrupt */
        FMSTR_SETBIT(fmstr_sciBaseAddr, FMSTR_SCICR_OFFSET, FMSTR_SCICR_RFIE);
    }
    else
    {
        /* Disable interrupt */
        FMSTR_CLRBIT(fmstr_sciBaseAddr, FMSTR_SCICR_OFFSET, FMSTR_SCICR_RFIE);
    }
}

/******************************************************************************
*
* @brief    Returns TRUE if the transmit register is empty, and it's possible to put next char
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_56F800E_Sci_IsTransmitRegEmpty(void)
{
    return (FMSTR_BOOL) FMSTR_TSTBIT(fmstr_sciBaseAddr, FMSTR_SCISR_OFFSET, FMSTR_SCISR_TDRE);
}

/******************************************************************************
*
* @brief    Returns TRUE if the receive register is full, and it's possible to get received char
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_56F800E_Sci_IsReceiveRegFull(void)
{
    return (FMSTR_BOOL) FMSTR_TSTBIT(fmstr_sciBaseAddr, FMSTR_SCISR_OFFSET, FMSTR_SCISR_RDRF);
}

/******************************************************************************
*
* @brief    Returns TRUE if the transmitter is still active
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_56F800E_Sci_IsTransmitterActive(void)
{
    return !(FMSTR_TSTBIT(fmstr_sciBaseAddr, FMSTR_SCISR_OFFSET, FMSTR_SCISR_TIDLE));
}

/******************************************************************************
*
* @brief    The function puts the char for transmit
*
******************************************************************************/

static void _FMSTR_56F800E_Sci_PutChar(FMSTR_BCHR  ch)
{
    FMSTR_SETREG(fmstr_sciBaseAddr, FMSTR_SCIDR_OFFSET, ch);
}

/******************************************************************************
*
* @brief    The function gets the received char
*
******************************************************************************/
static FMSTR_BCHR _FMSTR_56F800E_Sci_GetChar(void)
{
    FMSTR_BCHR ch = FMSTR_GETREG(fmstr_sciBaseAddr, FMSTR_SCIDR_OFFSET);

    /* also clear overrun (OR) flag to prevent excessive interrupts */
    if(FMSTR_TSTBIT(fmstr_sciBaseAddr, FMSTR_SCISR_OFFSET, FMSTR_SCISR_OR))
        FMSTR_SETREG(fmstr_sciBaseAddr, FMSTR_SCISR_OFFSET, FMSTR_SCISR_OR);

    return ch;
}

/******************************************************************************
*
* @brief    The function sends buffered data
*
******************************************************************************/

static void _FMSTR_56F800E_Sci_Flush(void)
{
}

/******************************************************************************
*
* @brief    Assign FreeMASTER communication module base address
*
******************************************************************************/

void FMSTR_SerialSetBaseAddress(FMSTR_U16 base)
{
    fmstr_sciBaseAddr = base;
}

/******************************************************************************
*
* @brief    Process FreeMASTER serial interrupt (call this function from SCI ISR)
*
******************************************************************************/

void FMSTR_SerialIsr(void)
{
    /* process incoming or just transmitted byte */
    #if (FMSTR_LONG_INTR) || (FMSTR_SHORT_INTR)
    FMSTR_ProcessSerial();

    /* Clear any error bits by reading SR and writing 0 back */
    FMSTR_GETREG(fmstr_sciBaseAddr, FMSTR_SCISR_OFFSET);
    FMSTR_SETREG(fmstr_sciBaseAddr, FMSTR_SCISR_OFFSET, 0);

    #endif
}

#else /* !(FMSTR_DISABLE) */

/* Empty API functions when FMSTR_DISABLE is set */
void FMSTR_SerialSetBaseAddress(FMSTR_U16 base)
{
    FMSTR_UNUSED(base);
}

void FMSTR_SerialIsr(void)
{
}

#endif /* !(FMSTR_DISABLE) */
#endif /* defined(FMSTR_SERIAL_DRV) && (FMSTR_MK_IDSTR(FMSTR_SERIAL_DRV) == FMSTR_SERIAL_56F800E_SCI_ID) */
