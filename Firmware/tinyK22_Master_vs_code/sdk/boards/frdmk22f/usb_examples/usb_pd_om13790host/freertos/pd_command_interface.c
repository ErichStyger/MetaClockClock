/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_i2c.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "pd_app.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pd_power_interface.h"
#include "pd_command_interface.h"
#include "board.h"

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

pd_status_t PD_DpmAppCommandCallback(void *callbackParam, uint32_t event, void *param)
{
    pd_status_t status = kStatus_PD_Error;

    switch (event)
    {
        /* hard reset */
        case PD_DPM_SNK_HARD_RESET_REQUEST:
        case PD_DPM_SRC_HARD_RESET_REQUEST:
            status = PD_DpmHardResetCallback(callbackParam);
            break;

        /* pr_swap */
        /* fast role swap */
        case PD_DPM_PR_SWAP_REQUEST:
        case PD_DPM_FR_SWAP_REQUEST:
            status =
                PD_DpmPowerRoleSwapRequestCallback(callbackParam, (event == PD_DPM_FR_SWAP_REQUEST), (uint8_t *)param);
            break;
        case PD_DPM_PR_SWAP_SUCCESS:
        case PD_DPM_FR_SWAP_SUCCESS:
            status = PD_DpmPowerRoleSwapResultCallback(callbackParam, 1, 0);
            break;
        case PD_DPM_PR_SWAP_FAIL:
        case PD_DPM_FR_SWAP_FAIL:
            status = PD_DpmPowerRoleSwapResultCallback(callbackParam, 0, *((uint8_t *)param));
            break;

        /* dr swap */
        case PD_DPM_DR_SWAP_REQUEST:
            status = PD_DpmDataRoleSwapRequestCallback(callbackParam, (uint8_t *)param);
            break;
        case PD_DPM_DR_SWAP_SUCCESS:
            status = PD_DpmDataRoleSwapResultCallback(callbackParam, 1, 0);
            break;
        case PD_DPM_DR_SWAP_FAIL:
            status = PD_DpmDataRoleSwapResultCallback(callbackParam, 0, *((uint8_t *)param));
            break;

        /* vconn swap */
        case PD_DPM_VCONN_SWAP_REQUEST:
            status = PD_DpmVconnSwapRequestCallback(callbackParam, (uint8_t *)param);
            break;
        case PD_DPM_VCONN_SWAP_SUCCESS:
            status = PD_DpmVconnSwapResultCallback(callbackParam, 1, 0);
            break;
        case PD_DPM_VCONN_SWAP_FAIL:
            status = PD_DpmVconnSwapResultCallback(callbackParam, 0, *((uint8_t *)param));
            break;

        /* rdo request and power negotiation */
        case PD_DPM_SRC_RDO_REQUEST:
            status = PD_DpmSrcRDORequestCallback(callbackParam, ((pd_negotiate_power_request_t *)param)->rdo,
                                                 &(((pd_negotiate_power_request_t *)param)->negotiateResult));
            break;
        case PD_DPM_SRC_CONTRACT_STILL_VALID:
            status = PD_DpmSrcPreContractStillValidCallback(callbackParam, (uint8_t *)param);
            break;
        case PD_DPM_SRC_SEND_SRC_CAP_FAIL:
            status = PD_DpmSrcRDOResultCallback(callbackParam, 0, kCommandResult_Error);
            break;
        case PD_DPM_SRC_RDO_SUCCESS:
            status = PD_DpmSrcRDOResultCallback(callbackParam, 1, 0);
            break;
        case PD_DPM_SRC_RDO_FAIL:
            status = PD_DpmSrcRDOResultCallback(callbackParam, 0, *((uint8_t *)param));
            break;
        case PD_DPM_SNK_RECEIVE_PARTNER_SRC_CAP:
            status = PD_DpmReceivePartnerSrcCapsCallback(callbackParam, (pd_capabilities_t *)param);
            break;
        case PD_DPM_SNK_GET_RDO:
            status = PD_DpmSnkGetRequestRDOCallback(callbackParam, (pd_rdo_t *)param);
            break;
        case PD_DPM_SNK_RDO_SUCCESS:
            status = PD_DpmSnkRDOResultCallback(callbackParam, 1, *((uint32_t *)param));
            break;
        case PD_DPM_SNK_RDO_FAIL:
            status = PD_DpmSnkRDOResultCallback(callbackParam, 0, *((uint8_t *)param));
            break;

        /* get source cap */
        case PD_DPM_GET_PARTNER_SRC_CAP_FAIL:
            status = PD_DpmGetPartnerSrcCapsFailCallback(callbackParam, *((uint8_t *)param));
            break;
        case PD_DPM_GET_PARTNER_SRC_CAP_SUCCESS:
            status = PD_DpmReceivePartnerSrcCapsCallback(callbackParam, (pd_capabilities_t *)param);
            break;

        /* goto min */
        case PD_DPM_SRC_GOTOMIN_FAIL:
            status = PD_DpmSrcGotoMinResultCallback(callbackParam, 0, *((uint8_t *)param));
            break;
        case PD_DPM_SRC_GOTOMIN_SUCCESS:
            status = PD_DpmSrcGotoMinResultCallback(callbackParam, 1, 0);
            break;
        case PD_DPM_SNK_GOTOMIN_SUCCESS:
            status = PD_DpmSnkGotoMinResultCallback(callbackParam, 1, 0);
            break;
        case PD_DPM_SNK_GOTOMIN_FAIL:
            status = PD_DpmSnkGotoMinResultCallback(callbackParam, 0, *((uint8_t *)param));
            break;

        /* get sink cap */
        case PD_DPM_GET_PARTNER_SNK_CAP_SUCCESS:
            status = PD_DpmReceivePartnerSnkCapsCallback(callbackParam, (pd_capabilities_t *)param);
            break;
        case PD_DPM_GET_PARTNER_SNK_CAP_FAIL:
            status = PD_DpmGetPartnerSnkCapsFailCallback(callbackParam, *((uint8_t *)param));
            break;

        /* soft reset */
        case PD_DPM_SOFT_RESET_SUCCESS:
        case PD_DPM_SOFT_RESET_REQUEST:
            status = PD_DpmSoftResetCallback(callbackParam);
            break;
        case PD_DPM_SOFT_RESET_FAIL:
            /* in normal situation, app don't need process it PD stack will do hard_reset if soft_reset fail. */
            break;

        /* structured vdm */
        case PD_DPM_STRUCTURED_VDM_REQUEST:
            status = PD_DpmSVDMRequestCallback(callbackParam, (pd_svdm_command_request_t *)param);
            break;
        case PD_DPM_STRUCTURED_VDM_SUCCESS:
            status = PD_DpmSVDMResultCallback(callbackParam, 1, (pd_svdm_command_result_t *)param);
            break;
        case PD_DPM_STRUCTURED_VDM_FAIL:
            status = PD_DpmSVDMResultCallback(callbackParam, 0, (pd_svdm_command_result_t *)param);
            break;

        /* unstructured vdm */
        case PD_DPM_UNSTRUCTURED_VDM_RECEIVED:
            status = PD_DpmUnstructuredVDMReceivedCallback(callbackParam, (pd_unstructured_vdm_command_param_t *)param);
            break;
        case PD_DPM_SEND_UNSTRUCTURED_VDM_SUCCESS:
            status = PD_DpmUnstructuredVDMSendResultCallback(callbackParam, 1, 0);
            break;
        case PD_DPM_SEND_UNSTRUCTURED_VDM_FAIL:
            status = PD_DpmUnstructuredVDMSendResultCallback(callbackParam, 0, *((uint8_t *)param));
            break;

#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
        /* get source extended cap */
        case PD_DPM_GIVE_SRC_EXT_CAP:
            status = PD_DpmGetInfoRequestCallback(callbackParam, kInfoType_SrcExtCap, (pd_command_data_param_t *)param);
            break;
        case PD_DPM_GET_SRC_EXT_CAP_SUCCESS:
            status =
                PD_DpmGetInfoResultCallback(callbackParam, kInfoType_SrcExtCap, 1, (pd_command_data_param_t *)param, 0);
            break;
        case PD_DPM_GET_SRC_EXT_CAP_FAIL:
            status = PD_DpmGetInfoResultCallback(callbackParam, kInfoType_SrcExtCap, 0, NULL, *((uint8_t *)param));
            break;

        /* get status */
        case PD_DPM_GIVE_STATUS:
            status = PD_DpmGetInfoRequestCallback(callbackParam, kInfoType_Status, (pd_command_data_param_t *)param);
            break;
        case PD_DPM_GET_STATUS_SUCCESS:
            status =
                PD_DpmGetInfoResultCallback(callbackParam, kInfoType_Status, 1, (pd_command_data_param_t *)param, 0);
            break;
        case PD_DPM_GET_STATUS_FAIL:
            status = PD_DpmGetInfoResultCallback(callbackParam, kInfoType_Status, 0, NULL, *((uint8_t *)param));
            break;

        /* get battery cap */
        case PD_DPM_GIVE_BATTERY_CAP:
            status =
                PD_DpmGetInfoRequestCallback(callbackParam, kInfoType_BatteryCap, (pd_command_data_param_t *)param);
            break;
        case PD_DPM_GET_BATTERY_CAP_SUCCESS:
            status = PD_DpmGetInfoResultCallback(callbackParam, kInfoType_BatteryCap, 1,
                                                 (pd_command_data_param_t *)param, 0);
            break;
        case PD_DPM_GET_BATTERY_CAP_FAIL:
            status = PD_DpmGetInfoResultCallback(callbackParam, kInfoType_BatteryCap, 0, NULL, *((uint8_t *)param));
            break;

        /* get battery status */
        case PD_DPM_GIVE_BATTERY_STATUS:
            status =
                PD_DpmGetInfoRequestCallback(callbackParam, kInfoType_BatteryStatus, (pd_command_data_param_t *)param);
            break;
        case PD_DPM_GET_BATTERY_STATUS_SUCCESS:
            status = PD_DpmGetInfoResultCallback(callbackParam, kInfoType_BatteryStatus, 1,
                                                 (pd_command_data_param_t *)param, 0);
            break;
        case PD_DPM_GET_BATTERY_STATUS_FAIL:
            status = PD_DpmGetInfoResultCallback(callbackParam, kInfoType_BatteryStatus, 0, NULL, *((uint8_t *)param));
            break;

        /* get manufacturer info */
        case PD_DPM_GIVE_MANUFACTURER_INFO:
            status = PD_DpmGetInfoRequestCallback(callbackParam, kInfoType_ManufacturerInfo,
                                                  (pd_command_data_param_t *)param);
            break;
        case PD_DPM_GET_MANUFACTURER_INFO_SUCCESS:
            status = PD_DpmGetInfoResultCallback(callbackParam, kInfoType_ManufacturerInfo, 1,
                                                 (pd_command_data_param_t *)param, 0);
            break;
        case PD_DPM_GET_MANUFACTURER_INFO_FAIL:
            status =
                PD_DpmGetInfoResultCallback(callbackParam, kInfoType_ManufacturerInfo, 0, NULL, *((uint8_t *)param));
            break;

        /* alert */
        case PD_DPM_ALERT_RECEIVED:
            status = PD_DpmReceiveAlertCallback(callbackParam, (pd_command_data_param_t *)param);
            break;
        case PD_DPM_SEND_ALERT_SUCCESS:
            status = PD_DpmSendAlertCallback(callbackParam, 1, 0);
            break;
        case PD_DPM_SEND_ALERT_FAIL:
            status = PD_DpmSendAlertCallback(callbackParam, 0, *((uint8_t *)param));
            break;
#endif

#if 0
        /* cable reset */
        case PD_DPM_CABLE_RESET_REQUEST:
            status = PD_DpmCableResetRquestCallback(callbackParam, 1);
            break;
        case PD_DPM_CABLE_RESET_COMPLETE:
            status = PD_DpmCableResetRquestCallback(callbackParam, 0);
            break;

        /* security request */
        case PD_DPM_RESPONSE_SECURITY_REQUEST:
        case PD_DPM_SECURITY_REQUEST_SUCCESS:
        case PD_DPM_SECURITY_RESPONSE_RECEIVED:
        case PD_DPM_SECURITY_REQUEST_FAIL:
            status = PD_DpmSecurityRequestCallback(callbackParam, event, param);
            break;
#endif

        default:
            break;
    }
    return status;
}
