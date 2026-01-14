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
 * FreeMASTER Communication Driver - USART low-level driver
 */

#include "freemaster.h"
#include "freemaster_private.h"

/* Compile this code only if the SERIAL driver is selected in freemaster_cfg.h. */
#if defined(FMSTR_SERIAL_DRV) && (FMSTR_MK_IDSTR(FMSTR_SERIAL_DRV) == FMSTR_SERIAL_MCUX_USART_ID)

#include "freemaster_serial_usart.h"

#if FMSTR_DISABLE == 0

#include "fsl_common.h"
#include "fsl_usart.h"

#include "freemaster_protocol.h"
#include "freemaster_serial.h"

/******************************************************************************
 * Adapter configuration
 ******************************************************************************/
#if FMSTR_SERIAL_SINGLEWIRE > 0
#error The SUART driver does not support single wire configuration of UART communication.
#endif

/***********************************
 *  local variables
 ***********************************/

/* Serial base address */
#ifdef FMSTR_SERIAL_BASE
static USART_Type *fmstr_serialBaseAddr = FMSTR_SERIAL_BASE;
#else
static USART_Type *fmstr_serialBaseAddr = NULL;
#endif

/***********************************
 *  local function prototypes
 ***********************************/

/* Interface function - Initialization of serial UART driver adapter */
static FMSTR_BOOL _FMSTR_SerialUsartInit(void);
static void _FMSTR_SerialUsartEnableTransmit(FMSTR_BOOL enable);
static void _FMSTR_SerialUsartEnableReceive(FMSTR_BOOL enable);
static void _FMSTR_SerialUsartEnableTransmitInterrupt(FMSTR_BOOL enable);
static void _FMSTR_SerialUsartEnableTransmitCompleteInterrupt(FMSTR_BOOL enable);
static void _FMSTR_SerialUsartEnableReceiveInterrupt(FMSTR_BOOL enable);
static FMSTR_BOOL _FMSTR_SerialUsartIsTransmitRegEmpty(void);
static FMSTR_BOOL _FMSTR_SerialUsartIsReceiveRegFull(void);
static FMSTR_BOOL _FMSTR_SerialUsartIsTransmitterActive(void);
static void _FMSTR_SerialUsartPutChar(FMSTR_BCHR ch);
static FMSTR_BCHR _FMSTR_SerialUsartGetChar(void);
static void _FMSTR_SerialUsartFlush(void);

/***********************************
 *  global variables
 ***********************************/
/* Interface of this serial UART driver */
const FMSTR_SERIAL_DRV_INTF FMSTR_SERIAL_MCUX_USART = {
    .Init                            = _FMSTR_SerialUsartInit,
    .EnableTransmit                  = _FMSTR_SerialUsartEnableTransmit,
    .EnableReceive                   = _FMSTR_SerialUsartEnableReceive,
    .EnableTransmitInterrupt         = _FMSTR_SerialUsartEnableTransmitInterrupt,
    .EnableTransmitCompleteInterrupt = _FMSTR_SerialUsartEnableTransmitCompleteInterrupt,
    .EnableReceiveInterrupt          = _FMSTR_SerialUsartEnableReceiveInterrupt,
    .IsTransmitRegEmpty              = _FMSTR_SerialUsartIsTransmitRegEmpty,
    .IsReceiveRegFull                = _FMSTR_SerialUsartIsReceiveRegFull,
    .IsTransmitterActive             = _FMSTR_SerialUsartIsTransmitterActive,
    .PutChar                         = _FMSTR_SerialUsartPutChar,
    .GetChar                         = _FMSTR_SerialUsartGetChar,
    .Flush                           = _FMSTR_SerialUsartFlush,

};

/******************************************************************************
 *
 * @brief    Serial communication initialization
 *
 ******************************************************************************/
static FMSTR_BOOL _FMSTR_SerialUsartInit(void)
{
    /* valid runtime module address must be assigned */
    if (fmstr_serialBaseAddr == NULL)
    {
        return FMSTR_FALSE;
    }

    return FMSTR_TRUE;
}

/******************************************************************************
 *
 * @brief    Enable/Disable Serial transmitter
 *
 ******************************************************************************/

static void _FMSTR_SerialUsartEnableTransmit(FMSTR_BOOL enable)
{
}

/******************************************************************************
 *
 * @brief    Enable/Disable Serial receiver
 *
 ******************************************************************************/

static void _FMSTR_SerialUsartEnableReceive(FMSTR_BOOL enable)
{
}

/******************************************************************************
 *
 * @brief    Enable/Disable interrupt from transmit register empty event
 *
 ******************************************************************************/

static void _FMSTR_SerialUsartEnableTransmitInterrupt(FMSTR_BOOL enable)
{
    if (enable != FMSTR_FALSE)
    {
        USART_EnableInterrupts(fmstr_serialBaseAddr, (uint32_t)kUSART_TxLevelInterruptEnable);
    }
    else
    {
        USART_DisableInterrupts(fmstr_serialBaseAddr, (uint32_t)kUSART_TxLevelInterruptEnable);
    }
}

/******************************************************************************
 *
 * @brief    Enable/Disable interrupt from transmit complete event
 *
 ******************************************************************************/

static void _FMSTR_SerialUsartEnableTransmitCompleteInterrupt(FMSTR_BOOL enable)
{
    /* The UASRT peripheral has common interrupt enable for both events */
    _FMSTR_SerialUsartEnableTransmitInterrupt(enable);
}

/******************************************************************************
 *
 * @brief    Enable/Disable interrupt from receive register full event
 *
 ******************************************************************************/

static void _FMSTR_SerialUsartEnableReceiveInterrupt(FMSTR_BOOL enable)
{
    if (enable != FMSTR_FALSE)
    {
        USART_EnableInterrupts(fmstr_serialBaseAddr, (uint32_t)kUSART_RxLevelInterruptEnable);
    }
    else
    {
        USART_DisableInterrupts(fmstr_serialBaseAddr, (uint32_t)kUSART_RxLevelInterruptEnable);
    }
}

/******************************************************************************
 *
 * @brief    Returns TRUE if the transmit register is empty, and it's possible to put next char
 *
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_SerialUsartIsTransmitRegEmpty(void)
{
    uint32_t sr = USART_GetStatusFlags(fmstr_serialBaseAddr);

    return (FMSTR_BOOL)((sr & (uint32_t)kUSART_TxFifoNotFullFlag) != 0U);
}

/******************************************************************************
 *
 * @brief    Returns TRUE if the receive register is full, and it's possible to get received char
 *
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_SerialUsartIsReceiveRegFull(void)
{
    uint32_t sr = USART_GetStatusFlags(fmstr_serialBaseAddr);

    return (FMSTR_BOOL)((sr & (uint32_t)kUSART_RxFifoNotEmptyFlag) != 0U);
}

/******************************************************************************
 *
 * @brief    Returns TRUE if the transmitter is still active
 *
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_SerialUsartIsTransmitterActive(void)
{
    /* The SDK doesn't provide API access to USART status register,
        we need to access it directly. */
    return (FMSTR_BOOL)((fmstr_serialBaseAddr->STAT & USART_STAT_TXIDLE_MASK) == 0U);
}

/******************************************************************************
 *
 * @brief    The function puts the char for transmit
 *
 ******************************************************************************/

static void _FMSTR_SerialUsartPutChar(FMSTR_BCHR ch)
{
    USART_WriteByte(fmstr_serialBaseAddr, ch);
}

/******************************************************************************
 *
 * @brief    The function gets the received char
 *
 ******************************************************************************/
static FMSTR_BCHR _FMSTR_SerialUsartGetChar(void)
{
    return USART_ReadByte(fmstr_serialBaseAddr);
}

/******************************************************************************
 *
 * @brief    The function sends buffered data
 *
 ******************************************************************************/
static void _FMSTR_SerialUsartFlush(void)
{
}

/******************************************************************************
 *
 * @brief    Assigning FreeMASTER communication module base address
 *
 ******************************************************************************/
void FMSTR_SerialSetBaseAddress(USART_Type *base)
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

void FMSTR_SerialIsr(void)
{
    /* If RX overrun. */
    if ((fmstr_serialBaseAddr->FIFOSTAT & USART_FIFOSTAT_RXERR_MASK) != 0U)
    {
        /* Clear rx error state. */
        fmstr_serialBaseAddr->FIFOSTAT |= USART_FIFOSTAT_RXERR_MASK;
        /* clear rxFIFO */
        fmstr_serialBaseAddr->FIFOCFG |= USART_FIFOCFG_EMPTYRX_MASK;
    }

#if FMSTR_LONG_INTR > 0 || FMSTR_SHORT_INTR > 0
    /* Process received or just-transmitted byte. */
    FMSTR_ProcessSerial();
#endif
}

#else /* (!(FMSTR_DISABLE)) */

void FMSTR_SerialSetBaseAddress(USART_Type *base)
{
    FMSTR_UNUSED(base);
}

void FMSTR_SerialIsr(void)
{
}

#endif /* (!(FMSTR_DISABLE)) */
#endif /* defined(FMSTR_SERIAL_DRV) && (FMSTR_MK_IDSTR(FMSTR_SERIAL_DRV) == FMSTR_SERIAL_MCUX_USART_ID) */
