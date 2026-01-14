/*
 * Copyright 2015 - 2018 NXP
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

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define TX_ABORT_MASK (TCPC_ALERT_TRANSMIT_SOP_MESSAGE_FAILED_MASK | TCPC_ALERT_TRANSMIT_SOP_MESSAGE_DISCARDED_MASK)
/* The TCPC shall assert one and only one of ALERT.TransmitSuccessful, TransmitFailed, or TransmitDiscarded following a
 * write of the TRANSMIT register when a non-Hard Reset is transmitted. */
#define TX_DONE_MASK (TX_ABORT_MASK | TCPC_ALERT_TRANSMIT_SOP_MESSAGE_SUCCESSFUL_MASK)
/* The TCPC shall assert both ALERT.TransmitSuccessful and ALERT.TransmitFailed after it completes the sending of a Hard
 * Reset. */
#define TX_HARDRESET_MASK \
    (TCPC_ALERT_TRANSMIT_SOP_MESSAGE_FAILED_MASK | TCPC_ALERT_TRANSMIT_SOP_MESSAGE_SUCCESSFUL_MASK)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void PDPTN5110_PowerStatusChangeIntCallback(pd_instance_t *pdInstance);
static void PDPTN5110_FaultStatusChangeCallback(pd_instance_t *pdInstance);
static void PDPTN5110_VbusVoltageAlarmIntCallback(pd_instance_t *pdInstance, uint8_t hiOrLo);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Initialize TCPC.
 *
 * This function is used to initialize TCPC.
 *
 * @param pdHandle      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return kStatus_PD_Error or kStatus_PD_Success.
 */
pd_status_t PDPTN5110_HalInit(pd_handle pdHandle)
{
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;
    uint16_t blockdata        = 0xFFFFU;
    uint8_t count             = 0U;

    /* Wait tcpc initialization */
    while (Reg_BusReadByteField(pdInstance, power_status, TCPC_POWER_STATUS_TCPC_INITIALIZATION_STATUS_MASK) != 0U)
    {
        if (count++ > 200U)
        {
            return kStatus_PD_Error;
        }
        PD_WaitUsec(1000);
    }

    /* Disable I2C clock stretching. */
    Reg_BusWriteByte(pdInstance, tcpc_control, 0x00U);

    /* Read tcpc identification information to local buffer */
    if ((Reg_BusReadBlock(pdInstance, product_id, 2, (uint8_t *)&pdInstance->tcpcRegCache.GLOBAL.product_id) !=
         kStatus_PD_Success) ||
        (pdInstance->tcpcRegCache.GLOBAL.product_id != PRODUCT_ID_PTN5110))
    {
        return kStatus_PD_Error;
    }
    (void)Reg_BusReadBlock(pdInstance, vendor_id, 2, (uint8_t *)&pdInstance->tcpcRegCache.GLOBAL.vendor_id);
    (void)Reg_BusReadBlock(pdInstance, device_id, 2, (uint8_t *)&pdInstance->tcpcRegCache.GLOBAL.device_id);
    (void)Reg_BusReadBlock(pdInstance, usbtypec_rev, 2, (uint8_t *)&pdInstance->tcpcRegCache.GLOBAL.usbtypec_rev);
    (void)Reg_BusReadBlock(pdInstance, usbpd_rev_ver, 2, (uint8_t *)&pdInstance->tcpcRegCache.GLOBAL.usbpd_rev_ver);
    (void)Reg_BusReadBlock(pdInstance, pd_interface_rev, 2,
                           (uint8_t *)&pdInstance->tcpcRegCache.GLOBAL.pd_interface_rev);

    /*  Enable and clear all interrupt */
    if (Reg_CacheRead(pdInstance, GLOBAL, pd_interface_rev) >= PD_INTERFACE_REV2VER1)
    {
        Reg_CacheWrite(pdInstance, MASK, alert_mask, blockdata);
    }
    else
    {
        Reg_CacheWrite(pdInstance, MASK, alert_mask, 0x8FFFU);
        /* TCPCv1 threshold is not compatable with TypeC1.2
         * Update to match TCPCv2. 0x8c * 25mv = 3.5V */
        Reg_CacheWrite(pdInstance, VBUS, vbus_sink_disconnect_threshold, VBUS_SINK_DISCONNECT_THRESHOLD);
        Reg_BusWriteWord(pdInstance, vbus_sink_disconnect_threshold, VBUS_SINK_DISCONNECT_THRESHOLD);
    }
    /* Set VBUS_STOP_DISCHARGE_THRESHOLD to 0.6V(default 0.8V) to be compatible with UGREEN dongle(Type-C to
     * DisplayPort) */
    Reg_BusWriteWord(pdInstance, vbus_stop_discharge_threshold, VBUS_STOP_DISCHARGE_THRESHOLD);
    (void)Reg_BusWriteByte(pdInstance, alert_extended, 0x07U);
    (void)Reg_BusWriteBlock(pdInstance, (uint8_t *)&blockdata, 2, alert_mask);
    (void)Reg_BusWriteBlock(pdInstance, (uint8_t *)&blockdata, 2, alert);
    if (Reg_CacheRead(pdInstance, GLOBAL, vendor_id) == PD_VENDOR_ID_NXP &&
        Reg_CacheRead(pdInstance, GLOBAL, product_id) == PRODUCT_ID_PTN5110)
    {
        (void)Reg_BusWriteBlock(pdInstance, (uint8_t *)&blockdata, 2, ptn5110_ext_alert_mask);
        (void)Reg_BusWriteBlock(pdInstance, (uint8_t *)&blockdata, 2, ptn5110_ext_alert);
        /* Do not open fets on ADC Alarm */
        Reg_BusModifyByteField(pdInstance, ptn5110_ext_fault_config,
                               PTN5110_EXT_FAULT_CONFIG_OPEN_FETS_ON_ADC_ALARM_MASK, 0);
    }
    Reg_CacheWrite(pdInstance, MASK, power_status_mask, 0xFFU);
    Reg_BusWriteByte(pdInstance, power_status_mask, 0xFFU);

    /* Reset fault control to default value. */
    Reg_BusWriteByte(pdInstance, fault_control, 0x00U);

    /* Sync register */
    PDPTN5110_RegCacheSynC(pdInstance, kRegModule_All);

#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
    /* Disable fast role swap */
    PDPTN5110_DisableFRSwap(pdInstance);
#endif

#if (!defined PD_CONFIG_DEBUG_ACCESSORY_SUPPORT) || (!PD_CONFIG_DEBUG_ACCESSORY_SUPPORT)
    /* debug_accessory function controlled by TCPM */
    PDPTN5110_ConnectControlDebugAccessory(pdInstance, 1);
#endif

#if (defined USBPD_ENABLE_PTN5110_A0R1_WORKAROUNDS) && (USBPD_ENABLE_PTN5110_A0R1_WORKAROUNDS)
    /* Allow the TCPM to work-around incorrect entry to debug accessory */
    if (Reg_CacheRead(pdInstance, GLOBAL, vendor_id) == PD_VENDOR_ID_NXP &&
        Reg_CacheRead(pdInstance, GLOBAL, product_id) == PRODUCT_ID_PTN5110 &&
        Reg_CacheRead(pdInstance, GLOBAL, device_id) < DEVICE_ID_PTN5110_A0R2)
    {
        PDPTN5110_ConnectControlDebugAccessory(pdInstance, 1);
    }
    else
    {
        PDPTN5110_ConnectControlDebugAccessory(pdInstance, 0);
    }
#endif

    return kStatus_PD_Success;
}

/*!
 * @brief Read the remote registers to local buffer.
 *
 * This function is used to read the remote registers to local buffer.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param mask            Please refer to the enumeration RegModuleMaskTcpc_t.
 *
 * @return None.
 */
void PDPTN5110_RegCacheSynC(pd_instance_t *pdInstance, RegModuleMaskTcpc_t mask)
{
    if (((uint16_t)mask & (uint16_t)kRegModule_Intc) != 0U)
    {
        (void)Reg_BusReadBlock(pdInstance, alert, 2, (uint8_t *)&pdInstance->tcpcRegCache.INTERRUPT.alert);
    }

    /* update tcpc_control / roleControl / fault_control / power_control together */
    if (((uint16_t)mask & (uint16_t)kRegModule_Control) != 0U)
    {
        (void)Reg_BusReadBlock(pdInstance, tcpc_control, 1, (uint8_t *)&pdInstance->tcpcRegCache.CONTROL.tcpc_control);
        (void)Reg_BusReadBlock(pdInstance, role_control, 1, (uint8_t *)&pdInstance->tcpcRegCache.CONTROL.role_control);
        (void)Reg_BusReadBlock(pdInstance, fault_control, 1,
                               (uint8_t *)&pdInstance->tcpcRegCache.CONTROL.fault_control);
        (void)Reg_BusReadBlock(pdInstance, power_control, 1,
                               (uint8_t *)&pdInstance->tcpcRegCache.CONTROL.power_control);
    }

    if (((uint16_t)mask & (uint16_t)kRegModule_CCStatus) != 0U)
    {
        (void)Reg_BusReadBlock(pdInstance, cc_status, 1, (uint8_t *)&pdInstance->tcpcRegCache.STATUS.cc_status);
    }

    if (((uint16_t)mask & (uint16_t)kRegModule_PowerStatus) != 0U)
    {
        (void)Reg_BusReadBlock(pdInstance, power_status, 1, (uint8_t *)&pdInstance->tcpcRegCache.STATUS.power_status);
    }

    if (Reg_CacheRead(pdInstance, GLOBAL, pd_interface_rev) >= PD_INTERFACE_REV2VER1)
    {
        if (((uint16_t)mask & (uint16_t)kRegModule_ExtendedStatus) != 0U)
        {
            (void)Reg_BusReadBlock(pdInstance, extended_status, 1,
                                   (uint8_t *)&pdInstance->tcpcRegCache.STATUS.extended_status);
        }
    }
}

/*!
 * @brief Clear and enable external interrupt.
 *
 * This function is used to clear and enable external interrupt.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return None.
 */
void PDPTN5110_IntcIrqClearAndEnable(pd_instance_t *pdInstance, uint16_t mask)
{
    (void)Reg_BusWriteBlock(pdInstance, (uint8_t *)&mask, 2, alert);
    Reg_CacheRead(pdInstance, MASK, alert_mask) |= mask;
    (void)Reg_BusWriteBlock(pdInstance, (uint8_t *)&Reg_CacheRead(pdInstance, MASK, alert_mask), 2, alert_mask);
}

/*!
 * @brief Enable external interrupt.
 *
 * This function is used to enable external interrupt.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return None.
 */
void PDPTN5110_IntcIrqEnable(pd_instance_t *pdInstance, uint16_t mask)
{
    Reg_CacheRead(pdInstance, MASK, alert_mask) |= mask;
    (void)Reg_BusWriteBlock(pdInstance, (uint8_t *)&Reg_CacheRead(pdInstance, MASK, alert_mask), 2, alert_mask);
}

/*!
 * @brief Disable external interrupt.
 *
 * This function is used to disable external interrupt.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return None.
 */
void PDPTN5110_IntcIrqDisable(pd_instance_t *pdInstance, uint16_t mask)
{
    Reg_CacheRead(pdInstance, MASK, alert_mask) &= (uint16_t) ~(uint16_t)mask;
    (void)Reg_BusWriteBlock(pdInstance, (uint8_t *)&Reg_CacheRead(pdInstance, MASK, alert_mask), 2, alert_mask);
}

/*!
 * @brief Get if Vsys present.
 *
 * This function is used to get if Vsys present.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return 0 - Vsys not present, 1 - Vsys present.
 */
uint8_t PDPTN5110_VsysIsPresent(pd_instance_t *pdInstance)
{
    /* Always true for TCPM since no way to get this info from TCPC, we will assume Vsys present if MCU is running */
    return 1;
}

/*!
 * @brief Read Vbus voltage register.
 *
 * This function is used to read Vbus voltage.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return Vbus voltage. The LSB is 25mv.
 */
uint16_t PDPTN5110_GetVbusVoltage(pd_instance_t *pdInstance)
{
    uint16_t voltage = 0;

    (void)Reg_BusReadBlock(pdInstance, vbus_voltage, 2, (uint8_t *)&voltage);
    voltage = (uint16_t)(TcpcReadField(voltage, VBUS_VOLTAGE, VBUS_VOLTAGE_MEASUREMENT) *
                         ((uint16_t)1U << TcpcReadField(voltage, VBUS_VOLTAGE, VBUS_SCALE_FACTOR)));

    return voltage;
}

/*!
 * @brief Control Vbus voltage alarm.
 *
 * This function is used to enable or disable Vbus voltage alarm.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param enable          Enable command. 0 - disable, 1 - enable.
 *
 * @return None.
 */
void PDPTN5110_ControlVbusVoltageAlarm(pd_instance_t *pdInstance, uint8_t enable)
{
    uint8_t voltageAlarm = (enable != 0U) ? 0U : TCPC_POWER_CONTROL_DISABLE_VOLTAGE_ALARMS_MASK;

    if (Reg_CacheReadByteField(pdInstance, CONTROL, power_control, TCPC_POWER_CONTROL_DISABLE_VOLTAGE_ALARMS_MASK) !=
        voltageAlarm)
    {
        Reg_CacheAndBusModifyByteField(pdInstance, CONTROL, power_control,
                                       TCPC_POWER_CONTROL_DISABLE_VOLTAGE_ALARMS_MASK, voltageAlarm);
    }
}

/*!
 * @brief Set Vbus voltage alarm high and low value and enable Vbus alarm.
 *
 * This function is used to set Vbus voltage alarm high and low value and enable Vbus alarm.
 * This function is required if source voltage is greater than vSafe5V.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param vbusVoltage     High and Low alarm voltage. Low 16-bit of this value is the low alarm voltage value.
 *                        High 16-bit of this value is the high alarm voltage value.
 *
 * @return None.
 */
void PDPTN5110_SetVbusVoltageAlarmHiLo(pd_instance_t *pdInstance, uint32_t vbusVoltage)
{
    uint32_t paramVal;

    /* Disable Vbus voltage alarms */
    /* PDPTN5110_ControlVbusVoltageAlarm(pdInstance, 0); */

    if (Reg_CacheRead(pdInstance, GLOBAL, pd_interface_rev) >= PD_INTERFACE_REV2VER1)
    {
        paramVal = (vbusVoltage & (uint32_t)0x0000FFFFU);
#if defined(PD_CONFIG_VBUS_ALARM_SUPPORT) && (PD_CONFIG_VBUS_ALARM_SUPPORT)
        paramVal /= 25U; /* 25mv unit */
#else
        paramVal = (paramVal - paramVal * 15U / 100U) / 25U; /* 25mv unit */
#endif
        Reg_BusWriteWord(pdInstance, vbus_voltage_alarm_lo_cfg,
                         (uint16_t)((uint16_t)TCPC_VBUS_VOLTAGE_ALARM_LO_CFG_VBUS_HI_VOLTAGE_TRIP_POINT_MASK &
                                    (paramVal << TCPC_VBUS_VOLTAGE_ALARM_LO_CFG_VBUS_HI_VOLTAGE_TRIP_POINT_LSB)));

        paramVal = (vbusVoltage & (uint32_t)0xFFFF0000U) >> 16U;
#if defined(PD_CONFIG_VBUS_ALARM_SUPPORT) && (PD_CONFIG_VBUS_ALARM_SUPPORT)
        paramVal /= 25U; /* 25mv unit */
#else
        paramVal = (paramVal + paramVal * 15U / 100U) / 25U; /* 25mv unit */
#endif
        Reg_BusWriteWord(pdInstance, vbus_voltage_alarm_hi_cfg,
                         (uint16_t)((uint16_t)TCPC_VBUS_VOLTAGE_ALARM_HI_CFG_VBUS_LO_VOLTAGE_TRIP_POINT_MASK &
                                    (paramVal << TCPC_VBUS_VOLTAGE_ALARM_HI_CFG_VBUS_LO_VOLTAGE_TRIP_POINT_LSB)));
    }
    /* Enable Vbus voltage alarms */
    PDPTN5110_ControlVbusVoltageAlarm(pdInstance, 1);
}

/*!
 * @brief Read Vbus voltage register.
 *
 * This function is used to read Vbus voltage.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param enable          1 - enable Vbus monitor and detect.
 *                        0 - disable Vbus monitor and detect.
 *
 * @return None.
 */
void PDPTN5110_ControlVbusMonitorAndDetect(pd_instance_t *pdInstance, uint8_t enable)
{
    if (enable != 0U)
    {
        /* Turn on VBUS Voltage monitor.  0 means enabled. */
        Reg_CacheAndBusModifyByteField(pdInstance, CONTROL, power_control, TCPC_POWER_CONTROL_VBUS_VOLTAGE_MONITOR_MASK,
                                       0);
        Reg_BusWriteByte(pdInstance, command, (uint8_t)TCPC_ENABLEVBUSDETECT);
    }
    else
    {
        /* Turn off VBUS Voltage monitor.  1 means disabled. */
        Reg_CacheAndBusModifyByteField(pdInstance, CONTROL, power_control, TCPC_POWER_CONTROL_VBUS_VOLTAGE_MONITOR_MASK,
                                       1);
        Reg_BusWriteByte(pdInstance, command, (uint8_t)TCPC_DISABLEVBUSDETECT);
    }
}

/*!
 * @brief Control Vbus FET to Source or Sink.
 *
 * This function is used to control Vbus FET to Source or Sink.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param set             Please refer to the structure pd_ptn5110_ctrl_pin_t.
 *                        set->enSRC:  1 - enable Vbus source.
 *                                     0 - disable Vbus source.
 *                        set->enSNK1: 1 - enable Vbus sink.
 *                                     0 - disable Vbus sink.
 *
 * @return None.
 */
void PDPTN5110_ControlVbusSourceOrSink(pd_instance_t *pdInstance, pd_ptn5110_ctrl_pin_t *set)
{
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    if (set->enSRC != 0U)
    {
        /* Enable auto discharge when sourcing or sinking Vbus(attached) */
        PDPTN5110_ConnectControlAutoDischarge(pdInstance, 1);
        if ((Reg_CacheRead(pdInstance, STATUS, power_status) & TCPC_POWER_STATUS_SINKING_VBUS_MASK) != 0U)
        {
            Reg_BusWriteByte(pdInstance, command, (uint8_t)TCPC_DISABLESINKVBUS);
        }
        Reg_BusWriteByte(pdInstance, command, (uint8_t)TCPC_SOURCEVBUSDEFAULTVOLTAGE);
    }
    else if (set->enSNK1 != 0U)
    {
        /* Enable auto discharge when sourcing or sinking Vbus(attached) */
        PDPTN5110_ConnectControlAutoDischarge(pdInstance, 1);
        if ((Reg_CacheRead(pdInstance, STATUS, power_status) & TCPC_POWER_STATUS_SOURCING_VBUS_MASK) != 0U)
        {
            Reg_BusWriteByte(pdInstance, command, (uint8_t)TCPC_DISABLESOURCEVBUS);
        }
        Reg_BusWriteByte(pdInstance, command, (uint8_t)TCPC_SINKVBUS);
    }
    else
    {
        Reg_BusWriteByte(pdInstance, command, (uint8_t)TCPC_DISABLESOURCEVBUS);
        Reg_BusWriteByte(pdInstance, command, (uint8_t)TCPC_DISABLESINKVBUS);
    }

    OSA_EXIT_CRITICAL();
}

#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
/*!
 * @brief Signal FR Swap.
 *
 * This function is used to signal FR Swap.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return None.
 */
void PDPTN5110_SignalFRSwap(pd_instance_t *pdInstance)
{
    /* Do the following actions manually and atomically, for the shortest latency */
    if (Reg_CacheRead(pdInstance, GLOBAL, pd_interface_rev) >= PD_INTERFACE_REV2VER1)
    {
        Reg_BusWriteByte(pdInstance, command, (uint8_t)TCPC_SENDFRSWAPSIGNAL); /* generate the FRS signal */
    }
    else
    {
        Reg_BusWriteByte(pdInstance, ptn5110_ext_command, (uint8_t)TCPC_SEND_FRS); /* generate the FRS signal */
    }
}

/*!
 * @brief Enable FR Swap.
 *
 * This function is used to enable FR Swap.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return None.
 */
void PDPTN5110_EnableFRSwap(pd_instance_t *pdInstance)
{
    if (Reg_CacheRead(pdInstance, GLOBAL, pd_interface_rev) >= PD_INTERFACE_REV2VER1)
    {
        if (pdInstance->curPowerRole == kPD_PowerRoleSource)
        {
            Reg_CacheAndBusModifyByteField(pdInstance, MASK, alert_extended_mask,
                                           TCPC_ALERT_EXTENDED_MASK_SOURCE_FAST_ROLE_SWAP_MASK_MASK,
                                           TCPC_ALERT_EXTENDED_MASK_SOURCE_FAST_ROLE_SWAP_MASK_MASK);
        }
        else
        {
            Reg_CacheAndBusModifyByteField(pdInstance, MASK, alert_extended_mask,
                                           TCPC_ALERT_EXTENDED_MASK_SINK_FAST_ROLE_SWAP_MASK_MASK,
                                           TCPC_ALERT_EXTENDED_MASK_SINK_FAST_ROLE_SWAP_MASK_MASK);
            Reg_CacheWrite(pdInstance, VBUS, vbus_sink_disconnect_threshold, 0);
            Reg_BusWriteWord(pdInstance, vbus_sink_disconnect_threshold, 0);
        }
        Reg_CacheAndBusModifyByteField(pdInstance, CONTROL, power_control,
                                       TCPC_POWER_CONTROL_FAST_ROLE_SWAP_ENABLE_MASK,
                                       TCPC_POWER_CONTROL_FAST_ROLE_SWAP_ENABLE_MASK);
    }
    else
    {
        /* Don't enable FRS for MEAD A1R3 when PORT is source, otherwise Source Port will wrongly receive FRS signal
           when TCPM sends FRS command */
        if ((pdInstance->curPowerRole == kPD_PowerRoleSink) &&
            (Reg_CacheRead(pdInstance, GLOBAL, vendor_id) == PD_VENDOR_ID_NXP) &&
            (Reg_CacheRead(pdInstance, GLOBAL, product_id) == PRODUCT_ID_PTN5110))
        {
            Reg_BusModifyWordField(pdInstance, ptn5110_ext_config, PTN5110_EXT_CONFIG_FAST_ROLE_SWAP_ENABLE_MASK,
                                   PTN5110_EXT_CONFIG_FAST_ROLE_SWAP_ENABLE_MASK);
            Reg_BusModifyWordField(pdInstance, ptn5110_ext_alert_mask,
                                   PTN5110_EXT_ALERT_MASK_FAST_ROLE_SWAP_VSAFE5V_BEING_SOURCED_MASK |
                                       PTN5110_EXT_ALERT_MASK_FAST_ROLE_SWAP_REQUEST_RECEIVED_MASK,
                                   PTN5110_EXT_ALERT_MASK_FAST_ROLE_SWAP_VSAFE5V_BEING_SOURCED_MASK |
                                       PTN5110_EXT_ALERT_MASK_FAST_ROLE_SWAP_REQUEST_RECEIVED_MASK);
        }
    }
}
#endif

/*!
 * @brief Disable FR Swap.
 *
 * This function is used to disable FR Swap.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return None.
 */
void PDPTN5110_DisableFRSwap(pd_instance_t *pdInstance)
{
    if (Reg_CacheRead(pdInstance, GLOBAL, pd_interface_rev) >= PD_INTERFACE_REV2VER1)
    {
        Reg_CacheAndBusModifyByteField(pdInstance, CONTROL, power_control,
                                       TCPC_POWER_CONTROL_FAST_ROLE_SWAP_ENABLE_MASK, 0);
        Reg_CacheAndBusModifyByteField(pdInstance, MASK, alert_extended_mask,
                                       TCPC_ALERT_EXTENDED_MASK_SOURCE_FAST_ROLE_SWAP_MASK_MASK |
                                           TCPC_ALERT_EXTENDED_MASK_SINK_FAST_ROLE_SWAP_MASK_MASK,
                                       0);
    }
    else
    {
        if (Reg_CacheRead(pdInstance, GLOBAL, vendor_id) == PD_VENDOR_ID_NXP &&
            Reg_CacheRead(pdInstance, GLOBAL, product_id) == PRODUCT_ID_PTN5110)
        {
            Reg_BusModifyWordField(pdInstance, ptn5110_ext_config, PTN5110_EXT_CONFIG_FAST_ROLE_SWAP_ENABLE_MASK, 0);
            Reg_BusModifyWordField(pdInstance, ptn5110_ext_alert_mask,
                                   PTN5110_EXT_ALERT_MASK_FAST_ROLE_SWAP_VSAFE5V_BEING_SOURCED_MASK |
                                       PTN5110_EXT_ALERT_MASK_FAST_ROLE_SWAP_REQUEST_RECEIVED_MASK,
                                   0);
        }
    }
}

/*!
 * @brief Callback for power state change.
 *
 * This function is used to inform upper layer of Vbus state change.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return None.
 */
static void PDPTN5110_PowerStatusChangeIntCallback(pd_instance_t *pdInstance)
{
    /* When using the autonomous state machine, power_control may change, so sync it here */
    PDPTN5110_RegCacheSynC(pdInstance, kRegModule_PowerStatus);

    /* Inform vbus_present status is changing */
    PD_Notify(pdInstance, PD_PHY_EVENT_VBUS_STATE_CHANGE, NULL);
}

/*!
 * @brief Callback for fault status assertion.
 *
 * This function is used to handle fault status change.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return None.
 */
static void PDPTN5110_FaultStatusChangeCallback(pd_instance_t *pdInstance)
{
    uint8_t faultStatus = Reg_BusReadByte(pdInstance, fault_status);

    /* Fault status register is set by TCPC and cleared by TCPM */
    Reg_BusWriteByte(pdInstance, fault_status, 0xFFU);

    /* TODO: fault process */
    if ((faultStatus & TCPC_FAULT_STATUS_FORCE_DISCHARGE_FAILED_MASK) != 0U)
    {
        PDPTN5110_ConnectRawVbusDischarge(pdInstance, 0U);
    }
    if ((faultStatus & (TCPC_FAULT_STATUS_INTERNAL_OR_EXTERNAL_VBUS_OVER_CURRENT_PROTECTION_FAULT_MASK |
                        TCPC_FAULT_STATUS_INTERNAL_OR_EXTERNAL_VBUS_OVER_VOLTAGE_PROTECTION_FAULT_MASK)) != 0U)
    {
        /* when dead battery boot up,
         * TCPC_FAULT_STATUS_INTERNAL_OR_EXTERNAL_VBUS_OVER_VOLTAGE_PROTECTION_FAULT_MASK is set
         * and TCPC_FAULT_STATUS_MASK_ALL_REGISTERS_RESET_TO_DEFAULT_INTERRUPT_STATUS_MASK_MASK is set */
        if ((faultStatus & TCPC_FAULT_STATUS_MASK_ALL_REGISTERS_RESET_TO_DEFAULT_INTERRUPT_STATUS_MASK_MASK) == 0U)
        {
            /* remove TCPC_FAULT_STATUS_I2_C_INTERFACE_ERROR_MASK, because dead battery boot up with this I2C
             * error
             */
            PDPTN5110_ConnectDetachOrDisable(pdInstance);
            PD_Notify(pdInstance, PD_PHY_EVENT_TYPEC_OVP_OCP_FAULT, &faultStatus);
        }
    }
}

/*!
 * @brief Callback for Vbus Voltage Alarm Hi or Lo alert.
 *
 * This function is used to inform upper layer of Vbus state change.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param hiOrLo          Hi or Lo alarm asserted, 0 - Lo asserted, 1 - Hi asserted.
 *
 * @return None.
 */
static void PDPTN5110_VbusVoltageAlarmIntCallback(pd_instance_t *pdInstance, uint8_t hiOrLo)
{
#if !(defined(PD_CONFIG_VBUS_ALARM_SUPPORT) && (PD_CONFIG_VBUS_ALARM_SUPPORT))
    /* Disable Vbus voltage alarms in case TCPC keeps asserting VBUS alarm. */
    PDPTN5110_ControlVbusVoltageAlarm(pdInstance, 0);
#endif

    PD_Notify(pdInstance, PD_PHY_EVENT_VBUS_ALARM, &hiOrLo);
}

/*!
 * @brief Callback for assertion of Alert# pin.
 *
 * This function is used to handle TCPC state change.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return None.
 */
void PDPTN5110_IntcProcessIntAll(pd_instance_t *pdInstance)
{
    uint16_t ClrValue;
    uint16_t RegValue;

    PDPTN5110_RegCacheSynC(pdInstance, kRegModule_Intc);
    ClrValue = Reg_CacheRead(pdInstance, INTERRUPT, alert);
    RegValue = ClrValue & Reg_CacheRead(pdInstance, MASK, alert_mask); /* Clear only SEEN status */

    if (RegValue != 0U)
    {
        /* NOTE: We MUST process msg_rcvd_and_available first */
        /* the TxDone callback will use the status to evaluate a TX/RX collision */
        /* We will always receive the msg no matter RX_BUFFER_OVERFLOW or not */
        /* 1.ALERT_RECEIVE_SOP_MESSAGE_STATUS only, which means a normal msg is waiting to be received */
        /* 2.ALERT_RECEIVE_SOP_MESSAGE_STATUS + ALERT_RX_BUFFER_OVERFLOW, which means that B msg is receiving by TCPC */
        /* when A msg has been received by TCPC. */
        /* In this case, B msg will be discarded and alert ALERT_RX_BUFFER_OVERFLOW, A msg is still waiting to be */
        /* received. B won't disrupt A. */
        if ((RegValue & TCPC_ALERT_RECEIVE_SOP_MESSAGE_STATUS_MASK) != 0U)
        {
            if (Reg_CacheReadByteField(pdInstance, CONTROL, tcpc_control, TCPC_TCPC_CONTROL_BIST_TEST_MODE_MASK) == 0U)
            {
                PDPTN5110_MsgHalReceiveCompleteCallback(pdInstance, kStatus_PD_Success);
            }
        }

        if ((RegValue & TX_HARDRESET_MASK) == TX_HARDRESET_MASK)
        {
            /* alert_mask register shall be initialized per Table 4-1 upon power on or Hard Reset. so reinstate it
             * here*/
            (void)Reg_BusWriteBlock(pdInstance, (uint8_t *)&Reg_CacheRead(pdInstance, MASK, alert_mask), 2, alert_mask);
        }

        if ((RegValue & TCPC_ALERT_RECEIVED_HARD_RESET_MASK) != 0U)
        {
            /* alert_mask register shall be initialized per Table 4-1 upon power on or Hard Reset. so reinstate it
             * here*/
            (void)Reg_BusWriteBlock(pdInstance, (uint8_t *)&Reg_CacheRead(pdInstance, MASK, alert_mask), 2, alert_mask);
            PD_Notify(pdInstance, PD_PHY_EVENT_HARD_RESET_RECEIVED, NULL);
        }

        if ((RegValue & TX_ABORT_MASK) != 0U)
        {
            PDPTN5110_MsgHalSendCompleteCallback(pdInstance, kStatus_PD_Abort);
        }
        else if ((RegValue & TCPC_ALERT_TRANSMIT_SOP_MESSAGE_SUCCESSFUL_MASK) != 0U)
        {
            PDPTN5110_MsgHalSendCompleteCallback(pdInstance, kStatus_PD_Success);
        }
        else
        {
            /* No action required. */
        }

#if defined(PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
        /* Check VBUS sink disconnect */
        if ((RegValue & TCPC_ALERT_VBUS_SINK_DISCONNECT_DETECTED_MASK) != 0U)
        {
            PD_Notify(pdInstance, PD_PHY_EVENT_VBUS_SINK_DISCONNECT, NULL);
        }
#endif

        /* Check VBUS Voltage Alarm Lo alert */
        if ((RegValue & TCPC_ALERT_VBUS_VOLTAGE_ALARM_LO_MASK) != 0U)
        {
            PDPTN5110_VbusVoltageAlarmIntCallback(pdInstance, 0);
        }

        /* Check VBUS Voltage Alarm Hi alert */
        if ((RegValue & TCPC_ALERT_VBUS_VOLTAGE_ALARM_HI_MASK) != 0U)
        {
            PDPTN5110_VbusVoltageAlarmIntCallback(pdInstance, 1);
        }

        /* Fault status */
        if ((RegValue & TCPC_ALERT_FAULT_MASK) != 0U)
        {
            PDPTN5110_FaultStatusChangeCallback(pdInstance);
        }

        /* Extended status */
        if ((RegValue & TCPC_ALERT_EXTENDED_STATUS_MASK) != 0U)
        {
            PDPTN5110_RegCacheSynC(pdInstance, kRegModule_ExtendedStatus);
        }

        /* Power status */
        if ((RegValue & TCPC_ALERT_PORT_POWER_STATUS_MASK) != 0U)
        {
            PDPTN5110_PowerStatusChangeIntCallback(pdInstance);
        }

        /* ALERT_RX_BUFFER_OVERFLOW only, which means that a message longer than TCPC extend buffer has been received */
        /* and discarded */
        /* In this case, no Normal SOP is received, no ALERT_RECEIVE_SOP_MESSAGE_STATUS, we just need to clear this */
        /* alert */
        /* But we still need to use both ALERT_RECEIVE_SOP_MESSAGE_STATUS and ALERT_RX_BUFFER_OVERFLOW to clear */
        /* ALERT_RX_BUFFER_OVERFLOW, as per TCPC spec. */
        if ((RegValue & TCPC_ALERT_RX_BUFFER_OVERFLOW_MASK) != 0U)
        {
            /* Must write back the receive bit to clear the overflow flag */
            ClrValue |= TCPC_ALERT_RECEIVE_SOP_MESSAGE_STATUS_MASK;
        }

        /********************************* TODO ***************************************/
        /*********************** handle other interrupts ******************************/

        if ((Reg_CacheRead(pdInstance, GLOBAL, vendor_id) == PD_VENDOR_ID_NXP) &&
            (Reg_CacheRead(pdInstance, GLOBAL, product_id) == PRODUCT_ID_PTN5110))
        {
            uint16_t ExtClrValue = 0;

            if (Reg_CacheRead(pdInstance, GLOBAL, pd_interface_rev) >= PD_INTERFACE_REV2VER1)
            {
                if ((RegValue & TCPC_ALERT_ALERT_EXTENDED_MASK) != 0U)
                {
                    ExtClrValue = Reg_BusReadByte(pdInstance, alert_extended);
                }
            }
            else
            {
                /* Handle A0-01 and A0-02 silicon differently */
                if ((Reg_CacheRead(pdInstance, GLOBAL, device_id) == DEVICE_ID_PTN5110_A0R1) ||
                    ((RegValue & TCPC_ALERT_VENDOR_DEFINED_EXTENDED_MASK) != 0U))
                {
                    ExtClrValue = Reg_BusReadByte(pdInstance, ptn5110_ext_alert);
                }
            }

            if (ExtClrValue != 0U)
            {
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
                uint16_t frsSigReceivedMask;

                if (Reg_CacheRead(pdInstance, GLOBAL, pd_interface_rev) >= PD_INTERFACE_REV2VER1)
                {
                    frsSigReceivedMask = TCPC_ALERT_EXTENDED_SINK_FAST_ROLE_SWAP_MASK;
                }
                else
                {
                    frsSigReceivedMask = PTN5110_EXT_ALERT_FAST_ROLE_SWAP_REQUEST_RECEIVED_MASK;
                }

                /* Check FR Swap signal recieved */
                if ((ExtClrValue & frsSigReceivedMask) != 0U)
                {
                    PD_Notify(pdInstance, PD_PHY_EVENT_FR_SWAP_SINGAL_RECEIVED, NULL);
                }

                /* Check VSafe5V has been sourced */
                if ((ExtClrValue & PTN5110_EXT_ALERT_FAST_ROLE_SWAP_VSAFE5V_BEING_SOURCED_MASK) != 0U)
                {
                    pdInstance->fr5VOpened = 1;
                }
#endif

                /* Clear all valid extend interrupts after all interrupts have been processed */
                if (Reg_CacheRead(pdInstance, GLOBAL, pd_interface_rev) >= PD_INTERFACE_REV2VER1)
                {
                    Reg_BusWriteByte(pdInstance, alert_extended, (uint8_t)(ExtClrValue & 0x00FFU));
                }
                else
                {
                    (void)Reg_BusWriteBlock(pdInstance, (uint8_t *)&ExtClrValue, 2, ptn5110_ext_alert);
                }
            }
        }

        /********************************* TODO ***************************************/
        /*********************** handle other extended interrupts ******************************/
    }
    else
    {
        /* Report an error if only a masked interrupt is seen */
    }

    if (ClrValue != 0U)
    {
        /* Reset the flooding interrupt counter when we see any valid interrupt */
        /* Clear all valid interrupts after all interrupts have been processed */
        (void)Reg_BusWriteBlock(pdInstance, (uint8_t *)&ClrValue, 2, alert);

        /* Read CC_STATUS after clearing the alert, as it could be updated */
        if ((RegValue & TCPC_ALERT_CC_STATUS_ALERT_MASK) != 0U)
        {
            PDPTN5110_RegCacheSynC(pdInstance, kRegModule_CCStatus);
        }
    }
}
