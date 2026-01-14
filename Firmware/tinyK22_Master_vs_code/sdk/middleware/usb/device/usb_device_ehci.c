/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017,2019 - 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "fsl_device_registers.h"
#include "usb.h"
/* CONFIG_UDC_DRIVER is for Zephyr, it will not be defined in NXP MCUXpresso SDK */
#if !((defined CONFIG_UDC_DRIVER) && (CONFIG_UDC_DRIVER))
#include "usb_device.h"
#endif
#if ((defined(USB_DEVICE_CONFIG_EHCI)) && (USB_DEVICE_CONFIG_EHCI > 0U))

/* CONFIG_UDC_DRIVER is for Zephyr, it will not be defined in NXP MCUXpresso SDK */
#if ((defined CONFIG_UDC_DRIVER) && (CONFIG_UDC_DRIVER))
#include "usb_device_mcux_drv_port.h"
#include "usb_device_ehci.h"
#else
#include "usb_device_dci.h"

#include "usb_device_ehci.h"
#endif
#if (defined(USB_DEVICE_CONFIG_LOW_POWER_MODE) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))
#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#include "usb_phy.h"
#endif
#endif
#if (defined(USB_DEVICE_CONFIG_CHARGER_DETECT) && (USB_DEVICE_CONFIG_CHARGER_DETECT > 0U)) && \
    (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U))
#include "usb_hsdcd.h"
#elif (defined(USB_DEVICE_CONFIG_CHARGER_DETECT) && (USB_DEVICE_CONFIG_CHARGER_DETECT > 0U)) && \
    (defined(FSL_FEATURE_SOC_USB_ANALOG_COUNT) && (FSL_FEATURE_SOC_USB_ANALOG_COUNT > 0U))
#include "usb_phydcd.h"
#endif
#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
#include "fsl_memory.h"
#endif
#if (defined(USB_DEVICE_CONFIG_BUFFER_PROPERTY_CACHEABLE) && (USB_DEVICE_CONFIG_BUFFER_PROPERTY_CACHEABLE > 0U))
#include "fsl_cache.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if defined(USB_STACK_USE_DEDICATED_RAM) && (USB_STACK_USE_DEDICATED_RAM > 0U)

#error The SOC does not suppoort dedicated RAM case.

#endif

#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
#define USB_DEV_MEMORY_CPU_2_DMA(x) MEMORY_ConvertMemoryMapAddress((uint32_t)(x), kMEMORY_Local2DMA)
#define USB_DEV_MEMORY_DMA_2_CPU(x) MEMORY_ConvertMemoryMapAddress((uint32_t)(x), kMEMORY_DMA2Local)
#endif

#if (defined(FSL_FEATURE_USB_ATLANTIC_EHCI_SUPPORT) && (FSL_FEATURE_USB_ATLANTIC_EHCI_SUPPORT > 0U))
#define USB_PHY_POLLING_DELAY (1000000U)
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void USB_DeviceEhciSetDefaultState(usb_device_ehci_state_struct_t *ehciState);
static usb_status_t USB_DeviceEhciEndpointInit(usb_device_ehci_state_struct_t *ehciState,
                                               usb_device_endpoint_init_struct_t *epInit);
static usb_status_t USB_DeviceEhciEndpointDeinit(usb_device_ehci_state_struct_t *ehciState, uint8_t ep);
static usb_status_t USB_DeviceEhciEndpointStall(usb_device_ehci_state_struct_t *ehciState, uint8_t ep);
static usb_status_t USB_DeviceEhciEndpointUnstall(usb_device_ehci_state_struct_t *ehciState, uint8_t ep);
static void USB_DeviceEhciFillSetupBuffer(usb_device_ehci_state_struct_t *ehciState, uint8_t ep);
static void USB_DeviceEhciCancelControlPipe(usb_device_ehci_state_struct_t *ehciState,
                                            uint8_t endpoint,
                                            uint8_t direction);
static void USB_DeviceEhciInterruptTokenDone(usb_device_ehci_state_struct_t *ehciState);
static void USB_DeviceEhciInterruptPortChange(usb_device_ehci_state_struct_t *ehciState);
static void USB_DeviceEhciInterruptReset(usb_device_ehci_state_struct_t *ehciState);
#if (defined(USB_DEVICE_CONFIG_SOF_NOTIFICATIONS) && (USB_DEVICE_CONFIG_SOF_NOTIFICATIONS > 0U))
static void USB_DeviceEhciInterruptSOF(usb_device_ehci_state_struct_t *ehciState);
#endif /* USB_DEVICE_CONFIG_SOF_NOTIFICATIONS */
#if (defined(USB_DEVICE_CONFIG_LOW_POWER_MODE) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))
static void USB_DeviceEhciInterruptSuspend(usb_device_ehci_state_struct_t *ehciState);
#endif /* USB_DEVICE_CONFIG_LOW_POWER_MODE */
static usb_status_t USB_DeviceEhciTransfer(usb_device_ehci_state_struct_t *ehciState,
                                           uint8_t endpointAddress,
                                           uint8_t *buffer,
                                           uint32_t length);
static usb_status_t USB_DeviceEhciNotification(usb_device_ehci_state_struct_t *ehciState,
                                               usb_device_callback_message_struct_t *message);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Apply for QH buffer, 2048-byte alignment */
USB_RAM_ADDRESS_ALIGNMENT(2048)
USB_CONTROLLER_DATA static uint8_t qh_buffer[(USB_DEVICE_CONFIG_EHCI - 1) * 2048 +
                                             USB_DEVICE_CONFIG_ENDPOINTS * 2 * sizeof(usb_device_ehci_qh_struct_t)];

/* Apply for DTD buffer, 32-byte alignment */
USB_RAM_ADDRESS_ALIGNMENT(32)
USB_CONTROLLER_DATA static usb_device_ehci_dtd_struct_t s_UsbDeviceEhciDtd[USB_DEVICE_CONFIG_EHCI]
                                                                          [USB_DEVICE_CONFIG_EHCI_MAX_DTD];

/* Apply for ehci device state structure */
static usb_device_ehci_state_struct_t g_UsbDeviceEhciState[USB_DEVICE_CONFIG_EHCI];

/* Apply for whether the corresponding g_UsbDeviceEhciState is used or not, if used, it is set to 1, if not used, it is
 * set to 0 */
static uint8_t g_UsbDeviceEhciStateStatus[USB_DEVICE_CONFIG_EHCI] = {0};

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief EHCI get USB base address.
 *
 * This function is used to get USB base address according to EHCI controller ID.
 *
 * @param[in] controllerId    EHCI controller ID; See the #usb_controller_index_t.
 * @param[in] baseArray       USB base address array.
 * @param[in] baseCount       The number of elements of baseArray.
 *
 * @retval USB base address.
 */
#if ((defined(USB_DEVICE_CONFIG_LOW_POWER_MODE) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U)) && \
     (defined(FSL_FEATURE_SOC_USBNC_COUNT) && (FSL_FEATURE_SOC_USBNC_COUNT > 0U))) ||          \
    ((defined(USB_DEVICE_CONFIG_CHARGER_DETECT) && (USB_DEVICE_CONFIG_CHARGER_DETECT > 0U)) && \
     (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U)))
static void *USB_EhciGetBase(uint8_t controllerId, uint32_t *baseArray, uint8_t baseCount)
{
    if (controllerId < (uint8_t)kUSB_ControllerEhci0)
    {
        return NULL;
    }

    controllerId = controllerId - (uint8_t)kUSB_ControllerEhci0;
    if (controllerId >= baseCount)
    {
        return NULL;
    }

    return (void *)(uint8_t *)baseArray[controllerId];
}
#endif

/*!
 * @brief Set device controller state to default state.
 *
 * The function is used to set device controller state to default state.
 * The function will be called when USB_DeviceEhciInit called or the control type kUSB_DeviceControlGetEndpointStatus
 * received in USB_DeviceEhciControl.
 *
 * @param ehciState       Pointer of the device EHCI state structure.
 *
 */
static void USB_DeviceEhciSetDefaultState(usb_device_ehci_state_struct_t *ehciState)
{
    usb_device_ehci_dtd_struct_t *p;

    /* Initialize the dtd free queue */
    ehciState->dtdFree = ehciState->dtd;
    p                  = ehciState->dtdFree;
    for (uint32_t i = 1U; i < USB_DEVICE_CONFIG_EHCI_MAX_DTD; i++)
    {
        p->nextDtdPointer = (uint32_t)&ehciState->dtd[i];
        p                 = (usb_device_ehci_dtd_struct_t *)p->nextDtdPointer;
    }
    p->nextDtdPointer   = 0U;
    ehciState->dtdCount = USB_DEVICE_CONFIG_EHCI_MAX_DTD;

    /* Not use interrupt threshold. */
    ehciState->registerBase->USBCMD &= ~USBHS_USBCMD_ITC_MASK;
    ehciState->registerBase->USBCMD |= USBHS_USBCMD_ITC(0U);

    /* Disable setup lockout, please refer to "Control Endpoint Operation" section in RM. */
    ehciState->registerBase->USBMODE |= USBHS_USBMODE_SLOM_MASK;

/* Set the endian by using CPU's endian */
#if (ENDIANNESS == USB_BIG_ENDIAN)
    ehciState->registerBase->USBMODE |= USBHS_USBMODE_ES_MASK;
#else
    ehciState->registerBase->USBMODE &= ~USBHS_USBMODE_ES_MASK;
#endif
    /* Initialize the QHs of endpoint. */
    for (uint32_t i = 0U; i < (USB_DEVICE_CONFIG_ENDPOINTS * 2U); i++)
    {
        ehciState->qh[i].nextDtdPointer = USB_DEVICE_ECHI_DTD_TERMINATE_MASK;
        ehciState->qh[i].capabilttiesCharacteristicsUnion.capabilttiesCharacteristicsBitmap.maxPacketSize =
            USB_CONTROL_MAX_PACKET_SIZE;
        ehciState->dtdHard[i]                                              = NULL;
        ehciState->dtdTail[i]                                              = NULL;
        ehciState->qh[i].endpointStatusUnion.endpointStatusBitmap.isOpened = 0U;
    }

#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
    /* Add QH buffer address to USBHS_EPLISTADDR_REG */
    ehciState->registerBase->EPLISTADDR = (uint32_t)USB_DEV_MEMORY_CPU_2_DMA(ehciState->qh);
#else
    /* Add QH buffer address to USBHS_EPLISTADDR_REG */
    ehciState->registerBase->EPLISTADDR = (uint32_t)ehciState->qh;
#endif

    /* Clear device address */
    ehciState->registerBase->DEVICEADDR = 0U;

#if defined(USB_DEVICE_CONFIG_DETACH_ENABLE) && (USB_DEVICE_CONFIG_DETACH_ENABLE > 0U)
    ehciState->registerBase->OTGSC = ehciState->registerBase->OTGSC & 0x0000FFFFU;
    ehciState->registerBase->OTGSC |= USBHS_OTGSC_BSVIE_MASK;
#endif /* USB_DEVICE_CONFIG_DETACH_ENABLE */

    /* Enable USB Interrupt, USB Error Interrupt, Port Change detect Interrupt, USB-Reset Interrupt*/
    ehciState->registerBase->USBINTR =
        (USBHS_USBINTR_UE_MASK | USBHS_USBINTR_UEE_MASK | USBHS_USBINTR_PCE_MASK | USBHS_USBINTR_URE_MASK
#if (defined(USB_DEVICE_CONFIG_LOW_POWER_MODE) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))
         | USBHS_USBINTR_SLE_MASK
#endif /* USB_DEVICE_CONFIG_LOW_POWER_MODE */
#if (defined(USB_DEVICE_CONFIG_SOF_NOTIFICATIONS) && (USB_DEVICE_CONFIG_SOF_NOTIFICATIONS > 0U))
         | USBHS_USBINTR_SRE_MASK
#endif /* USB_DEVICE_CONFIG_SOF_NOTIFICATIONS */
        );

    /* Enable USB LPM L1 entry interrupt and exit interrupt */
#if ((defined(USB_DEVICE_CONFIG_LOW_POWER_MODE)) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))
#if ((defined(USB_DEVICE_CONFIG_LPM_L1)) && (USB_DEVICE_CONFIG_LPM_L1 > 0U))
    ehciState->registerBase->USBINTR |= USB_USBINTR_LPM_L1_ENTRYIE_MASK;
    ehciState->registerNcBase->LPM_CSR0 |= USBNC_LPM_CSR0_LPM_EN_MASK;
    ehciState->registerNcBase->LPM_CSR1 |= USBNC_LPM_CSR1_LPM_DEV_RES_MASK;
#endif
#endif
    /* Clear reset flag */
    ehciState->isResetting = 0U;
}

/*!
 * @brief Initialize a specified endpoint.
 *
 * The function is used to initialize a specified endpoint.
 *
 * @param ehciState       Pointer of the device EHCI state structure.
 * @param epInit          The endpoint initialization structure pointer.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
static usb_status_t USB_DeviceEhciEndpointInit(usb_device_ehci_state_struct_t *ehciState,
                                               usb_device_endpoint_init_struct_t *epInit)
{
    uint32_t primeBit      = 1UL << ((epInit->endpointAddress & USB_ENDPOINT_NUMBER_MASK) +
                                ((epInit->endpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >> 0x03U));
    uint16_t maxPacketSize = epInit->maxPacketSize & USB_DESCRIPTOR_ENDPOINT_MAXPACKETSIZE_SIZE_MASK;
    uint8_t endpoint       = (epInit->endpointAddress & USB_ENDPOINT_NUMBER_MASK);
    uint8_t direction      = (epInit->endpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >>
                        USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT;
    uint8_t index        = ((uint8_t)((uint32_t)endpoint << 1U)) | direction;
    uint8_t transferType = epInit->transferType & USB_DESCRIPTOR_ENDPOINT_ATTRIBUTE_TYPE_MASK;

    /* Cancel pending transfer of the endpoint */
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    if (kStatus_USB_Success != USB_DeviceEhciCancel(ehciState, epInit->endpointAddress))
    {
        return kStatus_USB_Error;
    }
#else
    (void)USB_DeviceEhciCancel(ehciState, epInit->endpointAddress);
#endif

    if ((0U != (ehciState->registerBase->EPPRIME & primeBit)) || (0U != (ehciState->registerBase->EPSR & primeBit)))
    {
        return kStatus_USB_Busy;
    }

    /* Make the endpoint max packet size align with USB Specification 2.0. */
    if (USB_ENDPOINT_ISOCHRONOUS == transferType)
    {
        if (maxPacketSize > USB_DEVICE_MAX_HS_ISO_MAX_PACKET_SIZE)
        {
            maxPacketSize = USB_DEVICE_MAX_HS_ISO_MAX_PACKET_SIZE;
        }
        ehciState->qh[index].capabilttiesCharacteristicsUnion.capabilttiesCharacteristicsBitmap.mult =
            1UL + ((((uint32_t)epInit->maxPacketSize) & USB_DESCRIPTOR_ENDPOINT_MAXPACKETSIZE_MULT_TRANSACTIONS_MASK) >>
                   USB_DESCRIPTOR_ENDPOINT_MAXPACKETSIZE_MULT_TRANSACTIONS_SHFIT);
    }
    else
    {
        ehciState->qh[index].capabilttiesCharacteristicsUnion.capabilttiesCharacteristicsBitmap.mult = 0U;
    }

    /* Save the max packet size of the endpoint */
    ehciState->qh[index].capabilttiesCharacteristicsUnion.capabilttiesCharacteristicsBitmap.maxPacketSize =
        maxPacketSize;
    ehciState->qh[index].endpointStatusUnion.endpointStatusBitmap.zlt = epInit->zlt;
    if ((USB_CONTROL_ENDPOINT == endpoint))
    {
        /* Set ZLT bit. disable control endpoint automatic zlt by default,only send zlt when it is needed*/
        ehciState->qh[index].capabilttiesCharacteristicsUnion.capabilttiesCharacteristicsBitmap.zlt = 1U;
    }
    else
    {
        /* Set ZLT bit. */
        ehciState->qh[index].capabilttiesCharacteristicsUnion.capabilttiesCharacteristicsBitmap.zlt =
            ((0U == epInit->zlt) ? 1U : 0U);
    }

    /* Enable the endpoint. */
    if ((USB_CONTROL_ENDPOINT == endpoint))
    {
        ehciState->qh[index].capabilttiesCharacteristicsUnion.capabilttiesCharacteristicsBitmap.ios = 1U;
        ehciState->registerBase->EPCR0 |=
            ((0U != direction) ?
                 (USBHS_EPCR_TXE_MASK | USBHS_EPCR_TXR_MASK | ((uint32_t)transferType << USBHS_EPCR_TXT_SHIFT)) :
                 (USBHS_EPCR_RXE_MASK | USBHS_EPCR_RXR_MASK | ((uint32_t)transferType << USBHS_EPCR_RXT_SHIFT)));
    }
    else
    {
        ehciState->qh[index].capabilttiesCharacteristicsUnion.capabilttiesCharacteristicsBitmap.ios = 0U;
        ehciState->registerBase->EPCR[endpoint - 1U] |=
            ((0U != direction) ?
                 (USBHS_EPCR_TXE_MASK | USBHS_EPCR_TXR_MASK | ((uint32_t)transferType << USBHS_EPCR_TXT_SHIFT)) :
                 (USBHS_EPCR_RXE_MASK | USBHS_EPCR_RXR_MASK | ((uint32_t)transferType << USBHS_EPCR_RXT_SHIFT)));
    }

    ehciState->qh[index].endpointStatusUnion.endpointStatusBitmap.isOpened = 1U;
    return kStatus_USB_Success;
}

/*!
 * @brief De-initialize a specified endpoint.
 *
 * The function is used to de-initialize a specified endpoint.
 * Current transfer of the endpoint will be cancelled and the specified endpoint will be disabled.
 *
 * @param ehciState       Pointer of the device EHCI state structure.
 * @param ep               The endpoint address, Bit7, 0U - USB_OUT, 1U - USB_IN.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
static usb_status_t USB_DeviceEhciEndpointDeinit(usb_device_ehci_state_struct_t *ehciState, uint8_t ep)
{
    uint32_t primeBit =
        1UL << ((ep & USB_ENDPOINT_NUMBER_MASK) + ((ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >> 0x03U));
    uint8_t endpoint = (ep & USB_ENDPOINT_NUMBER_MASK);
    uint8_t direction =
        (ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >> USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT;
    uint8_t index = ((uint8_t)((uint32_t)endpoint << 1U)) | direction;

    ehciState->qh[index].endpointStatusUnion.endpointStatusBitmap.isOpened = 0U;

    /* Cancel the transfer of the endpoint */
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    if (kStatus_USB_Success != USB_DeviceEhciCancel(ehciState, ep))
    {
        return kStatus_USB_Error;
    }
#else
    (void)USB_DeviceEhciCancel(ehciState, ep);
#endif

    if ((0U != (ehciState->registerBase->EPPRIME & primeBit)) || (0U != (ehciState->registerBase->EPSR & primeBit)))
    {
        return kStatus_USB_Busy;
    }

    /* Clear endpoint state */
    ehciState->qh[index].capabilttiesCharacteristicsUnion.capabilttiesCharacteristics = 0U;
    /* Disable the endpoint */
    if (0U == endpoint)
    {
        ehciState->registerBase->EPCR0 &=
            ~((0U != direction) ? (USBHS_EPCR_TXE_MASK | USBHS_EPCR_TXT_MASK | USBHS_EPCR_TXS_MASK) :
                                  (USBHS_EPCR_RXE_MASK | USBHS_EPCR_RXT_MASK | USBHS_EPCR_RXS_MASK));
    }
    else
    {
        ehciState->registerBase->EPCR[endpoint - 1U] &=
            ~((0U != direction) ? (USBHS_EPCR_TXE_MASK | USBHS_EPCR_TXT_MASK | USBHS_EPCR_TXS_MASK) :
                                  (USBHS_EPCR_RXE_MASK | USBHS_EPCR_RXT_MASK | USBHS_EPCR_RXS_MASK));
    }

    return kStatus_USB_Success;
}

/*!
 * @brief Stall a specified endpoint.
 *
 * The function is used to stall a specified endpoint.
 * Current transfer of the endpoint will be cancelled and the specified endpoint will be stalled.
 *
 * @param ehciState       Pointer of the device EHCI state structure.
 * @param ep               The endpoint address, Bit7, 0U - USB_OUT, 1U - USB_IN.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
static usb_status_t USB_DeviceEhciEndpointStall(usb_device_ehci_state_struct_t *ehciState, uint8_t ep)
{
    uint8_t endpoint = ep & USB_ENDPOINT_NUMBER_MASK;
    uint8_t direction =
        (ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >> USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT;

    if (0U == endpoint)
    {
        /* Cancel the transfer of the endpoint */
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
        if ((kStatus_USB_Success != USB_DeviceEhciCancel(ehciState, 0x00)) ||
            (kStatus_USB_Success != USB_DeviceEhciCancel(ehciState, 0x80)))
        {
            return kStatus_USB_Error;
        }
#else
        (void)USB_DeviceEhciCancel(ehciState, 0x00);
        (void)USB_DeviceEhciCancel(ehciState, 0x80);
#endif
        ehciState->registerBase->EPCR0 |= (USBHS_EPCR_TXS_MASK | USBHS_EPCR_RXS_MASK);
    }
    else
    {
        /* Cancel the transfer of the endpoint */
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
        if (kStatus_USB_Success != USB_DeviceEhciCancel(ehciState, ep))
        {
            return kStatus_USB_Error;
        }
#else
        (void)USB_DeviceEhciCancel(ehciState, ep);
#endif

        ehciState->registerBase->EPCR[endpoint - 1U] |= ((0U != direction) ? USBHS_EPCR_TXS_MASK : USBHS_EPCR_RXS_MASK);
    }

    return kStatus_USB_Success;
}

/*!
 * @brief Un-stall a specified endpoint.
 *
 * The function is used to un-stall a specified endpoint.
 * Current transfer of the endpoint will be cancelled and the specified endpoint will be un-stalled.
 *
 * @param ehciState       Pointer of the device EHCI state structure.
 * @param ep               The endpoint address, Bit7, 0U - USB_OUT, 1U - USB_IN.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
static usb_status_t USB_DeviceEhciEndpointUnstall(usb_device_ehci_state_struct_t *ehciState, uint8_t ep)
{
    uint8_t endpoint = ep & USB_ENDPOINT_NUMBER_MASK;
    uint8_t direction =
        (ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >> USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT;

    /* Clear the endpoint stall state */
    if (0U == endpoint)
    {
        ehciState->registerBase->EPCR0 &= ~((0U != direction) ? USBHS_EPCR_TXS_MASK : USBHS_EPCR_RXS_MASK);
    }
    else
    {
        ehciState->registerBase->EPCR[endpoint - 1U] &=
            ~((0U != direction) ? USBHS_EPCR_TXS_MASK : USBHS_EPCR_RXS_MASK);
        ehciState->registerBase->EPCR[endpoint - 1U] |= ((0U != direction) ? USBHS_EPCR_TXR_MASK : USBHS_EPCR_RXR_MASK);
    }
    /* Cancel the transfer of the endpoint */
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    if (kStatus_USB_Success != USB_DeviceEhciCancel(ehciState, ep))
    {
        return kStatus_USB_Error;
    }
#else
    (void)USB_DeviceEhciCancel(ehciState, ep);
#endif

    return kStatus_USB_Success;
}

/*!
 * @brief Get setup packet data.
 *
 * The function is used to get setup packet data and copy to a backup buffer.
 *
 * @param ehciState       Pointer of the device EHCI state structure.
 * @param ep               The endpoint number.
 *
 */
static void USB_DeviceEhciFillSetupBuffer(usb_device_ehci_state_struct_t *ehciState, uint8_t ep)
{
    uint8_t waitingSafelyAccess = 1U;
    uint8_t index               = ((uint8_t)((uint32_t)ep << 1U)) | USB_OUT;

    /* Write 1U to clear corresponding bit in EPSETUPSR. */
    ehciState->registerBase->EPSETUPSR = 1UL << ep;

    while (0U != waitingSafelyAccess)
    {
        /* Set the setup tripwire bit. */
        ehciState->registerBase->USBCMD |= USBHS_USBCMD_SUTW_MASK;

        /* Copy setup packet data to backup buffer */
        ehciState->qh[index].setupBufferBack[0] = ehciState->qh[index].setupBuffer[0];
        ehciState->qh[index].setupBufferBack[1] = ehciState->qh[index].setupBuffer[1];

        /* Read the USBCMD[SUTW] bit. If set, jump out from the while loop; if cleared continue */
        if (0U != (ehciState->registerBase->USBCMD & USBHS_USBCMD_SUTW_MASK))
        {
            waitingSafelyAccess = 0U;
        }
    }
    /* Clear the setup tripwire bit */
    ehciState->registerBase->USBCMD &= ~USBHS_USBCMD_SUTW_MASK;
}

/*!
 * @brief Cancel the transfer of the control pipe.
 *
 * The function is used to cancel the transfer of the control pipe.
 *
 * @param ehciState       Pointer of the device EHCI state structure.
 * @param endpoint         The endpoint number.
 * @param direction        The direction of the endpoint.
 *
 */
static void USB_DeviceEhciCancelControlPipe(usb_device_ehci_state_struct_t *ehciState,
                                            uint8_t endpoint,
                                            uint8_t direction)
{
    usb_device_ehci_dtd_struct_t *currentDtd;
    uint32_t index    = ((uint32_t)endpoint << 1U) + (uint32_t)direction;
    uint32_t primeBit = 1UL << (endpoint + 16U * direction);
    usb_device_callback_message_struct_t message;

    message.buffer = NULL;
    message.length = 0U;
    /* Get the dtd of the control pipe */
    currentDtd =
        (usb_device_ehci_dtd_struct_t *)((uint32_t)ehciState->dtdHard[index] & USB_DEVICE_ECHI_DTD_POINTER_MASK);
    while (NULL != currentDtd)
    {
        /* Pass the transfer buffer address */
        if (NULL == message.buffer)
        {
#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
            uint32_t bufferAddress = (uint32_t)USB_DEV_MEMORY_DMA_2_CPU(currentDtd->bufferPointerPage[0]);
#else
            uint32_t bufferAddress    = currentDtd->bufferPointerPage[0];
#endif
            message.buffer = (uint8_t *)((bufferAddress & USB_DEVICE_ECHI_DTD_PAGE_MASK) |
                                         (currentDtd->reservedUnion.originalBufferInfo.originalBufferOffest));
        }
        /* If the dtd is active, set the message length to USB_CANCELLED_TRANSFER_LENGTH. Or set the length by using
         * finished length. */
        if (0U != (currentDtd->dtdTokenUnion.dtdTokenBitmap.status & USB_DEVICE_ECHI_DTD_STATUS_ACTIVE))
        {
            message.length = USB_CANCELLED_TRANSFER_LENGTH;
            /* Flush the endpoint to stop a transfer. */
            do
            {
                /* Set the corresponding bit(s) in the EPFLUSH register */
                ehciState->registerBase->EPFLUSH |= primeBit;

                /* Wait until all bits in the EPFLUSH register are cleared. */
                while (0U != (ehciState->registerBase->EPFLUSH & primeBit))
                {
                }
                /*
                 * Read the EPSR register to ensure that for all endpoints
                 * commanded to be flushed, that the corresponding bits
                 * are now cleared.
                 */
            } while (0U != (ehciState->registerBase->EPSR & primeBit));
        }
        else
        {
            if (currentDtd->reservedUnion.originalBufferInfo.originalBufferLength >=
                currentDtd->dtdTokenUnion.dtdTokenBitmap.totalBytes)
            {
                message.length += (currentDtd->reservedUnion.originalBufferInfo.originalBufferLength -
                                   currentDtd->dtdTokenUnion.dtdTokenBitmap.totalBytes);
            }
        }
        /* Clear the endpoint transfer done status */
        ehciState->registerBase->EPCOMPLETE = primeBit;

        /* Move the dtd head pointer to next. */
        /* If the pointer of the head equals to the tail, set the dtd queue to null. */
        if (ehciState->dtdHard[index] == ehciState->dtdTail[index])
        {
            ehciState->dtdHard[index]                   = NULL;
            ehciState->dtdTail[index]                   = NULL;
            ehciState->qh[index].nextDtdPointer         = USB_DEVICE_ECHI_DTD_TERMINATE_MASK;
            ehciState->qh[index].dtdTokenUnion.dtdToken = 0U;
        }
        else
        {
#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
            ehciState->dtdHard[index] =
                (usb_device_ehci_dtd_struct_t *)USB_DEV_MEMORY_DMA_2_CPU(ehciState->dtdHard[index]->nextDtdPointer);
#else
            ehciState->dtdHard[index] = (usb_device_ehci_dtd_struct_t *)ehciState->dtdHard[index]->nextDtdPointer;
#endif
        }

        /* When the ioc is set or the dtd queue is empty, the up layer will be notified. */
        if ((0U != currentDtd->dtdTokenUnion.dtdTokenBitmap.ioc) ||
            (0U == ((uint32_t)ehciState->dtdHard[index] & USB_DEVICE_ECHI_DTD_POINTER_MASK)))
        {
            message.code    = endpoint | (uint8_t)((uint32_t)direction << 0x07U);
            message.isSetup = 0U;
            (void)USB_DeviceEhciNotification(ehciState, &message);
            message.buffer = NULL;
            message.length = 0U;
        }

        /* Clear the token field of the dtd. */
        currentDtd->dtdTokenUnion.dtdToken = 0U;
        /* Add the dtd to the free dtd queue. */
        currentDtd->nextDtdPointer = (uint32_t)ehciState->dtdFree;
        ehciState->dtdFree         = currentDtd;
        ehciState->dtdCount++;

        /* Get the next in-used dtd. */
        currentDtd =
            (usb_device_ehci_dtd_struct_t *)((uint32_t)ehciState->dtdHard[index] & USB_DEVICE_ECHI_DTD_POINTER_MASK);
    }
}

/*!
 * @brief Handle the endpoint token done interrupt.
 *
 * The function is used to handle the endpoint token done interrupt.
 *
 * @param ehciState       Pointer of the device EHCI state structure.
 *
 */
static void USB_DeviceEhciInterruptTokenDone(usb_device_ehci_state_struct_t *ehciState)
{
    uint32_t status;
    uint32_t primeBit;
    usb_device_ehci_dtd_struct_t *currentDtd;
    void *temp;
    usb_device_callback_message_struct_t message;
    uint8_t endpoint;
    uint8_t direction;
    uint8_t count;
    uint8_t index;

    /* Get the EPSETUPSR to check the setup packect received in which one endpoint. */
    status = ehciState->registerBase->EPSETUPSR;

    if (0U != status)
    {
        for (endpoint = 0U; endpoint < USB_DEVICE_CONFIG_ENDPOINTS; endpoint++)
        {
            /* Check the endpoint receive the setup packet. */
            if (0U != (status & (1UL << endpoint)))
            {
                /* Get last setup packet */
                temp = (void *)&ehciState->qh[(uint8_t)((uint32_t)endpoint << 1U) + USB_OUT].setupBufferBack;
                usb_setup_struct_t *deviceSetup = (usb_setup_struct_t *)temp;

                /* Check the direction of the data phase. */
                direction = (deviceSetup->bmRequestType & USB_REQUEST_TYPE_DIR_IN) >> USB_REQUEST_TYPE_DIR_SHIFT;
                /* Cancel the data phase transfer */
                USB_DeviceEhciCancelControlPipe(ehciState, endpoint, direction);
                /* Cancel the status phase transfer */
                USB_DeviceEhciCancelControlPipe(ehciState, endpoint, 1U ^ direction);
                message.code    = (endpoint) | (USB_OUT << 0x07U);
                message.buffer  = (uint8_t *)deviceSetup;
                message.length  = USB_SETUP_PACKET_SIZE;
                message.isSetup = 1U;
                /* Fill the setup packet to the backup buffer */
                USB_DeviceEhciFillSetupBuffer(ehciState, endpoint);
                /* Notify the up layer the EHCI status changed. */
                (void)USB_DeviceEhciNotification(ehciState, &message);
            }
        }
    }
    /* Read the USBHS_EPCOMPLETE_REG to get the endpoint transfer done status */
    status = ehciState->registerBase->EPCOMPLETE;
    /* Clear the endpoint transfer done status */
    ehciState->registerBase->EPCOMPLETE = status;

    if (0U != status)
    {
        for (count = 0U; count < 32U; count++)
        {
            /* Check the transfer is done or not in the specified endpoint. */
            if (0U != (status & (1UL << count)))
            {
                if (count > 15U)
                {
                    endpoint  = count - 16U;
                    direction = USB_IN;
                }
                else
                {
                    endpoint  = count;
                    direction = USB_OUT;
                }
                if (endpoint >= USB_DEVICE_CONFIG_ENDPOINTS)
                {
                    continue;
                }
                index          = (endpoint << 1U) + direction;
                message.buffer = NULL;
                message.length = 0U;
                if ((USB_CONTROL_ENDPOINT == endpoint) && (USB_IN == direction))
                {
                    if (1U == ehciState->qh[index].endpointStatusUnion.endpointStatusBitmap.zlt)
                    {
                        if (0U ==
                            ehciState->qh[index].capabilttiesCharacteristicsUnion.capabilttiesCharacteristicsBitmap.zlt)
                        {
                            /*disable zlt after send zlt*/
                            ehciState->qh[index]
                                .capabilttiesCharacteristicsUnion.capabilttiesCharacteristicsBitmap.zlt = 1U;
                        }
                    }
                }
                /* Get the in-used dtd of the specified endpoint. */
                currentDtd = (usb_device_ehci_dtd_struct_t *)((uint32_t)ehciState->dtdHard[index] &
                                                              USB_DEVICE_ECHI_DTD_POINTER_MASK);
                while (NULL != currentDtd)
                {
                    uint8_t isTokenDone = 0;
                    /* Get the in-used dtd of the specified endpoint. */
                    currentDtd = (usb_device_ehci_dtd_struct_t *)((uint32_t)ehciState->dtdHard[index] &
                                                                  USB_DEVICE_ECHI_DTD_POINTER_MASK);

                    while (NULL != currentDtd)
                    {
#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
                        currentDtd = (usb_device_ehci_dtd_struct_t *)USB_DEV_MEMORY_DMA_2_CPU((uint8_t *)currentDtd);
#endif
                        /* Don't handle the active dtd. */
                        if ((0U !=
                             (currentDtd->dtdTokenUnion.dtdTokenBitmap.status & USB_DEVICE_ECHI_DTD_STATUS_ACTIVE)) ||
                            (0U != currentDtd->dtdTokenUnion.dtdTokenBitmap.ioc))
                        {
                            if ((0U == (currentDtd->dtdTokenUnion.dtdTokenBitmap.status &
                                        USB_DEVICE_ECHI_DTD_STATUS_ACTIVE)) &&
                                (0U != currentDtd->dtdTokenUnion.dtdTokenBitmap.ioc))
                            {
                                isTokenDone = 1U;
                            }
                            break;
                        }
                        currentDtd = (usb_device_ehci_dtd_struct_t *)(currentDtd->nextDtdPointer &
                                                                      USB_DEVICE_ECHI_DTD_POINTER_MASK);
                    }

                    if ((0U == isTokenDone) && (NULL != currentDtd))
                    {
                        break;
                    }

                    /* Get the in-used dtd of the specified endpoint. */
                    currentDtd = (usb_device_ehci_dtd_struct_t *)((uint32_t)ehciState->dtdHard[index] &
                                                                  USB_DEVICE_ECHI_DTD_POINTER_MASK);
                    while (NULL != currentDtd)
                    {
#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
                        currentDtd = (usb_device_ehci_dtd_struct_t *)USB_DEV_MEMORY_DMA_2_CPU((uint8_t *)currentDtd);
#endif
                        /* Don't handle the active dtd. */
                        if (0U != (currentDtd->dtdTokenUnion.dtdTokenBitmap.status & USB_DEVICE_ECHI_DTD_STATUS_ACTIVE))
                        {
                            break;
                        }

                        /* Save the transfer buffer address */
                        if (NULL == message.buffer)
                        {
                            message.buffer =
                                (uint8_t *)((currentDtd->bufferPointerPage[0] & USB_DEVICE_ECHI_DTD_PAGE_MASK) |
                                            (currentDtd->reservedUnion.originalBufferInfo.originalBufferOffest));
#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
                            message.buffer = (uint8_t *)USB_DEV_MEMORY_DMA_2_CPU(message.buffer);
#endif
                        }
                        /* Save the transferred data length */
                        if (currentDtd->reservedUnion.originalBufferInfo.originalBufferLength >=
                            currentDtd->dtdTokenUnion.dtdTokenBitmap.totalBytes)
                        {
                            message.length += (currentDtd->reservedUnion.originalBufferInfo.originalBufferLength -
                                               currentDtd->dtdTokenUnion.dtdTokenBitmap.totalBytes);
                        }

                        /* Move the dtd queue head pointer to next */
                        if (ehciState->dtdHard[index] == ehciState->dtdTail[index])
                        {
                            ehciState->dtdHard[index]                   = NULL;
                            ehciState->dtdTail[index]                   = NULL;
                            ehciState->qh[index].nextDtdPointer         = USB_DEVICE_ECHI_DTD_TERMINATE_MASK;
                            ehciState->qh[index].dtdTokenUnion.dtdToken = 0U;
                        }
                        else
                        {
#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
                            ehciState->dtdHard[index] = (usb_device_ehci_dtd_struct_t *)USB_DEV_MEMORY_DMA_2_CPU(
                                (uint8_t *)ehciState->dtdHard[index]->nextDtdPointer);
#else
                            ehciState->dtdHard[index] =
                                (usb_device_ehci_dtd_struct_t *)ehciState->dtdHard[index]->nextDtdPointer;
#endif
                        }

                        /* When the ioc is set or the dtd queue is empty, the up layer will be notified. */
                        if ((0U != currentDtd->dtdTokenUnion.dtdTokenBitmap.ioc) ||
                            (0U == ((uint32_t)ehciState->dtdHard[index] & USB_DEVICE_ECHI_DTD_POINTER_MASK)))
                        {
                            message.code    = endpoint | (uint8_t)((uint32_t)direction << 0x07U);
                            message.isSetup = 0U;
                            (void)USB_DeviceEhciNotification(ehciState, &message);
                            message.buffer = NULL;
                            message.length = 0U;
                        }
                        /* Clear the token field of the dtd */
                        currentDtd->dtdTokenUnion.dtdToken = 0U;
                        currentDtd->nextDtdPointer         = (uint32_t)ehciState->dtdFree;
                        ehciState->dtdFree                 = currentDtd;
                        ehciState->dtdCount++;
                        /* Get the next in-used dtd */
                        currentDtd = (usb_device_ehci_dtd_struct_t *)((uint32_t)ehciState->dtdHard[index] &
                                                                      USB_DEVICE_ECHI_DTD_POINTER_MASK);
#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
                        currentDtd = (usb_device_ehci_dtd_struct_t *)USB_DEV_MEMORY_DMA_2_CPU((uint8_t *)currentDtd);
#endif
                        if ((NULL != currentDtd) && (0U != (currentDtd->dtdTokenUnion.dtdTokenBitmap.status &
                                                            USB_DEVICE_ECHI_DTD_STATUS_ACTIVE)))
                        {
                            primeBit = 1UL << (endpoint + 16U * direction);

                            /* Try to prime the next dtd. */
                            ehciState->registerBase->EPPRIME = primeBit;

                            /* Whether the endpoint transmit/receive buffer is ready or not. If not, wait for prime bit
                             * cleared and prime the next dtd. */
                            if (0U == (ehciState->registerBase->EPSR & primeBit))
                            {
                                /* Wait for the endpoint prime bit cleared by HW */
                                while (0U != (ehciState->registerBase->EPPRIME & primeBit))
                                {
                                }

                                /* If the endpoint transmit/receive buffer is not ready */
                                if (0U == (ehciState->registerBase->EPSR & primeBit))
                                {
#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
                                    currentDtd =
                                        (usb_device_ehci_dtd_struct_t *)USB_DEV_MEMORY_CPU_2_DMA((uint8_t *)currentDtd);
#endif
                                    /* Prime next dtd and prime the transfer */
                                    ehciState->qh[index].nextDtdPointer         = (uint32_t)currentDtd;
                                    ehciState->qh[index].dtdTokenUnion.dtdToken = 0U;
                                    ehciState->registerBase->EPPRIME            = primeBit;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/*!
 * @brief Handle the port status change interrupt.
 *
 * The function is used to handle the port status change interrupt.
 *
 * @param ehciState       Pointer of the device EHCI state structure.
 *
 */
static void USB_DeviceEhciInterruptPortChange(usb_device_ehci_state_struct_t *ehciState)
{
    usb_device_callback_message_struct_t message;

    message.buffer  = (uint8_t *)NULL;
    message.length  = 0U;
    message.isSetup = 0U;

    /* Whether the port is doing reset. */
    if (0U == (ehciState->registerBase->PORTSC1 & USBHS_PORTSC1_PR_MASK))
    {
        /* If not, update the USB speed. */
        if (0U != (ehciState->registerBase->PORTSC1 & USBHS_PORTSC1_HSP_MASK))
        {
            ehciState->speed = USB_SPEED_HIGH;
        }
        else
        {
            ehciState->speed = USB_SPEED_FULL;
        }

        /* If the device reset flag is non-zero, notify the up layer the device reset finished. */
        if (0U != ehciState->isResetting)
        {
            message.code = (uint8_t)kUSB_DeviceNotifyBusReset;
            (void)USB_DeviceEhciNotification(ehciState, &message);
            ehciState->isResetting = 0U;
        }
    }

#if (defined(USB_DEVICE_CONFIG_LOW_POWER_MODE) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))
    if ((0U != ehciState->isSuspending) && (0U == (ehciState->registerBase->PORTSC1 & USBHS_PORTSC1_SUSP_MASK)))
    {
#if (defined(USB_DEVICE_CONFIG_LPM_L1) && (USB_DEVICE_CONFIG_LPM_L1 > 0U))
        /* pollinng if the device exits the L1 state */
        while (!(ehciState->registerBase->USBSTS & USB_USBSTS_LPM_L1_EXITI_MASK))
        {
            __NOP();
        }
        ehciState->registerBase->USBSTS |= USB_USBSTS_LPM_L1_EXITI_MASK;
#endif
        /* Set the resume flag */
        ehciState->isSuspending = 0U;

        message.code = (uint8_t)kUSB_DeviceNotifyResume;
        (void)USB_DeviceEhciNotification(ehciState, &message);
    }
#endif /* USB_DEVICE_CONFIG_LOW_POWER_MODE */
}

/*!
 * @brief Handle the reset interrupt.
 *
 * The function is used to handle the reset interrupt.
 *
 * @param ehciState       Pointer of the device EHCI state structure.
 *
 */
static void USB_DeviceEhciInterruptReset(usb_device_ehci_state_struct_t *ehciState)
{
    uint32_t status = 0U;

    /* Clear the setup flag */
    status                             = ehciState->registerBase->EPSETUPSR;
    ehciState->registerBase->EPSETUPSR = status;
    /* Clear the endpoint complete flag */
    status                              = ehciState->registerBase->EPCOMPLETE;
    ehciState->registerBase->EPCOMPLETE = status;

    do
    {
        /* Flush the pending transfers */
        ehciState->registerBase->EPFLUSH = USBHS_EPFLUSH_FERB_MASK | USBHS_EPFLUSH_FETB_MASK;
    } while (0U != (ehciState->registerBase->EPPRIME & (USBHS_EPPRIME_PERB_MASK | USBHS_EPPRIME_PETB_MASK)));

    /* Whether is the port reset. If yes, set the isResetting flag. Or, notify the up layer. */
    if (0U != (ehciState->registerBase->PORTSC1 & USBHS_PORTSC1_PR_MASK))
    {
        ehciState->isResetting = 1U;
    }
    else
    {
        usb_device_callback_message_struct_t message;
        message.buffer  = (uint8_t *)NULL;
        message.code    = (uint8_t)kUSB_DeviceNotifyBusReset;
        message.length  = 0U;
        message.isSetup = 0U;
        (void)USB_DeviceEhciNotification(ehciState, &message);
    }
}

#if (defined(USB_DEVICE_CONFIG_SOF_NOTIFICATIONS) && (USB_DEVICE_CONFIG_SOF_NOTIFICATIONS > 0U))
/*!
 * @brief Handle Start of Frame (SOF) Interrupt.
 *
 * The function is used to handle the SOF interrupt.
 *
 * @param ehciState       Pointer of the device EHCI state structure.
 *
 */
static void USB_DeviceEhciInterruptSOF(usb_device_ehci_state_struct_t *ehciState)
{
    usb_device_callback_message_struct_t message;

    message.buffer  = (uint8_t *)NULL;
    message.code    = (uint8_t)kUSB_DeviceNotifySOF;
    message.length  = 0U;
    message.isSetup = 0U;

    /* Notify upper layer */
    (void)USB_DeviceEhciNotification(ehciState, &message);
}
#endif /* USB_DEVICE_CONFIG_SOF_NOTIFICATIONS */

#if (defined(USB_DEVICE_CONFIG_LOW_POWER_MODE) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))
/*!
 * @brief Handle the suspend interrupt.
 *
 * The function is used to handle the suspend interrupt.
 *
 * @param ehciState       Pointer of the device EHCI state structure.
 *
 */
static void USB_DeviceEhciInterruptSuspend(usb_device_ehci_state_struct_t *ehciState)
{
    /* If the port is in suspend state, notify the up layer */
    if (0U != (ehciState->registerBase->PORTSC1 & USBHS_PORTSC1_SUSP_MASK))
    {
#if (defined(FSL_FEATURE_SOC_USBNC_COUNT) && (FSL_FEATURE_SOC_USBNC_COUNT > 0U))
#else
#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#if defined(FSL_FEATURE_USBHS_SUPPORT_EUSBn)
        if (0 == (int32_t)FSL_FEATURE_USBHS_SUPPORT_EUSBn(ehciState->registerBase))
        {
#endif
            if (0U != (ehciState->registerPhyBase->USB1_VBUS_DET_STAT & USBPHY_USB1_VBUS_DET_STAT_VBUS_VALID_3V_MASK))
#endif
#endif
        {
            usb_device_callback_message_struct_t message;
            message.buffer  = (uint8_t *)NULL;
            message.length  = 0U;
            message.isSetup = 0U;
            message.code    = (uint8_t)kUSB_DeviceNotifySuspend;
            (void)USB_DeviceEhciNotification(ehciState, &message);
        }
#if (defined(FSL_FEATURE_SOC_USBNC_COUNT) && (FSL_FEATURE_SOC_USBNC_COUNT > 0U))
#else
#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#if defined(FSL_FEATURE_USBHS_SUPPORT_EUSBn)
        }
#endif
#endif
#endif
    }
}

#if (defined(USB_DEVICE_CONFIG_LPM_L1) && (USB_DEVICE_CONFIG_LPM_L1 > 0U))
static usb_status_t USB_DeviceEhciInterruptLPMSleep(usb_device_ehci_state_struct_t *ehciState)
{
    usb_device_callback_message_struct_t message;

    message.buffer  = &ehciState->lpmRemoteWakeUp;
    message.code    = (uint8_t)kUSB_DeviceNotifyLPMSleep;
    message.length  = 0U;
    message.isSetup = 0U;

    /*TODO: maybe need 3us delay */

    ehciState->lpmRemoteWakeUp =
        (uint8_t)((ehciState->registerNcBase->LPM_CSR1 & USBNC_LPM_CSR1_LPM_DEV_RWKENRCVD_MASK) >>
                  USBNC_LPM_CSR1_LPM_DEV_RWKENRCVD_SHIFT);

    /* Notify up layer the USB suspend signal detected. */
    return USB_DeviceEhciNotification(ehciState, &message);
}
#endif

#endif /* USB_DEVICE_CONFIG_LOW_POWER_MODE */

/*!
 * @brief Get dtds and link to QH.
 *
 * The function is used to get dtds and link to QH.
 *
 * @param ehciState       Pointer of the device EHCI state structure.
 * @param endpointAddress The endpoint address, Bit7, 0U - USB_OUT, 1U - USB_IN.
 * @param buffer           The memory address needed to be transferred.
 * @param length           Data length.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
static usb_status_t USB_DeviceEhciTransfer(usb_device_ehci_state_struct_t *ehciState,
                                           uint8_t endpointAddress,
                                           uint8_t *buffer,
                                           uint32_t length)
{
    usb_device_ehci_dtd_struct_t *dtd;
    usb_device_ehci_dtd_struct_t *dtdHard;
    uint32_t index = (((uint32_t)endpointAddress & USB_ENDPOINT_NUMBER_MASK) << 1U) |
                     (((uint32_t)endpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >>
                      USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT);
    uint32_t primeBit = 1UL << ((endpointAddress & USB_ENDPOINT_NUMBER_MASK) +
                                ((endpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >> 0x03U));
    uint32_t epStatus = primeBit;
    uint32_t sendLength;
    uint32_t currentIndex       = 0U;
    uint32_t dtdRequestCount    = (length + USB_DEVICE_ECHI_DTD_TOTAL_BYTES - 1U) / USB_DEVICE_ECHI_DTD_TOTAL_BYTES;
    uint8_t qhIdle              = 0U;
    uint8_t waitingSafelyAccess = 1U;
    uint32_t primeTimesCount    = 0U;
/* CONFIG_UDC_DRIVER is for Zephyr, it will not be defined in NXP MCUXpresso SDK */
#if !((defined CONFIG_UDC_DRIVER) && (CONFIG_UDC_DRIVER))
    void *temp;
#endif
    OSA_SR_ALLOC();

    if (NULL == ehciState)
    {
        return kStatus_USB_InvalidHandle;
    }

    if (0U == ehciState->qh[index].endpointStatusUnion.endpointStatusBitmap.isOpened)
    {
        return kStatus_USB_Error;
    }
    /* Return error when ehci is doing reset */
    if (0U != ehciState->isResetting)
    {
        return kStatus_USB_Error;
    }

#if (defined(USB_DEVICE_CONFIG_BUFFER_PROPERTY_CACHEABLE) && (USB_DEVICE_CONFIG_BUFFER_PROPERTY_CACHEABLE > 0U))
    if (0U != length)
    {
        DCACHE_CleanByRange((uint32_t)buffer, length);
    }
#endif

    if (0U == dtdRequestCount)
    {
        dtdRequestCount = 1U;
    }

    OSA_ENTER_CRITICAL();
    /* The free dtd count need to not less than the transfer requests. */
    if (dtdRequestCount > (uint32_t)ehciState->dtdCount)
    {
        OSA_EXIT_CRITICAL();
        return kStatus_USB_Busy;
    }

    do
    {
        /* The transfer length need to not more than USB_DEVICE_ECHI_DTD_TOTAL_BYTES for each dtd. */
        if (length > USB_DEVICE_ECHI_DTD_TOTAL_BYTES)
        {
            sendLength = USB_DEVICE_ECHI_DTD_TOTAL_BYTES;
        }
        else
        {
            sendLength = length;
        }
        length -= sendLength;

        /* Get a free dtd */
        dtd = ehciState->dtdFree;

        ehciState->dtdFree = (usb_device_ehci_dtd_struct_t *)dtd->nextDtdPointer;
        ehciState->dtdCount--;

        /* Save the dtd head when current active buffer offset is zero. */
        if (0U == currentIndex)
        {
            dtdHard = dtd;
        }

        /* Set the dtd field */
        dtd->nextDtdPointer         = USB_DEVICE_ECHI_DTD_TERMINATE_MASK;
        dtd->dtdTokenUnion.dtdToken = 0U;
#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
        dtd->bufferPointerPage[0] = (uint32_t)USB_DEV_MEMORY_CPU_2_DMA((buffer + currentIndex));
#else
        dtd->bufferPointerPage[0] = (uint32_t)(buffer + currentIndex);
#endif
        dtd->bufferPointerPage[1] =
            (dtd->bufferPointerPage[0] + USB_DEVICE_ECHI_DTD_PAGE_BLOCK) & USB_DEVICE_ECHI_DTD_PAGE_MASK;
        dtd->bufferPointerPage[2] = dtd->bufferPointerPage[1] + USB_DEVICE_ECHI_DTD_PAGE_BLOCK;
        dtd->bufferPointerPage[3] = dtd->bufferPointerPage[2] + USB_DEVICE_ECHI_DTD_PAGE_BLOCK;
        dtd->bufferPointerPage[4] = dtd->bufferPointerPage[3] + USB_DEVICE_ECHI_DTD_PAGE_BLOCK;

        dtd->dtdTokenUnion.dtdTokenBitmap.totalBytes = sendLength;

        /* Save the data length needed to be transferred. */
        dtd->reservedUnion.originalBufferInfo.originalBufferLength = sendLength;
        /* Save the original buffer address */

        dtd->reservedUnion.originalBufferInfo.originalBufferOffest =
#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
            ((uint32_t)(buffer + currentIndex)) & USB_DEVICE_ECHI_DTD_PAGE_OFFSET_MASK;
#else
            dtd->bufferPointerPage[0] & USB_DEVICE_ECHI_DTD_PAGE_OFFSET_MASK;
#endif
        dtd->reservedUnion.originalBufferInfo.dtdInvalid = 0U;

        /* Set the IOC field in last dtd. */
        if (0U == length)
        {
            dtd->dtdTokenUnion.dtdTokenBitmap.ioc = 1U;
        }

        /* Set dtd active */
        dtd->dtdTokenUnion.dtdTokenBitmap.status = USB_DEVICE_ECHI_DTD_STATUS_ACTIVE;

        /* Move the buffer offset index */
        currentIndex += sendLength;

        /* Add dtd to the in-used dtd queue */
        if (NULL != (ehciState->dtdTail[index]))
        {
#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
            ehciState->dtdTail[index]->nextDtdPointer = (uint32_t)USB_DEV_MEMORY_CPU_2_DMA(dtd);
#else
            ehciState->dtdTail[index]->nextDtdPointer = (uint32_t)dtd;
#endif
            ehciState->dtdTail[index] = dtd;
        }
        else
        {
            ehciState->dtdHard[index] = dtd;
            ehciState->dtdTail[index] = dtd;
            qhIdle                    = 1U;
        }
    } while (0U != length);
/* CONFIG_UDC_DRIVER is for Zephyr, it will not be defined in NXP MCUXpresso SDK */
#if !((defined CONFIG_UDC_DRIVER) && (CONFIG_UDC_DRIVER))
    if ((USB_CONTROL_ENDPOINT == (endpointAddress & USB_ENDPOINT_NUMBER_MASK)) &&
        (USB_IN == ((endpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >>
                    USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT)))
    {
        uint8_t setupindex = ((endpointAddress & USB_ENDPOINT_NUMBER_MASK) * 2U);
        /* Get last setup packet */
        temp                            = (void *)&ehciState->qh[setupindex].setupBufferBack[0];
        usb_setup_struct_t *deviceSetup = (usb_setup_struct_t *)temp;
        if (1U == ehciState->qh[index].endpointStatusUnion.endpointStatusBitmap.zlt)
        {
            if ((0U != sendLength) && (sendLength < deviceSetup->wLength) &&
                (0U ==
                 (sendLength % ehciState->qh[index]
                                   .capabilttiesCharacteristicsUnion.capabilttiesCharacteristicsBitmap.maxPacketSize)))
            {
                /* enable ZLT. */
                ehciState->qh[index].capabilttiesCharacteristicsUnion.capabilttiesCharacteristicsBitmap.zlt = 0U;
            }
        }
    }
#endif
    /* If the QH is not empty */
    if (0U == qhIdle)
    {
        /* If the prime bit is set, nothing need to do. */
        if (0U != (ehciState->registerBase->EPPRIME & primeBit))
        {
            OSA_EXIT_CRITICAL();
            return kStatus_USB_Success;
        }

        /* To safely a dtd */
        while (0U != waitingSafelyAccess)
        {
            /* set the ATDTW flag to USBHS_USBCMD_REG. */
            ehciState->registerBase->USBCMD |= USBHS_USBCMD_ATDTW_MASK;
            /* Read EPSR */
            epStatus = ehciState->registerBase->EPSR;
            /* Wait the ATDTW bit set */
            if (0U != (ehciState->registerBase->USBCMD & USBHS_USBCMD_ATDTW_MASK))
            {
                waitingSafelyAccess = 0U;
            }
        }
        /* Clear the ATDTW bit */
        ehciState->registerBase->USBCMD &= ~USBHS_USBCMD_ATDTW_MASK;
    }

    /* If QH is empty or the endpoint is not primed, need to link current dtd head to the QH. */
    /* When the endpoint is not primed if qhIdle is zero, it means the QH is empty. */
    if ((0U != qhIdle) || (0U == (epStatus & primeBit)))
    {
#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
        ehciState->qh[index].nextDtdPointer = (uint32_t)USB_DEV_MEMORY_CPU_2_DMA(dtdHard);
#else
        ehciState->qh[index].nextDtdPointer = (uint32_t)dtdHard;
#endif
        ehciState->qh[index].dtdTokenUnion.dtdToken = 0U;
        /*make sure dtd is linked to dqh*/
        __DSB();
        ehciState->registerBase->EPPRIME = primeBit;
        while (0U == (ehciState->registerBase->EPSR & primeBit))
        {
            primeTimesCount++;
            if (primeTimesCount == USB_DEVICE_MAX_TRANSFER_PRIME_TIMES)
            {
                OSA_EXIT_CRITICAL();
                return kStatus_USB_Error;
            }
            if (0U != (ehciState->registerBase->EPCOMPLETE & primeBit))
            {
                break;
            }
            else
            {
                ehciState->registerBase->EPPRIME = primeBit;
            }
        }
    }

    OSA_EXIT_CRITICAL();
    return kStatus_USB_Success;
}

/*!
 * @brief Get a valid device EHCI state for the device EHCI instance.
 *
 * This function gets a valid device EHCI state for the USB device EHCI module specified by the controllerId.
 *
 * @param instanceIndex The instanceIndex is used for other EHCI device structure to identify their instance index.
 *
 * @return A valid EHCI state or NULL.
 */
static void *USB_EhciGetValidEhciState(uint8_t *instanceIndex)
{
    for (uint8_t instance = 0; instance < USB_DEVICE_CONFIG_EHCI; instance++)
    {
        if (0U == g_UsbDeviceEhciStateStatus[instance])
        {
            g_UsbDeviceEhciStateStatus[instance] = 1U;
            *instanceIndex                       = instance;
            return (void *)(&g_UsbDeviceEhciState[instance]);
        }
    }
    return NULL;
}

#if (defined(USB_DEVICE_CONFIG_CHARGER_DETECT) && (USB_DEVICE_CONFIG_CHARGER_DETECT > 0U)) && \
    (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U))
/* The device dcd callback */
static usb_hsdcd_status_t USB_DeviceEhciIsrHSDCDCallback(void *handle, uint32_t event, void *param)
{
    usb_hsdcd_status_t error = kStatus_hsdcd_Success;
    usb_device_callback_message_struct_t message;
    usb_device_ehci_state_struct_t *ehciState = (usb_device_ehci_state_struct_t *)handle;

    if (ehciState == NULL)
    {
        return kStatus_hsdcd_Error;
    }

    /*messsgae buffer contain event information*/
    message.buffer  = (uint8_t *)param;
    message.length  = 0U;
    message.isSetup = 0U;
    message.code    = (uint8_t)kUSB_DeviceNotifyDcdDetectFinished;
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    if (kStatus_USB_Success != USB_DeviceEhciNotification(ehciState, &message))
    {
        return kStatus_hsdcd_Error;
    }
#else
    (void)USB_DeviceEhciNotification(ehciState, &message);
#endif

    return error;
}

void USB_DeviceEhciIsrHSDCDFunction(void *deviceHandle)
{
    usb_device_struct_t *handle = (usb_device_struct_t *)deviceHandle;
    usb_device_ehci_state_struct_t *ehciState;
    if (NULL == deviceHandle)
    {
        return;
    }
    ehciState = (usb_device_ehci_state_struct_t *)(handle->controllerHandle);
    USB_HSDcdIsrFunction(ehciState->dcdHandle);
}
#elif (defined(USB_DEVICE_CONFIG_CHARGER_DETECT) && (USB_DEVICE_CONFIG_CHARGER_DETECT > 0U)) && \
    (defined(FSL_FEATURE_SOC_USB_ANALOG_COUNT) && (FSL_FEATURE_SOC_USB_ANALOG_COUNT > 0U))
/* The device dcd callback */
static usb_phydcd_status_t USB_DeviceEhciIsrPHYDCDCallback(void *handle, uint32_t event, void *param)
{
    usb_phydcd_status_t error = kStatus_phydcd_Success;
    usb_device_callback_message_struct_t message;
    usb_device_ehci_state_struct_t *ehciState = (usb_device_ehci_state_struct_t *)handle;

    if (ehciState == NULL)
    {
        return kStatus_phydcd_Error;
    }

    /*messsgae buffer contain event information*/
    message.buffer  = (uint8_t *)param;
    message.length  = 0U;
    message.isSetup = 0U;
    message.code    = (uint8_t)kUSB_DeviceNotifyDcdDetectFinished;
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    if (kStatus_USB_Success != USB_DeviceEhciNotification(ehciState, &message))
    {
        return kStatus_phydcd_Error;
    }
#else
    (void)USB_DeviceEhciNotification(ehciState, &message);
#endif

    return error;
}
#endif

/*!
 * @brief Notify the device that the controller status changed.
 *
 * This function is used to notify the device that the controller status changed.
 *
 * @param ehciState              Pointer of the device EHCI state structure.
 * @param message                The device callback message handle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
static usb_status_t USB_DeviceEhciNotification(usb_device_ehci_state_struct_t *ehciState,
                                               usb_device_callback_message_struct_t *message)
{
    usb_status_t error;

#if (defined(USB_DEVICE_CONFIG_BUFFER_PROPERTY_CACHEABLE) && (USB_DEVICE_CONFIG_BUFFER_PROPERTY_CACHEABLE > 0U))
    /* If valid OUT packet, invalidate data in cache */
    if ((message->code < kUSB_DeviceNotifyBusReset) && (message->buffer != NULL) && (message->length != 0U) &&
        (message->length != USB_CANCELLED_TRANSFER_LENGTH))
    {
        DCACHE_InvalidateByRange((uint32_t)message->buffer, message->length);
    }
#endif

    error = USB_DeviceNotificationTrigger(ehciState->deviceHandle, message);
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    if (kStatus_USB_Success != error)
    {
#if (defined(DEVICE_ECHO) && (DEVICE_ECHO > 0U))
        usb_echo("notification error\n");
#endif
    }
#else
    error                = kStatus_USB_Success;
#endif

    return error;
}

/*!
 * @brief Initialize the USB device EHCI instance.
 *
 * This function initializes the USB device EHCI module specified by the controllerId.
 *
 * @param controllerId The controller id of the USB IP. Please refer to enumeration type usb_controller_index_t.
 * @param handle        Pointer of the device handle, used to identify the device object is belonged to.
 * @param ehciHandle   It is out parameter, is used to return pointer of the device EHCI handle to the caller.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceEhciInit(uint8_t controllerId,
                                usb_device_handle handle,
                                usb_device_controller_handle *ehciHandle)
{
    usb_device_ehci_state_struct_t *ehciState = NULL;
#if defined(USBHS_STACK_BASE_ADDRS)
    uint32_t ehci_base[] = USBHS_STACK_BASE_ADDRS;
#else
    uint32_t ehci_base[] = USBHS_BASE_ADDRS;
#endif
#if (defined(USB_DEVICE_CONFIG_LOW_POWER_MODE) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))
#if (defined(FSL_FEATURE_SOC_USBNC_COUNT) && (FSL_FEATURE_SOC_USBNC_COUNT > 0U))
#if defined(USBNC_STACK_BASE_ADDRS)
    uint32_t usbnc_base[] = USBNC_STACK_BASE_ADDRS;
#else
    uint32_t usbnc_base[] = USBNC_BASE_ADDRS;
#endif
#endif
#endif
    uint8_t intanceIndex;
    void *temp;
#if (defined(USB_DEVICE_CONFIG_CHARGER_DETECT) && (USB_DEVICE_CONFIG_CHARGER_DETECT > 0U)) && \
    ((defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U)) ||    \
     (defined(FSL_FEATURE_SOC_USB_ANALOG_COUNT) && (FSL_FEATURE_SOC_USB_ANALOG_COUNT > 0U)))

    usb_device_callback_message_struct_t message;
#endif

#if (defined(USB_DEVICE_CONFIG_CHARGER_DETECT) && (USB_DEVICE_CONFIG_CHARGER_DETECT > 0U)) && \
    (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U))
#if defined(USBHSDCD_STACK_BASE_ADDRS)
    uint32_t hsdcd_base[] = USBHSDCD_STACK_BASE_ADDRS;
#else
    uint32_t hsdcd_base[] = USBHSDCD_BASE_ADDRS;
#endif
    USBHSDCD_Type *base;
    usb_hsdcd_config_struct_t dcdParamConfig;
    usb_hsdcd_status_t dcdError = kStatus_hsdcd_Success;
#elif (defined(USB_DEVICE_CONFIG_CHARGER_DETECT) && (USB_DEVICE_CONFIG_CHARGER_DETECT > 0U)) && \
    ((defined FSL_FEATURE_SOC_USB_ANALOG_COUNT) && (FSL_FEATURE_SOC_USB_ANALOG_COUNT > 0U))

    uint8_t index;
    usb_phydcd_config_struct_t phyDcdParamConfig;
    usb_phydcd_status_t phyDcdError = kStatus_phydcd_Success;
#endif
#if (defined(FSL_FEATURE_USB_ATLANTIC_EHCI_SUPPORT) && (FSL_FEATURE_USB_ATLANTIC_EHCI_SUPPORT > 0U))
    uint32_t delay;
#endif

    if ((controllerId < (uint8_t)kUSB_ControllerEhci0) ||
        ((uint32_t)((uint32_t)controllerId - (uint32_t)kUSB_ControllerEhci0) >= (sizeof(ehci_base) / sizeof(uint32_t))))
    {
        return kStatus_USB_ControllerNotFound;
    }

    ehciState = USB_EhciGetValidEhciState(&intanceIndex);
    if (NULL == ehciState)
    {
        return kStatus_USB_InvalidHandle;
    }
    ehciState->dtd = s_UsbDeviceEhciDtd[intanceIndex];
    temp           = (void *)&qh_buffer[intanceIndex * 2048U];
    ehciState->qh  = (usb_device_ehci_qh_struct_t *)temp;

    ehciState->controllerId = controllerId;

    ehciState->registerBase = (USBHS_Type *)ehci_base[controllerId - (uint8_t)kUSB_ControllerEhci0];
#if (defined(USB_DEVICE_CONFIG_LOW_POWER_MODE) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))
#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#if defined(FSL_FEATURE_USBHS_SUPPORT_EUSBn)
    if (0 == (int32_t)FSL_FEATURE_USBHS_SUPPORT_EUSBn(ehciState->registerBase))
#endif
    {
        ehciState->registerPhyBase = (USBPHY_Type *)USB_EhciPhyGetBase(controllerId);
    }
#endif
#if (defined(FSL_FEATURE_SOC_USBNC_COUNT) && (FSL_FEATURE_SOC_USBNC_COUNT > 0U))
    ehciState->registerNcBase =
        (USBNC_Type *)USB_EhciGetBase(controllerId, &usbnc_base[0], sizeof(usbnc_base) / sizeof(uint32_t));
#endif

#endif
    /* For eUSB, do not need to reset controller. */
#if defined(FSL_FEATURE_USBHS_SUPPORT_EUSBn)
    if (0 == (int32_t)FSL_FEATURE_USBHS_SUPPORT_EUSBn(ehciState->registerBase))
#endif
    {
        /* Reset the controller. */
        ehciState->registerBase->USBCMD |= USBHS_USBCMD_RST_MASK;
        while (0U != (ehciState->registerBase->USBCMD & USBHS_USBCMD_RST_MASK))
        {
        }
    }

    /* Get the HW's endpoint count */
    ehciState->endpointCount =
        (uint8_t)((ehciState->registerBase->DCCPARAMS & USBHS_DCCPARAMS_DEN_MASK) >> USBHS_DCCPARAMS_DEN_SHIFT);

    if (ehciState->endpointCount < USB_DEVICE_CONFIG_ENDPOINTS)
    {
        return kStatus_USB_Error;
    }
    ehciState->deviceHandle = (usb_device_struct_t *)handle;

    /* Clear the controller mode field and set to device mode. */
    ehciState->registerBase->USBMODE &= ~USBHS_USBMODE_CM_MASK;
    ehciState->registerBase->USBMODE |= USBHS_USBMODE_CM(0x02U);
#if (defined(FSL_FEATURE_USB_ATLANTIC_EHCI_SUPPORT) && (FSL_FEATURE_USB_ATLANTIC_EHCI_SUPPORT > 0U))
    /* Compilance margin settings recommended by validation team */
    /* Adjust PLL VCO calibration voltage to 3 (1.1V) */
    ehciState->registerBase->CALIBRATION_CONTROL &= ~(USBC_CALIBRATION_CONTROL_PLLCAL_MASK);
    ehciState->registerBase->CALIBRATION_CONTROL |=
        ((3U << USBC_CALIBRATION_CONTROL_PLLCAL_SHIFT) | USBC_CALIBRATION_CONTROL_PLLCAL_START_MASK);
    delay = USB_PHY_POLLING_DELAY;
    /* Verify PLL calibration has been done (max time between CAL_START and CAL_DONE is 500us) */
    while ((delay--) && (!(ehciState->registerBase->CALIBRATION_CONTROL & USBC_CALIBRATION_CONTROL_PLLCAL_DONE_MASK)))
    {
    }
    ehciState->registerBase->CALIBRATION_CONTROL &= ~(USBC_CALIBRATION_CONTROL_PLLCAL_START_MASK);
    /* Adjust impedance calibration threshold IMPCAL_VTH to 7 (900/23=39 ohms) */
    ehciState->registerBase->CALIBRATION_CONTROL &= ~(USBC_CALIBRATION_CONTROL_IMPCAL_VTH_MASK);
    ehciState->registerBase->CALIBRATION_CONTROL |=
        ((7U << USBC_CALIBRATION_CONTROL_IMPCAL_VTH_SHIFT) | USBC_CALIBRATION_CONTROL_IMPCAL_START_MASK);
    delay = USB_PHY_POLLING_DELAY;
    /* Verify impedance calibration has been done (max time between CAL_START and CAL_DONE is 500us) */
    while ((delay--) && (!(ehciState->registerBase->CALIBRATION_CONTROL & USBC_CALIBRATION_CONTROL_IMPCAL_DONE_MASK)))
    {
    }
    ehciState->registerBase->CALIBRATION_CONTROL &= ~(USBC_CALIBRATION_CONTROL_IMPCAL_START_MASK);
    /* Fixed the static value for impedance using Bit 11 and Bit [3:0] */
    ehciState->registerBase->CALIBRATION_CONTROL |= USBC_CALIBRATION_CONTROL_EXT_FS_IMP_EN_MASK;
    /* Adjust HS_SR_SEL[1:0] to b'11 */
    ehciState->registerBase->TX_CHANNEL_CONTRL_0 |= USBC_TX_CHANNEL_CONTRL_0_HS_SR_SEL_MASK;
#endif

    /* Set the EHCI to default status. */
    USB_DeviceEhciSetDefaultState(ehciState);
    *ehciHandle = (usb_device_controller_handle)ehciState;
#if (defined(USB_DEVICE_CONFIG_CHARGER_DETECT) && (USB_DEVICE_CONFIG_CHARGER_DETECT > 0U)) && \
    (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U))
    base = (USBHSDCD_Type *)USB_EhciGetBase(controllerId, &hsdcd_base[0], sizeof(hsdcd_base) / sizeof(uint32_t));
    dcdParamConfig.dcdCallback      = USB_DeviceEhciIsrHSDCDCallback;
    dcdParamConfig.dcdCallbackParam = (void *)ehciState;
    dcdError                        = USB_HSDCD_Init(base, &dcdParamConfig, &ehciState->dcdHandle);
    if (kStatus_hsdcd_Success != dcdError)
    {
        return kStatus_USB_Error;
    }

    if (0U != (ehciState->registerBase->OTGSC & USBHS_OTGSC_BSV_MASK))
    {
        /* Device is connected to a host. */
        message.code = (uint8_t)kUSB_DeviceNotifyAttach;
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
        if ((kStatus_USB_Success != USB_DeviceEhciNotification(ehciState, &message)) ||
            (kStatus_hsdcd_Success != USB_HSDCD_Control(ehciState->dcdHandle, kUSB_DeviceHSDcdRun, NULL)))
        {
            return kStatus_USB_Error;
        }
#else
        (void)USB_DeviceEhciNotification(ehciState, &message);
        (void)USB_HSDCD_Control(ehciState->dcdHandle, kUSB_DeviceHSDcdRun, NULL);
#endif
    }
#elif (defined(USB_DEVICE_CONFIG_CHARGER_DETECT) && (USB_DEVICE_CONFIG_CHARGER_DETECT > 0U)) && \
    (defined(FSL_FEATURE_SOC_USB_ANALOG_COUNT) && (FSL_FEATURE_SOC_USB_ANALOG_COUNT > 0U))

    index = controllerId - (uint8_t)kUSB_ControllerEhci0;

    phyDcdParamConfig.dcdCallback      = USB_DeviceEhciIsrPHYDCDCallback;
    phyDcdParamConfig.dcdCallbackParam = (void *)ehciState;

    phyDcdError =
        USB_PHYDCD_Init(index, (usb_phydcd_config_struct_t *)&phyDcdParamConfig, (void *)&ehciState->dcdHandle);
    if (kStatus_phydcd_Success != phyDcdError)
    {
        return kStatus_USB_Error;
    }

    if (0U != (ehciState->registerBase->OTGSC & USBHS_OTGSC_BSV_MASK))
    {
        /* Device is connected to a host. */
        message.code = (uint8_t)kUSB_DeviceNotifyAttach;
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
        if ((kStatus_USB_Success != USB_DeviceEhciNotification(ehciState, &message)) ||
            (kStatus_USB_Success != USB_HSDCD_Control(ehciState->dcdHandle, kUSB_DevicePHYDcdRun, NULL)))
        {
            return kStatus_USB_Error;
        }
#else
        (void)USB_DeviceEhciNotification(ehciState, &message);
        (void)USB_PHYDCD_Control(ehciState->dcdHandle, kUSB_DevicePHYDcdRun, NULL);
#endif
    }
#endif

    return kStatus_USB_Success;
}

/*!
 * @brief De-initialize the USB device EHCI instance.
 *
 * This function de-initializes the USB device EHCI module.
 *
 * @param ehciHandle   Pointer of the device EHCI handle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceEhciDeinit(usb_device_controller_handle ehciHandle)
{
    usb_device_ehci_state_struct_t *ehciState = (usb_device_ehci_state_struct_t *)ehciHandle;

    if (NULL == ehciHandle)
    {
        return kStatus_USB_InvalidHandle;
    }
    for (uint8_t instance = 0; instance < USB_DEVICE_CONFIG_EHCI; instance++)
    {
        if (ehciState == &g_UsbDeviceEhciState[instance])
        {
            g_UsbDeviceEhciStateStatus[instance] = 0;
        }
    }

    /* Disable all interrupt. */
    ehciState->registerBase->USBINTR = 0U;
    /* Stop the device functionality. */
    ehciState->registerBase->USBCMD &= ~USBHS_USBCMD_RS_MASK;
    /* Reset the controller. */
    ehciState->registerBase->USBCMD |= USBHS_USBCMD_RST_MASK;

#if (defined(USB_DEVICE_CONFIG_CHARGER_DETECT) && (USB_DEVICE_CONFIG_CHARGER_DETECT > 0U)) && \
    (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U))
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    if (kStatus_hsdcd_Success != USB_HSDCD_Deinit(ehciState->dcdHandle))
    {
        return kStatus_USB_Error;
    }
#else
    (void)USB_HSDCD_Deinit(ehciState->dcdHandle);
#endif
#elif (defined(USB_DEVICE_CONFIG_CHARGER_DETECT) && (USB_DEVICE_CONFIG_CHARGER_DETECT > 0U)) && \
    (defined(FSL_FEATURE_SOC_USB_ANALOG_COUNT) && (FSL_FEATURE_SOC_USB_ANALOG_COUNT > 0U))
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    if (kStatus_USB_Success != USB_PHYDCD_Deinit(ehciState->dcdHandle))
    {
        return kStatus_USB_Error;
    }
#else
    (void)USB_PHYDCD_Deinit(ehciState->dcdHandle);
#endif
#endif

    return kStatus_USB_Success;
}

/*!
 * @brief Send data through a specified endpoint.
 *
 * This function sends data through a specified endpoint.
 *
 * @param ehciHandle      Pointer of the device EHCI handle.
 * @param endpointAddress Endpoint index.
 * @param buffer           The memory address to hold the data need to be sent.
 * @param length           The data length need to be sent.
 *
 * @return A USB error code or kStatus_USB_Success.
 *
 * @note The return value just means if the sending request is successful or not; the transfer done is notified by the
 * corresponding callback function.
 * Currently, only one transfer request can be supported for one specific endpoint.
 * If there is a specific requirement to support multiple transfer requests for one specific endpoint, the application
 * should implement a queue in the application level.
 * The subsequent transfer could begin only when the previous transfer is done (get notification through the endpoint
 * callback).
 */
usb_status_t USB_DeviceEhciSend(usb_device_controller_handle ehciHandle,
                                uint8_t endpointAddress,
                                uint8_t *buffer,
                                uint32_t length)
{
    /* Add dtd to the QH */
    return USB_DeviceEhciTransfer(
        (usb_device_ehci_state_struct_t *)ehciHandle,
        (endpointAddress & USB_ENDPOINT_NUMBER_MASK) | (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT),
        buffer, length);
}

/*!
 * @brief Receive data through a specified endpoint.
 *
 * This function Receives data through a specified endpoint.
 *
 * @param ehciHandle      Pointer of the device EHCI handle.
 * @param endpointAddress Endpoint index.
 * @param buffer           The memory address to save the received data.
 * @param length           The data length want to be received.
 *
 * @return A USB error code or kStatus_USB_Success.
 *
 * @note The return value just means if the receiving request is successful or not; the transfer done is notified by the
 * corresponding callback function.
 * Currently, only one transfer request can be supported for one specific endpoint.
 * If there is a specific requirement to support multiple transfer requests for one specific endpoint, the application
 * should implement a queue in the application level.
 * The subsequent transfer could begin only when the previous transfer is done (get notification through the endpoint
 * callback).
 */
usb_status_t USB_DeviceEhciRecv(usb_device_controller_handle ehciHandle,
                                uint8_t endpointAddress,
                                uint8_t *buffer,
                                uint32_t length)
{
    /* Add dtd to the QH */
    return USB_DeviceEhciTransfer(
        (usb_device_ehci_state_struct_t *)ehciHandle,
        (endpointAddress & USB_ENDPOINT_NUMBER_MASK) | (USB_OUT << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT),
        buffer, length);
}

/*!
 * @brief Cancel the pending transfer in a specified endpoint.
 *
 * The function is used to cancel the pending transfer in a specified endpoint.
 *
 * @param ehciHandle      Pointer of the device EHCI handle.
 * @param ep               Endpoint address, bit7 is the direction of endpoint, 1U - IN, 0U - OUT.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceEhciCancel(usb_device_controller_handle ehciHandle, uint8_t ep)
{
    usb_device_ehci_state_struct_t *ehciState = (usb_device_ehci_state_struct_t *)ehciHandle;
    usb_device_callback_message_struct_t message;
    usb_device_ehci_dtd_struct_t *currentDtd;
    uint32_t primeBit =
        1UL << ((ep & USB_ENDPOINT_NUMBER_MASK) + ((ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >> 0x03U));
    uint8_t index =
        ((ep & USB_ENDPOINT_NUMBER_MASK) << 1U) | ((ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >> 0x07U);
    uint8_t flag = 0;
#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
    uint32_t convert_addr = 0U;
#endif

    OSA_SR_ALLOC();

    if (NULL == ehciHandle)
    {
        return kStatus_USB_InvalidHandle;
    }

    OSA_ENTER_CRITICAL();

    message.buffer = NULL;
    message.length = USB_CANCELLED_TRANSFER_LENGTH;

    /* Get the first dtd */
    currentDtd =
        (usb_device_ehci_dtd_struct_t *)((uint32_t)ehciState->dtdHard[index] & USB_DEVICE_ECHI_DTD_POINTER_MASK);

    /* In the next loop, USB_DeviceNotificationTrigger function may trigger a new transfer and the context always
     * keep in the critical section, so the Dtd sequence would still keep non-empty and the loop would be endless.
     * We set the Dtd's dtdInvalid in this while and add an if statement in the next loop so that this issue could
     * be fixed.
     */
    while (NULL != currentDtd)
    {
        currentDtd->reservedUnion.originalBufferInfo.dtdInvalid = 1U;
#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
        currentDtd = (usb_device_ehci_dtd_struct_t *)USB_DEV_MEMORY_DMA_2_CPU(currentDtd->nextDtdPointer &
                                                                              USB_DEVICE_ECHI_DTD_POINTER_MASK);
#else
        currentDtd = (usb_device_ehci_dtd_struct_t *)(currentDtd->nextDtdPointer & USB_DEVICE_ECHI_DTD_POINTER_MASK);
#endif
    }

    /* Get the first dtd */
    currentDtd =
        (usb_device_ehci_dtd_struct_t *)((uint32_t)ehciState->dtdHard[index] & USB_DEVICE_ECHI_DTD_POINTER_MASK);
    while (NULL != currentDtd)
    {
        /* this if statement is used with  the previous while loop to avoid the endless loop */
        if (0U == currentDtd->reservedUnion.originalBufferInfo.dtdInvalid)
        {
            break;
        }
        else
        {
            if (0U != (currentDtd->dtdTokenUnion.dtdTokenBitmap.status & USB_DEVICE_ECHI_DTD_STATUS_ACTIVE))
            {
                /* Flush the endpoint to stop a transfer. */
                do
                {
                    /* Set the corresponding bit(s) in the EPFLUSH register */
                    ehciState->registerBase->EPFLUSH |= primeBit;

                    /* Wait until all bits in the EPFLUSH register are cleared. */
                    while (0U != (ehciState->registerBase->EPFLUSH & primeBit))
                    {
                    }
                    /*
                     * Read the EPSR register to ensure that for all endpoints
                     * commanded to be flushed, that the corresponding bits
                     * are now cleared.
                     */
                } while (0U != (ehciState->registerBase->EPSR & primeBit));
            }

            /* Save the original buffer address. */
            if (NULL == message.buffer)
            {
#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
                convert_addr   = (uint32_t)USB_DEV_MEMORY_DMA_2_CPU(currentDtd->bufferPointerPage[0]);
                message.buffer = (uint8_t *)((convert_addr & USB_DEVICE_ECHI_DTD_PAGE_MASK) |
                                             (currentDtd->reservedUnion.originalBufferInfo.originalBufferOffest));
#else
                message.buffer = (uint8_t *)((currentDtd->bufferPointerPage[0] & USB_DEVICE_ECHI_DTD_PAGE_MASK) |
                                             (currentDtd->reservedUnion.originalBufferInfo.originalBufferOffest));
#endif
            }

            /* Remove the dtd from the dtd in-used queue. */
            if (ehciState->dtdHard[index] == ehciState->dtdTail[index])
            {
                ehciState->dtdHard[index] = NULL;
                ehciState->dtdTail[index] = NULL;
            }
            else
            {
#if defined FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET && FSL_FEATURE_MEMORY_HAS_ADDRESS_OFFSET
                ehciState->dtdHard[index] =
                    (usb_device_ehci_dtd_struct_t *)USB_DEV_MEMORY_DMA_2_CPU(ehciState->dtdHard[index]->nextDtdPointer);
#else
                ehciState->dtdHard[index] = (usb_device_ehci_dtd_struct_t *)ehciState->dtdHard[index]->nextDtdPointer;
#endif
            }

            /* When the ioc is set or the dtd queue is empty, the up layer will be notified. */
            if ((0U != currentDtd->dtdTokenUnion.dtdTokenBitmap.ioc) ||
                (0U == ((uint32_t)ehciState->dtdHard[index] & USB_DEVICE_ECHI_DTD_POINTER_MASK)))
            {
                flag = 1;
            }
            /* Clear the token field. */
            currentDtd->dtdTokenUnion.dtdToken = 0U;
            /* Save the dtd to the free queue. */
            currentDtd->nextDtdPointer = (uint32_t)ehciState->dtdFree;
            ehciState->dtdFree         = currentDtd;
            ehciState->dtdCount++;
        }
        /* Get the next dtd. */
        currentDtd =
            (usb_device_ehci_dtd_struct_t *)((uint32_t)ehciState->dtdHard[index] & USB_DEVICE_ECHI_DTD_POINTER_MASK);
    }
    if (NULL == currentDtd)
    {
        /* Set the QH to empty. */
        ehciState->qh[index].nextDtdPointer         = USB_DEVICE_ECHI_DTD_TERMINATE_MASK;
        ehciState->qh[index].dtdTokenUnion.dtdToken = 0U;
    }
    OSA_EXIT_CRITICAL();

    if (0U != flag)
    {
        message.code    = ep;
        message.isSetup = 0U;
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
        if (kStatus_USB_Success != USB_DeviceEhciNotification(ehciState, &message))
        {
            return kStatus_USB_Error;
        }
#else
        (void)USB_DeviceEhciNotification(ehciState, &message);
#endif
        message.buffer = NULL;
    }

    return kStatus_USB_Success;
}

/*!
 * @brief Control the status of the selected item.
 *
 * The function is used to control the status of the selected item.
 *
 * @param ehciHandle      Pointer of the device EHCI handle.
 * @param type             The selected item. Please refer to enumeration type usb_device_control_type_t.
 * @param param            The param type is determined by the selected item.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceEhciControl(usb_device_controller_handle ehciHandle, usb_device_control_type_t type, void *param)
{
    usb_device_ehci_state_struct_t *ehciState = (usb_device_ehci_state_struct_t *)ehciHandle;
    usb_status_t error                        = kStatus_USB_Error;
#if defined(USB_DEVICE_CONFIG_GET_SOF_COUNT) && (USB_DEVICE_CONFIG_GET_SOF_COUNT > 0U)
    uint32_t *temp32;
#endif
    uint16_t *temp16;
    uint8_t *temp8;
#if (defined(USB_DEVICE_CONFIG_LOW_POWER_MODE) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))
    OSA_SR_ALLOC();
#endif

#if ((defined(USB_DEVICE_CONFIG_REMOTE_WAKEUP)) && (USB_DEVICE_CONFIG_REMOTE_WAKEUP > 0U))
    usb_device_struct_t *deviceHandle;
#endif
#if (defined(USB_DEVICE_CONFIG_LOW_POWER_MODE) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))
#if ((defined(USB_DEVICE_CONFIG_REMOTE_WAKEUP)) && (USB_DEVICE_CONFIG_REMOTE_WAKEUP > 0U))
    uint64_t startTick;
#endif
#endif

    if (NULL == ehciHandle)
    {
        return kStatus_USB_InvalidHandle;
    }

#if ((defined(USB_DEVICE_CONFIG_REMOTE_WAKEUP)) && (USB_DEVICE_CONFIG_REMOTE_WAKEUP > 0U))
    deviceHandle = (usb_device_struct_t *)ehciState->deviceHandle;
#endif

    switch (type)
    {
        case kUSB_DeviceControlRun:
            ehciState->registerBase->USBCMD |= USBHS_USBCMD_RS_MASK;
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceControlStop:
            ehciState->registerBase->USBCMD &= ~USBHS_USBCMD_RS_MASK;
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceControlEndpointInit:
            if (NULL != param)
            {
                error = USB_DeviceEhciEndpointInit(ehciState, (usb_device_endpoint_init_struct_t *)param);
            }
            break;
        case kUSB_DeviceControlEndpointDeinit:
            if (NULL != param)
            {
                temp8 = (uint8_t *)param;
                error = USB_DeviceEhciEndpointDeinit(ehciState, *temp8);
            }
            break;
        case kUSB_DeviceControlEndpointStall:
            if (NULL != param)
            {
                temp8 = (uint8_t *)param;
                error = USB_DeviceEhciEndpointStall(ehciState, *temp8);
            }
            break;
        case kUSB_DeviceControlEndpointUnstall:
            if (NULL != param)
            {
                temp8 = (uint8_t *)param;
                error = USB_DeviceEhciEndpointUnstall(ehciState, *temp8);
            }
            break;
        case kUSB_DeviceControlGetDeviceStatus:
            if (NULL != param)
            {
                temp16  = (uint16_t *)param;
#if defined(USB_DEVICE_CONFIG_SELF_POWER) && (USB_DEVICE_CONFIG_SELF_POWER > 0U)
                *temp16 = ((uint16_t)1U
#else
                *temp16 = ((uint16_t)0U
#endif
                           << (USB_REQUEST_STANDARD_GET_STATUS_DEVICE_SELF_POWERED_SHIFT))
#if ((defined(USB_DEVICE_CONFIG_REMOTE_WAKEUP)) && (USB_DEVICE_CONFIG_REMOTE_WAKEUP > 0U))
                          | ((uint16_t)deviceHandle->remotewakeup
                             << (USB_REQUEST_STANDARD_GET_STATUS_DEVICE_REMOTE_WARKUP_SHIFT))
#endif
                    ;
                error = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceControlGetEndpointStatus:
            if (NULL != param)
            {
                usb_device_endpoint_status_struct_t *endpointStatus = (usb_device_endpoint_status_struct_t *)param;
                uint8_t ep = (endpointStatus->endpointAddress) & USB_ENDPOINT_NUMBER_MASK;
                uint8_t direction =
                    ((endpointStatus->endpointAddress) & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) >>
                    USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT;

                if (ep < USB_DEVICE_CONFIG_ENDPOINTS)
                {
                    if (0U != ep)
                    {
                        endpointStatus->endpointStatus =
                            (0U != (ehciState->registerBase->EPCR[ep - 1U] &
                                    ((0U != direction) ? USBHS_EPCR_TXS_MASK : USBHS_EPCR_RXS_MASK))) ?
                                (uint16_t)kUSB_DeviceEndpointStateStalled :
                                (uint16_t)kUSB_DeviceEndpointStateIdle;
                    }
                    else
                    {
                        endpointStatus->endpointStatus =
                            (0U != (ehciState->registerBase->EPCR0 &
                                    ((0U != direction) ? USBHS_EPCR_TXS_MASK : USBHS_EPCR_RXS_MASK))) ?
                                (uint16_t)kUSB_DeviceEndpointStateStalled :
                                (uint16_t)kUSB_DeviceEndpointStateIdle;
                    }
                    error = kStatus_USB_Success;
                }
            }
            break;
        case kUSB_DeviceControlPreSetDeviceAddress:
            if (NULL != param)
            {
                temp8 = (uint8_t *)param;
                ehciState->registerBase->DEVICEADDR =
                    ((((uint32_t)(*temp8)) << USBHS_DEVICEADDR_USBADR_SHIFT) | USBHS_DEVICEADDR_USBADRA_MASK);
                error = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceControlSetDeviceAddress:
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceControlGetSynchFrame:
            break;
#if (defined(USB_DEVICE_CONFIG_LOW_POWER_MODE) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))
#if defined(USB_DEVICE_CONFIG_REMOTE_WAKEUP) && (USB_DEVICE_CONFIG_REMOTE_WAKEUP > 0U)
        case kUSB_DeviceControlResume:
#if (defined(FSL_FEATURE_SOC_USBNC_COUNT) && (FSL_FEATURE_SOC_USBNC_COUNT > 0U))
            ehciState->registerNcBase->USB_OTGn_CTRL &= ~USBNC_USB_OTGn_CTRL_WIE_MASK;
#else
#if (defined(FSL_FEATURE_USB_ATLANTIC_EHCI_SUPPORT) && (FSL_FEATURE_USB_ATLANTIC_EHCI_SUPPORT > 0U))
#else
            ehciState->registerBase->USBGENCTRL &= ~USBHS_USBGENCTRL_WU_IE_MASK;
#endif
#endif
            ehciState->registerBase->PORTSC1 &= ~USBHS_PORTSC1_PHCD_MASK;
            ehciState->registerBase->PORTSC1 |= USBHS_PORTSC1_FPR_MASK;
            startTick = deviceHandle->hwTick;
            while ((deviceHandle->hwTick - startTick) < 10U)
            {
                __NOP();
            }
            ehciState->registerBase->PORTSC1 &= ~USBHS_PORTSC1_FPR_MASK;
            error = kStatus_USB_Success;
            break;
#if (defined(USB_DEVICE_CONFIG_LPM_L1) && (USB_DEVICE_CONFIG_LPM_L1 > 0U))
        case kUSB_DeviceControlSleepResume:
#if (defined(FSL_FEATURE_SOC_USBNC_COUNT) && (FSL_FEATURE_SOC_USBNC_COUNT > 0U))
            ehciState->registerNcBase->USB_OTGn_CTRL &= ~USBNC_USB_OTGn_CTRL_WIE_MASK;
#endif
            ehciState->registerBase->PORTSC1 &= ~USBHS_PORTSC1_PHCD_MASK;
            ehciState->registerBase->PORTSC1 |= USBHS_PORTSC1_FPR_MASK;
            error = kStatus_USB_Success;
            break;
#endif
#endif /* USB_DEVICE_CONFIG_REMOTE_WAKEUP */
        case kUSB_DeviceControlSuspend:
            ehciState->registerBase->OTGSC |= 0x007F0000U;
            OSA_ENTER_CRITICAL();
#if (defined(FSL_FEATURE_USB_ATLANTIC_EHCI_SUPPORT) && (FSL_FEATURE_USB_ATLANTIC_EHCI_SUPPORT > 0U))
            /* ehciState->registerBase->PLL_CONTROL_1 |= USBC_PLL_CONTROL_1_PLL_SUSPEND_EN(1); */
#endif
#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#if defined(FSL_FEATURE_USBHS_SUPPORT_EUSBn)
            if (0 == (int32_t)FSL_FEATURE_USBHS_SUPPORT_EUSBn(ehciState->registerBase))
#endif
            {
                ehciState->registerPhyBase->PWD = 0xFFFFFFFFU;
                /* ehciState->registerBase->OTGCTL |= ((1U<<10) | (1U<<17) | (1U<<16)); */
            }
#endif
            /* ehciState->registerPhyBase->CTRL |= ((1U << 21) | (1U << 22) | (1U << 23)); */
            ehciState->registerBase->USBSTS |= USBHS_USBSTS_SRI_MASK;
#if (defined(FSL_FEATURE_USBPHY_28FDSOI) && (FSL_FEATURE_USBPHY_28FDSOI > 0U))
#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#if defined(FSL_FEATURE_USBHS_SUPPORT_EUSBn)
            if (0 == (int32_t)FSL_FEATURE_USBHS_SUPPORT_EUSBn(ehciState->registerBase))
#endif
            {
                ehciState->registerPhyBase->USB1_VBUS_DETECT_SET |= USBPHY_USB1_VBUS_DETECT_VBUSVALID_TO_SESSVALID_MASK;
            }
#endif
#endif
            ehciState->registerBase->PORTSC1 |= USBHS_PORTSC1_PHCD_MASK;
#if (defined(FSL_FEATURE_SOC_USBNC_COUNT) && (FSL_FEATURE_SOC_USBNC_COUNT > 0U))
#if (defined(USBPHY_CTRL_ENVBUSCHG_WKUP_MASK))
#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#if defined(FSL_FEATURE_USBHS_SUPPORT_EUSBn)
            if (0 == (int32_t)FSL_FEATURE_USBHS_SUPPORT_EUSBn(ehciState->registerBase))
#endif
            {
                ehciState->registerPhyBase->CTRL |= USBPHY_CTRL_ENVBUSCHG_WKUP_MASK | USBPHY_CTRL_ENIDCHG_WKUP_MASK |
                                                    USBPHY_CTRL_ENDPDMCHG_WKUP_MASK |
                                                    USBPHY_CTRL_ENIRQRESUMEDETECT_MASK;
            }
#endif
#endif
#if defined(FSL_FEATURE_USBHS_SUPPORT_EUSBn)
            ehciState->registerNcBase->USB_OTGn_CTRL |=
                USBNC_USB_OTGn_CTRL_WKUP_VBUS_EN_MASK | USBNC_USB_OTGn_CTRL_WKUP_DPDM_EN_MASK;
#else
            ehciState->registerNcBase->USB_OTGn_CTRL |= USBNC_USB_OTGn_CTRL_WKUP_ID_EN_MASK |
                                                        USBNC_USB_OTGn_CTRL_WKUP_VBUS_EN_MASK |
                                                        USBNC_USB_OTGn_CTRL_WKUP_DPDM_EN_MASK;
#endif
            ehciState->registerNcBase->USB_OTGn_CTRL |= USBNC_USB_OTGn_CTRL_WIE_MASK;
#else
#if (defined(FSL_FEATURE_USB_ATLANTIC_EHCI_SUPPORT) && (FSL_FEATURE_USB_ATLANTIC_EHCI_SUPPORT > 0U))
            PMU->WAKEUP_PM2_MASK1 |= PMU_WAKEUP_PM2_MASK1_USB(1);
#else
            ehciState->registerBase->USBGENCTRL = USBHS_USBGENCTRL_WU_IE_MASK;
#endif
#endif
#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#if defined(FSL_FEATURE_USBHS_SUPPORT_EUSBn)
            if (0 == (int32_t)FSL_FEATURE_USBHS_SUPPORT_EUSBn(ehciState->registerBase))
#endif
            {
                ehciState->registerPhyBase->CTRL |= USBPHY_CTRL_CLKGATE_MASK;
            }
#endif
            ehciState->isSuspending = 1U;
            OSA_EXIT_CRITICAL();
            error = kStatus_USB_Success;
            break;

#if (defined(USB_DEVICE_CONFIG_LPM_L1) && (USB_DEVICE_CONFIG_LPM_L1 > 0U))
        case kUSB_DeviceControlSleep:
            ehciState->registerBase->OTGSC |= 0x007F0000U;
            OSA_ENTER_CRITICAL();
#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#if defined(FSL_FEATURE_USBHS_SUPPORT_EUSBn)
            if (0 == (int32_t)FSL_FEATURE_USBHS_SUPPORT_EUSBn(ehciState->registerBase))
#endif
            {
                ehciState->registerPhyBase->PWD = 0xFFFFFFFFU;
                /* ehciState->registerBase->OTGCTL |= ((1U<<10) | (1U<<17) | (1U<<16)); */
            }
#endif
            ehciState->registerBase->USBSTS |= USBHS_USBSTS_SRI_MASK;
#if (defined(FSL_FEATURE_USBPHY_28FDSOI) && (FSL_FEATURE_USBPHY_28FDSOI > 0U))
#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#if defined(FSL_FEATURE_USBHS_SUPPORT_EUSBn)
            if (0 == (int32_t)FSL_FEATURE_USBHS_SUPPORT_EUSBn(ehciState->registerBase))
#endif
            {
                ehciState->registerPhyBase->USB1_VBUS_DETECT_SET |= USBPHY_USB1_VBUS_DETECT_VBUSVALID_TO_SESSVALID_MASK;
            }
#endif
#endif
            /* ehciState->registerBase->PORTSC1 |= USBHS_PORTSC1_PHCD_MASK;*/
#if (defined(FSL_FEATURE_SOC_USBNC_COUNT) && (FSL_FEATURE_SOC_USBNC_COUNT > 0U))
#if (defined(USBPHY_CTRL_ENVBUSCHG_WKUP_MASK))
#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#if defined(FSL_FEATURE_USBHS_SUPPORT_EUSBn)
            if (0 == (int32_t)FSL_FEATURE_USBHS_SUPPORT_EUSBn(ehciState->registerBase))
#endif
            {
                ehciState->registerPhyBase->CTRL |= USBPHY_CTRL_ENVBUSCHG_WKUP_MASK | USBPHY_CTRL_ENIDCHG_WKUP_MASK |
                                                    USBPHY_CTRL_ENDPDMCHG_WKUP_MASK |
                                                    USBPHY_CTRL_ENIRQRESUMEDETECT_MASK;
            }
#endif
#endif
#if defined(FSL_FEATURE_USBHS_SUPPORT_EUSBn)
            ehciState->registerNcBase->USB_OTGn_CTRL |=
                USBNC_USB_OTGn_CTRL_WKUP_VBUS_EN_MASK | USBNC_USB_OTGn_CTRL_WKUP_DPDM_EN_MASK;
#else
            ehciState->registerNcBase->USB_OTGn_CTRL |= USBNC_USB_OTGn_CTRL_WKUP_ID_EN_MASK |
                                                        USBNC_USB_OTGn_CTRL_WKUP_VBUS_EN_MASK |
                                                        USBNC_USB_OTGn_CTRL_WKUP_DPDM_EN_MASK;
#endif
            ehciState->registerNcBase->USB_OTGn_CTRL |= USBNC_USB_OTGn_CTRL_WIE_MASK;
#else
#if (defined(FSL_FEATURE_USB_ATLANTIC_EHCI_SUPPORT) && (FSL_FEATURE_USB_ATLANTIC_EHCI_SUPPORT > 0U))
            PMU->WAKEUP_PM2_MASK1 |= PMU_WAKEUP_PM2_MASK1_USB(1);
#else
            ehciState->registerBase->USBGENCTRL = USBHS_USBGENCTRL_WU_IE_MASK;
#endif
#endif
#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#if defined(FSL_FEATURE_USBHS_SUPPORT_EUSBn)
            if (0 == (int32_t)FSL_FEATURE_USBHS_SUPPORT_EUSBn(ehciState->registerBase))
#endif
            {
                ehciState->registerPhyBase->CTRL |= USBPHY_CTRL_CLKGATE_MASK;
            }
#endif
            /* TODO: wait 3us delay */
            ehciState->isSuspending = 1U;
            OSA_EXIT_CRITICAL();
            error = kStatus_USB_Success;
            break;
#endif

#endif /* USB_DEVICE_CONFIG_LOW_POWER_MODE */
        case kUSB_DeviceControlSetDefaultStatus:
            for (uint8_t count = 0U; count < USB_DEVICE_CONFIG_ENDPOINTS; count++)
            {
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
                if ((kStatus_USB_Success != USB_DeviceEhciEndpointDeinit(ehciState, (count | (USB_IN << 0x07U)))) ||
                    (kStatus_USB_Success != USB_DeviceEhciEndpointDeinit(ehciState, (count | (USB_OUT << 0x07U)))))
                {
                    return kStatus_USB_Error;
                }
#else
                (void)USB_DeviceEhciEndpointDeinit(ehciState, (count | (USB_IN << 0x07U)));
                (void)USB_DeviceEhciEndpointDeinit(ehciState, (count | (USB_OUT << 0x07U)));
#endif
            }
            USB_DeviceEhciSetDefaultState(ehciState);
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceControlGetSpeed:
            if (NULL != param)
            {
                temp8  = (uint8_t *)param;
                *temp8 = ehciState->speed;
                error  = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceControlGetOtgStatus:
            break;
        case kUSB_DeviceControlSetOtgStatus:
            break;
#if (defined(USB_DEVICE_CONFIG_USB20_TEST_MODE) && (USB_DEVICE_CONFIG_USB20_TEST_MODE > 0U))
        case kUSB_DeviceControlSetTestMode:
            if (param)
            {
                temp8 = (uint8_t *)param;
                ehciState->registerBase->PORTSC1 |= ((uint32_t)(*temp8) << 16U);
                error = kStatus_USB_Success;
            }
            break;
#endif
#if (defined(USB_DEVICE_CONFIG_CHARGER_DETECT) && (USB_DEVICE_CONFIG_CHARGER_DETECT > 0U))

        case kUSB_DeviceControlUpdateHwTick:
            /*udpate 1ms time tick*/
#if (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U))
#ifndef USBHSDCD_IRQS
            USB_HSDcdIsrFunction(ehciState->dcdHandle);
#endif
#elif (defined(FSL_FEATURE_SOC_USB_ANALOG_COUNT) && (FSL_FEATURE_SOC_USB_ANALOG_COUNT > 0U))
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
            if (kStatus_USB_Success != USB_PHYDCD_TimerIsrFunction(ehciState->dcdHandle))
            {
                return kStatus_USB_Error;
            }
#else
            (void)USB_PHYDCD_TimerIsrFunction(ehciState->dcdHandle);
#endif
#endif

            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceControlDcdEnable:

#if (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U))
            if (kStatus_hsdcd_Success == USB_HSDCD_Control(ehciState->dcdHandle, kUSB_DeviceHSDcdEnable, NULL))
            {
                error = kStatus_USB_Success;
            }
#elif (defined(FSL_FEATURE_SOC_USB_ANALOG_COUNT) && (FSL_FEATURE_SOC_USB_ANALOG_COUNT > 0U))
            if (kStatus_phydcd_Success == USB_PHYDCD_Control(ehciState->dcdHandle, kUSB_DevicePHYDcdEnable, NULL))
            {
                error = kStatus_USB_Success;
            }
#endif

            break;
        case kUSB_DeviceControlDcdDisable:

#if (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U))
            if (kStatus_hsdcd_Success == USB_HSDCD_Control(ehciState->dcdHandle, kUSB_DeviceHSDcdDisable, NULL))
            {
                error = kStatus_USB_Success;
            }
#elif (defined(FSL_FEATURE_SOC_USB_ANALOG_COUNT) && (FSL_FEATURE_SOC_USB_ANALOG_COUNT > 0U))
            if (kStatus_phydcd_Success == USB_PHYDCD_Control(ehciState->dcdHandle, kUSB_DevicePHYDcdDisable, NULL))
            {
                error = kStatus_USB_Success;
            }
#endif

            break;
#endif
#if defined(USB_DEVICE_CONFIG_GET_SOF_COUNT) && (USB_DEVICE_CONFIG_GET_SOF_COUNT > 0U)
        case kUSB_DeviceControlGetCurrentFrameCount:
            if (NULL != param)
            {
                temp32 = (uint32_t *)param;
                if (USB_SPEED_HIGH == ehciState->speed)
                {
                    *temp32 = ehciState->registerBase->FRINDEX & (USB_DEVICE_MAX_FRAME_COUNT);
                }
                else /* if not high speed, change to use frame count */
                {
                    *temp32 = (ehciState->registerBase->FRINDEX & (USB_DEVICE_MAX_FRAME_COUNT)) / 8U;
                }
                error = kStatus_USB_Success;
            }
            break;
#endif
        default:
            /*no action*/
            break;
    }

    return error;
}

/*!
 * @brief Handle the EHCI device interrupt.
 *
 * The function is used to handle the EHCI device interrupt.
 *
 * @param deviceHandle    The device handle got from USB_DeviceInit.
 *
 */
void USB_DeviceEhciIsrFunction(void *deviceHandle)
{
    usb_device_struct_t *handle = (usb_device_struct_t *)deviceHandle;
    usb_device_ehci_state_struct_t *ehciState;
    uint32_t status;

    if (NULL == deviceHandle)
    {
        return;
    }

    ehciState = (usb_device_ehci_state_struct_t *)(handle->controllerHandle);

#if ((defined(USB_DEVICE_CONFIG_LOW_POWER_MODE)) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))

#if (defined(FSL_FEATURE_SOC_USBNC_COUNT) && (FSL_FEATURE_SOC_USBNC_COUNT > 0U))

    if (0U != (ehciState->registerNcBase->USB_OTGn_CTRL & USBNC_USB_OTGn_CTRL_WIE_MASK))
    {
        if (0U != (ehciState->registerNcBase->USB_OTGn_CTRL & USBNC_USB_OTGn_CTRL_WIR_MASK))
        {
            ehciState->registerBase->PORTSC1 &= ~USBHS_PORTSC1_PHCD_MASK;
            ehciState->registerNcBase->USB_OTGn_CTRL &= ~USBNC_USB_OTGn_CTRL_WIE_MASK;
        }
    }
    else
    {
    }

#else
#if (defined(FSL_FEATURE_USB_ATLANTIC_EHCI_SUPPORT) && (FSL_FEATURE_USB_ATLANTIC_EHCI_SUPPORT > 0U))
#else
    if (0U != (ehciState->registerBase->USBGENCTRL & USBHS_USBGENCTRL_WU_IE_MASK))
    {
        if (0U != (ehciState->registerBase->USBGENCTRL & (1UL << 8)))
        {
            ehciState->registerBase->USBGENCTRL &= ~(1UL << 8);
            ehciState->registerBase->USBGENCTRL |= USBHS_USBGENCTRL_WU_INT_CLR_MASK;
            ehciState->registerBase->PORTSC1 &= ~USBHS_PORTSC1_PHCD_MASK;
            ehciState->registerBase->USBGENCTRL &= ~USBHS_USBGENCTRL_WU_IE_MASK;
        }
    }
    else
    {
    }
#endif
#endif

#endif

#if defined(USB_DEVICE_CONFIG_DETACH_ENABLE) && (USB_DEVICE_CONFIG_DETACH_ENABLE > 0U)
    if ((ehciState->registerBase->OTGSC & USBHS_OTGSC_BSVIS_MASK) != 0U)
    {
        usb_device_callback_message_struct_t message;

        ehciState->registerBase->OTGSC |= USBHS_OTGSC_BSVIS_MASK;

        message.buffer  = (uint8_t *)NULL;
        message.length  = 0U;
        message.isSetup = 0U;
        if (0U != (ehciState->registerBase->OTGSC & USBHS_OTGSC_BSV_MASK))
        {
            /* Device is connected to a host. */
            message.code = (uint8_t)kUSB_DeviceNotifyAttach;
            (void)USB_DeviceEhciNotification(ehciState, &message);

#if (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U)) && \
    (defined(USB_DEVICE_CONFIG_CHARGER_DETECT) && (USB_DEVICE_CONFIG_CHARGER_DETECT > 0U))
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
            if (kStatus_hsdcd_Success != USB_HSDCD_Control(ehciState->dcdHandle, kUSB_DeviceHSDcdRun, NULL))
            {
#if (defined(DEVICE_ECHO) && (DEVICE_ECHO > 0U))
                usb_echo("hsdcd run error\n");
#endif
            }
#else
            (void)USB_HSDCD_Control(ehciState->dcdHandle, kUSB_DeviceHSDcdRun, NULL);
#endif
#elif (defined(USB_DEVICE_CONFIG_CHARGER_DETECT) && (USB_DEVICE_CONFIG_CHARGER_DETECT > 0U)) && \
    (defined(FSL_FEATURE_SOC_USB_ANALOG_COUNT) && (FSL_FEATURE_SOC_USB_ANALOG_COUNT > 0U))
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
            if (kStatus_phydcd_Error != USB_PHYDCD_Control(ehciState->dcdHandle, kUSB_DevicePHYDcdRun, NULL))
            {
#if (defined(DEVICE_ECHO) && (DEVICE_ECHO > 0U))
                usb_echo("phydcd run error\n");
#endif
            }
#else
            (void)USB_PHYDCD_Control(ehciState->dcdHandle, kUSB_DevicePHYDcdRun, NULL);
#endif
#endif
        }
        else
        {
            /* Device is disconnected from a host. */
            message.code = (uint8_t)kUSB_DeviceNotifyDetach;
            (void)USB_DeviceEhciNotification(ehciState, &message);
        }
    }
#endif /* USB_DEVICE_CONFIG_DETACH_ENABLE */

    status = ehciState->registerBase->USBSTS;
    status &= ehciState->registerBase->USBINTR;

    ehciState->registerBase->USBSTS = status;

#if defined(USB_DEVICE_CONFIG_ERROR_HANDLING) && (USB_DEVICE_CONFIG_ERROR_HANDLING > 0U)
    if (0U != (status & USBHS_USBSTS_UEI_MASK))
    {
        /* Error interrupt */
        USB_DeviceEhciInterruptError(ehciState);
    }
#endif /* USB_DEVICE_CONFIG_ERROR_HANDLING */

    if (0U != (status & USBHS_USBSTS_URI_MASK))
    {
        /* Reset interrupt */
        USB_DeviceEhciInterruptReset(ehciState);
    }

    if (0U != (status & USBHS_USBSTS_UI_MASK))
    {
        /* Token done interrupt */
        USB_DeviceEhciInterruptTokenDone(ehciState);
    }

    if (0U != (status & USBHS_USBSTS_PCI_MASK))
    {
        /* Port status change interrupt */
        USB_DeviceEhciInterruptPortChange(ehciState);
    }
#if (defined(USB_DEVICE_CONFIG_LOW_POWER_MODE) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))
    if (0U != (status & USBHS_USBSTS_SLI_MASK))
    {
        /* Suspend interrupt */
        USB_DeviceEhciInterruptSuspend(ehciState);
    }
#if (defined(USB_DEVICE_CONFIG_LPM_L1) && (USB_DEVICE_CONFIG_LPM_L1 > 0U))
    if (0U != (status & USB_USBSTS_LPM_L1_ENTRYI_MASK))
    {
        USB_DeviceEhciInterruptLPMSleep(ehciState);
    }
#endif
#endif /* USB_DEVICE_CONFIG_LOW_POWER_MODE */

#if (defined(USB_DEVICE_CONFIG_SOF_NOTIFICATIONS) && (USB_DEVICE_CONFIG_SOF_NOTIFICATIONS > 0U))
    if (0U != (status & USBHS_USBSTS_SRI_MASK))
    {
        /* SOF interrupt */
        USB_DeviceEhciInterruptSOF(ehciState);
    }
#endif
}

#endif /* USB_DEVICE_CONFIG_EHCI */
