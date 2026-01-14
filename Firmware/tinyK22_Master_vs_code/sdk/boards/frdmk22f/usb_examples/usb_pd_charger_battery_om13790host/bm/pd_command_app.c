/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdbool.h>
#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_i2c.h"
#include "usb_pd_phy.h"
#include "pd_app.h"
#include "fsl_debug_console.h"
#include "pd_power_interface.h"
#include "pd_command_interface.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void PD_DemoReset(pd_app_t *pdAppInstance);
pd_status_t PD_DemoFindPDO(pd_app_t *pdAppInstance, pd_rdo_t *rdo, uint32_t *voltage);
void PD_PowerGetVbusVoltage(uint32_t *partnerSourceCaps, pd_rdo_t rdo, pd_vbus_power_t *vbusPower);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

pd_status_t PD_DpmSoftResetCallback(void *callbackParam)
{
    /* reset soft status */
    PRINTF("app soft reset\r\n");
    return kStatus_PD_Success;
}

pd_status_t PD_DpmHardResetCallback(void *callbackParam)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    PD_DemoReset(pdAppInstance);
    /* reset state, The Sink shall not draw more than iSafe0mA when VBUS is driven to vSafe0V. */
    PRINTF("hard reset request\r\n");
    return kStatus_PD_Success;
}

pd_status_t PD_DpmPowerRoleSwapRequestCallback(void *callbackParam, uint8_t frSwap, uint8_t *evaluateResult)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    *evaluateResult = ((pdAppInstance->prSwapAccept) ? kCommandResult_Accept : kCommandResult_Reject);
    return kStatus_PD_Success;
}

pd_status_t PD_DpmPowerRoleSwapResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (pdAppInstance->commandWait == 1)
    {
        pdAppInstance->commandWait   = 0;
        pdAppInstance->commandResult = (success ? kCommandResult_Success : failResultType);
    }

    if (success)
    {
        PD_DemoReset(pdAppInstance);
        PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &pdAppInstance->selfPowerRole);
        if (pdAppInstance->selfPowerRole == kPD_PowerRoleSource)
        {
            PRINTF("enter source\r\n");
        }
        else
        {
            PRINTF("enter sink\r\n");
        }
    }
    else
    {
        switch (failResultType)
        {
            case kCommandResult_Reject:
                PRINTF("pr swap result: reject\r\n");
                break;

            case kCommandResult_Wait:
                PRINTF("pr swap result: wait\r\n");
                break;

            case kCommandResult_Error:
                PRINTF("pr swap result: fail\r\n");
                break;

            case kCommandResult_NotSupported:
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmVconnSwapRequestCallback(void *callbackParam, uint8_t *evaluateResult)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    *evaluateResult = ((pdAppInstance->vconnSwapAccept) ? kCommandResult_Accept : kCommandResult_Reject);
    return kStatus_PD_Success;
}

pd_status_t PD_DpmVconnSwapResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    uint8_t roleInfo;

    if (success)
    {
        PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_VCONN_ROLE, &roleInfo);
        if (roleInfo == kPD_IsVconnSource)
        {
            PRINTF("vconn swap result: turn as source\r\n");
        }
        else
        {
            PRINTF("vconn swap result: not vconn source\r\n");
        }
    }
    else
    {
        switch (failResultType)
        {
            case kCommandResult_Reject:
                PRINTF("vconn swap result: reject\r\n");
                break;

            case kCommandResult_Wait:
                PRINTF("vconn swap result: wait\r\n");
                break;

            case kCommandResult_Error:
                PRINTF("vconn swap result: fail\r\n");
                break;

            case kCommandResult_NotSupported:
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSrcRDORequestCallback(void *callbackParam, pd_rdo_t rdo, uint8_t *negotiateResult)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    /* prepare for power supply, The power supply shall be ready to operate at the new power level within
     * tSrcReady */
    pdAppInstance->partnerRequestRDO = rdo;
    *negotiateResult                 = kCommandResult_Accept;
    return kStatus_PD_Success;
}

pd_status_t PD_DpmSrcPreContractStillValidCallback(void *callbackParam, uint8_t *isStillValid)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    /* if pre contract exist, return true */
    if (pdAppInstance->partnerRequestRDO.bitFields.objectPosition != 0)
    {
        *isStillValid = 1;
    }
    else
    {
        *isStillValid = 0;
    }
    return kStatus_PD_Success;
}

pd_status_t PD_DpmSrcRDOResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (success)
    {
        pdAppInstance->contractValid = 1;
    }
    else
    {
        switch (failResultType)
        {
            case kCommandResult_Error:
                PRINTF("source has error in power negotiation\r\n");
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmReceivePartnerSrcCapsCallback(void *callbackParam, pd_capabilities_t *caps)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    uint8_t index;

    if (pdAppInstance->commandWait == 1)
    {
        pdAppInstance->commandWait   = 0;
        pdAppInstance->commandResult = kCommandResult_Success;
    }
    pdAppInstance->partnerSourceCapNumber = caps->capabilitiesCount;
    for (index = 0; index < caps->capabilitiesCount; ++index)
    {
        pdAppInstance->partnerSourceCaps[index].PDOValue = caps->capabilities[index];
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmGetPartnerSrcCapsFailCallback(void *callbackParam, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (pdAppInstance->commandWait == 1)
    {
        pdAppInstance->commandWait   = 0;
        pdAppInstance->commandResult = failResultType;
    }
    switch (failResultType)
    {
        case kCommandResult_Error:
            PRINTF("get src cap fail");
            break;

        case kCommandResult_NotSupported:
            PRINTF("get src cap replying not supported");
            break;
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSnkGetRequestRDOCallback(void *callbackParam, pd_rdo_t *rdo)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    uint32_t voltage;

    if (PD_DemoFindPDO(pdAppInstance, rdo, &voltage) != kStatus_PD_Success)
    {
        PRINTF("cap mismatch\r\n");
    }
    pdAppInstance->sinkRequestVoltage = voltage;
    pdAppInstance->sinkRequestRDO     = *rdo;

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSnkRDOResultCallback(void *callbackParam, uint8_t success, uint32_t failResultTypeOrRDO)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (pdAppInstance->commandWait == 1)
    {
        pdAppInstance->commandWait   = 0;
        pdAppInstance->commandResult = (success ? kCommandResult_Success : failResultTypeOrRDO);
    }

    if (success)
    {
        pd_vbus_power_t vbusPower;
        pd_rdo_t rdo;

        pdAppInstance->contractValid = 1U;
        rdo.rdoVal                   = failResultTypeOrRDO;
        PD_PowerGetVbusVoltage(&(pdAppInstance->partnerSourceCaps[0].PDOValue), rdo, &vbusPower);
        PRINTF("sink request %dV success\r\n", (uint16_t)((vbusPower.minVoltage * 50U) / 1000U));
    }
    else
    {
        switch (failResultTypeOrRDO)
        {
            case kCommandResult_Error:
                PRINTF("sink request power result: fail\r\n");
                break;

            case kCommandResult_Reject:
                PRINTF("sink request power result: reject\r\n");
                break;

            case kCommandResult_Wait:
                PRINTF("sink request power result: wait\r\n");
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSrcGotoMinResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    return kStatus_PD_Success;
}

pd_status_t PD_DpmSnkGotoMinResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    return kStatus_PD_Success;
}

pd_status_t PD_DpmReceivePartnerSnkCapsCallback(void *callbackParam, pd_capabilities_t *caps)
{
    uint8_t index;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    pd_sink_pdo_t sinkPDO;
    pdAppInstance->partnerSinkCapNumber = caps->capabilitiesCount;
    for (index = 0; index < pdAppInstance->partnerSinkCapNumber; ++index)
    {
        sinkPDO.PDOValue = pdAppInstance->partnerSinkCaps[index].PDOValue = caps->capabilities[index];
        switch (sinkPDO.commonPDO.pdoType)
        {
            case kPDO_Fixed:
            {
                PRINTF("%d: fixed PDO; ", index + 1);
                PRINTF("vol:%dmV, current:%dmA\r\n", (uint16_t)(sinkPDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT),
                       (uint16_t)(sinkPDO.fixedPDO.operateCurrent * PD_PDO_CURRENT_UNIT));
                break;
            }

            case kPDO_Variable:
            {
                PRINTF("%d: variable PDO; ", index + 1);
                PRINTF("vol:%dmV ~ %dmV, current:%dmA\r\n",
                       (uint16_t)(sinkPDO.variablePDO.maxVoltage * PD_PDO_VOLTAGE_UNIT),
                       (uint16_t)(sinkPDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT),
                       (uint16_t)(sinkPDO.variablePDO.operateCurrent * PD_PDO_CURRENT_UNIT));
                break;
            }

            case kPDO_Battery:
            {
                PRINTF("%d: battery PDO; ", index + 1);
                PRINTF("vol:%dmV ~ %dmV, power:%dmW\r\n",
                       (uint16_t)(sinkPDO.batteryPDO.maxVoltage * PD_PDO_VOLTAGE_UNIT),
                       (uint16_t)(sinkPDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT),
                       (uint16_t)(sinkPDO.batteryPDO.operatePower * PD_PDO_POWER_UNIT));
            }

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmGetPartnerSnkCapsFailCallback(void *callbackParam, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (pdAppInstance->commandWait == 1U)
    {
        pdAppInstance->commandWait   = 0;
        pdAppInstance->commandResult = failResultType;
    }
    switch (failResultType)
    {
        case kCommandResult_Error:
            PRINTF("get snk cap fail");
            break;

        case kCommandResult_NotSupported:
            PRINTF("get snk cap replying not supported");
            break;
    }

    return kStatus_PD_Success;
}
