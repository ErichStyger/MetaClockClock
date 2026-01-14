/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2018-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * FreeMASTER Communication Driver - USB Example Implementation
 */

#ifndef __FREEMASTER_EXAMPLE_USB_H
#define __FREEMASTER_EXAMPLE_USB_H

#ifndef __FREEMASTER_H
#error Please include the freemaster.h master header file before the freemaster_usb.h
#endif

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0)
#define USB_INTERRUPT_HANDLER USB_DeviceEhciIsrFunction
#define USB_DATA_BUFF_SIZE    HS_CDC_VCOM_BULK_OUT_PACKET_SIZE
#define USB_MCU_INT_HANDLER   USBHS_IRQHandler
#define USB_IRQS_LIST         USBHS_IRQS
#define CONTROLLER_ID         kUSB_ControllerEhci0
#define USB_CONTROLLER_IX     0

#elif defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0)
#define USB_INTERRUPT_HANDLER USB_DeviceKhciIsrFunction
#define USB_DATA_BUFF_SIZE    FS_CDC_VCOM_BULK_OUT_PACKET_SIZE
#if defined(USBFS_IRQHandler)
    #define USB_MCU_INT_HANDLER USBFS_IRQHandler /* MCXNxxx with both KHCI and EHCI */
#else
    #define USB_MCU_INT_HANDLER USB0_IRQHandler  /* Default USB0 ISR name */
#endif
#define USB_IRQS_LIST         USB_IRQS
#define CONTROLLER_ID         kUSB_ControllerKhci0
#define USB_CONTROLLER_IX     0

#elif defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
#define USB_INTERRUPT_HANDLER USB_DeviceLpcIp3511IsrFunction
#define USB_DATA_BUFF_SIZE    FS_CDC_VCOM_BULK_OUT_PACKET_SIZE
#define USB_MCU_INT_HANDLER   USB0_IRQHandler
#define USB_IRQS_LIST         USB_IRQS
#define CONTROLLER_ID         kUSB_ControllerLpcIp3511Fs0
#define USB_CONTROLLER_IX     0

#elif defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
#define USB_INTERRUPT_HANDLER USB_DeviceLpcIp3511IsrFunction
#define USB_DATA_BUFF_SIZE    HS_CDC_VCOM_BULK_OUT_PACKET_SIZE
#define USB_MCU_INT_HANDLER   USB_IRQHandler
#define USB_MCU_INT_HANDLER_2 USB0_IRQHandler
#define USB_IRQS_LIST         USBHSD_IRQS
#define CONTROLLER_ID         kUSB_ControllerLpcIp3511Hs0
#define USB_CONTROLLER_IX     0

#else
#error USB device config not defined!
#endif

#define USB_DEVICE_INTERRUPT_PRIORITY (3U)

/******************************************************************************
 * Functions definitions
 ******************************************************************************/
FMSTR_BOOL FMSTR_ExampleUsbInit(void);

#endif /* __FREEMASTER_EXAMPLE_USB_H */
