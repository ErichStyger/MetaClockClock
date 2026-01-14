/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_HOST_CDC_ECM_H__
#define __USB_HOST_CDC_ECM_H__

/*!
 * @addtogroup usb_host_cdc_ecm_drv
 * @{
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief Class-Specific Codes */
#define USB_HOST_CDC_ECM_CLASS_CODE    (0x02U)
#define USB_HOST_CDC_ECM_SUBCLASS_CODE (0x06U)
#define USB_HOST_CDC_ECM_PROTOCOL_CODE (0x00U)

/*! @brief Class-Specific Request Codes for Ethernet subclass */
#define CDC_ECM_SET_ETHERNET_MULTICAST_FILTER                (0x40U)
#define CDC_ECM_SET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER (0x41U)
#define CDC_ECM_GET_ETHERNET_POWER_MANAGEMENT_PATTERN_FILTER (0x42U)
#define CDC_ECM_SET_ETHERNET_PACKET_FILTER                   (0x43U)
#define CDC_ECM_GET_ETHERNET_STATISTIC                       (0x44U)

/*! @brief Ethernet Packet Filter Bitmap */
#define CDC_ECM_ETH_PACKET_FILTER_PROMISCUOUS   (0x01U)
#define CDC_ECM_ETH_PACKET_FILTER_ALL_MULTICAST (0x02U)
#define CDC_ECM_ETH_PACKET_FILTER_DIRECTED      (0x04U)
#define CDC_ECM_ETH_PACKET_FILTER_BROADCAST     (0x08U)
#define CDC_ECM_ETH_PACKET_FILTER_MULTICAST     (0x10U)

/*! @brief Ethernet Statistics Capabilities */
typedef enum _usb_host_cdc_ecm_eth_stats_cap_enum
{
    XMIT_OK = 0,           /*!< Frames transmitted without errors */
    RVC_OK,                /*!< Frames received without errors */
    XMIT_ERROR,            /*!< Frames not transmitted, or transmitted with errors */
    RCV_ERROR,             /*!< Frames received with errors that are not delivered to the USB host. */
    RCV_NO_BUFFER,         /*!< Frame missed, no buffers */
    DIRECTED_BYTES_XMIT,   /*!< Directed bytes transmitted without errors */
    DIRECTED_FRAMES_XMIT,  /*!< Directed frames transmitted without errors */
    MULTICAST_BYTES_XMIT,  /*!< Multicast bytes transmitted without errors */
    MULTICAST_FRAMES_XMIT, /*!< Multicast frames transmitted without errors */
    BROADCAST_BYTES_XMIT,  /*!< Broadcast bytes transmitted without errors */
    BROADCAST_FRAMES_XMIT, /*!< Broadcast frames transmitted without errors */
    DIRECTED_BYTES_RCV,    /*!< Directed bytes received without errors */
    DIRECTED_FRAMES_RCV,   /*!< Directed frames received without errors */
    MULTICAST_BYTES_RCV,   /*!< Multicast bytes received without errors */
    MULTICAST_FRAMES_RCV,  /*!< Multicast frames received without errors */
    BROADCAST_BYTES_RCV,   /*!< Broadcast bytes received without errors */
    BROADCAST_FRAMES_RCV,  /*!< Broadcast frames received without errors */
    RCV_CRC_ERROR, /*!< Frames received with circular redundancy check (CRC) or frame check sequence (FCS) error */
    TRANSMIT_QUEUE_LENGTH,  /*!< Length of transmit queue */
    RCV_ERROR_ALIGNMENT,    /*!< Frames received with alignment error */
    XMIT_ONE_COLLISION,     /*!< Frames transmitted with one collision */
    XMIT_MORE_COLLISIONS,   /*!< Frames transmitted with more than one collision */
    XMIT_DEFERRED,          /*!< Frames transmitted after deferral */
    XMIT_MAX_COLLISIONS,    /*!< Frames not transmitted due to collisions */
    RCV_OVERRUN,            /*!< Frames not received due to overrun */
    XMIT_UNDERRUN,          /*!< Frames not transmitted due to underrun */
    XMIT_HEARTBEAT_FAILURE, /*!< Frames transmitted with heartbeat failure */
    XMIT_TIMES_CRS_LOST,    /*!< Times carrier sense signal lost during transmission */
    XMIT_LATE_COLLISIONS,   /*!< Late collisions detected */
    ETH_STATS_CAP_COUNT     /*!< The count of Ethernet Statistics Capabilities */
} USB_HostCdcEcmEthStatsCap_t;

/*! @brief Ethernet Statistics Capabilities Bitmap */
#define CDC_ECM_ETH_STATS_BITMAP(type) ((uint32_t)(1 << type))

/*! @brief Ethernet Statistics Feature Selector Codes */
#define CDC_ECM_ETH_STATS_FEATURE_SEL_CODE(type) ((uint8_t)(type + 1))

/*! @brief Default maximum Ethernet frame size */
#define CDC_ECM_FRAME_MAX_FRAMELEN (1514U)

/*! @brief Power Management Pattern Filter Structure */
typedef struct
{
    uint16_t MaskSize;
    uint8_t *Mask;
    uint8_t *Pattern;
} USB_HostCdcEcmPowerManagementPatternFilter_t;

/*! @brief Parameters of Request SetEthernetMulticastFilters */
typedef struct
{
    uint16_t filterNumSum;
    uint8_t (*multicastAddressList)[6];
} USB_HostCdcEcmSetEthernetMulticastFiltersParam_t;

/*! @brief Parameters of Request SetEthernetPowerManagementPatternFilter */
typedef struct
{
    uint16_t filternum;
    USB_HostCdcEcmPowerManagementPatternFilter_t *filterData;
} USB_HostCdcEcmSetEthernetPowerManagementPatternFilterParam_t;

/*! @brief Parameters of Request GetEthernetPowerManagementPatternFilter */
typedef struct
{
    uint16_t filternum;
    uint16_t patternActive;
} USB_HostCdcEcmGetEthernetPowerManagementPatternFilterParam_t;

/*! @brief Parameters of Request SetEthernetPacketFilter */
typedef uint16_t USB_HostCdcEcmSetEthernetPacketFilterParam_t;

/*! @brief Parameters of Request GetEthernetStatistic */
typedef struct
{
    uint16_t featureSelector;
    uint32_t statistics;
} USB_HostCdcEcmGetEthernetStatisticParam_t;

/*! @brief Parameters of CDC-ECM Class-Specific Request */
typedef union
{
    USB_HostCdcEcmSetEthernetMulticastFiltersParam_t SetEthernetMulticastFilters;
    USB_HostCdcEcmSetEthernetPowerManagementPatternFilterParam_t SetEthernetPowerManagementPatternFilter;
    USB_HostCdcEcmGetEthernetPowerManagementPatternFilterParam_t GetEthernetPowerManagementPatternFilter;
    USB_HostCdcEcmSetEthernetPacketFilterParam_t SetEthernetPacketFilter;
    USB_HostCdcEcmGetEthernetStatisticParam_t GetEthernetStatistic;
} USB_HostCdcEcmRequestParam_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @name USB CDC-ECM host class driver
 * @{
 */

/*!
 * @brief Set the Ethernet device multicast filters as specified in the sequential list of 48 bit Ethernet multicast
 * addresses.
 * @param classHandle The class handle.
 * @param filterNumSum The sum of filters to be set.
 * @param filterData The data of filters.
 * @param callbackFn This callback is called after this function completes.
 * @param callbackParam The parameter in the callback function.
 * @return An error code or kStatus_USB_Success.
 */
usb_status_t USB_HostCdcEcmSetEthernetMulticastFilters(usb_host_class_handle classHandle,
                                                       uint16_t filterNumSum,
                                                       uint8_t (*filterData)[6],
                                                       transfer_callback_t callbackFn,
                                                       void *callbackParam);

/*!
 * @brief Set up the specified Ethernet power management pattern filter as described in the data structure.
 * @param classHandle The class handle.
 * @param filterNum The number of the specific pattern filter to be set.
 * @param buffer The data of pattern filters.
 * @param bufferLength The length of pattern filters.
 * @param callbackFn This callback is called after this function completes.
 * @param callbackParam The parameter in the callback function.
 * @return An error code or kStatus_USB_Success.
 */
usb_status_t USB_HostCdcEcmSetEthernetPowerManagementPatternFilter(usb_host_class_handle classHandle,
                                                                   uint16_t filterNum,
                                                                   uint8_t *buffer,
                                                                   uint16_t bufferLength,
                                                                   transfer_callback_t callbackFn,
                                                                   void *callbackParam);

/*!
 * @brief Retrieves the status of the specified Ethernet power management pattern filter from the device.
 * @param classHandle The class handle.
 * @param filterNum The number of the specific pattern filter to be set.
 * @param buffer The pattern filter active.
 * @param callbackFn This callback is called after this function completes.
 * @param callbackParam The parameter in the callback function.
 * @returns An error code or kStatus_USB_Success.
 */
usb_status_t USB_HostCdcEcmGetEthernetPowerManagementPatternFilter(usb_host_class_handle classHandle,
                                                                   uint16_t filterNum,
                                                                   uint8_t (*buffer)[2],
                                                                   transfer_callback_t callbackFn,
                                                                   void *callbackParam);

/*!
 * @brief Configure device Ethernet packet filter settings.
 * @param classHandle The class handle.
 * @param packetFilterBitmap The config of the packet filter to be set.
 * @param callbackFn This callback is called after this function completes.
 * @param callbackParam The parameter in the callback function.
 * @returns An error code or kStatus_USB_Success.
 */
usb_status_t USB_HostCdcEcmSetEthernetPacketFilter(usb_host_class_handle classHandle,
                                                   uint16_t packetFilterBitmap,
                                                   transfer_callback_t callbackFn,
                                                   void *callbackParam);

/*!
 * @brief Retrieve a statistic based on the feature selector.
 * @param classHandle The class handle.
 * @param featureSelector The ethernet statistics feature selector to be set.
 * @param buffer The receiving address for Ethernet statistics.
 * @param callbackFn This callback is called after this function completes.
 * @param callbackParam The parameter in the callback function.
 * @returns An error code or kStatus_USB_Success.
 */
usb_status_t USB_HostCdcEcmGetEthernetStatistic(usb_host_class_handle classHandle,
                                                uint16_t featureSelector,
                                                uint8_t (*buffer)[4],
                                                transfer_callback_t callbackFn,
                                                void *callbackParam);

/*!
 * @brief Receive data.
 * @param classHandle The class handle.
 * @param buffer Received data.
 * @param bufferLength The length of data.
 * @param callbackFn This callback is called after this function completes.
 * @param callbackParam The parameter in the callback function.
 * @returns An error code or kStatus_USB_Success.
 */
usb_status_t USB_HostCdcEcmDataRecv(usb_host_class_handle classHandle,
                                    uint8_t *buffer,
                                    uint32_t bufferLength,
                                    transfer_callback_t callbackFn,
                                    void *callbackParam);

/*!
 * @brief Send data.
 * @param classHandle The class handle.
 * @param buffer Data to be sent.
 * @param bufferLength The length of data.
 * @param maxSegmentSize The max segment size of the data to be sent.
 * @param callbackFn This callback is called after this function completes.
 * @param callbackParam The parameter in the callback function.
 * @returns An error code or kStatus_USB_Success.
 */
usb_status_t USB_HostCdcEcmDataSend(usb_host_class_handle classHandle,
                                    uint8_t *buffer,
                                    uint32_t bufferLength,
                                    uint16_t maxSegmentSize,
                                    transfer_callback_t callbackFn,
                                    void *callbackParam);

/*!
 * @brief Get CDC-ECM MAC string descriptor.
 * @param classHandle The class handle.
 * @param index The index of string descriptor where MAC is in it.
 * @param langID Language ID of string descriptor.
 * @param buffer The string to be received.
 * @param bufferLength The length of string.
 * @param callbackFn This callback is called after this function completes.
 * @param callbackParam The parameter in the callback function.
 * @returns An error code or kStatus_USB_Success.
 */
usb_status_t USB_HostCdcEcmGetMacStringDescriptor(usb_host_class_handle classHandle,
                                                  uint16_t index,
                                                  uint16_t langID,
                                                  uint8_t *buffer,
                                                  uint16_t bufferLength,
                                                  transfer_callback_t callbackFn,
                                                  void *callbackParam);

/*!
 * @brief Encapsulating a Power Management Pattern Filter.
 * @param param Information of Power Management Pattern Filter.
 * @param dataBuf The buffer to store Encapsulated Power Management Pattern Filter.
 * @returns The length of Power Management Pattern Filter.
 */
uint32_t USB_HostCdcEcmPowerManagementPatternFilterData(USB_HostCdcEcmPowerManagementPatternFilter_t *param,
                                                        uint8_t *dataBuf);

/*! @} */
/*! @} */
#endif
