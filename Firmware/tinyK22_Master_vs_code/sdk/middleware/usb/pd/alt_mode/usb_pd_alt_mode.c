/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_phy.h"
#include "usb_pd_interface.h"
#include "usb_pd_alt_mode.h"
#if (defined PD_CONFIG_ALT_MODE_DP_SUPPORT) && (PD_CONFIG_ALT_MODE_DP_SUPPORT)
#include "usb_pd_alt_mode_dp.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if (defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT)

#define PD_ALT_MODE_MSG_Q_ITEM_COUNT (8)
#define PD_ALT_MODE_MSG_Q_ITEM_SIZE  (sizeof(pd_alt_mode_q_item_t))

typedef struct pd_alt_mode_module_fun
{
    uint16_t SVID;
    pd_status_t (*pd_alt_mode_init)(pd_handle pdHandle,
                                    void *altModeHandle,
                                    const void *moduleConfig,
                                    void **moduleInstance);
    pd_status_t (*pd_alt_mode_deinit)(void *moduleInstance);
    pd_status_t (*pd_alt_mode_callback_event)(void *moduleInstance,
                                              uint32_t processCode,
                                              uint16_t msgSVID,
                                              void *param);
    pd_status_t (*pd_alt_mode_control)(void *moduleInstance, uint32_t controlCode, void *controlParam);
    void (*pd_alt_mode_task)(void *taskParam);
    void (*pd_alt_mode_1ms_isr)(void *moduleInstance);
} pd_alt_mode_module_interface_t;

typedef struct _pd_dpm_alt_mode
{
    pd_handle pdHandle;
    const pd_alt_mode_config_t *altModeConfig;
    void *altModeModuleInstance[PD_ALT_MODE_MAX_MODULES];
    pd_alt_mode_module_interface_t *modulesInterfaces[PD_ALT_MODE_MAX_MODULES];
    volatile uint32_t delayTime; /* retry delay */
    uint32_t dataBuff[7];
    pd_command_t dfpCommand;            /* the processing command */
    volatile pd_command_t retryCommand; /* will do this command after delay */
    volatile uint8_t retryCount;        /* how many time one command has been retried */

    volatile uint8_t occupied;
#if (defined PD_CONFIG_ALT_MODE_HOST_SUPPORT) && (PD_CONFIG_ALT_MODE_HOST_SUPPORT)
    volatile uint8_t altModeStart;
#endif
} pd_alt_mode_t;

typedef struct _pd_alt_mode_q_item
{
    void *altModeInstanceHandle;
    uint32_t event;
} pd_alt_mode_q_item_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static pd_alt_mode_t s_AltModeInstances[PD_CONFIG_MAX_PORT];
static OSA_MSGQ_HANDLE_DEFINE(s_AltModeMsgQ, PD_ALT_MODE_MSG_Q_ITEM_COUNT, PD_ALT_MODE_MSG_Q_ITEM_SIZE);
static pd_alt_mode_module_interface_t s_ModuleDPInterfaces[] = {
#if (defined PD_CONFIG_ALT_MODE_DP_SUPPORT) && (PD_CONFIG_ALT_MODE_DP_SUPPORT)
    {DP_SVID, PD_DPInit, PD_DPDeinit, PD_DPCallbackEvent, PD_DPControl, PD_DPTask, PD_DPModule1msISR}
#endif
};

/*******************************************************************************
 * Code
 ******************************************************************************/

static void PD_AltModeGetModuleInterface(pd_alt_mode_t *altModeInstance)
{
    uint8_t moduleIndex;
    uint8_t interfaceIndex;

    for (moduleIndex = 0; moduleIndex < sizeof(s_ModuleDPInterfaces) / sizeof(pd_alt_mode_module_interface_t);
         ++moduleIndex)
    {
        altModeInstance->modulesInterfaces[moduleIndex] = NULL;
        for (interfaceIndex = 0; interfaceIndex < sizeof(s_ModuleDPInterfaces) / sizeof(pd_alt_mode_module_interface_t);
             ++interfaceIndex)
        {
            if (altModeInstance->altModeConfig->modules[moduleIndex].SVID == s_ModuleDPInterfaces[interfaceIndex].SVID)
            {
                altModeInstance->modulesInterfaces[moduleIndex] = &s_ModuleDPInterfaces[interfaceIndex];
                break;
            }
        }
    }
}

static void PD_AltModeSetMsg(pd_alt_mode_t *altModeInstance, uint32_t events)
{
    pd_alt_mode_q_item_t qItem;

    qItem.altModeInstanceHandle = altModeInstance;
    qItem.event                 = events;
    (void)OSA_MsgQPut((osa_msgq_handle_t)&s_AltModeMsgQ[0], &qItem);
}

static osa_status_t PD_AltModeGetMsg(pd_alt_mode_q_item_t *qItem)
{
    return OSA_MsgQGet((osa_msgq_handle_t)&s_AltModeMsgQ[0], qItem, osaWaitForever_c);
}

static void PD_AltModeDelayRetryCommand(pd_alt_mode_t *altModeInstance, pd_command_t command, uint32_t delay)
{
    altModeInstance->delayTime    = delay;
    altModeInstance->retryCommand = command;
}

static void PD_AltModeTrigerCommand(pd_alt_mode_t *altModeInstance, pd_command_t command)
{
    altModeInstance->dfpCommand = command;
    PD_AltModeSetMsg(altModeInstance, PD_ALT_MODE_EVENT_COMMAND);
}

static pd_status_t PD_AltModeSendCommand(pd_alt_mode_t *altModeInstance, pd_command_t command)
{
    pd_svdm_command_param_t structuredVDMCommandParam;
    pd_command_t vdmCommand = PD_DPM_INVALID;
    void *param             = NULL;

    structuredVDMCommandParam.vdmSop                          = (uint8_t)kPD_MsgSOP;
    structuredVDMCommandParam.vdmHeader.bitFields.SVID        = 0xFF00u;
    structuredVDMCommandParam.vdmHeader.bitFields.vdmType     = 1;
    structuredVDMCommandParam.vdmHeader.bitFields.objPos      = 0;
    structuredVDMCommandParam.vdmHeader.bitFields.commandType = (uint8_t)kVDM_Initiator;

    switch (command)
    {
        case PD_DPM_CONTROL_DISCOVERY_IDENTITY:
            structuredVDMCommandParam.vdoCount                    = 0;
            structuredVDMCommandParam.vdoData                     = NULL;
            structuredVDMCommandParam.vdmHeader.bitFields.command = (uint8_t)kVDM_DiscoverIdentity;
            vdmCommand                                            = PD_DPM_CONTROL_DISCOVERY_IDENTITY;
            param                                                 = &structuredVDMCommandParam;
            break;

        case PD_DPM_CONTROL_DISCOVERY_SVIDS:
            structuredVDMCommandParam.vdoCount                    = 0;
            structuredVDMCommandParam.vdoData                     = NULL;
            structuredVDMCommandParam.vdmHeader.bitFields.command = (uint8_t)kVDM_DiscoverSVIDs;
            vdmCommand                                            = PD_DPM_CONTROL_DISCOVERY_SVIDS;
            param                                                 = &structuredVDMCommandParam;
            break;

        case PD_DPM_CONTROL_DR_SWAP:
        {
            pd_data_role_t dataRole;

            (void)PD_Control(altModeInstance->pdHandle, (uint32_t)PD_CONTROL_GET_DATA_ROLE, &dataRole);
            if (((altModeInstance->altModeConfig->altModeRole == (uint8_t)kDataConfig_DFP) ||
                 (altModeInstance->altModeConfig->altModeRole == (uint8_t)kDataConfig_DRD)) &&
                (dataRole == kPD_DataRoleUFP))
            {
                vdmCommand = PD_DPM_CONTROL_DR_SWAP;
            }
            else
            {
                PD_AltModeTrigerCommand(altModeInstance, PD_DPM_CONTROL_DISCOVERY_IDENTITY);
            }
            param = NULL;
            break;
        }

        default:
            /* No action required. */
            break;
    }

    if (vdmCommand != PD_DPM_INVALID)
    {
        if (PD_Command(altModeInstance->pdHandle, (uint32_t)vdmCommand, param) != kStatus_PD_Success)
        {
            /* wait and retry again */
            PD_AltModeDelayRetryCommand(altModeInstance, vdmCommand, PD_ALT_MODE_ERROR_RETRY_WAIT_TIME);
        }
    }

    return kStatus_PD_Success;
}

static void PD_AltModeInstanceReset(pd_alt_mode_t *altModeInstance)
{
#if (defined PD_CONFIG_ALT_MODE_HOST_SUPPORT) && (PD_CONFIG_ALT_MODE_HOST_SUPPORT)
    altModeInstance->dfpCommand   = PD_DPM_INVALID;
    altModeInstance->altModeStart = 0;
#endif
    altModeInstance->retryCommand = PD_DPM_INVALID;
}

#if (defined PD_CONFIG_ALT_MODE_HOST_SUPPORT) && (PD_CONFIG_ALT_MODE_HOST_SUPPORT)
static pd_status_t PD_AltModeEnter(pd_alt_mode_t *altModeInstance)
{
    pd_data_role_t dataRole;

    if ((altModeInstance->altModeConfig->altModeRole == (uint8_t)kDataConfig_DFP) ||
        (altModeInstance->altModeConfig->altModeRole == (uint8_t)kDataConfig_DRD))
    {
        altModeInstance->retryCommand = PD_DPM_INVALID;
        (void)PD_Control(altModeInstance->pdHandle, (uint32_t)PD_CONTROL_GET_DATA_ROLE, &dataRole);
        if (dataRole == kPD_DataRoleDFP)
        {
            /* start discover */
            PD_AltModeTrigerCommand(altModeInstance, PD_DPM_CONTROL_DISCOVERY_IDENTITY);
        }
    }
    else
    {
        /* do nothing */
    }

    return kStatus_PD_Success;
}
#endif

void PD_AltModeModuleTaskWakeUp(pd_alt_mode_handle altModeHandle, void *moduleHandle)
{
    uint32_t index;
    pd_alt_mode_t *altModeInstance = (pd_alt_mode_t *)altModeHandle;

    if ((altModeHandle == NULL) || (moduleHandle == NULL))
    {
        return;
    }

    for (index = 0; index < altModeInstance->altModeConfig->moduleCount; ++index)
    {
        if (altModeInstance->altModeModuleInstance[index] == moduleHandle)
        {
            PD_AltModeSetMsg(altModeInstance, (0x00000001UL << index));
        }
    }
}

void PD_AltModeTimer1msISR(void)
{
    pd_alt_mode_t *altModeInstance = NULL;
    uint8_t index;
    uint32_t index32;

    for (index = 0; index < sizeof(s_AltModeInstances) / sizeof(pd_alt_mode_t); ++index)
    {
        if (s_AltModeInstances[index].occupied != 0U)
        {
            altModeInstance = (pd_alt_mode_t *)&s_AltModeInstances[index];
            if (altModeInstance->delayTime > 0U)
            {
                altModeInstance->delayTime--;
                if (altModeInstance->delayTime == 0U)
                {
                    PD_AltModeTrigerCommand(altModeInstance, altModeInstance->retryCommand);
                }
            }

            /* modules' 1ms ISR */
            for (index32 = 0; index32 < altModeInstance->altModeConfig->moduleCount; ++index32)
            {
                if ((altModeInstance->altModeModuleInstance[index32] != NULL) &&
                    (altModeInstance->modulesInterfaces[index32]->pd_alt_mode_1ms_isr != NULL))
                {
                    altModeInstance->modulesInterfaces[index32]->pd_alt_mode_1ms_isr(
                        altModeInstance->altModeModuleInstance[index32]);
                }
            }
        }
    }
}

pd_status_t PD_AltModeInit(void)
{
    if (OSA_MsgQCreate((osa_msgq_handle_t)&s_AltModeMsgQ[0], PD_ALT_MODE_MSG_Q_ITEM_COUNT,
                       PD_ALT_MODE_MSG_Q_ITEM_SIZE) == KOSA_StatusSuccess)
    {
        return kStatus_PD_Success;
    }

    return kStatus_PD_Error;
}

pd_status_t PD_AltModeDeinit(void)
{
    uint8_t index;

    for (index = 0; index < sizeof(s_AltModeInstances) / sizeof(pd_alt_mode_t); ++index)
    {
        if (s_AltModeInstances[index].occupied == 1U)
        {
            return kStatus_PD_Error;
        }
    }

    (void)OSA_MsgQDestroy((osa_msgq_handle_t)&s_AltModeMsgQ[0]);

    return kStatus_PD_Success;
}

pd_status_t PD_AltModeInstanceInit(pd_handle pdHandle,
                                   const pd_alt_mode_config_t *altModeConfig,
                                   pd_alt_mode_handle *altModeHandle)
{
    pd_alt_mode_t *altModeInstance = NULL;
    pd_status_t status             = kStatus_PD_Success;
    uint32_t index;
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    for (index = 0; index < sizeof(s_AltModeInstances) / sizeof(pd_alt_mode_t); ++index)
    {
        if (s_AltModeInstances[index].occupied == 0U)
        {
            s_AltModeInstances[index].occupied = 1;
            altModeInstance                    = &s_AltModeInstances[index];
            break;
        }
    }
    if (altModeInstance == NULL)
    {
        OSA_EXIT_CRITICAL();
        return kStatus_PD_Error;
    }
    OSA_EXIT_CRITICAL();

    altModeInstance->pdHandle      = pdHandle;
    altModeInstance->altModeConfig = altModeConfig;
    PD_AltModeInstanceReset(altModeInstance);
    PD_AltModeGetModuleInterface(altModeInstance);
    /* according SVID list, initialize every alt mode module. */
    for (index = 0; index < altModeConfig->moduleCount; ++index)
    {
        if (altModeInstance->modulesInterfaces[index] == NULL)
        {
            status = kStatus_PD_Error;
            break;
        }

        status = altModeInstance->modulesInterfaces[index]->pd_alt_mode_init(
            pdHandle, altModeInstance, altModeConfig->modules[index].config,
            &(altModeInstance->altModeModuleInstance[index]));
        if ((altModeInstance->altModeModuleInstance[index] == NULL) || (status != kStatus_PD_Success))
        {
            status = kStatus_PD_Error;
            break;
        }
    }

    if (status != kStatus_PD_Success)
    {
        for (index = 0; index < altModeConfig->moduleCount; ++index)
        {
            if (altModeInstance->altModeModuleInstance[index] != NULL)
            {
                (void)altModeInstance->modulesInterfaces[index]->pd_alt_mode_deinit(
                    altModeInstance->altModeModuleInstance[index]);
                altModeInstance->altModeModuleInstance[index] = NULL;
            }
        }
        s_AltModeInstances[index].occupied = 0;
        return status;
    }

    *altModeHandle = altModeInstance;
    return kStatus_PD_Success;
}

pd_status_t PD_AltModeInstanceDeinit(pd_alt_mode_handle altModeHandle)
{
    uint32_t index;
    pd_alt_mode_t *altModeInstance = (pd_alt_mode_t *)altModeHandle;

    if (altModeHandle == NULL)
    {
        return kStatus_PD_Error;
    }

    /* according SVID list, initialize every alt mode module. */
    for (index = 0; index < altModeInstance->altModeConfig->moduleCount; ++index)
    {
        if (altModeInstance->altModeModuleInstance[index] != NULL)
        {
            (void)altModeInstance->modulesInterfaces[index]->pd_alt_mode_deinit(
                altModeInstance->altModeModuleInstance[index]);
            altModeInstance->altModeModuleInstance[index] = NULL;
        }
    }

    altModeInstance->occupied = 0;
    return kStatus_PD_Success;
}

pd_status_t PD_AltModeState(pd_handle pdHandle, uint8_t *state)
{
    uint32_t index;
    pd_alt_mode_t *altModeInstance = NULL;
    pd_alt_mode_state_t enteredMode;

    if ((pdHandle == NULL) || (state == NULL))
    {
        return kStatus_PD_Error;
    }
    for (index = 0; index < sizeof(s_AltModeInstances) / sizeof(pd_alt_mode_t); ++index)
    {
        if ((s_AltModeInstances[index].occupied == 1U) && (s_AltModeInstances[index].pdHandle == pdHandle))
        {
            altModeInstance = &s_AltModeInstances[index];
            break;
        }
    }
    if (altModeInstance == NULL)
    {
        return kStatus_PD_Error;
    }

    for (index = 0; index < altModeInstance->altModeConfig->moduleCount; ++index)
    {
        if (altModeInstance->altModeModuleInstance[index] != NULL)
        {
            enteredMode.SVID = 0;
            (void)altModeInstance->modulesInterfaces[index]->pd_alt_mode_control(
                altModeInstance->altModeModuleInstance[index], (uint32_t)kAltMode_GetModeState, &enteredMode);
            if (enteredMode.SVID != 0U)
            {
                *state = 1;
                return kStatus_PD_Success;
            }
        }
    }

    return kStatus_PD_Success;
}

void PD_AltModeTask(void)
{
    pd_alt_mode_t *altModeInstance;
    pd_alt_mode_q_item_t qItem;
    uint32_t index;

    qItem.altModeInstanceHandle = NULL;
    qItem.event                 = 0;

    /* Alt Mode events process */
    if (PD_AltModeGetMsg(&qItem) == KOSA_StatusSuccess)
    {
        altModeInstance = (pd_alt_mode_t *)qItem.altModeInstanceHandle;
        if ((qItem.altModeInstanceHandle == NULL) || (qItem.event == 0U))
        {
            return;
        }
        if ((qItem.event & PD_ALT_MODE_EVENT_COMMAND) != 0U)
        {
            if (altModeInstance->retryCommand != altModeInstance->dfpCommand)
            {
                altModeInstance->retryCount   = PD_ALT_MODE_COMMAND_RETRY_COUNT;
                altModeInstance->retryCommand = altModeInstance->dfpCommand;
                (void)PD_AltModeSendCommand(altModeInstance, altModeInstance->dfpCommand);
            }
            else
            {
                if (altModeInstance->retryCount > 0U)
                {
                    altModeInstance->retryCount--;
                    (void)PD_AltModeSendCommand(altModeInstance, altModeInstance->dfpCommand);
                }
                else
                {
                    /* do hard reset */
                    /* PD_Command(altModeInstance->pdHandle, (uint32_t)PD_DPM_CONTROL_HARD_RESET, NULL); */
                }
            }
        }
        else
        {
            /* No action required. */
        }

        if ((qItem.event & PD_ALT_MODE_EVENT_MODULES) != 0U)
        {
            for (index = 0; index < altModeInstance->altModeConfig->moduleCount; ++index)
            {
                if ((qItem.event & (0x00000001UL << index)) != 0U)
                {
                    altModeInstance->modulesInterfaces[index]->pd_alt_mode_task(
                        altModeInstance->altModeModuleInstance[index]);
                }
            }
        }
    }
}

static pd_status_t PD_AltModeStandardVDMCallbackProcess(pd_alt_mode_t *altModeInstance, uint32_t event, void *param)
{
    pd_svdm_command_request_t *svdmRequest = (pd_svdm_command_request_t *)param;
    pd_status_t status                     = kStatus_PD_Error;

    switch ((pd_dpm_callback_event_t)event)
    {
        case PD_DPM_STRUCTURED_VDM_REQUEST:
        {
            /* ack or nak, no busy */
            /* partner return nak if it is not in the alternate mode */
            switch ((pd_vdm_command_t)svdmRequest->vdmHeader.bitFields.command)
            {
                case kVDM_DiscoverIdentity: /* DFP or UFP */
                {
                    svdmRequest->vdoData             = altModeInstance->altModeConfig->identityData;
                    svdmRequest->vdoCount            = altModeInstance->altModeConfig->identityObjectCount;
                    svdmRequest->requestResultStatus = (uint8_t)kCommandResult_VDMACK;
                    break;
                }

                case kVDM_DiscoverSVIDs: /* DFP or UFP */
                {
                    uint32_t index;

                    if (PD_AltModeCheckModalOperationSupported((void *)altModeInstance) != 0U)
                    {
                        for (index = 0; index < altModeInstance->altModeConfig->moduleCount; ++index)
                        {
                            uint32_t svidsObj = 0;
                            if ((index & 0x01U) != 0U)
                            {
                                svidsObj = altModeInstance->dataBuff[index / 2U];
                            }
                            if ((index & 0x01U) == 0U)
                            {
                                svidsObj &= 0x0000FFFFU;
                                svidsObj |= ((uint32_t)altModeInstance->altModeConfig->modules[index].SVID << 16U);
                            }
                            else
                            {
                                svidsObj &= 0xFFFF0000U;
                                svidsObj |= altModeInstance->altModeConfig->modules[index].SVID;
                            }
                            altModeInstance->dataBuff[index / 2U] = svidsObj;
                        }
                        svdmRequest->vdoData             = altModeInstance->dataBuff;
                        svdmRequest->vdoCount            = ((uint8_t)(index + 1U) >> 1U);
                        svdmRequest->requestResultStatus = (uint8_t)kCommandResult_VDMACK;
                    }
                    else
                    {
                        svdmRequest->requestResultStatus = (uint8_t)kCommandResult_VDMNAK;
                        svdmRequest->vdoData             = NULL;
                        svdmRequest->vdoCount            = 0;
                    }
                    break;
                }
                case kVDM_DiscoverModes: /* DFP or UFP (module self process) */
                case kVDM_EnterMode:     /* UFP (module self process) */
                case kVDM_ExitMode:      /* UFP (module self process) */
                case kVDM_Attention:     /* DFP (module self process) */
                    /* PD VID NAK */
                    svdmRequest->vdoData             = NULL;
                    svdmRequest->vdoCount            = 0;
                    svdmRequest->requestResultStatus = (uint8_t)kCommandResult_VDMNAK;
                    break;

                default:
                    /* No action required. */
                    break;
            }
            status = kStatus_PD_Success;
            break;
        }

        case PD_DPM_STRUCTURED_VDM_SUCCESS:
        {
#if (defined PD_CONFIG_ALT_MODE_HOST_SUPPORT) && (PD_CONFIG_ALT_MODE_HOST_SUPPORT)
            pd_svdm_command_result_t *svdmResult = (pd_svdm_command_result_t *)param;

            switch ((pd_vdm_command_t)svdmResult->vdmCommand)
            {
                case kVDM_DiscoverIdentity: /* ACK msg, DFP */
                    /* 1. TODO, receive identity */
                    /* 2. get SVIDs */
                    PD_AltModeTrigerCommand(altModeInstance, PD_DPM_CONTROL_DISCOVERY_SVIDS);
                    break;

                case kVDM_DiscoverSVIDs: /* ACK msg, DFP */
                {
                    uint32_t index;
                    uint16_t svid = 0;
                    /* what SVID to enter?  */
                    for (index = 0; index < altModeInstance->altModeConfig->moduleCount; ++index)
                    {
                        uint8_t index1;
                        for (index1 = 0; index1 < svdmResult->vdoCount * 2U; ++index1)
                        {
                            svid =
                                (uint16_t)(svdmResult->vdoData[index1 >> 1U] >> (((index1 & 0x01U) != 0U) ? 0U : 16U));
                            if (svid == altModeInstance->altModeConfig->modules[index].SVID)
                            {
                                break;
                            }
                        }
                        if (index1 < svdmResult->vdoCount * 2U)
                        {
                            break;
                        }
                    }
                    if (index < altModeInstance->altModeConfig->moduleCount)
                    {
                        (void)altModeInstance->modulesInterfaces[index]->pd_alt_mode_control(
                            altModeInstance->altModeModuleInstance[index], (uint32_t)kAltMode_TriggerEnterMode, NULL);
                    }
                    else
                    {
                        /* if SVIDs is not end, get next SVIDs */
                        /* last SVID */
                        if (svdmResult->vdoCount >= 1U)
                        {
                            svid = (uint16_t)svdmResult->vdoData[svdmResult->vdoCount - 1U];
                        }
                        else
                        {
                            break;
                        }
                        if (svid != 0x0000U)
                        {
                            /* get next SVIDs LIST */
                            PD_AltModeTrigerCommand(altModeInstance, PD_DPM_CONTROL_DISCOVERY_SVIDS);
                        }
                    }
                    break;
                }

                case kVDM_DiscoverModes: /* ACK msg, DFP */
                case kVDM_EnterMode:     /* ACK msg, DFP */
                case kVDM_ExitMode:      /* ACK msg, DFP */
                                         /* shoudn't go here becase don't do PD VID discover/enter/exit,
                                            they all dedicated module related */
                    ;
                    break;

                default:
                    /* No action required. */
                    break;
            }
#endif
            status = kStatus_PD_Success;
            break;
        }

        case PD_DPM_STRUCTURED_VDM_FAIL:
        {
            pd_svdm_command_result_t *svdmResult = (pd_svdm_command_result_t *)param;
            if (svdmResult->vdmCommandResult == (uint8_t)kCommandResult_VDMNAK)
            {
                /* don't support this command */
                return kStatus_PD_Success;
            }
            if (svdmResult->vdmCommand == (uint8_t)kVDM_DiscoverIdentity)
            {
                /* wait and retry again */
                PD_AltModeDelayRetryCommand(altModeInstance, PD_DPM_CONTROL_DISCOVERY_IDENTITY,
                                            PD_ALT_MODE_ERROR_RETRY_WAIT_TIME);
            }
            else if (svdmResult->vdmCommand == (uint8_t)kVDM_DiscoverSVIDs)
            {
                /* wait and retry again */
                PD_AltModeDelayRetryCommand(altModeInstance, PD_DPM_CONTROL_DISCOVERY_SVIDS,
                                            PD_ALT_MODE_ERROR_RETRY_WAIT_TIME);
            }
            else
            {
                /* No action required. */
            }
            status = kStatus_PD_Success;
            break;
        }

        default:
            /* No action required. */
            break;
    }

    return status;
}

static pd_status_t PD_AltModeNotifyModulesEvent(pd_alt_mode_t *altModeInstance,
                                                pd_alt_mode_callback_code_t eventCode,
                                                void *eventParam,
                                                uint16_t svid)
{
    pd_status_t status = kStatus_PD_Error;
    uint32_t index;

    if (eventCode != kAltMode_EventInvalid)
    {
        for (index = 0; index < altModeInstance->altModeConfig->moduleCount; ++index)
        {
            if (altModeInstance->altModeModuleInstance[index] != NULL)
            {
                if ((svid == altModeInstance->altModeConfig->modules[index].SVID) || (svid == 0U))
                {
                    status = altModeInstance->modulesInterfaces[index]->pd_alt_mode_callback_event(
                        altModeInstance->altModeModuleInstance[index], (uint32_t)eventCode, svid, eventParam);
                }
                else
                {
                    /* Not recognize the SVID, reply with a NAK command for structured VDM
                       or NotSupported for unstructured VDM */
                    if (eventCode == kAltMode_UnstructedVDMMsgReceived)
                    {
                        ((pd_unstructured_vdm_command_param_t *)eventParam)->resultStatus =
                            (uint8_t)kCommandResult_NotSupported;
                    }
                    else
                    {
                        pd_svdm_command_request_t *svdmRequest = (pd_svdm_command_request_t *)eventParam;

                        svdmRequest->requestResultStatus = (uint8_t)kCommandResult_VDMNAK;
                        svdmRequest->vdoData             = NULL;
                        svdmRequest->vdoCount            = 0;
                    }

                    status = kStatus_PD_Success;
                }
            }
        }
    }
    return status;
}

pd_status_t PD_AltModeCallback(pd_handle pdHandle, uint32_t event, void *param)
{
    /* if this event is processed, return kStatus_PD_Success */
    pd_status_t status                        = kStatus_PD_Error;
    uint32_t index                            = 0;
    pd_alt_mode_t *altModeInstance            = NULL;
    pd_alt_mode_callback_code_t callbackEvent = kAltMode_EventInvalid;
    void *controlParam                        = NULL;
    uint16_t msgSVID                          = 0;

    if (pdHandle == NULL)
    {
        return kStatus_PD_Error;
    }
    for (index = 0; index < sizeof(s_AltModeInstances) / sizeof(pd_alt_mode_t); ++index)
    {
        if ((s_AltModeInstances[index].occupied == 1U) && (s_AltModeInstances[index].pdHandle == pdHandle))
        {
            altModeInstance = &s_AltModeInstances[index];
            break;
        }
    }
    if (altModeInstance == NULL)
    {
        return kStatus_PD_Error;
    }

    /* process attach/detach events */
    switch ((pd_dpm_callback_event_t)event)
    {
        case PD_CONNECTED:
        case PD_CONNECT_ROLE_CHANGE:
            PD_AltModeInstanceReset(altModeInstance);
            callbackEvent = kAltMode_Attach;
            controlParam  = NULL;
            break;

        case PD_DISCONNECTED:
            PD_AltModeInstanceReset(altModeInstance);
            callbackEvent = kAltMode_Detach;
            controlParam  = NULL;
            break;

        case PD_DPM_SNK_HARD_RESET_REQUEST:
        case PD_DPM_SRC_HARD_RESET_REQUEST:
            PD_AltModeInstanceReset(altModeInstance);
            callbackEvent = kAltMode_HardReset;
            controlParam  = NULL;
            break;

        case PD_DPM_SNK_RDO_SUCCESS:
        case PD_DPM_SRC_RDO_SUCCESS:
#if (defined PD_CONFIG_ALT_MODE_HOST_SUPPORT) && (PD_CONFIG_ALT_MODE_HOST_SUPPORT)
            if (altModeInstance->altModeStart == 0U)
            {
                altModeInstance->altModeStart = 1U;
                /* trigger to start alt mode function */
                (void)PD_AltModeEnter(altModeInstance);
            }
#endif
            break;

        default:
            /* No action required. */
            break;
    }

    if (callbackEvent != kAltMode_EventInvalid)
    {
        return PD_AltModeNotifyModulesEvent(altModeInstance, callbackEvent, controlParam, 0x00u);
    }

    /* process AMS events */
    switch ((pd_dpm_callback_event_t)event)
    {
        /* structured vdm */
        case PD_DPM_STRUCTURED_VDM_REQUEST:
        case PD_DPM_STRUCTURED_VDM_SUCCESS:
        case PD_DPM_STRUCTURED_VDM_FAIL:
            if ((pd_dpm_callback_event_t)event == PD_DPM_STRUCTURED_VDM_SUCCESS)
            {
                callbackEvent = kAltMode_StructedVDMMsgSuccess;
            }
            else if ((pd_dpm_callback_event_t)event == PD_DPM_STRUCTURED_VDM_REQUEST)
            {
                callbackEvent = kAltMode_StructedVDMMsgReceivedProcess;
            }
            else
            {
                callbackEvent = kAltMode_StructedVDMMsgFail;
            }
            controlParam = param;
            msgSVID      = (uint16_t)((pd_svdm_command_result_t *)param)->vdmHeader.bitFields.SVID;
            break;

        /* unstructured vdm */
        case PD_DPM_UNSTRUCTURED_VDM_RECEIVED:
            callbackEvent = kAltMode_UnstructedVDMMsgReceived;
            controlParam  = param;
            msgSVID       = (uint16_t)(((pd_unstructured_vdm_command_param_t *)param)->vdmHeaderAndVDOsData[0] >> 16);
            break;

        case PD_DPM_SEND_UNSTRUCTURED_VDM_SUCCESS:
        case PD_DPM_SEND_UNSTRUCTURED_VDM_FAIL:
            callbackEvent = kAltMode_UnstructedVDMMsgSentResult;
            controlParam  = param;
            msgSVID       = 0;
            break;

        default:
            /* No action required. */
            break;
    }

    if (callbackEvent != kAltMode_EventInvalid)
    {
        if (msgSVID == 0xFF00u)
        {
            /* standard VDM process */
            status = PD_AltModeStandardVDMCallbackProcess(altModeInstance, event, param);
        }
        else
        {
            /* DP proccess */
            status = PD_AltModeNotifyModulesEvent(altModeInstance, callbackEvent, controlParam, msgSVID);
        }
    }

    /* process AMS events */
    switch ((pd_dpm_callback_event_t)event)
    {
        /* dr swap */
        case PD_DPM_DR_SWAP_SUCCESS:
#if (defined PD_CONFIG_ALT_MODE_HOST_SUPPORT) && (PD_CONFIG_ALT_MODE_HOST_SUPPORT)
            altModeInstance->altModeStart = 1;
            (void)PD_AltModeEnter(altModeInstance);
#endif
            break;

        case PD_DPM_DR_SWAP_FAIL:
#if 0
            if (altModeInstance->dfpCommand == PD_DPM_CONTROL_DR_SWAP)
            {
                /* wait and retry again */
                PD_AltModeDelayRetryCommand(altModeInstance, PD_DPM_CONTROL_DR_SWAP, PD_ALT_MODE_DR_SWAP_DELAY_TIME);
            }
#endif
            break;

        default:
            /* No action required. */
            break;
    }

    return status;
}

void PD_AltModeControl(void *altModeHandle, void *controlParam)
{
    pd_alt_mode_t *altModeInstance        = (pd_alt_mode_t *)altModeHandle;
    pd_alt_mode_control_t *altModeControl = (pd_alt_mode_control_t *)controlParam;

    if ((altModeControl->altModeModuleIndex <= 0U) ||
        (altModeControl->altModeModuleIndex > altModeInstance->altModeConfig->moduleCount))
    {
        return;
    }
    if (altModeInstance->altModeModuleInstance[altModeControl->altModeModuleIndex - 1U] == NULL)
    {
        return;
    }

    (void)altModeInstance->modulesInterfaces[altModeControl->altModeModuleIndex - 1U]->pd_alt_mode_control(
        altModeInstance->altModeModuleInstance[altModeControl->altModeModuleIndex - 1U], altModeControl->controlCode,
        altModeControl->controlParam);
}

uint8_t PD_AltModeExitModeForDrSwap(pd_alt_mode_handle altModeHandle)
{
    pd_alt_mode_t *altModeInstance = (pd_alt_mode_t *)altModeHandle;
    uint8_t result                 = 0;
    uint32_t index;

    for (index = 0; index < altModeInstance->altModeConfig->moduleCount; ++index)
    {
        if (altModeInstance->altModeModuleInstance[index] != NULL)
        {
            (void)altModeInstance->modulesInterfaces[index]->pd_alt_mode_control(
                altModeInstance->altModeModuleInstance[index], (uint32_t)kAltMode_TriggerExitMode, &result);
        }
    }

    return result;
}

uint8_t PD_AltModeCheckModalOperationSupported(void *altModeHandle)
{
    pd_alt_mode_t *altModeInstance = (pd_alt_mode_t *)altModeHandle;

    return (uint8_t)((pd_id_header_vdo_t *)((void *)&((const pd_alt_mode_config_t *)altModeInstance->altModeConfig)
                                                ->identityData[0]))
        ->bitFields.modalOperateSupport;
}

#endif
