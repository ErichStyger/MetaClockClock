/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_phy.h"
#include "usb_pd_timer.h"
#include "usb_pd_interface.h"
#if ((defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT))
#include "usb_pd_alt_mode.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static pd_instance_t *PD_GetInstance(void);
static void PD_ReleaseInstance(pd_instance_t *pdInstance);
static void PD_GetPhyInterface(uint8_t phyType, const pd_phy_api_interface_t **controllerTable);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static pd_instance_t s_PDInstance[PD_CONFIG_MAX_PORT];
static OSA_EVENT_HANDLE_DEFINE(s_PDStackEventHandle);
static uint8_t s_PDInitializeLabel = 0xFFu;

#if ((defined PD_CONFIG_PTN5100_PORT) && (PD_CONFIG_PTN5100_PORT))
static const pd_phy_api_interface_t s_PTN5100Interface = {
    PDphy_PTN5100Init, PDphy_PTN5100Deinit, PDphy_PTN5100Send, PDphy_PTN5100Receive, PDphy_PTN5100Control,
};
#endif /* PD_CONFIG_PTN5100_PORT */

#if ((defined PD_CONFIG_PTN5110_PORT) && (PD_CONFIG_PTN5110_PORT))
static const pd_phy_api_interface_t s_PTN5110Interface = {
    PDPTN5110_Init, PDPTN5110_Deinit, PDPTN5110_Send, PDPTN5110_Receive, PDPTN5110_Control,
};
#endif /* PD_CONFIG_PTN5110_PORT */

/*******************************************************************************
 * Code
 ******************************************************************************/
static pd_instance_t *PD_GetInstance(void)
{
    uint8_t i = 0;
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    for (; i < (uint8_t)PD_CONFIG_MAX_PORT; i++)
    {
        if (s_PDInstance[i].occupied != 1U)
        {
            (void)memset(&s_PDInstance[i], 0, sizeof(pd_instance_t));
            s_PDInstance[i].occupied = 1;
            OSA_EXIT_CRITICAL();
            return &s_PDInstance[i];
        }
    }
    OSA_EXIT_CRITICAL();
    return NULL;
}

static void PD_ReleaseInstance(pd_instance_t *pdInstance)
{
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    pdInstance->occupied = 0;
    OSA_EXIT_CRITICAL();
}

static void PD_GetPhyInterface(uint8_t phyType, const pd_phy_api_interface_t **controllerTable)
{
#if ((defined PD_CONFIG_PTN5100_PORT) && (PD_CONFIG_PTN5100_PORT))
    if (phyType == kPD_PhyPTN5100)
    {
        *controllerTable = &s_PTN5100Interface;
    }
#endif
#if ((defined PD_CONFIG_PTN5110_PORT) && (PD_CONFIG_PTN5110_PORT))
    if (phyType == (uint8_t)kPD_PhyPTN5110)
    {
        *controllerTable = &s_PTN5110Interface;
    }
#endif
}

uint8_t PD_StackHasPendingEvent(pd_instance_t *pdInstance)
{
    uint32_t taskEventSet = 0U;

    (void)OSA_EventGet(pdInstance->taskEventHandle, PD_TASK_EVENT_ALL, &taskEventSet);
    if (taskEventSet != 0U)
    {
        return 1U;
    }
    return 0U;
}

void PD_StackSetEvent(pd_instance_t *pdInstance, uint32_t event)
{
#if (defined PD_CONFIG_COMMON_TASK) && (PD_CONFIG_COMMON_TASK)
    int32_t instance = pdInstance - s_PDInstance;
#endif
    (void)OSA_EventSet(pdInstance->taskEventHandle, event);
#if (defined PD_CONFIG_COMMON_TASK) && (PD_CONFIG_COMMON_TASK)
    (void)OSA_EventSet(s_PDStackEventHandle, 0x00000001UL << (uint32_t)instance);
#endif
}

pd_status_t PD_PhyControl(pd_instance_t *pdInstance, pd_phy_control_t control, void *param)
{
    uint8_t enable        = 1;
    uint32_t taskEventSet = 0;

    (void)OSA_EventGet(pdInstance->taskEventHandle, PD_TASK_EVENT_PHY_STATE_CHAGNE, &taskEventSet);

    if ((control == PD_PHY_UPDATE_STATE) || (taskEventSet != 0U))
    {
        (void)OSA_EventClear(pdInstance->taskEventHandle, PD_TASK_EVENT_PHY_STATE_CHAGNE);
        (void)pdInstance->phyInterface->pdPhyControl(pdInstance, PD_PHY_UPDATE_STATE, NULL);
        (void)pdInstance->phyInterface->pdPhyControl(pdInstance, PD_PHY_CONTROL_ALERT_INTERRUPT, &enable);
#if (defined(__DSC__) && defined(__CW__))
        if (pdInstance->pdSetEvent != 0U)
        {
            pdInstance->pdSetEvent = 0;
        }
#endif
    }

    if (control != PD_PHY_UPDATE_STATE)
    {
        return pdInstance->phyInterface->pdPhyControl(pdInstance, control, param);
    }
    else
    {
        return kStatus_PD_Success;
    }
}

pd_status_t PD_InstanceInit(pd_handle *pdHandle,
                            pd_stack_callback_t callbackFn,
                            pd_power_handle_callback_t *callbackFunctions,
                            void *callbackParam,
                            pd_instance_config_t *config)
{
    pd_instance_t *pdInstance;
    uint8_t enable = 1;
#if (defined PD_CONFIG_COMMON_TASK) && (PD_CONFIG_COMMON_TASK)
    uint8_t index;
#endif

#if ((defined PD_CONFIG_COMMON_TASK) && (PD_CONFIG_COMMON_TASK)) || \
    ((defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT))
    if (s_PDInitializeLabel == 0xFFU)
    {
#if (defined PD_CONFIG_COMMON_TASK) && (PD_CONFIG_COMMON_TASK)
        if (KOSA_StatusSuccess != OSA_EventCreate(&(s_PDStackEventHandle[0]), 1))
        {
            return kStatus_PD_Error;
        }
        for (index = 0; index < (uint8_t)PD_CONFIG_MAX_PORT; ++index)
        {
            s_PDInstance[index].occupied = 0;
        }
#endif
        s_PDInitializeLabel = 0x00U;
#if ((defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT))
        if (PD_AltModeInit() != kStatus_PD_Success)
        {
#if (defined PD_CONFIG_COMMON_TASK) && (PD_CONFIG_COMMON_TASK)
            (void)OSA_EventDestroy(&(s_PDStackEventHandle[0]));
#endif
            return kStatus_PD_Error;
        }
#endif
    }
#endif

    /* Get pd instance */
    pdInstance = PD_GetInstance();
    if (pdInstance == NULL)
    {
        return kStatus_PD_Error;
    }

    pdInstance->pdCallback             = callbackFn;
    pdInstance->callbackFns            = callbackFunctions;
    pdInstance->callbackParam          = callbackParam;
    pdInstance->revision               = PD_CONFIG_REVISION;
    pdInstance->initializeLabel        = 0;
    pdInstance->taskWaitTime           = PD_WAIT_EVENT_TIME;
    pdInstance->curConnectState        = TYPEC_DISABLED;
    pdInstance->partnerSourcePDOsCount = 0;
    pdInstance->pdConfig               = config;
    pdInstance->phyInterface           = NULL;
    if (pdInstance->pdConfig->deviceType == (uint8_t)kDeviceType_NormalPowerPort)
    {
        pdInstance->pdPowerPortConfig = (pd_power_port_config_t *)pdInstance->pdConfig->portConfig;
        pdInstance->pendingSOP        = (uint8_t)kPD_MsgSOPMask;
    }
    else if (pdInstance->pdConfig->deviceType == (uint8_t)kDeviceType_Cable)
    {
        pdInstance->pendingSOP = (uint8_t)kPD_MsgSOPpMask | (uint8_t)kPD_MsgSOPppMask;
    }
    else
    {
        pdInstance->pendingSOP = (uint8_t)kPD_MsgSOPMask;
    }

    /* Get phy API table */
    PD_GetPhyInterface(pdInstance->pdConfig->phyType, &pdInstance->phyInterface);
    if ((pdInstance->phyInterface == NULL) || (pdInstance->phyInterface->pdPhyInit == NULL) ||
        (pdInstance->phyInterface->pdPhyDeinit == NULL) || (pdInstance->phyInterface->pdPhySend == NULL) ||
        (pdInstance->phyInterface->pdPhyReceive == NULL) || (pdInstance->phyInterface->pdPhyControl == NULL))
    {
        PD_ReleaseInstance(pdInstance);
        return kStatus_PD_Error;
    }

    /* Create pd event */
    if (KOSA_StatusSuccess != OSA_EventCreate(&(pdInstance->taskEventHandle[0]), 0))
    {
        PD_ReleaseInstance(pdInstance);
        return kStatus_PD_Error;
    }

    /* Initialize TCPC register cache */
    (void)memset(&pdInstance->tcpcRegCache, 0, sizeof(pd_phy_TCPC_reg_cache_t));

    /* Initialize phy */
    if (pdInstance->phyInterface->pdPhyInit(pdInstance) != kStatus_PD_Success)
    {
        (void)OSA_EventDestroy(pdInstance->taskEventHandle);
        PD_ReleaseInstance(pdInstance);
        return kStatus_PD_Error;
    }

    /* Initialize pd stack */
    PD_MsgReset(pdInstance);

    PD_TimerInit(pdInstance);

    PD_StackSetEvent(pdInstance, PD_TASK_EVENT_RESET_CONFIGURE);

#if ((defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT))
    /* Initialize alt mode */
    if (pdInstance->pdPowerPortConfig->altModeConfig != NULL)
    {
        if (PD_AltModeInstanceInit(pdInstance, (pd_alt_mode_config_t *)(pdInstance->pdPowerPortConfig->altModeConfig),
                                   &(pdInstance->altModeHandle)) != kStatus_PD_Success)
        {
            (void)pdInstance->phyInterface->pdPhyDeinit(pdInstance);
            (void)OSA_EventDestroy(pdInstance->taskEventHandle);
            PD_ReleaseInstance(pdInstance);
            return kStatus_PD_Error;
        }
    }
#endif

    *pdHandle = pdInstance;

    /* Enable alert pin interrupt. HAL_GpioSetTriggerMode will trigger ALERT interrupt.
       It should be called after pdInstance is assigned or ALERT IRQ function always is entered. */
    (void)pdInstance->phyInterface->pdPhyControl(pdInstance, PD_PHY_CONTROL_ALERT_INTERRUPT, &enable);

    return kStatus_PD_Success;
}

pd_status_t PD_InstanceDeinit(pd_handle pdHandle)
{
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;
    pd_status_t status        = kStatus_PD_Success;

#if ((defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT))
    (void)PD_AltModeInstanceDeinit(pdInstance->altModeHandle);
#endif
    (void)OSA_EventDestroy(pdInstance->taskEventHandle);
    status = pdInstance->phyInterface->pdPhyDeinit(pdInstance);
    PD_ReleaseInstance(pdInstance);

    return status;
}

/* pd instance task */
void PD_InstanceTask(pd_handle pdHandle)
{
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;

    if (pdHandle == NULL)
    {
        return;
    }
    PD_StackStateMachine(pdInstance);
}

/* pd common task for all instances */
void PD_Task(void)
{
    uint32_t taskEventSet = 0;
    uint8_t index;

    if (s_PDInitializeLabel == 0xFFU)
    {
        return;
    }

    (void)OSA_EventWait(s_PDStackEventHandle, PD_TASK_EVENT_ALL, 0, PD_WAIT_EVENT_TIME, &taskEventSet);
    /* if wait forever this will not enter or BM enter */
    if (taskEventSet == 0U)
    {
        return;
    }

    /* process events */
    for (index = 0; index < (uint8_t)PD_CONFIG_MAX_PORT; ++index)
    {
        if ((taskEventSet & ((uint32_t)0x01U << index)) != 0U)
        {
            /* The index instance has events */
            PD_StackStateMachine(&s_PDInstance[index]);
        }
    }
}

void PD_Notify(pd_handle pdHandle, pd_phy_notify_event_t event, void *param)
{
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;
    uint8_t enable;

    switch (event)
    {
        case PD_PHY_EVENT_STATE_CHANGE:
        {
            enable = 0;
            (void)pdInstance->phyInterface->pdPhyControl(pdInstance, PD_PHY_CONTROL_ALERT_INTERRUPT, &enable);
            PD_StackSetEvent(pdInstance, PD_TASK_EVENT_PHY_STATE_CHAGNE);
            break;
        }

        case PD_PHY_EVENT_SEND_COMPLETE:
        {
            PD_MsgSendDone(pdInstance, (pd_status_t) * ((uint8_t *)param));
            break;
        }

#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
        case PD_PHY_EVENT_FR_SWAP_SINGAL_RECEIVED:
        {
            if (pdInstance->inProgress != kVbusPower_InFRSwap)
            {
                PD_ConnectSetPowerProgress(pdInstance, kVbusPower_InFRSwap);
#if 0
                if (PD_DpmCheckLessOrEqualVsafe5v(pdInstance) == 0U)
                {
                    (void)pdInstance->callbackFns->PD_SrcTurnOffVbus(pdInstance->callbackParam, kVbusPower_Stable);
                }
#endif
                PD_StackSetEvent(pdInstance, PD_TASK_EVENT_FR_SWAP_SINGAL);
            }
            break;
        }
#endif
        case PD_PHY_EVENT_VBUS_SINK_DISCONNECT:
        {
            PD_StackSetEvent(pdInstance, PD_TASK_EVENT_VBUS_SINK_DISCONNECT);
            break;
        }

        case PD_PHY_EVENT_RECEIVE_COMPLETE:
        {
            PD_MsgReceived(pdInstance, (pd_phy_rx_result_t *)param);
            break;
        }

        case PD_PHY_EVENT_HARD_RESET_RECEIVED:
        {
            PD_ConnectSetPowerProgress(pdInstance, kVbusPower_InHardReset);
            PD_MsgReceivedHardReset(pdInstance);
            break;
        }

        case PD_PHY_EVENT_VBUS_ALARM:
            (void)pdInstance->pdCallback(pdInstance->callbackParam, PD_DPM_VBUS_ALARM, param);
            break;

        case PD_PHY_EVENT_REQUEST_STACK_RESET:
            /* BootSoftRebootToMain(); */
            break;

        case PD_PHY_EVENT_VCONN_PROTECTION_FAULT:

            break;

        case PD_PHY_EVENT_TYPEC_OVP_OCP_FAULT:
        {
            enable = 1;
            (void)pdInstance->pdCallback(pdInstance->callbackParam, PD_DPM_OVP_OCP_FAULT, param);
            (void)pdInstance->phyInterface->pdPhyDeinit(pdInstance);
            (void)pdInstance->phyInterface->pdPhyInit(pdInstance);
            (void)pdInstance->phyInterface->pdPhyControl(pdInstance, PD_PHY_CONTROL_ALERT_INTERRUPT, &enable);
            break;
        }

        case PD_PHY_EVENT_VBUS_STATE_CHANGE:
            PD_StackSetEvent(pdInstance, PD_TASK_EVENT_OTHER);
            break;

        default:
            /* No action required. */
            break;
    }
}

void USB_WEAK_FUN PD_WaitUsec(uint32_t us)
{
    uint32_t usDelay;

    while (us-- > 0U)
    {
        usDelay = 15;
        while (--usDelay > 0U)
        {
            __NOP();
        }
    }
}
