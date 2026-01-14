/**
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "usb_host_config.h"

#if defined(USB_HOST_CONFIG_CDC_ECM) && USB_HOST_CONFIG_CDC_ECM
#include "usb_host.h"
#include "usb_host_cdc.h"
#include "usb_host_cdc_ecm.h"
#include "usb_host_devices.h"
#include "usb_host_framework.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void USB_HostCdcEcmGetMacStringDescriptorPipeCallback(void *param,
                                                             usb_host_transfer_t *transfer,
                                                             usb_status_t status);
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
usb_status_t USB_HostCdcEcmSetEthernetMulticastFilters(usb_host_class_handle classHandle,
                                                       uint16_t filterNumSum,
                                                       uint8_t (*filterData)[6],
                                                       transfer_callback_t callbackFn,
                                                       void *callbackParam)
{
    return USB_HostCdcControl(
        classHandle, USB_REQUEST_TYPE_DIR_OUT | USB_REQUEST_TYPE_TYPE_CLASS | USB_REQUEST_TYPE_RECIPIENT_INTERFACE,
        CDC_ECM_SET_ETHERNET_MULTICAST_FILTER, USB_SHORT_GET_LOW(filterNumSum), USB_SHORT_GET_HIGH(filterNumSum),
        filterNumSum * 6, &filterData[0][0], callbackFn, callbackParam);
}

usb_status_t USB_HostCdcEcmSetEthernetPowerManagementPatternFilter(usb_host_class_handle classHandle,
                                                                   uint16_t filterNum,
                                                                   uint8_t *buffer,
                                                                   uint16_t bufferLength,
                                                                   transfer_callback_t callbackFn,
                                                                   void *callbackParam)
{
    return USB_HostCdcControl(
        classHandle, USB_REQUEST_TYPE_DIR_OUT | USB_REQUEST_TYPE_TYPE_CLASS | USB_REQUEST_TYPE_RECIPIENT_INTERFACE,
        CDC_ECM_SET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER, USB_SHORT_GET_LOW(filterNum),
        USB_SHORT_GET_HIGH(filterNum), bufferLength, buffer, callbackFn, callbackParam);
}

usb_status_t USB_HostCdcEcmGetEthernetPowerManagementPatternFilter(usb_host_class_handle classHandle,
                                                                   uint16_t filterNum,
                                                                   uint8_t (*buffer)[2],
                                                                   transfer_callback_t callbackFn,
                                                                   void *callbackParam)
{
    return USB_HostCdcControl(
        classHandle, USB_REQUEST_TYPE_DIR_IN | USB_REQUEST_TYPE_TYPE_CLASS | USB_REQUEST_TYPE_RECIPIENT_INTERFACE,
        CDC_ECM_GET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER, USB_SHORT_GET_LOW(filterNum),
        USB_SHORT_GET_HIGH(filterNum), 2, &buffer[0][0], callbackFn, callbackParam);
}

usb_status_t USB_HostCdcEcmSetEthernetPacketFilter(usb_host_class_handle classHandle,
                                                   uint16_t packetFilterBitmap,
                                                   transfer_callback_t callbackFn,
                                                   void *callbackParam)
{
    return USB_HostCdcControl(
        classHandle, USB_REQUEST_TYPE_DIR_OUT | USB_REQUEST_TYPE_TYPE_CLASS | USB_REQUEST_TYPE_RECIPIENT_INTERFACE,
        CDC_ECM_SET_ETHERNET_PACKET_FILTER, USB_SHORT_GET_LOW(packetFilterBitmap),
        USB_SHORT_GET_HIGH(packetFilterBitmap), 0, NULL, callbackFn, callbackParam);
}

usb_status_t USB_HostCdcEcmGetEthernetStatistic(usb_host_class_handle classHandle,
                                                uint16_t featureSelector,
                                                uint8_t (*buffer)[4],
                                                transfer_callback_t callbackFn,
                                                void *callbackParam)
{
    return USB_HostCdcControl(
        classHandle, USB_REQUEST_TYPE_DIR_IN | USB_REQUEST_TYPE_TYPE_CLASS | USB_REQUEST_TYPE_RECIPIENT_INTERFACE,
        CDC_ECM_GET_ETHERNET_STATISTIC, USB_SHORT_GET_LOW(featureSelector), USB_SHORT_GET_HIGH(featureSelector), 4,
        &buffer[0][0], callbackFn, callbackParam);
}

usb_status_t USB_HostCdcEcmDataRecv(usb_host_class_handle classHandle,
                                    uint8_t *buffer,
                                    uint32_t bufferLength,
                                    transfer_callback_t callbackFn,
                                    void *callbackParam)
{
    return USB_HostCdcDataRecv(classHandle, buffer, bufferLength, callbackFn, callbackParam);
}

usb_status_t USB_HostCdcEcmDataSend(usb_host_class_handle classHandle,
                                    uint8_t *buffer,
                                    uint32_t bufferLength,
                                    uint16_t maxSegmentSize,
                                    transfer_callback_t callbackFn,
                                    void *callbackParam)
{
    return USB_HostCdcDataSend(classHandle, buffer, MIN(bufferLength, maxSegmentSize), callbackFn, callbackParam);
}

usb_status_t USB_HostCdcEcmGetMacStringDescriptor(usb_host_class_handle classHandle,
                                                  uint16_t index,
                                                  uint16_t langID,
                                                  uint8_t *buffer,
                                                  uint16_t bufferLength,
                                                  transfer_callback_t callbackFn,
                                                  void *callbackParam)
{
    usb_status_t status                            = kStatus_USB_Success;
    usb_host_cdc_instance_struct_t *cdcEcmInstance = (usb_host_cdc_instance_struct_t *)classHandle;
    usb_host_device_instance_t *deviceInstance     = cdcEcmInstance->deviceHandle;
    usb_host_transfer_t *transfer;
    usb_host_process_descriptor_param_t descriptorParam;

    cdcEcmInstance->controlCallbackFn    = callbackFn;
    cdcEcmInstance->controlCallbackParam = callbackParam;

    status = USB_HostMallocTransfer(deviceInstance->hostHandle, &transfer);
    if (status != kStatus_USB_Success)
    {
        usb_echo("Malloc get string descriptor transfer error.\r\n");
        return status;
    }

    transfer->setupPacket->bmRequestType = USB_REQUEST_TYPE_DIR_IN;
    transfer->setupPacket->bRequest      = USB_REQUEST_STANDARD_GET_DESCRIPTOR;
    transfer->callbackFn                 = USB_HostCdcEcmGetMacStringDescriptorPipeCallback;
    transfer->callbackParam              = cdcEcmInstance;

    descriptorParam.descriptorBuffer = buffer;
    descriptorParam.descriptorType   = USB_DESCRIPTOR_TYPE_STRING;
    descriptorParam.descriptorIndex  = index;
    descriptorParam.descriptorLength = bufferLength;
    descriptorParam.languageId       = langID;

    return USB_HostStandardSetGetDescriptor(deviceInstance, transfer, &descriptorParam);
}

static void USB_HostCdcEcmGetMacStringDescriptorPipeCallback(void *param,
                                                             usb_host_transfer_t *transfer,
                                                             usb_status_t status)
{
    usb_host_cdc_instance_struct_t *cdcEcmInstance = (usb_host_cdc_instance_struct_t *)param;
    if (cdcEcmInstance->controlCallbackFn != NULL)
    {
        cdcEcmInstance->controlCallbackFn(cdcEcmInstance->controlCallbackParam, transfer->transferBuffer,
                                          transfer->transferSofar, status);
    }
    (void)USB_HostFreeTransfer(cdcEcmInstance->hostHandle, transfer);

    return;
}

uint32_t USB_HostCdcEcmPowerManagementPatternFilterData(USB_HostCdcEcmPowerManagementPatternFilter_t *param,
                                                        uint8_t *dataBuf)
{
    uint8_t *p            = dataBuf;
    volatile uint32_t len = 0;
    *p++                  = param->MaskSize & 0xFF;
    *p++                  = (param->MaskSize >> 8) & 0xFF;
    len += 2;

    for (int i = 0; i < param->MaskSize; i++)
    {
        *p++ = param->Mask[i];
        len++;
    }
    for (int i = 0; i < strlen((char *)param->Pattern); i++)
    {
        *p++ = param->Pattern[i];
        len++;
    }

    return len;
}
#endif
