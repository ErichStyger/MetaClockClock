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
#include "usb_pd_alt_mode_dp.h"
#if (defined PD_ALT_MODE_LOG) && (PD_ALT_MODE_LOG)
#include "fsl_debug_console.h"
#endif

#if (defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT)
#if (defined PD_CONFIG_ALT_MODE_DP_SUPPORT) && (PD_CONFIG_ALT_MODE_DP_SUPPORT)
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define HPD_DETECT_QUEUE_LEN (4U)

typedef struct _pd_alt_mode_displayport
{
    pd_handle pdHandle;
    void *altModeHandle;
    void *dpBoardChipHandle;
    const pd_alt_mode_dp_config_t *dpConfig;
    uint32_t taskEvent;
    uint32_t hpdTime;
    uint32_t pdMsgBuffer[7];
    uint32_t pdMsgReceivedBuffer[7];
    volatile uint32_t delayTime;
    volatile uint32_t delayEvents;
    pd_dp_status_obj_t dpSelfStatus;
    pd_dp_status_obj_t dpPartnerStatus;
    pd_dp_configure_obj_t dpConfigure;
    pd_structured_vdm_header_t pdVDMMsgReceivedHeader;
    volatile uint32_t retryCount;
    volatile uint8_t retryCommand;
    uint8_t pdMsgReceivedVDOCount;
    uint8_t selectModeIndex;
    uint8_t occupied;
    pd_dp_mode_state_t dpState; /* pd_dp_state_t */
    uint8_t triggerCommand;
    volatile uint8_t dpCommand;
#if (defined PD_CONFIG_ALT_MODE_SLAVE_SUPPORT) && (PD_CONFIG_ALT_MODE_SLAVE_SUPPORT)
    volatile pd_hpd_detect_type_t attnStatusHPD;
    volatile uint8_t hpdDetectQueueLength;
    volatile uint8_t hpdDetectQueueGetPos;
    volatile uint8_t hpdDetectQueuePutPos;
    volatile pd_hpd_detect_type_t hpdDetectQueue[HPD_DETECT_QUEUE_LEN];
#endif

    uint8_t waitSendResult : 1;
    uint8_t CommandDoing : 1;
#if (defined PD_CONFIG_ALT_MODE_SLAVE_SUPPORT) && (PD_CONFIG_ALT_MODE_SLAVE_SUPPORT)
    volatile uint8_t hpdDetectEnable : 1;
#endif
} pd_alt_mode_dp_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static pd_alt_mode_dp_t s_AltModeDisplayPortInstance[PD_CONFIG_MAX_PORT];

/*******************************************************************************
 * Code
 ******************************************************************************/

static void PD_DpDelayRetryCommand(pd_alt_mode_dp_t *dpInstance, uint8_t command, uint32_t delay)
{
    dpInstance->delayTime = delay;
}

/* DP DFP and UFP */
static pd_status_t PD_DpSendCommand(pd_alt_mode_dp_t *dpInstance, uint8_t command)
{
    pd_svdm_command_param_t structuredVDMCommandParam;
    pd_command_t vdmCommand = PD_DPM_INVALID;

    if (dpInstance->CommandDoing != 0U)
    {
        return kStatus_PD_Error;
    }
    structuredVDMCommandParam.vdmSop                          = (uint8_t)kPD_MsgSOP;
    structuredVDMCommandParam.vdmHeader.bitFields.SVID        = DP_SVID;
    structuredVDMCommandParam.vdmHeader.bitFields.vdmType     = 1;
    structuredVDMCommandParam.vdmHeader.bitFields.objPos      = 0;
    structuredVDMCommandParam.vdmHeader.bitFields.commandType = (uint8_t)kVDM_Initiator;
    structuredVDMCommandParam.vendorVDMNeedResponse           = 0;

    switch (command)
    {
#if (defined PD_CONFIG_ALT_MODE_HOST_SUPPORT) && (PD_CONFIG_ALT_MODE_HOST_SUPPORT)
        case (uint8_t)kVDM_DiscoverModes:
            if (dpInstance->dpState != kDPMode_Exited)
            {
                return kStatus_PD_Error;
            }
            structuredVDMCommandParam.vdoCount = 0;
            structuredVDMCommandParam.vdoData  = NULL;
            vdmCommand                         = PD_DPM_CONTROL_DISCOVERY_MODES;
            break;

        case (uint8_t)kVDM_EnterMode:
            if (dpInstance->dpState != kDPMode_Exited)
            {
                return kStatus_PD_Error;
            }
            structuredVDMCommandParam.vdoCount                    = 0;
            structuredVDMCommandParam.vdoData                     = NULL;
            structuredVDMCommandParam.vdmHeader.bitFields.objPos  = dpInstance->selectModeIndex;
            structuredVDMCommandParam.vdmHeader.bitFields.command = (uint8_t)kVDM_EnterMode;
            vdmCommand                                            = PD_DPM_CONTROL_ENTER_MODE;
            break;

        case (uint8_t)kDPVDM_StatusUpdate:
            if (dpInstance->dpState < kDPMode_EnterDPDone)
            {
                return kStatus_PD_Error;
            }
            structuredVDMCommandParam.vdoCount                    = 1;
            structuredVDMCommandParam.vdoData                     = (uint32_t *)((void *)&dpInstance->dpSelfStatus);
            structuredVDMCommandParam.vdmHeader.bitFields.objPos  = dpInstance->selectModeIndex;
            structuredVDMCommandParam.vdmHeader.bitFields.command = (uint8_t)kDPVDM_StatusUpdate;
            structuredVDMCommandParam.vendorVDMNeedResponse       = 1;
            vdmCommand                                            = PD_DPM_SEND_VENDOR_STRUCTURED_VDM;
            break;

        case (uint8_t)kDPVDM_Configure:
            if (dpInstance->dpState < kDPMode_EnterDPDone)
            {
                return kStatus_PD_Error;
            }
            structuredVDMCommandParam.vdoCount                    = 1;
            structuredVDMCommandParam.vdoData                     = (uint32_t *)((void *)&dpInstance->dpConfigure);
            structuredVDMCommandParam.vdmHeader.bitFields.objPos  = dpInstance->selectModeIndex;
            structuredVDMCommandParam.vdmHeader.bitFields.command = (uint8_t)kDPVDM_Configure;
            structuredVDMCommandParam.vendorVDMNeedResponse       = 1;
            vdmCommand                                            = PD_DPM_SEND_VENDOR_STRUCTURED_VDM;
            break;

        case (uint8_t)kVDM_ExitMode:
            if (dpInstance->dpState < kDPMode_EnterDPDone)
            {
                return kStatus_PD_Error;
            }
            structuredVDMCommandParam.vdoCount                    = 0;
            structuredVDMCommandParam.vdoData                     = NULL;
            structuredVDMCommandParam.vdmHeader.bitFields.objPos  = dpInstance->selectModeIndex;
            structuredVDMCommandParam.vdmHeader.bitFields.command = (uint8_t)kVDM_ExitMode;
            vdmCommand                                            = PD_DPM_CONTROL_EXIT_MODE;
            break;
#endif

#if (defined PD_CONFIG_ALT_MODE_SLAVE_SUPPORT) && (PD_CONFIG_ALT_MODE_SLAVE_SUPPORT)
        case (uint8_t)kVDM_Attention:
            structuredVDMCommandParam.vdoCount                    = 1;
            structuredVDMCommandParam.vdoData                     = (uint32_t *)((void *)&dpInstance->dpSelfStatus);
            structuredVDMCommandParam.vdmHeader.bitFields.objPos  = dpInstance->selectModeIndex;
            structuredVDMCommandParam.vdmHeader.bitFields.command = (uint8_t)kVDM_Attention;
            vdmCommand                                            = PD_DPM_CONTROL_SEND_ATTENTION;
            break;
#endif

        default:
            /* No action required. */
            break;
    }

    if (vdmCommand != PD_DPM_INVALID)
    {
        dpInstance->CommandDoing = 1;
        if (PD_Command(dpInstance->pdHandle, (uint32_t)vdmCommand, &structuredVDMCommandParam) != kStatus_PD_Success)
        {
            dpInstance->CommandDoing = 0;
            /* wait and retry again */
            PD_DpDelayRetryCommand(dpInstance, command, PD_ALT_MODE_ERROR_RETRY_WAIT_TIME);
        }
    }

    return kStatus_PD_Success;
}

/* DP DFP or UFP */
static void PD_DpTrigerCommand(pd_alt_mode_dp_t *dpInstance, uint8_t command, uint8_t retry)
{
    OSA_SR_ALLOC();

    if (retry == 0U)
    {
        dpInstance->retryCount = PD_ALT_MODE_COMMAND_RETRY_COUNT;
    }
    dpInstance->dpCommand = command;
    OSA_ENTER_CRITICAL();
    dpInstance->triggerCommand = 1;
    OSA_EXIT_CRITICAL();
    PD_AltModeModuleTaskWakeUp(dpInstance->altModeHandle, dpInstance);
}

#if (defined PD_CONFIG_ALT_MODE_HOST_SUPPORT) && (PD_CONFIG_ALT_MODE_HOST_SUPPORT)
/* DP DFP function */
static pd_status_t PD_DpDFPGetModesCheckHaveSupportedMode(pd_alt_mode_dp_t *dpInstance)
{
    if (dpInstance->pdMsgReceivedVDOCount < 1U)
    {
        return kStatus_PD_Success;
    }
    dpInstance->selectModeIndex = 0;

#if !(defined PD_CONFIG_ALT_MODE_DP_AUTO_SELECT_MODE) || (!PD_CONFIG_ALT_MODE_DP_AUTO_SELECT_MODE)
    {
        pd_altmode_dp_modes_sel_t dpModes;
        dpModes.modesCount  = dpInstance->pdMsgReceivedVDOCount;
        dpModes.modes       = (pd_dp_mode_obj_t *)&(dpInstance->pdMsgReceivedBuffer[0]);
        dpModes.selectIndex = 0;

        PD_DpmAltModeCallback(dpInstance->pdHandle, PD_DPM_ALTMODE_DP_DFP_SELECT_MODE_AND_PINASSIGN, &dpModes);
        if ((dpModes.selectIndex > 0) && (dpModes.selectIndex <= dpModes.modesCount))
        {
            dpInstance->selectModeIndex                        = dpModes.selectIndex;
            dpInstance->dpConfigure.bitFields.configureUFPUPin = dpModes.selectPinAssign;
        }
    }
#else
    {
        pd_dp_mode_obj_t modeObj;
        uint8_t configurePin = 0;
        uint8_t index;
        for (index = 0; index < dpInstance->pdMsgReceivedVDOCount; ++index)
        {
            modeObj.modeVal = dpInstance->pdMsgReceivedBuffer[index];
            if (((modeObj.modeVal & (uint32_t)0xFF000000U) != 0U) ||
                ((modeObj.modeVal & (uint32_t)0x00FFFFFFU) == 0U) ||
                ((modeObj.bitFields.portCap & (uint8_t)kDPPortCap_UFPD) == 0U))
            {
                /* invalid mode */
                continue;
            }

            if (modeObj.bitFields.receptacleIndication != 0U)
            {
                /* receptacle */
                configurePin = (uint8_t)modeObj.bitFields.UFPDPinSupport;
            }
            else
            {
                configurePin = (uint8_t)modeObj.bitFields.DFPDPinSupport;
            }

            if ((configurePin & dpInstance->dpConfig->dpConfigurationsConfig.bitFields.configureUFPUPin) != 0U)
            {
                dpInstance->selectModeIndex = index + 1U;
                break;
            }
        }
    }
#endif

    if ((dpInstance->selectModeIndex > 0U) && (dpInstance->selectModeIndex <= dpInstance->pdMsgReceivedVDOCount))
    {
#if (defined PD_ALT_MODE_LOG) && (PD_ALT_MODE_LOG)
        uint8_t configurePin = 0;
        pd_dp_mode_obj_t modeObj;

        if ((dpInstance->selectModeIndex - 1U) >= 7U)
        {
            return kStatus_PD_Error; 
        }
        modeObj.modeVal = dpInstance->pdMsgReceivedBuffer[dpInstance->selectModeIndex - 1U];
        if (modeObj.bitFields.receptacleIndication != 0U)
        {
            /* receptacle */
            configurePin = (uint8_t)modeObj.bitFields.UFPDPinSupport;
        }
        else
        {
            configurePin = (uint8_t)modeObj.bitFields.DFPDPinSupport;
        }

        PRINTF("device supported pin assignments: ");
        if (configurePin & (uint8_t)kPinAssign_A)
        {
            PRINTF("A");
        }
        if (configurePin & (uint8_t)kPinAssign_B)
        {
            PRINTF("B");
        }
        if (configurePin & (uint8_t)kPinAssign_C)
        {
            PRINTF("C");
        }
        if (configurePin & (uint8_t)kPinAssign_D)
        {
            PRINTF("D");
        }
        if (configurePin & (uint8_t)kPinAssign_E)
        {
            PRINTF("E");
        }
        PRINTF("\r\n");
#endif
        return kStatus_PD_Success;
    }

    return kStatus_PD_Error;
}

/* DP DFP function */
static pd_status_t PD_DpDFPConstructConfigure(pd_alt_mode_dp_t *dpInstance)
{
#if defined(PD_CONFIG_ALT_MODE_DP_AUTO_SELECT_MODE) && (PD_CONFIG_ALT_MODE_DP_AUTO_SELECT_MODE)
    pd_dp_mode_obj_t modeObj;
#endif
    dp_mode_signal_t setSignal = kDPSignal_Unspecified;
    uint8_t configurePin       = 0;

    if (dpInstance->selectModeIndex == 0U)
    {
        return kStatus_PD_Error;
    }

#if !(defined PD_CONFIG_ALT_MODE_DP_AUTO_SELECT_MODE) || (!PD_CONFIG_ALT_MODE_DP_AUTO_SELECT_MODE)
    {
        configurePin = dpInstance->dpConfigure.bitFields.configureUFPUPin;
    }
#else
    if ((dpInstance->selectModeIndex - 1U) >= 7U)
    {
        return kStatus_PD_Error; 
    }
    modeObj.modeVal = dpInstance->pdMsgReceivedBuffer[dpInstance->selectModeIndex - 1U];
    /* if prefer multi function, kPinAssign_B and kPinAssign_D has high priority */
    if ((dpInstance->dpPartnerStatus.bitFields.multiFunctionPreferred != 0U) ||
        (dpInstance->dpConfig->dpStatusConfig.bitFields.multiFunctionPreferred != 0U))
    {
        if (modeObj.bitFields.receptacleIndication != 0U)
        {
            /* receptacle */
            configurePin = (uint8_t)modeObj.bitFields.UFPDPinSupport;
        }
        else
        {
            configurePin = (uint8_t)modeObj.bitFields.DFPDPinSupport;
        }
        configurePin &= ((uint8_t)kPinAssign_B | (uint8_t)kPinAssign_D);
        configurePin &= (uint8_t)dpInstance->dpConfig->dpConfigurationsConfig.bitFields.configureUFPUPin;
        if (configurePin != 0U)
        {
            if ((configurePin & (uint8_t)kPinAssign_D) != 0U)
            {
                setSignal    = kDPSignal_DP;
                configurePin = (uint8_t)kPinAssign_D;
            }
            else if ((configurePin & (uint8_t)kPinAssign_B) != 0U)
            {
                setSignal    = kDPSignal_USBGEN2;
                configurePin = (uint8_t)kPinAssign_B;
            }
            else
            {
                /* No action required. */
            }
        }
    }

    /* multi function is not prefered or don't get kPinAssign_B and kPinAssign_D
     * prefer the 4 lane pin assignment*/
    if (configurePin == 0U)
    {
        if (modeObj.bitFields.receptacleIndication != 0U)
        {
            /* receptacle */
            configurePin = (uint8_t)modeObj.bitFields.UFPDPinSupport;
        }
        else
        {
            configurePin = (uint8_t)modeObj.bitFields.DFPDPinSupport;
        }

        configurePin &= (~((uint8_t)kPinAssign_B | (uint8_t)kPinAssign_D));
        configurePin &= (uint8_t)dpInstance->dpConfig->dpConfigurationsConfig.bitFields.configureUFPUPin;
        if (configurePin != 0U)
        {
            if ((configurePin & (uint8_t)kPinAssign_C) != 0U)
            {
                setSignal    = kDPSignal_DP;
                configurePin = (uint8_t)kPinAssign_C;
            }
            else if ((configurePin & (uint8_t)kPinAssign_E) != 0U)
            {
                setSignal    = kDPSignal_DP;
                configurePin = (uint8_t)kPinAssign_E;
            }
            else if ((configurePin & (uint8_t)kPinAssign_A) != 0U)
            {
                setSignal    = kDPSignal_USBGEN2;
                configurePin = (uint8_t)kPinAssign_A;
            }
            else
            {
                /* No action required. */
            }
        }
    }

    /* get the first one */
    if (configurePin == 0U)
    {
        if (modeObj.bitFields.receptacleIndication != 0U)
        {
            /* receptacle */
            configurePin = (uint8_t)modeObj.bitFields.UFPDPinSupport;
        }
        else
        {
            configurePin = (uint8_t)modeObj.bitFields.DFPDPinSupport;
        }

        configurePin &= (uint8_t)dpInstance->dpConfig->dpConfigurationsConfig.bitFields.configureUFPUPin;
        if (configurePin != 0U)
        {
            if ((configurePin & (uint8_t)kPinAssign_A) != 0U)
            {
                setSignal    = kDPSignal_USBGEN2;
                configurePin = (uint8_t)kPinAssign_A;
            }
            else if ((configurePin & (uint8_t)kPinAssign_B) != 0U)
            {
                setSignal    = kDPSignal_USBGEN2;
                configurePin = (uint8_t)kPinAssign_B;
            }
            else if ((configurePin & (uint8_t)kPinAssign_C) != 0U)
            {
                setSignal    = kDPSignal_DP;
                configurePin = (uint8_t)kPinAssign_C;
            }
            else if ((configurePin & (uint8_t)kPinAssign_D) != 0U)
            {
                setSignal    = kDPSignal_DP;
                configurePin = (uint8_t)kPinAssign_D;
            }
            else if ((configurePin & (uint8_t)kPinAssign_E) != 0U)
            {
                setSignal    = kDPSignal_DP;
                configurePin = (uint8_t)kPinAssign_E;
            }
            else
            {
                /* No action required. */
            }
        }
    }
#endif

    if (configurePin != 0U)
    {
#if (defined PD_ALT_MODE_LOG) && (PD_ALT_MODE_LOG)
        PRINTF("select pin assignments: ");
        if ((configurePin & (uint8_t)kPinAssign_A) != 0U)
        {
            PRINTF("A");
        }
        else if ((configurePin & (uint8_t)kPinAssign_B) != 0U)
        {
            PRINTF("B");
        }
        else if ((configurePin & (uint8_t)kPinAssign_C) != 0U)
        {
            PRINTF("C");
        }
        else if ((configurePin & (uint8_t)kPinAssign_D) != 0U)
        {
            PRINTF("D");
        }
        else if ((configurePin & (uint8_t)kPinAssign_E) != 0U)
        {
            PRINTF("E");
        }
        else
        {
            /* No action required. */
        }
        PRINTF("\r\n");
#endif
        dpInstance->dpConfigure.bitFields.setConfig        = (uint8_t)kDPConfig_UFPD;
        dpInstance->dpConfigure.bitFields.setSignal        = (uint8_t)setSignal;
        dpInstance->dpConfigure.bitFields.configureUFPUPin = configurePin;
        return kStatus_PD_Success;
    }

    return kStatus_PD_Error;
}

static void PD_DpDFPSetConfigureAsUSB(pd_alt_mode_dp_t *dpInstance)
{
    dpInstance->dpConfigure.bitFields.setConfig        = (uint8_t)kDPConfig_USB;
    dpInstance->dpConfigure.bitFields.setSignal        = (uint8_t)kDPSignal_Unspecified;
    dpInstance->dpConfigure.bitFields.configureUFPUPin = (uint8_t)kPinAssign_DeSelect;
}

static void PD_DpDFPProcessUFPstatus(pd_alt_mode_dp_t *dpInstance)
{
    pd_dp_hpd_driver_t driverVal = kDPHPDDriver_Low;

    if (dpInstance->dpPartnerStatus.bitFields.HPDInterrupt != 0U)
    {
        driverVal = kDPHPDDriver_IRQ;
    }
    else if (dpInstance->dpPartnerStatus.bitFields.HPDState != 0U)
    {
        driverVal = kDPHPDDriver_High;
    }
    else
    {
        driverVal = kDPHPDDriver_Low;
    }
    (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(dpInstance->dpBoardChipHandle,
                                                                         kDPPeripheal_ControlHPDValue, &driverVal);

    /* Figure 5-4 */
    if ((dpInstance->dpPartnerStatus.bitFields.exitDPModeReq != 0U) ||
        (dpInstance->dpPartnerStatus.bitFields.USBConfigReq != 0U) ||
        ((dpInstance->dpPartnerStatus.bitFields.DFPDUFPDConnected & (uint8_t)kUFP_D_Connected) == 0U))
    {
#if (defined PD_ALT_MODE_LOG) && (PD_ALT_MODE_LOG)
        PRINTF("start exit mode\r\n");
#endif
        driverVal = kDPHPDDriver_Low;
        (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(dpInstance->dpBoardChipHandle,
                                                                             kDPPeripheal_ControlHPDValue, &driverVal);
        /* The DFP_U shall issue an Exit Mode command only when the port is configured to be in USB configuration.
         */
        /* Receipt of an Exit Mode command while not configured in USB Configuration indicates an error in the
         * DFP_U. */
        (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(dpInstance->dpBoardChipHandle,
                                                                             kDPPeripheal_ControlHPDSetLow, NULL);
        if (dpInstance->dpState == kDPMode_ConfigureDone)
        {
            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlSetMuxSaftMode, NULL);
            PD_DpDFPSetConfigureAsUSB(dpInstance);
            PD_DpTrigerCommand(dpInstance, (uint8_t)kDPVDM_Configure, 0);
        }
        else
        {
            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlSetMuxUSB3Only, NULL);
            PD_DpTrigerCommand(dpInstance, (uint8_t)kVDM_ExitMode, 0);
        }
    }
}
#endif

#if (defined PD_CONFIG_ALT_MODE_SLAVE_SUPPORT) && (PD_CONFIG_ALT_MODE_SLAVE_SUPPORT)

static pd_hpd_detect_type_t PD_DpUFPHPDDetectPeek(pd_alt_mode_dp_t *dpInstance)
{
    pd_hpd_detect_type_t result;
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    if (dpInstance->hpdDetectQueueLength == 0U)
    {
        result = kDPHPDDetect_Empty;
    }
    else
    {
        result = dpInstance->hpdDetectQueue[dpInstance->hpdDetectQueueGetPos];
    }
    OSA_EXIT_CRITICAL();
    return result;
}

static void PD_DpUFPHPDDetectPop(pd_alt_mode_dp_t *dpInstance, pd_hpd_detect_type_t popValue)
{
    OSA_SR_ALLOC();

    if (popValue == kDPHPDDetect_Empty)
    {
        return;
    }

    OSA_ENTER_CRITICAL();
    if (dpInstance->hpdDetectQueueLength != 0U)
    {
        if (dpInstance->hpdDetectQueue[dpInstance->hpdDetectQueueGetPos] == popValue)
        {
            dpInstance->hpdDetectQueueGetPos = (dpInstance->hpdDetectQueueGetPos + 1U) % HPD_DETECT_QUEUE_LEN;
            --(dpInstance->hpdDetectQueueLength);
        }
    }
    OSA_EXIT_CRITICAL();
}

static void PD_DpUFPHPDDetectReset(pd_alt_mode_dp_t *dpInstance)
{
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    dpInstance->hpdDetectEnable      = 0;
    dpInstance->hpdDetectQueueGetPos = 0;
    dpInstance->hpdDetectQueuePutPos = 0;
    dpInstance->hpdDetectQueueLength = 0;
    OSA_EXIT_CRITICAL();
}

static void PD_DpUFPHPDDetectEnable(pd_alt_mode_dp_t *dpInstance)
{
    dpInstance->hpdDetectEnable = 1;
}

static void PD_DpUFPHPDDetectAdd(pd_alt_mode_dp_t *dpInstance, pd_hpd_detect_type_t detectValue)
{
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    if (dpInstance->hpdDetectQueueLength < HPD_DETECT_QUEUE_LEN)
    {
        if (detectValue == kDPHPDDetect_IRQ)
        {
            uint8_t lastIndex = 0U;
            if (dpInstance->hpdDetectQueuePutPos < 1U)
            {
                OSA_EXIT_CRITICAL();
                return;
            }
            lastIndex = dpInstance->hpdDetectQueuePutPos - 1U;
            if (dpInstance->hpdDetectQueuePutPos == 0U)
            {
                lastIndex = HPD_DETECT_QUEUE_LEN - 1U;
            }
            if (dpInstance->hpdDetectQueue[lastIndex] == kDPHPDDetect_High)
            {
                dpInstance->hpdDetectQueue[lastIndex] = kDPHPDDetect_IRQ;
                OSA_EXIT_CRITICAL();
                return;
            }
        }
        dpInstance->hpdDetectQueue[dpInstance->hpdDetectQueuePutPos] = detectValue;
        dpInstance->hpdDetectQueuePutPos = (dpInstance->hpdDetectQueuePutPos + 1U) % HPD_DETECT_QUEUE_LEN;
        dpInstance->hpdDetectQueueLength++;
    }
    OSA_EXIT_CRITICAL();
}

static void PD_DpUFPHPDDetectPut(pd_alt_mode_dp_t *dpInstance, pd_hpd_detect_type_t detectValue)
{
    if (dpInstance->hpdDetectEnable == 0U)
    {
        return;
    }

    if (detectValue == kDPHPDDetect_Low)
    {
        /* Low detected clears the queue */
        PD_DpUFPHPDDetectReset(dpInstance);
        PD_DpUFPHPDDetectEnable(dpInstance);
        PD_DpUFPHPDDetectAdd(dpInstance, kDPHPDDetect_Low);
    }
    else if (detectValue == kDPHPDDetect_High)
    {
        /* Just add the high in (shouldn't go beyond the end of the queue, but better to be safe) */
        PD_DpUFPHPDDetectAdd(dpInstance, kDPHPDDetect_High);
    }
    else
    {
        uint8_t index    = 0;
        uint8_t irqFound = 0;
        if (dpInstance->hpdDetectQueueLength > 0U)
        {
            index = dpInstance->hpdDetectQueueGetPos;
            do
            {
                if (dpInstance->hpdDetectQueue[index] == kDPHPDDetect_IRQ)
                {
                    irqFound++;
                    if (irqFound >= 2U)
                    {
                        break;
                    }
                }
                index = (index + 1U) % HPD_DETECT_QUEUE_LEN;
            } while (index != dpInstance->hpdDetectQueuePutPos);
        }

        if (irqFound < 2U)
        {
            PD_DpUFPHPDDetectAdd(dpInstance, kDPHPDDetect_IRQ);
        }
    }
}

/* DP UFP */
static void PD_DpUFPTriggerTask(pd_alt_mode_dp_t *dpInstance)
{
    PD_AltModeModuleTaskWakeUp(dpInstance->altModeHandle, dpInstance);
}

/* DP UFP function */
static void PD_DpUFPGetStatus(pd_alt_mode_dp_t *dpInstance)
{
    pd_hpd_detect_type_t getState;
    uint8_t hpdState = 0;
    uint8_t irqState = 0;

    getState = PD_DpUFPHPDDetectPeek(dpInstance);
    /* We will only clear this from the detect queue when the message TX is confirmed.
     * Store it here as the queue may change when we check again */
    dpInstance->attnStatusHPD = getState;

    dpInstance->dpSelfStatus.statusVal = 0;
    dpInstance->dpSelfStatus.bitFields.DFPDUFPDConnected =
        dpInstance->dpConfig->dpStatusConfig.bitFields.DFPDUFPDConnected;
    dpInstance->dpSelfStatus.bitFields.powerLow = dpInstance->dpConfig->dpStatusConfig.bitFields.powerLow;
    dpInstance->dpSelfStatus.bitFields.enabled  = dpInstance->dpConfig->dpStatusConfig.bitFields.enabled;
    dpInstance->dpSelfStatus.bitFields.multiFunctionPreferred =
        dpInstance->dpConfig->dpStatusConfig.bitFields.multiFunctionPreferred;

    if (getState != kDPHPDDetect_Empty)
    {
        if (getState == kDPHPDDetect_High)
        {
            hpdState = 0x01u;
        }
        else if (getState == kDPHPDDetect_IRQ)
        {
            hpdState = 0x01u;
            irqState = 0x01u;
        }
        else
        {
            /* No action required. */
        }
    }
    else
    {
        /* else just get the state of the HPD input */
        (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
            dpInstance->dpBoardChipHandle, kDPPeripheal_ControlHPDGetCurrentState, &getState);
        if (getState == kDPHPDDetect_High)
        {
            hpdState = 0x01u;
        }
    }

    dpInstance->dpSelfStatus.bitFields.HPDState     = hpdState;
    dpInstance->dpSelfStatus.bitFields.HPDInterrupt = irqState;

#if 0
#ifndef USBPD_DP_INTEGRATED_UFP_D
    /* Connected status is based on HPD status if we do not have an integrated UFP_D. */
    if (ret & HPD_HIGH)
    {
        dpInstance->dpSelfStatus.bitFields.DFPDUFPDConnected |= kUFP_D_Connected;
    }
#endif
#endif
}

/* DP UFP function */
static void PD_DpUFPProcessStatusUpdate(pd_alt_mode_dp_t *dpInstance)
{
    PD_DpUFPGetStatus(dpInstance);
    PD_DpUFPHPDDetectPop(dpInstance, dpInstance->attnStatusHPD);
}

/* DP UFP function */
static uint8_t PD_DpUFPProcessConfigure(pd_alt_mode_dp_t *dpInstance)
{
    /* Configure has arrived - change MUXes and respond
     * By default NACK the config */
    uint8_t configAck = 0;

    if (dpInstance->dpConfigure.bitFields.setConfig == (uint8_t)kDPConfig_USB)
    {
        /* Only expect to receive this in: Figure 5-10: UFP_U DisplayPort Operation */
        if (dpInstance->dpState >= kDPMode_ConfigureDone)
        {
            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlSetMuxUSB3Only, NULL);

            /* we are a dock here so don't need HpdDriverSetLow() */
            configAck = 1;

            /* Figure 5-10: UFP_U DisplayPort Operation DisplayPort <ConfigureCommand: USB Configuration?> */
            dpInstance->dpState = kDPMode_EnterDPDone;
        }
        else
        {
            /* Already in USB mode */
            configAck = 1;
        }
    }
    else if (dpInstance->dpConfigure.bitFields.setConfig == (uint8_t)kDPConfig_UFPD)
    {
        uint32_t pinAssignment = dpInstance->dpConfigure.bitFields.configureUFPUPin;
        /* We can receive this in :
         *       Figure 5-8: UFP_U USB Configuration Wait State
         *       Figure 5-10: UFP_U DisplayPort Operation
         * Default NACK - Stay in: Figure 5-8: UFP_U USB Configuration Wait State */
        configAck = 0;
        if (dpInstance->dpState < kDPMode_EnterDPDone)
        {
            return configAck;
        }

        /* configAck set: Figure 5-9: UFP_U DisplayPort Configuration */

#ifdef USBPD_ENABLE_FAST_CENTER_ALT_MODE_CONTROL
        if ((pinAssignment & (uint8_t)kPinAssign_VR) != 0U)
        {
            uint32_t pdo = (0x80 << 8);
            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlSetMuxDP4LANEUSB3, &pdo);
            /* can handle the configure so ack it */
            configAck = 1;
        }
        else
#endif
            if (((pinAssignment & ((uint8_t)kPinAssign_D | (uint8_t)kPinAssign_F)) != 0U)
                /* Pin assignment in the old DP Alt Mode 1.0 location */
                || ((dpInstance->dpConfigure.bitFields.reserved1 & (uint8_t)kPinAssign_D) != 0U))
        {
            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlSetMuxDP2LANEUSB3, &dpInstance->dpConfigure);

            /* can handle the configure so ack it */
            configAck = 1;
        }
        else
        {
            /* Default to mandatory pin assignment C in all other cases. */
            uint32_t pdo = ((uint32_t)kPinAssign_C << 8U);

            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlSetMuxDP4LANE, &pdo);

            /* can handle the configure so ack it */
            configAck = 1;
        }
        /* There is intentionally no error case here.  We default to pin assignment C.
         * Figure 5-10: UFP_U DisplayPort Operation DisplayPort <ConfigureCommand: USB Configuration?> */
        dpInstance->dpState = kDPMode_ConfigureDone;
    }
    else
    {
        configAck = 0;
    }

    return configAck;
}
#endif

static void PD_DpInstanceReset(pd_alt_mode_dp_t *dpInstance)
{
#if (defined PD_CONFIG_ALT_MODE_HOST_SUPPORT) && (PD_CONFIG_ALT_MODE_HOST_SUPPORT)
    pd_dp_hpd_driver_t driverVal = kDPHPDDriver_Low;
#endif
    dpInstance->dpState      = kDPMode_Exited;
    dpInstance->taskEvent    = 0u;
    dpInstance->CommandDoing = 0;
    dpInstance->retryCommand = 0;
#if (defined PD_CONFIG_ALT_MODE_HOST_SUPPORT) && (PD_CONFIG_ALT_MODE_HOST_SUPPORT)
    (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(dpInstance->dpBoardChipHandle,
                                                                         kDPPeripheal_ControlHPDValue, &driverVal);
#endif
    (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(dpInstance->dpBoardChipHandle,
                                                                         kDPPeripheal_ControlSetMuxUSB3Only, NULL);
}

void PD_DPModule1msISR(void *moduleInstance)
{
    pd_alt_mode_dp_t *dpInstance = (pd_alt_mode_dp_t *)moduleInstance;

    if (dpInstance->delayTime > 0U)
    {
        dpInstance->delayTime--;
        if (dpInstance->delayTime == 0U)
        {
            /* PD_DpModuleSetEvent(dpInstance, dpInstance->delayEvents); */
            PD_DpTrigerCommand(dpInstance, dpInstance->dpCommand, 1);
        }
    }
}

/*
 * pdHandle - PD tack handle.
 * altModeHandle - alt mode driver handle.
 * moduleConfig - displayport module configuration parameter.
 * moduleInstance - return the displayport module instance handle
 *
 */
pd_status_t PD_DPInit(pd_handle pdHandle, void *altModeHandle, const void *moduleConfig, void **moduleInstance)
{
    uint32_t index               = 0;
    pd_alt_mode_dp_t *dpInstance = NULL;
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    for (index = 0; index < sizeof(s_AltModeDisplayPortInstance) / sizeof(pd_alt_mode_dp_t); ++index)
    {
        if (s_AltModeDisplayPortInstance[index].occupied == 0U)
        {
            s_AltModeDisplayPortInstance[index].occupied = 1U;
            dpInstance                                   = &s_AltModeDisplayPortInstance[index];
            break;
        }
    }

    if (dpInstance == NULL)
    {
        OSA_EXIT_CRITICAL();
        return kStatus_PD_Error;
    }
    OSA_EXIT_CRITICAL();
    dpInstance->pdHandle      = pdHandle;
    dpInstance->altModeHandle = altModeHandle;

    dpInstance->dpConfig = (const pd_alt_mode_dp_config_t *)moduleConfig;
    if (dpInstance->dpConfig->peripheralInterface->dpPeripheralInit(
            &(dpInstance->dpBoardChipHandle), pdHandle, (const void *)dpInstance->dpConfig->peripheralConfig) !=
        kStatus_PD_Success)
    {
        dpInstance->occupied = 0;
        return kStatus_PD_Error;
    }

    PD_DpInstanceReset(dpInstance);

    *moduleInstance = dpInstance;
    return kStatus_PD_Success;
}

pd_status_t PD_DPDeinit(void *moduleInstance)
{
    pd_alt_mode_dp_t *dpInstance = (pd_alt_mode_dp_t *)moduleInstance;

    (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralDeinit(dpInstance->dpBoardChipHandle);
    dpInstance->occupied = 0;

    return kStatus_PD_Success;
}

pd_status_t PD_DPControl(void *moduleInstance, uint32_t controlCode, void *controlParam)
{
    pd_alt_mode_dp_t *dpInstance = (pd_alt_mode_dp_t *)moduleInstance;
    pd_status_t status           = kStatus_PD_Success;

    /* dfp and ufp */
    switch ((pd_alt_mode_control_code_t)controlCode)
    {
#if (defined PD_CONFIG_ALT_MODE_HOST_SUPPORT) && (PD_CONFIG_ALT_MODE_HOST_SUPPORT)
        /* DFP start to enter mode sequence */
        case kAltMode_TriggerEnterMode:
            PD_DpTrigerCommand(dpInstance, (uint8_t)kVDM_DiscoverModes, 0);
            break;
#endif

        case kAltMode_TriggerExitMode:
        {
            uint8_t dataRole;
            (void)PD_Control(dpInstance->pdHandle, (uint32_t)PD_CONTROL_GET_DATA_ROLE, &dataRole);
            if (dpInstance->dpState >= kDPMode_EnterDPDone)
            {
                *((uint8_t *)controlParam) = 1;
#if (defined PD_CONFIG_ALT_MODE_HOST_SUPPORT) && (PD_CONFIG_ALT_MODE_HOST_SUPPORT)
                if (dataRole == (uint8_t)kPD_DataRoleDFP)
                {
                    PD_DpTrigerCommand(dpInstance, (uint8_t)kVDM_ExitMode, 0);
                }
#endif
#if (defined PD_CONFIG_ALT_MODE_SLAVE_SUPPORT) && (PD_CONFIG_ALT_MODE_SLAVE_SUPPORT)
                if (dataRole == (uint8_t)kPD_DataRoleUFP)
                {
                    dpInstance->dpSelfStatus.bitFields.exitDPModeReq = 1;
                    PD_DpTrigerCommand(dpInstance, (uint8_t)kVDM_Attention, 0);
                }
#endif
            }
            else
            {
                *((uint8_t *)controlParam) = 0;
            }
            break;
        }

        case kAltMode_GetModeState:
        {
            if (controlParam == NULL)
            {
                status = kStatus_PD_Error;
            }
            else
            {
                pd_alt_mode_state_t *modeState = (pd_alt_mode_state_t *)controlParam;
                modeState->SVID                = 0u;
                modeState->mode                = 0;
                if (dpInstance->dpState >= kDPMode_EnterDPDone)
                {
                    modeState->SVID = DP_SVID;
                    modeState->mode = dpInstance->selectModeIndex;
                }
            }
            break;
        }

#if (defined PD_CONFIG_ALT_MODE_SLAVE_SUPPORT) && (PD_CONFIG_ALT_MODE_SLAVE_SUPPORT)
        case kDPControl_HPDDetectEvent:
        {
            if ((dpInstance->dpState >= kDPMode_ConfigureDone) ||
                (dpInstance->dpState == kDPMode_StatusUpdateDone &&
                 dpInstance->dpSelfStatus.bitFields.DFPDUFPDConnected == (uint8_t)kDFP_D_NonConnected))
            {
                /* add hpd detect to queue */
                PD_DpUFPHPDDetectPut(dpInstance, (pd_hpd_detect_type_t)(*(uint8_t *)controlParam));
                PD_DpUFPTriggerTask(dpInstance);
            }
            break;
        }
#endif

        default:
            /* No action required. */
            break;
    }

    return status;
}

/* msgSVID: 0 - this msg related event doesn't know SVID. */
pd_status_t PD_DPCallbackEvent(void *moduleInstance, uint32_t processCode, uint16_t msgSVID, void *param)
{
    pd_status_t status           = kStatus_PD_Error;
    pd_alt_mode_dp_t *dpInstance = (pd_alt_mode_dp_t *)moduleInstance;
    uint32_t index               = 0;

    if ((msgSVID != 0U) && (msgSVID != 0xFF01U))
    {
        return status;
    }

    /* process the msg related events, if not self msg or self shouldn't process this event return error. */
    switch ((pd_alt_mode_callback_code_t)processCode)
    {
        case kAltMode_Attach:
        case kAltMode_HardReset:
            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlSetMuxUSB3Only, NULL);
#if (defined PD_CONFIG_ALT_MODE_HOST_SUPPORT) && (PD_CONFIG_ALT_MODE_HOST_SUPPORT)
            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(dpInstance->dpBoardChipHandle,
                                                                                 kDPPeripheal_ControlHPDSetLow, NULL);
#endif
#if (defined PD_CONFIG_ALT_MODE_SLAVE_SUPPORT) && (PD_CONFIG_ALT_MODE_SLAVE_SUPPORT)
            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlHPDQueueDisable, NULL);
            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlHPDDetectStart, NULL);
            PD_DpUFPHPDDetectReset(dpInstance);
#endif
            PD_DpInstanceReset(dpInstance);
            break;

        case kAltMode_Detach:
            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlSetMuxShutDown, NULL);
#if (defined PD_CONFIG_ALT_MODE_HOST_SUPPORT) && (PD_CONFIG_ALT_MODE_HOST_SUPPORT)
            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(dpInstance->dpBoardChipHandle,
                                                                                 kDPPeripheal_ControlHPDSetLow, NULL);
#endif
#if (defined PD_CONFIG_ALT_MODE_SLAVE_SUPPORT) && (PD_CONFIG_ALT_MODE_SLAVE_SUPPORT)
            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlHPDQueueDisable, NULL);
            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlHPDDetectStop, NULL);
            PD_DpUFPHPDDetectReset(dpInstance);
#endif
            PD_DpInstanceReset(dpInstance);
            break;

        case kAltMode_StructedVDMMsgReceivedProcess:
        {
            pd_svdm_command_request_t *svdmRequest = (pd_svdm_command_request_t *)param;

            if (msgSVID == 0xFF01U)
            {
                status = kStatus_PD_Success;
                switch (svdmRequest->vdmHeader.bitFields.command)
                {
                    case (uint32_t)kVDM_DiscoverModes: /* DP DFP or UFP */
                    {
                        if (PD_AltModeCheckModalOperationSupported(dpInstance->altModeHandle) != 0U)
                        {
                            for (index = 0; index < dpInstance->dpConfig->modesCount; ++index)
                            {
                                dpInstance->pdMsgBuffer[index] = dpInstance->dpConfig->modesList[index];
                            }
                            svdmRequest->vdoCount            = (uint8_t)dpInstance->dpConfig->modesCount;
                            svdmRequest->vdoData             = (uint32_t *)&dpInstance->pdMsgBuffer[0];
                            svdmRequest->requestResultStatus = (uint8_t)kCommandResult_VDMACK;
                        }
                        else
                        {
                            svdmRequest->vdoCount            = 0;
                            svdmRequest->vdoData             = NULL;
                            svdmRequest->requestResultStatus = (uint8_t)kCommandResult_VDMNAK;
                        }
                        break;
                    }

                    case (uint32_t)kVDM_EnterMode: /* DP UFP */
                    {
                        svdmRequest->vdoData             = NULL;
                        svdmRequest->vdoCount            = 0;
                        svdmRequest->requestResultStatus = (uint8_t)kCommandResult_VDMNAK;

#if (defined PD_CONFIG_ALT_MODE_SLAVE_SUPPORT) && (PD_CONFIG_ALT_MODE_SLAVE_SUPPORT)
                        if ((PD_AltModeCheckModalOperationSupported(dpInstance->altModeHandle) != 0U) &&
                            (svdmRequest->vdmHeader.bitFields.objPos > 0U) &&
                            (svdmRequest->vdmHeader.bitFields.objPos <= dpInstance->dpConfig->modesCount) &&
                            ((((pd_dp_mode_obj_t *)(void *)
                                   dpInstance->dpConfig->modesList)[svdmRequest->vdmHeader.bitFields.objPos - 1U]
                                  .bitFields.portCap &
                              (uint8_t)kDPPortCap_UFPD) != 0U))
                        {
                            dpInstance->selectModeIndex = (uint8_t)svdmRequest->vdmHeader.bitFields.objPos;

                            PD_DpUFPHPDDetectReset(dpInstance);
                            PD_DpUFPHPDDetectEnable(dpInstance);
                            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlHPDQueueEnable, NULL);
                            dpInstance->dpState              = kDPMode_EnterDPDone;
                            svdmRequest->requestResultStatus = (uint8_t)kCommandResult_VDMACK;
                        }
#endif
                        break;
                    }

                    case (uint32_t)kVDM_ExitMode: /* DP UFP */
                    {
                        svdmRequest->vdoData             = NULL;
                        svdmRequest->vdoCount            = 0;
                        svdmRequest->requestResultStatus = (uint8_t)kCommandResult_VDMNAK;

#if (defined PD_CONFIG_ALT_MODE_SLAVE_SUPPORT) && (PD_CONFIG_ALT_MODE_SLAVE_SUPPORT)
                        if ((dpInstance->dpState == kDPMode_EnterDPDone) ||
                            (dpInstance->dpState == kDPMode_StatusUpdateDone) ||
                            (dpInstance->dpState == kDPMode_ConfigureDone))
                        {
                            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlHPDQueueDisable, NULL);
                            PD_DpUFPHPDDetectReset(dpInstance);
                            dpInstance->dpState              = kDPMode_Exited;
                            svdmRequest->requestResultStatus = (uint8_t)kCommandResult_VDMACK;
                            PD_DpmAltModeCallback(dpInstance->pdHandle, PD_DPM_ALTMODE_DP_DFP_MODE_UNCONFIGURED, NULL);
                        }
#endif
                        break;
                    }

#if (defined PD_CONFIG_ALT_MODE_SLAVE_SUPPORT) && (PD_CONFIG_ALT_MODE_SLAVE_SUPPORT)
                    case (uint32_t)kDPVDM_StatusUpdate: /* DP UFP */
                    {
                        /* can receive at any time
                           The UFP_U is not allowed to return a Responder NAK */
                        if ((dpInstance->dpState == kDPMode_EnterDPDone) ||
                            (dpInstance->dpState == kDPMode_StatusUpdateDone) ||
                            (dpInstance->dpState == kDPMode_ConfigureDone))
                        {
                            dpInstance->dpPartnerStatus.statusVal = svdmRequest->vdoData[0];
                            PD_DpUFPProcessStatusUpdate(dpInstance);
                            svdmRequest->vdoData             = (uint32_t *)((void *)&dpInstance->dpSelfStatus);
                            svdmRequest->vdoCount            = 1;
                            svdmRequest->requestResultStatus = (uint8_t)kCommandResult_VDMACK;
                            if (dpInstance->dpState == kDPMode_EnterDPDone)
                            {
                                dpInstance->dpState = kDPMode_StatusUpdateDone;
                            }
                        }
                        break;
                    }

                    case (uint32_t)kDPVDM_Configure: /* DP UFP */
                    {
                        /* can receive at any time */
                        if ((dpInstance->dpState == kDPMode_EnterDPDone) ||
                            (dpInstance->dpState == kDPMode_StatusUpdateDone) ||
                            (dpInstance->dpState == kDPMode_ConfigureDone))
                        {
                            dpInstance->dpConfigure.configureVal = svdmRequest->vdoData[0];
                            if ((dpInstance->dpConfigure.bitFields.setConfig == (uint8_t)kDPConfig_DFPD) ||
                                (dpInstance->dpConfigure.bitFields.setConfig == (uint8_t)kDPConfig_UFPD))
                            {
                                if (dpInstance->dpState != kDPMode_ConfigureDone)
                                {
                                    dpInstance->dpState = kDPMode_ConfigureDone;
                                }
                            }
                            else
                            {
                                /* back the state */
                                dpInstance->dpState = kDPMode_StatusUpdateDone;
                            }

                            /* after set the displayport signal then ACK */
                            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlSetMuxSaftMode, NULL);

                            if (PD_DpUFPProcessConfigure(dpInstance) != 0U)
                            {
                                svdmRequest->requestResultStatus = (uint8_t)kCommandResult_VDMACK;

#if (defined PD_ALT_MODE_LOG) && (PD_ALT_MODE_LOG)
                                PRINTF("dp configure success\r\n");
#endif
                                if ((dpInstance->dpConfigure.bitFields.setConfig == (uint8_t)kDPConfig_DFPD) ||
                                    (dpInstance->dpConfigure.bitFields.setConfig == (uint8_t)kDPConfig_UFPD))
                                {
                                    PD_DpmAltModeCallback(dpInstance->pdHandle, PD_DPM_ALTMODE_DP_UFP_MODE_CONFIGURED,
                                                          NULL);
                                }
                            }
                            else
                            {
                                svdmRequest->requestResultStatus = (uint8_t)kCommandResult_VDMNAK;
                            }
                        }
                        else
                        {
                            svdmRequest->requestResultStatus = (uint8_t)kCommandResult_VDMNAK;
                        }
                        svdmRequest->vdoData  = NULL;
                        svdmRequest->vdoCount = 0;
                        break;
                    }
#endif

#if (defined PD_CONFIG_ALT_MODE_HOST_SUPPORT) && (PD_CONFIG_ALT_MODE_HOST_SUPPORT)
                    case (uint32_t)kVDM_Attention: /* DP DFP */
                    {
                        if ((PD_AltModeCheckModalOperationSupported(dpInstance->altModeHandle) != 0U) &&
                            (svdmRequest->vdmHeader.bitFields.objPos > 0U) &&
                            (svdmRequest->vdmHeader.bitFields.objPos <= dpInstance->dpConfig->modesCount) &&
                            ((((pd_dp_mode_obj_t *)((void *)dpInstance->dpConfig
                                                        ->modesList))[svdmRequest->vdmHeader.bitFields.objPos - 1U]
                                  .bitFields.portCap &
                              (uint8_t)kDPPortCap_DFPD) != 0U))
                        {
                            if (svdmRequest->vdoCount == 1U)
                            {
                                dpInstance->dpPartnerStatus.statusVal = svdmRequest->vdoData[0];
                                /* process DP status */
                                if (dpInstance->dpState == kDPMode_StatusUpdateDone)
                                {
                                    if ((dpInstance->dpPartnerStatus.bitFields.DFPDUFPDConnected &
                                         (uint8_t)kUFP_D_Connected) != 0U)
                                    {
                                        (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                                            dpInstance->dpBoardChipHandle, kDPPeripheal_ControlSetMuxSaftMode, NULL);
                                        if (PD_DpDFPConstructConfigure(dpInstance) == kStatus_PD_Success)
                                        {
                                            PD_DpTrigerCommand(dpInstance, (uint8_t)kDPVDM_Configure, 0);
                                        }
                                    }
                                }
                                else if (dpInstance->dpState == kDPMode_ConfigureDone)
                                {
                                    /* PD_DpModuleSetEvent(dpInstance, DP_TASK_EVENT_DFP_DP_CHECK_UFP_STATUS); */
#if (defined PD_ALT_MODE_LOG) && (PD_ALT_MODE_LOG)
                                    PRINTF("receive attention\r\n");
#endif
                                    PD_DpDFPProcessUFPstatus(dpInstance);
                                }
                                else
                                {
                                    /* don't process */
                                }
                            }
                        }
                        break;
                    }
#endif
                    default:
                        /* No action required. */
                        break;
                }
            }
            break;
        }

        case kAltMode_StructedVDMMsgSuccess:
        {
            /* ACK msg, DP DFP */
            /* DFP is doing ASM command */
            if (dpInstance->CommandDoing != 0U)
            {
                pd_svdm_command_result_t *svdmResult = (pd_svdm_command_result_t *)param;

                dpInstance->CommandDoing = 0;
                status                   = kStatus_PD_Success;
                switch (svdmResult->vdmCommand)
                {
#if (defined PD_CONFIG_ALT_MODE_HOST_SUPPORT) && (PD_CONFIG_ALT_MODE_HOST_SUPPORT)
                    case (uint8_t)kVDM_DiscoverModes:
                        dpInstance->dpSelfStatus.statusVal                   = 0;
                        dpInstance->dpSelfStatus.bitFields.DFPDUFPDConnected = (uint8_t)kDFP_D_Connected;
                        dpInstance->pdMsgReceivedVDOCount                    = svdmResult->vdoCount;
                        for (index = 0; index < svdmResult->vdoCount; ++index)
                        {
                            dpInstance->pdMsgReceivedBuffer[index] = svdmResult->vdoData[index];
                        }
                        dpInstance->pdVDMMsgReceivedHeader.structuredVdmHeaderVal =
                            svdmResult->vdmHeader.structuredVdmHeaderVal;
                        if (PD_DpDFPGetModesCheckHaveSupportedMode(dpInstance) == kStatus_PD_Success)
                        {
                            PD_DpTrigerCommand(dpInstance, (uint8_t)kVDM_EnterMode, 0);
                        }
                        break;

                    case (uint8_t)kVDM_EnterMode:
                        dpInstance->dpState = kDPMode_EnterDPDone;
                        PD_DpTrigerCommand(dpInstance, (uint8_t)kDPVDM_StatusUpdate, 0);
                        break;

                    case (uint8_t)kDPVDM_StatusUpdate:
                        dpInstance->dpState                   = kDPMode_StatusUpdateDone;
                        dpInstance->dpPartnerStatus.statusVal = svdmResult->vdoData[0];
                        if ((dpInstance->dpPartnerStatus.bitFields.DFPDUFPDConnected & (uint8_t)kUFP_D_Connected) != 0U)
                        {
                            /* if false, wait attention message */
                            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlSetMuxSaftMode, NULL);
                            if (PD_DpDFPConstructConfigure(dpInstance) == kStatus_PD_Success)
                            {
                                PD_DpTrigerCommand(dpInstance, (uint8_t)kDPVDM_Configure, 0);
                            }
                        }
                        break;

                    case (uint8_t)kDPVDM_Configure:
                    {
                        /* 1. configure as DP; 2. configure as USB (exit DP) */
                        if ((dpInstance->dpConfigure.bitFields.setConfig == (uint8_t)kDPConfig_DFPD) ||
                            (dpInstance->dpConfigure.bitFields.setConfig == (uint8_t)kDPConfig_UFPD))
                        {
                            dpInstance->dpState = kDPMode_ConfigureDone;
                            if ((dpInstance->dpConfigure.bitFields.configureUFPUPin == (uint8_t)kPinAssign_C) ||
                                (dpInstance->dpConfigure.bitFields.configureUFPUPin == (uint8_t)kPinAssign_E))
                            {
                                (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                                    dpInstance->dpBoardChipHandle, kDPPeripheal_ControlSetMuxDP4LANE,
                                    &dpInstance->dpConfigure.configureVal);
                            }
                            else
                            {
                                (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                                    dpInstance->dpBoardChipHandle, kDPPeripheal_ControlSetMuxDP2LANEUSB3,
                                    &dpInstance->dpConfigure.configureVal);
                            }
                            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlHPDReleaseLow, NULL);
                            /* PD_DpModuleSetEvent(dpInstance, DP_TASK_EVENT_DFP_DP_CHECK_UFP_STATUS); */
                            PD_DpDFPProcessUFPstatus(dpInstance);
#if (defined PD_ALT_MODE_LOG) && (PD_ALT_MODE_LOG)
                            PRINTF("dp configure success\r\n");
#endif
                            PD_DpmAltModeCallback(dpInstance->pdHandle, PD_DPM_ALTMODE_DP_DFP_MODE_CONFIGURED, NULL);
                        }
                        else if (dpInstance->dpConfigure.bitFields.setConfig == (uint8_t)kDPConfig_USB)
                        {
                            /* back the state */
                            dpInstance->dpState = kDPMode_StatusUpdateDone;
                            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlHPDSetLow, NULL);
                            (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                                dpInstance->dpBoardChipHandle, kDPPeripheal_ControlSetMuxUSB3Only, NULL);
                            PD_DpTrigerCommand(dpInstance, (uint8_t)kVDM_ExitMode, 0);
                        }
                        else
                        {
                            /* No action required. */
                        }
                        break;
                    }
                    case (uint8_t)kVDM_ExitMode:
                        (void)dpInstance->dpConfig->peripheralInterface->dpPeripheralControl(
                            dpInstance->dpBoardChipHandle, kDPPeripheal_ControlHPDSetLow, NULL);
                        dpInstance->dpState = kDPMode_Exited;
                        PD_DpmAltModeCallback(dpInstance->pdHandle, PD_DPM_ALTMODE_DP_DFP_MODE_UNCONFIGURED, NULL);
                        break;
#endif
#if (defined PD_CONFIG_ALT_MODE_SLAVE_SUPPORT) && (PD_CONFIG_ALT_MODE_SLAVE_SUPPORT)
                    case (uint8_t)kVDM_Attention:
                        PD_DpUFPHPDDetectPop(dpInstance, dpInstance->attnStatusHPD);
                        /* check next HPD queue item */
                        PD_DpUFPTriggerTask(dpInstance);
                        break;
#endif
                    default:
                        /* No action required. */
                        break;
                }
            }
            break;
        }

        case kAltMode_StructedVDMMsgFail:
        {
            /* NAK/Not_supported/BUSY/time_out, DP DFP */
            /* DFP is doing ASM command */
            if (dpInstance->CommandDoing != 0U)
            {
                pd_svdm_command_result_t *svdmResult = (pd_svdm_command_result_t *)param;

                status                   = kStatus_PD_Success;
                dpInstance->CommandDoing = 0;
                switch (svdmResult->vdmCommand)
                {
#if defined(PD_CONFIG_ALT_MODE_HOST_SUPPORT) && (PD_CONFIG_ALT_MODE_HOST_SUPPORT)
                    case (uint8_t)kVDM_DiscoverModes:
                    case (uint8_t)kVDM_EnterMode:
                    case (uint8_t)kVDM_ExitMode:
                    case (uint8_t)kDPVDM_StatusUpdate:
                    case (uint8_t)kDPVDM_Configure:
                        if (svdmResult->vdmCommandResult == (uint8_t)kCommandResult_VDMNAK)
                        {
                            /* don't support this command */
                            return status;
                        }
                        PD_DpDelayRetryCommand(dpInstance, svdmResult->vdmCommand, PD_ALT_MODE_ERROR_RETRY_WAIT_TIME);
                        break;
#endif
#if defined(PD_CONFIG_ALT_MODE_SLAVE_SUPPORT) && (PD_CONFIG_ALT_MODE_SLAVE_SUPPORT)
                    case (uint8_t)kVDM_Attention:
                        PD_DpUFPTriggerTask(dpInstance);
                        break;
#endif
                    default:
                        /* No action required. */
                        break;
                }
            }
            break;
        }

        case kAltMode_UnstructedVDMMsgReceived:
        case kAltMode_UnstructedVDMMsgSentResult:
            /* DP doesn't have this type message */
            break;

        default:
            /* No action required. */
            break;
    }
    return status;
}

#if (defined PD_CONFIG_ALT_MODE_SLAVE_SUPPORT) && (PD_CONFIG_ALT_MODE_SLAVE_SUPPORT)
static uint8_t PD_DpCtrlReadyToSendHpdDetection(pd_alt_mode_dp_t *dpInstance)
{
    return ((dpInstance->dpState >= kDPMode_EnterDPDone) && (dpInstance->CommandDoing == 0U)) ? 1U : 0U;
}
#endif

/* 1. send msg from self.
 * wait for send result callback (timer); wait for ACK reply msg callback.
 *
 * 2. ACK received VDM msg.
 * ACK is sent in the callback -> task wait the send result (timer);
 *
 * 3. HPD
 * Dock board detect HPD, Host board driver HPD.
 */
void PD_DPTask(void *taskParam)
{
    pd_alt_mode_dp_t *dpInstance = (pd_alt_mode_dp_t *)taskParam;
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    if (dpInstance->triggerCommand != 0U)
    {
        dpInstance->triggerCommand = 0;
        OSA_EXIT_CRITICAL();

        if (dpInstance->retryCount > 0U)
        {
            dpInstance->retryCount--;
            (void)PD_DpSendCommand(dpInstance, dpInstance->dpCommand);
        }
        else
        {
            /* do hard reset */
            if (PD_Command(dpInstance->pdHandle, (uint32_t)PD_DPM_CONTROL_HARD_RESET, NULL) != kStatus_PD_Success)
            {
                return;
            }
        }
    }
    else
    {
        OSA_EXIT_CRITICAL();

#if (defined PD_CONFIG_ALT_MODE_SLAVE_SUPPORT) && (PD_CONFIG_ALT_MODE_SLAVE_SUPPORT)
        /* HPD detection */
        uint8_t getState;
        (void)PD_Control(dpInstance->pdHandle, (uint32_t)PD_CONTROL_GET_DATA_ROLE, &getState);
        if (getState == (uint8_t)kPD_DataRoleUFP)
        {
            getState = (uint8_t)PD_DpUFPHPDDetectPeek(dpInstance);
            /* detect queue has items and can send in current state */
            if ((getState != (uint8_t)kDPHPDDetect_Empty) && (PD_DpCtrlReadyToSendHpdDetection(dpInstance) != 0U))
            {
                PD_DpUFPGetStatus(dpInstance);
                PD_DpTrigerCommand(dpInstance, (uint8_t)kVDM_Attention, 0);
            }
        }
#endif
    }
}

#endif
#endif
