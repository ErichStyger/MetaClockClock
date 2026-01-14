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
        case PD_DPM_SNK_GOTOMIN_SUCCESS:
            status = PD_DpmSnkGotoMinResultCallback(callbackParam, 1, 0);
            break;
        case PD_DPM_SNK_GOTOMIN_FAIL:
            status = PD_DpmSnkGotoMinResultCallback(callbackParam, 0, *((uint8_t *)param));
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

#if 0
        case PD_DPM_STRUCTURED_VDM_SUCCESS:
            status = PD_DpmSVDMResultCallback(callbackParam, 1, (pd_svdm_command_result_t *)param);
            break;
        case PD_DPM_STRUCTURED_VDM_FAIL:
            status = PD_DpmSVDMResultCallback(callbackParam, 0, (pd_svdm_command_result_t *)param);
            break;

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
