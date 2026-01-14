/**
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_CDC_ECM_H__
#define __USB_DEVICE_CDC_ECM_H__

/*******************************************************************************
 * Includes
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define USB_DEVICE_CDC_COMM_CLASS_CODE (0x02U)
#define USB_DEVICE_CDC_DATA_CLASS_CODE (0x0AU)
#define USB_DEVICE_CDC_ECM_CLASS_CODE (0x02U)
#define USB_DEVICE_CDC_ECM_SUBCLASS_CODE (0x06U)
#define USB_DEVICE_CDC_ECM_PROTOCOL_CODE (0x00U)

#define USB_DEVICE_CDC_NETWORK_CONNECTION (0x00U)
#define USB_DEVICE_CDC_CONNECTION_SPEED_CHANGE (0x2AU)
#define USB_DEVICE_CDC_ECM_SET_ETHERNET_MULTICAST_FILTER (0x40U)
#define USB_DEVICE_CDC_ECM_SET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER (0x41U)
#define USB_DEVICE_CDC_ECM_GET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER (0x42U)
#define USB_DEVICE_CDC_ECM_SET_ETHERNET_PACKET_FILTER (0x43U)
#define USB_DEVICE_CDC_ECM_GET_ETHERNET_STATISTIC (0x44U)

#define USB_DEVICE_CDC_ECM_PACKET_TYPE_MULTICAST_MASK (1U << 4)
#define USB_DEVICE_CDC_ECM_PACKET_TYPE_BROADCAST_MASK (1U << 3)
#define USB_DEVICE_CDC_ECM_PACKET_TYPE_DIRECTED_MASK (1U << 2)
#define USB_DEVICE_CDC_ECM_PACKET_TYPE_ALL_MULTICAST_MASK (1U << 1)
#define USB_DEVICE_CDC_ECM_PACKET_TYPE_PROMISCUOUS_MASK (1U << 0)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
typedef struct _usb_device_cdc_ecm_pipe
{
    uint8_t ep;
    uint8_t isBusy;
    uint8_t *pipeDataBuffer;
    uint32_t pipeDataLen;
    uint8_t pipeStall;
} usb_device_cdc_ecm_pipe_t;

typedef struct _usb_device_cdc_ecm_struct
{
    usb_device_handle handle;
    usb_device_class_config_struct_t *config;
    usb_device_interface_struct_t *commInterfaceHandle;
    usb_device_interface_struct_t *dataInterfaceHandle;
    usb_device_cdc_ecm_pipe_t bulkIn;
    usb_device_cdc_ecm_pipe_t bulkOut;
    usb_device_cdc_ecm_pipe_t interruptIn;
    uint8_t configurationValue;
    uint8_t interfaceNumber;
    uint8_t alternate;
} usb_device_cdc_ecm_struct_t;

typedef enum _usb_device_cdc_ecm_event
{
    kUSB_DeviceCdcEcmEventSendResponse,
    kUSB_DeviceCdcEcmEventRecvResponse,
    kUSB_DeviceCdcEcmEventNotifyResponse,
    kUSB_DeviceCdcEcmEventSetEthernetMulticastFilters,
    kUSB_DeviceCdcEcmEventSetEthernetPowerManagementPatternFilter,
    kUSB_DeviceCdcEcmEventGetEthernetPowerManagementPatternFilter,
    kUSB_DeviceCdcEcmEventSetEthernetPacketFilter,
    kUSB_DeviceCdcEcmEventGetEthernetStatistic,
} usb_device_cdc_ecm_event_t;

usb_status_t USB_DeviceCdcEcmInit(uint8_t controllerId, usb_device_class_config_struct_t *config, class_handle_t *handle);

usb_status_t USB_DeviceCdcEcmDeinit(class_handle_t handle);

usb_status_t USB_DeviceCdcEcmSend(class_handle_t handle, uint8_t ep, uint8_t *buffer, uint32_t length);

usb_status_t USB_DeviceCdcEcmRecv(class_handle_t handle, uint8_t ep, uint8_t *buffer, uint32_t length);

usb_status_t USB_DeviceCdcEcmEvent(void *handle, uint32_t event, void *param);
#endif
