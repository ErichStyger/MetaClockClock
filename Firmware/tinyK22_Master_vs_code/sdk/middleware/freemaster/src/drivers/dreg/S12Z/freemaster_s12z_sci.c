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
 * FreeMASTER Communication Driver - Serial Communication Interface
 */
#include "freemaster.h"
#include "freemaster_private.h"

/* Compile this code only if the SERIAL driver is selected in freemaster_cfg.h. */
#if defined(FMSTR_SERIAL_DRV) && (FMSTR_MK_IDSTR(FMSTR_SERIAL_DRV) == FMSTR_SERIAL_S12Z_SCI_ID)
#if !(FMSTR_DISABLE) 

#include "freemaster_serial.h"
#include "freemaster_s12z_sci.h"

/***********************************
*  local variables
***********************************/

/* SCI base address */
#ifdef FMSTR_SCI_BASE
    static FMSTR_ADDR fmstr_sciBaseAddr = (FMSTR_ADDR)FMSTR_SCI_BASE;
#else
    static FMSTR_ADDR fmstr_sciBaseAddr = (FMSTR_ADDR)0;
#endif

/***********************************
*  local function prototypes
***********************************/

/* Interface function - Initialization of SCI driver adapter */
static FMSTR_BOOL _FMSTR_S12zSci_Init(void);
static void _FMSTR_S12zSci_EnableTransmit(FMSTR_BOOL enable);
static void _FMSTR_S12zSci_EnableReceive(FMSTR_BOOL enable);
static void _FMSTR_S12zSci_EnableTransmitInterrupt(FMSTR_BOOL enable);
static void _FMSTR_S12zSci_EnableReceiveInterrupt(FMSTR_BOOL enable);
static void _FMSTR_S12zSci_EnableTransmitCompleteInterrupt(FMSTR_BOOL enable);
static FMSTR_BOOL _FMSTR_S12zSci_IsTransmitRegEmpty(void);
static FMSTR_BOOL _FMSTR_S12zSci_IsReceiveRegFull(void);
static FMSTR_BOOL _FMSTR_S12zSci_IsTransmitterActive(void);
static void _FMSTR_S12zSci_PutChar(FMSTR_BCHR  ch);
static FMSTR_BCHR _FMSTR_S12zSci_GetChar(void);
static void _FMSTR_S12zSci_Flush(void);

/***********************************
*  global variables
***********************************/
/* Interface of this SCI driver */ 

const FMSTR_SERIAL_DRV_INTF FMSTR_SERIAL_S12Z_SCI =
{
    FMSTR_C99_INIT(Init                       ) _FMSTR_S12zSci_Init,
    FMSTR_C99_INIT(EnableTransmit             ) _FMSTR_S12zSci_EnableTransmit,
    FMSTR_C99_INIT(EnableReceive              ) _FMSTR_S12zSci_EnableReceive,
    FMSTR_C99_INIT(EnableTransmitInterrupt    ) _FMSTR_S12zSci_EnableTransmitInterrupt,
    FMSTR_C99_INIT(EnableTransmitCompleteInterrupt ) _FMSTR_S12zSci_EnableTransmitCompleteInterrupt,
    FMSTR_C99_INIT(EnableReceiveInterrupt     ) _FMSTR_S12zSci_EnableReceiveInterrupt,
    FMSTR_C99_INIT(IsTransmitRegEmpty         ) _FMSTR_S12zSci_IsTransmitRegEmpty,
    FMSTR_C99_INIT(IsReceiveRegFull           ) _FMSTR_S12zSci_IsReceiveRegFull,
    FMSTR_C99_INIT(IsTransmitterActive        ) _FMSTR_S12zSci_IsTransmitterActive,
    FMSTR_C99_INIT(PutChar                    ) _FMSTR_S12zSci_PutChar,
    FMSTR_C99_INIT(GetChar                    ) _FMSTR_S12zSci_GetChar,
    FMSTR_C99_INIT(Flush                      ) _FMSTR_S12zSci_Flush,
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
* SCI module constants
*****************************************************************************************/

/* SCI module registers */
#define FMSTR_SCIBDH_OFFSET     0
#define FMSTR_SCIBDL_OFFSET     1
#define FMSTR_SCICR1_OFFSET     2
#define FMSTR_SCICR2_OFFSET     3
#define FMSTR_SCISR1_OFFSET     4
#define FMSTR_SCISR2_OFFSET     5
#define FMSTR_SCIDRH_OFFSET     6
#define FMSTR_SCIDRL_OFFSET     7

/* SCI Control Register bits */
#define FMSTR_SCICR1_LOOPS     0x80
#define FMSTR_SCICR1_SWAI      0x40
#define FMSTR_SCICR1_RSRC      0x20
#define FMSTR_SCICR1_M         0x10
#define FMSTR_SCICR1_WAKE      0x08
#define FMSTR_SCICR1_ILT       0x04
#define FMSTR_SCICR1_PE        0x02
#define FMSTR_SCICR1_PT        0x01
#define FMSTR_SCICR2_TIE       0x80
#define FMSTR_SCICR2_TCIE      0x40
#define FMSTR_SCICR2_RIE       0x20
#define FMSTR_SCICR2_ILIE      0x10
#define FMSTR_SCICR2_TE        0x08
#define FMSTR_SCICR2_RE        0x04
#define FMSTR_SCICR2_RWU       0x02
#define FMSTR_SCICR2_SBK       0x01

/* SCI Status registers bits */
#define FMSTR_SCISR_TDRE       0x80
#define FMSTR_SCISR_TC         0x40
#define FMSTR_SCISR_RDRF       0x20
#define FMSTR_SCISR_IDLE       0x10
#define FMSTR_SCISR_OR         0x08
#define FMSTR_SCISR_NF         0x04
#define FMSTR_SCISR_FE         0x02
#define FMSTR_SCISR_PF         0x01
#define FMSTR_SCISR2_BRK13     0x04
#define FMSTR_SCISR2_TXDIR     0x02
#define FMSTR_SCISR2_RAF       0x01

/******************************************************************************
*
* @brief    SCI communication initialization
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_S12zSci_Init(void)
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

static void _FMSTR_S12zSci_EnableTransmit(FMSTR_BOOL enable)
{
    if(enable)
    {
        /* Enable transmitter */
        FMSTR_SETBIT(fmstr_sciBaseAddr, FMSTR_SCICR2_OFFSET, FMSTR_SCICR2_TE);
    }
    else
    {
        /* Disable transmitter */
        FMSTR_CLRBIT(fmstr_sciBaseAddr, FMSTR_SCICR2_OFFSET, FMSTR_SCICR2_TE);
    }
}

/******************************************************************************
*
* @brief    Enable/Disable SCI receiver
*
******************************************************************************/

static void _FMSTR_S12zSci_EnableReceive(FMSTR_BOOL enable)
{
    if(enable)
    {
        /* Enable receiver (enables single-wire connection) */
        FMSTR_SETBIT(fmstr_sciBaseAddr, FMSTR_SCICR2_OFFSET, FMSTR_SCICR2_RE);
    } 
    else
    {
        /* Disable receiver */
        FMSTR_CLRBIT(fmstr_sciBaseAddr, FMSTR_SCICR2_OFFSET, FMSTR_SCICR2_RE);
    }
}

/******************************************************************************
*
* @brief    Enable/Disable interrupt from transmit register empty event
*
******************************************************************************/

static void _FMSTR_S12zSci_EnableTransmitInterrupt(FMSTR_BOOL enable)
{
    if(enable)
    {
        /* Enable interrupt */
        FMSTR_SETBIT(fmstr_sciBaseAddr, FMSTR_SCICR2_OFFSET, FMSTR_SCICR2_TIE);
    }
    else
    {
        /* Disable interrupt */
        FMSTR_CLRBIT(fmstr_sciBaseAddr, FMSTR_SCICR2_OFFSET, FMSTR_SCICR2_TIE);
    }
}

/******************************************************************************
*
* @brief    Enable/Disable interrupt when transmission is complete
*
******************************************************************************/

static void _FMSTR_S12zSci_EnableTransmitCompleteInterrupt(FMSTR_BOOL enable)
{
    if(enable)
    {
        /* Enable interrupt */
        FMSTR_SETBIT(fmstr_sciBaseAddr, FMSTR_SCICR2_OFFSET, FMSTR_SCICR2_TCIE);
    }
    else
    {
        /* Disable interrupt */
        FMSTR_CLRBIT(fmstr_sciBaseAddr, FMSTR_SCICR2_OFFSET, FMSTR_SCICR2_TCIE);
    }
}

/******************************************************************************
*
* @brief    Enable/Disable interrupt from receive register full event
*
******************************************************************************/

static void _FMSTR_S12zSci_EnableReceiveInterrupt(FMSTR_BOOL enable)
{
    if(enable)
    {
        /* Enable interrupt */
        FMSTR_SETBIT(fmstr_sciBaseAddr, FMSTR_SCICR2_OFFSET, FMSTR_SCICR2_RIE);
    }
    else
    {
        /* Disable interrupt */
        FMSTR_CLRBIT(fmstr_sciBaseAddr, FMSTR_SCICR2_OFFSET, FMSTR_SCICR2_RIE);
    }
}

/******************************************************************************
*
* @brief    Returns TRUE if the transmit register is empty, and it's possible to put next char
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_S12zSci_IsTransmitRegEmpty(void)
{
    return (FMSTR_BOOL) FMSTR_TSTBIT(fmstr_sciBaseAddr, FMSTR_SCISR1_OFFSET, FMSTR_SCISR_TDRE);
}

/******************************************************************************
*
* @brief    Returns TRUE if the receive register is full, and it's possible to get received char
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_S12zSci_IsReceiveRegFull(void)
{
    return (FMSTR_BOOL) FMSTR_TSTBIT(fmstr_sciBaseAddr, FMSTR_SCISR1_OFFSET, FMSTR_SCISR_RDRF);
}

/******************************************************************************
*
* @brief    Returns TRUE if the transmitter is still active 
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_S12zSci_IsTransmitterActive(void)
{
    /* 0 - Transmission in progress, 1 - No transmission in progress */
    return (FMSTR_BOOL) (!(FMSTR_TSTBIT(fmstr_sciBaseAddr, FMSTR_SCISR1_OFFSET, FMSTR_SCISR_TC)));
}

/******************************************************************************
*
* @brief    The function puts the char for transmit
*
******************************************************************************/

static void _FMSTR_S12zSci_PutChar(FMSTR_BCHR  ch)
{
    FMSTR_SETREG(fmstr_sciBaseAddr, FMSTR_SCIDRL_OFFSET, ch);
}

/******************************************************************************
*
* @brief    The function gets the received char
*
******************************************************************************/
static FMSTR_BCHR _FMSTR_S12zSci_GetChar(void)
{
    return FMSTR_GETREG(fmstr_sciBaseAddr, FMSTR_SCIDRL_OFFSET);
}

/******************************************************************************
*
* @brief    The function sends buffered data
*
******************************************************************************/

static void _FMSTR_S12zSci_Flush(void)
{
}

/******************************************************************************
*
* @brief    Assign FreeMASTER communication module base address
*
******************************************************************************/

void FMSTR_SerialSetBaseAddress(FMSTR_ADDR base)
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
    /* process incomming or just transmitted byte */
    #if (FMSTR_LONG_INTR) || (FMSTR_SHORT_INTR)
        FMSTR_ProcessSerial();
    #endif
}

#else /* !(FMSTR_DISABLE) */

/* Empty API functions when FMSTR_DISABLE is set */
void FMSTR_SerialSetBaseAddress(FMSTR_ADDR base)
{
    FMSTR_UNUSED(base);
}

void FMSTR_SerialIsr(void)
{
}

#endif /* !(FMSTR_DISABLE) */ 
#endif /* defined(FMSTR_SERIAL_DRV) && (FMSTR_MK_IDSTR(FMSTR_SERIAL_DRV) == FMSTR_SERIAL_S12Z_SCI_ID) */
