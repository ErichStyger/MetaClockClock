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

pd_status_t PD_DemoFindPDO(
    pd_app_t *pdAppInstance, pd_rdo_t *rdo, uint32_t requestVoltagemV, uint32_t requestCurrentmA, uint32_t *voltage);
void PD_DemoReset(pd_app_t *pdAppInstance);

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
    *evaluateResult = kCommandResult_Reject;
    return kStatus_PD_Success;
}

pd_status_t PD_DpmPowerRoleSwapResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    return kStatus_PD_Success;
}

pd_status_t PD_DpmDataRoleSwapRequestCallback(void *callbackParam, uint8_t *evaluateResult)
{
    *evaluateResult = kCommandResult_Reject;
    return kStatus_PD_Success;
}

pd_status_t PD_DpmDataRoleSwapResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    return kStatus_PD_Success;
}

pd_status_t PD_DpmVconnSwapRequestCallback(void *callbackParam, uint8_t *evaluateResult)
{
    *evaluateResult = kCommandResult_Reject;
    return kStatus_PD_Success;
}

pd_status_t PD_DpmVconnSwapResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    return kStatus_PD_Success;
}

pd_status_t PD_DpmReceivePartnerSrcCapsCallback(void *callbackParam, pd_capabilities_t *caps)
{
    uint8_t index;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    pd_source_pdo_t sourcePDO;
    PRINTF("receive source capabilities:\r\n");
    pdAppInstance->partnerSourceCapNumber = caps->capabilitiesCount;
    for (index = 0; index < pdAppInstance->partnerSourceCapNumber; ++index)
    {
        pdAppInstance->partnerSourceCaps[index].PDOValue = caps->capabilities[index];
        sourcePDO.PDOValue                               = caps->capabilities[index];
        switch (sourcePDO.commonPDO.pdoType)
        {
            case kPDO_Fixed:
            {
                PRINTF("%d: fixed PDO; ", index + 1);
                PRINTF("vol:%dmV, current:%dmA\r\n", (uint16_t)(sourcePDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT),
                       (uint16_t)(sourcePDO.fixedPDO.maxCurrent * PD_PDO_CURRENT_UNIT));
                break;
            }

            case kPDO_Variable:
            {
                PRINTF("%d: variable PDO; ", index + 1);
                PRINTF("vol:%dmV ~ %dmV, current:%dmA\r\n",
                       (uint16_t)(sourcePDO.variablePDO.maxVoltage * PD_PDO_VOLTAGE_UNIT),
                       (uint16_t)(sourcePDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT),
                       (uint16_t)(sourcePDO.variablePDO.maxCurrent * PD_PDO_CURRENT_UNIT));
                break;
            }

            case kPDO_Battery:
            {
                PRINTF("%d: battery PDO; ", index + 1);
                PRINTF("vol:%dmV ~ %dmV, power:%dmW\r\n",
                       (uint16_t)(sourcePDO.batteryPDO.maxVoltage * PD_PDO_VOLTAGE_UNIT),
                       (uint16_t)(sourcePDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT),
                       (uint16_t)(sourcePDO.batteryPDO.maxAllowPower * PD_PDO_POWER_UNIT));
                break;
            }

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmGetPartnerSrcCapsFailCallback(void *callbackParam, uint8_t failResultType)
{
    switch (failResultType)
    {
        case kCommandResult_Error:
            PRINTF("get src cap fail");
            break;

        case kCommandResult_NotSupported:
            PRINTF("get src cap replying not supported");
            break;

        default:
            break;
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSnkGetRequestRDOCallback(void *callbackParam, pd_rdo_t *rdo)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    pd_rdo_t tmpRDO;
    uint8_t snkCapIndex;
    uint32_t voltage;
    pd_sink_pdo_t snkPDO;

    /* check the sink cpas rquired voltage and current */
    for (snkCapIndex = (((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sinkCapCount - 1);
         snkCapIndex > 0; --snkCapIndex)
    {
        snkPDO.PDOValue = ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sinkCaps[snkCapIndex];
        if (PD_DemoFindPDO(pdAppInstance, &tmpRDO, snkPDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT,
                           snkPDO.fixedPDO.operateCurrent * PD_PDO_CURRENT_UNIT, &voltage) == kStatus_PD_Success)
        {
            break;
        }
    }

    /* check the sink cpas rquired voltage and current is the source supported max current */
    for (snkCapIndex = (((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sinkCapCount - 1);
         snkCapIndex > 0; --snkCapIndex)
    {
        snkPDO.PDOValue = ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sinkCaps[snkCapIndex];
        if (PD_DemoFindPDO(pdAppInstance, &tmpRDO, snkPDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT, 0u, &voltage) ==
            kStatus_PD_Success)
        {
            break;
        }
    }

    snkPDO.PDOValue = ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sinkCaps[0];
    if (snkCapIndex > 0)
    {
        pdAppInstance->sinkRequestVoltage = voltage;
        rdo->rdoVal = pdAppInstance->sinkRequestRDO.rdoVal = tmpRDO.rdoVal;
        return kStatus_PD_Success;
    }
    /* all high voltage is not satisfied */
    else if (PD_DemoFindPDO(pdAppInstance, &tmpRDO, snkPDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT,
                            snkPDO.fixedPDO.operateCurrent * PD_PDO_CURRENT_UNIT, &voltage) == kStatus_PD_Success)
    {
        pdAppInstance->sinkRequestVoltage = voltage;
        rdo->rdoVal = pdAppInstance->sinkRequestRDO.rdoVal = tmpRDO.rdoVal;
        return kStatus_PD_Success;
    }
    else
    {
        PRINTF("cap mismatch\r\n");
        pdAppInstance->sinkRequestVoltage                                = 5000;
        pdAppInstance->sinkRequestRDO.bitFields.objectPosition           = 1;
        pdAppInstance->sinkRequestRDO.bitFields.giveBack                 = 0;
        pdAppInstance->sinkRequestRDO.bitFields.capabilityMismatch       = 1;
        pdAppInstance->sinkRequestRDO.bitFields.usbCommunicationsCapable = 0;
        pdAppInstance->sinkRequestRDO.bitFields.noUsbSuspend             = 1;
        pdAppInstance->sinkRequestRDO.bitFields.operateValue             = 270; /* 2.7A */
        pdAppInstance->sinkRequestRDO.bitFields.maxOrMinOperateValue     = 270;
        rdo->rdoVal                                                      = pdAppInstance->sinkRequestRDO.rdoVal;
        return kStatus_PD_Error;
    }
}

pd_status_t PD_DpmSnkRDOResultCallback(void *callbackParam, uint8_t success, uint32_t failResultTypeOrRDO)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (success)
    {
        pd_vbus_power_t vbusPower;
        pd_rdo_t rdo;

        pdAppInstance->contractValid = 1;
        rdo.rdoVal                   = failResultTypeOrRDO;
        PD_PowerGetVbusVoltage(&(pdAppInstance->partnerSourceCaps[0].PDOValue), rdo, &vbusPower);
        PRINTF("sink request %dV success\r\n", (uint16_t)((vbusPower.minVoltage * 50U) / 1000));
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

pd_status_t PD_DpmSnkGotoMinResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    if (success)
    {
        PRINTF("success\r\n");
    }
    else
    {
        PRINTF("fail\r\n");
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSVDMRequestCallback(void *callbackParam, pd_svdm_command_request_t *svdmRequest)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    /* common process */
    switch (svdmRequest->vdmHeader.bitFields.command)
    {
        case kVDM_DiscoverIdentity:
        case kVDM_DiscoverSVIDs:
            break;

        case kVDM_DiscoverModes:
        case kVDM_EnterMode:
        case kVDM_ExitMode:
            if (svdmRequest->vdmHeader.bitFields.SVID != PD_VENDOR_VID)
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMNAK;
                return kStatus_PD_Success;
            }
            break;

        default:
            break;
    }

    /* ack or nak, no busy */
    /* partner return nak if it is not in the alternate mode */
    switch (svdmRequest->vdmHeader.bitFields.command)
    {
        case kVDM_DiscoverIdentity:
            svdmRequest->vdoData             = (uint32_t *)&pdAppInstance->selfVdmIdentity;
            svdmRequest->vdoCount            = sizeof(pdAppInstance->selfVdmIdentity) / 4;
            svdmRequest->requestResultStatus = kCommandResult_VDMACK;
            break;

        case kVDM_DiscoverSVIDs:
            svdmRequest->vdoData             = (uint32_t *)&pdAppInstance->selfVdmSVIDs;
            svdmRequest->vdoCount            = 1;
            svdmRequest->requestResultStatus = kCommandResult_VDMACK;
            break;

        case kVDM_DiscoverModes:
            svdmRequest->vdoData             = NULL;
            svdmRequest->vdoCount            = 0;
            svdmRequest->requestResultStatus = kCommandResult_VDMACK;
            break;

        case kVDM_EnterMode:
            svdmRequest->requestResultStatus = kCommandResult_VDMNAK;
            break;

        case kVDM_ExitMode:
            svdmRequest->requestResultStatus = kCommandResult_VDMNAK;
            break;

        case kVDM_Attention:
            PRINTF("receive attention\r\n");
            break;

        default:
            /* vendor defined structured vdm */
            svdmRequest->requestResultStatus = kCommandResult_VDMACK;
            break;
    }

    return kStatus_PD_Success;
}
