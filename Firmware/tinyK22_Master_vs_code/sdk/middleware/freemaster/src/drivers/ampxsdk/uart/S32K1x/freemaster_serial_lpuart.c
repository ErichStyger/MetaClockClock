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
 * FreeMASTER Communication Driver - LPUART low-level driver
 */

#include "freemaster.h"
#include "freemaster_private.h"

#if FMSTR_SERIAL_SINGLEWIRE
    #error Not supported by S32K1x
#endif

/* Compile this code only if the SERIAL driver is selected in freemaster_cfg.h. */
#if defined(FMSTR_SERIAL_DRV) && (FMSTR_MK_IDSTR(FMSTR_SERIAL_DRV) == FMSTR_SERIAL_S32K1x_LPUART_ID)

#include "freemaster_serial_lpuart.h"

#if (!(FMSTR_DISABLE))
    
#include "lpuart_hw_access.h"

#include "freemaster_protocol.h"
#include "freemaster_serial.h"

/***********************************
*  local variables
***********************************/

/* Serial base address */
#ifdef FMSTR_SERIAL_BASE
    static LPUART_Type *fmstr_serialBaseAddr = FMSTR_SERIAL_BASE;
#else
    static LPUART_Type *fmstr_serialBaseAddr = NULL;
#endif

/***********************************
*  local function prototypes
***********************************/

/* Interface function - Initialization of serial UART driver adapter */
static FMSTR_BOOL _FMSTR_SerialLpuartInit(void);
static void _FMSTR_SerialLpuartEnableTransmit(FMSTR_BOOL enable);
static void _FMSTR_SerialLpuartEnableReceive(FMSTR_BOOL enable);
static void _FMSTR_SerialLpuartEnableTransmitInterrupt(FMSTR_BOOL enable);
static void _FMSTR_SerialLpuartEnableTransmitCompleteInterrupt(FMSTR_BOOL enable);
static void _FMSTR_SerialLpuartEnableReceiveInterrupt(FMSTR_BOOL enable);
static FMSTR_BOOL _FMSTR_SerialLpuartIsTransmitRegEmpty(void);
static FMSTR_BOOL _FMSTR_SerialLpuartIsReceiveRegFull(void);
static FMSTR_BOOL _FMSTR_SerialLpuartIsTransmitterActive(void);
static void _FMSTR_SerialLpuartPutChar(FMSTR_BCHR  ch);
static FMSTR_BCHR _FMSTR_SerialLpuartGetChar(void);
static void _FMSTR_SerialLpuartFlush(void);

/***********************************
*  global variables
***********************************/
/* Interface of this serial UART driver */
const FMSTR_SERIAL_DRV_INTF FMSTR_SERIAL_S32K1x_LPUART =
{
    .Init                           = _FMSTR_SerialLpuartInit,
    .EnableTransmit                 = _FMSTR_SerialLpuartEnableTransmit,
    .EnableReceive                  = _FMSTR_SerialLpuartEnableReceive,
    .EnableTransmitInterrupt        = _FMSTR_SerialLpuartEnableTransmitInterrupt,
    .EnableTransmitCompleteInterrupt= _FMSTR_SerialLpuartEnableTransmitCompleteInterrupt,
    .EnableReceiveInterrupt         = _FMSTR_SerialLpuartEnableReceiveInterrupt,
    .IsTransmitRegEmpty             = _FMSTR_SerialLpuartIsTransmitRegEmpty,
    .IsReceiveRegFull               = _FMSTR_SerialLpuartIsReceiveRegFull,
    .IsTransmitterActive            = _FMSTR_SerialLpuartIsTransmitterActive,
    .PutChar                        = _FMSTR_SerialLpuartPutChar,
    .GetChar                        = _FMSTR_SerialLpuartGetChar,
    .Flush                          = _FMSTR_SerialLpuartFlush,

};

/******************************************************************************
*
* @brief    Serial communication initialization
*
******************************************************************************/
static FMSTR_BOOL _FMSTR_SerialLpuartInit(void)
{
    /* valid runtime module address must be assigned */
    if(!fmstr_serialBaseAddr)
        return FMSTR_FALSE;

    return FMSTR_TRUE;
}

/******************************************************************************
*
* @brief    Enable/Disable Serial transmitter
*
******************************************************************************/

static void _FMSTR_SerialLpuartEnableTransmit(FMSTR_BOOL enable)
{
    LPUART_SetTransmitterCmd(fmstr_serialBaseAddr, enable);
}

/******************************************************************************
*
* @brief    Enable/Disable Serial receiver
*
******************************************************************************/

static void _FMSTR_SerialLpuartEnableReceive(FMSTR_BOOL enable)
{
    LPUART_SetReceiverCmd(fmstr_serialBaseAddr, enable);
}

/******************************************************************************
*
* @brief    Enable/Disable interrupt from transmit register empty event
*
******************************************************************************/

static void _FMSTR_SerialLpuartEnableTransmitInterrupt(FMSTR_BOOL enable)
{
    if(enable)
        LPUART_SetIntMode(fmstr_serialBaseAddr, LPUART_INT_TX_DATA_REG_EMPTY, 1);
    else
        LPUART_SetIntMode(fmstr_serialBaseAddr, LPUART_INT_TX_DATA_REG_EMPTY, 0);
}

/******************************************************************************
*
* @brief    Enable/Disable interrupt from transmit complete event
*
******************************************************************************/

static void _FMSTR_SerialLpuartEnableTransmitCompleteInterrupt(FMSTR_BOOL enable)
{
    // Single-wire communication is not (yet) enabled in this driver.
}

/******************************************************************************
*
* @brief    Enable/Disable interrupt from receive register full event
*
******************************************************************************/

static void _FMSTR_SerialLpuartEnableReceiveInterrupt(FMSTR_BOOL enable)
{
     if(enable)
         LPUART_SetIntMode(fmstr_serialBaseAddr, LPUART_INT_RX_DATA_REG_FULL, 1);
    else
         LPUART_SetIntMode(fmstr_serialBaseAddr, LPUART_INT_RX_DATA_REG_FULL, 0);
}

/******************************************************************************
*
* @brief    Returns TRUE if the transmit register is empty, and it's possible to put next char
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_SerialLpuartIsTransmitRegEmpty(void)
{
    return LPUART_GetStatusFlag(fmstr_serialBaseAddr, LPUART_TX_DATA_REG_EMPTY);
}

/******************************************************************************
*
* @brief    Returns TRUE if the receive register is full, and it's possible to get received char
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_SerialLpuartIsReceiveRegFull(void)
{
    return LPUART_GetStatusFlag(fmstr_serialBaseAddr, LPUART_RX_DATA_REG_FULL);
}

/******************************************************************************
*
* @brief    Returns TRUE if the transmitter is still active
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_SerialLpuartIsTransmitterActive(void)
{
    return LPUART_GetStatusFlag(fmstr_serialBaseAddr, LPUART_TX_COMPLETE);
}

/******************************************************************************
*
* @brief    The function puts the char for transmit
*
******************************************************************************/

static void _FMSTR_SerialLpuartPutChar(FMSTR_BCHR  ch)
{
    LPUART_Putchar9(fmstr_serialBaseAddr, (uint16_t)ch);
}

/******************************************************************************
*
* @brief    The function gets the received char
*
******************************************************************************/
static FMSTR_BCHR _FMSTR_SerialLpuartGetChar(void)
{
    uint16_t readData;
    LPUART_Getchar9(fmstr_serialBaseAddr, &readData) ;
    return (unsigned char)(readData & 0xFF);
}

/******************************************************************************
*
* @brief    The function sends buffered data
*
******************************************************************************/
static void _FMSTR_SerialLpuartFlush(void)
{
}

/******************************************************************************
*
* @brief    Assigning FreeMASTER communication module base address
*
******************************************************************************/
void FMSTR_SerialSetBaseAddress(LPUART_Type *base)
{
    fmstr_serialBaseAddr = base;
}

/******************************************************************************
*
* @brief    API: Interrupt handler call
*
* This Interrupt Service Routine handles the UART  interrupts for the FreeMASTER
* driver. In case you want to handle the interrupt in the application yourselves,
* call the FMSTR_ProcessSerial function which
* does the same job but is not compiled as an Interrupt Service Routine.
*
* In poll-driven mode (FMSTR_POLL_DRIVEN) this function does nothing.
*
******************************************************************************/

void FMSTR_SerialIsr()
{
    /* process incomming or just transmitted byte */
    #if (FMSTR_LONG_INTR) || (FMSTR_SHORT_INTR)
        FMSTR_ProcessSerial();
    #endif
}

#else /* (!(FMSTR_DISABLE)) */

void FMSTR_SerialSetBaseAddress(LPUART_Type *base)
{
    FMSTR_UNUSED(base);
}

void FMSTR_SerialIsr(void)
{
}

#endif /* (!(FMSTR_DISABLE)) */
#endif /* defined(FMSTR_SERIAL_DRV) && (FMSTR_MK_IDSTR(FMSTR_SERIAL_DRV) == FMSTR_SERIAL_S32K1x_LPUART_ID) */
