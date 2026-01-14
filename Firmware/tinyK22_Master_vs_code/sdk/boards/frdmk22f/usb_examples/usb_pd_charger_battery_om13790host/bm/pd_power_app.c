/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_i2c.h"
#include "pd_app.h"
#include "pd_power_interface.h"
#include "pd_board_config.h"
#include "pd_power_nx20p3483.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define USBPD_SINK_OVP_VALUE_SHILED2            23000U
#define SHIELD_BOARD_VBUS_DECREASE_FROM_9V_TIME (100U)
#define SHIELD_BOARD_VBUS_INCREASE_TO_9V_TIME   (30U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern uint32_t PD_DemoSoftTimer_msGet(void);
extern uint32_t PD_DemoSoftTimer_getInterval(uint32_t startTime);

typedef struct _pd_power_control_instance
{
    uint32_t sourceVbusVoltage;
    void *pdHandle;
    hal_gpio_handle_t powerGpioHandle;
} pd_power_control_instance_t;

static pd_power_control_instance_t s_PowerControlInstances[PD_CONFIG_MAX_PORT];

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void PD_PowerBoardControlInit(uint8_t port, pd_handle pdHandle, hal_gpio_handle_t powerGpioHandle)
{
    uint32_t getInfo = 0U;
    pd_power_control_instance_t *powerControl = NULL;
    if ((port == 0U) || (port > PD_CONFIG_MAX_PORT))
    {
        return;
    }
    powerControl = &s_PowerControlInstances[port - 1U];	
    powerControl->sourceVbusVoltage           = 0U;
    powerControl->pdHandle                    = pdHandle;
    powerControl->powerGpioHandle             = powerGpioHandle;
    PD_Control(pdHandle, PD_CONTROL_GET_TYPEC_CURRENT_VALUE, &getInfo);
    switch (getInfo)
    {
        case kCurrent_StdUSB:
            getInfo = 1000U;
            break;
        case kCurrent_1A5:
            getInfo = 1800U;
            break;
        case kCurrent_3A:
            getInfo = 3400U;
            break;
        default:
            getInfo = 1000U;
            break;
    }
    PD_NX20PInit(port, getInfo, USBPD_SINK_OVP_VALUE_SHILED2);
}

void PD_PowerBoardControlDeinit(uint8_t port)
{
    pd_power_control_instance_t *powerControl = NULL;

    if ((port == 0U) || (port > PD_CONFIG_MAX_PORT))
    {
        return;
    }
    powerControl = &s_PowerControlInstances[port - 1U];	
    powerControl->sourceVbusVoltage = 0U;
    powerControl->pdHandle          = NULL;
}

pd_status_t PD_PowerBoardReset(uint8_t port)
{
    pd_ptn5110_ctrl_pin_t phyPowerPinCtrl;
    pd_power_control_instance_t *powerControl = NULL;

    if ((port == 0U) || (port > PD_CONFIG_MAX_PORT))
    {
        return kStatus_PD_Error;
    }
    powerControl = &s_PowerControlInstances[port - 1U];	
    powerControl->sourceVbusVoltage = 0u;
    phyPowerPinCtrl.enSRC           = 0U;
    phyPowerPinCtrl.enSNK1          = 0U;
#if (defined BOARD_PD_EXTERNAL_POWER_SUPPORT) && (BOARD_PD_EXTERNAL_POWER_SUPPORT)
    /* Disable extra SRC */
    HAL_GpioSetOutput(powerControl->powerGpioHandle, 1U);
#endif
    PD_Control(powerControl->pdHandle, PD_CONTROL_PHY_POWER_PIN, &phyPowerPinCtrl);

    return kStatus_PD_Success;
}

/***************source need implement follow vbus power related functions***************/

/* voltage: the unit is 50mV */
pd_status_t PD_PowerBoardSourceEnableVbusPower(uint8_t port, pd_vbus_power_t vbusPower)
{
    pd_ptn5110_ctrl_pin_t phyPowerPinCtrl;
    uint32_t voltage;
    pd_power_control_instance_t *powerControl = NULL;

    if ((port == 0U) || (port > PD_CONFIG_MAX_PORT))
    {
        return kStatus_PD_Error;
    }
    powerControl = &s_PowerControlInstances[port - 1U];
    phyPowerPinCtrl.enSNK1 = 0U;
#if (defined BOARD_PD_EXTERNAL_POWER_SUPPORT) && (BOARD_PD_EXTERNAL_POWER_SUPPORT)
    if (vbusPower.minVoltage > VSAFE5V_IN_50MV)
    {
        uint32_t timeCount;

        HAL_GpioSetOutput(powerControl->powerGpioHandle, 0U); /* Enable extra SRC */
        /* when return, the power is ready. */
        timeCount = PD_DemoSoftTimer_msGet();
        while (PD_DemoSoftTimer_getInterval(timeCount) < SHIELD_BOARD_VBUS_INCREASE_TO_9V_TIME)
        {
            __NOP();
        }
        phyPowerPinCtrl.enSRC = 0U;
        PD_Control(powerControl->pdHandle, PD_CONTROL_PHY_POWER_PIN, &phyPowerPinCtrl);
    }
    else
    {
#endif
        phyPowerPinCtrl.enSRC = 1U;
        PD_Control(powerControl->pdHandle, PD_CONTROL_PHY_POWER_PIN, &phyPowerPinCtrl);

#if (defined BOARD_PD_EXTERNAL_POWER_SUPPORT) && (BOARD_PD_EXTERNAL_POWER_SUPPORT)
        HAL_GpioSetOutput(powerControl->powerGpioHandle, 1U); /* Disable extra SRC */
        if (powerControl->sourceVbusVoltage > VSAFE5V_IN_50MV)
        {
            uint32_t timeCount = PD_DemoSoftTimer_msGet();
            /* transition start (t0) to when the Source is ready
             * to provide the newly negotiated power level. */
            while (PD_DemoSoftTimer_getInterval(timeCount) < SHIELD_BOARD_VBUS_DECREASE_FROM_9V_TIME)
            {
                __NOP();
            }
        }
    }
#endif

    voltage = (vbusPower.minVoltage * 50U) | (((uint32_t)(vbusPower.maxVoltage * 50U)) << 16U);
    PD_Control(powerControl->pdHandle, PD_CONTROL_INFORM_VBUS_VOLTAGE_RANGE, &voltage);
    powerControl->sourceVbusVoltage = vbusPower.minVoltage;
    if ((voltage & 0x0000FFFFu) == 5000U)
    {
        uint32_t getInfo = 0U;
        PD_Control(powerControl->pdHandle, PD_CONTROL_GET_TYPEC_CURRENT_VALUE, &getInfo);
        switch (getInfo)
        {
            case kCurrent_StdUSB:
                getInfo = 1000U;
                break;
            case kCurrent_1A5:
                getInfo = 1800U;
                break;
            case kCurrent_3A:
                getInfo = 3400U;
                break;
            default:
                getInfo = 1000U;
                break;
        }
        PD_NX20PSet5VSourceOCP(port, getInfo);
    }

    return kStatus_PD_Success;
}

/***************sink need implement follow vbus power related functions***************/

pd_status_t PD_PowerBoardSinkEnableVbusPower(uint8_t port, pd_vbus_power_t vbusPower)
{
    pd_ptn5110_ctrl_pin_t phyPowerPinCtrl;
    uint32_t voltage;
    pd_power_control_instance_t *powerControl = NULL;

    if ((port == 0U) || (port > PD_CONFIG_MAX_PORT))
    {
        return kStatus_PD_Error;
    }
    powerControl = &s_PowerControlInstances[port - 1U];
    phyPowerPinCtrl.enSRC  = 0U;
    phyPowerPinCtrl.enSNK1 = 1U;
    PD_Control(powerControl->pdHandle, PD_CONTROL_PHY_POWER_PIN, &phyPowerPinCtrl);
    voltage = (vbusPower.minVoltage * 50U) | (((uint32_t)(vbusPower.maxVoltage * 50U)) << 16U);
    PD_Control(powerControl->pdHandle, PD_CONTROL_INFORM_VBUS_VOLTAGE_RANGE, &voltage);
    return kStatus_PD_Success;
}

/***************if support vconn, need implement the follow related functions***************/
pd_status_t PD_PowerBoardControlVconn(uint8_t port, uint8_t on)
{
    uint8_t controlVal;
    pd_power_control_instance_t *powerControl = NULL;

    if ((port == 0U) || (port > PD_CONFIG_MAX_PORT))
    {
        return kStatus_PD_Error;
    }
    powerControl = &s_PowerControlInstances[port - 1U];	
    controlVal = ((0U != on) ? 1U : 0U);
    PD_Control(powerControl->pdHandle, PD_CONTROL_VCONN, &controlVal);
    return kStatus_PD_Success;
}
