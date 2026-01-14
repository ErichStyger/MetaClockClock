/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2018-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * FreeMASTER Communication Driver - USB Example Implementation
 */

#include "freemaster.h"
#include "freemaster_serial_usb.h"

#include <stdio.h>
#include <stdlib.h>

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_cdc_acm.h"
#include "usb_device_ch9.h"

#include "freemaster_usb_device_descriptor.h"
#include "freemaster_usb.h"

#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#include "usb_phy.h"
#endif

/* Currently configured line coding */
#define LINE_CODING_SIZE       (0x07)
#define LINE_CODING_DTERATE    (115200)
#define LINE_CODING_CHARFORMAT (0x00)
#define LINE_CODING_PARITYTYPE (0x00)
#define LINE_CODING_DATABITS   (0x08)

/* Communications feature */
#define COMM_FEATURE_DATA_SIZE (0x02)
#define STATUS_ABSTRACT_STATE  (0x0000)
#define COUNTRY_SETTING        (0x0000)

/* Notification of serial state */
#define NOTIF_PACKET_SIZE  (0x08U)
#define UART_BITMAP_SIZE   (0x02U)
#define NOTIF_REQUEST_TYPE (0xA1U)

/******************************************************************************
 * Local functions
 ******************************************************************************/

static void USB_DeviceIsrEnable(void);
static FMSTR_BOOL _FMSTR_UsbTxFunction(FMSTR_U8 *data, FMSTR_U32 size);
static usb_status_t USB_DeviceCdcVcomCallback(class_handle_t handle, uint32_t event, void *param);
static usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);

void USB_MCU_INT_HANDLER(void);

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/* Define the information relates to abstract control model */
typedef struct _usb_cdc_acm_info
{
    uint8_t serialStateBuf[NOTIF_PACKET_SIZE + UART_BITMAP_SIZE]; /* Serial state buffer of the CDC device to notify the
                                                                     serial state to host. */
    bool dtePresent;          /* A flag to indicate whether DTE is present.         */
    uint16_t breakDuration;   /* Length of time in milliseconds of the break signal */
    uint8_t dteStatus;        /* Status of data terminal equipment                  */
    uint8_t currentInterface; /* Current interface index.                           */
    uint16_t uartState;       /* UART state of the CDC device.                      */
} usb_cdc_acm_info_t;

typedef struct
{
    usb_device_handle deviceHandle; /* USB device handle. */
    class_handle_t cdcAcmHandle; /* USB CDC ACM class handle.                                                         */
    volatile uint8_t attach;     /* A flag to indicate whether a usb device is attached. 1: attached, 0: not attached */
    uint8_t speed;               /* Speed of USB device. USB_SPEED_FULL/USB_SPEED_LOW/USB_SPEED_HIGH.                 */
    uint8_t currentConfiguration;                                           /* Current configuration value. */
    uint8_t currentInterfaceAlternateSetting[USB_CDC_VCOM_INTERFACE_COUNT]; /* Current alternate setting value for each
                                                                               interface. */
    volatile uint8_t
        startTransactions; /* A flag to indicate whether a CDC device is ready to transmit and receive data.    */

} FMSTR_EXAMPLE_USB_CTX;

/******************************************************************************
 * Local variables
 ******************************************************************************/

/* Line coding of cdc device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t s_lineCoding[LINE_CODING_SIZE] = {
    /* E.g. 0x00,0xC2,0x01,0x00 : 0x0001C200 is 115200 bits per second */
    (LINE_CODING_DTERATE >> 0U) & 0x000000FFU,
    (LINE_CODING_DTERATE >> 8U) & 0x000000FFU,
    (LINE_CODING_DTERATE >> 16U) & 0x000000FFU,
    (LINE_CODING_DTERATE >> 24U) & 0x000000FFU,
    LINE_CODING_CHARFORMAT,
    LINE_CODING_PARITYTYPE,
    LINE_CODING_DATABITS};

/* Abstract state of cdc device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t s_abstractState[COMM_FEATURE_DATA_SIZE] = {
  (STATUS_ABSTRACT_STATE >> 0U) & 0x00FFU,
  (STATUS_ABSTRACT_STATE >> 8U) & 0x00FFU
};

/* Country code of cdc device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t s_countryCode[COMM_FEATURE_DATA_SIZE] = {
  (COUNTRY_SETTING >> 0U) & 0x00FFU,
  (COUNTRY_SETTING >> 8U) & 0x00FFU
};

/* Class contxt information */
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) 
static usb_cdc_acm_info_t s_usbCdcAcmInfo;

/* Shared Rx/Tx buffer used to exchange data with FreeMASTER driver */
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) 
static uint8_t s_usbRxTxBuff[USB_DATA_BUFF_SIZE];

extern usb_device_endpoint_struct_t g_UsbDeviceCdcVcomDicEndpoints[];
extern usb_device_class_struct_t g_UsbDeviceCdcVcomConfig;

/* USB device class information */
static usb_device_class_config_struct_t s_cdcAcmConfig[1] = {{
    USB_DeviceCdcVcomCallback,
    0,
    &g_UsbDeviceCdcVcomConfig,
}};

/* USB device class configuration information */
static usb_device_class_config_list_struct_t s_cdcAcmConfigList = {
    s_cdcAcmConfig,
    USB_DeviceCallback,
    1,
};

/* context data that this application needs to handle USB communication */
static FMSTR_EXAMPLE_USB_CTX _fmstr_exampleUsbCtx;

/******************************************************************************
 * API functions
 ******************************************************************************/

/* Initializiation of the USB stack */
FMSTR_BOOL FMSTR_ExampleUsbInit(void)
{
    (void)memset(&_fmstr_exampleUsbCtx, 0, sizeof(_fmstr_exampleUsbCtx));

#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    _fmstr_exampleUsbCtx.speed        = USB_SPEED_FULL;
    _fmstr_exampleUsbCtx.attach       = 0;
    _fmstr_exampleUsbCtx.cdcAcmHandle = (class_handle_t)NULL;
    _fmstr_exampleUsbCtx.deviceHandle = NULL;

    if (USB_DeviceClassInit((uint8_t)CONTROLLER_ID, &s_cdcAcmConfigList, 
                            &_fmstr_exampleUsbCtx.deviceHandle) != kStatus_USB_Success)
    {
        return FMSTR_FALSE;
    }
    else
    {
        _fmstr_exampleUsbCtx.cdcAcmHandle = s_cdcAcmConfigList.config->classHandle;
    }

    USB_DeviceIsrEnable();

    (void)USB_DeviceRun(_fmstr_exampleUsbCtx.deviceHandle);

    FMSTR_SerialUsbRegisterAppFunctions(_FMSTR_UsbTxFunction);

    return FMSTR_TRUE;
}

/******************************************************************************
 * Local functions
 ******************************************************************************/

/* This function is called from FreeMASTER, when any data to send. */
static FMSTR_BOOL _FMSTR_UsbTxFunction(FMSTR_U8 *data, FMSTR_U32 size)
{
    usb_status_t error = kStatus_USB_Error;

    /* Check if sane data used */
    if (data == NULL || size == 0U || size > (FMSTR_U32)USB_DATA_BUFF_SIZE)
    {
        return FMSTR_FALSE;
    }
    /* Check if USB attached */
    if (_fmstr_exampleUsbCtx.attach != 1U)
    {
        return FMSTR_FALSE;
    }

    /* We use the shared rx/tx memory to make sure the cached/uncached memory is used.
       FreeMASTER uses half-duplex communication anyway. */
    (void)memcpy(s_usbRxTxBuff, data, size);

    /* Send data via USB CDC */
    error = USB_DeviceCdcAcmSend(_fmstr_exampleUsbCtx.cdcAcmHandle, USB_CDC_VCOM_BULK_IN_ENDPOINT, s_usbRxTxBuff, size);
    return error == kStatus_USB_Success ? FMSTR_TRUE : FMSTR_FALSE;
}

static usb_status_t USB_DeviceCdcVcomCallback(class_handle_t handle, uint32_t event, void *param)
{
    uint32_t len;
    uint8_t *uartBitmap;
    usb_device_cdc_acm_request_param_struct_t *acmReqParam;
    usb_device_endpoint_callback_message_struct_t *epCbParam;
    
    usb_status_t error = kStatus_USB_InvalidRequest;
    usb_cdc_acm_info_t *acmInfo = &s_usbCdcAcmInfo;
    
    acmReqParam = (usb_device_cdc_acm_request_param_struct_t *)param;
    epCbParam   = (usb_device_endpoint_callback_message_struct_t *)param;
    
    /* Coverity: Intentional cast to enum type. */
    /* coverity[cert_int31_c_violation:FALSE] */
    switch ((usb_device_cdc_acm_event_t)event)
    {
        case kUSB_DeviceCdcEventSendResponse:
        {
            if ((epCbParam->length != 0U) && ((epCbParam->length % g_UsbDeviceCdcVcomDicEndpoints[0].maxPacketSize) == 0U))
            {
                /* If the last packet is the size of endpoint, then send also zero-ended packet,
                 ** meaning that we want to inform the host that we do not have any additional
                 ** data, so it can flush the output.
                 */
                error = USB_DeviceCdcAcmSend(handle, USB_CDC_VCOM_BULK_IN_ENDPOINT, NULL, 0);
            }
            else if ((_fmstr_exampleUsbCtx.attach == 1U))
            {
                if ((epCbParam->buffer != NULL) || ((epCbParam->buffer == NULL) && (epCbParam->length == 0U)))
                {
                    /* Schedule buffer for next receive event */
                    error = USB_DeviceCdcAcmRecv(handle, USB_CDC_VCOM_BULK_OUT_ENDPOINT, s_usbRxTxBuff,
                                                 g_UsbDeviceCdcVcomDicEndpoints[0].maxPacketSize);
                    /* FreeMASTER send complete event */
                    FMSTR_SerialUsbProcessEvent(FMSTR_SERIAL_USBCDC_EVENT_TYPE_SENT, NULL, 0);
                }
            }
            else
            {
            }
        }
        break;
        case kUSB_DeviceCdcEventRecvResponse:
        {
            if ((_fmstr_exampleUsbCtx.attach == 1U))
            {
                /* Call Freemaster interrupt with received data */
                FMSTR_SerialUsbProcessEvent(FMSTR_SERIAL_USBCDC_EVENT_TYPE_RECEIVED, (FMSTR_U8 *)s_usbRxTxBuff,
                                            epCbParam->length);
                /* Schedule buffer for next receive event */
                error = USB_DeviceCdcAcmRecv(handle, USB_CDC_VCOM_BULK_OUT_ENDPOINT, s_usbRxTxBuff,
                                             g_UsbDeviceCdcVcomDicEndpoints[0].maxPacketSize);
            }
        }
        break;
        case kUSB_DeviceCdcEventSerialStateNotif:
            ((usb_device_cdc_acm_struct_t *)handle)->hasSentState = 0;
            error                                                 = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventSendEncapsulatedCommand:
            break;
        case kUSB_DeviceCdcEventGetEncapsulatedResponse:
            break;
        case kUSB_DeviceCdcEventSetCommFeature:
            if (USB_DEVICE_CDC_FEATURE_ABSTRACT_STATE == acmReqParam->setupValue)
            {
                if (acmReqParam->isSetup == 1U)
                {
                    *(acmReqParam->buffer) = s_abstractState;
                    *(acmReqParam->length) = sizeof(s_abstractState);
                }
                else
                {
                    /* no action, data phase, s_abstractState has been assigned */
                }
                error = kStatus_USB_Success;
            }
            else if (USB_DEVICE_CDC_FEATURE_COUNTRY_SETTING == acmReqParam->setupValue)
            {
                if (acmReqParam->isSetup == 1U)
                {
                    *(acmReqParam->buffer) = s_countryCode;
                    *(acmReqParam->length) = sizeof(s_countryCode);
                }
                else
                {
                    /* no action, data phase, s_countryCode has been assigned */
                }
                error = kStatus_USB_Success;
            }
            else
            {
                /* no action, return kStatus_USB_InvalidRequest */
            }
            break;
        case kUSB_DeviceCdcEventGetCommFeature:
            if (USB_DEVICE_CDC_FEATURE_ABSTRACT_STATE == acmReqParam->setupValue)
            {
                *(acmReqParam->buffer) = s_abstractState;
                *(acmReqParam->length) = COMM_FEATURE_DATA_SIZE;
                error                  = kStatus_USB_Success;
            }
            else if (USB_DEVICE_CDC_FEATURE_COUNTRY_SETTING == acmReqParam->setupValue)
            {
                *(acmReqParam->buffer) = s_countryCode;
                *(acmReqParam->length) = COMM_FEATURE_DATA_SIZE;
                error                  = kStatus_USB_Success;
            }
            else
            {
                /* no action, return kStatus_USB_InvalidRequest */
            }
            break;
        case kUSB_DeviceCdcEventClearCommFeature:
            break;
        case kUSB_DeviceCdcEventGetLineCoding:
            *(acmReqParam->buffer) = s_lineCoding;
            *(acmReqParam->length) = LINE_CODING_SIZE;
            error                  = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventSetLineCoding:
        {
            if (acmReqParam->isSetup == 1U)
            {
                *(acmReqParam->buffer) = s_lineCoding;
                *(acmReqParam->length) = sizeof(s_lineCoding);
            }
            else
            {
                /* no action, data phase, s_lineCoding has been assigned */
            }
            error = kStatus_USB_Success;
        }
        break;
        case kUSB_DeviceCdcEventSetControlLineState:
        {
            /* Sanity check parameter data */
            if (acmReqParam->interfaceIndex > (uint16_t)UINT8_MAX || acmReqParam->setupValue > (uint16_t)UINT8_MAX)
            {
                /* The values can not be greater than one byte. */
                error = kStatus_USB_Error;
            }
            else
            {
                s_usbCdcAcmInfo.dteStatus = (uint8_t) acmReqParam->setupValue;
                /* activate/deactivate Tx carrier */
                if ((acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_CARRIER_ACTIVATION) != 0U)
                {
                    acmInfo->uartState |= USB_DEVICE_CDC_UART_STATE_TX_CARRIER;
                }
                else
                {
                    /* Coverity: Intentional bit-clear operation on the U16 type. */
                    /* coverity[misra_c_2012_rule_10_8_violation:FALSE] */
                    /* coverity[cert_int31_c_violation:FALSE] */
                    acmInfo->uartState &= (uint16_t)~USB_DEVICE_CDC_UART_STATE_TX_CARRIER;
                }

                /* Activate carrier and DTE. Com port of terminal tool running on PC is open now */
                if ((acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE) != 0U)
                {
                    acmInfo->uartState |= USB_DEVICE_CDC_UART_STATE_RX_CARRIER;
                }
                /* Com port of terminal tool running on PC is closed now */
                else
                {
                    /* Coverity: Intentional bit-clear operation on the U16 type. */
                    /* coverity[misra_c_2012_rule_10_8_violation:FALSE] */
                    /* coverity[cert_int31_c_violation:FALSE] */
                    acmInfo->uartState &= (uint16_t)~USB_DEVICE_CDC_UART_STATE_RX_CARRIER;
                }

                /* Indicates to DCE if DTE is present or not */
                acmInfo->dtePresent = (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE) != 0U ? true : false;

                /* Initialize the serial state buffer */
                acmInfo->serialStateBuf[0] = NOTIF_REQUEST_TYPE;                /* bmRequestType */
                acmInfo->serialStateBuf[1] = USB_DEVICE_CDC_NOTIF_SERIAL_STATE; /* bNotification */
                acmInfo->serialStateBuf[2] = 0x00;                              /* wValue */
                acmInfo->serialStateBuf[3] = 0x00;
                acmInfo->serialStateBuf[4] = 0x00; /* wIndex */
                acmInfo->serialStateBuf[5] = 0x00;
                acmInfo->serialStateBuf[6] = UART_BITMAP_SIZE; /* wLength */
                acmInfo->serialStateBuf[7] = 0x00;

                /* Notify to host the line state */
                acmInfo->serialStateBuf[4] = (uint8_t)acmReqParam->interfaceIndex;

                /* Lower byte of UART BITMAP */
                uartBitmap    = (uint8_t *)&acmInfo->serialStateBuf[NOTIF_PACKET_SIZE + UART_BITMAP_SIZE - 2U];
                uartBitmap[0] = (uint8_t)(acmInfo->uartState & 0xFFu);
                uartBitmap[1] = (uint8_t)((acmInfo->uartState >> 8) & 0xFFu);

                /* coverity[misra_c_2012_rule_10_8_violation:FALSE] */
                len = (uint32_t)(NOTIF_PACKET_SIZE + UART_BITMAP_SIZE);
                
                if (((usb_device_cdc_acm_struct_t *)handle)->hasSentState == 0U)
                {
                    error = USB_DeviceCdcAcmSend(handle, USB_CDC_VCOM_INTERRUPT_IN_ENDPOINT, acmInfo->serialStateBuf, len);
                    if (kStatus_USB_Success != error)
                    {
                        (void)usb_echo("kUSB_DeviceCdcEventSetControlLineState error!");
                    }
                    ((usb_device_cdc_acm_struct_t *)handle)->hasSentState = 1;
                }
                else
                {
                    error = kStatus_USB_Success;
                }

                /* Update status */
                if ((acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_CARRIER_ACTIVATION) != 0U)
                {
                    /* CARRIER_ACTIVATED */
                }
                else
                {
                    /* CARRIER_DEACTIVATED */
                }
                if ((acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE) != 0U)
                {
                    /* DTE_ACTIVATED */
                    if (_fmstr_exampleUsbCtx.attach == 1U)
                    {
                        _fmstr_exampleUsbCtx.startTransactions = 1;
#if defined(FSL_FEATURE_USB_KHCI_KEEP_ALIVE_ENABLED) && (FSL_FEATURE_USB_KHCI_KEEP_ALIVE_ENABLED > 0U) && \
    defined(USB_DEVICE_CONFIG_KEEP_ALIVE_MODE) && (USB_DEVICE_CONFIG_KEEP_ALIVE_MODE > 0U) &&             \
    defined(FSL_FEATURE_USB_KHCI_USB_RAM) && (FSL_FEATURE_USB_KHCI_USB_RAM > 0U)
                        s_waitForDataReceive = 1;
                        USB0->INTEN &= ~USB_INTEN_SOFTOKEN_MASK;
                        s_comOpen = 1;
                        (void)usb_echo("USB_APP_CDC_DTE_ACTIVATED\r\n");
#endif
                    }
                }
                else
                {
                    /* DTE_DEACTIVATED */
                    if (_fmstr_exampleUsbCtx.attach == 1U)
                    {
                        _fmstr_exampleUsbCtx.startTransactions = 0;
                    }
                }
            }
        }
        break;
        case kUSB_DeviceCdcEventSendBreak:
            break;
        default:
            /* Ignore unknown events */
            break;
    }

    if (error != kStatus_USB_Success && error != kStatus_USB_Busy)
    {
        (void)usb_echo("USB Error: %d, event: %d.\n", error, event);
    }
    return error;
}

static usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint16_t *temp16   = (uint16_t *)param;
    uint8_t *temp8     = (uint8_t *)param;

    /* Coverity: Intentional cast to enum type. */
    /* coverity[cert_int31_c_violation:FALSE] */
    switch ((usb_device_event_t)event)
    {
        case kUSB_DeviceEventBusReset:
        {
            _fmstr_exampleUsbCtx.attach = 0U;
            _fmstr_exampleUsbCtx.currentConfiguration = 0U;
            error = kStatus_USB_Success;
            
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed((uint8_t)CONTROLLER_ID, &_fmstr_exampleUsbCtx.speed))
            {
                (void)USB_DeviceSetSpeed(handle, _fmstr_exampleUsbCtx.speed);
            }
#endif
        }
        break;
        case kUSB_DeviceEventSetConfiguration:
            if ((*temp8) == 0U)
            {
                _fmstr_exampleUsbCtx.attach = 0U;
                _fmstr_exampleUsbCtx.currentConfiguration = 0U;
                error = kStatus_USB_Success;
            }
            else if ((*temp8) == (uint8_t)USB_CDC_VCOM_CONFIGURE_INDEX)
            {
                _fmstr_exampleUsbCtx.attach = 1U;
                _fmstr_exampleUsbCtx.currentConfiguration = *temp8;
                error = kStatus_USB_Success;
                
                /* Schedule buffer for receive */
                (void)USB_DeviceCdcAcmRecv(_fmstr_exampleUsbCtx.cdcAcmHandle, USB_CDC_VCOM_BULK_OUT_ENDPOINT, s_usbRxTxBuff,
                                     g_UsbDeviceCdcVcomDicEndpoints[0].maxPacketSize);
            }
            else
            {
                /* no action, return kStatus_USB_InvalidRequest */
            }
            break;
        case kUSB_DeviceEventSetInterface:
            if (_fmstr_exampleUsbCtx.attach == 1U)
            {
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);
                
                if (interface < (uint8_t)USB_CDC_VCOM_INTERFACE_COUNT)
                {
                    _fmstr_exampleUsbCtx.currentInterfaceAlternateSetting[interface] = alternateSetting;
                    error = kStatus_USB_Success;
                }
            }
            break;
        case kUSB_DeviceEventGetConfiguration:
            break;
        case kUSB_DeviceEventGetInterface:
            break;
        case kUSB_DeviceEventGetDeviceDescriptor:
            if (param != NULL)
            {
                error = USB_DeviceGetDeviceDescriptor(handle, (usb_device_get_device_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetConfigurationDescriptor:
            if (param != NULL)
            {
                error = USB_DeviceGetConfigurationDescriptor(handle, (usb_device_get_configuration_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetStringDescriptor:
            if (param != NULL)
            {
                /* Get device string descriptor request */
                error = USB_DeviceGetStringDescriptor(handle, (usb_device_get_string_descriptor_struct_t *)param);
            }
            break;
        default:
            /* Ignore unknown events */
            break;
    }

    return error;
}

void USB_MCU_INT_HANDLER(void)
{
    USB_INTERRUPT_HANDLER(_fmstr_exampleUsbCtx.deviceHandle);

#ifdef SDK_ISR_EXIT_BARRIER
    /* May be needed for ARM errata 838869 */
    SDK_ISR_EXIT_BARRIER;
#endif
}

#ifdef USB_MCU_INT_HANDLER_2
void USB_MCU_INT_HANDLER_2(void)
{
    USB_INTERRUPT_HANDLER(_fmstr_exampleUsbCtx.deviceHandle);

#ifdef SDK_ISR_EXIT_BARRIER
    /* May be needed for ARM errata 838869 */
    SDK_ISR_EXIT_BARRIER;
#endif
}
#endif

/* Most of SDK-supported parts are CortexM devices with NVIC controller.
 * All other platforms (e.g. DSC) enable the USB interrupt in the main application code. */
#ifdef __NVIC_PRIO_BITS
static void USB_DeviceIsrEnable(void)
{
    IRQn_Type usbDeviceIrq[] = USB_IRQS_LIST;
    IRQn_Type irqNumber = usbDeviceIrq[USB_CONTROLLER_IX];

    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority(irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    (void)EnableIRQ(irqNumber);
}
#endif
