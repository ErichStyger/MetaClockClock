/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PD_PHY_H__
#define __PD_PHY_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @addtogroup usb_pd_phy_drv
 * @{
 */

/*! @brief In the return value of PHY control (PD_PHY_GET_VBUS_POWER_STATE), this bit set means vbus is vsafe5v. */
#define PD_VBUS_POWER_STATE_VSAFE5V_MASK (0x01u)
/*! @brief In the return value of PHY control (PD_PHY_GET_VBUS_POWER_STATE), this bit set means vbus is vsafe0v. */
#define PD_VBUS_POWER_STATE_VSAFE0V_MASK (0x02u)
/*! @brief In the return value of PHY control (PD_PHY_GET_VBUS_POWER_STATE), this bit set means vbus exist. */
#define PD_VBUS_POWER_STATE_VBUS_MASK (0x04u)
/*! @brief In the return value of PHY control (PD_PHY_GET_VBUS_POWER_STATE), this bit set means phy vsys exist. */
#define PD_VBUS_POWER_STATE_VSYS_MASK (0x08u)
/*! @brief In the return value of PHY control (PD_PHY_GET_VBUS_POWER_STATE), this bit set means vbus is above
 * vsinkDisconnect. */
#define PD_VBUS_POWER_STATE_SINK_DISCONNECT (0x10u)

/*! @brief The CC type */
typedef enum _pd_cc_type
{
    /*! Disable CC communication function */
    kPD_CCInvalid,
    /*! Enable CC1 communication function */
    kPD_CC1,
    /*! Enable CC2 communication function */
    kPD_CC2,
} pd_cc_type_t;

/*! @brief The CC state value */
typedef enum _pd_phy_cc_state
{
    /*! as source, detect no Rd or Ra */
    kCCState_SrcOpen,
    /*! as source, detect partner's Rd */
    kCCState_SrcRd,
    /*! as source, detect partner's Ra */
    kCCState_SrcRa,
    /*! as sink, detect no Rp */
    kCCState_SnkOpen,
    /*! as sink, detect partner's Rp, PHY driver please use kCCState_SnkRpDefault,kCCState_SnkRp15,kCCState_SnkRp3 */
    kCCState_SnkRp,
    /*! as sink, detect partner's Rp (default current) */
    kCCState_SnkRpDefault,
    /*! as sink, detect partner's Rp (1.5A current) */
    kCCState_SnkRp1_5,
    /*! as sink, detect partner's Rp (3.0A current) */
    kCCState_SnkRp3_0,
    /*! looking for connection (not connected) or state is unknown */
    kCCState_Unknown,
    /*!
     * CC state is not stable, CC state cannot stay as this value, need return stable state a little time later.
     * PD stack cannot change state machine relying on this state , need wait the stable state.
     */
    kCCState_Unstable,
} pd_phy_cc_state_t;

/*! @brief PD PHY interface table's control function implement these control codes */
typedef enum _pd_phy_control
{
    /*! when PHY state change, PHY driver will call PD_Notify(PD_PHY_EVENT_STATE_CHANGE),
     * PD stack will call this control code to update PHY state.
     * This method can keep all PHY operations are in the same task and there are no nested PHY operations.
     * So the PHY operations are steady.
     */
    PD_PHY_UPDATE_STATE,
    /*! Enable or Disable alert pin interrupt function */
    PD_PHY_CONTROL_ALERT_INTERRUPT,
    /*! control PHY enter low power */
    PD_PHY_ENTER_LOW_POWER_STATE,
    /*! control PHY exit low power */
    PD_PHY_EXIT_LOW_POWER_STATE,
    /*! Get PHY vendor info, the return value is #pd_phy_vendor_info_t */
    PD_PHY_GET_PHY_VENDOR_INFO,
    /*! control the PHY's pins output 1 or 0. these pins are used to control board's vbus power. */
    PD_PHY_CONTROL_POWER_PIN,
    /*! (vbus control) enable/disable vbus detect function */
    PD_PHY_CONTROL_VBUS_DETECT,
    /*! (vbus control) Tell PHY driver vbus is changing or stable. (for example: pr swap) */
    PD_PHY_SET_VBUS_TRANSFORM_STATE,
    /*! (vbus control) discharge VBus */
    PD_PHY_DISCHARGE_VBUS,
    /*! (vbus control) control auto discharge */
    PD_PHY_CONTROL_VBUS_AUTO_DISCHARGE,
    /*! (vbus control) control voltage alarm */
    PD_PHY_CONTROL_VBUS_ALARM,
    /*! (vbus control) set Type-C VBus current ilim */
    PD_PHY_SET_TYPEC_VBUS_ILIM,
    /*! (vbus info) get vbus state, bit0 - vsafe5v, bit1 - vsafe0v, bit2 - vbus, bit3 - vsys
        the parameter: when the bit is set, it means the vbus state is requested.
        For example: bit0 and bit2 is set, it means vsafe5v and vbus states is requested,
                     and don't care the other vbus states.
     */
    PD_PHY_GET_VBUS_POWER_STATE,
    /*! (vbus info) get the vbus supply state in fast role swap */
    PD_PHY_FR_SWAP_CHECK_VBUS_APPLIED,
    /*! (vbus info) inform the vbus voltage, high 16 bits - high voltage, low 16 bits - low voltage. */
    PD_PHY_INFORM_VBUS_VOLTAGE_RANGE,
    /*! (vconn control) enable/disable vconn power */
    PD_PHY_CONTROL_VCONN,
    /*! (vconn control) discharge vconn */
    PD_PHY_DISCHARGE_VCONN,
    /*! (CC line control) configure PHY to detect attach, currently state is detached. */
    PD_PHY_CONFIG_ATTACH_DETECTION,
    /*! (CC line control) configure PHY to detect detach, currently state is attached. */
    PD_PHY_CONFIG_DETACH_DETECTION,
    /*! (CC line control) reset the PHY connect/disconnect detection function */
    PD_PHY_RESET_CONNECT_DETECTION,
    /*! (CC line control) signal fast role swap */
    PD_PHY_SIGNAL_FR_SWAP,
    /*! (CC line control) enable/disable fast role swap function (detect the fast role swap signal) */
    PD_PHY_CONTROL_FR_SWAP,
    /*! (CC line control) Set the Rp value to relect the typec current level  */
    PD_PHY_SRC_SET_TYPEC_CURRENT_CAP,
    /*! (CC line info) get typec current level */
    PD_PHY_GET_TYPEC_CURRENT_CAP,
    /*! (CC line info) get CC line state, used for connect/disconnect judgement */
    PD_PHY_GET_CC_LINE_STATE,
    /*! (CC line info) get connection state, #pd_phy_look4connect_state_t */
    PD_PHY_GET_LOOK4_CONNECTION_STATE,
    /*! (message control) set the PD communication CC */
    PD_PHY_CONNECT_SET_CC_ORIENTATION,
    /*! (message control) reset the message function. */
    PD_PHY_RESET_MSG_FUNCTION,
    /*! (message control, not supported yet) disable message RX function */
    PD_PHY_DISABLE_MSG_RX,
    /*! (message control, not supported yet) disable message TX function */
    PD_PHY_DISABLE_MSG_TX,
    /*! (message control, not supported yet) cancel the sending message */
    PD_PHY_CANCEL_MSG_TX,
    /*! (message control, not supported yet) cancel the receiving message */
    PD_PHY_CANCEL_MSG_RX,
    /*! (message control) send hard reset */
    PD_PHY_SEND_HARD_RESET,
    /*! (message control) send cable reset */
    PD_PHY_SEND_CABLE_RESET,
    /*! (message control) set message header info, goodCRC use this info. the parameter is #pd_phy_msg_header_info_t */
    PD_PHY_SET_MSG_HEADER_INFO,
    /*! not supported yet */
    PD_PHY_RESET_BIST,
    /*! Enter BIST Mode */
    PD_PHY_ENTER_BIST,
    /*! Exit BIST Mode */
    PD_PHY_EXIT_BIST,
    /*! not supported yet */
    PD_PHY_GET_BIST_MODE,
    /*! not supported yet */
    PD_PHY_GET_BIST_ERR_COUNT,
    /*! Set register VBUS_SINK_DISCONNECT */
    PD_PHY_SET_VBUS_SINK_DISCONNECT,
} pd_phy_control_t;

/*! @brief PD PHY driver notify PD stack the PHY state through these events. */
typedef enum _pd_notify_event_
{
    /*! PHY state change,
     * When PD stack receive this event, it will call PD_PHY_UPDATE_STATE control interface to update PHY state.
     * see PD_PHY_UPDATE_STATE discription.
     */
    PD_PHY_EVENT_STATE_CHANGE,
    /*! message sent complete */
    PD_PHY_EVENT_SEND_COMPLETE,
    /*! message receive complete */
    PD_PHY_EVENT_RECEIVE_COMPLETE,
    /*! receive hard reset */
    PD_PHY_EVENT_HARD_RESET_RECEIVED,
    /*! vbus state change. */
    PD_PHY_EVENT_VBUS_STATE_CHANGE,
    /*! sink disconnect. */
    PD_PHY_EVENT_VBUS_SINK_DISCONNECT,
    /*! receive fast role swap signal */
    PD_PHY_EVENT_FR_SWAP_SINGAL_RECEIVED,
    /*! vbus alarm high or low asserted */
    PD_PHY_EVENT_VBUS_ALARM,
    /*! PHY run into the unrecovered error state, need PD stack to reset PHY */
    PD_PHY_EVENT_REQUEST_STACK_RESET,
    /*! vconn protection fault */
    PD_PHY_EVENT_VCONN_PROTECTION_FAULT,
    /*! there are TYPE-C OVP or OCP fault */
    PD_PHY_EVENT_TYPEC_OVP_OCP_FAULT,
    /*! there are fault */
    PD_PHY_EVENT_FAULT_RECOVERY,
} pd_phy_notify_event_t;

/*! @brief BIST message type (not supported yet)
 */
typedef enum _pd_bist_msg
{
    kBIST_ReceiverMode = 0,
    kBIST_TransmitMode,
    kBIST_ReturnedBistCounters,
    kBIST_CarrierMode0,
    kBIST_CarrierMode1,
    kBIST_CarrierMode2,
    kBIST_CarrierMode3,
    kBIST_EyePattern,
    kBIST_TestData,
} pd_bist_mst_t;

/*! @brief PD PHY receive result information.
 * PD_PHY_EVENT_RECEIVE_COMPLETE event use this structure as parameter.
 */
typedef struct _pd_phy_rx_result
{
    /*! receive result */
    pd_status_t rxResultStatus;
    /*! message's sop */
    start_of_packet_t rxSop;
    /*! message's length */
    uint16_t rxLength;
} pd_phy_rx_result_t;

/*! @brief detect detach parameter
 * used in the PD_PHY_CONFIG_DETACH_DETECTION control.
 */
typedef struct _pd_detach_detection_param
{
    /*! power role */
    uint8_t powerRole;
    /*! used CC line after attach */
    uint8_t usedCC;
    /*! the typec connect state, the value is #typec_port_connect_state_t */
    uint8_t typecConnectState;
    /*! Rp value (Typec current level) */
    uint8_t srcRpCurrent;
    /*! if Ture: sink detect detach through CC open; if False: sink detect detach through Vbus */
    uint8_t snkDetachDetectCCOpen;
    /*! (not supported yet) debug accessory info */
    uint8_t debugUnoriented;
    /*! (not supported yet) debug accessory info */
    uint8_t debugDTS;
} pd_detach_detection_param_t;

/*! @brief detect attach parameter
 * used in the PD_PHY_CONFIG_ATTACH_DETECTION control.
 */
typedef struct _pd_attach_detection_param
{
    /*! if True: DRP; if False: normal source, sink or accessory */
    uint8_t isDRP;
    /*!
     * - isDRP is True:
     *    - source: the initial power role is source and toggling between source with sink.
     *    - sink: the initial power role is sink and toggling between source with sink.
     * - isDRP is False:
     *    - source: it is source role.
     *    - sink: it is sink role or accessory.
     */
    uint8_t powerRole;
    /*!
     * device type, it's value is #pd_device_type_t.
     */
    uint8_t deviceType;
    /*! if it is source, it configure Rp current level */
    uint8_t srcRpCurrent;
} pd_attach_detection_param_t;

/*! @brief PHY's vendor information */
typedef struct _pd_phy_vendor_info
{
    /*! vendor ID */
    uint16_t vendorID;
    /*! product ID */
    uint16_t productID;
    /*! device ID */
    uint16_t deviceID;
} pd_phy_vendor_info_t;

/*! @brief configure PHY's data role, power role or goodCRC's msg header.
 * Used as the PD_PHY_SET_MSG_HEADER_INFO control code parameter.
 */
typedef struct _pd_phy_msg_header_info
{
    /*! data role */
    uint8_t dataRole;
    /*! power role */
    uint8_t powerRole;
    /*! self is cable plug */
    uint8_t cablePlug;
    /*! spec revision */
    uint8_t revision;
} pd_phy_msg_header_info_t;

/*! @brief Get PHY's CC line state.
 * Used as the PD_PHY_GET_CC_LINE_STATE control code parameter.
 */
typedef struct _pd_phy_get_cc_state
{
    /*! CC1 line state, the value is #pd_phy_cc_state_t */
    pd_phy_cc_state_t cc1State;
    /*! CC2 line state, the value is #pd_phy_cc_state_t */
    pd_phy_cc_state_t cc2State;
} pd_phy_get_cc_state_t;

/*! @brief Get DRP is looking for connection state.
 */
typedef enum _pd_phy_look4connect_state
{
    /*! is looking for connection */
    kLook4ConnState_Looking = 0,
    /* connection is done and Rd is asserted */
    kLook4ConnState_AssertRd,
    /* connection is done and Rp is asserted */
    kLook4ConnState_AssertRp,
} pd_phy_look4connect_state_t;

/**
 * @brief PD PHY interface table structure
 */
typedef struct _pd_phy_api_interface
{
    /*!
     * @brief Initializes PD PHY module.
     *
     * This function initializes one PD PHY module instance specified by the parameters.
     *
     * @param[in] pdHandle        rPD instance handle.
     *
     * @retval kStatus_USB_Success    initialization success.
     * @retval other value            error code.
     */
    pd_status_t (*pdPhyInit)(pd_handle pdHandle);
    /*!
     * @brief de-initialize phy instance
     *
     * This function de-initializes the PD PHY module specified by the pdHandle.
     *
     * @param[in] pdHandle        PD phy instance handle.
     *
     * @retval kStatus_USB_Success    initialization success.
     * @retval other value            error code.
     */
    pd_status_t (*pdPhyDeinit)(pd_handle pdHandle);
    /*!
     * @brief send message
     *
     * This function send one message, this function is asynchronous,
     * the message send result return to PD stack by the PD_Notify(PD_PHY_EVENT_SEND_COMPLETE);
     *
     * @param[in] pdHandle        PD phy instance handle.
     * @param[in] startOfPacket    message's sop
     * @param[in] buffer           data buffer.
     * @param[in] length           data length.
     *
     * @retval kStatus_USB_Success    initialization success.
     * @retval other value            error code.
     */
    pd_status_t (*pdPhySend)(pd_handle pdHandle, uint8_t startOfPacket, uint8_t *buffer, uint32_t length);
    /*!
     * @brief receive message
     *
     * This function pend to receive message, this function is asynchronous,
     * the message receive result return to PD stack by the PD_Notify(PD_PHY_EVENT_RECEIVE_COMPLETE);
     *
     * @param[in] pdHandle         PD phy instance handle.
     * @param[in] startOfPacketMask   message's SOPs that will receive.
     * @param[in] buffer              data buffer.
     * @param[in] length              data length.
     *
     * @retval kStatus_USB_Success    initialization success.
     * @retval other value            error code.
     */
    pd_status_t (*pdPhyReceive)(pd_handle pdHandle, uint8_t startOfPacketMask, uint8_t *buffer);
    /*!
     * @brief control phy
     *
     * This function control PHY operatin.
     * The control codes are defined at enumeration #pd_phy_control_t
     *
     * @param[in] pdHandle         PD phy instance handle.
     * @param[in] control             The control code.
     * @param[in] param               The control parameter.
     *
     * @retval kStatus_USB_Success    initialization success.
     * @retval other value            error code.
     */
    pd_status_t (*pdPhyControl)(pd_handle pdHandle, pd_phy_control_t control, void *param);
} pd_phy_api_interface_t;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Notify PD stack the PHY status.
 *
 * This function is implemented in the PD stack not PHY driver.
 * PHY driver will call this function to notify PD stack that changes and process flow.
 *
 * @param[in] pdHandle      the pdHandle that is got through PD_InstanceInit.
 * @param[in] event         see the #pd_phy_notify_event_t
 * @param[in] param         the param is different for different event.
 */
void PD_Notify(pd_handle pdHandle, pd_phy_notify_event_t event, void *param);

#ifdef __cplusplus
}
#endif

/*! @}*/

#if (defined PD_CONFIG_PTN5110_PORT) && (PD_CONFIG_PTN5110_PORT)
/*!
 * @addtogroup usb_pd_phy_ptn5110
 * @{
 */

/*!
 * @brief Implement the init function interface.
 */
pd_status_t PDPTN5110_Init(pd_handle pdHandle);

/*!
 * @brief Implement the de-init function interface.
 */
pd_status_t PDPTN5110_Deinit(pd_handle pdHandle);

/*!
 * @brief Implement the control function interface.
 */
pd_status_t PDPTN5110_Control(pd_handle pdHandle, pd_phy_control_t control, void *param);

/*!
 * @brief Implement the message send function interface.
 */
pd_status_t PDPTN5110_Send(pd_handle pdHandle, uint8_t startOfPacket, uint8_t *buffer, uint32_t length);

/*!
 * @brief Implement the message receive function interface.
 */
pd_status_t PDPTN5110_Receive(pd_handle pdHandle, uint8_t startOfPacketMask, uint8_t *buffer);

/*! @}*/
#endif

#if (defined PD_CONFIG_PTN5100_PORT) && (PD_CONFIG_PTN5100_PORT)
pd_status_t PDphy_PTN5100Init(pd_handle pdHandle);
pd_status_t PDphy_PTN5100Deinit(pd_handle pdHandle);
pd_status_t PDphy_PTN5100Control(pd_handle pdHandle, uint32_t control, void *param);
pd_status_t PDphy_PTN5100Send(pd_handle pdHandle, uint8_t startOfPacket, uint8_t *buffer, uint32_t length);
pd_status_t PDphy_PTN5100Receive(pd_handle pdHandle, uint8_t startOfPacketMask, uint8_t *buffer, uint32_t length);
#endif

#endif
