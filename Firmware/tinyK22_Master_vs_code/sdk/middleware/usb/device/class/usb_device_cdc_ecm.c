/**
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "usb_device_config.h"

#if defined(USB_DEVICE_CONFIG_CDC_ECM) && USB_DEVICE_CONFIG_CDC_ECM
#include "usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_cdc_ecm.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_cdc_ecm_struct_t s_cdcEcmHandle[USB_DEVICE_CONFIG_CDC_ECM];

/*******************************************************************************
 * Code
 ******************************************************************************/
static usb_status_t USB_DeviceCdcEcmAllocateHandle(usb_device_cdc_ecm_struct_t **handle)
{
    for (uint32_t cnt = 0U; cnt < (uint32_t)USB_DEVICE_CONFIG_CDC_ECM; cnt++)
    {
        if (NULL == s_cdcEcmHandle[cnt].handle)
        {
            *handle = &s_cdcEcmHandle[cnt];
            return kStatus_USB_Success;
        }
    }

    return kStatus_USB_AllocFail;
}

static void USB_DeviceCdcEcmFreeHandle(usb_device_cdc_ecm_struct_t *handle)
{
    handle->handle = NULL;
    handle->config = NULL;
    handle->alternate = 0U;
    handle->configurationValue = 0U;
}

static usb_status_t USB_DeviceCdcEcmBulkIn(usb_device_handle handle, usb_device_endpoint_callback_message_struct_t *message, void *callbackParam)
{
    usb_device_cdc_ecm_struct_t *cdcEcmHandle = (usb_device_cdc_ecm_struct_t *)callbackParam;
    usb_status_t status = kStatus_USB_Error;

    if (NULL == cdcEcmHandle)
    {
        return kStatus_USB_InvalidHandle;
    }

    if ((NULL != cdcEcmHandle->config) && (NULL != cdcEcmHandle->config->classCallback))
    {
        /* classCallback is initialized in classInit of s_UsbDeviceClassInterfaceMap, it is from the second parameter of classInit */
        status = cdcEcmHandle->config->classCallback((class_handle_t)cdcEcmHandle, kUSB_DeviceCdcEcmEventSendResponse, message);
    }

    cdcEcmHandle->bulkIn.isBusy = 0U;

    return status;
}

static usb_status_t USB_DeviceCdcEcmBulkOut(usb_device_handle handle, usb_device_endpoint_callback_message_struct_t *message, void *callbackParam)
{
    usb_device_cdc_ecm_struct_t *cdcEcmHandle = (usb_device_cdc_ecm_struct_t *)callbackParam;
    usb_status_t status = kStatus_USB_Error;

    if (NULL == cdcEcmHandle)
    {
        return kStatus_USB_InvalidHandle;
    }

    if ((NULL != cdcEcmHandle->config) && (NULL != cdcEcmHandle->config->classCallback))
    {
        /* classCallback is initialized in classInit of s_UsbDeviceClassInterfaceMap, it is from the second parameter of classInit */
        status = cdcEcmHandle->config->classCallback((class_handle_t)cdcEcmHandle, kUSB_DeviceCdcEcmEventRecvResponse, message);
    }

    cdcEcmHandle->bulkOut.isBusy = 0U;

    return status;
}

static usb_status_t USB_DeviceCdcEcmInterruptIn(usb_device_handle handle, usb_device_endpoint_callback_message_struct_t *message, void *callbackParam)
{
    usb_device_cdc_ecm_struct_t *cdcEcmHandle = (usb_device_cdc_ecm_struct_t *)callbackParam;
    usb_status_t error = kStatus_USB_Error;

    if (NULL == cdcEcmHandle)
    {
        return kStatus_USB_InvalidHandle;
    }

    if ((NULL != cdcEcmHandle->config) && (NULL != cdcEcmHandle->config->classCallback))
    {
        /* classCallback is initialized in classInit of s_UsbDeviceClassInterfaceMap, it is from the second parameter of classInit */
        error = cdcEcmHandle->config->classCallback((class_handle_t)cdcEcmHandle, kUSB_DeviceCdcEcmEventNotifyResponse, message);
    }

    cdcEcmHandle->interruptIn.isBusy = 0U;

    return error;
}

static usb_status_t USB_DeviceCdcEcmEndpointsInit(usb_device_cdc_ecm_struct_t *cdcEcmHandle)
{
    usb_status_t status = kStatus_USB_Error;
    usb_device_interface_list_t *interfaceList;
    usb_device_interface_struct_t *interface = NULL;
    usb_device_endpoint_callback_struct_t epCallback;

    if (NULL == cdcEcmHandle)
    {
        return kStatus_USB_InvalidHandle;
    }

    epCallback.callbackFn = NULL;

    /* return error when configuration is invalid (0 or more than the configuration number) */
    if ((cdcEcmHandle->configurationValue == 0U) || (cdcEcmHandle->configurationValue > cdcEcmHandle->config->classInfomation->configurations))
    {
        return status;
    }

    interfaceList = &cdcEcmHandle->config->classInfomation->interfaceList[cdcEcmHandle->configurationValue - 1U];

    for (uint32_t cnt = 0U; cnt < interfaceList->count; cnt++)
    {
        if (USB_DEVICE_CDC_COMM_CLASS_CODE == interfaceList->interfaces[cnt].classCode)
        {
            for (uint32_t idx = 0U; idx < interfaceList->interfaces[cnt].count; idx++)
            {
                if (interfaceList->interfaces[cnt].interface[idx].alternateSetting == cdcEcmHandle->alternate)
                {
                    interface = &interfaceList->interfaces[cnt].interface[idx];
                    break;
                }
            }
            cdcEcmHandle->interfaceNumber = interfaceList->interfaces[cnt].interfaceNumber;
            break;
        }
    }

    if (NULL == interface)
    {
        return status;
    }

    cdcEcmHandle->commInterfaceHandle = interface;

    for (uint32_t cnt = 0U; cnt < interface->endpointList.count; cnt++)
    {
        usb_device_endpoint_init_struct_t epInitStruct;
        epInitStruct.zlt = 0U;
        epInitStruct.interval = interface->endpointList.endpoint[cnt].interval;
        epInitStruct.endpointAddress = interface->endpointList.endpoint[cnt].endpointAddress;
        epInitStruct.maxPacketSize = interface->endpointList.endpoint[cnt].maxPacketSize;
        epInitStruct.transferType = interface->endpointList.endpoint[cnt].transferType;

        if ((USB_IN == ((epInitStruct.endpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >> USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT)) && (USB_ENDPOINT_INTERRUPT == epInitStruct.transferType))
        {
            cdcEcmHandle->interruptIn.ep = (epInitStruct.endpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_NUMBER_MASK);
            cdcEcmHandle->interruptIn.pipeDataBuffer = (uint8_t *)USB_INVALID_TRANSFER_BUFFER;
            cdcEcmHandle->interruptIn.pipeStall = 0U;
            cdcEcmHandle->interruptIn.pipeDataLen = 0U;
            epCallback.callbackFn = USB_DeviceCdcEcmInterruptIn;
            epCallback.callbackParam = cdcEcmHandle;

            status = USB_DeviceInitEndpoint(cdcEcmHandle->handle, &epInitStruct, &epCallback);
            if (kStatus_USB_Success != status)
            {
                return status;
            }
        }
    }

    interface = NULL;

    for (uint32_t cnt = 0U; cnt < interfaceList->count; cnt++)
    {
        if (USB_DEVICE_CDC_DATA_CLASS_CODE == interfaceList->interfaces[cnt].classCode)
        {
            for (uint32_t idx = 0U; idx < interfaceList->interfaces[cnt].count; idx++)
            {
                if (interfaceList->interfaces[cnt].interface[idx].alternateSetting == cdcEcmHandle->alternate)
                {
                    interface = &interfaceList->interfaces[cnt].interface[idx];
                    break;
                }
            }
            break;
        }
    }

    if (NULL == interface)
    {
        return status;
    }

    cdcEcmHandle->dataInterfaceHandle = interface;

    for (uint32_t cnt = 0U; cnt < interface->endpointList.count; cnt++)
    {
        usb_device_endpoint_init_struct_t epInitStruct;
        epInitStruct.zlt = 0U;
        epInitStruct.interval = interface->endpointList.endpoint[cnt].interval;
        epInitStruct.endpointAddress = interface->endpointList.endpoint[cnt].endpointAddress;
        epInitStruct.maxPacketSize = interface->endpointList.endpoint[cnt].maxPacketSize;
        epInitStruct.transferType = interface->endpointList.endpoint[cnt].transferType;

        if ((USB_IN == ((epInitStruct.endpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >> USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT)) &&
            (USB_ENDPOINT_BULK == epInitStruct.transferType))
        {
            cdcEcmHandle->bulkIn.ep = (epInitStruct.endpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_NUMBER_MASK);
            cdcEcmHandle->bulkIn.isBusy = 0U;
            cdcEcmHandle->bulkIn.pipeDataBuffer = (uint8_t *)USB_INVALID_TRANSFER_BUFFER;
            cdcEcmHandle->bulkIn.pipeStall = 0U;
            cdcEcmHandle->bulkIn.pipeDataLen = 0U;
            epCallback.callbackFn = USB_DeviceCdcEcmBulkIn;
            epCallback.callbackParam = cdcEcmHandle;

            status = USB_DeviceInitEndpoint(cdcEcmHandle->handle, &epInitStruct, &epCallback);
            if (kStatus_USB_Success != status)
            {
                return status;
            }
        }
        else if ((USB_OUT == ((epInitStruct.endpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >> USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT)) &&
                 (USB_ENDPOINT_BULK == epInitStruct.transferType))
        {
            cdcEcmHandle->bulkOut.ep = (epInitStruct.endpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_NUMBER_MASK);
            cdcEcmHandle->bulkOut.isBusy = 0U;
            cdcEcmHandle->bulkOut.pipeDataBuffer = (uint8_t *)USB_INVALID_TRANSFER_BUFFER;
            cdcEcmHandle->bulkOut.pipeStall = 0U;
            cdcEcmHandle->bulkOut.pipeDataLen = 0U;
            epCallback.callbackFn = USB_DeviceCdcEcmBulkOut;
            epCallback.callbackParam = cdcEcmHandle;

            status = USB_DeviceInitEndpoint(cdcEcmHandle->handle, &epInitStruct, &epCallback);
            if (kStatus_USB_Success != status)
            {
                return status;
            }
        }
    }

    return status;
}

static usb_status_t USB_DeviceCdcEcmEndpointsDeinit(usb_device_cdc_ecm_struct_t *handle)
{
    usb_status_t status = kStatus_USB_Error;
    uint32_t cnt;

    if ((NULL == handle->commInterfaceHandle) || (NULL == handle->dataInterfaceHandle))
    {
        return kStatus_USB_InvalidHandle;
    }

    for (cnt = 0U; cnt < handle->commInterfaceHandle->endpointList.count; cnt++)
    {
        status = USB_DeviceDeinitEndpoint(handle->handle, handle->commInterfaceHandle->endpointList.endpoint[cnt].endpointAddress);
        if (kStatus_USB_Success != status)
        {
            return status;
        }
    }

    for (cnt = 0U; cnt < handle->dataInterfaceHandle->endpointList.count; cnt++)
    {
        status = USB_DeviceDeinitEndpoint(handle->handle, handle->dataInterfaceHandle->endpointList.endpoint[cnt].endpointAddress);
        if (kStatus_USB_Success != status)
        {
            return status;
        }
    }

    handle->commInterfaceHandle = NULL;
    handle->dataInterfaceHandle = NULL;

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceCdcEcmInit(uint8_t controllerId, usb_device_class_config_struct_t *config, class_handle_t *handle)
{
    usb_device_cdc_ecm_struct_t *cdcEcmHandle;
    usb_status_t status;

    status = USB_DeviceCdcEcmAllocateHandle(&cdcEcmHandle);
    if (kStatus_USB_Success != status)
    {
        return status;
    }

    status = USB_DeviceClassGetDeviceHandle(controllerId, &cdcEcmHandle->handle);
    if (kStatus_USB_Success != status)
    {
        return status;
    }

    cdcEcmHandle->config = config;
    *handle = (class_handle_t)cdcEcmHandle;

    return status;
}

usb_status_t USB_DeviceCdcEcmDeinit(class_handle_t handle)
{
    usb_device_cdc_ecm_struct_t *cdcEcmHandle = (usb_device_cdc_ecm_struct_t *)handle;
    usb_status_t status;

    if (NULL == cdcEcmHandle)
    {
        return kStatus_USB_InvalidHandle;
    }

    status = USB_DeviceCdcEcmEndpointsDeinit(cdcEcmHandle);
    USB_DeviceCdcEcmFreeHandle(cdcEcmHandle);

    return status;
}

usb_status_t USB_DeviceCdcEcmSend(class_handle_t handle, uint8_t ep, uint8_t *buffer, uint32_t length)
{
    usb_device_cdc_ecm_struct_t *cdcEcmHandle = (usb_device_cdc_ecm_struct_t *)handle;
    usb_status_t status = kStatus_USB_InvalidParameter;
    usb_device_cdc_ecm_pipe_t *cdcEcmPipe = NULL;

    if (NULL == handle)
    {
        return kStatus_USB_InvalidHandle;
    }

    if (cdcEcmHandle->bulkIn.ep == ep)
    {
        cdcEcmPipe = &(cdcEcmHandle->bulkIn);
    }
    else if (cdcEcmHandle->interruptIn.ep == ep)
    {
        cdcEcmPipe = &(cdcEcmHandle->interruptIn);
    }

    if (cdcEcmPipe)
    {
        if (cdcEcmPipe->isBusy)
        {
            return kStatus_USB_Busy;
        }

        if (cdcEcmPipe->pipeStall)
        {
            cdcEcmPipe->pipeDataBuffer = buffer;
            cdcEcmPipe->pipeDataLen = length;
            return kStatus_USB_Success;
        }

        cdcEcmPipe->isBusy = 1U;

        status = USB_DeviceSendRequest(cdcEcmHandle->handle, ep, buffer, length);
        if (kStatus_USB_Success != status)
        {
            cdcEcmPipe->isBusy = 0U;
        }
    }

    return status;
}

usb_status_t USB_DeviceCdcEcmRecv(class_handle_t handle, uint8_t ep, uint8_t *buffer, uint32_t length)
{
    usb_device_cdc_ecm_struct_t *cdcEcmHandle = (usb_device_cdc_ecm_struct_t *)handle;
    usb_status_t status = kStatus_USB_InvalidParameter;
    usb_device_cdc_ecm_pipe_t *cdcEcmPipe = NULL;

    if (NULL == handle)
    {
        return kStatus_USB_InvalidHandle;
    }

    if (cdcEcmHandle->bulkOut.ep == ep)
    {
        cdcEcmPipe = &(cdcEcmHandle->bulkOut);
    }

    if (cdcEcmPipe)
    {
        if (cdcEcmPipe->isBusy)
        {
            return kStatus_USB_Busy;
        }

        if (cdcEcmPipe->pipeStall)
        {
            cdcEcmPipe->pipeDataBuffer = buffer;
            cdcEcmPipe->pipeDataLen = length;
            return kStatus_USB_Success;
        }

        cdcEcmPipe->isBusy = 1U;

        status = USB_DeviceRecvRequest(cdcEcmHandle->handle, ep, buffer, length);
        if (kStatus_USB_Success != status)
        {
            cdcEcmPipe->isBusy = 0U;
        }
    }

    return status;
}

usb_status_t USB_DeviceCdcEcmEvent(void *handle, uint32_t event, void *param)
{
    usb_device_cdc_ecm_struct_t *cdcEcmHandle = (usb_device_cdc_ecm_struct_t *)handle;
    usb_device_class_event_t eventCode = (usb_device_class_event_t)event;
    usb_status_t status = kStatus_USB_Error;

    if ((NULL == param) || (NULL == handle))
    {
        return kStatus_USB_InvalidHandle;
    }

    switch (eventCode)
    {
        case kUSB_DeviceClassEventDeviceReset:
            cdcEcmHandle->configurationValue = 0U;
            cdcEcmHandle->bulkIn.isBusy = 0U;
            cdcEcmHandle->bulkOut.isBusy = 0U;
            cdcEcmHandle->interruptIn.isBusy = 0U;
            cdcEcmHandle->commInterfaceHandle = NULL;
            cdcEcmHandle->dataInterfaceHandle = NULL;
            status = USB_DeviceCdcEcmEndpointsDeinit(cdcEcmHandle);
            break;

        case kUSB_DeviceClassEventSetConfiguration:
            if (NULL == cdcEcmHandle->config)
            {
                break;
            }
            if (*((uint8_t *)param) == cdcEcmHandle->configurationValue)
            {
                status = kStatus_USB_Success;
                break;
            }

            cdcEcmHandle->configurationValue = *((uint8_t *)param);
            cdcEcmHandle->alternate = 0U;
            status = USB_DeviceCdcEcmEndpointsInit(cdcEcmHandle);
            if (kStatus_USB_Success != status)
            {
                (void)usb_echo("USB_DeviceCdcEcmEndpointsInit() error in kUSB_DeviceClassEventSetConfiguration of USB_DeviceCdcEcmEvent().\r\n");
            }
            break;

        case kUSB_DeviceClassEventSetInterface:
            if (NULL == cdcEcmHandle->config)
            {
                break;
            }
            if (cdcEcmHandle->interfaceNumber != *((uint8_t *)param + 1))
            {
                break;
            }

            if (*((uint8_t *)param) == cdcEcmHandle->alternate)
            {
                status = kStatus_USB_Success;
                break;
            }
            status = USB_DeviceCdcEcmEndpointsDeinit(cdcEcmHandle);
            cdcEcmHandle->alternate = *((uint8_t *)param);
            status = USB_DeviceCdcEcmEndpointsInit(cdcEcmHandle);
            if (kStatus_USB_Success != status)
            {
                (void)usb_echo("USB_DeviceCdcEcmEndpointsInit() error in kUSB_DeviceClassEventSetInterface of USB_DeviceCdcEcmEvent().\r\n");
            }
            break;

        case kUSB_DeviceClassEventSetEndpointHalt:
            if ((NULL == cdcEcmHandle->config) || (NULL == cdcEcmHandle->commInterfaceHandle) || (NULL == cdcEcmHandle->dataInterfaceHandle))
            {
                break;
            }
            for (uint32_t cnt = 0U; cnt < cdcEcmHandle->commInterfaceHandle->endpointList.count; cnt++)
            {
                if (*((uint8_t *)param) == cdcEcmHandle->commInterfaceHandle->endpointList.endpoint[cnt].endpointAddress)
                {
                    cdcEcmHandle->interruptIn.pipeStall = 1U;
                    status = USB_DeviceStallEndpoint(cdcEcmHandle->handle, *((uint8_t *)param));
                }
            }
            for (uint32_t cnt = 0U; cnt < cdcEcmHandle->dataInterfaceHandle->endpointList.count; cnt++)
            {
                if (*((uint8_t *)param) == cdcEcmHandle->dataInterfaceHandle->endpointList.endpoint[cnt].endpointAddress)
                {
                    if (USB_IN == (((*((uint8_t *)param)) & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >> USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT))
                    {
                        cdcEcmHandle->bulkIn.pipeStall = 1U;
                    }
                    else
                    {
                        cdcEcmHandle->bulkOut.pipeStall = 1U;
                    }
                    status = USB_DeviceStallEndpoint(cdcEcmHandle->handle, *((uint8_t *)param));
                }
            }
            break;

        case kUSB_DeviceClassEventClearEndpointHalt:
            if ((NULL == cdcEcmHandle->config) || (NULL == cdcEcmHandle->commInterfaceHandle) || (NULL == cdcEcmHandle->dataInterfaceHandle))
            {
                break;
            }
            for (uint32_t cnt = 0U; cnt < cdcEcmHandle->commInterfaceHandle->endpointList.count; cnt++)
            {
                if (*((uint8_t *)param) == cdcEcmHandle->commInterfaceHandle->endpointList.endpoint[cnt].endpointAddress)
                {
                    status = USB_DeviceUnstallEndpoint(cdcEcmHandle->handle, *((uint8_t *)param));
                    if (USB_IN == (((*((uint8_t *)param)) & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >> USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT))
                    {
                        if (cdcEcmHandle->interruptIn.pipeStall)
                        {
                            cdcEcmHandle->interruptIn.pipeStall = 0U;

                            if ((uint8_t *)USB_INVALID_TRANSFER_BUFFER != cdcEcmHandle->interruptIn.pipeDataBuffer)
                            {
                                status = USB_DeviceSendRequest(cdcEcmHandle->handle, (cdcEcmHandle->interruptIn.ep), cdcEcmHandle->interruptIn.pipeDataBuffer, cdcEcmHandle->interruptIn.pipeDataLen);
                                if (kStatus_USB_Success != status)
                                {
                                    usb_device_endpoint_callback_message_struct_t endpointCallbackMessage;
                                    endpointCallbackMessage.buffer = cdcEcmHandle->interruptIn.pipeDataBuffer;
                                    endpointCallbackMessage.length = cdcEcmHandle->interruptIn.pipeDataLen;
                                    endpointCallbackMessage.isSetup = 0U;

                                    if (kStatus_USB_Success != USB_DeviceCdcEcmInterruptIn(cdcEcmHandle->handle, (void *)&endpointCallbackMessage, handle))
                                    {
                                        return kStatus_USB_Error;
                                    }
                                }
                                cdcEcmHandle->interruptIn.pipeDataBuffer = (uint8_t *)USB_INVALID_TRANSFER_BUFFER;
                                cdcEcmHandle->interruptIn.pipeDataLen = 0U;
                            }
                        }
                    }
                }
            }
            for (uint32_t cnt = 0U; cnt < cdcEcmHandle->dataInterfaceHandle->endpointList.count; cnt++)
            {
                if (*((uint8_t *)param) == cdcEcmHandle->dataInterfaceHandle->endpointList.endpoint[cnt].endpointAddress)
                {
                    status = USB_DeviceUnstallEndpoint(cdcEcmHandle->handle, *((uint8_t *)param));
                    if (USB_IN == (((*((uint8_t *)param)) & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >> USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT))
                    {
                        if (cdcEcmHandle->bulkIn.pipeStall)
                        {
                            cdcEcmHandle->bulkIn.pipeStall = 0U;

                            if ((uint8_t *)USB_INVALID_TRANSFER_BUFFER != cdcEcmHandle->bulkIn.pipeDataBuffer)
                            {
                                status = USB_DeviceSendRequest(cdcEcmHandle->handle, (cdcEcmHandle->bulkIn.ep), cdcEcmHandle->bulkIn.pipeDataBuffer, cdcEcmHandle->bulkIn.pipeDataLen);
                                if (kStatus_USB_Success != status)
                                {
                                    usb_device_endpoint_callback_message_struct_t endpointCallbackMessage;
                                    endpointCallbackMessage.buffer = cdcEcmHandle->bulkIn.pipeDataBuffer;
                                    endpointCallbackMessage.length = cdcEcmHandle->bulkIn.pipeDataLen;
                                    endpointCallbackMessage.isSetup = 0U;
                                    if (kStatus_USB_Success != USB_DeviceCdcEcmBulkIn(cdcEcmHandle->handle, (void *)&endpointCallbackMessage, handle))
                                    {
                                        return kStatus_USB_Error;
                                    }
                                }
                                cdcEcmHandle->bulkIn.pipeDataBuffer = (uint8_t *)USB_INVALID_TRANSFER_BUFFER;
                                cdcEcmHandle->bulkIn.pipeDataLen = 0U;
                            }
                        }
                    }
                    else
                    {
                        if (cdcEcmHandle->bulkOut.pipeStall)
                        {
                            cdcEcmHandle->bulkOut.pipeStall = 0U;

                            if ((uint8_t *)USB_INVALID_TRANSFER_BUFFER != cdcEcmHandle->bulkOut.pipeDataBuffer)
                            {
                                status = USB_DeviceRecvRequest(cdcEcmHandle->handle, (cdcEcmHandle->bulkOut.ep),
                                                               cdcEcmHandle->bulkOut.pipeDataBuffer,
                                                               cdcEcmHandle->bulkOut.pipeDataLen);
                                if (kStatus_USB_Success != status)
                                {
                                    usb_device_endpoint_callback_message_struct_t endpointCallbackMessage;
                                    endpointCallbackMessage.buffer = cdcEcmHandle->bulkOut.pipeDataBuffer;
                                    endpointCallbackMessage.length = cdcEcmHandle->bulkOut.pipeDataLen;
                                    endpointCallbackMessage.isSetup = 0U;
                                    if (kStatus_USB_Success != USB_DeviceCdcEcmBulkOut(cdcEcmHandle->handle, (void *)&endpointCallbackMessage, handle))
                                    {
                                        return kStatus_USB_Error;
                                    }
                                }
                                cdcEcmHandle->bulkOut.pipeDataBuffer = (uint8_t *)USB_INVALID_TRANSFER_BUFFER;
                                cdcEcmHandle->bulkOut.pipeDataLen = 0U;
                            }
                        }
                    }
                }
            }
            break;

        case kUSB_DeviceClassEventClassRequest:
        {
            usb_device_control_request_struct_t *controlRequest = (usb_device_control_request_struct_t *)param;

            if ((controlRequest->setup->bmRequestType & USB_REQUEST_TYPE_RECIPIENT_MASK) != USB_REQUEST_TYPE_RECIPIENT_INTERFACE)
            {
                break;
            }

            if ((USB_SHORT_GET_LOW(controlRequest->setup->wIndex)) != cdcEcmHandle->interfaceNumber)
            {
                break;
            }

            switch (controlRequest->setup->bRequest)
            {
                case USB_DEVICE_CDC_ECM_SET_ETHERNET_MULTICAST_FILTER:
                    if ((controlRequest->setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_OUT)
                    {
                        status = cdcEcmHandle->config->classCallback((class_handle_t)cdcEcmHandle, kUSB_DeviceCdcEcmEventSetEthernetMulticastFilters, param);
                    }
                    break;

                case USB_DEVICE_CDC_ECM_SET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER:
                    if ((controlRequest->setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_OUT)
                    {
                        status = cdcEcmHandle->config->classCallback((class_handle_t)cdcEcmHandle, kUSB_DeviceCdcEcmEventSetEthernetPowerManagementPatternFilter, param);
                    }
                    break;

                case USB_DEVICE_CDC_ECM_GET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER:
                    if ((controlRequest->setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_IN)
                    {
                        status = cdcEcmHandle->config->classCallback((class_handle_t)cdcEcmHandle, kUSB_DeviceCdcEcmEventGetEthernetPowerManagementPatternFilter, param);
                    }
                    break;

                case USB_DEVICE_CDC_ECM_SET_ETHERNET_PACKET_FILTER:
                    if ((controlRequest->setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_OUT)
                    {
                        status = cdcEcmHandle->config->classCallback((class_handle_t)cdcEcmHandle, kUSB_DeviceCdcEcmEventSetEthernetPacketFilter, param);
                    }
                    break;

                case USB_DEVICE_CDC_ECM_GET_ETHERNET_STATISTIC:
                    if ((controlRequest->setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_IN)
                    {
                        status = cdcEcmHandle->config->classCallback((class_handle_t)cdcEcmHandle, kUSB_DeviceCdcEcmEventGetEthernetStatistic, param);
                    }
                    break;

                default:
                    break;
            }

            break;
        }

        default:
            break;
    }

    return status;
}
#endif
