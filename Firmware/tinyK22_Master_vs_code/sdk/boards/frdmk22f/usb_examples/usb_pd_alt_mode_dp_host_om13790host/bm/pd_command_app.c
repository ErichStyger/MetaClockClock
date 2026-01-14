/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
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
/* The follow MACROs are for passing compliance test, it is not actual product logical */
#define PD_STATUS_DATA_SIZE            (6)
#define PD_BATTERY_CAP_DATA_SIZE       (9)
#define PD_BATTERY_STATUS_DATA_SIZE    (4)
#define PD_MANUFACTURER_INFO_DATA_SIZE (4)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

extern pd_status_t PD_DemoFindPDO(
    pd_app_t *pdAppInstance, pd_rdo_t *rdo, uint32_t requestVoltagemV, uint32_t requestCurrentmA, uint32_t *voltage);

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
    /* reset state, The Sink shall not draw more than iSafe0mA when VBUS is driven to vSafe0V. */
    PRINTF("hard reset request\r\n");
    return kStatus_PD_Success;
}

pd_status_t PD_DpmPowerRoleSwapRequestCallback(void *callbackParam, uint8_t frSwap, uint8_t *evaluateResult)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
#if (defined PD_CONFIG_COMPLIANCE_TEST_ENABLE) && (PD_CONFIG_COMPLIANCE_TEST_ENABLE)
    uint8_t powerRole;
    pd_source_pdo_t pdo;

    if (frSwap)
    {
        *evaluateResult = kCommandResult_Accept;
    }
    else
    {
        PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &powerRole);

        pdo.PDOValue = ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCaps[0];
        if ((powerRole == kPD_PowerRoleSource) && (pdo.fixedPDO.externalPowered))
        {
            if ((pdAppInstance->partnerSourceCapNumber == 0) && (pdAppInstance->partnerSinkCapNumber == 0))
            {
                *evaluateResult = kCommandResult_Wait;
                PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_GET_PARTNER_SOURCE_CAPABILITIES, NULL);
            }
            else
            {
                if (pdAppInstance->partnerSourceCapNumber)
                {
                    if (pdAppInstance->partnerSourceCaps[0].fixedPDO.externalPowered == 0)
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
                    if (pdAppInstance->partnerSinkCaps[0].fixedPDO.externalPowered == 0)
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
    *evaluateResult = ((pdAppInstance->prSwapAccept) ? kCommandResult_Accept : kCommandResult_Reject);
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
        if (roleInfo == kPD_PowerRoleSource)
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
                PRINTF("power role swap result: reject\r\n");
                break;

            case kCommandResult_Wait:
                PRINTF("power role swap result: wait\r\n");
                break;

            case kCommandResult_Error:
                PRINTF("power role swap result: fail\r\n");
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
    uint8_t accept          = 0;

    /* prepare for power supply, The power supply shall be ready to operate at the new power level within
     * tSrcReady */
    if (rdo.bitFields.objectPosition <=
        ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount)
    {
        pd_source_pdo_t sourcePDO;
        sourcePDO.PDOValue = ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)
                                 ->sourceCaps[rdo.bitFields.objectPosition - 1];
        accept = 1;
        switch (sourcePDO.commonPDO.pdoType)
        {
            case kPDO_Fixed:
            {
                if (rdo.bitFields.operateValue > sourcePDO.fixedPDO.maxCurrent)
                {
                    accept = 0;
                }
                break;
            }

            case kPDO_Variable:
            {
                if (rdo.bitFields.operateValue > sourcePDO.variablePDO.maxCurrent)
                {
                    accept = 0;
                }
                break;
            }

            case kPDO_Battery:
            {
                if (rdo.bitFields.operateValue > sourcePDO.batteryPDO.maxAllowPower)
                {
                    accept = 0;
                }
                break;
            }

            default:
                break;
        }
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
               (pdAppInstance->sinkRequestRDO.bitFields.objectPosition == 1) ? 5 : 9);
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
    for (index = 0; index < pdAppInstance->partnerSourceCapNumber; ++index)
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
    uint32_t voltage        = 0;

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
    for (index = 0; index < pdAppInstance->partnerSinkCapNumber; ++index)
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
    switch (failResultType)
    {
        case kCommandResult_Error:
            PRINTF("get snk cap fail");
            break;

        case kCommandResult_NotSupported:
            PRINTF("get snk cap replying not supported");
            break;

        default:
            break;
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
            if ((getManufacInfo->manufacturerInfoRef != 0U) || (getManufacInfo->manufacturerInfoTarget > 1))
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
                pd_source_cap_ext_data_block_t *extDataBlock;
                extDataBlock = (pd_source_cap_ext_data_block_t *)successData->dataBuffer;
                PRINTF("vid:%x, pid:%x, xid:%x, fw version:%d, hw version:%d\r\n", (uint16_t)extDataBlock->vid,
                       (uint16_t)extDataBlock->pid,
                       USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)&(extDataBlock->xid))),
                       (uint8_t)extDataBlock->fwVersion, (uint8_t)extDataBlock->hwVersion);
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
                PRINTF("design cap:%dmWH, last full cap:%dmWH\r\n", partnerBatteryCap->batteryDesignCap * 100,
                       partnerBatteryCap->batteryLastFullChargeCap * 100);
                break;
            }

            case kInfoType_BatteryStatus:
            {
                pd_battery_status_data_object_t *partnerBatteryStatus =
                    (pd_battery_status_data_object_t *)(successData->dataBuffer);
                PRINTF("battery charge state:%dmWH\r\n", partnerBatteryStatus->batteryPC * 100);
                break;
            }

            case kInfoType_ManufacturerInfo:
            {
                pd_manufac_info_data_block_t *partnerManufacturerInfo =
                    (pd_manufac_info_data_block_t *)(successData->dataBuffer);
                partnerManufacturerInfo->manufacturerString[successData->dataLength - 4] = 0;
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
