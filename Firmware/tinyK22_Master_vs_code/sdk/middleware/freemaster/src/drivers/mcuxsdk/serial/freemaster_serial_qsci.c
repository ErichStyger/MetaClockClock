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
 * FreeMASTER Communication Driver - QSCI low-level driver
 */

#include "freemaster.h"
#include "freemaster_private.h"

//* Compile this code only if the SERIAL driver is selected in freemaster_cfg.h. */
#if defined(FMSTR_SERIAL_DRV) && (FMSTR_MK_IDSTR(FMSTR_SERIAL_DRV) == FMSTR_SERIAL_MCUX_QSCI_ID)

#include "freemaster_serial_qsci.h"

#if FMSTR_DISABLE == 0

#include "fsl_common.h"
#include "fsl_qsci.h"

#include "freemaster_protocol.h"
#include "freemaster_serial.h"

/******************************************************************************
 * Adapter configuration
 ******************************************************************************/

/***********************************
 *  local variables
 ***********************************/

/* Serial base address */
#ifdef FMSTR_SERIAL_BASE
static QSCI_Type *fmstr_serialBaseAddr = FMSTR_SERIAL_BASE;
#else
static QSCI_Type *fmstr_serialBaseAddr = NULL;
#endif

/***********************************
 *  local function prototypes
 ***********************************/

/* Interface function - Initialization of serial UART driver adapter */
static FMSTR_BOOL _FMSTR_QSCIInit(void);
static void _FMSTR_QSCIEnableTransmit(FMSTR_BOOL enable);
static void _FMSTR_QSCIEnableReceive(FMSTR_BOOL enable);
static void _FMSTR_QSCIEnableTransmitInterrupt(FMSTR_BOOL enable);
static void _FMSTR_QSCIEnableTransmitCompleteInterrupt(FMSTR_BOOL enable);
static void _FMSTR_QSCIEnableReceiveInterrupt(FMSTR_BOOL enable);
static FMSTR_BOOL _FMSTR_QSCIIsTransmitRegEmpty(void);
static FMSTR_BOOL _FMSTR_QSCIIsReceiveRegFull(void);
static FMSTR_BOOL _FMSTR_QSCIIsTransmitterActive(void);
static void _FMSTR_QSCIPutChar(FMSTR_BCHR ch);
static FMSTR_BCHR _FMSTR_QSCIGetChar(void);
static void _FMSTR_QSCIFlush(void);

/***********************************
 *  global variables
 ***********************************/
/* Interface of this serial UART driver */
const FMSTR_SERIAL_DRV_INTF FMSTR_SERIAL_MCUX_QSCI = {
    FMSTR_C99_INIT(Init) _FMSTR_QSCIInit,
    FMSTR_C99_INIT(EnableTransmit) _FMSTR_QSCIEnableTransmit,
    FMSTR_C99_INIT(EnableReceive) _FMSTR_QSCIEnableReceive,
    FMSTR_C99_INIT(EnableTransmitInterrupt) _FMSTR_QSCIEnableTransmitInterrupt,
    FMSTR_C99_INIT(EnableTransmitCompleteInterrupt) _FMSTR_QSCIEnableTransmitCompleteInterrupt,
    FMSTR_C99_INIT(EnableReceiveInterrupt) _FMSTR_QSCIEnableReceiveInterrupt,
    FMSTR_C99_INIT(IsTransmitRegEmpty) _FMSTR_QSCIIsTransmitRegEmpty,
    FMSTR_C99_INIT(IsReceiveRegFull) _FMSTR_QSCIIsReceiveRegFull,
    FMSTR_C99_INIT(IsTransmitterActive) _FMSTR_QSCIIsTransmitterActive,
    FMSTR_C99_INIT(PutChar) _FMSTR_QSCIPutChar,
    FMSTR_C99_INIT(GetChar) _FMSTR_QSCIGetChar,
    FMSTR_C99_INIT(Flush) _FMSTR_QSCIFlush,
};

/******************************************************************************
 *
 * @brief    Serial communication initialization
 *
 ******************************************************************************/
static FMSTR_BOOL _FMSTR_QSCIInit(void)
{
    /* Valid runtime module address must be assigned */
    if (fmstr_serialBaseAddr == NULL)
    {
        return FMSTR_FALSE;
    }

#if FMSTR_SERIAL_SINGLEWIRE > 0
#error Internal single wire mode is not supported.
    return FMSTR_FALSE;
#endif

    return FMSTR_TRUE;
}

/******************************************************************************
 *
 * @brief    Enable/Disable Serial transmitter
 *
 ******************************************************************************/

static void _FMSTR_QSCIEnableTransmit(FMSTR_BOOL enable)
{
    QSCI_EnableTx(fmstr_serialBaseAddr, enable != FMSTR_FALSE);
}

/******************************************************************************
 *
 * @brief    Enable/Disable Serial receiver
 *
 ******************************************************************************/

static void _FMSTR_QSCIEnableReceive(FMSTR_BOOL enable)
{
    QSCI_EnableRx(fmstr_serialBaseAddr, enable != FMSTR_FALSE);
}

/******************************************************************************
 *
 * @brief    Enable/Disable interrupt from transmit register empty event
 *
 ******************************************************************************/

static void _FMSTR_QSCIEnableTransmitInterrupt(FMSTR_BOOL enable)
{
    if (enable != FMSTR_FALSE)
    {
        QSCI_EnableInterrupts(fmstr_serialBaseAddr, (uint8_t)kQSCI_TxEmptyInterruptEnable);
    }
    else
    {
        QSCI_DisableInterrupts(fmstr_serialBaseAddr, (uint8_t)kQSCI_TxEmptyInterruptEnable);
    }
}

/******************************************************************************
 *
 * @brief    Enable/Disable interrupt from transmit complete event
 *
 ******************************************************************************/

static void _FMSTR_QSCIEnableTransmitCompleteInterrupt(FMSTR_BOOL enable)
{
    if (enable != FMSTR_FALSE)
    {
        QSCI_EnableInterrupts(fmstr_serialBaseAddr, (uint8_t)kQSCI_TxIdleInterruptEnable);
    }
    else
    {
        QSCI_DisableInterrupts(fmstr_serialBaseAddr, (uint8_t)kQSCI_TxIdleInterruptEnable);
    }
}

/******************************************************************************
 *
 * @brief    Enable/Disable interrupt from receive register full event
 *
 ******************************************************************************/

static void _FMSTR_QSCIEnableReceiveInterrupt(FMSTR_BOOL enable)
{
    /* Note that if RX Idle and Error interrupts are "accidentally" enabled by user, we disable
     * them along with the RX Data interrupt. This avoids unnecessary interrupts to be serviced. */
    if (enable != FMSTR_FALSE)
    {
        QSCI_EnableInterrupts(fmstr_serialBaseAddr, (uint8_t)kQSCI_RxFullInterruptEnable);
    }
    else
    {
        QSCI_DisableInterrupts(fmstr_serialBaseAddr, (uint8_t)(
            (uint8_t)kQSCI_RxFullInterruptEnable | 
            (uint8_t)kQSCI_RxErrorInterruptEnable 
#if defined(FSL_FEATURE_QSCI_HAS_RX_IDLE_INTERRUPT) && FSL_FEATURE_QSCI_HAS_RX_IDLE_INTERRUPT
            | (uint8_t)kQSCI_RxIdleLineInterruptEnable
#endif
            ));
    }
}

/******************************************************************************
 *
 * @brief    Returns TRUE if the transmit register is empty, and it's possible to put next char
 *
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_QSCIIsTransmitRegEmpty(void)
{
    uint32_t sr = QSCI_GetStatusFlags(fmstr_serialBaseAddr);

    return (FMSTR_BOOL)((sr & (uint32_t)kQSCI_TxDataRegEmptyFlag) != 0U);
}

/******************************************************************************
 *
 * @brief    Returns TRUE if the receive register is full, and it's possible to get received char
 *
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_QSCIIsReceiveRegFull(void)
{
    uint32_t sr = QSCI_GetStatusFlags(fmstr_serialBaseAddr);

    return (FMSTR_BOOL)((sr & (uint32_t)kQSCI_RxDataRegFullFlag) != 0U);
}

/******************************************************************************
 *
 * @brief    Returns TRUE if the transmitter is still active
 *
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_QSCIIsTransmitterActive(void)
{
    uint32_t sr = QSCI_GetStatusFlags(fmstr_serialBaseAddr);

    return (FMSTR_BOOL)((sr & (uint32_t)kQSCI_TxIdleFlag) == 0U);
}

/******************************************************************************
 *
 * @brief    The function puts the char for transmit
 *
 ******************************************************************************/

static void _FMSTR_QSCIPutChar(FMSTR_BCHR ch)
{
    QSCI_WriteByte(fmstr_serialBaseAddr, ch);
}

/******************************************************************************
 *
 * @brief    The function gets the received char
 *
 ******************************************************************************/
static FMSTR_BCHR _FMSTR_QSCIGetChar(void)
{
    return QSCI_ReadByte(fmstr_serialBaseAddr);
}

/******************************************************************************
 *
 * @brief    The function sends buffered data
 *
 ******************************************************************************/
static void _FMSTR_QSCIFlush(void)
{
}

/******************************************************************************
 *
 * @brief    Assigning FreeMASTER communication module base address
 *
 ******************************************************************************/
void FMSTR_SerialSetBaseAddress(QSCI_Type *base)
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
#if FMSTR_LONG_INTR > 0 || FMSTR_SHORT_INTR > 0
    /* Process received or just-transmitted byte. */
    FMSTR_ProcessSerial();

    /* Clear error flags and also the input-edge flag. Note that "RX Idle" flag is not cleared as it
     * messes up with receive process, rather the interrupt is disabled as a whole above. */
    QSCI_ClearStatusFlags(fmstr_serialBaseAddr, (uint32_t)(kQSCI_Group1Flags | kQSCI_RxInputEdgeFlag));

#endif
}

#else /* (!(FMSTR_DISABLE)) */

void FMSTR_SerialSetBaseAddress(QSCI_Type *base)
{
    FMSTR_UNUSED(base);
}

void FMSTR_SerialIsr(void)
{
}

#endif /* (!(FMSTR_DISABLE)) */
#endif /* defined(FMSTR_SERIAL_DRV) && (FMSTR_MK_IDSTR(FMSTR_SERIAL_DRV) == FMSTR_SERIAL_MCUX_QSCI_ID) */
