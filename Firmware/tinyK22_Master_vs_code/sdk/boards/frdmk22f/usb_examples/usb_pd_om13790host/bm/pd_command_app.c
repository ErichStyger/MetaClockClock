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
#include "pd_board_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* The follow MACROs are for passing compliance test, it is not actual product logical */
#define PD_STATUS_DATA_SIZE            (6U)
#define PD_BATTERY_CAP_DATA_SIZE       (9U)
#define PD_BATTERY_STATUS_DATA_SIZE    (4U)
#define PD_MANUFACTURER_INFO_DATA_SIZE (4U)

pd_status_t PD_DemoFindPDO(
    pd_app_t *pdAppInstance, pd_rdo_t *rdo, uint32_t requestVoltagemV, uint32_t requestCurrentmA, uint32_t *voltage);
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

    /* reset state, The Sink shall not draw more than iSafe0mA when VBUS is driven to vSafe0V. */
    pdAppInstance->selfHasEnterAlernateMode = 0U;
    PRINTF("hard reset request\r\n");
    return kStatus_PD_Success;
}

pd_status_t PD_DpmPowerRoleSwapRequestCallback(void *callbackParam, uint8_t frSwap, uint8_t *evaluateResult)
{
#if (defined PD_CONFIG_COMPLIANCE_TEST_ENABLE) && (PD_CONFIG_COMPLIANCE_TEST_ENABLE)
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    uint8_t powerRole;
    pd_source_pdo_t pdo;

    if (frSwap)
    {
        *evaluateResult = kCommandResult_Accept;
    }
    else
    {
        PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &powerRole);

        pdo.PDOValue = ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCaps[0U];
        if ((powerRole == kPD_PowerRoleSource) && (pdo.fixedPDO.externalPowered))
        {
            if ((pdAppInstance->partnerSourceCapNumber == 0U) && (pdAppInstance->partnerSinkCapNumber == 0U))
            {
                *evaluateResult = kCommandResult_Wait;
                PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_GET_PARTNER_SOURCE_CAPABILITIES, NULL);
            }
            else
            {
                if (pdAppInstance->partnerSourceCapNumber)
                {
                    if (pdAppInstance->partnerSourceCaps[0U].fixedPDO.externalPowered == 0U)
                    {
                        *evaluateResult = kCommandResult_Reject;
                    }
                    else
                    {
                        *evaluateResult = kCommandResult_Accept;
                    }
                }
                else
                {
                    if (pdAppInstance->partnerSinkCaps[0].fixedPDO.externalPowered == 0U)
                    {
                        *evaluateResult = kCommandResult_Reject;
                    }
                    else
                    {
                        *evaluateResult = kCommandResult_Accept;
                    }
                }
            }
        }
        else
        {
            *evaluateResult = kCommandResult_Accept;
        }
    }
#else
    *evaluateResult = kCommandResult_Accept;
#endif
    return kStatus_PD_Success;
}

pd_status_t PD_DpmPowerRoleSwapResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    uint8_t roleInfo;

    if (success)
    {
        PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &roleInfo);
        PRINTF("enter ");
        if (roleInfo == kPD_PowerRoleSource)
        {
            PRINTF("source\r\n");
        }
        else
        {
            g_PDAppInstancePort1.isDoingFrs = 0U;
            PRINTF("sink\r\n");
        }
    }
    else
    {
        PRINTF("power role swap result: ");
        switch (failResultType)
        {
            case kCommandResult_Reject:
                PRINTF("reject\r\n");
                break;

            case kCommandResult_Wait:
                PRINTF("wait\r\n");
                break;

            case kCommandResult_Error:
                PRINTF("fail\r\n");
                break;

            case kCommandResult_NotSupported:
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmDataRoleSwapRequestCallback(void *callbackParam, uint8_t *evaluateResult)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    *evaluateResult = ((pdAppInstance->drSwapAccept) ? kCommandResult_Accept : kCommandResult_Reject);
    return kStatus_PD_Success;
}

pd_status_t PD_DpmDataRoleSwapResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    uint8_t roleInfo;

    if (success)
    {
        PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_DATA_ROLE, &roleInfo);
        PRINTF("dr swap result: enter");
        if (roleInfo == kPD_DataRoleDFP)
        {
            PRINTF("dfp\r\n");
        }
        else
        {
            PRINTF("ufp\r\n");
        }
    }
    else
    {
        PRINTF("dr swap result: ");
        switch (failResultType)
        {
            case kCommandResult_Error:
                PRINTF("fail\r\n");
                break;

            case kCommandResult_Reject:
                PRINTF("reject\r\n");
                break;

            case kCommandResult_Wait:
                PRINTF("wait\r\n");
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
        PRINTF("vconn swap result: ");
        if (roleInfo == kPD_IsVconnSource)
        {
            PRINTF("turn as source\r\n");
        }
        else
        {
            PRINTF("not vconn source\r\n");
        }
    }
    else
    {
        PRINTF("vconn swap result: ");
        switch (failResultType)
        {
            case kCommandResult_Reject:
                PRINTF("reject\r\n");
                break;

            case kCommandResult_Wait:
                PRINTF("wait\r\n");
                break;

            case kCommandResult_Error:
                PRINTF("fail\r\n");
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
    pd_source_pdo_t pdo;
    uint8_t accept = 0U;

    /* prepare for power supply, The power supply shall be ready to operate at the new power level within
     * tSrcReady */
    if (rdo.bitFields.objectPosition == 1U)
    {
        pdo.PDOValue = ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCaps[0U];
        if (rdo.bitFields.operateValue <= pdo.fixedPDO.maxCurrent)
        {
            accept = 1U;
        }
    }
    else if ((rdo.bitFields.objectPosition == 2U) &&
             (((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount >= 2U))
    {
        pdo.PDOValue = ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCaps[1U];
        if (rdo.bitFields.operateValue <= pdo.fixedPDO.maxCurrent)
        {
            accept = 1U;
        }
    }
    else
    {
    }

    if (accept)
    {
        pdAppInstance->sinkRequestRDO.rdoVal = rdo.rdoVal;
        *negotiateResult                     = kCommandResult_Accept;
    }
    else
    {
        *negotiateResult = kCommandResult_Reject;
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSrcPreContractStillValidCallback(void *callbackParam, uint8_t *isStillValid)
{
    /* if pre contract exist, return true */
    *isStillValid = 1;
    return kStatus_PD_Success;
}

pd_status_t PD_DpmSrcRDOResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (success)
    {
        PRINTF("partner sink's request %dV success\r\n",
               (pdAppInstance->sinkRequestRDO.bitFields.objectPosition == 1U) ? 5U : 9U);
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
    uint8_t index;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    pd_source_pdo_t sourcePDO;
    PRINTF("receive source capabilities:\r\n");
    pdAppInstance->partnerSourceCapNumber = caps->capabilitiesCount;
    for (index = 0U; index < pdAppInstance->partnerSourceCapNumber; ++index)
    {
        pdAppInstance->partnerSourceCaps[index].PDOValue = caps->capabilities[index];
        sourcePDO.PDOValue                               = caps->capabilities[index];
        switch (sourcePDO.commonPDO.pdoType)
        {
            case kPDO_Fixed:
            {
                PRINTF("%d: fixed PDO; ", index + 1U);
                PRINTF("vol:%dmV, current:%dmA\r\n", (uint16_t)(sourcePDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT),
                       (uint16_t)(sourcePDO.fixedPDO.maxCurrent * PD_PDO_CURRENT_UNIT));
                break;
            }

            case kPDO_Variable:
            {
                PRINTF("%d: variable PDO; ", index + 1U);
                PRINTF("vol:%dmV ~ %dmV, current:%dmA\r\n",
                       (uint16_t)(sourcePDO.variablePDO.maxVoltage * PD_PDO_VOLTAGE_UNIT),
                       (uint16_t)(sourcePDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT),
                       (uint16_t)(sourcePDO.variablePDO.maxCurrent * PD_PDO_CURRENT_UNIT));
                break;
            }

            case kPDO_Battery:
            {
                PRINTF("%d: battery PDO; ", index + 1U);
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
    PRINTF("get src cap ");
    switch (failResultType)
    {
        case kCommandResult_Error:
            PRINTF("fail");
            break;

        case kCommandResult_NotSupported:
            PRINTF("replying not supported");
            break;

        default:
            break;
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSnkGetRequestRDOCallback(void *callbackParam, pd_rdo_t *rdo)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    uint32_t voltage;

    if (PD_DemoFindPDO(pdAppInstance, rdo, PD_DEMO_EXPECTED_VOLTAGE, PD_DEMO_EXPECTED_CURRENT, &voltage) !=
        kStatus_PD_Success)
    {
        PRINTF("cap mismatch\r\n");
    }
    pdAppInstance->sinkRequestRDO     = *rdo;
    pdAppInstance->sinkRequestVoltage = voltage;

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSnkRDOResultCallback(void *callbackParam, uint8_t success, uint32_t failResultTypeOrRDO)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (success)
    {
        pd_vbus_power_t vbusPower;
        pd_rdo_t rdo;

        rdo.rdoVal = failResultTypeOrRDO;
        PD_PowerGetVbusVoltage(&(pdAppInstance->partnerSourceCaps[0].PDOValue), rdo, &vbusPower);
        PRINTF("sink request %dV success\r\n", (uint16_t)((vbusPower.minVoltage * 50U) / 1000U));
    }
    else
    {
        PRINTF("sink request power result: ");
        switch (failResultTypeOrRDO)
        {
            case kCommandResult_Error:
                PRINTF("fail\r\n");
                break;

            case kCommandResult_Reject:
                PRINTF("reject\r\n");
                break;

            case kCommandResult_Wait:
                PRINTF("wait\r\n");
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSrcGotoMinResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
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

pd_status_t PD_DpmReceivePartnerSnkCapsCallback(void *callbackParam, pd_capabilities_t *caps)
{
    uint8_t index;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    pd_sink_pdo_t sinkPDO;
    PRINTF("receive sink capabilities:\r\n");
    pdAppInstance->partnerSinkCapNumber = caps->capabilitiesCount;
    for (index = 0U; index < pdAppInstance->partnerSinkCapNumber; ++index)
    {
        pdAppInstance->partnerSinkCaps[index].PDOValue = caps->capabilities[index];
        sinkPDO.PDOValue                               = caps->capabilities[index];
        switch (sinkPDO.commonPDO.pdoType)
        {
            case kPDO_Fixed:
            {
                PRINTF("%d: fixed PDO; ", index + 1U);
                PRINTF("vol:%dmV, current:%dmA\r\n", (uint16_t)(sinkPDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT),
                       (uint16_t)(sinkPDO.fixedPDO.operateCurrent * PD_PDO_CURRENT_UNIT));
                break;
            }

            case kPDO_Variable:
            {
                PRINTF("%d: variable PDO; ", index + 1U);
                PRINTF("vol:%dmV ~ %dmV, current:%dmA\r\n",
                       (uint16_t)(sinkPDO.variablePDO.maxVoltage * PD_PDO_VOLTAGE_UNIT),
                       (uint16_t)(sinkPDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT),
                       (uint16_t)(sinkPDO.variablePDO.operateCurrent * PD_PDO_CURRENT_UNIT));
                break;
            }

            case kPDO_Battery:
            {
                PRINTF("%d: battery PDO; ", index + 1U);
                PRINTF("vol:%dmV ~ %dmV, power:%dmW\r\n",
                       (uint16_t)(sinkPDO.batteryPDO.maxVoltage * PD_PDO_VOLTAGE_UNIT),
                       (uint16_t)(sinkPDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT),
                       (uint16_t)(sinkPDO.batteryPDO.operatePower * PD_PDO_POWER_UNIT));
                break;
            }

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmGetPartnerSnkCapsFailCallback(void *callbackParam, uint8_t failResultType)
{
    PRINTF("get snk cap ");
    switch (failResultType)
    {
        case kCommandResult_Error:
            PRINTF("fail");
            break;

        case kCommandResult_NotSupported:
            PRINTF("replying not supported");
            break;

        default:
            break;
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
            svdmRequest->vdoData  = (uint32_t *)&pdAppInstance->selfVdmIdentity;
            svdmRequest->vdoCount = sizeof(pdAppInstance->selfVdmIdentity) / 4U;
            if (pdAppInstance->reqestResponse == kCommandResult_Accept)
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMACK;
            }
            else
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMBUSY;
            }
            break;

        case kVDM_DiscoverSVIDs:
            svdmRequest->vdoData  = (uint32_t *)&pdAppInstance->selfVdmSVIDs;
            svdmRequest->vdoCount = 1U;
            if (pdAppInstance->reqestResponse == kCommandResult_Accept)
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMACK;
            }
            else
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMBUSY;
            }
            break;

        case kVDM_DiscoverModes:
            svdmRequest->vdoData  = (uint32_t *)&pdAppInstance->selfVdmModes;
            svdmRequest->vdoCount = 1U;
            if (pdAppInstance->reqestResponse == kCommandResult_Accept)
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMACK;
            }
            else
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMBUSY;
            }
            break;

        case kVDM_EnterMode:
            svdmRequest->vdoData  = NULL;
            svdmRequest->vdoCount = 0U;
            if (pdAppInstance->reqestResponse == kCommandResult_Accept)
            {
                if (svdmRequest->vdmHeader.bitFields.objPos == 1U)
                {
                    svdmRequest->requestResultStatus        = kCommandResult_VDMACK;
                    pdAppInstance->selfHasEnterAlernateMode = 1U;
                }
                else
                {
                    svdmRequest->requestResultStatus        = kCommandResult_VDMNAK;
                    pdAppInstance->selfHasEnterAlernateMode = 1U;
                }
            }
            else
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMBUSY;
            }
            break;

        case kVDM_ExitMode:
            svdmRequest->vdoData  = NULL;
            svdmRequest->vdoCount = 0U;
            if ((pdAppInstance->selfHasEnterAlernateMode == 1U) && (svdmRequest->vdmHeader.bitFields.objPos == 1U))
            {
                pdAppInstance->selfHasEnterAlernateMode = 0U;
                svdmRequest->requestResultStatus        = kCommandResult_VDMACK;
            }
            else
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMNAK;
            }
            break;

        case kVDM_Attention:
        {
            uint8_t dataRole;

            PRINTF("receive attention\r\n");
            pdAppInstance->structuredVDMCommandParam.vdmSop = pdAppInstance->msgSop;
            pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.objPos =
                svdmRequest->vdmHeader.bitFields.objPos;
            pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.SVID = svdmRequest->vdmHeader.bitFields.SVID;

            PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_DATA_ROLE, &dataRole);
            if (dataRole == kPD_DataRoleDFP)
            {
                PRINTF("start exit mode command\r\n");
                if (PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_EXIT_MODE,
                               &pdAppInstance->structuredVDMCommandParam) != kStatus_PD_Success)
                {
                    PRINTF("command start fail\r\n");
                }
            }
            break;
        }

        default:
            /* vendor defined structured vdm */
            if (pdAppInstance->reqestResponse == kCommandResult_Accept)
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMACK;
            }
            else
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMBUSY;
            }
            break;
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSVDMResultCallback(void *callbackParam, uint8_t success, pd_svdm_command_result_t *svdmResult)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (success)
    {
        switch (svdmResult->vdmCommand)
        {
            case kVDM_DiscoverIdentity:
            {
                pd_id_header_vdo_t idHeaderVDO;
                idHeaderVDO.vdoValue = USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)(svdmResult->vdoData)));
                PRINTF("vendor:%x, modual support:%d, usb communication capable device:%d",
                       (uint16_t)idHeaderVDO.bitFields.usbVendorID, (uint16_t)idHeaderVDO.bitFields.modalOperateSupport,
                       (uint16_t)idHeaderVDO.bitFields.usbCommunicationCapableAsDevice);
                PRINTF(", usb communication capable host:%d",
                       (uint16_t)idHeaderVDO.bitFields.usbCommunicationCapableAsHost);
                PRINTF(", product type: (DFP or Cable)");
                if (idHeaderVDO.bitFields.productTypeDFP != 0U)
                {
                    switch (idHeaderVDO.bitFields.productTypeDFP)
                    {
                        case 0:
                            PRINTF("undefined, ");
                            break;

                        case 1:
                            PRINTF("PDUSB Hub, ");
                            break;

                        case 2:
                            PRINTF("PDUSB Host, ");
                            break;

                        case 3:
                            PRINTF("Power Brick, ");
                            break;

                        case 4:
                            PRINTF("Alternate Mode Controller (AMC), ");
                            break;

                        default:
                            PRINTF("cannot recognition, ");
                            break;
                    }
                }

                PRINTF("(UFP)");
                if (idHeaderVDO.bitFields.productTypeUFPOrCablePlug != 0U)
                {
                    switch (idHeaderVDO.bitFields.productTypeUFPOrCablePlug)
                    {
                        case 0:
                            PRINTF("undefined\r\n");
                            break;

                        case 1:
                            PRINTF("PDUSB Hub\r\n");
                            break;

                        case 2:
                            PRINTF("PDUSB Peripheral\r\n");
                            break;

                        case 3:
                            PRINTF("Passive Cable\r\n");
                            break;

                        case 4:
                            PRINTF("Active Cable\r\n");
                            break;

                        case 5:
                            PRINTF("Alternate Mode Adapter (AMA)\r\n");
                            break;

                        default:
                            PRINTF("cannot recognition\r\n");
                            break;
                    }
                }

                PRINTF("(2) discovery SVIDs\r\n");
                pdAppInstance->structuredVDMCommandParam.vdmSop = pdAppInstance->msgSop;
                if (PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_DISCOVERY_SVIDS,
                               &pdAppInstance->structuredVDMCommandParam) != kStatus_PD_Success)
                {
                    PRINTF("command fail\r\n");
                }
                break;
            }

            case kVDM_DiscoverSVIDs:
            {
                uint32_t index;
                uint8_t svidIndex = 1U;
                uint32_t SVID;
                uint8_t *buffPtr = (uint8_t *)svdmResult->vdoData;
                for (index = 0; index < (svdmResult->vdoCount); ++index)
                {
                    SVID = USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(buffPtr);
                    buffPtr += 4U;
                    if (SVID != 0U)
                    {
                        pdAppInstance->partnerSVIDs[index * 2U] = SVID >> 16U;
                        if (pdAppInstance->partnerSVIDs[index * 2U] != 0x0000u)
                        {
                            PRINTF("SVID%d: %x\r\n", svidIndex, (uint16_t)pdAppInstance->partnerSVIDs[index * 2U]);
                            svidIndex++;
                        }

                        pdAppInstance->partnerSVIDs[index * 2U + 1U] = (SVID & 0x0000FFFFu);
                        if (pdAppInstance->partnerSVIDs[index * 2U + 1U] != 0x0000u)
                        {
                            PRINTF("SVID%d: %x\r\n", svidIndex, (uint16_t)pdAppInstance->partnerSVIDs[index * 2U + 1U]);
                            svidIndex++;
                        }
                    }

                    pdAppInstance->partnerSVIDs[index * 2U + 1U] = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(buffPtr);
                }

                if (pdAppInstance->partnerSVIDs[0] == 0x0000u)
                {
                    PRINTF("SVIDs: none\r\n");
                }
                else
                {
                    PRINTF("(3) discovery Modes\r\n");
                    pdAppInstance->structuredVDMCommandParam.vdmSop                   = pdAppInstance->msgSop;
                    pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.SVID = pdAppInstance->partnerSVIDs[0];
                    if (PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_DISCOVERY_MODES,
                                   &pdAppInstance->structuredVDMCommandParam) != kStatus_PD_Success)
                    {
                        PRINTF("command fail\r\n");
                    }
                }
                break;
            }

            case kVDM_DiscoverModes:
            {
                uint8_t index;
                uint8_t *buffPtr = (uint8_t *)svdmResult->vdoData;
                uint8_t dataRole;
                for (index = 0U; index < (svdmResult->vdoCount); ++index)
                {
                    pdAppInstance->partnerModes[index * 2U] = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(buffPtr);
                    buffPtr += 2U;
                    PRINTF("Mode%d: %x\r\n", 1 + index * 2U, (uint16_t)pdAppInstance->partnerModes[index * 2U]);
                    pdAppInstance->partnerModes[index * 2U + 1U] = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(buffPtr);
                    buffPtr += 2U;
                    PRINTF("Mode%d: %x\r\n", 1 + index * 2U + 1U,
                           (uint16_t)pdAppInstance->partnerModes[index * 2U + 1U]);
                }

                PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_DATA_ROLE, &dataRole);

                if (dataRole == kPD_DataRoleDFP)
                {
                    PRINTF("(4) enter mode 1\r\n");
                    pdAppInstance->structuredVDMCommandParam.vdmSop                     = pdAppInstance->msgSop;
                    pdAppInstance->structuredVDMCommandParam.vdoCount                   = 0U;
                    pdAppInstance->structuredVDMCommandParam.vdoData                    = NULL;
                    pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.objPos = 1U;
                    pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.SVID = pdAppInstance->partnerSVIDs[0U];
                    if (PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_ENTER_MODE,
                                   &pdAppInstance->structuredVDMCommandParam) != kStatus_PD_Success)
                    {
                        PRINTF("command fail\r\n");
                    }
                }
                break;
            }

            case kVDM_EnterMode:
                PRINTF("enter mode result: ack\r\n");
                break;

            case kVDM_ExitMode:
                PRINTF("exit mode result: ack\r\n");
                break;

            case kVDM_Attention:
                PRINTF("send attention result: success\r\n");
                break;

            default:
            {
                /* process the buffer data and length */
                /* pd_svdm_command_result_t *vdmResult = (pd_svdm_command_result_t*)param; */
                PRINTF("vendor structured vdm result: success\r\n");
                break;
            }
        }
    }
    else
    {
        PRINTF("structured vdm result: ");
        switch (svdmResult->vdmCommandResult)
        {
            case kCommandResult_Error:
                PRINTF("fail\r\n");
                break;

            case kCommandResult_VDMNAK:
                PRINTF("nak\r\n");
                break;

            case kCommandResult_VDMBUSY:
                PRINTF("busy\r\n");
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmUnstructuredVDMReceivedCallback(void *callbackParam,
                                                  pd_unstructured_vdm_command_param_t *unstructuredVDMParam)
{
    uint8_t index;
    pd_unstructured_vdm_header_t vdmHeader =
        ((pd_unstructured_vdm_header_t *)(unstructuredVDMParam->vdmHeaderAndVDOsData))[0];

    if (vdmHeader.bitFields.SVID != PD_VENDOR_ID_NXP)
    {
        unstructuredVDMParam->resultStatus = kCommandResult_NotSupported;
    }

    /* process the unstructured vdm */
    PRINTF("receive unstructured vdm, sop:%d, vdo count:%d\r\n", unstructuredVDMParam->vdmSop,
           unstructuredVDMParam->vdmHeaderAndVDOsCount);
    PRINTF("VDO Header:%x\r\n", unstructuredVDMParam->vdmHeaderAndVDOsData[0]);
    for (index = 1U; index < unstructuredVDMParam->vdmHeaderAndVDOsCount; ++index)
    {
        PRINTF("VDO%d:%d\r\n", index, unstructuredVDMParam->vdmHeaderAndVDOsData[index]);
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmUnstructuredVDMSendResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    PRINTF("send unstructured vdm result: ");
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

#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
pd_status_t PD_DpmGetInfoRequestCallback(void *callbackParam, uint8_t type, pd_command_data_param_t *dataParam)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    switch (type)
    {
        case kInfoType_SrcExtCap:
            if (pdAppInstance->reqestResponse != kCommandResult_Accept)
            {
                dataParam->resultStatus = kCommandResult_NotSupported;
            }
            else
            {
                dataParam->resultStatus = kCommandResult_Accept;
            }

            dataParam->dataBuffer = (uint8_t *)&pdAppInstance->selfExtCap;
            dataParam->dataLength = sizeof(pdAppInstance->selfExtCap);
            break;

        case kInfoType_Status:
            if (pdAppInstance->reqestResponse != kCommandResult_Accept)
            {
                dataParam->resultStatus = kCommandResult_NotSupported;
            }
            else
            {
                dataParam->resultStatus = kCommandResult_Accept;
            }

            dataParam->dataBuffer = (uint8_t *)&pdAppInstance->selfStatus;
            dataParam->dataLength = PD_STATUS_DATA_SIZE;
            break;

        case kInfoType_BatteryCap:
            if (pdAppInstance->reqestResponse != kCommandResult_Accept)
            {
                dataParam->resultStatus = kCommandResult_NotSupported;
            }
            else
            {
                dataParam->resultStatus = kCommandResult_Accept;
            }
            dataParam->dataBuffer = (uint8_t *)&pdAppInstance->selfBatteryCap;
            dataParam->dataLength = PD_BATTERY_CAP_DATA_SIZE;
            break;

        case kInfoType_BatteryStatus:
            if (pdAppInstance->reqestResponse != kCommandResult_Accept)
            {
                dataParam->resultStatus = kCommandResult_NotSupported;
            }
            else
            {
                dataParam->resultStatus = kCommandResult_Accept;
            }
            dataParam->dataBuffer = (uint8_t *)&pdAppInstance->selfBatteryStatus;
            dataParam->dataLength = PD_BATTERY_STATUS_DATA_SIZE;
            break;

        case kInfoType_ManufacturerInfo:
        {
            uint8_t length;
            pd_get_manufac_info_data_block_t *getManufacInfo;

            if (pdAppInstance->reqestResponse != kCommandResult_Accept)
            {
                dataParam->resultStatus = kCommandResult_NotSupported;
            }
            else
            {
                dataParam->resultStatus = kCommandResult_Accept;
            }

            getManufacInfo = (pd_get_manufac_info_data_block_t *)(&dataParam->dataBuffer[0]);
            if ((getManufacInfo->manufacturerInfoRef != 0U) || (getManufacInfo->manufacturerInfoTarget > 1U))
            {
                /* Manufacturer String:
                 * If the Manufacturer Info Target field or Manufacturer Info Ref
                 * field in the Get_Manufacturer_Info Message is unrecognized
                 * return zero bytes */
                /* there is no battery */
                length                             = sizeof("Not Supported");
                pdAppInstance->selfManufacInfo.vid = PD_INVALID_VID;
                pdAppInstance->selfManufacInfo.pid = PD_INVALID_PID;
                memcpy(pdAppInstance->selfManufacInfo.manufacturerString, "Not Supported", length);
            }
            else
            {
                length                                               = 4U;
                pdAppInstance->selfManufacInfo.vid                   = PD_VENDOR_VID;
                pdAppInstance->selfManufacInfo.pid                   = PD_CONFIG_PID;
                pdAppInstance->selfManufacInfo.manufacturerString[0] = 'N';
                pdAppInstance->selfManufacInfo.manufacturerString[1] = 'X';
                pdAppInstance->selfManufacInfo.manufacturerString[2] = 'P';
                pdAppInstance->selfManufacInfo.manufacturerString[3] = '\0'; /* TEST.PD.PROT.ALL3.2 */
            }
            dataParam->dataLength = PD_MANUFACTURER_INFO_DATA_SIZE + length;
            dataParam->dataBuffer = (uint8_t *)&pdAppInstance->selfManufacInfo;
            break;
        }

        default:
            break;
    }
    return kStatus_PD_Success;
}

pd_status_t PD_DpmGetInfoResultCallback(
    void *callbackParam, uint8_t type, uint8_t success, pd_command_data_param_t *successData, uint8_t failResultType)
{
    if (success)
    {
        switch (type)
        {
            case kInfoType_SrcExtCap:
            {
                pd_source_cap_ext_data_block_t extDataBlock;
                uint32_t xid;
                memcpy(&extDataBlock, successData->dataBuffer, sizeof(pd_source_cap_ext_data_block_t));
                xid = USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)&(extDataBlock.xid)));
                PRINTF("vid:%x, pid:%x, xid:%x%x, fw version:%d, hw version:%d\r\n", (uint16_t)extDataBlock.vid,
                       (uint16_t)extDataBlock.pid, (uint16_t)(xid >> 16), (uint16_t)(xid),
                       (uint8_t)extDataBlock.fwVersion, (uint8_t)extDataBlock.hwVersion);
                break;
            }

            case kInfoType_Status:
            {
                pd_status_data_block_t *status = (pd_status_data_block_t *)(&successData->dataBuffer[0]);
                PRINTF("Internal Temp:%d\r\n", status->internalTemp);
                break;
            }

            case kInfoType_BatteryCap:
            {
                pd_battery_cap_data_block_t *partnerBatteryCap =
                    (pd_battery_cap_data_block_t *)(successData->dataBuffer);
                PRINTF("design cap:%dmWH, last full cap:%dmWH\r\n", partnerBatteryCap->batteryDesignCap * 100U,
                       partnerBatteryCap->batteryLastFullChargeCap * 100);
                break;
            }

            case kInfoType_BatteryStatus:
            {
                pd_battery_status_data_object_t *partnerBatteryStatus =
                    (pd_battery_status_data_object_t *)(successData->dataBuffer);
                PRINTF("battery charge state:%dmWH\r\n", partnerBatteryStatus->batteryPC * 100U);
                break;
            }

            case kInfoType_ManufacturerInfo:
            {
                pd_manufac_info_data_block_t *partnerManufacturerInfo =
                    (pd_manufac_info_data_block_t *)(successData->dataBuffer);
                partnerManufacturerInfo->manufacturerString[successData->dataLength - 4U] = 0U;
                PRINTF("manufacturer string:%s\r\n", partnerManufacturerInfo->manufacturerString);
                break;
            }

            default:
                break;
        }
    }
    else
    {
        PRINTF("get info ");
        switch (failResultType)
        {
            case kCommandResult_Error:
                PRINTF("fail\r\n");
                break;

            case kCommandResult_NotSupported:
                PRINTF("is not supported by partner\r\n");
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmReceiveAlertCallback(void *callbackParam, pd_command_data_param_t *dataParam)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    pd_alert_data_object_t alertObj;

    dataParam->resultStatus = kCommandResult_Accept;
    alertObj.alertValue     = USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(dataParam->dataBuffer);
    PRINTF("alert change:%x\r\n", alertObj.bitFields.typeOfAlert);

    PRINTF("start get status command\r\n");
    if (PD_Command(pdAppInstance->pdHandle, PD_DPM_GET_STATUS, NULL) != kStatus_PD_Success)
    {
        PRINTF("command start fail\r\n");
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSendAlertCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    PRINTF("send alert result: ");
    if (success)
    {
        PRINTF("success\r\n");
    }
    else
    {
        switch (failResultType)
        {
            case kCommandResult_Error:
                PRINTF("fail\r\n");
                break;

            case kCommandResult_NotSupported:
                PRINTF("not supported\r\n");
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}
#endif
