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

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/* application related functions.
 * need implement these two functions to satisfy this file's function.
 */
static uint32_t *PD_PowerBoardGetPartnerSourceCaps(void *callbackParam)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    return &(pdAppInstance->partnerSourceCaps[0].PDOValue);
}

static uint32_t *PD_PowerBoardGetSelfSourceCaps(void *callbackParam)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    return (uint32_t *)&(((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCaps[0]);
}

void PD_PowerGetVbusVoltage(uint32_t *partnerSourceCaps, pd_rdo_t rdo, pd_vbus_power_t *vbusPower)
{
    pd_source_pdo_t pdo;

    if (partnerSourceCaps == NULL)
    {
        return;
    }

    vbusPower->requestValue = rdo.bitFields.operateValue;
    if (rdo.bitFields.objectPosition < 1U)
    {
        return;
    }
    pdo.PDOValue            = partnerSourceCaps[rdo.bitFields.objectPosition - 1U];
    switch (pdo.commonPDO.pdoType)
    {
        case kPDO_Fixed:
            vbusPower->minVoltage = pdo.fixedPDO.voltage;
            vbusPower->maxVoltage = pdo.fixedPDO.voltage;
            vbusPower->valueType  = kRequestPower_Current; /* current */
            break;

        case kPDO_Battery:
            vbusPower->minVoltage = pdo.batteryPDO.minVoltage;
            vbusPower->maxVoltage = pdo.batteryPDO.maxVoltage;
            vbusPower->valueType  = kRequestPower_Power; /* power */
            break;

        case kPDO_Variable:
            vbusPower->minVoltage = pdo.variablePDO.minVoltage;
            vbusPower->maxVoltage = pdo.variablePDO.maxVoltage;
            vbusPower->valueType  = kRequestPower_Current; /* current */
            break;

        default:
            break;
    }
}

/***************source need implement follow vbus power related functions***************/

pd_status_t PD_PowerSrcTurnOnDefaultVbus(void *callbackParam, uint8_t powerProgress)
{
    pd_vbus_power_t vbusPower;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    vbusPower.valueType  = kRequestPower_Current;
    vbusPower.minVoltage = vbusPower.maxVoltage = VSAFE5V_IN_50MV;
    vbusPower.requestValue                      = 0U;
    PD_PowerBoardSourceEnableVbusPower(pdAppInstance->portNumber, vbusPower);
    return kStatus_PD_Success;
}

pd_status_t PD_PowerSrcTurnOnRequestVbus(void *callbackParam, pd_rdo_t rdo)
{
    pd_vbus_power_t vbusPower;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    PD_PowerGetVbusVoltage(PD_PowerBoardGetSelfSourceCaps(callbackParam), rdo, &vbusPower);

    PD_PowerBoardSourceEnableVbusPower(pdAppInstance->portNumber, vbusPower);
    return kStatus_PD_Success;
}

pd_status_t PD_PowerSrcTurnOffVbus(void *callbackParam, uint8_t powerProgress)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    PD_PowerBoardReset(pdAppInstance->portNumber);
    PD_Control(pdAppInstance->pdHandle, PD_CONTROL_DISCHARGE_VBUS, NULL);
    return kStatus_PD_Success;
}

pd_status_t PD_PowerSrcGotoMinReducePower(void *callbackParam)
{
    /* in normal situation, only current is reduced,
       voltage don't change, so don't need any operation */
    return kStatus_PD_Success;
}

/***************sink need implement follow vbus power related functions***************/

pd_status_t PD_PowerSnkDrawTypeCVbus(void *callbackParam, uint8_t typecCurrentLevel, uint8_t powerProgress)
{
    pd_vbus_power_t vbusPower;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (powerProgress != (uint8_t)kVbusPower_ChangeInProgress)
    {
        vbusPower.valueType  = kRequestPower_Current;
        vbusPower.minVoltage = vbusPower.maxVoltage = VSAFE5V_IN_50MV;
        vbusPower.requestValue                      = 0U;
        PD_PowerBoardSinkEnableVbusPower(pdAppInstance->portNumber, vbusPower);
    }
    else
    {
        /* DPM instructs the power supply to reduce power comsumption to pSnkStdby. */
    }
    return kStatus_PD_Success;
}

pd_status_t PD_PowerSnkDrawRequestVbus(void *callbackParam, pd_rdo_t rdo)
{
    pd_vbus_power_t vbusPower;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    PD_PowerGetVbusVoltage(PD_PowerBoardGetPartnerSourceCaps(callbackParam), rdo, &vbusPower);
    if (vbusPower.minVoltage >= PD_POWER_REQUEST_MIN_VOLTAGE) /* 5V */
    {
        PD_PowerBoardSinkEnableVbusPower(pdAppInstance->portNumber, vbusPower);
    }

    return kStatus_PD_Success;
}

pd_status_t PD_PowerSnkStopDrawVbus(void *callbackParam, uint8_t powerProgress)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    PD_PowerBoardReset(pdAppInstance->portNumber);
    PD_Control(pdAppInstance->pdHandle, PD_CONTROL_DISCHARGE_VBUS, NULL);
    return kStatus_PD_Success;
}

pd_status_t PD_PowerSnkGotoMinReducePower(void *callbackParam)
{
    /* in normal situation, only current is reduced,
       voltage don't change, so don't need any operation */
    return kStatus_PD_Success;
}

/***************if support vconn, need implement the follow related functions***************/
pd_status_t PD_PowerControlVconn(void *callbackParam, uint8_t on)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    PD_PowerBoardControlVconn(pdAppInstance->portNumber, on);
    return kStatus_PD_Success;
}
