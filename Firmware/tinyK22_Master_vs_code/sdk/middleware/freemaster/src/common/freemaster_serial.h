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
 * FreeMASTER Communication Driver - Serial communication
 */

#ifndef __FREEMASTER_SERIAL_H
#define __FREEMASTER_SERIAL_H

/******************************************************************************
* Required header files include check
******************************************************************************/
#ifndef __FREEMASTER_H
#error Please include the freemaster.h master header file before the freemaster_serial.h
#endif

/******************************************************************************
* Serial transport related configuration checks
******************************************************************************/

/* obsolete transport and driver macros */
#if defined(FMSTR_USE_SCI) || defined(FMSTR_USE_LPUART) || defined(FMSTR_USE_ESCI) || defined(FMSTR_USE_UART) || \
    defined(FMSTR_USE_JTAG) || defined(FMSTR_USE_USB_CDC) || defined(FMSTR_USE_MQX_IO) || defined(FMSTR_USE_MBED)
    #warning Obsolete configuration option detected. Define FMSTR_TRANSPORT as FMSTR_SERIAL, and define also FMSTR_SERIAL_DRV option.
#endif

/* transport MUST be defined in configuration */
#if !defined(FMSTR_SERIAL_DRV)
    #error No FreeMASTER communication serial driver interface is enabled. Please choose the driver (FMSTR_SERIAL_DRV) or set FMSTR_DISABLE option to 1.
#endif

/* Switch serial for single wire communication */
#ifndef FMSTR_SERIAL_SINGLEWIRE
    #define FMSTR_SERIAL_SINGLEWIRE 0
#endif

#if FMSTR_COMM_BUFFER_SIZE > 254
    #error FMSTR_COMM_BUFFER_SIZE must be set less than 254 on Serial transport.
#endif

#if FMSTR_SHORT_INTR > 0
    /* default short-interrupt FIFO size */
    #ifndef FMSTR_COMM_RQUEUE_SIZE
    #define FMSTR_COMM_RQUEUE_SIZE 32
    #endif

    #if FMSTR_COMM_RQUEUE_SIZE == 0
    #undef  FMSTR_COMM_RQUEUE_SIZE
    #define FMSTR_COMM_RQUEUE_SIZE 32
    #endif
#endif

/* obsolete dynamic address selection */
#if defined(FMSTR_SCI_BASE_DYNAMIC)
    #warning The FMSTR_SCI_BASE_DYNAMIC is now obsolete and may be removed.
#endif

#ifdef __cplusplus
  extern "C" {
#endif

/******************************************************************************
* Types definition
******************************************************************************/

/* Serial Driver interface between the Serial layer and hardware driver */
typedef struct FMSTR_SERIAL_DRV_INTF_S
{
    FMSTR_BOOL (*Init)(void);                           /* Initialization */
    void (*EnableTransmit)(FMSTR_BOOL enable);          /* Enable/Disable transmitter */
    void (*EnableReceive)(FMSTR_BOOL enable);           /* Enable/Disable receiver */
    void (*EnableTransmitInterrupt)(FMSTR_BOOL enable); /* Enable/Disable interrupt from transmit register empty event */
    void (*EnableTransmitCompleteInterrupt)(FMSTR_BOOL enable); /* Enable/Disable interrupt from transmit complete event */
    void (*EnableReceiveInterrupt)(FMSTR_BOOL enable);  /* Enable/Disable interrupt from receive register full event */
    FMSTR_BOOL (*IsTransmitRegEmpty)(void);             /* Returns TRUE if the transmit register is empty, and it's possible to put next char */
    FMSTR_BOOL (*IsReceiveRegFull)(void);               /* Returns TRUE if the receive register is full, and it's possible to get received char */
    FMSTR_BOOL (*IsTransmitterActive)(void);            /* Returns TRUE if the transmitter is still active */
    void (*PutChar)(FMSTR_BCHR  ch);                    /* Transmit character */
    FMSTR_BCHR (*GetChar)(void);                        /* Receive character */
    void (*Flush)(void);                                /* Transmit-flush all buffered data */
    void (*Poll)(void);                                 /* General poll call (optional) */
} FMSTR_SERIAL_DRV_INTF;

/******************************************************************************
* Functions definitions
******************************************************************************/

/* Serial Process function - This function is called by communication sub system,
    in case of new data received or next data byte has been transmitted. */
void FMSTR_ProcessSerial(void);

/* If defined in freemaster_cfg.h, make sure the SERIAL driver interface is also declared properly. */
#ifdef FMSTR_SERIAL_DRV
extern const FMSTR_SERIAL_DRV_INTF FMSTR_SERIAL_DRV;
#endif

#ifdef __cplusplus
  }
#endif

#endif /* __FREEMASTER_SERIAL_H */
