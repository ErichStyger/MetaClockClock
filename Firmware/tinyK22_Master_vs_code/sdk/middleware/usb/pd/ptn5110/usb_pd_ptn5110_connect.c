/*
 * Copyright 2015 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_phy.h"
#include "usb_pd_interface.h"
#include "usb_pd_ptn5110.h"
#include "usb_pd_ptn5110_register.h"
#include "usb_pd_timer.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void PDPTN5110_IocSetIlim(pd_instance_t *pdInstance, uint8_t enable);
static void PDPTN5110_ConnectSetCCOpenDrain(pd_instance_t *pdInstance);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Control the ILIM_5V_VBUS pin.
 *
 * This function is used to control the ILIM_5V_VBUS pin to drive high level or low level.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param enable          1 - ILIM_5V_VBUS pin output high level
 *                        0 - ILIM_5V_VBUS pin output low level
 *
 * @return None.
 */
static void PDPTN5110_IocSetIlim(pd_instance_t *pdInstance, uint8_t enable)
{
    if (enable != 0U)
    {
        /* ILIM_5V_VBUS drive High */
        Reg_BusSetBit(pdInstance, ptn5110_ext_gpio_control, PTN5110_EXT_GPIO_CONTROL_ILIM_5V_VBUS_MASK);
    }
    else
    {
        /* ILIM_5V_VBUS drive Low */
        Reg_BusClrBit(pdInstance, ptn5110_ext_gpio_control, PTN5110_EXT_GPIO_CONTROL_ILIM_5V_VBUS_MASK);
    }
    /* Set the ILIM output direction */
    Reg_BusSetBit(pdInstance, ptn5110_ext_gpio_config, PTN5110_EXT_GPIO_CONFIG_ILIM_5V_VBUS_MASK);
}

/*!
 * @brief Control automatically discharge upon a disconnect detection
 *
 * This function is used to control automatically discharge upon a disconnect detection.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param enable          Enable command. 0 - disable, 1 - enable
 *
 * @return None.
 */
void PDPTN5110_ConnectControlAutoDischarge(pd_instance_t *pdInstance, uint8_t enable)
{
    uint8_t autoDischarge = (enable != 0U) ? TCPC_POWER_CONTROL_AUTO_DISCHARGE_DISCONNECT_MASK : 0U;
    uint8_t tcpcControl;

    if (Reg_CacheReadByteField(pdInstance, CONTROL, power_control, TCPC_POWER_CONTROL_AUTO_DISCHARGE_DISCONNECT_MASK) !=
        autoDischarge)
    {
        /* During fast role swap, cannot enable autodischarge, or TCPC will assert FaultStatus.i2cInterfaceError */
        if (pdInstance->inProgress != kVbusPower_InFRSwap)
        {
            /* Disable TCPM controls debug accessory here before setting auto discharge, or TCPC will assert
               FaultStatus.i2cInterfaceError.*/
            tcpcControl = 0U;
            if (Reg_CacheReadByteField(pdInstance, CONTROL, tcpc_control,
                                       TCPC_TCPC_CONTROL_DEBUG_ACCESSORY_CONTROL_MASK) != 0U)
            {
                tcpcControl = 1U;
                PDPTN5110_ConnectControlDebugAccessory(pdInstance, 0U);
            }

            Reg_CacheAndBusModifyByteField(pdInstance, CONTROL, power_control,
                                           TCPC_POWER_CONTROL_AUTO_DISCHARGE_DISCONNECT_MASK, autoDischarge);

            /* Restore debug accessory. */
            PDPTN5110_ConnectControlDebugAccessory(pdInstance, tcpcControl);
        }
        else
        {
            Reg_CacheAndBusModifyByteField(pdInstance, CONTROL, power_control,
                                           TCPC_POWER_CONTROL_AUTO_DISCHARGE_DISCONNECT_MASK, 0);
        }
    }
}

/*!
 * @brief Get both CC pins states.
 *
 * This function is used to get both CC pins states.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param ccStates        It is out parameter, is used to return pointer of CC pins states.
 *                        Please refer to the enumeration pd_phy_cc_state_t.
 *
 * @return None.
 */
void PDPTN5110_ConnectGetCC(pd_instance_t *pdInstance, pd_phy_get_cc_state_t *ccStates)
{
    uint8_t cc1Status;
    uint8_t cc2Status;
    uint8_t roleControl = Reg_CacheRead(pdInstance, CONTROL, role_control);
    uint8_t rawStatus   = Reg_CacheRead(pdInstance, STATUS, cc_status);
    ccStates->cc1State  = kCCState_Unknown;
    ccStates->cc2State  = kCCState_Unknown;

    if (PD_TimerCheckInvalidOrTimeOut(pdInstance, timrCCFilter) != 0U)
    {
        if (rawStatus == 0xFFU)
        {
            PDPTN5110_RegCacheSynC(pdInstance, kRegModule_CCStatus);
        }
    }

    rawStatus = Reg_CacheRead(pdInstance, STATUS, cc_status);

    cc1Status = rawStatus & TCPC_CC_STATUS_CC1_STATE_MASK;
    cc2Status = rawStatus & TCPC_CC_STATUS_CC2_STATE_MASK;

    /* If we are still looking, then just use the previous value */
    if ((rawStatus & TCPC_CC_STATUS_LOOKING4_CONNECTION_MASK) != 0U)
    {
        if (rawStatus == 0xFFU)
        {
            ccStates->cc1State = kCCState_Unstable;
            ccStates->cc2State = kCCState_Unstable;
        }
        return;
    }

    if ((rawStatus & TCPC_CC_STATUS_CONNECT_RESULT_MASK) == 0U) /* Rp */
    {
        if ((roleControl & TCPC_ROLE_CONTROL_CC1_MASK) == (uint8_t)ROLE_CONTROL_CC1_OPEN)
        {
            ccStates->cc1State = kCCState_Unknown;
        }
        else
        {
            switch ((CC1_State_t)cc1Status)
            {
                case CC_STATUS_CC1_STATE_SRC_OPEN:
                    ccStates->cc1State = kCCState_SrcOpen;
                    break;
                case CC_STATUS_CC1_STATE_SRC_RA:
                    ccStates->cc1State = kCCState_SrcRa;
                    break;
                case CC_STATUS_CC1_STATE_SRC_RD:
                    ccStates->cc1State = kCCState_SrcRd;
                    break;
                default:
                    ccStates->cc1State = kCCState_Unknown;
                    break;
            }
        }

        if ((roleControl & TCPC_ROLE_CONTROL_CC2_MASK) == (uint8_t)ROLE_CONTROL_CC2_OPEN)
        {
            ccStates->cc2State = kCCState_Unknown;
        }
        else
        {
            switch ((CC2_State_t)cc2Status)
            {
                case CC_STATUS_CC2_STATE_SRC_OPEN:
                    ccStates->cc2State = kCCState_SrcOpen;
                    break;
                case CC_STATUS_CC2_STATE_SRC_RA:
                    ccStates->cc2State = kCCState_SrcRa;
                    break;
                case CC_STATUS_CC2_STATE_SRC_RD:
                    ccStates->cc2State = kCCState_SrcRd;
                    break;
                default:
                    ccStates->cc2State = kCCState_Unknown;
                    break;
            }
        }
    }
    else /* Rd */
    {
        if ((roleControl & TCPC_ROLE_CONTROL_CC1_MASK) == (uint8_t)ROLE_CONTROL_CC1_OPEN)
        {
            ccStates->cc1State = kCCState_Unknown;
        }
        else
        {
            switch ((CC1_State_t)cc1Status)
            {
                case CC_STATUS_CC1_STATE_SNK_DEFAULT:
                    ccStates->cc1State = kCCState_SnkRpDefault;
                    break;
                case CC_STATUS_CC1_STATE_SNK_POWER1_5:
                    ccStates->cc1State = kCCState_SnkRp1_5;
                    break;
                case CC_STATUS_CC1_STATE_SNK_POWER3_0:
                    ccStates->cc1State = kCCState_SnkRp3_0;
                    break;
                case CC_STATUS_CC1_STATE_SNK_OPEN:
                    ccStates->cc1State = kCCState_SnkOpen;
                    break;
                default:
                    /* No action required. */
                    break;
            }
        }

        if ((roleControl & TCPC_ROLE_CONTROL_CC2_MASK) == (uint8_t)ROLE_CONTROL_CC2_OPEN)
        {
            ccStates->cc2State = kCCState_Unknown;
        }
        else
        {
            switch ((CC2_State_t)cc2Status)
            {
                case CC_STATUS_CC2_STATE_SNK_DEFAULT:
                    ccStates->cc2State = kCCState_SnkRpDefault;
                    break;
                case CC_STATUS_CC2_STATE_SNK_POWER1_5:
                    ccStates->cc2State = kCCState_SnkRp1_5;
                    break;
                case CC_STATUS_CC2_STATE_SNK_POWER3_0:
                    ccStates->cc2State = kCCState_SnkRp3_0;
                    break;
                case CC_STATUS_CC2_STATE_SNK_OPEN:
                    ccStates->cc2State = kCCState_SnkOpen;
                    break;
                default:
                    /* No action required. */
                    break;
            }
        }
    }

    return;
}

/*!
 * @brief Reset connection when connection is detached or disabled
 *
 * This function is used to reset connection when connection is detached or disabled
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return None.
 */
void PDPTN5110_ConnectDetachOrDisable(pd_instance_t *pdInstance)
{
    pd_ptn5110_ctrl_pin_t set = {0, 0};

#if defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
    /* Disable VCONN */
    PDPTN5110_ConnectSwitchVConn(pdInstance, 0);
#endif

    /* Stop sourcing or sinking Vbus */
    PDPTN5110_ControlVbusSourceOrSink(pdInstance, (pd_ptn5110_ctrl_pin_t *)(&set));

    /* Set CC to open drain*/
    PDPTN5110_ConnectSetCCOpenDrain(pdInstance);

    /* Disable Vbus monitor and detect when detached */
    PDPTN5110_ControlVbusMonitorAndDetect(pdInstance, 0);

    /* Disable Vbus voltage alarms */
    PDPTN5110_ControlVbusVoltageAlarm(pdInstance, 0);

    /* Disable AutoDischarge before waiting for connection */
    PDPTN5110_ConnectControlAutoDischarge(pdInstance, 0);

    /* reenable CC_Status change alert */
    PDPTN5110_IntcIrqClearAndEnable(pdInstance, TCPC_ALERT_MASK_CC_STATUS_INTERRUPT_MASK_MASK);
}

/*!
 * @brief Enable or disable the forced discharge function of Vbus.
 *
 * This function is used to discharge Vbus.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param enable          1 - Enable commands and discharge path is enabled.
 *                        0 - Vbus discharge path is disabled
 *
 * @return None.
 */
void PDPTN5110_ConnectRawVbusDischarge(pd_instance_t *pdInstance, uint8_t enable)
{
    uint8_t forceDischarge = (enable != 0U) ? TCPC_POWER_CONTROL_FORCE_DISCHARGE_MASK : 0U;

    if (Reg_CacheReadByteField(pdInstance, CONTROL, power_control, TCPC_POWER_CONTROL_FORCE_DISCHARGE_MASK) !=
        forceDischarge)
    {
        Reg_CacheAndBusModifyByteField(pdInstance, CONTROL, power_control, TCPC_POWER_CONTROL_FORCE_DISCHARGE_MASK,
                                       forceDischarge);
    }
}

#if defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
/*!
 * @brief Select the CC line to route to the Vconn source.
 *
 * This function is used to enable or disable Vconn source.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param enable          1 - Enable commands and route according to cc_orient_reverse
 *                        0 - Disable switch
 *
 * @return None.
 */
void PDPTN5110_ConnectSwitchVConn(pd_instance_t *pdInstance, uint8_t enable)
{
    uint8_t vconnSelectMask = TCPC_POWER_CONTROL_ENABLE_VCONN_MASK | TCPC_POWER_CONTROL_VCONN_POWER_SUPPORTED_MASK;
    uint8_t vconnSelect     = (enable != 0U) ? vconnSelectMask : 0U;

    if (Reg_CacheReadByteField(pdInstance, CONTROL, power_control, vconnSelectMask) != vconnSelect)
    {
        /* Enable Vconn must be set after roleControl to ensure there is no overlap */
        Reg_CacheAndBusModifyByteField(pdInstance, CONTROL, power_control, vconnSelectMask, vconnSelect);
    }
}

/*!
 * @brief Enable or disable the discharge function of Vconn.
 *
 * This function is used to discharge Vconn.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param enable          1 - Enable commands and discharge path is enabled.
 *                        0 - Vconn discharge path is disabled
 *
 * @return None.
 */
void PDPTN5110_ConnectRawVconnDischarge(pd_instance_t *pdInstance, uint8_t enable)
{
    if (Reg_CacheRead(pdInstance, GLOBAL, vendor_id) == PD_VENDOR_ID_NXP &&
        Reg_CacheRead(pdInstance, GLOBAL, product_id) == PRODUCT_ID_PTN5110)
    {
        Reg_BusModifyByteField(pdInstance, ptn5110_ext_control, PTN5110_EXT_CONTROL_VCONN_FORCE_DISCHARGE_MASK,
                               (enable != 0U) ? PTN5110_EXT_CONTROL_VCONN_FORCE_DISCHARGE_MASK : 0u);
    }
}
#endif /* PD_CONFIG_VCONN_SUPPORT */

/*!
 * @brief Set CC communication orientation.
 *
 * This function is used to set Type-C orientation
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit
 * @param usedCC          Current communication CC pin. Please refer to the enumeration pd_cc_type_t.
 *
 * @return None.
 */
void PDPTN5110_ConnectSetCCOrientaion(pd_instance_t *pdInstance, pd_cc_type_t usedCC)
{
    Reg_CacheAndBusModifyByteField(pdInstance, CONTROL, tcpc_control, TCPC_TCPC_CONTROL_PLUG_ORIENTATION_MASK,
                                   (usedCC == kPD_CC2) ? TCPC_TCPC_CONTROL_PLUG_ORIENTATION_MASK : 0U);
}

/*!
 * @brief Debug Accessory Control.
 *
 * This function is used to control Debug Accessory controlled by TCPC or TCPM.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param enable          Debug Accessory controlled by TCPC or TCPM. 0 - by TCPC, 1 - by TCPM
 *
 * @return None.
 */
void PDPTN5110_ConnectControlDebugAccessory(pd_instance_t *pdInstance, uint8_t enable)
{
    uint8_t debugAccessory = (enable != 0U) ? TCPC_POWER_CONTROL_AUTO_DISCHARGE_DISCONNECT_MASK : 0U;

    if (Reg_CacheReadByteField(pdInstance, CONTROL, tcpc_control, TCPC_TCPC_CONTROL_DEBUG_ACCESSORY_CONTROL_MASK) !=
        debugAccessory)
    {
        Reg_CacheAndBusModifyByteField(pdInstance, CONTROL, tcpc_control,
                                       TCPC_TCPC_CONTROL_DEBUG_ACCESSORY_CONTROL_MASK, debugAccessory);
    }
}

/*!
 * @brief Update role control register cache to TCPC register
 *
 * This function is used to set role control register. This function should be called when the role control
 * register cache is changed.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return None.
 */
void PDPTN5110_ConnectSyncRoleControl(pd_instance_t *pdInstance)
{
    uint8_t roleControl;

    roleControl = Reg_CacheRead(pdInstance, CONTROL, role_control);

    /* Set CC cache to invalid value when role control register is updated. */
    Reg_CacheWrite(pdInstance, STATUS, cc_status, 0xFFU);
    /* A TCPM which is using polling rather than Alerts should assume the data in the CC_STATUS
       register is not valid until at least tCcStatusDelay(max 200us) + tTCPCFilter(250~500us) +
       tCcTCPCSampleRate (max) (Table 4-48) after the ROLE_CONTROL has been updated. */
    (void)PD_TimerStart(pdInstance, timrCCFilter, T_CC_FILTER_MAX);

#if (defined USBPD_ENABLE_PTN5110_A0R2_WORKAROUNDS) && (USBPD_ENABLE_PTN5110_A0R2_WORKAROUNDS)
    if ((Reg_CacheRead(pdInstance, GLOBAL, vendor_id) == PD_VENDOR_ID_NXP) &&
        (Reg_CacheRead(pdInstance, GLOBAL, product_id) == PRODUCT_ID_PTN5110) &&
        (Reg_CacheRead(pdInstance, GLOBAL, device_id) < DEVICE_ID_PTN5110_A0R2))
    {
        /* If we are starting to toggle, then preset the hardware state machine to the right levels */
        if (RoleControlIsToggleSrcFirst(roleControl) || RoleControlIsToggleSnkFirst(roleControl))
        {
            Reg_BusWriteByte(pdInstance, role_control,
                             (uint8_t)(roleControl & (uint8_t) ~(uint8_t)TCPC_ROLE_CONTROL_DRP_MASK));
        }
    }
#endif

    Reg_BusWriteByte(pdInstance, role_control, roleControl);
    PDPTN5110_IocSetIlim(pdInstance, (((roleControl >> 4U) & 0x03U) == 0x02U) ? 1U : 0U);

    /* send Look4connection command when Drp/Rp/Rp or Drp/Rd/Rd */
    if ((RoleControlIsToggleSrcFirst(roleControl)) || (RoleControlIsToggleSnkFirst(roleControl)))
    {
        Reg_BusWriteByte(pdInstance, command, (uint8_t)TCPC_LOOK4CONNECTION);
    }
}

/*!
 * @brief Assert and monitor Rp in attached state.
 *
 * This function is used to assert Rp and set Rp value on only one CC pin.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param usedCC          Current communication CC pin. Please refer to the enumeration pd_cc_type_t.
 * @param srcRp           Rp value. Please refer to the enumeration typec_current_val_t
 *
 * @return None.
 */
void PDPTN5110_ConnectAssertRpAttached(pd_instance_t *pdInstance, pd_cc_type_t usedCC, uint8_t srcRp)
{
    if (usedCC == kPD_CC2)
    {
        Reg_CacheWrite(pdInstance, CONTROL, role_control,
                       (uint8_t)ROLE_CONTROL_CC1_OPEN | (uint8_t)ROLE_CONTROL_CC2_RP |
                           (uint8_t)(srcRp << TCPC_ROLE_CONTROL_RP_VALUE_LSB));
    }
    else
    {
        Reg_CacheWrite(pdInstance, CONTROL, role_control,
                       (uint8_t)ROLE_CONTROL_CC1_RP | (uint8_t)ROLE_CONTROL_CC2_OPEN |
                           (uint8_t)(srcRp << TCPC_ROLE_CONTROL_RP_VALUE_LSB));
    }
}

/*!
 * @brief Assert and monitor Rp in unattached state.
 *
 * This function is used to assert Rp and set Rp value on only one CC pin.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param srcRp           Rp value. Please refer to the enumeration typec_current_val_t
 *
 * @return None.
 */
void PDPTN5110_ConnectAssertRpUnattached(pd_instance_t *pdInstance, uint8_t srcRp)
{
    /* Both Rp pull ups */
    Reg_CacheWrite(pdInstance, CONTROL, role_control,
                   ((uint8_t)ROLE_CONTROL_CC1_RP | (uint8_t)ROLE_CONTROL_CC2_RP) |
                       (uint8_t)((uint8_t)srcRp << TCPC_ROLE_CONTROL_RP_VALUE_LSB));
}

/*!
 * @brief Assert and monitor Rp as DTS source.
 *
 * This function is used to assert Rp and set Rp value on both CCs.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return None.
 */
void PDPTN5110_ConnectAssertRpDbgAccSrc(pd_instance_t *pdInstance)
{
    /* Both Rp pull ups, one at Rp3.0 and one at Rp1.5 */
    /* (but there is no way in TCPC to set different RP strengths) */
    Reg_CacheWrite(pdInstance, CONTROL, role_control, ((uint8_t)ROLE_CONTROL_CC1_RP | (uint8_t)ROLE_CONTROL_CC2_RP));
}

/*!
 * @brief Assert and monitor Rd as DTS source.
 *
 * This function is used to assert Rd on both CCs.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return None.
 */
void PDPTN5110_ConnectAssertRdDbgAccSnk(pd_instance_t *pdInstance)
{
    /* pull down both pins, one to Rd and the other to Ra (except TCPC cannot pull down to Ra) */
    Reg_CacheWrite(pdInstance, CONTROL, role_control, ((uint8_t)ROLE_CONTROL_CC1_RD | (uint8_t)ROLE_CONTROL_CC2_RD));
}

/*!
 * @brief Assert and monitor Rd in attached state.
 *
 * This function is used to assert Rd on only one CC pin.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param usedCC          Current communication CC pin. Please refer to the enumeration pd_cc_type_t.
 *
 * @return None.
 */
void PDPTN5110_ConnectAssertRdAttached(pd_instance_t *pdInstance, pd_cc_type_t usedCC)
{
    /* Just the connected pull down */
    Reg_CacheWrite(pdInstance, CONTROL, role_control,
                   (usedCC == kPD_CC2) ? ((uint8_t)ROLE_CONTROL_CC1_OPEN | (uint8_t)ROLE_CONTROL_CC2_RD) :
                                         ((uint8_t)ROLE_CONTROL_CC1_RD | (uint8_t)ROLE_CONTROL_CC2_OPEN));
}

/*!
 * @brief Assert and monitor Rd in unattached state.
 *
 * This function is used to assert Rd on both CCs.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param usedCC          Current communication CC pin. Please refer to the enumeration pd_cc_type_t.
 *
 * @return None.
 */
void PDPTN5110_ConnectAssertRdUnattached(pd_instance_t *pdInstance)
{
    /* pull down both pins */
    Reg_CacheWrite(pdInstance, CONTROL, role_control, ((uint8_t)ROLE_CONTROL_CC1_RD | (uint8_t)ROLE_CONTROL_CC2_RD));
}

/*!
 * @brief Toggle and monitor Rp/Rd in unattached state.
 *
 * This function is used to assert Rp/Rd on both CCs.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param startSrc        Start toggling with Rp or Rd
 * @param srcRp           Rp value. Please refer to the enumeration typec_current_val_t
 *
 * @return None.
 */
void PDPTN5110_ConnectAssertDrpUnattached(pd_instance_t *pdInstance, uint8_t startSrc, uint8_t srcRp)
{
    uint8_t regVal;

    if (startSrc != 0U)
    {
        regVal = (uint8_t)TCPC_ROLE_CONTROL_DRP_MASK | (uint8_t)ROLE_CONTROL_CC2_RP | (uint8_t)ROLE_CONTROL_CC1_RP |
                 (uint8_t)((uint8_t)srcRp << TCPC_ROLE_CONTROL_RP_VALUE_LSB);
    }
    else
    {
        regVal = (uint8_t)TCPC_ROLE_CONTROL_DRP_MASK | (uint8_t)ROLE_CONTROL_CC2_RD | (uint8_t)ROLE_CONTROL_CC1_RD |
                 (uint8_t)((uint8_t)srcRp << TCPC_ROLE_CONTROL_RP_VALUE_LSB);
    }
    Reg_CacheWrite(pdInstance, CONTROL, role_control, regVal);
}

/*!
 * @brief Assert Rd on both CCs in OrientedDebugAccessory.SNK state.
 *
 * This function is used to assert Rd on both CCs.
 * When entering OrientedDebugAccessory.SNK state, this function should be called.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return None.
 */
void PDPTN5110_ConnectDbgAccSnkAttached(pd_instance_t *pdInstance)
{
    /* Note: Don't change Role Control, CC line should be kept as Rd/Rd when working at Dbg Acc sink state */
}

/*!
 * @brief Assert Rp and set different Rp value on both CCs in OrientedDebugAccessory.SRC state.
 *
 * This function is used to assert Rp and set different Rp value on both CCs.
 * When entering OrientedDebugAccessory.SRC state, this function should be called.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return None.
 */
void PDPTN5110_ConnectDbgAccSrcAttached(pd_instance_t *pdInstance)
{
    /* Note: Don't change Role Control, CC line should be kept as Rp/Rp when working at Dbg Acc source state */
}

/*!
 * @brief Assert and monitor Rp in audio accessory attached state.
 *
 * This function is used to assert Rp on only one CC pin.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param cc              Current communication CC pin. Please refer to the enumeration pd_cc_type_t.
 *
 * @return None.
 */
void PDPTN5110_ConnectAudioAccessoryAttached(pd_instance_t *pdInstance, pd_cc_type_t usedCC)
{
    uint8_t cc1Orcc2 = (usedCC == kPD_CC2) ? ((uint8_t)ROLE_CONTROL_CC2_RP | (uint8_t)ROLE_CONTROL_CC1_OPEN) :
                                             ((uint8_t)ROLE_CONTROL_CC1_RP | (uint8_t)ROLE_CONTROL_CC2_OPEN);
    Reg_CacheWrite(pdInstance, CONTROL, role_control, cc1Orcc2);
}

/*!
 * @brief Set both CCs to open drain.
 *
 * This function is used to disable port.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return None.
 */
static void PDPTN5110_ConnectSetCCOpenDrain(pd_instance_t *pdInstance)
{
    /* Clear terminations */
    Reg_CacheWrite(pdInstance, CONTROL, role_control,
                   ((uint8_t)ROLE_CONTROL_CC1_OPEN | (uint8_t)ROLE_CONTROL_CC2_OPEN));

    /* Update role control cache to register */
    PDPTN5110_ConnectSyncRoleControl(pdInstance);
}

/*!
 * @brief Set different Rp value on only one CC pin in attached state.
 *
 * This function is used to set different Rp value on only one CC pin.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param srcRp           Rp value. Please refer to the enumeration typec_current_val_t
 *
 * @return None.
 */
void PDPTN5110_ConnectSrcSetTypecCurrent(pd_instance_t *pdInstance, pd_cc_type_t usedCC, uint8_t srcRp)
{
    if (usedCC == kPD_CC2)
    {
        Reg_CacheWriteByteField(pdInstance, CONTROL, role_control,
                                ((uint8_t)TCPC_ROLE_CONTROL_RP_VALUE_MASK | (uint8_t)TCPC_ROLE_CONTROL_CC2_MASK),
                                ((srcRp << TCPC_ROLE_CONTROL_RP_VALUE_LSB) | (uint8_t)ROLE_CONTROL_CC2_RP));
    }
    else
    {
        Reg_CacheWriteByteField(pdInstance, CONTROL, role_control,
                                ((uint8_t)TCPC_ROLE_CONTROL_RP_VALUE_MASK | (uint8_t)TCPC_ROLE_CONTROL_CC1_MASK),
                                ((srcRp << TCPC_ROLE_CONTROL_RP_VALUE_LSB) | (uint8_t)ROLE_CONTROL_CC1_RP));
    }

    /* Update role control cache to register */
    /* PDPTN5110_ConnectSyncRoleControl(pdInstance); */
    Reg_BusWriteByte(pdInstance, role_control, Reg_CacheRead(pdInstance, CONTROL, role_control));
}

/*!
 * @brief Get the current CC current limit.
 *
 * This function is used to get Rp value on only one CC pin.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param usedCC          Current communication CC pin. Please refer to the enumeration pd_cc_type_t.
 *
 * @return current CC current limit. Please refer to typec_current_val_t.
 */
uint8_t PDPTN5110_ConnectGetTypeCCurrent(pd_instance_t *pdInstance, pd_cc_type_t usedCC)
{
    uint8_t typecCurrent = (uint8_t)kCurrent_Invalid;
    uint8_t ccStatus;

    if ((pdInstance->inProgress == kVbusPower_Stable) || (pdInstance->inProgress == kVbusPower_ChangeInProgress))
    {
        ccStatus = Reg_CacheRead(pdInstance, STATUS, cc_status);
        if (ccStatus == 0xFFU)
        {
            typecCurrent = (uint8_t)kCurrent_Invalid;
        }
        else if ((ccStatus & TCPC_CC_STATUS_CONNECT_RESULT_MASK) != 0U)
        {
            /* Connect Result that TCPC is presenting Rd */
            typecCurrent = (usedCC == kPD_CC2) ? (uint8_t)TcpcReadField(ccStatus, CC_STATUS, CC2_STATE) :
                                                 (uint8_t)TcpcReadField(ccStatus, CC_STATUS, CC1_STATE);
        }
        else
        {
            /* Connect Result that TCPC is presenting Rp */
            typecCurrent =
                (uint8_t)TcpcReadField(Reg_CacheRead(pdInstance, CONTROL, role_control), ROLE_CONTROL, RP_VALUE) + 1U;
        }
    }
    else
    {
        typecCurrent = (uint8_t)kCurrent_Invalid;
    }

    return typecCurrent;
}
