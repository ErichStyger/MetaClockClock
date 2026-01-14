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
 * FreeMASTER Communication Driver - low-level driver hooking the CDC class driver in USB stack
 */

#ifndef __FREEMASTER_SERIAL_USBCDC_H
#define __FREEMASTER_SERIAL_USBCDC_H

/******************************************************************************
 * Required header files include check
 ******************************************************************************/
#ifndef __FREEMASTER_H
#error Please include the freemaster.h master header file before the freemaster_serial_usb.h
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Types definition
 ******************************************************************************/

/* FreeMASTER Serial USB CDC event types. */
typedef enum
{
    FMSTR_SERIAL_USBCDC_EVENT_TYPE_SENT,     /* Interrupt type, that data was sent via USB CDC */
    FMSTR_SERIAL_USBCDC_EVENT_TYPE_RECEIVED, /* Interrupt type, that data was received from USB CDC */
} FMSTR_SERIAL_USBCDC_EVENT_TYPE;

/* Function for sending data via USB CDC. It must be registered from application. */
typedef FMSTR_BOOL (*FMSTR_TX_FUNC)(FMSTR_U8 *data, FMSTR_U32 size);

/******************************************************************************
 * inline functions
 ******************************************************************************/

/******************************************************************************
 * Global API functions
 ******************************************************************************/

/* Function for registering USB CDC transmit function.  */
void FMSTR_SerialUsbRegisterAppFunctions(FMSTR_TX_FUNC tx_func);
/* USB CDC event called from application into FreeMASTER serial interface. */
void FMSTR_SerialUsbProcessEvent(FMSTR_SERIAL_USBCDC_EVENT_TYPE type, FMSTR_U8 *data, FMSTR_U32 size);

#ifdef __cplusplus
}
#endif

/****************************************************************************************
 * Global variable declaration
 *****************************************************************************************/

#endif /* __FREEMASTER_SERIAL_USBCDC_H */
