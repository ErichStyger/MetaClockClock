/*
 * Copyright 2016 - 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_phy.h"
#include "usb_pd_interface.h"
#include "usb_pd_ptn5110.h"
#include "usb_pd_ptn5110_register.h"
#include "usb_pd_spec.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define PDPTN5110_VbusIsPresent(PORT) \
    ((uint8_t)(Reg_CacheReadByteField(pdInstance, STATUS, power_status, TCPC_POWER_STATUS_VBUS_PRESENT_MASK)))

#define PDPTN5110_VbusIsVSafe5V(pdInstance) PDPTN5110_VbusIsPresent(pdInstance)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

pd_status_t PDPTN5110_Init(pd_handle pdHandle)
{
    pd_instance_t *pdInstance  = (pd_instance_t *)pdHandle;
    pd_phy_config_t *phyConfig = (pd_phy_config_t *)pdInstance->pdConfig->phyConfig;
    hal_gpio_pin_config_t alertConfig;

    /* I2C initialization */
    if (PD_I2cInit(&pdInstance->i2cHandle, (uint8_t)(phyConfig->i2cInstance), phyConfig->i2cSrcClock,
                   (PD_I2cReleaseBus)(phyConfig->i2cReleaseBus)) != kStatus_PD_Success)
    {
        return kStatus_PD_Error;
    }

    if (pdInstance->i2cHandle == NULL)
    {
        return kStatus_PD_Error;
    }

    /* Alert pin initialization */
    alertConfig.direction = kHAL_GpioDirectionIn;
    alertConfig.port      = phyConfig->alertPort;
    alertConfig.pin       = phyConfig->alertPin;
    alertConfig.level     = 1;
    if (HAL_GpioInit(&pdInstance->gpioAlertHandle[0], &alertConfig) != kStatus_HAL_GpioSuccess)
    {
        (void)PD_I2cDeinit(pdInstance->i2cHandle);
        return kStatus_PD_Error;
    }
    (void)HAL_GpioInstallCallback(&pdInstance->gpioAlertHandle[0], PD_PTN5110IsrFunction, pdInstance);

    if (PDPTN5110_HalInit(pdInstance) != kStatus_PD_Success)
    {
        (void)PDPTN5110_Deinit(pdInstance);
        return kStatus_PD_Error;
    }

    /* @TEST_ANCHOR */

    return kStatus_PD_Success;
}

pd_status_t PDPTN5110_Deinit(pd_handle pdHandle)
{
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;
    (void)HAL_GpioDeinit(&pdInstance->gpioAlertHandle[0]);
    (void)PD_I2cDeinit(pdInstance->i2cHandle);
    pdInstance->i2cHandle = NULL;

    return kStatus_PD_Success;
}

pd_status_t PDPTN5110_Control(pd_handle pdHandle, pd_phy_control_t control, void *param)
{
    pd_status_t status        = kStatus_PD_Success;
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;

    /* common process */
    switch (control)
    {
        case PD_PHY_FR_SWAP_CHECK_VBUS_APPLIED:
        case PD_PHY_GET_VBUS_POWER_STATE:
        case PD_PHY_SRC_SET_TYPEC_CURRENT_CAP:
        case PD_PHY_GET_TYPEC_CURRENT_CAP:
        case PD_PHY_GET_CC_LINE_STATE:
        case PD_PHY_GET_LOOK4_CONNECTION_STATE:
        case PD_PHY_GET_BIST_MODE:
        case PD_PHY_GET_BIST_ERR_COUNT:
        case PD_PHY_CONTROL_ALERT_INTERRUPT:
            if (param == NULL)
            {
                return kStatus_PD_Error;
            }
            break;

        default:
            /* No action required. */
            break;
    }

    switch (control)
    {
        case PD_PHY_SIGNAL_FR_SWAP:
        {
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
            PDPTN5110_SignalFRSwap(pdInstance);
#endif
            break;
        }

        case PD_PHY_CONTROL_FR_SWAP:
        {
            if (*((uint8_t *)param) != 0U)
            {
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
                pdInstance->fr5VOpened = 0;
                PDPTN5110_EnableFRSwap(pdInstance);
#endif
            }
            else
            {
                PDPTN5110_DisableFRSwap(pdInstance);
            }
        }
        break;

        case PD_PHY_UPDATE_STATE:
        {
            PDPTN5110_IntcProcessIntAll(pdInstance);
            break;
        }

        case PD_PHY_CONTROL_ALERT_INTERRUPT:
        {
            if (*((uint8_t *)param) != 0U)
            {
                (void)HAL_GpioSetTriggerMode(&pdInstance->gpioAlertHandle[0], kHAL_GpioInterruptLogicZero);
#if (defined PD_CONFIG_PHY_LOW_POWER_LEVEL) && (PD_CONFIG_PHY_LOW_POWER_LEVEL)
                /* Set waked-up source to exit sleep mode */
                (void)HAL_GpioWakeUpSetting(&pdInstance->gpioAlertHandle[0], 1);
#endif
            }
            else
            {
                (void)HAL_GpioSetTriggerMode(&pdInstance->gpioAlertHandle[0], kHAL_GpioInterruptDisable);
            }
            break;
        }

        case PD_PHY_CONTROL_VBUS_DETECT:
            PDPTN5110_ControlVbusMonitorAndDetect(pdInstance, *(uint8_t *)param);
            break;

        /* enable/disable */
        case PD_PHY_CONTROL_VBUS_AUTO_DISCHARGE:
            PDPTN5110_ConnectControlAutoDischarge(pdInstance, *((uint8_t *)param));
            break;

        case PD_PHY_CONFIG_ATTACH_DETECTION:
            PDPTN5110_ConnectDetectAttach(pdInstance);
            break;

        case PD_PHY_CONFIG_DETACH_DETECTION:
            PDPTN5110_ConnectDetectDetach(pdInstance);
            break;

#if defined(PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
        case PD_PHY_SET_VBUS_SINK_DISCONNECT:
        {
            uint16_t paramVal = *(uint16_t *)param / 25U;

            if (Reg_CacheRead(pdInstance, VBUS, vbus_sink_disconnect_threshold) != paramVal)
            {
                Reg_CacheAndBusModifyWordField(pdInstance, VBUS, vbus_sink_disconnect_threshold,
                                               TCPC_VBUS_SINK_DISCONNECT_THRESHOLD_VBUS_SINK_DISCONNECT_TRIP_POINT_MASK,
                                               paramVal);
            }
            break;
        }
#endif

        /* disable attach and detach detection:
           when connect state machine start, or enter disable, error state */
        case PD_PHY_RESET_CONNECT_DETECTION:
            PDPTN5110_ConnectDetachOrDisable(pdInstance);
            break;

#if (defined PD_CONFIG_PHY_LOW_POWER_LEVEL) && (PD_CONFIG_PHY_LOW_POWER_LEVEL)
        /* enter power state */
        case PD_PHY_ENTER_LOW_POWER_STATE:
            PDPTN5110_EnterLowPowerMode(pdInstance);
            break;

        /* exit low power state */
        case PD_PHY_EXIT_LOW_POWER_STATE:
            PDPTN5110_ExitLowPowerMode(pdInstance);
            break;
#endif

        /* pd_ptn5110_ctrl_pin_t* */
        case PD_PHY_CONTROL_POWER_PIN:
            PDPTN5110_ControlVbusSourceOrSink(pdInstance, (pd_ptn5110_ctrl_pin_t *)param);
            break;

        case PD_PHY_CONNECT_SET_CC_ORIENTATION:
            PDPTN5110_ConnectSetCCOrientaion(pdInstance, *((pd_cc_type_t *)param));
            break;

        /* enable/disable */
        case PD_PHY_DISCHARGE_VBUS:
            /* discharge the vbus */
            PDPTN5110_ConnectRawVbusDischarge(pdInstance, *((uint8_t *)param));
            break;

        /* pd_power_role_t */
        /* pd_data_role_t */
        case PD_PHY_SET_MSG_HEADER_INFO:
            PDPTN5110_MsgHalSetMessageHeaderInfo(pdInstance, pdInstance->revision, (uint8_t)pdInstance->curPowerRole,
                                                 (uint8_t)pdInstance->curDataRole);
            break;

#if defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
        /* enable/disable */
        case PD_PHY_CONTROL_VCONN:
            PDPTN5110_ConnectSwitchVConn(pdInstance, *((uint8_t *)param));
            break;

        /* enable/disable */
        case PD_PHY_DISCHARGE_VCONN:
            PDPTN5110_ConnectRawVconnDischarge(pdInstance, *((uint8_t *)param));
            break;
#endif /* PD_CONFIG_VCONN_SUPPORT */

        /* typec Rp current value */
        case PD_PHY_SRC_SET_TYPEC_CURRENT_CAP:
            PDPTN5110_ConnectSrcSetTypecCurrent(pdInstance, pdInstance->ccUsed, *((uint8_t *)param) - 1U);
            break;

        /* reset phy protocol layer message function */
        case PD_PHY_RESET_MSG_FUNCTION:
            PDPTN5110_MsgHalDisableMessageRx(pdInstance);
            break;

        /* bist mode */
        case PD_PHY_ENTER_BIST:
            PDPTN5110_MsgHalEnterBist(pdInstance, (pd_bist_mst_t) * ((uint8_t *)param));
            break;

        case PD_PHY_EXIT_BIST:
            PDPTN5110_MsgHalExitBist(pdInstance);
            break;

        /* cable_reset */
        case PD_PHY_SEND_CABLE_RESET:
            PDPTN5110_MsgHalSendReset(pdInstance, 1);
            break;

        /* hard reset */
        case PD_PHY_SEND_HARD_RESET:
            PDPTN5110_MsgHalSendReset(pdInstance, 0);
            break;

        case PD_PHY_FR_SWAP_CHECK_VBUS_APPLIED:
            if (Reg_CacheRead(pdInstance, GLOBAL, pd_interface_rev) >= PD_INTERFACE_REV2VER1)
            {
                *((uint8_t *)param) = (Reg_CacheReadByteField(pdInstance, STATUS, power_status,
                                                              TCPC_POWER_STATUS_SOURCING_VBUS_MASK) != 0U) ?
                                          1U :
                                          0U;
            }
            else
            {
                *((uint8_t *)param) = pdInstance->fr5VOpened;
            }
            break;

        case PD_PHY_GET_VBUS_POWER_STATE:
        {
            uint32_t paramVal = 0;

            if (((*((uint8_t *)param)) & PD_VBUS_POWER_STATE_VSAFE0V_MASK) != 0U)
            {
                uint16_t voltage = PDPTN5110_GetVbusVoltage(pdInstance);
                if (voltage <= VBUS_VSAFE0V_MAX_THRESHOLD)
                {
                    paramVal |= PD_VBUS_POWER_STATE_VSAFE0V_MASK;
                }

#if defined(PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
                if (voltage >= VBUS_SINK_DISCONNECT_THRESHOLD)
                {
                    paramVal |= PD_VBUS_POWER_STATE_SINK_DISCONNECT;
                }
#endif

                if ((voltage <= VBUS_VSAFE5V_MAX_THRESHOLD) && (voltage >= VBUS_VSAFE5V_MIN_THRESHOLD))
                {
                    if (PDPTN5110_VbusIsPresent(pdInstance) == 0U)
                    {
                        PDPTN5110_RegCacheSynC(pdInstance, kRegModule_PowerStatus);
                    }
                    paramVal |= PD_VBUS_POWER_STATE_VSAFE5V_MASK;
                }
            }
            else
            {
                if (PDPTN5110_VbusIsPresent(pdInstance) != 0U)
                {
                    paramVal |= (PD_VBUS_POWER_STATE_VBUS_MASK | PD_VBUS_POWER_STATE_VSAFE5V_MASK);
                }
            }

            if (PDPTN5110_VbusIsPresent(pdInstance) != 0U)
            {
                paramVal |= PD_VBUS_POWER_STATE_VBUS_MASK;
            }

            if (PDPTN5110_VsysIsPresent(pdInstance) != 0U)
            {
                paramVal |= PD_VBUS_POWER_STATE_VSYS_MASK;
            }

            *((uint8_t *)param) = (uint8_t)(paramVal & (*((uint8_t *)param)));
            break;
        }

        /* Read the current current limit */
        case PD_PHY_GET_TYPEC_CURRENT_CAP:
            *((uint8_t *)param) = PDPTN5110_ConnectGetTypeCCurrent(pdInstance, pdInstance->ccUsed);
            break;

        case PD_PHY_GET_CC_LINE_STATE:
            PDPTN5110_ConnectGetCC(pdInstance, (pd_phy_get_cc_state_t *)param);
            break;

        case PD_PHY_GET_LOOK4_CONNECTION_STATE:
            /* cc_status will be set as 0xFF when updating ROLE_CONTROL. If there is no cc_status interrupt, the
             * LOOKING4_CONNECTION keeps as set */
            if ((Reg_CacheRead(pdInstance, STATUS, cc_status) & TCPC_CC_STATUS_LOOKING4_CONNECTION_MASK) != 0U)
            {
                *((uint8_t *)param) = (uint8_t)kLook4ConnState_Looking;
            }
            else
            {
                if ((Reg_CacheRead(pdInstance, STATUS, cc_status) & TCPC_CC_STATUS_CONNECT_RESULT_MASK) != 0U)
                {
                    *((uint8_t *)param) = (uint8_t)kLook4ConnState_AssertRd;
                }
                else
                {
                    *((uint8_t *)param) = (uint8_t)kLook4ConnState_AssertRp;
                }
            }
            break;

#if defined(PD_CONFIG_VBUS_ALARM_SUPPORT) && (PD_CONFIG_VBUS_ALARM_SUPPORT)
        /* enable/disable */
        case PD_PHY_CONTROL_VBUS_ALARM:
            PDPTN5110_ControlVbusVoltageAlarm(pdInstance, *((uint8_t *)param));
            break;
#endif

        case PD_PHY_INFORM_VBUS_VOLTAGE_RANGE:
            PDPTN5110_SetVbusVoltageAlarmHiLo(pdInstance, *(uint32_t *)param);
            break;

        case PD_PHY_GET_BIST_MODE:
            *((uint8_t *)param) = 0;
            break;

        case PD_PHY_GET_BIST_ERR_COUNT:
            *((uint8_t *)param) = 0; /* TCPC doesn't support ErrorCnt, so response 0 here. */
            break;

        default:
            status = kStatus_PD_Error;
            break;
    }

    return status;
}

pd_status_t PDPTN5110_Send(pd_handle pdHandle, uint8_t startOfPacket, uint8_t *buffer, uint32_t length)
{
    pd_msg_header_t msgHeader;
    pd_extended_msg_header_t extendMsgHeader;
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;

    msgHeader.msgHeaderVal               = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)buffer + 2));
    extendMsgHeader.extendedMsgHeaderVal = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)buffer + 4));

    /* Need to control TX and RX sop together. Otherwise, could not receive GoodCRC response */
    PDPTN5110_MsgHalSetRxSopEnable(pdInstance,
                                   ((uint8_t)1U << startOfPacket) | Reg_CacheRead(pdInstance, MSG_RX, receive_detect));

    if ((msgHeader.bitFields.extended == 0U) || (extendMsgHeader.bitFields.chunked != 0U))
    {
        /* control, data or chunked message */
        return PDPTN5110_MsgHalSendControlDataOrChunked(pdInstance, startOfPacket, buffer, (uint8_t)length);
    }
#if defined(PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
    else
    {
        /* unchunked message */
        return PDPTN5110_MsgHalSendUnchunked(pdInstance, startOfPacket, buffer, (uint16_t)length);
    }
#else
    return kStatus_PD_Error;
#endif
}

pd_status_t PDPTN5110_Receive(pd_handle pdHandle, uint8_t startOfPacketMask, uint8_t *buffer)
{
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;

    /* check PHY has pending message. */
    if (pdInstance->pendingMsg != 0U)
    {
        pdInstance->pendingMsg = 0;

        /* rxLength exclude RECEIVE_BYTE_COUNT and RX_BUF_FRAME_TYPE, so plus 2 here. */
        (void)memcpy(buffer, (uint8_t *)&pdInstance->receivingDataCache[0],
                     (uint32_t)pdInstance->cacheRxResult.rxLength + 2U);
        PD_Notify(pdInstance, PD_PHY_EVENT_RECEIVE_COMPLETE, (void *)&(pdInstance->cacheRxResult));

        return kStatus_PD_Success;
    }

    PDPTN5110_MsgHalSetRxSopEnable(pdInstance, startOfPacketMask);

    return kStatus_PD_Success;
}

void PD_PTN5110IsrFunction(pd_handle pdHandle)
{
    uint8_t alertPinState;
    pd_instance_t *pdInstance;

    if (pdHandle != NULL)
    {
        alertPinState = 1;
        pdInstance    = (pd_instance_t *)pdHandle;

        (void)HAL_GpioGetInput(pdInstance->gpioAlertHandle, &alertPinState);
        if (alertPinState == 0U)
        {
#if (defined(__DSC__) && defined(__CW__))
            if (pdInstance->pdSetEvent == 0U)
            {
#endif
                PD_Notify(pdHandle, PD_PHY_EVENT_STATE_CHANGE, NULL);
#if (defined(__DSC__) && defined(__CW__))
                pdInstance->pdSetEvent = 1;
            }
#endif
        }
    }
}

void PDPTN5110_ConnectDetectAttach(pd_instance_t *pdInstance)
{
    uint8_t typecSrcCurrent = pdInstance->pdPowerPortConfig->typecSrcCurrent;

    /* Disable auto discharge in detached state */
    PDPTN5110_ConnectControlAutoDischarge(pdInstance, 0);

    switch (pdInstance->curConnectState)
    {
        case TYPEC_UNATTACHED_SRC:
        case TYPEC_UNATTACHED_ACCESSORY:
        case TYPEC_TRY_SRC:
        case TYPEC_TRY_WAIT_SRC:
            if (typecSrcCurrent < 1U)
            {
                break;
            }
            PDPTN5110_ConnectAssertRpUnattached(pdInstance, typecSrcCurrent - 1U);
            break;
        case TYPEC_UNATTACHED_SNK:
        case TYPEC_TRY_SNK:
        case TYPEC_TRY_WAIT_SNK:
            PDPTN5110_ConnectAssertRdUnattached(pdInstance);
            break;
        case TYPEC_TOGGLE_SRC_FIRST:
            if (typecSrcCurrent < 1U)
            {
                break;
            }
            PDPTN5110_ConnectAssertDrpUnattached(pdInstance, 1, typecSrcCurrent - 1U);
            break;
        case TYPEC_TOGGLE_SNK_FIRST:
            if (typecSrcCurrent < 1U)
            {
                break;
            }
            PDPTN5110_ConnectAssertDrpUnattached(pdInstance, 0, typecSrcCurrent - 1U);
            break;
        default:
            /* No action required. */
            break;
    }

    /* Update role control cache to register */
    PDPTN5110_ConnectSyncRoleControl(pdInstance);
}

void PDPTN5110_ConnectDetectDetach(pd_instance_t *pdInstance)
{
    switch (pdInstance->connectState)
    {
        case kTYPEC_ConnectSource:
            if (pdInstance->pdPowerPortConfig->typecSrcCurrent < 1U)
            {
                break;
            }
            PDPTN5110_ConnectAssertRpAttached(pdInstance, pdInstance->ccUsed,
                                              pdInstance->pdPowerPortConfig->typecSrcCurrent - 1U);
            break;
        case kTYPEC_ConnectSink:
            PDPTN5110_ConnectAssertRdAttached(pdInstance, pdInstance->ccUsed);
            break;
#if (defined PD_CONFIG_AUDIO_ACCESSORY_SUPPORT) && (PD_CONFIG_AUDIO_ACCESSORY_SUPPORT)
        case kTYPEC_ConnectAudioAccessory:
            PDPTN5110_ConnectAudioAccessoryAttached(pdInstance, pdInstance->ccUsed);
            break;
#endif
#if defined(PD_CONFIG_DEBUG_ACCESSORY_SUPPORT) && (PD_CONFIG_DEBUG_ACCESSORY_SUPPORT)
        case kTYPEC_ConnectDebugAccessory:
        {
            switch (pdInstance->curConnectState)
            {
                case TYPEC_UNORIENTED_DEBUG_ACCESSORY_SRC:
                    if (PD_CONFIG_DEBUG_ACCESSORY_ROLE == CONFIG_DEBUG_ACCESSORY_DTS)
                    {
                        PDPTN5110_ConnectAssertRpDbgAccSrc(pdInstance);
                    }
                    break;
                case TYPEC_ORIENTED_DEBUG_ACCESSORY_SRC:
                    PDPTN5110_ConnectDbgAccSrcAttached(pdInstance);
                    break;
                case TYPEC_DEBUG_ACCESSORY_SNK:
                    if (PD_CONFIG_DEBUG_ACCESSORY_ROLE == CONFIG_DEBUG_ACCESSORY_DTS)
                    {
                        PDPTN5110_ConnectAssertRdDbgAccSnk(pdInstance);
                    }
                    break;
                case TYPEC_ORIENTED_DEBUG_ACCESSORY_SNK:
                    PDPTN5110_ConnectDbgAccSnkAttached(pdInstance);
                    break;
                default:
                    /* No action required. */
                    break;
            }
            break;
        }
#endif
        default:
            /* No action required. */
            break;
    }

    /* Update role control cache to register */
    PDPTN5110_ConnectSyncRoleControl(pdInstance);
}

#if (defined PD_CONFIG_PHY_LOW_POWER_LEVEL) && (PD_CONFIG_PHY_LOW_POWER_LEVEL)
void PDPTN5110_EnterLowPowerMode(pd_instance_t *pdInstance)
{
    uint8_t getInfo;
    uint16_t getInfoWord;

    if (pdInstance->isConnected == 0U)
    {
        /* unattached state */
        pdInstance->lowPowerState = 0xFFU;
    }
    else
    {
        /* attached state and idle */
#if (PD_CONFIG_PHY_LOW_POWER_LEVEL == 1U)
        pdInstance->lowPowerState = 0x01U;
#elif (PD_CONFIG_PHY_LOW_POWER_LEVEL == 2U)
        pdInstance->lowPowerState = 0x03U;
#else
        pdInstance->lowPowerState = 0x00U;
#endif
    }

    if ((pdInstance->lowPowerState & 0x02U) != 0U)
    {
        /* Sync control register to local cache in order to restore remote register value when exiting low power state.
         */
        PDPTN5110_RegCacheSynC(pdInstance, kRegModule_Control);

        /* Disable Vbus detection when Type-C is unattched state to reduce the current consumption.
           When sourcing or sinking Vbus, if disabling Vbus Detection, Alert register will assert Fault bit.
           Fault status register will assert I2C interface error. */
        if (pdInstance->isConnected == 0U)
        {
            /* Mask power status interrupt */
            getInfoWord = Reg_CacheRead(pdInstance, MASK, alert_mask);
            getInfoWord &= ~(uint16_t)TCPC_ALERT_MASK_PORT_POWER_STATUS_INTERRUPT_MASK_MASK;
            Reg_BusWriteWord(pdInstance, alert_mask, getInfoWord);
            /* Mask Vbus detection status */
            getInfo = Reg_CacheRead(pdInstance, MASK, power_status_mask);
            getInfo &= ~(uint8_t)TCPC_POWER_STATUS_MASK_VBUS_PRESENT_DETECTION_STATUS_INTERRUPT_MASK_MASK;
            Reg_BusWriteByte(pdInstance, power_status_mask, getInfo);
            /* Before disabling Vbus detection, need to mask power status interrupt.
               Because writing DisableVbusDetect to COMMAND register, Alert register will assert power status bit.
               Power status register will set VBUS Detection Enabled to 1, which will generate a interrupt and
               wake-up PD stack to exit low power state, which will prevent PD stack from entering low power state. */
            Reg_BusWriteByte(pdInstance, command, (uint8_t)TCPC_DISABLEVBUSDETECT);
        }

        getInfo = Reg_CacheRead(pdInstance, CONTROL, power_control);
        /* Disable Fast Role Swap. If not disabling FR Swap in sink role, the power consumption will not decrease.
           When FRS is enabled, the state machine in the PTN5110 will automatically enable the ADC circuitry to allow
           it to read VBUS voltage. This is done to make sure VBUS falls below Vsafe5V before the sink turns on the
           source FET to provide 5V to VBUS. */
        getInfo &= ~(uint8_t)TCPC_POWER_CONTROL_FAST_ROLE_SWAP_ENABLE_MASK;
        /* Disable VBUS Monitoring */
        getInfo |= TCPC_POWER_CONTROL_VBUS_VOLTAGE_MONITOR_MASK;
        /* Disable VBUS Voltage Alarm */
        getInfo |= TCPC_POWER_CONTROL_DISABLE_VOLTAGE_ALARMS_MASK;
        /* Disable VBUS Auto Discharge */
        getInfo &= ~(uint8_t)TCPC_POWER_CONTROL_AUTO_DISCHARGE_DISCONNECT_MASK;
        Reg_BusWriteByte(pdInstance, power_control, getInfo);
        Reg_BusWriteWord(pdInstance, vbus_sink_disconnect_threshold, 0x0000U);

        /* Disable Fault Status Reporting */
        getInfo = Reg_CacheRead(pdInstance, CONTROL, fault_control);
        getInfo |= TCPC_FAULT_CONTROL_FORCE_OFF_VBUS_ENABLE_MASK | TCPC_FAULT_CONTROL_VBUS_DISCHARGE_TIMER_MASK |
                   TCPC_FAULT_CONTROL_VBUS_OVER_CURRENT_PROTECTION_FAULT_ENABLE_MASK |
                   TCPC_FAULT_CONTROL_VBUS_OVER_VOLTAGE_PROTECTION_FAULT_ENABLE_MASK |
                   TCPC_FAULT_CONTROL_VCONN_OVER_CURRENT_FAULT_ENABLE_MASK;
        Reg_BusWriteByte(pdInstance, fault_control, getInfo);

        /* Enable I2C clock stretching */
        getInfo = Reg_CacheRead(pdInstance, CONTROL, tcpc_control);
        getInfo |= TCPC_TCPC_CONTROL_I2_C_CLOCK_STRETCHING_CONTROL_MASK;
        Reg_BusWriteByte(pdInstance, tcpc_control, getInfo);
    }
}

void PDPTN5110_ExitLowPowerMode(pd_instance_t *pdInstance)
{
    if ((pdInstance->lowPowerState & 0x02U) != 0U)
    {
        /* Disable I2C clock stretching.
           Disable TCPM controls debug accessory here before setting auto discharge, or TCPC will assert
           FaultStatus.i2cInterfaceError. */
        Reg_BusWriteByte(pdInstance, tcpc_control,
                         Reg_CacheRead(pdInstance, CONTROL, tcpc_control) &
                             (~(TCPC_TCPC_CONTROL_DEBUG_ACCESSORY_CONTROL_MASK |
                                TCPC_TCPC_CONTROL_I2_C_CLOCK_STRETCHING_CONTROL_MASK)));
        Reg_BusWriteByte(pdInstance, power_control, Reg_CacheRead(pdInstance, CONTROL, power_control));
        Reg_BusWriteByte(pdInstance, fault_control, Reg_CacheRead(pdInstance, CONTROL, fault_control));

        Reg_BusWriteWord(pdInstance, vbus_sink_disconnect_threshold,
                         Reg_CacheRead(pdInstance, VBUS, vbus_sink_disconnect_threshold));

        Reg_BusWriteByte(pdInstance, tcpc_control, Reg_CacheRead(pdInstance, CONTROL, tcpc_control));

        if (pdInstance->isConnected == 0U)
        {
            /* Enable Vbus Detection */
            Reg_BusWriteByte(pdInstance, command, (uint8_t)TCPC_ENABLEVBUSDETECT);
            /* Unmask power status interrupt */
            Reg_BusWriteWord(pdInstance, alert_mask, Reg_CacheRead(pdInstance, MASK, alert_mask));
            /* Unmask Vbus detection status */
            Reg_BusWriteByte(pdInstance, power_status_mask, Reg_CacheRead(pdInstance, MASK, power_status_mask));
        }
    }

    pdInstance->lowPowerState = 0U;
}
#endif

/* I2C, read byte */
uint8_t Reg_BusReadByteFun(pd_instance_t *pdInstance, uint32_t reg)
{
    uint8_t regVal = 0xFFU;
    (void)PD_I2cReadBlocking(pdInstance->i2cHandle, ((pd_phy_config_t *)pdInstance->pdConfig->phyConfig)->slaveAddress,
                             reg, 1, &regVal, 1);
    return regVal;
}

/* I2C, write byte */
void Reg_BusWriteByteFun(pd_instance_t *pdInstance, uint32_t reg, uint8_t byteData)
{
    (void)PD_I2cWriteBlocking(pdInstance->i2cHandle, ((pd_phy_config_t *)pdInstance->pdConfig->phyConfig)->slaveAddress,
                              reg, 1, &byteData, 1);
}

/* I2C, read word */
uint16_t Reg_BusReadWordFun(pd_instance_t *pdInstance, uint32_t reg)
{
    uint16_t regVal = 0xFFFFU;
    (void)PD_I2cReadBlocking(pdInstance->i2cHandle, ((pd_phy_config_t *)pdInstance->pdConfig->phyConfig)->slaveAddress,
                             reg, 1, (uint8_t *)&regVal, 2);
    return regVal;
}

/* I2C, write word */
void Reg_BusWriteWordFun(pd_instance_t *pdInstance, uint32_t reg, uint16_t wordData)
{
    (void)PD_I2cWriteBlocking(pdInstance->i2cHandle, ((pd_phy_config_t *)pdInstance->pdConfig->phyConfig)->slaveAddress,
                              reg, 1, (uint8_t *)&wordData, 2);
}

/* I2C, read block */
pd_status_t Reg_BusReadBlockFun(pd_instance_t *pdInstance, uint32_t reg, uint32_t length, uint8_t *dst)
{
    return PD_I2cReadBlocking(pdInstance->i2cHandle, ((pd_phy_config_t *)pdInstance->pdConfig->phyConfig)->slaveAddress,
                              reg, 1, dst, length);
}

/* I2C, write block */
pd_status_t Reg_BusWriteBlockFun(pd_instance_t *pdInstance, uint32_t reg, uint32_t length, uint8_t *src)
{
    return PD_I2cWriteBlocking(pdInstance->i2cHandle,
                               ((pd_phy_config_t *)pdInstance->pdConfig->phyConfig)->slaveAddress, reg, 1, src, length);
}

/* I2C, read-modify-write byte */
void Reg_BusModifyByteFieldFun(pd_instance_t *pdInstance, uint32_t reg, uint16_t mask, uint8_t updateVal)
{
    uint8_t regVal = 0xFFU;
    (void)PD_I2cReadBlocking(pdInstance->i2cHandle, ((pd_phy_config_t *)pdInstance->pdConfig->phyConfig)->slaveAddress,
                             reg, 1, &regVal, 1);
    regVal = (uint8_t)((((uint32_t)updateVal) & ((uint32_t)mask)) | ((uint32_t)regVal & ~((uint32_t)mask)));
    (void)PD_I2cWriteBlocking(pdInstance->i2cHandle, ((pd_phy_config_t *)pdInstance->pdConfig->phyConfig)->slaveAddress,
                              reg, 1, &regVal, 1);
}

/* I2C, read-modify-write word */
void Reg_BusModifyWordFieldFun(pd_instance_t *pdInstance, uint32_t reg, uint16_t mask, uint16_t updateVal)
{
    uint16_t regVal = 0xFFFFU;
    (void)PD_I2cReadBlocking(pdInstance->i2cHandle, ((pd_phy_config_t *)pdInstance->pdConfig->phyConfig)->slaveAddress,
                             reg, 1, (uint8_t *)&regVal, 2);
    regVal = (uint16_t)((((uint32_t)updateVal) & ((uint32_t)mask)) | ((uint32_t)regVal & ~((uint32_t)mask)));
    (void)PD_I2cWriteBlocking((pdInstance)->i2cHandle,
                              ((pd_phy_config_t *)pdInstance->pdConfig->phyConfig)->slaveAddress, reg, 1,
                              (uint8_t *)&regVal, 2);
}
