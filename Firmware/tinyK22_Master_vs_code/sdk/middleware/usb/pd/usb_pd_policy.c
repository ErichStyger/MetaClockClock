/*
 * Copyright 2015 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_phy.h"
#include "usb_pd_timer.h"
#include "usb_pd_interface.h"
#if (defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT)
#include "usb_pd_alt_mode.h"
#endif
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
#include "usb_pd_auto_policy.h"
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY_LOG) && (PD_CONFIG_ENABLE_AUTO_POLICY_LOG)
#include "fsl_debug_console.h"
#endif
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if defined(PD_CONFIG_SRC_AUTO_DISCOVER_CABLE_PLUG) && (PD_CONFIG_SRC_AUTO_DISCOVER_CABLE_PLUG)
#undef PD_CONFIG_CABLE_COMMUNICATION_ENABLE
#define PD_CONFIG_CABLE_COMMUNICATION_ENABLE (1)
#endif

#if (defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE))
#undef PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE
#define PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE (1)
#endif

#define N_CAPS_COUNT                            (50U)
#define N_HARD_RESET_COUNT                      (2U)
#define N_DISCOVER_IDENTITY_COUNTER             (20U)
#define N_DPM_COMMAND_RETRY_COUNTER             (20U)
#define PD_EXTENDED_SRC_CAP_DATA_LENGTH         (23U)
#define PD_EXTENDED_STATUS_MSG_DATA_LENGTH      (3U)
#define PD_EXTENDED_BATTERY_CAP_MSG_DATA_LENGTH (9U)
#define PD_EXTENDED_PPS_STATUS_MSG_DATA_LENGTH  (4U)

#define MSG_DATA_BUFFER ((uint32_t *)(&(pdInstance->receivedData[1])))
#define MSG_DATA_HEADER (pdInstance->receivedData[0] >> 16)

#define PD_NOT_SUPPORT_REPLY_MSG ((pdInstance->revision >= PD_SPEC_REVISION_30) ? kPD_MsgNotSupported : kPD_MsgReject)

#define VDM_ID_HEADER_VDO_PASSIVE_CABLE_VAL (0x03U)
#define VDM_ID_HEADER_VDO_ACTIVE_CABLE_VAL  (0x04U)

typedef enum _trigger_event
{
    PSM_TRIGGER_NONE,
    PSM_TRIGGER_NON_START,
    PSM_TRIGGER_DPM_MSG,
    PSM_TRIGGER_PD_MSG,
    PSM_TRIGGER_RECEIVE_HARD_RESET,
} trigger_event_t;

typedef struct _psm_trigger_info
{
    uint32_t *pdMsgDataBuffer;
    uint32_t pdExtMsgLength;
    pd_structured_vdm_header_t vdmHeader;
    pd_msg_header_t msgHeader;
    pd_command_t dpmMsg;
    start_of_packet_t pdMsgSop;
    message_type_t pdMsgType;
    trigger_event_t triggerEvent;
    uint8_t vdmMsgType;
    uint8_t pdMsgDataLength; /*!< the count of Number Data Object */
} psm_trigger_info_t;

typedef enum _pd_state_machine_state
{
    kSM_None = 0,
    kSM_Continue,
    kSM_WaitEvent,
    kSM_ErrorRecovery,
    kSM_Detach,
} pd_state_machine_state_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void PD_PsmTransitionOnMsgSendError(pd_instance_t *pdInstance,
                                           pd_psm_state_t interruptedState,
                                           pd_psm_state_t errorState);
static uint8_t PD_MsgSendMsgCommonTransition(pd_instance_t *pdInstance,
                                             start_of_packet_t sop,
                                             message_type_t msgType,
                                             uint32_t dataLength,
                                             uint8_t *dataBuffer,
                                             pd_psm_state_t successNextState,
                                             pd_psm_state_t interruptedState,
                                             pd_psm_state_t errorState);
static inline uint8_t PD_MsgSendDataTransition(pd_instance_t *pdInstance,
                                               message_type_t msgType,
                                               uint32_t doCount,
                                               uint32_t *dos,
                                               pd_psm_state_t successNextState,
                                               pd_psm_state_t interruptedState,
                                               pd_psm_state_t errorState);
static inline uint8_t PD_MsgSendControlTransition(pd_instance_t *pdInstance,
                                                  message_type_t msgType,
                                                  pd_psm_state_t successNextState,
                                                  pd_psm_state_t interruptedState,
                                                  pd_psm_state_t errorState);
#if defined(PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
static inline uint8_t PD_MsgSendExtTransition(pd_instance_t *pdInstance,
                                              start_of_packet_t sop,
                                              message_type_t msgType,
                                              uint32_t dataLength,
                                              uint8_t *dataBuffer,
                                              pd_psm_state_t successNextState,
                                              pd_psm_state_t interruptedState,
                                              pd_psm_state_t errorState);
#endif
static inline uint8_t PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(pd_instance_t *pdInstance,
                                                                         message_type_t msgType,
                                                                         pd_psm_state_t successNextState);
#if defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
static inline uint8_t PD_PsmSendControlTransitionWithErrorRecovery(pd_instance_t *pdInstance,
                                                                   message_type_t msgType,
                                                                   pd_psm_state_t successNextState);
#endif
static inline uint8_t PD_PsmSendControlTransitionWithHardReset(pd_instance_t *pdInstance,
                                                               message_type_t msgType,
                                                               pd_psm_state_t successNextState);
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
static uint8_t PD_PsmSendControlTransitionWithTry(pd_instance_t *pdInstance,
                                                  message_type_t msgType,
                                                  pd_psm_state_t successNextState,
                                                  pd_psm_state_t interruptedState,
                                                  pd_psm_state_t errorState);
#endif
static uint8_t PD_PsmSendControlTransitionWithSendResponserTimeOut(pd_instance_t *pdInstance,
                                                                   message_type_t msgType,
                                                                   pd_psm_state_t successNextState,
                                                                   pd_psm_state_t interruptedState,
                                                                   pd_psm_state_t errorState);
static void PD_FRSControl(pd_instance_t *pdInstance, uint8_t enable);
static void PD_PsmReset(pd_instance_t *pdInstance);
static void PD_PsmSetNormalPower(pd_instance_t *pdInstance);
static void PD_PsmCheckRevision(pd_instance_t *pdInstance, pd_msg_header_t msgHeader);
#if (defined PD_CONFIG_PD3_PPS_ENABLE) && (PD_CONFIG_PD3_PPS_ENABLE)
static uint8_t PD_PsmSinkIsPPSRDO(pd_instance_t *pdInstance);
static uint8_t PD_PsmSourceIsPPSRDO(pd_instance_t *pdInstance);
#endif
#if ((defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)) || \
    ((defined PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE))
static uint8_t PD_PsmIsDualRole(pd_instance_t *pdInstance);
#endif
#if (defined PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
static uint32_t *PD_PsmGetSourcePDOs(pd_instance_t *pdInstance);
#endif
#if (defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
static uint32_t *PD_PsmGetSinkPDOs(pd_instance_t *pdInstance);
#endif
static void PD_PsmClearStateFlags(pd_instance_t *pdInstance);
static void PD_PsmDisconnect(pd_instance_t *pdInstance);
static void PD_PsmConnect(pd_instance_t *pdInstance);
static pd_state_machine_state_t PD_PsmDisconnectCheck(pd_instance_t *pdInstance, pd_state_machine_state_t state);
#if (defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
static uint8_t PD_PsmCheckLessOrEqualVsafe5v(pd_instance_t *pdInstance);
static void PD_PsmCheckFRS5V(pd_instance_t *pdInstance);
#endif
static uint8_t PD_DpmGetMsg(pd_instance_t *pdInstance);
static void PD_DpmClearMsg(pd_instance_t *pdInstance, pd_command_t id);
static void PD_DpmSendMsg(pd_handle pdHandle, uint8_t id);
#if ((defined PD_CONFIG_EXTENDED_CHUNK_LAYER_SUPPORT) && (PD_CONFIG_EXTENDED_CHUNK_LAYER_SUPPORT))
static void PD_MsgChunkingLayerResetFlags(pd_instance_t *pdInstance);
static uint8_t PD_PsmChunkingLayerRXTimerWaiting(pd_instance_t *pdInstance);
static uint8_t PD_PsmChunkingLayerRXStateMachine(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo);
#if 0
static uint8_t PD_PsmChunkingLayerCheckSentDone(pd_instance_t *pdInstance);
static pd_status_t PD_PsmChunkingLayerTXStateMachine(pd_instance_t *pdInstance,
                                                     start_of_packet_t sop,
                                                     message_type_t msgType,
                                                     uint8_t *dataBuffer,
                                                     uint32_t dataLength,
                                                     psm_trigger_info_t *triggerInfo);
#endif
#endif
#if (defined PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
static uint8_t PD_PsmCheckInAltMode(pd_instance_t *pdInstance, start_of_packet_t sop);
#endif
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY_LOG) && (PD_CONFIG_ENABLE_AUTO_POLICY_LOG)
static void PD_PsmPrintAutoPolicyReplySwapRequestLog(pd_instance_t *pdInstance, uint8_t message);
static void PD_PsmPrintAutoPolicyStartSwapLog(pd_instance_t *pdInstance, uint8_t message);
#endif
#if (defined PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
static uint8_t PD_PsmAutoRequestVconnSwap(pd_instance_t *pdInstance);
#endif
#if (defined PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
static uint8_t PD_PsmAutoRequestDataSwap(pd_instance_t *pdInstance);
#endif
#if (defined PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
static uint8_t PD_PsmAutoRequestPowerSwap(pd_instance_t *pdInstance);
#endif
static void PD_PsmSetAutoPolicyState(pd_instance_t *pdInstance, pd_auto_policy_state_t newState);
static uint8_t PD_PsmGetExternalPowerState(pd_instance_t *pdInstance);
static uint8_t PD_PsmReadyAutoPolicyProcess(pd_instance_t *pdInstance);
static void PD_PsmReadyAutoPolicyResult(pd_instance_t *pdInstance, pd_command_result_t result);
static void PD_PsmExternalPowerChange(pd_instance_t *pdInstance);
#endif
#if (defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
static void PD_StateWaitReplyExtDataProcess(pd_instance_t *pdInstance,
                                            psm_trigger_info_t *triggerInfo,
                                            message_type_t requiredMsg,
                                            pd_dpm_callback_event_t successCallbackEvent,
                                            pd_dpm_callback_event_t failCallbackEvent,
                                            uint8_t *didNothingStepB);
#endif
static uint8_t PD_PsmStartCommand(pd_instance_t *pdInstance, pd_command_t command, uint8_t isInitiator);
static void PD_PsmCommandFail(pd_instance_t *pdInstance, pd_command_t command);
static uint8_t PD_PsmPrimaryStateProcessDpmMsg(pd_instance_t *pdInstance,
                                               pd_psm_state_t *returnNewState,
                                               psm_trigger_info_t *triggerInfo);
static pd_psm_state_t PD_PsmSinkHardResetfunction(pd_instance_t *pdInstance);
static uint8_t PD_PsmPrimaryStateProcessPdMsg(pd_instance_t *pdInstance,
                                              pd_psm_state_t *returnNewState,
                                              psm_trigger_info_t *triggerInfo);
static void PD_PsmEndCommand(pd_instance_t *pdInstance);
static void PD_PsmSinkAndSourceRdyProcessDpmMessage(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo);
#if (defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
static void PD_PsmSinkRdyProcessDpmMessage(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo);
#endif
#if (defined PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
static void PD_PsmSourceRdyProcessDpmMessage(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo);
#endif
static void PD_PsmRdyProcessPdMessage(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo);
static uint8_t PD_PsmCanPendingReceive(pd_instance_t *pdInstance);
static void PD_PsmProcessImportEventBeforeNextStateMachine(pd_instance_t *pdInstance);
static uint8_t PD_PsmNoResponseHardResetCountCheck(pd_instance_t *pdInstance);
#if (defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)) || \
    (defined(PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)) ||   \
    (defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT))
static void PD_PsmStateWaitReply(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo, uint8_t *didNothingStepB);
static void PD_PsmEvaluateSwap(pd_instance_t *pdInstance,
                               pd_dpm_callback_event_t requestEvent,
                               pd_psm_state_t acceptState,
                               pd_psm_state_t rejectState);
#endif
#if defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
static void PD_PsmPowerSwapSinkSourceTransitionOff(pd_instance_t *pdInstance,
                                                   psm_trigger_info_t *triggerInfo,
                                                   pd_psm_state_t newState,
                                                   uint8_t *didNothingStepB);
static void PD_PsmPowerSwapSinkOpenVbus(pd_instance_t *pdInstance, pd_dpm_callback_event_t callbackEvent);
#endif
#if defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
static void PD_PsmPowerSwapAssertRdRp(pd_instance_t *pdInstance, pd_power_role_t powerRole, pd_psm_state_t nextState);
#endif
#if (defined(PD_CONFIG_VBUS_ALARM_SUPPORT) && (PD_CONFIG_VBUS_ALARM_SUPPORT)) || \
    (defined(PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE))
static inline void PD_PsmSetVbusAlarmAndSinkDisconnect(pd_instance_t *pdInstance);
#endif
static void PD_PsmResetStateWhenEnterReadyState(pd_instance_t *pdInstance);
#if defined(PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
static void PD_StateSendReplyExtDataTransition(pd_instance_t *pdInstance, message_type_t msgType);
#endif
static void PD_PsmCheckChunkedFeature(pd_instance_t *pdInstance, pd_rdo_t rdo);
#if (defined PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
static void PD_PsmTurnOffVconnAndVbus(pd_instance_t *pdInstance, pd_vbus_power_progress_t powerState);
#endif
static inline void PD_PsmEnterIdleState(pd_instance_t *pdInstance);
static inline void PD_PsmEnterHardResetState(pd_instance_t *pdInstance);
static inline void PD_PsmEnterSendSoftResetState(pd_instance_t *pdInstance);
#if (defined PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
static inline void PD_PsmEnterSrcStartUpState(pd_instance_t *pdInstance);
static inline void PD_PsmEnterImplicitCableDiscoveryState(pd_instance_t *pdInstance);
static inline void PD_PsmEnterImplicitCableDiscoveryState(pd_instance_t *pdInstance);
static inline void PD_PsmEnterSrcSendCapsState(pd_instance_t *pdInstance);
static inline void PD_PsmEnterSrcNegotiateCapState(pd_instance_t *pdInstance);
static inline void PD_PsmEnterSrcTransitionSupplyState(pd_instance_t *pdInstance);
static inline void PD_PsmEnterSrcCapResponseState(pd_instance_t *pdInstance);
static inline void PD_PsmEnterSrcTransitionToDefaultState(pd_instance_t *pdInstance);
static inline void PD_PsmEnterGiveSrcCapState(pd_instance_t *pdInstance);
#endif
static inline void PD_PsmEnterGetSinkOrSourceCapState(pd_instance_t *pdInstance, pd_psm_state_t prevState);
#if (defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)) || \
    ((defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE))
static inline void PD_PsmEnterGiveSinkCapState(pd_instance_t *pdInstance);
#endif
#if (defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
static inline void PD_PsmEnterSnkStartUpState(pd_instance_t *pdInstance);
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
void PD_AutoSinkNegotiation(pd_instance_t *pdInstance, pd_rdo_t *rdoRequest);
#endif
static inline void PD_PsmEnterSnkEvaluateCapState(pd_instance_t *pdInstance);
static inline void PD_PsmEnterSnkReadyState(pd_instance_t *pdInstance);
static inline void PD_PsmEnterSnkTransitionToDefaultState(pd_instance_t *pdInstance);
#endif
static pd_state_machine_state_t PD_PsmEnterState(pd_instance_t *pdInstance);
static pd_state_machine_state_t PD_PsmGetNewestEvent(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo);
static uint8_t PD_PsmCheckTimeOutHardReset(pd_instance_t *pdInstance);
static uint8_t PD_PsmProcessHardResetState(pd_instance_t *pdInstance);
static uint8_t PD_PsmProcessSendSoftResetState(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo);
static void PD_PsmCheckUnexpectedReceivedMsg(pd_instance_t *pdInstance,
                                             trigger_event_t triggerEvent,
                                             uint8_t *didNothingStepB);
static uint8_t PD_PsmProcessReadyState(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo);
#if (defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)) || \
    ((defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE))
static inline uint8_t PD_PsmGetTypeCCurrent(pd_instance_t *pdInstance);
#endif
static pd_state_machine_state_t PD_PsmProcessState(pd_instance_t *pdInstance);
#if ((defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)) || \
    (defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE))
static void PD_PsmEndVdmCommand(pd_instance_t *pdInstance, pd_psm_state_t secondState);
#endif
#if (defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)
static pd_psm_state_t PD_PsmVdmCheckkVDMBusyTimer(pd_instance_t *pdInstance, pd_psm_state_t newState);
static pd_psm_state_t PD_PsmVdmIdleProcessDpmMessage(pd_instance_t *pdInstance,
                                                     start_of_packet_t statIndex,
                                                     psm_trigger_info_t *triggerInfo);
#if 0
static uint8_t PDPsmVdmEnterExitModePdMsgProcess(pd_instance_t *pdInstance,
                                                 uint8_t statIndex,
                                                 psm_trigger_info_t *triggerInfo,
                                                 pd_svdm_command_request_t *commandVdmRequest);
#endif
static void PD_PsmVdmIdleProcessPdMessage(pd_instance_t *pdInstance,
                                          start_of_packet_t statIndex,
                                          psm_trigger_info_t *triggerInfo);
#endif
#if (defined(PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)) || \
    (defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE))
static pd_psm_state_t PD_PsmStructuredVdmInitiatorRequest(pd_instance_t *pdInstance,
                                                          start_of_packet_t sopIndex,
                                                          uint16_t svid,
                                                          uint8_t position,
                                                          pd_vdm_command_t command,
                                                          uint8_t vdo_count,
                                                          uint32_t *vdos);
static pd_psm_state_t PD_PsmVdmResponseHandler(pd_instance_t *pdInstance,
                                               start_of_packet_t sop,
                                               uint8_t vdmMsgType,
                                               psm_trigger_info_t *triggerInfo,
                                               tTimer_t timr,
                                               pd_psm_state_t ackState,
                                               pd_psm_state_t nakState);
#if defined(PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
static void PD_PsmSecondaryStateHandlerTerminate(pd_instance_t *pdInstance, start_of_packet_t sop);
#endif
static uint8_t PD_PsmSecondaryStateHandler(pd_instance_t *pdInstance,
                                           start_of_packet_t statIndex,
                                           psm_trigger_info_t *triggerInfo);
#endif
#if (defined PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
static uint8_t PD_CheckWhetherInitiateCableDiscoveryIdentityOrNot(pd_instance_t *pdInstance);
#endif
static pd_state_machine_state_t PD_PsmStateMachine(pd_instance_t *pdInstance);

#if defined(PD_CONFIG_PHY_LOW_POWER_LEVEL) && (PD_CONFIG_PHY_LOW_POWER_LEVEL)
static void PD_LowPowerTransition(pd_instance_t *pdInstance);
#endif
static pd_status_t PD_InitiateAmsCheck(pd_instance_t *pdInstance, pd_command_t command);
static pd_status_t PD_DpmAppCallback(pd_instance_t *pdInstance,
                                     pd_dpm_callback_event_t event,
                                     void *param,
                                     uint8_t done);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*! ***************************************************************************
   Send message transition functions
******************************************************************************/
static void PD_PsmTransitionOnMsgSendError(pd_instance_t *pdInstance,
                                           pd_psm_state_t interruptedState,
                                           pd_psm_state_t errorState)
{
#if ((defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)) || \
    ((defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT))
    /* MISRA C:2012 Rule 14.3 */
    if (
#if ((defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE))
        ((PD_MsgRecvPending(pdInstance) != 0U) &&
         ((pdInstance->receivedSop == kPD_MsgSOP) &&
          (((MSG_DATA_HEADER & PD_MSG_HEADER_MESSAGE_TYPE_MASK) >> PD_MSG_HEADER_MESSAGE_TYPE_POS) ==
           ((uint8_t)kPD_MsgVendorDefined & PD_MSG_TYPE_VALUE_MASK))))
#if ((defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT))
        ||
#endif
#endif
#if ((defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT))
        ((PD_MsgRecvPending(pdInstance) != 0U) && (pdInstance->receivedSop == kPD_MsgSOP) &&
         ((MSG_DATA_HEADER & PD_MSG_HEADER_EXTENDED_MASK) != 0U))
#endif
    )
    {
        /* re-execute the psmCurState */
        if (pdInstance->psmCurState != PSM_INTERRUPTED_REQUEST)
        {
            pdInstance->psmInterruptedState = pdInstance->psmCurState;
            pdInstance->psmCurState         = PSM_INTERRUPTED_REQUEST;
        }
        pdInstance->psmNewState = PSM_INTERRUPTED_REQUEST;
    }
#endif
    if (PD_MsgRecvPending(pdInstance) != 0U)
    {
        pdInstance->psmNewState = interruptedState;
    }
    else
    {
        pdInstance->psmNewState = errorState;
    }
    return;
}

static uint8_t PD_MsgSendMsgCommonTransition(pd_instance_t *pdInstance,
                                             start_of_packet_t sop,
                                             message_type_t msgType,
                                             uint32_t dataLength,
                                             uint8_t *dataBuffer,
                                             pd_psm_state_t successNextState,
                                             pd_psm_state_t interruptedState,
                                             pd_psm_state_t errorState)
{
    pd_status_t result = kStatus_PD_Error;

#if defined(PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
    if ((((uint8_t)msgType & PD_MSG_TYPE_BITS_MASK) == PD_MSG_CONTROL_TYPE_MASK) ||
        (((uint8_t)msgType & PD_MSG_TYPE_BITS_MASK) == PD_MSG_DATA_TYPE_MASK))
#endif
    {
        result = PD_MsgSend(pdInstance, sop, msgType, 2U + (dataLength * 4U), (uint8_t *)dataBuffer);
    }
#if defined(PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
    else
    {
        /* dataSize for an unchunked message and only one chunked message equals to dataLength */
        result = PD_MsgSendExtendedMsg(pdInstance, sop, msgType, dataLength, dataBuffer, (uint16_t)dataLength, 0, 0);
    }
#endif

    if (result == kStatus_PD_Success)
    {
        if (successNextState != PE_PSM_STATE_NO_CHANGE)
        {
            pdInstance->psmNewState = successNextState;
        }
        return 1; /* success */
    }

    PD_PsmTransitionOnMsgSendError(pdInstance, interruptedState, errorState);

    return 0; /* fail */
}

static inline uint8_t PD_MsgSendDataTransition(pd_instance_t *pdInstance,
                                               message_type_t msgType,
                                               uint32_t doCount,
                                               uint32_t *dos,
                                               pd_psm_state_t successNextState,
                                               pd_psm_state_t interruptedState,
                                               pd_psm_state_t errorState)
{
    return PD_MsgSendMsgCommonTransition(pdInstance, kPD_MsgSOP, msgType, doCount, (uint8_t *)dos, successNextState,
                                         interruptedState, errorState);
}

static inline uint8_t PD_MsgSendControlTransition(pd_instance_t *pdInstance,
                                                  message_type_t msgType,
                                                  pd_psm_state_t successNextState,
                                                  pd_psm_state_t interruptedState,
                                                  pd_psm_state_t errorState)
{
    return PD_MsgSendMsgCommonTransition(pdInstance, kPD_MsgSOP, msgType, 0, NULL, successNextState, interruptedState,
                                         errorState);
}

#if defined(PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
static inline uint8_t PD_MsgSendExtTransition(pd_instance_t *pdInstance,
                                              start_of_packet_t sop,
                                              message_type_t msgType,
                                              uint32_t dataLength,
                                              uint8_t *dataBuffer,
                                              pd_psm_state_t successNextState,
                                              pd_psm_state_t interruptedState,
                                              pd_psm_state_t errorState)
{
    return PD_MsgSendMsgCommonTransition(pdInstance, sop, msgType, dataLength, dataBuffer, successNextState,
                                         interruptedState, errorState);
}
#endif

static inline uint8_t PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(pd_instance_t *pdInstance,
                                                                         message_type_t msgType,
                                                                         pd_psm_state_t successNextState)
{
    return PD_MsgSendControlTransition(pdInstance, msgType, successNextState, PSM_CHECK_ASYNC_RX, PSM_SEND_SOFT_RESET);
}

#if defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
static inline uint8_t PD_PsmSendControlTransitionWithErrorRecovery(pd_instance_t *pdInstance,
                                                                   message_type_t msgType,
                                                                   pd_psm_state_t successNextState)
{
    return PD_MsgSendControlTransition(pdInstance, msgType, successNextState, PSM_EXIT_TO_ERROR_RECOVERY_WITH_DELAY,
                                       PSM_EXIT_TO_ERROR_RECOVERY_WITH_DELAY);
}
#endif

static inline uint8_t PD_PsmSendControlTransitionWithHardReset(pd_instance_t *pdInstance,
                                                               message_type_t msgType,
                                                               pd_psm_state_t successNextState)
{
    return PD_MsgSendControlTransition(pdInstance, msgType, successNextState, PSM_HARD_RESET, PSM_HARD_RESET);
}

#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
/* 0 - fail; 1 -success */
static uint8_t PD_PsmSendControlTransitionWithTry(pd_instance_t *pdInstance,
                                                  message_type_t msgType,
                                                  pd_psm_state_t successNextState,
                                                  pd_psm_state_t interruptedState,
                                                  pd_psm_state_t errorState)
{
    uint8_t retryCount = 5;

    do
    {
        if (PD_MsgSend(pdInstance, kPD_MsgSOP, msgType, 2, NULL) == kStatus_PD_Success)
        {
            break; /* success */
        }
    } while (--retryCount > 0U);

    if (retryCount == 0U)
    {
        PD_PsmTransitionOnMsgSendError(pdInstance, interruptedState, errorState);
        return 0;
    }
    else
    {
        pdInstance->psmNewState = successNextState;
    }
    return 1;
}
#endif

static uint8_t PD_PsmSendControlTransitionWithSendResponserTimeOut(pd_instance_t *pdInstance,
                                                                   message_type_t msgType,
                                                                   pd_psm_state_t successNextState,
                                                                   pd_psm_state_t interruptedState,
                                                                   pd_psm_state_t errorState)
{
    if (PD_MsgSendControlTransition(pdInstance, msgType, successNextState, interruptedState, errorState) != 0U)
    {
        (void)PD_TimerStart(pdInstance, tSenderResponseTimer, T_SENDER_RESPONSE);
        return 1;
    }
    return 0;
}

/*! ***************************************************************************
   Other small functions
******************************************************************************/

static void PD_FRSControl(pd_instance_t *pdInstance, uint8_t enable)
{
    uint8_t control;
#if (defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
    if (enable != 0U)
    {
        if (pdInstance->revision >= PD_SPEC_REVISION_30)
        {
            control = 1;
            (void)PD_PhyControl(pdInstance, PD_PHY_CONTROL_FR_SWAP, &control);
        }
    }
    else
#endif
    {
        control = 0;
        (void)PD_PhyControl(pdInstance, PD_PHY_CONTROL_FR_SWAP, &control);
    }
}

static void PD_PsmReset(pd_instance_t *pdInstance)
{
#if (defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)
    uint8_t uint8Tmp;
#endif

#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
    pdInstance->vconnSwapResult                   = kCommandResult_None;
    pdInstance->drSwapResult                      = kCommandResult_None;
    pdInstance->swapToSnkSrcCapReceived           = 0;
    pdInstance->portPartnerDrSwapToUFPRejected    = 0;
    pdInstance->portPartnerDrSwapToDFPRejected    = 0;
    pdInstance->portPartnerPrSwapToSinkRejected   = 0;
    pdInstance->portPartnerPrSwapToSourceRejected = 0;
    pdInstance->portPartnerVconnSwapToOnRejected  = 0;
    pdInstance->portPartnerVconnSwapToOffRejected = 0;
#endif
    pdInstance->revision                 = PD_CONFIG_REVISION;
    pdInstance->psmGotoMinTx             = 0;
    pdInstance->psmGotoMinRx             = 0;
    pdInstance->psmHardResetNeedsVSafe0V = 0;
    pdInstance->psmPresentlyPdConnected  = 0;
    pdInstance->psmCablePlugResetNeeded  = 0;
    pdInstance->commandEvaluateResult    = kCommandResult_None;
#if defined(PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE) && (PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE)
    pdInstance->commandSrcOwner = 0;
#endif

#if (defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)
    pdInstance->psmVdmActiveModeValidMask = 0;
    for (uint8Tmp = 0; uint8Tmp < PSM_SECONDARY_STATE_COUNT; uint8Tmp++)
    {
        pdInstance->psmSecondaryState[uint8Tmp]    = PSM_IDLE;
        pdInstance->psmNewSecondaryState[uint8Tmp] = PSM_UNKNOWN;
    }
#endif

    PD_FRSControl(pdInstance, 0);
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
    pdInstance->frsEnabled           = 0;
    pdInstance->frSignaledWaitFrSwap = 0;
#endif
    PD_TimerCancelAllTimers(pdInstance, tSenderResponseTimer, _tMaxPSMTimer);
}

static void PD_PsmSetNormalPower(pd_instance_t *pdInstance)
{
    switch (pdInstance->psmCurState)
    {
        /* the follow state is stable state (may stay long time) */
        case PSM_PE_SRC_READY:
        case PSM_PE_SNK_READY:
        case PSM_PE_SNK_WAIT_FOR_CAPABILITIES:
        case PSM_PE_SRC_SEND_CAPABILITIES:
        case PSM_PE_SRC_DISABLED:
        case PSM_EXIT_TO_ERROR_RECOVERY:
        case PSM_EXIT_TO_ERROR_RECOVERY_WITH_DELAY:
#if (defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
            if (pdInstance->inProgress != kVbusPower_InFRSwap)
#endif
            {
                PD_ConnectSetPowerProgress(pdInstance, kVbusPower_Stable);
            }
            break;

        default:
            /* No action required. */
            break;
    }
}

static void PD_PsmCheckRevision(pd_instance_t *pdInstance, pd_msg_header_t msgHeader)
{
    if (pdInstance->revision > msgHeader.bitFields.specRevision)
    {
        pdInstance->revision = (uint8_t)msgHeader.bitFields.specRevision;
        (void)PD_PhyControl(pdInstance, PD_PHY_SET_MSG_HEADER_INFO, NULL);
    }
}

#if (defined PD_CONFIG_PD3_PPS_ENABLE) && (PD_CONFIG_PD3_PPS_ENABLE)
/* for sink */
static uint8_t PD_PsmSinkIsPPSRDO(pd_instance_t *pdInstance)
{
    uint8_t rdoIndex = 0U;
    if (pdInstance->rdoRequest.bitFields.objectPosition < 1U)
    {
        return 0;
    }
    rdoIndex = (uint8_t)pdInstance->rdoRequest.bitFields.objectPosition - 1U;

    if (rdoIndex < pdInstance->partnerSourcePDOsCount)
    {
        if ((pdInstance->partnerSourcePDOs[rdoIndex].commonPDO.pdoType == (uint8_t)kPDO_APDO) &&
            (pdInstance->partnerSourcePDOs[rdoIndex].apdoPDO.APDOType == (uint8_t)kAPDO_PPS))
        {
            return 1;
        }
    }
    return 0;
}

/* for source */
static uint8_t PD_PsmSourceIsPPSRDO(pd_instance_t *pdInstance)
{
    uint8_t rdoIndex = 0U;
    if (pdInstance->partnerRdoRequest.bitFields.objectPosition < 1U)
    {
        return 0;
    }
    rdoIndex = (uint8_t)pdInstance->partnerRdoRequest.bitFields.objectPosition - 1U;

    if (rdoIndex < pdInstance->pdPowerPortConfig->sourceCapCount)
    {
        pd_source_pdo_t sourcePDO;
        sourcePDO.PDOValue = pdInstance->pdPowerPortConfig->sourceCaps[rdoIndex];
        if ((sourcePDO.commonPDO.pdoType == (uint8_t)kPDO_APDO) && (sourcePDO.apdoPDO.APDOType == (uint8_t)kAPDO_PPS))
        {
            return 1;
        }
    }
    return 0;
}

#endif

#if ((defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)) || \
    ((defined PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE))
static uint8_t PD_PsmIsDualRole(pd_instance_t *pdInstance)
{
    if ((pdInstance->pdPowerPortConfig->typecRole == (uint8_t)kPowerConfig_SinkDefault) ||
        (pdInstance->pdPowerPortConfig->typecRole == (uint8_t)kPowerConfig_SourceDefault) ||
        (pdInstance->pdPowerPortConfig->typecRole == (uint8_t)kPowerConfig_DRPToggling) ||
        (pdInstance->pdPowerPortConfig->typecRole == (uint8_t)kPowerConfig_DRPSourcingDevice) ||
        (pdInstance->pdPowerPortConfig->typecRole == (uint8_t)kPowerConfig_DRPSinkingHost))
    {
        return 1;
    }
    return 0;
}
#endif

#if (defined PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
static uint32_t *PD_PsmGetSourcePDOs(pd_instance_t *pdInstance)
{
    uint8_t index;
    pd_source_pdo_t *pdo;
    pd_source_pdo_t *destPdo;
#if defined(PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT) && (PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT)
    uint8_t externalPowerState = 0;
#endif

    for (index = 0; index < pdInstance->pdPowerPortConfig->sourceCapCount; ++index)
    {
        pdo               = (pd_source_pdo_t *)((void *)&pdInstance->pdPowerPortConfig->sourceCaps[index]);
        destPdo           = (pd_source_pdo_t *)((void *)&pdInstance->sendingData[index + 1U]);
        destPdo->PDOValue = pdo->PDOValue;

        switch ((pd_pdo_type_t)pdo->commonPDO.pdoType)
        {
            case kPDO_Fixed:
            {
                if (pdo->fixedPDO.maxCurrent > pdInstance->dpmCableMaxCurrent)
                {
                    destPdo->fixedPDO.maxCurrent = pdInstance->dpmCableMaxCurrent;
                }
                break;
            }

            case kPDO_Variable:
            {
                if (pdo->variablePDO.maxCurrent > pdInstance->dpmCableMaxCurrent)
                {
                    destPdo->variablePDO.maxCurrent = pdInstance->dpmCableMaxCurrent;
                }
                break;
            }

            case kPDO_Battery:
            {
                if ((pdo->batteryPDO.maxAllowPower * 500U / pdo->batteryPDO.minVoltage) >
                    pdInstance->dpmCableMaxCurrent)
                {
                    destPdo->batteryPDO.maxAllowPower = (uint32_t)pdInstance->dpmCableMaxCurrent * 10U *
                                                        (pdo->batteryPDO.minVoltage * 50U / 1000U) / 250U;
                }
                break;
            }

            default:
                /* No action required. */
                break;
        }
    }

    destPdo = (pd_source_pdo_t *)((void *)&pdInstance->sendingData[1]);
#if defined(PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT) && (PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT)
    (void)pdInstance->pdCallback(pdInstance->callbackParam, PD_DPM_GET_EXTERNAL_POWER_STATE, &externalPowerState);
    destPdo->fixedPDO.externalPowered = externalPowerState;
#endif
    if (pdInstance->revision < PD_SPEC_REVISION_30)
    {
        destPdo->fixedPDO.unchunkedSupported = 0;
    }

    return (uint32_t *)&(pdInstance->sendingData[1]);
}
#endif

#if (defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
static uint32_t *PD_PsmGetSinkPDOs(pd_instance_t *pdInstance)
{
    uint8_t index;
    pd_sink_fixed_pdo_t *pdo = (pd_sink_fixed_pdo_t *)((void *)&pdInstance->sendingData[1]);
#if defined(PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT) && (PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT)
    uint8_t externalPowerState = 0;
#endif

    for (index = 0; index < pdInstance->pdPowerPortConfig->sinkCapCount; ++index)
    {
        pdInstance->sendingData[index + 1U] = pdInstance->pdPowerPortConfig->sinkCaps[index];
    }
#if defined(PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT) && (PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT)
    (void)pdInstance->pdCallback(pdInstance->callbackParam, PD_DPM_GET_EXTERNAL_POWER_STATE, &externalPowerState);
    pdo->externalPowered = externalPowerState;
#endif
    if (pdInstance->revision < PD_SPEC_REVISION_30)
    {
        pdo->frSwapRequiredCurrent = 0;
    }
    return (uint32_t *)&(pdInstance->sendingData[1]);
}
#endif

static void PD_PsmClearStateFlags(pd_instance_t *pdInstance)
{
    /* Initialise static flags */
    PD_PsmReset(pdInstance);
    pdInstance->psmCurState = PSM_UNKNOWN;
    pdInstance->psmNewState = PSM_UNKNOWN;

    pdInstance->dpmStateMachine             = 0;
    pdInstance->psmPresentlyPdConnected     = 0;
    pdInstance->psmPreviouslyPdConnected    = 0;
    pdInstance->psmExplicitContractExisted  = 0;
    pdInstance->unchunkedFeature            = 0;
    pdInstance->commandProcessing           = PD_DPM_INVALID;
    pdInstance->dpmMsgBits                  = 0;
    pdInstance->taskWaitTime                = PD_WAIT_EVENT_TIME;
    pdInstance->asmHardResetSnkProcessing   = 0;
    pdInstance->hardResetReceived           = 0;
    pdInstance->psmCableIdentitiesDataCount = 0;
#if (defined(PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)) && \
    (defined(PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE) && (PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE))
    pdInstance->commandRetryCount = 0;
#endif
#if ((defined PD_CONFIG_EXTENDED_CHUNK_LAYER_SUPPORT) && (PD_CONFIG_EXTENDED_CHUNK_LAYER_SUPPORT))
    PD_MsgChunkingLayerResetFlags(pdInstance);
#endif
}

static void PD_PsmDisconnect(pd_instance_t *pdInstance)
{
#if defined(PD_CONFIG_VBUS_ALARM_SUPPORT) && (PD_CONFIG_VBUS_ALARM_SUPPORT)
    uint8_t enable = 0;
#endif

#if defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
    pdInstance->psmPresentlyVconnSource = kPD_VconnNone;
#endif
    pdInstance->connectState = kTYPEC_ConnectNone;
    PD_PsmClearStateFlags(pdInstance);
    /* disconnect message layer */
    (void)PD_PhyControl(pdInstance, PD_PHY_RESET_MSG_FUNCTION, NULL);

#if defined(PD_CONFIG_VBUS_ALARM_SUPPORT) && (PD_CONFIG_VBUS_ALARM_SUPPORT)
    (void)PD_PhyControl(pdInstance, PD_PHY_CONTROL_VBUS_ALARM, &enable);
#endif
}

static void PD_PsmConnect(pd_instance_t *pdInstance)
{
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
    pdInstance->portPartnerSinkPDO1.PDOValue = 0;
#endif
    (void)PD_PhyControl(pdInstance, PD_PHY_SET_MSG_HEADER_INFO, NULL);
    pdInstance->dpmCableMaxCurrent = 3000 / 10; /* 3A */
    PD_PsmClearStateFlags(pdInstance);
}

static pd_state_machine_state_t PD_PsmDisconnectCheck(pd_instance_t *pdInstance, pd_state_machine_state_t state)
{
    if ((state != kSM_ErrorRecovery) && (state != kSM_Detach))
    {
        if (PD_ConnectCheck(pdInstance) == kConnectState_Disconnected)
        {
            state = kSM_Detach;
        }
    }
    return state;
}

/* internal function */
#if (defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
static uint8_t PD_PsmCheckLessOrEqualVsafe5v(pd_instance_t *pdInstance)
{
    uint8_t powerState =
        (PD_VBUS_POWER_STATE_VSAFE5V_MASK | PD_VBUS_POWER_STATE_VBUS_MASK | PD_VBUS_POWER_STATE_VSAFE0V_MASK);
    (void)PD_PhyControl(pdInstance, PD_PHY_GET_VBUS_POWER_STATE, &powerState);
    if ((powerState & PD_VBUS_POWER_STATE_VSAFE5V_MASK) != 0U)
    {
        return 1;
    }
    if ((powerState & PD_VBUS_POWER_STATE_VSAFE0V_MASK) != 0U)
    {
        return 1;
    }
    if (((powerState & PD_VBUS_POWER_STATE_VSAFE5V_MASK) == 0U) && ((powerState & PD_VBUS_POWER_STATE_VBUS_MASK) == 0U))
    {
        return 1;
    }

    return 0;
}

static void PD_PsmCheckFRS5V(pd_instance_t *pdInstance)
{
    if (pdInstance->fr5VOpened == 0U)
    {
        if (PD_PsmCheckLessOrEqualVsafe5v(pdInstance) != 0U)
        {
            (void)pdInstance->callbackFns->PD_SrcTurnOnTypeCVbus(pdInstance->callbackParam, kVbusPower_InFRSwap);
            pdInstance->fr5VOpened = 1;
        }
    }
}
#endif /* PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE */

static uint8_t PD_DpmGetMsg(pd_instance_t *pdInstance)
{
    uint8_t reVal = 0;
    uint8_t commandIndex;

    if (pdInstance->dpmMsgBits != 0U)
    {
        for (commandIndex = (uint8_t)PD_DPM_CONTROL_POWER_NEGOTIATION; commandIndex < (uint8_t)PD_DPM_CONTROL_COUNT;
             ++commandIndex)
        {
            if ((pdInstance->dpmMsgBits & ((uint32_t)0x01U << commandIndex)) != 0U)
            {
                reVal = commandIndex;
                break;
            }
        }
    }

    return reVal;
}

static void PD_DpmClearMsg(pd_instance_t *pdInstance, pd_command_t id)
{
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    if ((pdInstance->dpmMsgBits & ((uint32_t)0x01U << (uint8_t)id)) != 0U)
    {
        pdInstance->dpmMsgBits &= (~((uint32_t)0x01U << (uint8_t)id));
    }
    OSA_EXIT_CRITICAL();
}

static void PD_DpmSendMsg(pd_handle pdHandle, uint8_t id)
{
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    pdInstance->dpmMsgBits |= ((uint32_t)0x01U << id);
#if (defined(PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)) && \
    (defined(PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE) && (PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE))
    pdInstance->commandRetryCount = N_DPM_COMMAND_RETRY_COUNTER;
#endif
    OSA_EXIT_CRITICAL();
    PD_StackSetEvent(pdInstance, PD_TASK_EVENT_DPM_MSG);
}

/*! ***************************************************************************
   command flow and message flow and state machine related functions
******************************************************************************/

#if ((defined PD_CONFIG_EXTENDED_CHUNK_LAYER_SUPPORT) && (PD_CONFIG_EXTENDED_CHUNK_LAYER_SUPPORT))
static void PD_MsgChunkingLayerResetFlags(pd_instance_t *pdInstance)
{
    pdInstance->chunkingRXState = RCH_Wait_For_Message_From_Protocol_Layer;
    pdInstance->chunkingTXState = TCH_Wait_For_Message_Request_From_Policy_Engine;
}

static uint8_t PD_PsmChunkingLayerRXTimerWaiting(pd_instance_t *pdInstance)
{
    return (RCH_Waiting_Chunk == pdInstance->chunkingRXState) ? 1U : 0U;
}

static uint8_t PD_PsmChunkingLayerRXStateMachine(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo)
{
    pd_chunking_layer_state_t prevState = pdInstance->chunkingRXState;
    pd_extended_msg_header_t extHeader;

    if (triggerInfo->pdMsgDataBuffer != NULL)
    {
        extHeader.extendedMsgHeaderVal =
            USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)triggerInfo->pdMsgDataBuffer));
    }
    else
    {
        extHeader.extendedMsgHeaderVal = 0;
    }

    /* state not change */
    switch (pdInstance->chunkingRXState)
    {
        case RCH_Wait_For_Message_From_Protocol_Layer: /* B */
        {
            if ((triggerInfo->triggerEvent == PSM_TRIGGER_PD_MSG) &&
                ((triggerInfo->msgHeader.bitFields.extended == 0U) ||
                 ((extHeader.bitFields.chunked == 0U) && (pdInstance->unchunkedFeature != 0U))))
            {
                pdInstance->chunkingRXState = RCH_Pass_Up_Message;
            }
            else if ((triggerInfo->triggerEvent == PSM_TRIGGER_PD_MSG) &&
                     (triggerInfo->msgHeader.bitFields.extended != 0U) && (extHeader.bitFields.chunked != 0U))
            {
                pdInstance->chunkingRXState = RCH_Processing_Extended_Message;
            }
            else if (pdInstance->unchunkedFeature == extHeader.bitFields.chunked)
            {
                pdInstance->chunkingRXState = RCH_Report_Error;
            }
            else
            {
                /* No action required. */
            }
            break;
        }

        case RCH_Waiting_Chunk: /* B */
            if ((triggerInfo->triggerEvent == PSM_TRIGGER_PD_MSG) &&
                (triggerInfo->msgHeader.bitFields.extended != 0U) && (extHeader.bitFields.chunked != 0U) &&
                (pdInstance->chunkNumberExpected == extHeader.bitFields.chunkNumber))
            {
                pdInstance->chunkingRXState = RCH_Processing_Extended_Message;
            }
            else if ((PD_TimerCheckInvalidOrTimeOut(pdInstance, timrChunkSenderResponseTimer) != 0U) ||
                     (triggerInfo->triggerEvent == PSM_TRIGGER_PD_MSG))
            {
                pdInstance->chunkingRXState = RCH_Report_Error;
            }
            else
            {
                /* No action required. */
            }
            break;

        default:
            /* Any Message Received and not in state RCH_Waiting_Chunk or RCH_Wait_For_Message_From_Protocol_Layer */
            pdInstance->chunkingRXState = RCH_Report_Error;
            break;
    }

    while (prevState != pdInstance->chunkingRXState)
    {
        prevState = pdInstance->chunkingRXState;
        switch (pdInstance->chunkingRXState)
        {
            case RCH_Wait_For_Message_From_Protocol_Layer: /* C */
                /* pdInstance->abortFlag = 0; */
                /* TODO: clear extended Rx Buffer */
                break;

            case RCH_Pass_Up_Message: /* C */
                pdInstance->chunkingRXState = RCH_Wait_For_Message_From_Protocol_Layer;
                break;

            case RCH_Processing_Extended_Message: /* C */
                if (extHeader.bitFields.chunkNumber == 0U)
                {
                    pdInstance->chunkNumberExpected   = 0;
                    pdInstance->chunkNumBytesReceived = 0;
                }

                if (extHeader.bitFields.chunkNumber == pdInstance->chunkNumberExpected)
                {
                    pdInstance->chunkNumBytesReceived += (triggerInfo->msgHeader.bitFields.NumOfDataObjs * 4U - 2U);
                    pdInstance->chunkNumberExpected++;
                    /* TODO: copy data to buff */
                }
                else if (extHeader.bitFields.chunkNumber != pdInstance->chunkNumberExpected)
                {
                    pdInstance->chunkingRXState = RCH_Report_Error;
                }
                else
                {
                    /* No action required. */
                }

#if 0
                if (pdInstance->abortFlag)
                {
                    pdInstance->chunkingRXState = RCH_Wait_For_Message_From_Protocol_Layer;
                }
                else
#endif

                if ((extHeader.bitFields.chunkNumber * 26U +
                     (triggerInfo->msgHeader.bitFields.NumOfDataObjs * 4U - 2U)) >= extHeader.bitFields.dataSize)
                {
                    pdInstance->chunkingRXState = RCH_Pass_Up_Message;
                }
                else
                {
                    /* message is not completeed */
                    pdInstance->chunkingRXState = RCH_Requesting_Chunk;
                }
                break;

            case RCH_Requesting_Chunk: /* C */
                /* dataSize = 0, requestChunk = 1 */
                if (PD_MsgSendExtendedMsg(pdInstance, triggerInfo->pdMsgSop, triggerInfo->pdMsgType, 0, NULL, 0, 1,
                                          pdInstance->chunkNumberExpected) == kStatus_PD_Success)
                {
                    pdInstance->chunkingRXState = RCH_Waiting_Chunk;
                }
                else
                {
                    pdInstance->chunkingRXState = RCH_Report_Error;
                }
                break;

            case RCH_Waiting_Chunk: /* C */
                (void)PD_TimerStart(pdInstance, timrChunkSenderResponseTimer, T_CHUNK_SENDER_RESPONSE);
                break;

            case RCH_Report_Error: /* C */
                /* TODO: report error to policy engine */
                pdInstance->chunkingRXState = RCH_Wait_For_Message_From_Protocol_Layer;
                break;

            default:
                /* No action required. */
                break;
        }
    }

    return (pdInstance->chunkingRXState == RCH_Wait_For_Message_From_Protocol_Layer) ? 1U : 0U;
}

#if 0 /* comments it temparily for the warning (declared but never referenced) */
static uint8_t PD_PsmChunkingLayerCheckSentDone(pd_instance_t *pdInstance)
{
    return pdInstance->chunkSentDone;
}

static pd_status_t PD_PsmChunkingLayerTXStateMachine(pd_instance_t *pdInstance, start_of_packet_t sop, message_type_t msgType, uint8_t *dataBuffer, uint32_t dataLength, psm_trigger_info_t *triggerInfo)
{
    pd_chunking_layer_state_t prevState = pdInstance->chunkingTXState;
    pd_status_t status = kStatus_PD_Success;

    /* state not change */
    switch (pdInstance->chunkingTXState)
    {
        case TCH_Wait_For_Message_Request_From_Policy_Engine: /* B */
            if (pdInstance->chunkingRXState != RCH_Wait_For_Message_From_Protocol_Layer)
            {
                pdInstance->chunkingTXState = TCH_Report_Error;
            }
            else
            {
                /* only chunking msg is passed to this function */
                pdInstance->chunkingTXState = TCH_Prepare_To_Send_Chunked_Message;
            }
            break;

        case TCH_Wait_Chunk_Request: /* B */
        {
            pd_extended_msg_header_t extHeader;
            extHeader.extendedMsgHeaderVal = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)triggerInfo->pdMsgDataBuffer));
            if ((triggerInfo != NULL) && (triggerInfo->triggerEvent == PSM_TRIGGER_PD_MSG) && (triggerInfo->msgHeader.bitFields.extended != 0U) &&
                (extHeader.bitFields.chunked != 0U) && (pdInstance->chunkNumberToSend == extHeader.bitFields.chunkNumber))
            {
                pdInstance->chunkingTXState = TCH_Sending_Chunked_Message;
            }
            else if ((triggerInfo != NULL) && (triggerInfo->triggerEvent == PSM_TRIGGER_PD_MSG))
            {
                status = kStatus_PD_Error;
                pdInstance->chunkingTXState = TCH_Wait_For_Message_Request_From_Policy_Engine;
            }
            else if (PD_TimerCheckInvalidOrTimeOut(pdInstance, timrChunkSenderResponseTimer) != 0U)
            {
                /* still think success */
                pdInstance->chunkingTXState = TCH_Message_Sent;
            }
            else
            {
                /* No action required. */
            }
            break;
        }
    }

    while (prevState != pdInstance->chunkingTXState)
    {
        prevState = pdInstance->chunkingTXState;
        switch (pdInstance->chunkingTXState)
        {
            case TCH_Report_Error: /* C */
                status = kStatus_PD_Error;
                pdInstance->chunkingTXState = TCH_Wait_For_Message_Request_From_Policy_Engine;
                break;

            case TCH_Prepare_To_Send_Chunked_Message: /* C */
                pdInstance->chunkNumberToSend = 0;
                pdInstance->chunkSentDone = 0;
                pdInstance->chunkingTXState = TCH_Sending_Chunked_Message;
                break;

            case TCH_Sending_Chunked_Message: /* C */
            {
                uint32_t sendLength = (dataLength - pdInstance->chunkNumberToSend * 26);
                if (sendLength > 26)
                {
                    sendLength = 26;
                }
                /* dataSize = dataLength, requestChunk = 0 */
                if (PD_MsgSendExtendedMsg(pdInstance, sop, msgType, sendLength, dataBuffer,
                                          (uint16_t)dataLength, 0, pdInstance->chunkNumberToSend) == kStatus_PD_Success)
                {
                    if (pdInstance->chunkNumberToSend == ((dataLength + 25) / 26))
                    {
                        pdInstance->chunkingTXState = TCH_Message_Sent;
                    }
                    else
                    {
                        pdInstance->chunkingTXState = TCH_Wait_Chunk_Request;
                    }
                }
                else
                {
                    pdInstance->chunkingTXState = TCH_Report_Error;
                }
                break;
            }

            case TCH_Wait_Chunk_Request: /* C */
                pdInstance->chunkNumberToSend++;
                (void)PD_TimerStart(pdInstance, timrChunkSenderRequestTimer, T_CHUNK_SENDER_REQUEST);
                break;

            case TCH_Message_Sent: /* C */
                pdInstance->chunkSentDone = 1;
                pdInstance->chunkingTXState = TCH_Wait_For_Message_Request_From_Policy_Engine;
                break;

            case TCH_Wait_For_Message_Request_From_Policy_Engine: /* C */
                pdInstance->chunkSentDone = 1;
                break;

            default:
                /* No action required. */
                break;
        }
    }
    return status;
}
#endif
#endif

#if (defined PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
static uint8_t PD_PsmCheckInAltMode(pd_instance_t *pdInstance, start_of_packet_t sop)
{
#if (defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT)
    uint8_t altModeState = 0;
    if ((PD_AltModeState(pdInstance, &altModeState) == kStatus_PD_Success) && (altModeState != 0U))
    {
        return 1;
    }
#endif
#if (defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)
    if (pdInstance->psmVdmActiveModeValidMask != 0U)
    {
        return 1;
    }
#endif
    return 0;
}
#endif

#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY_LOG) && (PD_CONFIG_ENABLE_AUTO_POLICY_LOG)
static void PD_PsmPrintAutoPolicyReplySwapRequestLog(pd_instance_t *pdInstance, uint8_t message)
{
    (void)PRINTF("auto reply ");
    if (pdInstance->commandEvaluateResult == kCommandResult_Accept)
    {
        (void)PRINTF("accept");
    }
    else if (pdInstance->commandEvaluateResult == kCommandResult_Reject)
    {
        (void)PRINTF("reject");
    }
    else if (pdInstance->commandEvaluateResult == kCommandResult_Wait)
    {
        (void)PRINTF("wait");
    }
    else
    {
        /* No action required. */
    }
    (void)PRINTF(" for ");

    switch ((message_type_t)message)
    {
        case kPD_MsgPrSwap:
            (void)PRINTF("pr");
            break;
        case kPD_MsgDrSwap:
            (void)PRINTF("dr");
            break;
        case kPD_MsgVconnSwap:
        default:
            (void)PRINTF("vconn");
            break;
    }
    (void)PRINTF(" swap\r\n");
}

static void PD_PsmPrintAutoPolicyStartSwapLog(pd_instance_t *pdInstance, uint8_t message)
{
    (void)PRINTF("start auto request ");
    switch ((message_type_t)message)
    {
        case kPD_MsgPrSwap:
            (void)PRINTF("pr");
            break;
        case kPD_MsgDrSwap:
            (void)PRINTF("dr");
            break;
        case kPD_MsgVconnSwap:
        default:
            (void)PRINTF("vconn");
            break;
    }

    (void)PRINTF(" swap\r\n");
}
#endif /* PD_CONFIG_ENABLE_AUTO_POLICY_LOG */

#if (defined PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
static uint8_t PD_PsmAutoRequestVconnSwap(pd_instance_t *pdInstance)
{
    pd_vconn_role_t autoSwap = (pd_vconn_role_t)PD_POLICY_GET_AUTO_REQUEST_VCONNSWAP(pdInstance);
    if ((!PD_POLICY_SUPPORT(pdInstance)) || (autoSwap == kPD_VconnNone))
    {
        return 0;
    }

    if ((pdInstance->pdPowerPortConfig->vconnSupported != 0U) && (pdInstance->vconnSwapResult == kCommandResult_None) &&
        (autoSwap != pdInstance->psmPresentlyVconnSource))
    {
        if ((pdInstance->portPartnerVconnSwapToOffRejected == 0U) && (autoSwap == kPD_NotVconnSource))
        {
            return 1;
        }
        if ((pdInstance->portPartnerVconnSwapToOnRejected == 0U) && (autoSwap == kPD_IsVconnSource))
        {
            return 1;
        }
    }
    return 0;
}
#endif

#if (defined PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
static uint8_t PD_PsmAutoRequestDataSwap(pd_instance_t *pdInstance)
{
    pd_data_role_t autoSwap = (pd_data_role_t)PD_POLICY_GET_AUTO_REQUEST_DRSWAP(pdInstance);
    if ((!PD_POLICY_SUPPORT(pdInstance)) || (autoSwap == kPD_DataRoleNone))
    {
        return 0;
    }

    /* Only auto-swap if there has been no previous activity
     * Do not auto-swap if we are already in an alternate mode
     */
    if ((PD_PsmCheckInAltMode(pdInstance, kPD_MsgSOP) == 0U) &&
        (pdInstance->pdPowerPortConfig->dataFunction == (uint8_t)kDataConfig_DRD) &&
        (pdInstance->drSwapResult == kCommandResult_None) && (autoSwap != pdInstance->curDataRole))
    {
        if ((pdInstance->portPartnerDrSwapToDFPRejected == 0U) && (autoSwap == kPD_DataRoleDFP))
        {
            return 1;
        }
        if ((pdInstance->portPartnerDrSwapToUFPRejected == 0U) && (autoSwap == kPD_DataRoleUFP))
        {
            return 1;
        }
    }
    return 0;
}
#endif

#if (defined PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
static uint8_t PD_PsmAutoRequestPowerSwap(pd_instance_t *pdInstance)
{
    if ((!PD_POLICY_SUPPORT(pdInstance)) || ((PD_POLICY_GET_AUTO_REQUEST_PRSWAP_AS_SOURCE(pdInstance) == 0U) &&
                                             (PD_POLICY_GET_AUTO_REQUEST_PRSWAP_AS_SINK(pdInstance) == 0U)))
    {
        return 0;
    }

    if (PD_PsmIsDualRole(pdInstance) != 0U)
    {
        if ((pdInstance->curPowerRole == kPD_PowerRoleSource) && (pdInstance->portPartnerPrSwapToSinkRejected == 0U) &&
            (PD_POLICY_GET_AUTO_REQUEST_PRSWAP_AS_SOURCE(pdInstance) != 0U))
        {
            return 1;
        }

        if ((pdInstance->curPowerRole == kPD_PowerRoleSink) && (pdInstance->portPartnerPrSwapToSourceRejected == 0U) &&
            (PD_POLICY_GET_AUTO_REQUEST_PRSWAP_AS_SINK(pdInstance) != 0U))
        {
            return 1;
        }
    }
    return 0;
}
#endif

static void PD_PsmSetAutoPolicyState(pd_instance_t *pdInstance, pd_auto_policy_state_t newState)
{
    if (pdInstance->autoPolicyState == newState)
    {
        return;
    }

    pdInstance->autoPolicyState = newState;
    if (((uint8_t)newState & (uint8_t)PSM_RDY_DELAY_FLAG) != 0U)
    {
        if (pdInstance->curPowerRole == kPD_PowerRoleSource)
        {
            (void)PD_TimerStart(pdInstance, timrPsmRdyEvalDelayTimer, T_PSM_SRC_RDY_EVAL_DELAY);
        }
        else
        {
            (void)PD_TimerStart(pdInstance, timrPsmRdyEvalDelayTimer, T_PSM_SNK_RDY_EVAL_DELAY);
        }
    }
}

static uint8_t PD_PsmGetExternalPowerState(pd_instance_t *pdInstance)
{
    uint8_t externalPowerState;
#if defined(PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT) && (PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT)
    if (pdInstance->pdCallback(pdInstance->callbackParam, PD_DPM_GET_EXTERNAL_POWER_STATE, &externalPowerState) !=
        kStatus_PD_Success)
#endif
    {
        if ((pdInstance->curPowerRole == kPD_PowerRoleSource) &&
            (pdInstance->pdPowerPortConfig->sourceCapCount != 0U) &&
            (pdInstance->pdPowerPortConfig->sourceCaps != NULL))
        {
            pd_source_pdo_t sourcePDO;
            sourcePDO.PDOValue = pdInstance->pdPowerPortConfig->sourceCaps[0];
            externalPowerState = (uint8_t)sourcePDO.fixedPDO.externalPowered;
        }
        else if ((pdInstance->curPowerRole == kPD_PowerRoleSink) &&
                 (pdInstance->pdPowerPortConfig->sinkCapCount != 0U) &&
                 (pdInstance->pdPowerPortConfig->sinkCaps != NULL))
        {
            pd_sink_pdo_t sinkPDO;
            sinkPDO.PDOValue   = pdInstance->pdPowerPortConfig->sinkCaps[0];
            externalPowerState = (uint8_t)sinkPDO.fixedPDO.externalPowered;
        }
        else
        {
            externalPowerState = 0;
        }
    }
    return externalPowerState;
}

/* this function is called in rdy state */
static uint8_t PD_PsmReadyAutoPolicyProcess(pd_instance_t *pdInstance)
{
    pd_auto_policy_state_t newAutoPolicySate;
    uint8_t didNothing = 0;

    if (((uint8_t)pdInstance->autoPolicyState & (uint8_t)PSM_RDY_DELAY_FLAG) != 0U)
    {
        if (PD_TimerCheckInvalidOrTimeOut(pdInstance, timrPsmRdyEvalDelayTimer) != 0U)
        {
            uint8_t tmpAutoPolicyState = ((uint8_t)pdInstance->autoPolicyState) & (~((uint8_t)PSM_RDY_DELAY_FLAG));

            pdInstance->autoPolicyState = (pd_auto_policy_state_t)tmpAutoPolicyState;
        }
        else
        {
            return 1;
        }
    }

    (void)OSA_EventClear(pdInstance->taskEventHandle, PD_TASK_EVENT_EXTERNAL_POWER_CHANGE);
    newAutoPolicySate = pdInstance->autoPolicyState;
    switch (pdInstance->autoPolicyState)
    {
        case PSM_RDY_EVAL_INIT:
        {
            /* Update whether we have seen external power, so we don't get into an idle loop */
            pdInstance->rdySeenExtPower = PD_PsmGetExternalPowerState(pdInstance);
            /* default enter into idle state */
            newAutoPolicySate = PSM_RDY_EVAL_IDLE;

#if (defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)) ||                 \
    (defined(PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)) || \
    (defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE))
            /* MISRA C:2012 Rule 14.3 */
            if (
#if (defined PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
                (PD_PsmAutoRequestDataSwap(pdInstance) != 0U)
#if (defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)) || \
    (defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE))
                ||
#endif
#endif
#if defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
                (PD_PsmAutoRequestVconnSwap(pdInstance) != 0U)
#if defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
                ||
#endif
#endif
#if defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
                (PD_PsmAutoRequestPowerSwap(pdInstance) != 0U)
/* Need to also check get partner capabilities to evaluate swap requests
   (ConfigGetDualRolePower(CLPCfgTblPtr) && ConfigGetAutoAcceptPrSwap(CLPCfgTblPtr)) || */
#endif
            )
            {
#if defined(PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
                if (pdInstance->curPowerRole == kPD_PowerRoleSource)
                {
                    newAutoPolicySate =
                        (pd_auto_policy_state_t)((uint8_t)PSM_RDY_DELAY_FLAG | (uint8_t)PSM_RDY_EVAL_GET_SNK_CAP);
                }
                else
#endif
#if defined(PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
                    if (pdInstance->curPowerRole == kPD_PowerRoleSink)
                {
                    newAutoPolicySate =
                        (pd_auto_policy_state_t)((uint8_t)PSM_RDY_DELAY_FLAG | (uint8_t)PSM_RDY_EVAL_CHECK_PARTNER_CAP);
                }
                else
#endif
                {
                    /* No action required. */
                }
            }
#endif
            break;
        }

#if defined(PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
        case PSM_RDY_EVAL_GET_SNK_CAP:
            if (PD_PsmStartCommand(pdInstance, PD_DPM_CONTROL_GET_PARTNER_SINK_CAPABILITIES, 1) != 0U)
            {
                /* On return to PSM_SRC_READY check the result */
                newAutoPolicySate = PSM_RDY_EVAL_CHECK_PARTNER_CAP;
                /* Get the sink capabilities, on return to READY state */
                pdInstance->psmNewState  = PSM_PE_SRC_GET_SINK_CAP;
                pdInstance->dumpResponse = 1;
            }
            else
            {
                /* Fail, retry */
                newAutoPolicySate =
                    (pd_auto_policy_state_t)((uint8_t)PSM_RDY_DELAY_FLAG | (uint8_t)PSM_RDY_EVAL_GET_SNK_CAP);
            }
            break;
#endif

#if (defined(PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)) || \
    (defined(PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE))
        case PSM_RDY_EVAL_CHECK_PARTNER_CAP:
            pdInstance->rdySeenExtPower = PD_PsmGetExternalPowerState(pdInstance);
#if (defined PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
            if (PD_PsmAutoRequestVconnSwap(pdInstance) != 0U)
            {
                /* Attempt to vconn swap */
                newAutoPolicySate =
                    (pd_auto_policy_state_t)((uint8_t)PSM_RDY_DELAY_FLAG | (uint8_t)PSM_RDY_EVAL_VCONN_SWAP);
            }
            else
#endif
#if (defined PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
                if ((PD_PsmAutoRequestDataSwap(pdInstance) != 0U) &&
                    (((pdInstance->curPowerRole == kPD_PowerRoleSource) &&
                      (pdInstance->portPartnerSinkPDO1.fixedPDO.dualRoleData != 0U)) ||
                     ((pdInstance->curPowerRole == kPD_PowerRoleSink) &&
                      (pdInstance->selfOrPartnerFirstSourcePDO.fixedPDO.dualRoleData != 0U))))
            {
                /* Attempt to data role swap */
                newAutoPolicySate =
                    (pd_auto_policy_state_t)((uint8_t)PSM_RDY_DELAY_FLAG | (uint8_t)PSM_RDY_EVAL_DR_SWAP);
            }
            else
#endif
#if (defined PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
                /* auto request swap as sink && self is not external powered && partner is external powered */
                if ((PD_PsmAutoRequestPowerSwap(pdInstance) != 0U) && (pdInstance->rdySeenExtPower == 0U) &&
                    (pdInstance->curPowerRole == kPD_PowerRoleSource) &&
                    /* #if !defined(USBPD_ENABLE_IMMEDIATE_SWAP_TO_SINK_ON_POWER_LOSS) ||
                       (!USBPD_ENABLE_IMMEDIATE_SWAP_TO_SINK_ON_POWER_LOSS) */
                    (pdInstance->portPartnerSinkPDO1.fixedPDO.externalPowered != 0U) &&
                    /* #endif */
                    (pdInstance->portPartnerSinkPDO1.fixedPDO.dualRolePower != 0U))
            {
                /* Attempt to swap to sink */
                newAutoPolicySate =
                    (pd_auto_policy_state_t)((uint8_t)PSM_RDY_EVAL_SWAP_TO_SNK | (uint8_t)PSM_RDY_DELAY_FLAG);
#if 0
#if !defined(USBPD_ENABLE_IMMEDIATE_SWAP_TO_SINK_ON_POWER_LOSS) || (!USBPD_ENABLE_IMMEDIATE_SWAP_TO_SINK_ON_POWER_LOSS)
                                    PSM_RDY_DELAY_FLAG;
#else
                                    0;
#endif
#endif
            }
            /* auto request swap as source && self is external powered && partner is not external powered. */
            else if ((PD_PsmAutoRequestPowerSwap(pdInstance) != 0U) && (pdInstance->rdySeenExtPower != 0U) &&
                     (pdInstance->curPowerRole == kPD_PowerRoleSink) &&
                     (pdInstance->selfOrPartnerFirstSourcePDO.fixedPDO.externalPowered == 0U) &&
                     (pdInstance->selfOrPartnerFirstSourcePDO.fixedPDO.dualRolePower != 0U))
            {
                /* Attempt to swap to source */
                newAutoPolicySate =
                    (pd_auto_policy_state_t)((uint8_t)PSM_RDY_DELAY_FLAG | (uint8_t)PSM_RDY_EVAL_SWAP_TO_SRC);
            }
            else
#endif
            {
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY_LOG) && (PD_CONFIG_ENABLE_AUTO_POLICY_LOG)
                if ((PD_PsmAutoRequestPowerSwap(pdInstance) != 0U) &&
                    (pdInstance->curPowerRole == kPD_PowerRoleSource) &&
                    (pdInstance->portPartnerSinkPDO1.fixedPDO.externalPowered != 0U) &&
                    (pdInstance->portPartnerSinkPDO1.fixedPDO.dualRolePower != 0U))
                {
                    (void)PRINTF("don't do auto request pr swap to sink when self is external powered\r\n");
                }
                else if ((PD_PsmAutoRequestPowerSwap(pdInstance) != 0U) &&
                         (pdInstance->curPowerRole == kPD_PowerRoleSink) &&
                         (pdInstance->selfOrPartnerFirstSourcePDO.fixedPDO.externalPowered == 0U) &&
                         (pdInstance->selfOrPartnerFirstSourcePDO.fixedPDO.dualRolePower != 0U))
                {
                    (void)PRINTF("don't do auto request pr swap to source when self is not external powered\r\n");
                }
                else
                {
                    /* No action required. */
                }
#endif
                newAutoPolicySate = (pd_auto_policy_state_t)((uint8_t)PSM_RDY_DELAY_FLAG | (uint8_t)PSM_RDY_EVAL_IDLE);
            }
            break;
#endif

#if (defined PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
        case PSM_RDY_EVAL_SWAP_TO_SNK:
            if ((PD_PsmAutoRequestPowerSwap(pdInstance) != 0U) &&
                /* Check for a change in the externally powered status since last we saw it */
                (pdInstance->rdySeenExtPower == PD_PsmGetExternalPowerState(pdInstance)))
            {
                if (PD_PsmStartCommand(pdInstance, PD_DPM_CONTROL_PR_SWAP, 1) != 0U)
                {
                    newAutoPolicySate       = PSM_RDY_EVAL_CHECK_SWAP_TO_SNK;
                    pdInstance->psmNewState = PSM_PE_PRS_SRC_SNK_SEND_PR_SWAP;
                }
                else
                {
                    /* Fail, retry */
                    newAutoPolicySate =
                        (pd_auto_policy_state_t)((uint8_t)PSM_RDY_DELAY_FLAG | (uint8_t)PSM_RDY_EVAL_SWAP_TO_SNK);
                }
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY_LOG) && (PD_CONFIG_ENABLE_AUTO_POLICY_LOG)
                PD_PsmPrintAutoPolicyStartSwapLog(pdInstance, (uint8_t)kPD_MsgPrSwap);
#endif
            }
            else
            {
                newAutoPolicySate = PSM_RDY_EVAL_INIT;
            }
            break;
        case PSM_RDY_EVAL_SWAP_TO_SRC:
            if ((PD_PsmAutoRequestPowerSwap(pdInstance) != 0U) &&
                /* Check for a change in the externally powered status since last we saw it */
                (pdInstance->rdySeenExtPower == PD_PsmGetExternalPowerState(pdInstance)))
            {
                if (PD_PsmStartCommand(pdInstance, PD_DPM_CONTROL_PR_SWAP, 1) != 0U)
                {
                    newAutoPolicySate       = PSM_RDY_EVAL_CHECK_SWAP_TO_SRC;
                    pdInstance->psmNewState = PSM_PE_PRS_SNK_SRC_SEND_PR_SWAP;
                }
                else
                {
                    /* Fail, retry */
                    newAutoPolicySate =
                        (pd_auto_policy_state_t)((uint8_t)PSM_RDY_DELAY_FLAG | (uint8_t)PSM_RDY_EVAL_SWAP_TO_SRC);
                }
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY_LOG) && (PD_CONFIG_ENABLE_AUTO_POLICY_LOG)
                PD_PsmPrintAutoPolicyStartSwapLog(pdInstance, (uint8_t)kPD_MsgPrSwap);
#endif
            }
            else
            {
                newAutoPolicySate = PSM_RDY_EVAL_INIT;
            }
            break;
#endif

#if (defined PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
        case PSM_RDY_EVAL_DR_SWAP:
            if (PD_PsmAutoRequestDataSwap(pdInstance) != 0U)
            {
                if (PD_PsmStartCommand(pdInstance, PD_DPM_CONTROL_DR_SWAP, 1) != 0U)
                {
                    newAutoPolicySate       = PSM_RDY_EVAL_CHECK_DR_SWAP;
                    pdInstance->psmNewState = PSM_PE_DRS_SEND_DR_SWAP;
                }
                else
                {
                    /* Fail, retry */
                    newAutoPolicySate =
                        (pd_auto_policy_state_t)((uint8_t)PSM_RDY_DELAY_FLAG | (uint8_t)PSM_RDY_EVAL_DR_SWAP);
                }
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY_LOG) && (PD_CONFIG_ENABLE_AUTO_POLICY_LOG)
                PD_PsmPrintAutoPolicyStartSwapLog(pdInstance, (uint8_t)kPD_MsgDrSwap);
#endif
            }
            else
            {
                newAutoPolicySate = PSM_RDY_EVAL_INIT;
            }
            break;
#endif

#if defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
        case PSM_RDY_EVAL_VCONN_SWAP:
            if (PD_PsmAutoRequestVconnSwap(pdInstance) != 0U)
            {
                if (PD_PsmStartCommand(pdInstance, PD_DPM_CONTROL_VCONN_SWAP, 1) == 0U)
                {
                    /* Fail, retry */
                    newAutoPolicySate =
                        (pd_auto_policy_state_t)((uint8_t)PSM_RDY_DELAY_FLAG | (uint8_t)PSM_RDY_EVAL_VCONN_SWAP);
                }
                else
                {
                    newAutoPolicySate       = PSM_RDY_EVAL_CHECK_VCONN_SWAP;
                    pdInstance->psmNewState = PSM_PE_VCS_SEND_SWAP;
                }
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY_LOG) && (PD_CONFIG_ENABLE_AUTO_POLICY_LOG)
                PD_PsmPrintAutoPolicyStartSwapLog(pdInstance, (uint8_t)kPD_MsgVconnSwap);
#endif
            }
            else
            {
                newAutoPolicySate = PSM_RDY_EVAL_INIT;
            }
            break;
#endif

        case PSM_RDY_EVAL_IDLE:
            /* Check for a change in the externally powered status since last we saw it */
            if (pdInstance->rdySeenExtPower != PD_PsmGetExternalPowerState(pdInstance))
            {
                newAutoPolicySate = PSM_RDY_EVAL_INIT;
            }
            else
            {
                /* No further action required until something changes */
                didNothing = 1;
            }
            break;

        default:
            didNothing = 1;
            break;
    }

    PD_PsmSetAutoPolicyState(pdInstance, newAutoPolicySate);
    return didNothing;
}

static void PD_PsmReadyAutoPolicyResult(pd_instance_t *pdInstance, pd_command_result_t result)
{
    pd_auto_policy_state_t newAutoPolicySate = pdInstance->autoPolicyState;

    switch (result)
    {
        /* If far end accepts, then we will be reset to PSM_RDY_EVAL_INIT at swap
           For reject we move to check the conditions again after a short delay
           For wait we return to try the same thing again. */
        case kCommandResult_Reject:
            switch (newAutoPolicySate)
            {
#if (defined PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
                case PSM_RDY_EVAL_CHECK_SWAP_TO_SNK:
                    pdInstance->portPartnerPrSwapToSinkRejected = 1;
                    break;
                case PSM_RDY_EVAL_CHECK_SWAP_TO_SRC:
                    pdInstance->portPartnerPrSwapToSourceRejected = 1;
                    break;
#endif
#if (defined PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
                case PSM_RDY_EVAL_CHECK_DR_SWAP:
                    if (pdInstance->curDataRole == kPD_DataRoleDFP)
                    {
                        pdInstance->portPartnerDrSwapToUFPRejected = 1;
                    }
                    else
                    {
                        pdInstance->portPartnerDrSwapToDFPRejected = 1;
                    }
                    break;
#endif
#if defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
                case PSM_RDY_EVAL_CHECK_VCONN_SWAP:
                    if (pdInstance->psmPresentlyVconnSource == kPD_IsVconnSource)
                    {
                        pdInstance->portPartnerVconnSwapToOffRejected = 1;
                    }
                    else
                    {
                        pdInstance->portPartnerVconnSwapToOnRejected = 1;
                    }
                    break;
#endif
                default:
                    /* No action required. */
                    break;
            }
            newAutoPolicySate = (pd_auto_policy_state_t)((uint8_t)PSM_RDY_DELAY_FLAG | (uint8_t)PSM_RDY_EVAL_INIT);
            break;

        case kCommandResult_Error:
        case kCommandResult_Timeout:
            newAutoPolicySate = (pd_auto_policy_state_t)((uint8_t)PSM_RDY_DELAY_FLAG | (uint8_t)PSM_RDY_EVAL_INIT);
            break;

        case kCommandResult_Wait:
            switch (newAutoPolicySate)
            {
#if (defined PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
                case PSM_RDY_EVAL_CHECK_SWAP_TO_SNK:
                    newAutoPolicySate =
                        (pd_auto_policy_state_t)((uint8_t)PSM_RDY_DELAY_FLAG | (uint8_t)PSM_RDY_EVAL_SWAP_TO_SNK);
                    break;
                case PSM_RDY_EVAL_CHECK_SWAP_TO_SRC:
                    newAutoPolicySate =
                        (pd_auto_policy_state_t)((uint8_t)PSM_RDY_DELAY_FLAG | (uint8_t)PSM_RDY_EVAL_SWAP_TO_SRC);
                    break;
#endif
#if (defined PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
                case PSM_RDY_EVAL_CHECK_DR_SWAP:
                    newAutoPolicySate =
                        (pd_auto_policy_state_t)((uint8_t)PSM_RDY_DELAY_FLAG | (uint8_t)PSM_RDY_EVAL_DR_SWAP);
                    break;
#endif
#if defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
                case PSM_RDY_EVAL_CHECK_VCONN_SWAP:
                    newAutoPolicySate =
                        (pd_auto_policy_state_t)((uint8_t)PSM_RDY_DELAY_FLAG | (uint8_t)PSM_RDY_EVAL_VCONN_SWAP);
                    break;
#endif
                default:
                    /* No action required. */
                    break;
            }
            break;

        default:
            /* No action required. */
            break;
    }
    PD_PsmSetAutoPolicyState(pdInstance, newAutoPolicySate);
}

static void PD_PsmExternalPowerChange(pd_instance_t *pdInstance)
{
#if defined(PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
    if (pdInstance->curPowerRole == kPD_PowerRoleSink)
    {
        PD_StackSetEvent(pdInstance, PD_TASK_EVENT_EXTERNAL_POWER_CHANGE);
    }
#if 0
    else
    {
#if defined(USBPD_ENABLE_IMMEDIATE_SWAP_TO_SINK_ON_POWER_LOSS) && (USBPD_ENABLE_IMMEDIATE_SWAP_TO_SINK_ON_POWER_LOSS)
        if (pdInstance->externalPoweredState == 0U)
        {
            PD_PsmSetAutoPolicyState(pdInstance, PSM_RDY_EVAL_CHECK_PARTNER_CAP);
        }
        else
#endif /* USBPD_ENABLE_IMMEDIATE_SWAP_TO_SINK_ON_POWER_LOSS */
        {
            /* A source must immediately send new capabilities */
            (void)PD_Command(pdInstance, (uint32_t)PD_DPM_CONTROL_POWER_NEGOTIATION, NULL);
        }
    }
#endif /* 0 */
#endif /* PD_CONFIG_SOURCE_ROLE_ENABLE */
}
#endif /* PD_CONFIG_ENABLE_AUTO_POLICY */

#if (defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
static void PD_StateWaitReplyExtDataProcess(pd_instance_t *pdInstance,
                                            psm_trigger_info_t *triggerInfo,
                                            message_type_t requiredMsg,
                                            pd_dpm_callback_event_t successCallbackEvent,
                                            pd_dpm_callback_event_t failCallbackEvent,
                                            uint8_t *didNothingStepB)
{
    pd_command_result_t commandResultCallback;

    if (triggerInfo->triggerEvent == PSM_TRIGGER_PD_MSG)
    {
        pdInstance->psmNewState = PE_PSM_STATE_ROLE_RDY_STATE;

        if (triggerInfo->pdMsgType == requiredMsg)
        {
            pd_command_data_param_t extMsgParam;
            (void)PD_TimerClear(pdInstance, tSenderResponseTimer);

            /* the fixed 23 bytes length */
            if (requiredMsg == kPD_MsgBatteryStatus)
            {
                extMsgParam.dataBuffer = (uint8_t *)triggerInfo->pdMsgDataBuffer;
            }
            else
            {
                extMsgParam.dataBuffer = (uint8_t *)triggerInfo->pdMsgDataBuffer + 2U;
            }
            if ((requiredMsg == kPD_MsgSourceCapExtended) &&
                (triggerInfo->pdExtMsgLength) >= PD_EXTENDED_SRC_CAP_DATA_LENGTH)
            {
                extMsgParam.dataLength = PD_EXTENDED_SRC_CAP_DATA_LENGTH;
            }
            else if ((requiredMsg == kPD_MsgStatus) &&
                     ((triggerInfo->pdExtMsgLength) >= PD_EXTENDED_STATUS_MSG_DATA_LENGTH))
            {
                extMsgParam.dataLength = PD_EXTENDED_STATUS_MSG_DATA_LENGTH;
            }
            else if ((requiredMsg == kPD_MsgBatteryCapabilities) &&
                     ((triggerInfo->pdExtMsgLength) >= PD_EXTENDED_BATTERY_CAP_MSG_DATA_LENGTH))
            {
                extMsgParam.dataLength = PD_EXTENDED_BATTERY_CAP_MSG_DATA_LENGTH;
            }
#if (defined PD_CONFIG_PD3_PPS_ENABLE) && (PD_CONFIG_PD3_PPS_ENABLE)
            else if ((requiredMsg == kPD_MsgPpsStatus) &&
                     ((triggerInfo->pdExtMsgLength) >= PD_EXTENDED_PPS_STATUS_MSG_DATA_LENGTH))
            {
                extMsgParam.dataLength = PD_EXTENDED_PPS_STATUS_MSG_DATA_LENGTH;
            }
#endif
            else
            {
                extMsgParam.dataLength = triggerInfo->pdExtMsgLength;
            }
            (void)PD_DpmAppCallback(pdInstance, successCallbackEvent, &extMsgParam, 1);
        }
        else if ((triggerInfo->msgHeader.bitFields.NumOfDataObjs == 0U) &&
                 (triggerInfo->msgHeader.bitFields.messageType == (uint8_t)kPD_MsgNotSupported))
        {
            (void)PD_TimerClear(pdInstance, tSenderResponseTimer);

            commandResultCallback = kCommandResult_NotSupported;
            (void)PD_DpmAppCallback(pdInstance, failCallbackEvent, &commandResultCallback, 1);
        }
        else
        {
            pdInstance->psmNewState = PSM_SEND_SOFT_RESET;
        }
    }
    else if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tSenderResponseTimer) != 0U)
    {
        commandResultCallback = kCommandResult_Error;
        (void)PD_DpmAppCallback(pdInstance, failCallbackEvent, &commandResultCallback, 1);
        pdInstance->psmNewState = PE_PSM_STATE_ROLE_RDY_STATE;
    }
    else
    {
        *didNothingStepB = 1;
    }
}
#endif

/* return 0 wait, 1 start */
static uint8_t PD_PsmStartCommand(pd_instance_t *pdInstance, pd_command_t command, uint8_t isInitiator)
{
#if defined(PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE) && (PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE)
    uint8_t start = 1;

    if (pdInstance->revision >= PD_SPEC_REVISION_30)
    {
        if (isInitiator != 0U)
        {
            if (pdInstance->curPowerRole == kPD_PowerRoleSource)
            {
                start = PD_MsgSrcStartCommand(pdInstance);
            }
            else
            {
                start = PD_MsgSnkCheckStartCommand(pdInstance, command);
            }
        }
    }

    if (start != 0U)
    {
#endif
        pdInstance->commandProcessing  = command;
        pdInstance->commandIsInitiator = isInitiator;
#if defined(PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE) && (PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE)
    }
    return start;
#else
    return 1;
#endif
}

static void PD_PsmCommandFail(pd_instance_t *pdInstance, pd_command_t command)
{
    pd_command_result_t commandResultCallback = kCommandResult_Error;
    pd_dpm_callback_event_t event             = PD_DPM_EVENT_INVALID;

    switch (command)
    {
        case PD_DPM_CONTROL_POWER_NEGOTIATION:
        {
            event = PD_DPM_SRC_RDO_FAIL;
            break;
        }
        case PD_DPM_CONTROL_REQUEST:
        {
            if (pdInstance->curPowerRole == kPD_PowerRoleSink)
            {
                event = PD_DPM_SNK_RDO_FAIL;
            }
            else
            {
                event = PD_DPM_SRC_RDO_FAIL;
            }
            break;
        }
        case PD_DPM_CONTROL_GOTO_MIN:
        {
            if (pdInstance->commandIsInitiator != 0U)
            {
                event = PD_DPM_SRC_GOTOMIN_FAIL;
            }
            else
            {
                event = PD_DPM_SNK_GOTOMIN_FAIL;
            }
            break;
        }
        case PD_DPM_CONTROL_SOFT_RESET:
        {
            event = PD_DPM_SOFT_RESET_FAIL;
            break;
        }
        case PD_DPM_CONTROL_HARD_RESET:
        {
            /* hard reset will always success */
            break;
        }
#if (defined PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
        case PD_DPM_CONTROL_PR_SWAP:
        {
            event = PD_DPM_PR_SWAP_FAIL;
            break;
        }
#endif
#if (defined PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
        case PD_DPM_CONTROL_DR_SWAP:
        {
            event = PD_DPM_DR_SWAP_FAIL;
            break;
        }
#endif
#if (defined PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
        case PD_DPM_CONTROL_VCONN_SWAP:
        {
            event = PD_DPM_VCONN_SWAP_FAIL;
            break;
        }
#endif
        case PD_DPM_CONTROL_GET_PARTNER_SOURCE_CAPABILITIES:
        {
            event = PD_DPM_GET_PARTNER_SRC_CAP_FAIL;
            break;
        }
        case PD_DPM_CONTROL_GET_PARTNER_SINK_CAPABILITIES:
        {
            event = PD_DPM_GET_PARTNER_SNK_CAP_FAIL;
            break;
        }
#if (defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)
        case PD_DPM_CONTROL_DISCOVERY_IDENTITY:
        case PD_DPM_CONTROL_DISCOVERY_SVIDS:
        case PD_DPM_CONTROL_DISCOVERY_MODES:
        case PD_DPM_CONTROL_ENTER_MODE:
        case PD_DPM_CONTROL_EXIT_MODE:
        case PD_DPM_CONTROL_SEND_ATTENTION:
        case PD_DPM_SEND_VENDOR_STRUCTURED_VDM:
        {
            event = PD_DPM_STRUCTURED_VDM_FAIL;
            break;
        }
        case PD_DPM_SEND_UNSTRUCTURED_VDM:
        {
            event = PD_DPM_SEND_UNSTRUCTURED_VDM_FAIL;
            break;
        }
#endif
#if (defined PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
        case PD_DPM_CONTROL_CABLE_RESET:
        {
            break;
        }
#endif
#if (defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
        case PD_DPM_GET_SRC_EXT_CAP:
        {
            event = PD_DPM_GET_SRC_EXT_CAP_FAIL;
            break;
        }
        case PD_DPM_GET_STATUS:
        {
            event = PD_DPM_GET_STATUS_FAIL;
            break;
        }
#if (defined PD_CONFIG_PD3_PPS_ENABLE) && (PD_CONFIG_PD3_PPS_ENABLE)
        case PD_DPM_GET_PPS_STATUS:
        {
            event = PD_DPM_GET_PPS_STATUS_FAIL;
            break;
        }
#endif
        case PD_DPM_GET_BATTERY_CAP:
        {
            event = PD_DPM_GET_BATTERY_CAP_FAIL;
            break;
        }
        case PD_DPM_GET_BATTERY_STATUS:
        {
            event = PD_DPM_GET_BATTERY_STATUS_FAIL;
            break;
        }
        case PD_DPM_GET_MANUFACTURER_INFO:
        {
            event = PD_DPM_GET_MANUFACTURER_INFO_FAIL;
            break;
        }
#if 0
        case PD_DPM_SECURITY_REQUEST:
        {
            event = PD_DPM_SECURITY_REQUEST_FAIL;
            break;
        }
        case PD_DPM_FIRMWARE_UPDATE_REQUEST:
        {
            break;
        }
#endif
        case PD_DPM_ALERT:
        {
            event = PD_DPM_SEND_ALERT_FAIL;
            break;
        }
#endif
#if (defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
        case PD_DPM_FAST_ROLE_SWAP:
        {
            event = PD_DPM_FR_SWAP_FAIL;
            break;
        }
#endif

        default:
            /* No action required. */
            break;
    }

    if (event != PD_DPM_EVENT_INVALID)
    {
#if (defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)
        if (event == PD_DPM_STRUCTURED_VDM_FAIL)
        {
            pd_svdm_command_result_t commandVdmResult;
            commandVdmResult.vdmCommandResult = (uint8_t)kCommandResult_Error;
            commandVdmResult.vdmCommand =
                (uint8_t)pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.command;
            commandVdmResult.vdoData  = NULL;
            commandVdmResult.vdoCount = 0;
            commandVdmResult.vdmSVID  = (uint16_t)pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.SVID;

            (void)PD_DpmAppCallback(pdInstance, event, &commandVdmResult, 1);
        }
        else
#endif
        {
            (void)PD_DpmAppCallback(pdInstance, event, &commandResultCallback, 1);
        }
    }
}

/* process the primary pd commands that don't must require snkRdy or srcRdy */
static uint8_t PD_PsmPrimaryStateProcessDpmMsg(pd_instance_t *pdInstance,
                                               pd_psm_state_t *returnNewState,
                                               psm_trigger_info_t *triggerInfo)
{
    pd_psm_state_t newStateTmp = PSM_UNKNOWN;
    uint8_t didNothing         = 1;

    /* assume the message is processed */
    triggerInfo->triggerEvent = PSM_TRIGGER_NONE;
    switch (triggerInfo->dpmMsg)
    {
        case PD_DPM_CONTROL_HARD_RESET:
            newStateTmp = PSM_HARD_RESET;
            didNothing  = 0;
            break;

        case PD_DPM_CONTROL_SOFT_RESET:
            if (PD_PsmStartCommand(pdInstance, PD_DPM_CONTROL_SOFT_RESET, 1) != 0U)
            {
                newStateTmp = PSM_SEND_SOFT_RESET;
                didNothing  = 0;
            }
            else
            {
                PD_PsmCommandFail(pdInstance, PD_DPM_CONTROL_SOFT_RESET);
            }
            break;
#if 0
        case PD_DPM_CONTROL_EXIT_MODE:
            pdInstance->vdmExitReceived[pdInstance->structuredVdmCommandParameter.vdmSop] = 1;
            break;
#endif

#if (defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
        case PD_DPM_FAST_ROLE_SWAP:
            if (PD_PsmStartCommand(pdInstance, PD_DPM_FAST_ROLE_SWAP, 0) != 0U)
            {
                /* need enter the PE_FRS_SRC_SNK_CC_Signal to wait the fr_swap msg */
                if (pdInstance->frSignaledWaitFrSwap != 0U)
                {
                    pdInstance->psmNewState = PE_FRS_SRC_SNK_CC_Signal;
                }
            }
            break;
#endif

        default:
            /* the event is not processed */
            triggerInfo->triggerEvent = PSM_TRIGGER_DPM_MSG;
            break;
    }

    if (newStateTmp != PSM_UNKNOWN)
    {
        *returnNewState = newStateTmp;
    }

    return didNothing;
}

static pd_psm_state_t PD_PsmSinkHardResetfunction(pd_instance_t *pdInstance)
{
    (void)pdInstance->callbackFns->PD_SnkStopDrawVbus(pdInstance->callbackParam, kVbusPower_InHardReset);
    pdInstance->asmHardResetSnkProcessing = 1;
    return PSM_PE_SNK_TRANSITION_TO_DEFAULT;
}

/* process the primary pd message */
static uint8_t PD_PsmPrimaryStateProcessPdMsg(pd_instance_t *pdInstance,
                                              pd_psm_state_t *returnNewState,
                                              psm_trigger_info_t *triggerInfo)
{
    pd_psm_state_t newStateTmp = PSM_UNKNOWN;
    uint8_t didNothing         = 1;

    if (triggerInfo->triggerEvent == PSM_TRIGGER_RECEIVE_HARD_RESET)
    {
        triggerInfo->triggerEvent = PSM_TRIGGER_NONE;

        if (pdInstance->psmCurState == PSM_PE_BIST_TEST_DATA_MODE)
        {
            /* From the IAS: There is no exit from this test mode except some chip level reset.
               Reset the protocol layer to exit carrier mode */
            (void)PD_PhyControl(pdInstance, PD_PHY_EXIT_BIST, NULL);
        }
        /* Exit any active alternate mode */
        /* All timers and secondary states are reset during this call */
        PD_PsmReset(pdInstance);

        /* Receiving hard reset will discard other command in progress. */
        PD_PsmCommandFail(pdInstance, pdInstance->commandProcessing);
        PD_ConnectSetPowerProgress(pdInstance, kVbusPower_InHardReset);
        /* The reason to uncomment the function is that here the command is started
           but nowhere to end this command. */
        /* (void)PD_PsmStartCommand(pdInstance, PD_DPM_CONTROL_HARD_RESET, 0); */
        if (pdInstance->curPowerRole == kPD_PowerRoleSource)
        {
            /* source receive hard_reset, 1. start tPSHardReset */
            (void)PD_TimerStart(pdInstance, tPSHardResetTimer, T_PS_HARD_RESET);
            newStateTmp = PSM_PE_SRC_HARD_RESET_RECEIVED;
            (void)PD_DpmAppCallback(pdInstance, PD_DPM_SRC_HARD_RESET_REQUEST, NULL, 0);
        }
        else if (pdInstance->curPowerRole == kPD_PowerRoleSink)
        {
            newStateTmp = PD_PsmSinkHardResetfunction(pdInstance);
            (void)PD_DpmAppCallback(pdInstance, PD_DPM_SNK_HARD_RESET_REQUEST, NULL, 0);
        }
        else
        {
            /* No action required. */
        }
        didNothing = 0;
    }
    else if ((triggerInfo->triggerEvent == PSM_TRIGGER_PD_MSG) && (triggerInfo->pdMsgType == kPD_MsgBIST))
    {
#if (defined PD_CONFIG_COMPLIANCE_TEST_ENABLE) && (PD_CONFIG_COMPLIANCE_TEST_ENABLE)
        pd_bist_object_t bistObj;

        /* it the power > 5V, cannot do bist test */
        if (((pdInstance->curDataRole == kPD_DataRoleDFP) &&
             (pdInstance->partnerRdoRequest.bitFields.objectPosition > 1U)) ||
            ((pdInstance->curDataRole == kPD_DataRoleUFP) && (pdInstance->rdoRequest.bitFields.objectPosition > 1U)))
        {
            return didNothing;
        }
        bistObj.objVal = USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)triggerInfo->pdMsgDataBuffer));
        if (bistObj.bitFields.testMode == (uint8_t)kBIST_TestData)
        {
            /* Do nothing, the HW does not have a mode for kBIST_TestData */
            pd_bist_mst_t bistMode = kBIST_TestData;

            (void)PD_PhyControl(pdInstance, PD_PHY_ENTER_BIST, &bistMode);
            newStateTmp = PSM_PE_BIST_TEST_DATA_MODE;
        }
        else
        {
            (void)PD_TimerClear(pdInstance, tNoResponseTimer);
            newStateTmp = PSM_PE_BIST_CARRIER_MODE_2;
        }
#endif
        triggerInfo->triggerEvent = PSM_TRIGGER_NONE;
    }
    else if ((triggerInfo->triggerEvent == PSM_TRIGGER_PD_MSG) && (triggerInfo->pdMsgType == kPD_MsgSoftReset))
    {
        triggerInfo->triggerEvent = PSM_TRIGGER_NONE;
        /* Soft reset is ignored when in BIST carrier mode */
        /* Soft reset is ignored during fast role swap signalling */
        if ((pdInstance->psmCurState != PSM_PE_BIST_CARRIER_MODE_2) &&
            (pdInstance->psmCurState != PE_FRS_SRC_SNK_CC_Signal))
        {
            (void)PD_PsmStartCommand(pdInstance, PD_DPM_CONTROL_SOFT_RESET, 0);
            newStateTmp = PSM_SOFT_RESET;
            didNothing  = 0;
        }
    }
    /* The received message's data role is not right */
    else if (((uint16_t)pdInstance->curDataRole == triggerInfo->msgHeader.bitFields.portDataRole) &&
             (triggerInfo->pdMsgType != kPD_MsgGoodCRC))
    {
        triggerInfo->triggerEvent = PSM_TRIGGER_NON_START;
        newStateTmp               = PSM_EXIT_TO_ERROR_RECOVERY_WITH_DELAY;
        didNothing                = 0;
    }
    /* received source_capabilities */
    else if ((pdInstance->psmCurState != PSM_PE_SNK_DISCOVERY) && (triggerInfo->pdMsgType == kPD_MsgSourceCapabilities))
    {
        triggerInfo->triggerEvent = PSM_TRIGGER_NONE;
        if (pdInstance->pdPowerPortConfig->typecRole == (uint8_t)kPowerConfig_SourceOnly)
        {
            (void)PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(pdInstance, PD_NOT_SUPPORT_REPLY_MSG,
                                                                     pdInstance->psmCurState);
        }
        else
        {
            switch (pdInstance->psmCurState)
            {
#if (defined PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
                case PSM_PE_SRC_DISCOVERY:
                case PSM_PE_SRC_SEND_CAPABILITIES:
                case PSM_PE_SRC_TRANSITION_SUPPLY:
                case PSM_PE_SRC_READY:
                case PSM_PE_SRC_GET_SINK_CAP:
                {
                    newStateTmp = PSM_SEND_SOFT_RESET;
                }
                break;

#if defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
                case PSM_PE_PRS_SRC_SNK_TRANSITION_TO_OFF:
                case PSM_PE_PRS_SRC_SNK_WAIT_SOURCE_ON:
                case PSM_PE_PRS_SRC_SNK_SEND_PR_SWAP:
                case PSM_PE_PRS_SNK_SRC_SOURCE_ON:
                {
                    /* A protocol error during power role swap triggers a Hard Reset */
                    newStateTmp = PSM_HARD_RESET;
                }
                break;
#endif

#endif

/* Source state, message expected */
#if defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
                case PSM_PE_DR_SRC_GET_SOURCE_CAP:
                case PSM_PE_SNK_GET_SOURCE_CAP:
                {
                    uint32_t u32Tmp;
                    pd_capabilities_t sourceCapa;
                    (void)PD_TimerClear(pdInstance, tSenderResponseTimer);
                    /* Clear any pending message now that we have the latest */
                    (void)PD_DpmClearMsg(pdInstance, PD_DPM_CONTROL_GET_PARTNER_SOURCE_CAPABILITIES);
                    sourceCapa.capabilities            = triggerInfo->pdMsgDataBuffer;
                    sourceCapa.capabilitiesCount       = triggerInfo->pdMsgDataLength;
                    pdInstance->partnerSourcePDOsCount = sourceCapa.capabilitiesCount;
                    for (u32Tmp = 0; u32Tmp < sourceCapa.capabilitiesCount; ++u32Tmp)
                    {
                        pdInstance->partnerSourcePDOs[u32Tmp].PDOValue = sourceCapa.capabilities[u32Tmp];
                    }

                    if (pdInstance->psmCurState == PSM_PE_SNK_GET_SOURCE_CAP)
                    {
                        newStateTmp = PSM_PE_SNK_READY;
                    }
                    else
                    {
                        newStateTmp = PSM_PE_SRC_READY;
#if defined(PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
                        if (pdInstance->dumpResponse != 0U)
                        {
                            pdInstance->swapToSnkSrcCapReceived = 1;
                        }
#endif
                    }

                    (void)PD_DpmAppCallback(pdInstance, PD_DPM_GET_PARTNER_SRC_CAP_SUCCESS, &sourceCapa, 1);
                    break;
                }
#endif

/* Sink states, message accepted: */
#if (defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
                case PSM_PE_SNK_WAIT_FOR_CAPABILITIES:
                case PSM_PE_SNK_SELECT_CAPABILITY:
                case PSM_PE_SNK_TRANSITION_SINK:
                case PSM_PE_SNK_READY:
                case PSM_PE_SNK_GIVE_SINK_CAP:
#if defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
                case PSM_PE_PRS_SNK_SRC_TRANSITION_TO_OFF:
                case PSM_PE_PRS_SNK_SRC_SEND_PR_SWAP:
                case PSM_PE_DR_SNK_GET_SINK_CAP:
#endif
                {
                    uint32_t u32Tmp;

                    pd_capabilities_t sourceCapa;
                    PD_PsmCheckRevision(pdInstance, triggerInfo->msgHeader);
                    (void)PD_TimerClear(pdInstance, tSinkWaitCapTimer);
                    pdInstance->psmPresentlyPdConnected  = 1;
                    pdInstance->psmPreviouslyPdConnected = 1;

                    /* Update the PDOs for the EC and send interrupt */
                    sourceCapa.capabilities                          = triggerInfo->pdMsgDataBuffer;
                    sourceCapa.capabilitiesCount                     = triggerInfo->pdMsgDataLength;
                    pdInstance->selfOrPartnerFirstSourcePDO.PDOValue = sourceCapa.capabilities[0];
                    pdInstance->partnerSourcePDOsCount               = sourceCapa.capabilitiesCount;
                    for (u32Tmp = 0; u32Tmp < sourceCapa.capabilitiesCount; ++u32Tmp)
                    {
                        pdInstance->partnerSourcePDOs[u32Tmp].PDOValue = sourceCapa.capabilities[u32Tmp];
                    }
                    if (pdInstance->commandProcessing == PD_DPM_CONTROL_GET_PARTNER_SOURCE_CAPABILITIES)
                    {
                        pdInstance->commandProcessing = PD_DPM_INVALID;
                    }
                    (void)PD_DpmAppCallback(pdInstance, PD_DPM_SNK_RECEIVE_PARTNER_SRC_CAP, &sourceCapa, 0);
#if (defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)
                    /* Ensure secondary state machine does not transition primary state machine */
                    PD_PsmSecondaryStateHandlerTerminate(pdInstance, kPD_MsgSOPInvalid);
#endif
                    newStateTmp = PSM_PE_SNK_EVALUATE_CAPABILITY;
                }
                break;
#endif

                default:
                    /* No action required. */
                    break;
            }
        }

        didNothing = 0;
    }
#if (defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)
    else if ((triggerInfo->triggerEvent == PSM_TRIGGER_PD_MSG) && (triggerInfo->pdMsgType == kPD_MsgVendorDefined) &&
             (triggerInfo->vdmHeader.bitFields.command == (uint8_t)kVDM_ExitMode) &&
             (triggerInfo->vdmHeader.bitFields.commandType == (uint8_t)kVDM_Initiator))
    {
        pdInstance->vdmExitReceived[triggerInfo->pdMsgSop] = triggerInfo->vdmHeader.structuredVdmHeaderVal;
    }
#endif
    else
    {
    }

    if (newStateTmp != PSM_UNKNOWN)
    {
        *returnNewState = newStateTmp;
    }

    return didNothing;
}

static void PD_PsmEndCommand(pd_instance_t *pdInstance)
{
    if (pdInstance->commandProcessing == PD_DPM_INVALID)
    {
        return;
    }
    else
    {
        uint8_t commandFail = 0;

        switch (pdInstance->commandProcessing)
        {
            case PD_DPM_CONTROL_HARD_RESET:
            case PD_DPM_CONTROL_SOFT_RESET:
            case PD_DPM_CONTROL_CABLE_RESET:
            {
                /* process in the state machine */
                break;
            }

#if (defined PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
            case PD_DPM_CONTROL_PR_SWAP:
            {
                switch (pdInstance->psmCurState)
                {
                    case PSM_PE_PRS_SRC_SNK_EVALUATE_PR_SWAP:
                    case PSM_PE_PRS_SRC_SNK_ACCEPT_PR_SWAP:
                    case PSM_PE_PRS_SRC_SNK_TRANSITION_TO_OFF:
                    case PSM_PE_PRS_SRC_SNK_ASSERT_RD:
                    case PSM_PE_PRS_SRC_SNK_WAIT_SOURCE_ON:
                    case PSM_PE_PRS_SRC_SNK_SEND_PR_SWAP:
                    case PSM_PE_PRS_SRC_SNK_REJECT_PR_SWAP:
                    case PSM_PE_PRS_SNK_SRC_EVALUATE_PR_SWAP:
                    case PSM_PE_PRS_SNK_SRC_ACCEPT_PR_SWAP:
                    case PSM_PE_PRS_SNK_SRC_TRANSITION_TO_OFF:
                    case PSM_PE_PRS_SNK_SRC_ASSERT_RP:
                    case PSM_PE_PRS_SNK_SRC_SOURCE_ON:
                    case PSM_PE_PRS_SNK_SRC_SEND_PR_SWAP:
                    case PSM_PE_PRS_SNK_SRC_REJECT_PR_SWAP:
                    case PSM_PE_PRS_EVALUATE_PR_SWAP_WAIT_TIMER_TIME_OUT:
                        /* these states means the pr_swap ams is processing */
                        break;

                    default:
                        /* (void)PD_DpmAppCallback(pdInstance, PD_DPM_PR_SWAP_FAIL, NULL, 1); */
                        commandFail = 1;
                        break;
                }
                break;
            }
#endif

            case PD_DPM_CONTROL_REQUEST:
                switch (pdInstance->psmCurState)
                {
                    case PSM_PE_SRC_NEGOTIATE_CAPABILITY:
                    case PSM_PE_SRC_TRANSITION_SUPPLY:
                    case PSM_PE_SNK_EVALUATE_CAPABILITY:
                    case PSM_PE_SNK_TRANSITION_SINK:
                    case PSM_PE_SNK_SELECT_CAPABILITY:
                    case PSM_PE_SNK_SELECT_CAPABILITY_WAIT_TIMER_TIME_OUT:
                        break;

                    case PSM_PE_SNK_READY:
                        if (pdInstance->psmSnkReceiveRdoWaitRetry != 0U)
                        {
                            /* even in snk_rdy bu it is wait reply */
                        }
                        else
                        {
                            /* (void)PD_DpmAppCallback(pdInstance, PD_DPM_SNK_RDO_FAIL, NULL, 1); */
                            commandFail = 1;
                        }
                        break;

                    default:
                        commandFail = 1;
                        break;
                }
                break;

            case PD_DPM_CONTROL_POWER_NEGOTIATION:
                switch (pdInstance->psmCurState)
                {
                    case PSM_PE_SRC_DISCOVERY:
                    case PSM_PE_SRC_NEGOTIATE_CAPABILITY:
                    case PSM_PE_SRC_TRANSITION_SUPPLY:
                    case PSM_PE_SNK_TRANSITION_SINK:
                    case PSM_PE_SNK_SELECT_CAPABILITY:
                    case PSM_PE_SNK_SELECT_CAPABILITY_WAIT_TIMER_TIME_OUT:
                    case PSM_PE_SRC_SEND_CAPABILITIES:
                    case PSM_PE_SNK_WAIT_FOR_CAPABILITIES:
                    case PSM_PE_SNK_GIVE_SINK_CAP:
                    case PSM_PE_PRS_SNK_SRC_TRANSITION_TO_OFF:
                    case PSM_PE_PRS_SNK_SRC_SEND_PR_SWAP:
                    case PSM_PE_DR_SNK_GET_SINK_CAP:
                        break;

                    default:
                        commandFail = 1;
                        break;
                }
                break;

            case PD_DPM_CONTROL_GET_PARTNER_SOURCE_CAPABILITIES:
                switch (pdInstance->psmCurState)
                {
                    case PSM_PE_DR_SRC_GET_SOURCE_CAP:
                    case PSM_PE_SNK_GET_SOURCE_CAP:
                    case PSM_PE_SNK_READY:
                        break;

                    default:
                        commandFail = 1;
                        break;
                }
                break;

            case PD_DPM_CONTROL_GET_PARTNER_SINK_CAPABILITIES:
                switch (pdInstance->psmCurState)
                {
                    case PSM_PE_DR_SNK_GET_SINK_CAP:
                    case PSM_PE_SRC_GET_SINK_CAP:
                        break;

                    default:
                        commandFail = 1;
                        break;
                }
                break;

            case PD_DPM_CONTROL_GOTO_MIN:
                switch (pdInstance->psmCurState)
                {
                    case PSM_PE_SNK_TRANSITION_SINK:
                    case PSM_PE_SRC_TRANSITION_SUPPLY:
                        break;

                    default:
                        commandFail = 1;
                        break;
                }
                break;

#if (defined PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
            case PD_DPM_CONTROL_DR_SWAP:
                switch (pdInstance->psmCurState)
                {
                    case PSM_PE_DRS_SEND_DR_SWAP:
                    case PSM_PE_DRS_EVALUATE_DR_SWAP:
                    case PSM_PE_DRS_ACCEPT_DR_SWAP:
                    case PSM_PE_DRS_REJECT_DR_SWAP:
                    case PSM_PE_DRS_CHANGE_TO_DFP_OR_UFP:
                        break;

                    default:
                        commandFail = 1;
                        break;
                }
                break;
#endif

#if (defined PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
            case PD_DPM_CONTROL_VCONN_SWAP:
                switch (pdInstance->psmCurState)
                {
                    case PSM_PE_VCS_SEND_SWAP:
                    case PSM_PE_VCS_WAIT_FOR_VCONN:
                    case PSM_PE_VCS_TURN_OFF_VCONN:
                    case PSM_PE_VCS_TURN_ON_VCONN:
                    case PSM_PE_VCS_SEND_PS_RDY:
                    case PSM_PE_VCS_EVALUATE_SWAP:
                    case PSM_PE_VCS_ACCEPT_SWAP:
                    case PSM_PE_VCS_REJECT_SWAP:
                        break;

                    default:
                        commandFail = 1;
                        break;
                }
                break;
#endif

            case PD_DPM_GET_SRC_EXT_CAP:
                switch (pdInstance->psmCurState)
                {
                    case PE_SNK_GET_SOURCE_CAP_EXT:
                    case PE_DR_SRC_GET_SOURCE_CAP_EXT:
                    case PE_SRC_GIVE_SOURCE_CAP_EXT:
                    case PE_DR_SNK_GIVE_SOURCE_CAP_EXT:
                        break;

                    default:
                        commandFail = 1;
                        break;
                }
                break;

#if (defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
            case PD_DPM_GET_STATUS:
                switch (pdInstance->psmCurState)
                {
                    case PE_SNK_Get_Source_Status:
                    case PE_SRC_Give_Source_Status:
                    case PE_SRC_Get_Sink_Status:
                    case PE_SNK_Give_Sink_Status:
                        break;

                    default:
                        commandFail = 1;
                        break;
                }
                break;

#if (defined PD_CONFIG_PD3_PPS_ENABLE) && (PD_CONFIG_PD3_PPS_ENABLE)
            case PD_DPM_GET_PPS_STATUS:
                switch (pdInstance->psmCurState)
                {
                    case PE_SNK_Get_PPS_Status:
                    case PE_SRC_Give_PPS_Status:
                        break;

                    default:
                        commandFail = 1;
                        break;
                }
                break;
#endif

            case PD_DPM_GET_BATTERY_CAP:
                switch (pdInstance->psmCurState)
                {
                    case PE_Give_Battery_Cap:
                    case PE_Get_Battery_Cap:
                        break;

                    default:
                        commandFail = 1;
                        break;
                }
                break;

            case PD_DPM_GET_BATTERY_STATUS:
                switch (pdInstance->psmCurState)
                {
                    case PE_Get_Battery_Status:
                    case PE_Give_Battery_Status:
                        break;

                    default:
                        commandFail = 1;
                        break;
                }
                break;

            case PD_DPM_GET_MANUFACTURER_INFO:
                switch (pdInstance->psmCurState)
                {
                    case PE_Get_Manufacturer_Info:
                    case PE_Give_Manufacturer_Info:
                        break;

                    default:
                        commandFail = 1;
                        break;
                }
                break;

#if 0
            case PD_DPM_SECURITY_REQUEST:
                switch (pdInstance->psmCurState)
                {
                    case PE_Send_Security_Request:
                    case PE_Send_Security_Response:
                    case PE_Security_Response_Received:
                    case PSM_PE_SNK_READY:
                    case PSM_PE_SRC_READY:
                        break;

                    default:
                        commandFail = 1;
                        break;
                }
                break;
#endif

            case PD_DPM_ALERT:
                switch (pdInstance->psmCurState)
                {
                    case PE_SRC_Send_Source_Alert:
                    case PE_SNK_Source_Alert_Received:
                    case PE_SNK_Send_Sink_Alert:
                    case PE_SRC_Sink_Alert_Received:
                        break;

                    default:
                        commandFail = 1;
                        break;
                }
                break;
#endif

#if (defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
            case PD_DPM_FAST_ROLE_SWAP:
                switch (pdInstance->psmCurState)
                {
                    case PE_FRS_SRC_SNK_CC_Signal:
                    case PE_FRS_SRC_SNK_Evaluate_Swap:
                    case PE_FRS_SRC_SNK_Accept_Swap:
                    case PE_FRS_SRC_SNK_Transition_to_off:
                    case PE_FRS_SRC_SNK_Assert_Rd:
                    case PE_FRS_SRC_SNK_Wait_Source_on:
                    case PE_FRS_SNK_SRC_Send_Swap:
                    case PE_FRS_SNK_SRC_Transition_to_off:
                    case PE_FRS_SNK_SRC_Vbus_Applied:
                    case PE_FRS_SNK_SRC_Assert_Rp:
                    case PE_FRS_SNK_SRC_Source_on:
                        break;

                    default:
                        commandFail = 1;
                        break;
                }
                break;
#endif

            default:
                /* No action required. */
                break;
        }

        if (commandFail != 0U)
        {
            PD_PsmCommandFail(pdInstance, pdInstance->commandProcessing);
        }
    }
}

static uint8_t PD_PsmRdyStateCheckDpmMessage(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo)
{
    if (triggerInfo->dpmMsg >= PD_DPM_CONTROL_DISCOVERY_IDENTITY)
    {
        return 0;
    }
    triggerInfo->triggerEvent = PSM_TRIGGER_NONE;
    if (PD_PsmStartCommand(pdInstance, triggerInfo->dpmMsg, 1) == 0U)
    {
        PD_PsmCommandFail(pdInstance, triggerInfo->dpmMsg);
        return 0;
    }
    return 1;
}

static void PD_PsmSinkAndSourceRdyProcessDpmMessage(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo)
{
#if (((defined PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)) || \
     ((defined PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)) ||   \
     ((defined PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)) ||                   \
     ((defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)))
    switch (triggerInfo->dpmMsg)
    {
#if (defined PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
        case PD_DPM_CONTROL_PR_SWAP:
        {
            if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tPrSwapWaitTimer) != 0U)
            {
                if (pdInstance->curPowerRole == kPD_PowerRoleSink)
                {
                    pdInstance->psmNewState = PSM_PE_PRS_SNK_SRC_SEND_PR_SWAP;
                }
                else
                {
                    pdInstance->psmNewState = PSM_PE_PRS_SRC_SNK_SEND_PR_SWAP;
                }
            }
            else
            {
                pdInstance->psmNewState = PSM_PE_PRS_EVALUATE_PR_SWAP_WAIT_TIMER_TIME_OUT;
            }
            break;
        }
#endif

#if (defined PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
        case PD_DPM_CONTROL_DR_SWAP:
        {
#if (defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT)
            /* Check alternate mode */
            if (PD_PsmCheckInAltMode(pdInstance, kPD_MsgSOP) != 0U)
            {
                /* In the alt mode, wait the exit mode */
                /* Send Exit mode(DFP) or attention(UFP) to request to exit active mode. */
                if (PD_AltModeExitModeForDrSwap(pdInstance->altModeHandle) != 0U)
                {
                    pdInstance->drSwapTriggerExitMode = 1;
                    /* Reaching here, the commandProcessing has been assigned to PD_DPM_CONTROL_DR_SWAP, and DFP will
                       send kVDM_ExitMode or UFP will send kVDM_Attention. If the commandProcessing isn't assigned to
                       PD_DPM_INVALID, the kVDM_ExitMode or kVDM_Attention cannot be sent because calling the PD_Command
                       funtion to initiate an AMS will return kStatus_PD_Busy. */
                    pdInstance->commandProcessing = PD_DPM_INVALID;
                }
            }
            else
#endif
            {
                /* Not in the alt mode */
                if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tDrSwapWaitTimer) != 0U)
                {
                    pdInstance->psmNewState = PSM_PE_DRS_SEND_DR_SWAP;
                }
                else
                {
                    pdInstance->psmNewState = PSM_PE_DRS_EVALUATE_DR_SWAP_WAIT_TIMER_TIME_OUT;
                }
            }
            break;
        }
#endif
#if (defined PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
        case PD_DPM_CONTROL_VCONN_SWAP:
        {
            pdInstance->psmNewState = PSM_PE_VCS_SEND_SWAP;
            break;
        }
#endif
#if (defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
        case PD_DPM_GET_BATTERY_CAP:
        {
            pdInstance->psmNewState = PE_Get_Battery_Cap;
            break;
        }
        case PD_DPM_GET_BATTERY_STATUS:
        {
            pdInstance->psmNewState = PE_Get_Battery_Status;
            break;
        }
        case PD_DPM_GET_MANUFACTURER_INFO:
        {
            pdInstance->psmNewState = PE_Get_Manufacturer_Info;
            break;
        }
#if 0
        case PD_DPM_SECURITY_REQUEST:
        {
            pdInstance->psmNewState = PE_Send_Security_Request;
            break;
        }
#endif
#endif

        default:
            /* No action required. */
            break;
    }
#endif
}

#if (defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
static void PD_PsmSinkRdyProcessDpmMessage(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo)
{
    if (PD_PsmRdyStateCheckDpmMessage(pdInstance, triggerInfo) != 0U)
    {
        switch (triggerInfo->dpmMsg)
        {
            case PD_DPM_CONTROL_REQUEST:
            {
                if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tSinkRequestTimer) != 0U)
                {
                    pdInstance->psmNewState = PSM_PE_SNK_SELECT_CAPABILITY;
                }
                else
                {
                    pdInstance->psmNewState = PSM_PE_SNK_SELECT_CAPABILITY_WAIT_TIMER_TIME_OUT;
                }
                break;
            }
            case PD_DPM_CONTROL_GET_PARTNER_SOURCE_CAPABILITIES:
            {
                pdInstance->psmNewState = PSM_PE_SNK_GET_SOURCE_CAP;
                break;
            }
            case PD_DPM_CONTROL_GET_PARTNER_SINK_CAPABILITIES:
            {
                pdInstance->psmNewState = PSM_PE_DR_SNK_GET_SINK_CAP;
                break;
            }
#if (defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
            case PD_DPM_GET_SRC_EXT_CAP:
            {
                pdInstance->psmNewState = PE_SNK_GET_SOURCE_CAP_EXT;
                break;
            }
            case PD_DPM_GET_STATUS:
            {
                pdInstance->psmNewState = PE_SNK_Get_Source_Status;
                break;
            }
#if (defined PD_CONFIG_PD3_PPS_ENABLE) && (PD_CONFIG_PD3_PPS_ENABLE)
            case PD_DPM_GET_PPS_STATUS:
            {
                pdInstance->psmNewState = PE_SNK_Get_PPS_Status;
                break;
            }
#endif
            case PD_DPM_ALERT:
            {
                pdInstance->psmNewState = PE_SNK_Send_Sink_Alert;
                break;
            }
#endif

            default:
                /* No action required. */
                break;
        }

        PD_PsmSinkAndSourceRdyProcessDpmMessage(pdInstance, triggerInfo);
    }
}
#endif

#if (defined PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
static void PD_PsmSourceRdyProcessDpmMessage(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo)
{
    if (PD_PsmRdyStateCheckDpmMessage(pdInstance, triggerInfo) != 0U)
    {
        switch (triggerInfo->dpmMsg)
        {
            case PD_DPM_CONTROL_POWER_NEGOTIATION:
            {
                pdInstance->psmNewState = PSM_PE_SRC_SEND_CAPABILITIES;
                break;
            }
            case PD_DPM_CONTROL_GOTO_MIN:
            {
                pdInstance->psmGotoMinTx = 1;
                pdInstance->psmNewState  = PSM_PE_SRC_TRANSITION_SUPPLY;
                break;
            }
            case PD_DPM_CONTROL_GET_PARTNER_SOURCE_CAPABILITIES:
            {
                pdInstance->psmNewState = PSM_PE_DR_SRC_GET_SOURCE_CAP;
                break;
            }
            case PD_DPM_CONTROL_GET_PARTNER_SINK_CAPABILITIES:
            {
                pdInstance->psmNewState = PSM_PE_SRC_GET_SINK_CAP;
                break;
            }
#if (defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
            case PD_DPM_GET_SRC_EXT_CAP:
            {
                pdInstance->psmNewState = PE_DR_SRC_GET_SOURCE_CAP_EXT;
                break;
            }
            case PD_DPM_GET_STATUS:
            {
                pdInstance->psmNewState = PE_SRC_Get_Sink_Status;
                break;
            }
            case PD_DPM_ALERT:
            {
                pdInstance->psmNewState = PE_SRC_Send_Source_Alert;
                break;
            }
#endif

            default:
                /* No action required. */
                break;
        }

        PD_PsmSinkAndSourceRdyProcessDpmMessage(pdInstance, triggerInfo);
    }
}
#endif

static void PD_PsmRdyProcessPdMessage(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo)
{
    pd_psm_state_t replyNotSupport = PSM_UNKNOWN;
    pd_psm_state_t sinkOrSoruceNotSupportState;
    uint8_t msgProcessed = 1;

    if ((triggerInfo->pdMsgSop != kPD_MsgSOP) || (triggerInfo->pdMsgType == kPD_MsgVendorDefined))
    {
        return;
    }

    triggerInfo->triggerEvent = PSM_TRIGGER_NONE;

/* note: vdm message will be processed before this */
#if (defined PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
    if (pdInstance->curPowerRole == kPD_PowerRoleSource)
    {
        sinkOrSoruceNotSupportState = PSM_PE_SRC_READY;
        switch (triggerInfo->pdMsgType)
        {
            case kPD_MsgGotoMin:
            {
                /* not_supported */
                replyNotSupport = sinkOrSoruceNotSupportState;
                break;
            }
            case kPD_MsgGetSourceCap:
            {
                pdInstance->psmNewState = PSM_PE_SRC_SEND_CAPABILITIES;
#if defined(PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE) && (PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE)
                pdInstance->powerNegotiationInitiator = 0;
#endif
                break;
            }

            case kPD_MsgGetSinkCap:
            {
                if (pdInstance->pdPowerPortConfig->typecRole == (uint8_t)kPowerConfig_SourceOnly)
                {
                    replyNotSupport = sinkOrSoruceNotSupportState;
                }
                else
                {
                    pdInstance->psmNewState = PSM_PE_DR_SRC_GIVE_SINK_CAP;
                }
                break;
            }

            case kPD_MsgRequest:
            {
                pdInstance->partnerRdoRequest.rdoVal =
                    USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)triggerInfo->pdMsgDataBuffer));
#if (defined PD_CONFIG_PD3_PPS_ENABLE) && (PD_CONFIG_PD3_PPS_ENABLE)
                /* SourcePPSCommTimer */
                if (PD_PsmSourceIsPPSRDO(pdInstance) != 0U)
                {
                    (void)PD_TimerStart(pdInstance, tSourcePPSCommTimer, T_PPS_TIMEOUT);
                }
#endif
                pdInstance->psmNewState = PSM_PE_SRC_NEGOTIATE_CAPABILITY;
                (void)PD_PsmStartCommand(pdInstance, PD_DPM_CONTROL_REQUEST, 0);
                break;
            }

            case kPD_MsgGetStatus:
            {
                pdInstance->alertWaitReply = 0;
                pdInstance->psmNewState    = PE_SRC_Give_Source_Status;
                break;
            }

            case kPD_MsgFrSwap:
            {
#if (defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
                if ((PD_PsmIsDualRole(pdInstance) != 0U) && (pdInstance->frSignaledWaitFrSwap != 0U))
                {
                    /* no need to enter PE_FRS_SRC_SNK_CC_Signal */
                    PD_ConnectSetPowerProgress(pdInstance, kVbusPower_InFRSwap);
                    pdInstance->frSignaledWaitFrSwap = 0u;
                    pdInstance->psmNewState          = PE_FRS_SRC_SNK_Evaluate_Swap;
                }
                else
                {
                    pdInstance->psmNewState = PSM_HARD_RESET;
                }
#else
                replyNotSupport = sinkOrSoruceNotSupportState;
#endif
                break;
            }

#if (defined PD_CONFIG_PD3_PPS_ENABLE) && (PD_CONFIG_PD3_PPS_ENABLE)
            case kPD_MsgGetPpsStatus:
                pdInstance->psmNewState = PE_SRC_Give_PPS_Status;
                break;
#endif

            default:
                msgProcessed = 0;
                break;
        }
    }
    else
#endif
#if (defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
    {
        sinkOrSoruceNotSupportState = PSM_PE_SNK_READY;
        switch (triggerInfo->pdMsgType)
        {
            case kPD_MsgRequest:
            {
                replyNotSupport = sinkOrSoruceNotSupportState;
                break;
            }

            case kPD_MsgGotoMin:
            {
                (void)PD_PsmStartCommand(pdInstance, PD_DPM_CONTROL_GOTO_MIN, 0);
                pdInstance->psmGotoMinRx = 1;
                pdInstance->psmNewState  = PSM_PE_SNK_TRANSITION_SINK;
                break;
            }

            case kPD_MsgGetSinkCap:
            {
                pdInstance->psmNewState = PSM_PE_SNK_GIVE_SINK_CAP;
                break;
            }

            case kPD_MsgGetSourceCap:
            {
                if (pdInstance->pdPowerPortConfig->typecRole == (uint8_t)kPowerConfig_SinkOnly)
                {
                    replyNotSupport = sinkOrSoruceNotSupportState;
                }
                else
                {
                    pdInstance->psmNewState = PSM_PE_DR_SNK_GIVE_SOURCE_CAP;
                }
                break;
            }

            case kPD_MsgGetStatus:
            {
                pdInstance->alertWaitReply = 0;
                pdInstance->psmNewState    = PE_SNK_Give_Sink_Status;
                break;
            }

#if (defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
            case kPD_MsgFrSwap:
            {
                /* not_supported, only source can receive this msg */
                replyNotSupport = PSM_PE_SNK_READY;
                break;
            }
#endif

            default:
                msgProcessed = 0;
                break;
        }
    }
#else
    {
    }
#endif

    if (msgProcessed == 0U)
    {
        pd_command_result_t commandResultCallback;
#if (defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
        pd_extended_msg_header_t extHeader;

        extHeader.extendedMsgHeaderVal =
            USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)triggerInfo->pdMsgDataBuffer));
#endif

        triggerInfo->triggerEvent = PSM_TRIGGER_NONE;
        /* sink and source all process these messages similarly */
        /* note: vdm message will be processed before this */
        switch (triggerInfo->pdMsgType)
        {
            case kPD_MsgPrSwap:
            {
#if (defined PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
                if (PD_PsmIsDualRole(pdInstance) != 0U)
                {
                    (void)PD_PsmStartCommand(pdInstance, PD_DPM_CONTROL_PR_SWAP, 0);
                    if (pdInstance->curPowerRole == kPD_PowerRoleSource)
                    {
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
                        /*  auto accept is true and partner source cap is not got */
                        if ((pdInstance->swapToSnkSrcCapReceived == 0U) && (PD_POLICY_SUPPORT(pdInstance)) &&
                            (PD_POLICY_GET_AUTO_ACCEPT_PRSWAP_AS_SOURCE_SUPPORT(pdInstance)) &&
                            (PD_POLICY_GET_AUTO_ACCEPT_PRSWAP_AS_SOURCE(pdInstance) ==
                             (uint8_t)kAutoRequestProcess_Accept))
                        {
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY_LOG) && (PD_CONFIG_ENABLE_AUTO_POLICY_LOG)
                            (void)PRINTF("auto reply wait for pr swap, and start to get the partner's source caps\r\n");
#endif
                            if (PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(pdInstance, kPD_MsgWait,
                                                                                   PSM_PE_DR_SRC_GET_SOURCE_CAP) != 0U)
                            {
                                PD_PsmSetAutoPolicyState(pdInstance, PSM_RDY_EVAL_INIT);

                                commandResultCallback = kCommandResult_Wait;
                                (void)PD_DpmAppCallback(pdInstance, PD_DPM_PR_SWAP_FAIL, &commandResultCallback, 1);

                                if (PD_PsmStartCommand(pdInstance, PD_DPM_CONTROL_GET_PARTNER_SOURCE_CAPABILITIES, 1) !=
                                    0U)
                                {
                                    pdInstance->dumpResponse = 1;
                                }
                                else
                                {
                                    /* If fail, wait for next kPD_MsgPrSwap from the partner Sink. */
                                }
                            }
                        }
                        else
#endif
                        {
                            pdInstance->psmNewState = PSM_PE_PRS_SRC_SNK_EVALUATE_PR_SWAP;
                        }
                    }
                    else
                    {
                        pdInstance->psmNewState = PSM_PE_PRS_SNK_SRC_EVALUATE_PR_SWAP;
                    }
                }
                else
#endif
                {
                    replyNotSupport = sinkOrSoruceNotSupportState;
                }
                break;
            }

            case kPD_MsgDrSwap:
            {
#if (defined PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
                if (PD_PsmCheckInAltMode(pdInstance, kPD_MsgSOP) != 0U)
                {
                    pdInstance->psmNewState = PSM_HARD_RESET;
                }
                else
                {
                    if (pdInstance->pdPowerPortConfig->dataFunction == (uint8_t)kDataConfig_DRD)
                    {
                        (void)PD_PsmStartCommand(pdInstance, PD_DPM_CONTROL_DR_SWAP, 0);
                        pdInstance->psmNewState = PSM_PE_DRS_EVALUATE_DR_SWAP;
                    }
                    else
                    {
                        replyNotSupport = sinkOrSoruceNotSupportState;
                    }
                }
#else
                replyNotSupport = sinkOrSoruceNotSupportState;
#endif
                break;
            }

            case kPD_MsgVconnSwap:
            {
#if (defined PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
                if (pdInstance->pdPowerPortConfig->vconnSupported != 0U)
                {
                    (void)PD_PsmStartCommand(pdInstance, PD_DPM_CONTROL_VCONN_SWAP, 0);
                    pdInstance->psmNewState = PSM_PE_VCS_EVALUATE_SWAP;
                }
                else
#endif
                {
                    replyNotSupport = sinkOrSoruceNotSupportState;
                }
                break;
            }

            case kPD_MsgGetSourceCapExtended:
            {
                if (pdInstance->pdPowerPortConfig->typecRole == (uint8_t)kPowerConfig_SinkOnly)
                {
                    replyNotSupport = sinkOrSoruceNotSupportState;
                }
                else
                {
                    if (pdInstance->curPowerRole == kPD_PowerRoleSource)
                    {
                        pdInstance->psmNewState = PE_SRC_GIVE_SOURCE_CAP_EXT;
                    }
                    else
                    {
                        pdInstance->psmNewState = PE_DR_SNK_GIVE_SOURCE_CAP_EXT;
                    }
                }
                break;
            }

            case kPD_MsgAlert:
            {
                pd_command_data_param_t commandExtParam;
                if (pdInstance->curPowerRole == kPD_PowerRoleSink)
                {
                    pdInstance->psmNewState = PE_SNK_Source_Alert_Received;
                }
                else
                {
                    pdInstance->psmNewState = PE_SRC_Sink_Alert_Received;
                }
                commandExtParam.dataBuffer = (uint8_t *)triggerInfo->pdMsgDataBuffer;
                commandExtParam.dataLength = 4;
                commandExtParam.sop        = (uint8_t)kPD_MsgSOP;
                (void)PD_DpmAppCallback(pdInstance, PD_DPM_ALERT_RECEIVED, &commandExtParam, 0);
                if (commandExtParam.resultStatus == (uint8_t)kCommandResult_NotSupported)
                {
                    (void)PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(pdInstance, kPD_MsgNotSupported,
                                                                             PE_PSM_STATE_ROLE_RDY_STATE);
                }
                break;
            }

            case kPD_MsgNotSupported:
                commandResultCallback = kCommandResult_NotSupported;
#if 0
                if (pdInstance->commandProcessing == PD_DPM_SECURITY_REQUEST)
                {
                    (void)PD_DpmAppCallback(pdInstance, PD_DPM_SECURITY_REQUEST_FAIL, &commandResultCallback, 1);
                }
                else
#endif
                if (pdInstance->alertWaitReply != 0U)
                {
                    pdInstance->alertWaitReply = 0;
                    (void)PD_DpmAppCallback(pdInstance, PD_DPM_SEND_ALERT_FAIL, &commandResultCallback, 0);
                }
                else
                {
                }
                break;

#if (defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
            case kPD_MsgGetBatteryCap:
            case kPD_MsgGetBatteryStatus:
            case kPD_MsgGetManufacturerInfo:
                pdInstance->commandExtParamCallback.dataBuffer = (uint8_t *)&(pdInstance->receivedData[1]);
                pdInstance->commandExtParamCallback.dataBuffer += 2; /* data pos */
                pdInstance->commandExtParamCallback.dataLength = extHeader.bitFields.dataSize;
                pdInstance->commandExtParamCallback.sop        = (uint8_t)triggerInfo->pdMsgSop;
                if (triggerInfo->pdMsgType == kPD_MsgGetBatteryCap)
                {
                    pdInstance->psmNewState = PE_Give_Battery_Cap;
                }
                else if (triggerInfo->pdMsgType == kPD_MsgGetBatteryStatus)
                {
                    pdInstance->psmNewState = PE_Give_Battery_Status;
                }
                else
                {
                    pdInstance->psmNewState = PE_Give_Manufacturer_Info;
                }
                break;

            case kPD_MsgSecurityRequest:
            {
#if 0
                /* (initiator) chunked request received */
                pdInstance->psmNewState = PE_Send_Security_Response;
#endif
                replyNotSupport = sinkOrSoruceNotSupportState;
                break;
            }

#if 0
            case kPD_MsgSecurityResponse:
            {
                /* receive all the data */
                pdInstance->psmNewState = PE_Security_Response_Received;
                pdInstance->commandExtParamCallback.dataBuffer = (uint8_t *)&(pdInstance->receivedData[1]);
                pdInstance->commandExtParamCallback.dataBuffer += 2; /* data pos */
                pdInstance->commandExtParamCallback.dataLength = extHeader.bitFields.dataSize;
                pdInstance->commandExtParamCallback.sop = (uint8_t)triggerInfo->pdMsgSop;
                break;
            }
#endif
#endif

            case kPD_MsgPing:
                /* nothing need do, in case the default soft_reset */
                break;

            default:
                if (((triggerInfo->pdMsgType > kPD_MsgInvalid) && (triggerInfo->pdMsgType <= kPD_MsgSoftReset)) ||
                    ((triggerInfo->pdMsgType >= kPD_MsgNotSupported) && (triggerInfo->pdMsgType <= kPD_MsgFrSwap)) ||
                    ((triggerInfo->pdMsgType >= kPD_MsgSourceCapabilities) &&
                     (triggerInfo->pdMsgType <= kPD_MsgVendorDefined)) ||
                    ((triggerInfo->pdMsgType >= kPD_MsgSourceCapExtended) &&
                     (triggerInfo->pdMsgType <= kPD_MsgFirmwareUpdaetResponse)))
                {
                    /* unexpected message */
                    pdInstance->psmNewState = PSM_SEND_SOFT_RESET;
                }
                else
                {
                    /* receive unrecognized message */
                    replyNotSupport = sinkOrSoruceNotSupportState;
                }
                break;
        }
    }

    if (replyNotSupport != PSM_UNKNOWN)
    {
        (void)PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(pdInstance, PD_NOT_SUPPORT_REPLY_MSG, replyNotSupport);
    }
}

static uint8_t PD_PsmCanPendingReceive(pd_instance_t *pdInstance)
{
    if ((pdInstance->psmCurState == PSM_PE_SNK_STARTUP) || (pdInstance->psmCurState == PSM_PE_SNK_DISCOVERY) ||
        (pdInstance->psmCurState == PSM_PE_SNK_TRANSITION_TO_DEFAULT) || (pdInstance->psmCurState == PSM_HARD_RESET))
    {
        return 0;
    }

    if ((pdInstance->psmNewState == PSM_PE_SNK_STARTUP) || (pdInstance->psmNewState == PSM_PE_SNK_DISCOVERY) ||
        (pdInstance->psmNewState == PSM_PE_SNK_TRANSITION_TO_DEFAULT) || (pdInstance->psmNewState == PSM_HARD_RESET) ||
        (pdInstance->psmNewState == PSM_PE_SNK_WAIT_FOR_CAPABILITIES))
    {
        return 0;
    }

    return 1;
}

static void PD_PsmProcessImportEventBeforeNextStateMachine(pd_instance_t *pdInstance)
{
#if (defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
    uint32_t taskEventSet = 0;

    (void)OSA_EventGet(pdInstance->taskEventHandle, PD_TASK_EVENT_FR_SWAP_SINGAL, &taskEventSet);

    if (taskEventSet != 0U)
    {
        (void)OSA_EventClear(pdInstance->taskEventHandle, PD_TASK_EVENT_FR_SWAP_SINGAL);
        if (pdInstance->curPowerRole == kPD_PowerRoleSink)
        {
            pdInstance->psmNewState = PE_FRS_SNK_SRC_Send_Swap;
        }
    }
#endif
}

static uint8_t PD_PsmNoResponseHardResetCountCheck(pd_instance_t *pdInstance)
{
    if ((PD_TimerCheckValidTimeOut(pdInstance, tNoResponseTimer) != 0U) &&
        (pdInstance->psmHardResetCount > N_HARD_RESET_COUNT))
    {
        if (pdInstance->psmPreviouslyPdConnected != 0U)
        {
            pdInstance->psmNewState = PSM_EXIT_TO_ERROR_RECOVERY;
            return 1;
        }
#if (defined PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
        else
        {
            if (pdInstance->curPowerRole == kPD_PowerRoleSource)
            {
                pdInstance->psmNewState = PSM_PE_SRC_DISABLED;
                return 1;
            }
        }
#endif
    }
    return 0;
}

#if (defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)) || \
    (defined(PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)) ||   \
    (defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT))
static void PD_PsmStateWaitReply(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo, uint8_t *didNothingStepB)
{
    pd_psm_state_t acceptState;
    pd_psm_state_t otherState;
    pd_dpm_callback_event_t failCallbackEvent;
    pd_command_result_t commandResultCallback = kCommandResult_None;

    switch (pdInstance->psmCurState)
    {
#if defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
        case PSM_PE_PRS_SRC_SNK_SEND_PR_SWAP:
            acceptState       = PSM_PE_PRS_SRC_SNK_TRANSITION_TO_OFF;
            otherState        = PSM_PE_SRC_READY;
            failCallbackEvent = PD_DPM_PR_SWAP_FAIL;
            break;
        case PSM_PE_PRS_SNK_SRC_SEND_PR_SWAP:
            acceptState       = PSM_PE_PRS_SNK_SRC_TRANSITION_TO_OFF;
            otherState        = PSM_PE_SNK_READY;
            failCallbackEvent = PD_DPM_PR_SWAP_FAIL;
            break;
#endif
#if defined(PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
        case PSM_PE_DRS_SEND_DR_SWAP:
            acceptState       = PSM_PE_DRS_CHANGE_TO_DFP_OR_UFP;
            otherState        = (pd_psm_state_t)pdInstance->psmDrSwapPrevState;
            failCallbackEvent = PD_DPM_DR_SWAP_FAIL;
            break;
#endif
#if defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
        case PSM_PE_VCS_SEND_SWAP:
            acceptState       = (pdInstance->psmPresentlyVconnSource == kPD_IsVconnSource ? PSM_PE_VCS_WAIT_FOR_VCONN :
                                                                                      PSM_PE_VCS_TURN_ON_VCONN);
            otherState        = (pd_psm_state_t)pdInstance->psmVconnSwapPrevState;
            failCallbackEvent = PD_DPM_VCONN_SWAP_FAIL;
            break;
#endif
        default:
            acceptState       = PSM_UNKNOWN;
            otherState        = PSM_UNKNOWN;
            failCallbackEvent = PD_DPM_EVENT_INVALID;
            break;
    }

    if ((triggerInfo->triggerEvent == PSM_TRIGGER_PD_MSG) && (failCallbackEvent != PD_DPM_EVENT_INVALID))
    {
        switch (triggerInfo->pdMsgType)
        {
            case kPD_MsgAccept:
            {
#if defined(PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
                if (failCallbackEvent == PD_DPM_DR_SWAP_FAIL)
                {
                    pdInstance->curDataRole =
                        (pdInstance->curDataRole == kPD_DataRoleUFP) ? kPD_DataRoleDFP : kPD_DataRoleUFP;
                    (void)PD_PhyControl(pdInstance, PD_PHY_SET_MSG_HEADER_INFO, NULL);
                }
#endif
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
                commandResultCallback = kCommandResult_Accept;
#endif
                pdInstance->psmNewState = acceptState;
                break;
            }

            case kPD_MsgReject:
            {
                pdInstance->psmNewState = otherState;
                commandResultCallback   = kCommandResult_Reject;
                (void)PD_DpmAppCallback(pdInstance, failCallbackEvent, &commandResultCallback, 1);
                break;
            }

            case kPD_MsgWait:
            {
                pdInstance->psmNewState = otherState;
#if defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
                if (failCallbackEvent == PD_DPM_PR_SWAP_FAIL)
                {
                    (void)PD_TimerStart(pdInstance, tPrSwapWaitTimer, T_PRSWAP_WAIT);
                }
#endif
#if defined(PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
                if (failCallbackEvent == PD_DPM_DR_SWAP_FAIL)
                {
                    (void)PD_TimerStart(pdInstance, tDrSwapWaitTimer, T_DRSWAP_WAIT);
                }
#endif
                commandResultCallback = kCommandResult_Wait;
                (void)PD_DpmAppCallback(pdInstance, failCallbackEvent, &commandResultCallback, 1);
                break;
            }

            default:
                if (triggerInfo->pdMsgType != kPD_MsgInvalid)
                {
                    /* SourceCapabilities and VendorDefined are handled in the global section. */
                    /* soft reset other packets */
                    pdInstance->psmNewState = PSM_SEND_SOFT_RESET;
                    commandResultCallback   = kCommandResult_Error;
                }
                break;
        }
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
        if (commandResultCallback != kCommandResult_None)
        {
            PD_PsmReadyAutoPolicyResult(pdInstance, commandResultCallback);
        }
#endif
    }
    else if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tSenderResponseTimer) != 0U)
    {
        pdInstance->psmNewState = otherState;
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
        PD_PsmReadyAutoPolicyResult(pdInstance, kCommandResult_Timeout);
#endif
    }
    else
    {
        *didNothingStepB = 1;
    }
}
#endif

#if defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
static void PD_PsmPowerSwapSinkSourceTransitionOff(pd_instance_t *pdInstance,
                                                   psm_trigger_info_t *triggerInfo,
                                                   pd_psm_state_t newState,
                                                   uint8_t *didNothingStepB)
{
    if (triggerInfo->triggerEvent == PSM_TRIGGER_PD_MSG)
    {
        if ((triggerInfo->pdMsgSop == kPD_MsgSOP) && (triggerInfo->pdMsgType == kPD_MsgPsRdy))
        {
            (void)PD_TimerClear(pdInstance, tPSSourceOffTimer);
            pdInstance->curPowerRole = kPD_PowerRoleSource;
            (void)PD_PhyControl(pdInstance, PD_PHY_SET_MSG_HEADER_INFO, NULL);
            pdInstance->psmNewState = newState;
        }
        else if ((triggerInfo->pdMsgSop == kPD_MsgSOP) && (triggerInfo->pdMsgType == kPD_MsgPing))
        {
            /* Remain in the same state */
            pdInstance->psmNewState = pdInstance->psmCurState;
        }
        else
        {
            /* A protocol error during power role swap triggers a Hard Reset */
            pdInstance->psmNewState = PSM_HARD_RESET;
        }
    }
    else if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tPSSourceOffTimer) != 0U)
    {
        pdInstance->psmNewState = PSM_EXIT_TO_ERROR_RECOVERY;
    }
    else
    {
        *didNothingStepB = 1;
    }
}

static void PD_PsmPowerSwapSinkOpenVbus(pd_instance_t *pdInstance, pd_dpm_callback_event_t callbackEvent)
{
    if (PD_PsmSendControlTransitionWithErrorRecovery(pdInstance, kPD_MsgPsRdy, PSM_PE_SRC_STARTUP) != 0U)
    {
        /* Swap from SINK to SOURCE */
        /* pr swap end */
        PD_ConnectSetPowerProgress(pdInstance, kVbusPower_Stable);
#if defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
        /* Cable plug will need a soft reset */
        pdInstance->psmCablePlugResetNeeded = 1;
#endif
        pdInstance->enterSrcFromSwap = 1;
        (void)PD_DpmAppCallback(pdInstance, callbackEvent, NULL, 1);
    }
}
#endif

#if (defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)) || \
    (defined(PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)) ||   \
    (defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT))
static void PD_PsmEvaluateSwap(pd_instance_t *pdInstance,
                               pd_dpm_callback_event_t requestEvent,
                               pd_psm_state_t acceptState,
                               pd_psm_state_t rejectState)
{
    if (requestEvent != PD_DPM_EVENT_INVALID)
    {
        (void)PD_DpmAppCallback(pdInstance, requestEvent, &pdInstance->commandEvaluateResult, 0);
    }
    if (pdInstance->commandEvaluateResult == kCommandResult_Accept)
    {
        pdInstance->psmNewState = acceptState;
    }
    else
    {
        pdInstance->psmNewState = rejectState;
    }
}
#endif

#if defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
static void PD_PsmPowerSwapAssertRdRp(pd_instance_t *pdInstance, pd_power_role_t powerRole, pd_psm_state_t nextState)
{
    pdInstance->curPowerRole = powerRole;
    PD_ConnectSetPRSwapRole(pdInstance, pdInstance->curPowerRole);
    pdInstance->psmNewState = nextState;
}
#endif

#if (defined(PD_CONFIG_VBUS_ALARM_SUPPORT) && (PD_CONFIG_VBUS_ALARM_SUPPORT)) || \
    (defined(PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE))
static inline void PD_PsmSetVbusAlarmAndSinkDisconnect(pd_instance_t *pdInstance)
{
    pd_rdo_t rdo;
    pd_source_pdo_t pdo;
    uint16_t minVoltage;
#if defined(PD_CONFIG_VBUS_ALARM_SUPPORT) && (PD_CONFIG_VBUS_ALARM_SUPPORT)
    uint16_t maxVoltage;
    uint32_t paramVal;
#endif

    if (pdInstance->curPowerRole == kPD_PowerRoleSource)
    {
        rdo.rdoVal   = pdInstance->partnerRdoRequest.rdoVal;
        if (rdo.bitFields.objectPosition < 1U)
        {
            return;
        }
        pdo.PDOValue = pdInstance->pdPowerPortConfig->sourceCaps[rdo.bitFields.objectPosition - 1U];
    }
    else
    {
        rdo.rdoVal   = pdInstance->rdoRequest.rdoVal;
        if (rdo.bitFields.objectPosition < 1U)
        {
            return;
        }
        pdo.PDOValue = pdInstance->partnerSourcePDOs[rdo.bitFields.objectPosition - 1U].PDOValue;
    }

    switch ((pd_pdo_type_t)pdo.commonPDO.pdoType)
    {
        case kPDO_Fixed:
            minVoltage = (uint16_t)pdo.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT;
#if defined(PD_CONFIG_VBUS_ALARM_SUPPORT) && (PD_CONFIG_VBUS_ALARM_SUPPORT)
            maxVoltage = minVoltage / 100U * 105U + 500U; /* vSrcNew(max) + vSrcValid(max) */
#endif
            minVoltage = minVoltage / 100U * 95U - 500U; /* vSrcNew(min) + vSrcValid(min) */
            break;

        case kPDO_Battery:
            minVoltage = (uint16_t)pdo.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT;
#if defined(PD_CONFIG_VBUS_ALARM_SUPPORT) && (PD_CONFIG_VBUS_ALARM_SUPPORT)
            maxVoltage = (uint16_t)pdo.batteryPDO.maxVoltage * PD_PDO_VOLTAGE_UNIT;
#endif
            break;

        case kPDO_Variable:
            minVoltage = (uint16_t)pdo.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT;
#if defined(PD_CONFIG_VBUS_ALARM_SUPPORT) && (PD_CONFIG_VBUS_ALARM_SUPPORT)
            maxVoltage = (uint16_t)pdo.variablePDO.maxVoltage * PD_PDO_VOLTAGE_UNIT;
#endif
            break;

        case kPDO_APDO:
            minVoltage = (uint16_t)rdo.programRDOBitFields.outputVoltage * PD_PRDO_VOLTAGE_UNIT;
#if defined(PD_CONFIG_VBUS_ALARM_SUPPORT) && (PD_CONFIG_VBUS_ALARM_SUPPORT)
            maxVoltage = minVoltage / 100U * 105U + 100U; /* vPpsNew(max) + vPpsValid(max) */
#endif
            minVoltage = minVoltage / 100U * 95U - 100U; /* vPpsNew(min) + vPpsValid(min) */
            break;

        default:
            /* cannot reach here normally. */
            /* minVoltage = 5000U * 95U / 1000U - 500U;
            maxVoltage = 5000U * 105U / 100U + 500U; */
            break;
    }

#if defined(PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
    if (pdInstance->curPowerRole == kPD_PowerRoleSink)
    {
        if (minVoltage < 750U)
        {
            return;
        }
        minVoltage -= 750U; /* Voltage - Cable IR Drop(750mV) */
        /* Set VBUS_SINK_DISCONNECT_THRESHOLD according to power negotiation result. */
        (void)PD_PhyControl(pdInstance, PD_PHY_SET_VBUS_SINK_DISCONNECT, &minVoltage);
    }
#endif

#if defined(PD_CONFIG_VBUS_ALARM_SUPPORT) && (PD_CONFIG_VBUS_ALARM_SUPPORT)
    paramVal = ((uint32_t)maxVoltage << 16U) | (uint32_t)minVoltage;
    (void)PD_PhyControl(pdInstance, PD_PHY_INFORM_VBUS_VOLTAGE_RANGE, &paramVal);
#endif
}
#endif

static void PD_PsmResetStateWhenEnterReadyState(pd_instance_t *pdInstance)
{
#if ((defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE))
    pdInstance->psmSecondaryState[0] = PSM_IDLE;
    pdInstance->psmSecondaryState[1] = PSM_IDLE;
    pdInstance->psmSecondaryState[2] = PSM_IDLE;
#endif
    pdInstance->psmExplicitContractExisted = 1;
#if defined(PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE) && (PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE)
    pdInstance->powerNegotiationInitiator = 1;
#endif
#if (defined PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
    if (PD_CheckWhetherInitiateCableDiscoveryIdentityOrNot(pdInstance) != 0U)
    {
        pdInstance->pendingSOP = (uint8_t)kPD_MsgSOPMask | (uint8_t)kPD_MsgSOPpMask | (uint8_t)kPD_MsgSOPppMask;
        (void)PD_TimerStart(pdInstance, tDiscoverIdentityTimer, T_DISCOVER_IDENTITY);
    }
    else
#endif
    {
        pdInstance->pendingSOP = (uint8_t)kPD_MsgSOPMask;
    }

#if (defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
    if (pdInstance->frsEnabled == 0U)
    {
        PD_FRSControl(pdInstance, 1);
        pdInstance->frsEnabled = 1;
    }
#endif

    /* Stop ignoring droops on VBus */
    /* kVbusPower_ChangeInProgress is done */
    PD_ConnectSetPowerProgress(pdInstance, kVbusPower_Stable);

#if (defined(PD_CONFIG_VBUS_ALARM_SUPPORT) && (PD_CONFIG_VBUS_ALARM_SUPPORT)) || \
    (defined(PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE))
    PD_PsmSetVbusAlarmAndSinkDisconnect(pdInstance);
#endif
}

#if defined(PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
static void PD_StateSendReplyExtDataTransition(pd_instance_t *pdInstance, message_type_t msgType)
{
    if (pdInstance->commandExtParamCallback.resultStatus != (uint8_t)kCommandResult_NotSupported)
    {
        if (msgType == kPD_MsgBatteryStatus)
        {
            (void)PD_MsgSendDataTransition(pdInstance, msgType, pdInstance->commandExtParamCallback.dataLength >> 2,
                                           (void *)pdInstance->commandExtParamCallback.dataBuffer,
                                           PE_PSM_STATE_ROLE_RDY_STATE, PSM_CHECK_ASYNC_RX, PSM_SEND_SOFT_RESET);
        }
        else
        {
            (void)PD_MsgSendExtTransition(pdInstance, kPD_MsgSOP, msgType,
                                          pdInstance->commandExtParamCallback.dataLength,
                                          pdInstance->commandExtParamCallback.dataBuffer, PE_PSM_STATE_ROLE_RDY_STATE,
                                          PSM_CHECK_ASYNC_RX, PSM_SEND_SOFT_RESET);
        }
    }
    else
    {
        (void)PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(pdInstance, kPD_MsgNotSupported,
                                                                 PE_PSM_STATE_ROLE_RDY_STATE);
    }
}
#endif

static void PD_PsmCheckChunkedFeature(pd_instance_t *pdInstance, pd_rdo_t rdo)
{
    if ((rdo.bitFields.unchunkedSupported != 0U) &&
        (pdInstance->selfOrPartnerFirstSourcePDO.fixedPDO.unchunkedSupported != 0U))
    {
        pdInstance->unchunkedFeature = 1;
    }
    else
    {
        pdInstance->unchunkedFeature = 0;
    }
}

#if (defined PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
static void PD_PsmTurnOffVconnAndVbus(pd_instance_t *pdInstance, pd_vbus_power_progress_t powerState)
{
#if (defined PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
    if (pdInstance->callbackFns->PD_ControlVconn != NULL)
    {
        (void)pdInstance->callbackFns->PD_ControlVconn(pdInstance->callbackParam, 0);
    }
#endif
    if (pdInstance->callbackFns->PD_SrcTurnOffVbus != NULL)
    {
        (void)pdInstance->callbackFns->PD_SrcTurnOffVbus(pdInstance->callbackParam, powerState);
    }
}
#endif

/* state process small functions when enter the state */
static inline void PD_PsmEnterIdleState(pd_instance_t *pdInstance)
{
    pdInstance->psmHardResetCount = 0;
#if (defined PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
    pdInstance->psmSendCapsCounter = 0;
#endif
#if (defined PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
    pdInstance->cableDiscoverIdentityCounter = 0;
#endif
    /* kVbusPower_InHardReset is end */
    PD_ConnectSetPowerProgress(pdInstance, kVbusPower_Stable);
    PD_MsgReset(pdInstance);
    PD_PsmReset(pdInstance);
    if (pdInstance->pdConfig->deviceType == (uint8_t)kDeviceType_AlternateModeProduct)
    {
        /* If we are an alternate mode adapter, then set tAMETimeout. */
        (void)PD_TimerStart(pdInstance, tAMETimeoutTimer, T_AME_TIMEOUT);
    }

    if (pdInstance->curPowerRole == kPD_PowerRoleSource)
    {
#if (defined PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
        pdInstance->enterSrcFromSwap = 0;
        pdInstance->psmNewState      = PSM_PE_SRC_STARTUP;
#else
        pdInstance->psmNewState = PSM_BYPASS;
#endif
    }
    else if (pdInstance->curPowerRole == kPD_PowerRoleSink)
    {
#if (defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
        pdInstance->psmNewState = PSM_PE_SNK_STARTUP;
#else
        pdInstance->psmNewState = PSM_BYPASS;
#endif
    }
    else
    {
        /* No action required. */
    }
}

static inline void PD_PsmEnterHardResetState(pd_instance_t *pdInstance)
{
#if ((defined PD_CONFIG_EXTENDED_CHUNK_LAYER_SUPPORT) && (PD_CONFIG_EXTENDED_CHUNK_LAYER_SUPPORT))
    PD_MsgChunkingLayerResetFlags(pdInstance);
#endif
    /* Do this as early as possible, to prevent disconnects
     * Hard Reset is in progress, ignore VBus going away.
     * source need set this too, because judge vbus too (dead battery related) for disconnect.
     */
    /* If there is an AMS in progress, end this AMS with failure before starting Hard Reset. */
    PD_PsmCommandFail(pdInstance, pdInstance->commandProcessing);
    PD_ConnectSetPowerProgress(pdInstance, kVbusPower_InHardReset);

    PD_PsmReset(pdInstance);
    /* Delay for 1ms to make sure any previous GoodCRC tranmission has ended. */
    (void)PD_PsmTimerWait(pdInstance, tDelayTimer, mSec(1), 0U);
    /* source send hard_reset, 1. send hard_reset msg. */
    PD_MsgSendHardOrCableReset(pdInstance, 0);
    if (pdInstance->curPowerRole == kPD_PowerRoleSource)
    {
        (void)PD_TimerStart(pdInstance, tPSHardResetTimer, T_PS_HARD_RESET);
        (void)PD_DpmAppCallback(pdInstance, PD_DPM_SRC_HARD_RESET_REQUEST, NULL, 0);
    }
    else
    {
        (void)PD_DpmAppCallback(pdInstance, PD_DPM_SNK_HARD_RESET_REQUEST, NULL, 0);
    }

    pdInstance->psmHardResetCount++;
}

static inline void PD_PsmEnterSendSoftResetState(pd_instance_t *pdInstance)
{
    /* Interate at least 1 time to cover an RX between our initial discard, and tx completion */
    uint8_t retryCount = 1; /* as retry time variable */
#if ((defined PD_CONFIG_EXTENDED_CHUNK_LAYER_SUPPORT) && (PD_CONFIG_EXTENDED_CHUNK_LAYER_SUPPORT))
    PD_MsgChunkingLayerResetFlags(pdInstance);
#endif
#if (defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)
#if 0
    if (pdInstance->psmSoftResetSop != kPD_MsgSOP)
    {
        PD_PsmSecondaryStateHandlerTerminate(pdInstance, pdInstance->psmSoftResetSop);
        /*PD_PsmSecondaryStateHandler(pdInstance, pdInstance->psmSoftResetSop, pdInstance->psmSoftResetSop,
         */
        /*&triggerInfo); */
    }
#endif
#endif

    do
    {
        if (PD_MsgSend(pdInstance, kPD_MsgSOP, kPD_MsgSoftReset, 2, NULL) == kStatus_PD_Success)
        {
            (void)PD_TimerStart(pdInstance, tSenderResponseTimer, T_SENDER_RESPONSE);
        }
        else if (((PD_MsgRecvPending(pdInstance) != 0U) &&
                  ((uint8_t)((MSG_DATA_HEADER & PD_MSG_HEADER_MESSAGE_TYPE_MASK) >> PD_MSG_HEADER_MESSAGE_TYPE_POS) ==
                   (uint8_t)kPD_MsgSoftReset)) ||
                 (pdInstance->hardResetReceived == 1U))
        {
            (void)PD_DpmAppCallback(pdInstance, PD_DPM_SOFT_RESET_SUCCESS, NULL,
                                    (pdInstance->commandProcessing == PD_DPM_CONTROL_SOFT_RESET) ? 1U : 0U);
        }
        else if ((PD_MsgRecvPending(pdInstance) != 0U) && (retryCount > 0U))
        {
            retryCount--;
            continue;
        }
        else
        {
            if (pdInstance->commandProcessing == PD_DPM_CONTROL_SOFT_RESET)
            {
                (void)PD_DpmAppCallback(pdInstance, PD_DPM_SOFT_RESET_FAIL, NULL, 1);
            }
            pdInstance->psmNewState = PSM_HARD_RESET;
        }
        break;
    } while (true);
}

#if (defined PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
static inline void PD_PsmEnterSrcStartUpState(pd_instance_t *pdInstance)
{
    pdInstance->psmPresentlyPdConnected = 0;
    pdInstance->psmSendCapsCounter      = 0;
#if (defined PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
    pdInstance->cableDiscoverIdentityCounter = 0;
#endif
#if (defined PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE) && (PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE)
    pdInstance->powerNegotiationInitiator = 0;
#endif

    /* kVbusPower_InHardReset is end */
    PD_ConnectSetPowerProgress(pdInstance, kVbusPower_Stable);
    /* Delay for 1ms to make sure any previous GoodCRC tranmission has ended. */
    (void)PD_PsmTimerWait(pdInstance, tDelayTimer, mSec(1), 0U);
    PD_MsgReset(pdInstance);
    (void)PD_PhyControl(pdInstance, PD_PHY_SET_MSG_HEADER_INFO, NULL);
    pdInstance->pendingSOP = (uint8_t)kPD_MsgSOPMask;
    PD_MsgStartReceive(pdInstance);
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
    PD_PsmSetAutoPolicyState(pdInstance, PSM_RDY_EVAL_INIT);
#endif

    /* A large value allows the sink to debounce CC and VBUS after the connection. */
    if (pdInstance->enterSrcFromSwap != 0U)
    {
        (void)PD_TimerStart(pdInstance, tSwapSourceStartTimer, T_SEND_SOURCE_CAP);
    }
    else
    {
        pdInstance->psmNewState = PSM_PE_SRC_IMPLICIT_CABLE_DISCOVERY;
    }
}

static inline void PD_PsmEnterImplicitCableDiscoveryState(pd_instance_t *pdInstance)
{
#if (defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)) && \
    (defined(PD_CONFIG_SRC_AUTO_DISCOVER_CABLE_PLUG) && (PD_CONFIG_SRC_AUTO_DISCOVER_CABLE_PLUG))
    /* (The initial source && Ra present) || (swap to source from initial sink, cannot detect the Ra) */
    if (((pdInstance->raPresent != 0U) || (pdInstance->initialPowerRole == kPD_PowerRoleSink)) &&
        (pdInstance->psmPresentlyVconnSource == kPD_IsVconnSource) && (pdInstance->psmCableIdentitiesDataCount == 0U))
    {
        pdInstance->pendingSOP = (uint8_t)kPD_MsgSOPMask | (uint8_t)kPD_MsgSOPpMask | (uint8_t)kPD_MsgSOPppMask;
        if (pdInstance->psmCablePlugResetNeeded != 0U)
        {
            pdInstance->psmNewSecondaryState[1] = PSM_PE_SRC_IMPLICIT_CABLE_SOFT_RESET;
        }
        else
        {
            pdInstance->psmNewSecondaryState[1] = PSM_PE_SRC_VDM_IDENTITY_REQUEST;
        }
    }
    else
#endif
    {
#if (defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)) && \
    (defined(PD_CONFIG_SRC_AUTO_DISCOVER_CABLE_PLUG) && (PD_CONFIG_SRC_AUTO_DISCOVER_CABLE_PLUG))
        pdInstance->pendingSOP = (uint8_t)kPD_MsgSOPMask;
#endif
        pdInstance->psmNewState = PSM_PE_SRC_SEND_CAPABILITIES;
    }
}

static inline void PD_PsmEnterSrcSendCapsState(pd_instance_t *pdInstance)
{
    pdInstance->selfOrPartnerFirstSourcePDO.PDOValue = pdInstance->pdPowerPortConfig->sourceCaps[0];
    if (pdInstance->commandProcessing == PD_DPM_INVALID)
    {
        /* PD_DPM_INVALID means that it is the first time to send source capabilities,
           the GetSourceCap message is received or the SoftReset process(receive or send) has been completed. */
        (void)PD_PsmStartCommand(pdInstance, PD_DPM_CONTROL_POWER_NEGOTIATION,
#if defined(PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE) && (PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE)
                                 pdInstance->powerNegotiationInitiator
#else
                                 0
#endif
        );
    }
    pdInstance->psmSendCapsCounter++;
    if (PD_MsgSendDataTransition(
            pdInstance, kPD_MsgSourceCapabilities, pdInstance->pdPowerPortConfig->sourceCapCount,
            PD_PsmGetSourcePDOs(pdInstance), PE_PSM_STATE_NO_CHANGE, PSM_CHECK_ASYNC_RX,
            (pdInstance->psmPresentlyPdConnected != 0U) ? PSM_SEND_SOFT_RESET : PSM_PE_SRC_DISCOVERY) != 0U)
    {
        pdInstance->psmPresentlyPdConnected  = 1;
        pdInstance->psmPreviouslyPdConnected = 1;
        (void)PD_TimerClear(pdInstance, tNoResponseTimer);
        pdInstance->psmHardResetCount  = 0;
        pdInstance->psmSendCapsCounter = 0;
        (void)PD_TimerStart(pdInstance, tSenderResponseTimer, T_SENDER_RESPONSE);
    }
}

static inline void PD_PsmEnterSrcNegotiateCapState(pd_instance_t *pdInstance)
{
    pd_negotiate_power_request_t negotiateResult;

    PD_PsmCheckChunkedFeature(pdInstance, pdInstance->partnerRdoRequest);
    negotiateResult.rdo             = pdInstance->partnerRdoRequest;
    negotiateResult.negotiateResult = (uint8_t)kCommandResult_Accept;
    (void)PD_DpmAppCallback(pdInstance, PD_DPM_SRC_RDO_REQUEST, &negotiateResult, 0);
    if (negotiateResult.negotiateResult == (uint8_t)kCommandResult_Accept)
    {
        pdInstance->psmGotoMinTx = 0;
        pdInstance->psmNewState  = PSM_PE_SRC_TRANSITION_SUPPLY;
    }
    else
    {
        pdInstance->commandEvaluateResult = (pd_command_result_t)negotiateResult.negotiateResult;
        pdInstance->psmNewState           = PSM_PE_SRC_CAPABILITY_RESPONSE;
    }
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
    PD_PsmSetAutoPolicyState(pdInstance, PSM_RDY_EVAL_INIT);
#endif
}

static inline void PD_PsmEnterSrcTransitionSupplyState(pd_instance_t *pdInstance)
{
    uint8_t enable;

    if (PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(
            pdInstance, ((pdInstance->psmGotoMinTx != 0U) ? kPD_MsgGotoMin : kPD_MsgAccept), PE_PSM_STATE_NO_CHANGE) !=
        0U)
    {
        /* transition power */
        (void)PD_PsmTimerWait(pdInstance, tDelayTimer, T_SRC_TRANSITION, PD_TASK_EVENT_RECEIVED_HARD_RESET);
        if (pdInstance->psmGotoMinTx != 0U)
        {
            (void)pdInstance->callbackFns->PD_SrcGotoMinReducePower(pdInstance->callbackParam);
        }
        else
        {
            /* Start ignoring droops on VBus */
            PD_ConnectSetPowerProgress(pdInstance, kVbusPower_ChangeInProgress);
            (void)pdInstance->callbackFns->PD_SrcTurnOnRequestVbus(pdInstance->callbackParam,
                                                                   pdInstance->partnerRdoRequest);

            enable = 1;
            (void)PD_PhyControl(pdInstance, PD_PHY_CONTROL_VBUS_AUTO_DISCHARGE, &enable);
        }
    }
    else
    {
        if (pdInstance->psmGotoMinTx != 0U)
        {
            pdInstance->psmGotoMinTx = 0;
        }
    }
}

static inline void PD_PsmEnterSrcCapResponseState(pd_instance_t *pdInstance)
{
    message_type_t msgType;
    if (pdInstance->commandEvaluateResult == kCommandResult_Reject)
    {
        msgType = kPD_MsgReject;
    }
    else
    {
        msgType                           = kPD_MsgWait;
        pdInstance->commandEvaluateResult = kCommandResult_Error;
    }

    if (PD_PsmSendControlTransitionWithHardReset(pdInstance, msgType, PE_PSM_STATE_NO_CHANGE) != 0U)
    {
        (void)PD_DpmAppCallback(pdInstance, PD_DPM_SRC_RDO_FAIL, &pdInstance->commandEvaluateResult, 1);
        if (pdInstance->psmExplicitContractExisted == 0U)
        {
            pdInstance->psmNewState = PSM_PE_SRC_WAIT_NEW_CAPABILITIES;
        }
        else
        {
            uint8_t stillValid = 0;
            (void)PD_DpmAppCallback(pdInstance, PD_DPM_SRC_CONTRACT_STILL_VALID, &stillValid, 0);
            if ((stillValid != 0U) || (msgType == kPD_MsgWait))
            {
                pdInstance->psmNewState = PSM_PE_SRC_READY;
            }
            else
            {
                pdInstance->psmNewState = PSM_HARD_RESET;
            }
        }
    }
}

static inline void PD_PsmEnterSrcTransitionToDefaultState(pd_instance_t *pdInstance)
{
    uint32_t eventSet                      = 0;
    pdInstance->psmPresentlyPdConnected    = 0;
    pdInstance->psmExplicitContractExisted = 0;
    pdInstance->psmHardResetNeedsVSafe0V   = 1;

    /* source send hard_reset, 2. change to supply vsafe5v. */
    /* source receive hard_reset, 2. change to supply vsafe5v. */
    (void)PD_TimerClear(pdInstance, tSrcRecoverTimer);

    do
    {
        PD_PsmTurnOffVconnAndVbus(pdInstance, kVbusPower_InHardReset);
        PD_DpmDischargeVbus(pdInstance, 1);

        if (PD_PsmCheckVsafe0V(pdInstance) != 0U)
        {
            break;
        }
        /* When VBUS decreases to vSafe0V, the EXTENDED_STATUS.vSafe0V register will be set and an interrupt
           will be asserted, so software does not need to check vSafe0V constantly. */
        (void)OSA_EventWait(pdInstance->taskEventHandle, PD_TASK_EVENT_ALL, 0, 5, &eventSet);
    } while (true);

    PD_DpmDischargeVbus(pdInstance, 0);
    /* source send hard_reset, 3. start tSrcRecover timer. */
    /* source receive hard_reset, 3. start tSrcRecover timer. */
    /* 1. vbus is vsafe0v -> start the tSrcRecover timer -> After tSrcRecover the Source applies power to
     * VBUS */
    (void)PD_TimerStart(pdInstance, tSrcRecoverTimer, T_SRC_RECOVER);

    /* Message reception should not be re-enabled until PE_SNK_Startup */
    PD_MsgReset(pdInstance);
    /* Change our data role to DFP, and turn off vconn */
    pdInstance->curDataRole             = kPD_DataRoleDFP;
    pdInstance->psmPresentlyVconnSource = kPD_IsVconnSource;
    (void)PD_PhyControl(pdInstance, PD_PHY_SET_MSG_HEADER_INFO, NULL);
#if defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
    PD_DpmDischargeVconn(pdInstance, 1);
#endif /* PD_CONFIG_VCONN_SUPPORT */
}

static inline void PD_PsmEnterGiveSrcCapState(pd_instance_t *pdInstance)
{
    if (((pdInstance->pdPowerPortConfig->typecRole == (uint8_t)kPowerConfig_SourceOnly) ||
         (pdInstance->pdPowerPortConfig->typecRole == (uint8_t)kPowerConfig_SinkDefault) ||
         (pdInstance->pdPowerPortConfig->typecRole == (uint8_t)kPowerConfig_SourceDefault) ||
         (pdInstance->pdPowerPortConfig->typecRole == (uint8_t)kPowerConfig_DRPToggling)) &&
        (pdInstance->pdPowerPortConfig->sourceCaps != NULL))
    {
        (void)PD_MsgSendDataTransition(pdInstance, kPD_MsgSourceCapabilities,
                                       pdInstance->pdPowerPortConfig->sourceCapCount, PD_PsmGetSourcePDOs(pdInstance),
                                       PE_PSM_STATE_ROLE_RDY_STATE, PSM_CHECK_ASYNC_RX, PSM_SEND_SOFT_RESET);
    }
    else
    {
        (void)PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(pdInstance, PD_NOT_SUPPORT_REPLY_MSG,
                                                                 PE_PSM_STATE_ROLE_RDY_STATE);
    }
}
#endif

static inline void PD_PsmEnterGetSinkOrSourceCapState(pd_instance_t *pdInstance, pd_psm_state_t prevState)
{
    message_type_t msgType     = kPD_MsgGetSinkCap;
    pd_psm_state_t successSate = PE_PSM_STATE_NO_CHANGE;
    if ((pdInstance->psmCurState == PSM_PE_DR_SRC_GET_SOURCE_CAP) ||
        (pdInstance->psmCurState == PSM_PE_SNK_GET_SOURCE_CAP))
    {
        msgType = kPD_MsgGetSourceCap;
    }
    if (pdInstance->psmCurState == PSM_PE_SNK_GET_SOURCE_CAP)
    {
        successSate = PSM_PE_SNK_READY;
    }

    (void)PD_PsmSendControlTransitionWithSendResponserTimeOut(pdInstance, msgType, successSate, prevState,
                                                              PSM_SEND_SOFT_RESET);
}

#if (defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)) || \
    ((defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE))
static inline void PD_PsmEnterGiveSinkCapState(pd_instance_t *pdInstance)
{
    if ((pdInstance->pdPowerPortConfig->typecRole != (uint8_t)kPowerConfig_SourceOnly) &&
        (pdInstance->pdPowerPortConfig->sinkCaps != NULL))
    {
        (void)PD_MsgSendDataTransition(pdInstance, kPD_MsgSinkCapabilities, pdInstance->pdPowerPortConfig->sinkCapCount,
                                       PD_PsmGetSinkPDOs(pdInstance), PE_PSM_STATE_ROLE_RDY_STATE, PSM_CHECK_ASYNC_RX,
                                       PSM_SEND_SOFT_RESET);
    }
    else
    {
        (void)PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(pdInstance, PD_NOT_SUPPORT_REPLY_MSG,
                                                                 PE_PSM_STATE_ROLE_RDY_STATE);
    }
}
#endif

#if (defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
static inline void PD_PsmEnterSnkStartUpState(pd_instance_t *pdInstance)
{
    /* Delay for 1ms to make sure any previous GoodCRC tranmission has ended. */
    (void)PD_PsmTimerWait(pdInstance, tDelayTimer, mSec(1), 0U);
    PD_MsgReset(pdInstance);
    pdInstance->pendingSOP = (uint8_t)kPD_MsgSOPMask;
    PD_MsgStartReceive(pdInstance);
    (void)PD_PhyControl(pdInstance, PD_PHY_SET_MSG_HEADER_INFO, NULL);
#if (defined PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
    pdInstance->cableDiscoverIdentityCounter = 0;
#endif
    pdInstance->psmPresentlyPdConnected = 0;
/* Do not clear previouslyPdConnected here */
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
    PD_PsmSetAutoPolicyState(pdInstance, PSM_RDY_EVAL_INIT);
#endif
    pdInstance->psmNewState = PSM_PE_SNK_DISCOVERY;
}

#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
void PD_AutoSinkNegotiation(pd_instance_t *pdInstance, pd_rdo_t *rdoRequest)
{
    uint8_t snkCapIndex;
    uint8_t srcCapIndex;
    pd_sink_pdo_t sinkPdo;
    pd_source_pdo_t sourcePDO;
    uint32_t requestVoltageMin = 0; /* mV */
    uint32_t requestVoltageMax = 0; /* mV */
    uint32_t requestCurrent    = 0; /* mA */
    uint32_t currentPower      = 0; /* mW */

    rdoRequest->bitFields.objectPosition = 1;
    rdoRequest->bitFields.giveBack       = 0;
    rdoRequest->bitFields.noUsbSuspend   = 1;
#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
    rdoRequest->bitFields.unchunkedSupported = 1;
#endif
    sourcePDO.PDOValue                             = pdInstance->partnerSourcePDOs[0].PDOValue;
    sinkPdo.PDOValue                               = pdInstance->pdPowerPortConfig->sinkCaps[0];
    rdoRequest->bitFields.usbCommunicationsCapable = sinkPdo.fixedPDO.usbCommunicationsCapable;

    if (sourcePDO.fixedPDO.maxCurrent >= sinkPdo.fixedPDO.operateCurrent)
    {
        rdoRequest->bitFields.operateValue       = sinkPdo.fixedPDO.operateCurrent;
        rdoRequest->bitFields.capabilityMismatch = 0;
    }
    else
    {
        rdoRequest->bitFields.operateValue       = sourcePDO.fixedPDO.maxCurrent;
        rdoRequest->bitFields.capabilityMismatch = 1;
    }
    rdoRequest->bitFields.maxOrMinOperateValue = rdoRequest->bitFields.operateValue;
    currentPower =
        rdoRequest->bitFields.operateValue * PD_PDO_CURRENT_UNIT * sinkPdo.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT;

    for (snkCapIndex = (uint8_t)pdInstance->pdPowerPortConfig->sinkCapCount - 1U; snkCapIndex > 0U; --snkCapIndex)
    {
        sinkPdo.PDOValue = pdInstance->pdPowerPortConfig->sinkCaps[snkCapIndex];

        switch ((pd_pdo_type_t)sinkPdo.commonPDO.pdoType)
        {
            case kPDO_Fixed:
            {
                requestVoltageMin = sinkPdo.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT;
                requestVoltageMax = requestVoltageMin;
                requestCurrent    = sinkPdo.fixedPDO.operateCurrent * PD_PDO_CURRENT_UNIT;
                break;
            }

            case kPDO_Variable:
            {
                requestVoltageMin = sinkPdo.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT;
                requestVoltageMax = sinkPdo.variablePDO.maxVoltage * PD_PDO_VOLTAGE_UNIT;
                requestCurrent    = sinkPdo.variablePDO.operateCurrent * PD_PDO_CURRENT_UNIT;
                break;
            }

            case kPDO_Battery:
            {
                requestVoltageMin = sinkPdo.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT;
                requestVoltageMax = sinkPdo.batteryPDO.maxVoltage * PD_PDO_VOLTAGE_UNIT;
                requestCurrent    = (sinkPdo.batteryPDO.operatePower * PD_PDO_POWER_UNIT) * 1000U / requestVoltageMin;
                break;
            }

            case kPDO_APDO:
            {
                requestVoltageMin = sinkPdo.apdoPDO.minVoltage * PD_APDO_VOLTAGE_UNIT;
                requestVoltageMax = sinkPdo.apdoPDO.maxVoltage * PD_APDO_VOLTAGE_UNIT;
                requestCurrent    = sinkPdo.apdoPDO.maxCurrent * PD_APDO_CURRENT_UNIT;
                break;
            }

            default:
                /* No action required. */
                break;
        }

        for (srcCapIndex = 0; srcCapIndex < pdInstance->partnerSourcePDOsCount; ++srcCapIndex)
        {
            uint32_t power;
            uint32_t current;
            uint8_t misMatch       = 0;
            uint32_t pdoMaxVoltage = 0;
            uint32_t pdoMinVoltage = 0;
            sourcePDO.PDOValue     = pdInstance->partnerSourcePDOs[srcCapIndex].PDOValue;

            switch ((pd_pdo_type_t)sourcePDO.commonPDO.pdoType)
            {
                case kPDO_Fixed:
                {
                    pdoMinVoltage = sourcePDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT;
                    pdoMaxVoltage = pdoMinVoltage;
                    if ((pdoMaxVoltage > requestVoltageMax) || (pdoMinVoltage < requestVoltageMin))
                    {
                        continue;
                    }
                    current = requestCurrent;
                    if (current > sourcePDO.fixedPDO.maxCurrent * PD_PDO_CURRENT_UNIT)
                    {
                        current  = sourcePDO.fixedPDO.maxCurrent * PD_PDO_CURRENT_UNIT;
                        misMatch = 1U;
                    }
                    power = pdoMinVoltage * current / 1000U;
                    if (power > currentPower)
                    {
                        currentPower                               = power;
                        rdoRequest->bitFields.operateValue         = current / PD_PDO_CURRENT_UNIT;
                        rdoRequest->bitFields.capabilityMismatch   = misMatch;
                        rdoRequest->bitFields.objectPosition       = (uint32_t)srcCapIndex + 1U;
                        rdoRequest->bitFields.maxOrMinOperateValue = rdoRequest->bitFields.operateValue;
                    }
                    break;
                }

                case kPDO_Variable:
                {
                    pdoMaxVoltage = sourcePDO.variablePDO.maxVoltage * PD_PDO_VOLTAGE_UNIT;
                    pdoMinVoltage = sourcePDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT;
                    if ((pdoMaxVoltage > requestVoltageMax) || (pdoMinVoltage < requestVoltageMin))
                    {
                        continue;
                    }
                    current = requestCurrent;
                    if (current > sourcePDO.variablePDO.maxCurrent * PD_PDO_CURRENT_UNIT)
                    {
                        current  = sourcePDO.variablePDO.maxCurrent * PD_PDO_CURRENT_UNIT;
                        misMatch = 1;
                    }
                    power = pdoMinVoltage * current / 1000U;
                    if (power > currentPower)
                    {
                        currentPower                               = power;
                        rdoRequest->bitFields.operateValue         = current / PD_PDO_CURRENT_UNIT;
                        rdoRequest->bitFields.capabilityMismatch   = misMatch;
                        rdoRequest->bitFields.objectPosition       = (uint32_t)srcCapIndex + 1U;
                        rdoRequest->bitFields.maxOrMinOperateValue = rdoRequest->bitFields.operateValue;
                    }
                    break;
                }

                case kPDO_Battery:
                {
                    pdoMaxVoltage = sourcePDO.batteryPDO.maxVoltage * PD_PDO_VOLTAGE_UNIT;
                    pdoMinVoltage = sourcePDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT;
                    if ((pdoMaxVoltage > requestVoltageMax) || (pdoMinVoltage < requestVoltageMin))
                    {
                        continue;
                    }
                    power = requestVoltageMax * requestCurrent / 1000U;
                    if (power > (sourcePDO.batteryPDO.maxAllowPower * PD_PDO_POWER_UNIT))
                    {
                        power    = sourcePDO.batteryPDO.maxAllowPower * PD_PDO_POWER_UNIT;
                        misMatch = 1;
                    }
                    if (power > currentPower)
                    {
                        currentPower                               = power;
                        rdoRequest->bitFields.operateValue         = power / PD_PDO_POWER_UNIT;
                        rdoRequest->bitFields.capabilityMismatch   = misMatch;
                        rdoRequest->bitFields.objectPosition       = (uint32_t)srcCapIndex + 1U;
                        rdoRequest->bitFields.maxOrMinOperateValue = rdoRequest->bitFields.operateValue;
                    }
                    break;
                }

                case kPDO_APDO:
                {
                    uint32_t voltage;
                    pdoMaxVoltage = sourcePDO.apdoPDO.maxVoltage * PD_APDO_VOLTAGE_UNIT;
                    pdoMinVoltage = sourcePDO.apdoPDO.minVoltage * PD_APDO_VOLTAGE_UNIT;
                    if (((requestVoltageMin < pdoMinVoltage) && (requestVoltageMax < pdoMinVoltage)) ||
                        ((requestVoltageMin > pdoMaxVoltage) && (requestVoltageMax > pdoMaxVoltage)))
                    {
                        continue;
                    }

                    voltage = requestVoltageMax;
                    if (voltage > pdoMaxVoltage)
                    {
                        voltage = pdoMaxVoltage;
                    }
                    current = requestCurrent;
                    if (current > sourcePDO.apdoPDO.maxCurrent * PD_APDO_CURRENT_UNIT)
                    {
                        current  = sourcePDO.apdoPDO.maxCurrent * PD_APDO_CURRENT_UNIT;
                        misMatch = 1U;
                    }
                    power = voltage * current / 1000U;
                    if (power > currentPower)
                    {
                        currentPower                               = power;
                        rdoRequest->bitFields.operateValue         = current / PD_PRDO_CURRENT_UNIT;
                        rdoRequest->bitFields.capabilityMismatch   = misMatch;
                        rdoRequest->bitFields.objectPosition       = (uint32_t)srcCapIndex + 1U;
                        rdoRequest->bitFields.maxOrMinOperateValue = rdoRequest->bitFields.operateValue;
                    }
                    break;
                }

                default:
                    /* No action required. */
                    break;
            }
        }
    }
}
#endif

static inline void PD_PsmEnterSnkEvaluateCapState(pd_instance_t *pdInstance)
{
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
    PD_PsmSetAutoPolicyState(pdInstance, PSM_RDY_EVAL_INIT);
#endif
    pdInstance->psmHardResetCount = 0;
    (void)PD_TimerClear(pdInstance, tNoResponseTimer);

#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
    if (PD_POLICY_GET_AUTO_SINK_NEGOTIATION_SUPPORT(pdInstance))
    {
        PD_AutoSinkNegotiation(pdInstance, &pdInstance->rdoRequest);
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY_LOG) && (PD_CONFIG_ENABLE_AUTO_POLICY_LOG)
        (void)PRINTF("sink auto do the request negotiation\r\n");
#endif
    }
    else
#endif
    {
        (void)PD_DpmAppCallback(pdInstance, PD_DPM_SNK_GET_RDO, &pdInstance->rdoRequest, 0);
    }

    if (pdInstance->revision < PD_SPEC_REVISION_30)
    {
        pdInstance->rdoRequest.bitFields.unchunkedSupported = 0;
    }
    if (pdInstance->commandProcessing == PD_DPM_INVALID)
    {
        (void)PD_PsmStartCommand(pdInstance, PD_DPM_CONTROL_REQUEST, 0);
    }
    PD_PsmCheckChunkedFeature(pdInstance, pdInstance->rdoRequest);
    pdInstance->psmNewState = PSM_PE_SNK_SELECT_CAPABILITY;
}

static inline void PD_PsmEnterSnkReadyState(pd_instance_t *pdInstance)
{
    PD_PsmResetStateWhenEnterReadyState(pdInstance);

#if (defined PD_CONFIG_PD3_PPS_ENABLE) && (PD_CONFIG_PD3_PPS_ENABLE)
    /* SinkPPSPeriodicTimer */
    if (PD_PsmSinkIsPPSRDO(pdInstance) != 0U)
    {
        (void)PD_TimerStart(pdInstance, tSinkPPSPeriodicTimer, T_PPS_REQUEST);
    }
#endif
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
    PD_PsmSetAutoPolicyState(pdInstance, PSM_RDY_EVAL_INIT);
#endif
}

static inline void PD_PsmEnterSnkTransitionToDefaultState(pd_instance_t *pdInstance)
{
    /* Message reception should not be re-enabled until PE_SNK_Startup */
    PD_MsgReset(pdInstance);
    pdInstance->psmExplicitContractExisted = 0u;
    pdInstance->psmPresentlyPdConnected    = 0;
    pdInstance->curDataRole                = kPD_DataRoleUFP;
    pdInstance->psmPresentlyVconnSource    = kPD_NotVconnSource;
    (void)PD_PhyControl(pdInstance, PD_PHY_SET_MSG_HEADER_INFO, NULL);
#if defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
    /* Request DPM to turn off vconn */
    PD_DpmSetVconn(pdInstance, 0);
#endif

    (void)PD_TimerStart(pdInstance, tNoResponseTimer, T_NO_RESPONSE);
    if (PD_PsmCheckVsafe5V(pdInstance) != 0U)
    {
        pdInstance->psmHardResetNeedsVSafe0V = 1;
        (void)PD_TimerStart(pdInstance, tPSHardResetTimer, T_PS_HARD_RESET + T_SAFE0V_MAX);
    }
    pdInstance->psmNewState = PSM_PE_SNK_STARTUP;
}

#endif

static pd_state_machine_state_t PD_PsmEnterState(pd_instance_t *pdInstance)
{
    pd_psm_state_t prevState;
    uint8_t didNothingC                  = 1;
    pd_state_machine_state_t returnState = kSM_None;

    while (pdInstance->psmNewState != pdInstance->psmCurState)
    {
        if (pdInstance->psmNewState == PE_PSM_STATE_ROLE_RDY_STATE)
        {
            if (pdInstance->curPowerRole == kPD_PowerRoleSource)
            {
                pdInstance->psmNewState = PSM_PE_SRC_READY;
            }
            else
            {
                pdInstance->psmNewState = PSM_PE_SNK_READY;
            }
        }

        didNothingC             = 0;
        prevState               = pdInstance->psmCurState;
        pdInstance->psmCurState = pdInstance->psmNewState;

        switch (pdInstance->psmCurState)
        {
            case PSM_EXIT_TO_ERROR_RECOVERY:            /* (C) */
            case PSM_EXIT_TO_ERROR_RECOVERY_WITH_DELAY: /* (C) */
                if (pdInstance->psmCurState == PSM_EXIT_TO_ERROR_RECOVERY_WITH_DELAY)
                {
                    /* Delay for 1ms to make sure any previous GoodCRC transmission has ended. */
                    (void)PD_PsmTimerWait(pdInstance, tDelayTimer, mSec(1), 0U);
                }
                returnState = kSM_ErrorRecovery;
                break;

            case PSM_UNKNOWN: /* (C) */
            case PSM_IDLE:    /* (C) */
                pdInstance->psmNewState = PSM_IDLE;
                pdInstance->psmCurState = PSM_IDLE;
                PD_PsmEnterIdleState(pdInstance);
                break;

            case PSM_INTERRUPTED_REQUEST: /* (C) */
                break;

            case PSM_HARD_RESET: /* (C) */
                PD_PsmEnterHardResetState(pdInstance);
                break;

            case PSM_SEND_SOFT_RESET: /* (C) */
                PD_PsmEnterSendSoftResetState(pdInstance);
                break;

            case PSM_SOFT_RESET: /* (C) */
#if ((defined PD_CONFIG_EXTENDED_CHUNK_LAYER_SUPPORT) && (PD_CONFIG_EXTENDED_CHUNK_LAYER_SUPPORT))
                PD_MsgChunkingLayerResetFlags(pdInstance);
#endif
                /* Alert the DPM so it can reset it's state */
                if (PD_PsmSendControlTransitionWithHardReset(
                        pdInstance, kPD_MsgAccept,
                        (pd_psm_state_t)((pdInstance->curPowerRole == kPD_PowerRoleSource) ?
                                             PSM_PE_SRC_IMPLICIT_CABLE_DISCOVERY :
                                             PSM_PE_SNK_WAIT_FOR_CAPABILITIES)) != 0U)
                {
                    (void)PD_DpmAppCallback(pdInstance, PD_DPM_SOFT_RESET_REQUEST, NULL, 1);
                }
                break;

#if (defined PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
            case PSM_PE_SRC_STARTUP: /* (C) */
                PD_PsmEnterSrcStartUpState(pdInstance);
                break;

            case PSM_PE_SRC_DISCOVERY: /* (C) */
#if (defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)) && \
    (defined(PD_CONFIG_SRC_AUTO_DISCOVER_CABLE_PLUG) && (PD_CONFIG_SRC_AUTO_DISCOVER_CABLE_PLUG))
                /* cable plug didn't response data object on PE_SRC_STARTUP state */
                if ((pdInstance->psmSecondaryState[1] == PSM_IDLE) &&
                    PD_CheckWhetherInitiateCableDiscoveryIdentityOrNot(pdInstance) != 0U)
                {
                    pdInstance->pendingSOP =
                        (uint8_t)kPD_MsgSOPMask | (uint8_t)kPD_MsgSOPpMask | (uint8_t)kPD_MsgSOPppMask;
                    pdInstance->psmNewSecondaryState[1] = PSM_PE_SRC_VDM_IDENTITY_REQUEST;
                }
#endif
                pdInstance->psmPresentlyPdConnected = 0;
                /* Do not clear previouslyPdConnected here */
                (void)PD_TimerStart(pdInstance, tSourceCapabilityTimer, T_SEND_SOURCE_CAP);
                break;

            case PSM_PE_SRC_IMPLICIT_CABLE_DISCOVERY: /* (C) */
                PD_PsmEnterImplicitCableDiscoveryState(pdInstance);
                break;

            case PSM_PE_SRC_SEND_CAPABILITIES: /* (C) */
                PD_PsmEnterSrcSendCapsState(pdInstance);
                break;

            case PSM_PE_SRC_NEGOTIATE_CAPABILITY: /* (C) */
                PD_PsmEnterSrcNegotiateCapState(pdInstance);
                break;

            case PSM_PE_SRC_TRANSITION_SUPPLY: /* (C) */
                PD_PsmEnterSrcTransitionSupplyState(pdInstance);
                break;

            case PSM_PE_SRC_READY: /* (C) */
                PD_PsmResetStateWhenEnterReadyState(pdInstance);
#if (defined PD_CONFIG_PD3_PPS_ENABLE) && (PD_CONFIG_PD3_PPS_ENABLE)
                /* SourcePPSCommTimer */
                if (PD_PsmSourceIsPPSRDO(pdInstance) != 0U)
                {
                    (void)PD_TimerStart(pdInstance, tSourcePPSCommTimer, T_PPS_TIMEOUT);
                }
#endif
                break;

            case PSM_PE_SRC_WAIT_NEW_CAPABILITIES: /* (C) */
                break;

            case PSM_PE_SRC_DISABLED: /* (C) */
                (void)PD_PhyControl(pdInstance, PD_PHY_RESET_MSG_FUNCTION, NULL);
                (void)PD_DpmAppCallback(pdInstance, PD_FUNCTION_DISABLED, NULL, 0);
                break;

            case PSM_PE_SRC_CAPABILITY_RESPONSE: /* (C) */
                PD_PsmEnterSrcCapResponseState(pdInstance);
                break;

            case PSM_PE_SRC_HARD_RESET_RECEIVED: /* (C) */
                break;

            case PSM_PE_SRC_TRANSITION_TO_DEFAULT: /* (C) */
                PD_PsmEnterSrcTransitionToDefaultState(pdInstance);
                break;

            case PSM_PE_SRC_GIVE_SOURCE_CAP: /* (C) */
#if defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
            case PSM_PE_DR_SNK_GIVE_SOURCE_CAP: /* (C) */
#endif
                PD_PsmEnterGiveSrcCapState(pdInstance);
                break;
#endif

#if (defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
            case PSM_PE_SNK_STARTUP: /* (C) */
                PD_PsmEnterSnkStartUpState(pdInstance);
                break;

            case PSM_PE_SNK_DISCOVERY: /* (C) */
                break;

            case PSM_PE_SNK_WAIT_FOR_CAPABILITIES: /* (C) */
                (void)PD_PhyControl(pdInstance, PD_PHY_SET_MSG_HEADER_INFO, NULL);
                pdInstance->pendingSOP = (uint8_t)kPD_MsgSOPMask;
                PD_MsgStartReceive(pdInstance);
                (void)PD_TimerStart(pdInstance, tSinkWaitCapTimer, T_SINK_WAIT_CAP);
                break;

            case PSM_PE_SNK_EVALUATE_CAPABILITY: /* (C) */
                PD_PsmEnterSnkEvaluateCapState(pdInstance);
                break;

            case PSM_PE_SNK_SELECT_CAPABILITY: /* (C) */
                if (PD_MsgSendDataTransition(
                        pdInstance, kPD_MsgRequest, 1, (void *)(&pdInstance->rdoRequest), PE_PSM_STATE_NO_CHANGE,
                        (prevState == PSM_PE_SNK_READY ? prevState : PSM_CHECK_ASYNC_RX), PSM_SEND_SOFT_RESET) != 0U)
                {
                    (void)PD_TimerStart(pdInstance, tSenderResponseTimer, T_SENDER_RESPONSE);
                }
                break;

            case PSM_PE_SNK_SELECT_CAPABILITY_WAIT_TIMER_TIME_OUT: /* (C) */
                break;

            case PSM_PE_SNK_TRANSITION_SINK: /* (C) */
                (void)PD_TimerStart(pdInstance, tPSTransitionTimer, T_PS_TRANSITION);
                if (pdInstance->psmGotoMinRx != 0U)
                {
                    (void)pdInstance->callbackFns->PD_SnkGotoMinReducePower(pdInstance->callbackParam);
                }
                else
                {
                    /* Start ignoring droops on VBus */
                    /* Source starts to change its power capability tSrcTransition after the GoodCRC Message was
                       received, so it is safe to change the power progress here. */
                    PD_ConnectSetPowerProgress(pdInstance, kVbusPower_ChangeInProgress);
                    /* Reduce power comsumption to pSnkStdby. */
                    (void)pdInstance->callbackFns->PD_SnkDrawTypeCVbus(pdInstance->callbackParam, 0,
                                                                       kVbusPower_ChangeInProgress);
                }
                break;

            case PSM_PE_SNK_READY: /* (C) */
                PD_PsmEnterSnkReadyState(pdInstance);
                break;

            case PSM_PE_SNK_TRANSITION_TO_DEFAULT: /* (C) */
                PD_PsmEnterSnkTransitionToDefaultState(pdInstance);
                break;
#endif

#if (defined PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
            case PSM_PE_SRC_GET_SINK_CAP: /* (C) */
#endif
#if defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
            case PSM_PE_DR_SNK_GET_SINK_CAP:   /* (C) */
            case PSM_PE_DR_SRC_GET_SOURCE_CAP: /* (C) */
#endif
#if (defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
            case PSM_PE_SNK_GET_SOURCE_CAP: /* (C) */
#endif
                PD_PsmEnterGetSinkOrSourceCapState(pdInstance, prevState);
                break;

#if (defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)) || \
    ((defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE))
            case PSM_PE_DR_SRC_GIVE_SINK_CAP: /* (C) */
            case PSM_PE_SNK_GIVE_SINK_CAP:    /* (C) */
                PD_PsmEnterGiveSinkCapState(pdInstance);
                break;
#endif

#if defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
            case PSM_PE_PRS_SRC_SNK_EVALUATE_PR_SWAP: /* (C) */
            {
                pd_dpm_callback_event_t requestEvent = PD_DPM_PR_SWAP_REQUEST;

#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
                PD_PsmSetAutoPolicyState(pdInstance, PSM_RDY_EVAL_INIT);
                if (PD_POLICY_SUPPORT(pdInstance) && (PD_POLICY_GET_AUTO_ACCEPT_PRSWAP_AS_SOURCE_SUPPORT(pdInstance)))
                {
                    if (pdInstance->swapToSnkSrcCapReceived != 0U)
                    {
                        pd_source_pdo_t pdo;
                        pdInstance->swapToSnkSrcCapReceived = 0;
                        pdo.PDOValue                        = pdInstance->pdPowerPortConfig->sourceCaps[0];
                        if ((pdo.fixedPDO.externalPowered != 0U) &&
                            (pdInstance->partnerSourcePDOs[0].fixedPDO.externalPowered == 0U))
                        {
                            requestEvent                      = PD_DPM_EVENT_INVALID;
                            pdInstance->commandEvaluateResult = kCommandResult_Reject;
                        }
                    }

                    if (requestEvent != PD_DPM_EVENT_INVALID)
                    {
                        pdInstance->commandEvaluateResult =
                            (pd_command_result_t)PD_POLICY_GET_AUTO_ACCEPT_PRSWAP_AS_SOURCE(pdInstance);
                        requestEvent = PD_DPM_EVENT_INVALID;
                    }
                }
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY_LOG) && (PD_CONFIG_ENABLE_AUTO_POLICY_LOG)
                if (requestEvent == PD_DPM_EVENT_INVALID)
                {
                    PD_PsmPrintAutoPolicyReplySwapRequestLog(pdInstance, (uint8_t)kPD_MsgPrSwap);
                }
#endif
#endif
                PD_PsmEvaluateSwap(pdInstance, requestEvent, PSM_PE_PRS_SRC_SNK_ACCEPT_PR_SWAP,
                                   PSM_PE_PRS_SRC_SNK_REJECT_PR_SWAP);
                break;
            }

            case PSM_PE_PRS_SNK_SRC_EVALUATE_PR_SWAP: /* (C) */
            {
                pd_dpm_callback_event_t requestEvent = PD_DPM_PR_SWAP_REQUEST;

#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
                PD_PsmSetAutoPolicyState(pdInstance, PSM_RDY_EVAL_INIT);
                if (PD_POLICY_SUPPORT(pdInstance) && (PD_POLICY_GET_AUTO_ACCEPT_PRSWAP_AS_SINK_SUPPORT(pdInstance)))
                {
                    requestEvent = PD_DPM_EVENT_INVALID;
                    if ((PD_POLICY_GET_AUTO_ACCEPT_PRSWAP_AS_SINK(pdInstance) == (uint8_t)kAutoRequestProcess_Accept) &&
                        ((PD_PsmGetExternalPowerState(pdInstance) != 0U) ||
                         (pdInstance->selfOrPartnerFirstSourcePDO.fixedPDO.externalPowered == 0U)))
                    {
                        pdInstance->commandEvaluateResult = kCommandResult_Accept;
                    }
                    else
                    {
                        pdInstance->commandEvaluateResult = kCommandResult_Reject;
                    }
                }
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY_LOG) && (PD_CONFIG_ENABLE_AUTO_POLICY_LOG)
                if (requestEvent == PD_DPM_EVENT_INVALID)
                {
                    PD_PsmPrintAutoPolicyReplySwapRequestLog(pdInstance, (uint8_t)kPD_MsgPrSwap);
                }
#endif
#endif
                PD_PsmEvaluateSwap(pdInstance, requestEvent, PSM_PE_PRS_SNK_SRC_ACCEPT_PR_SWAP,
                                   PSM_PE_PRS_SNK_SRC_REJECT_PR_SWAP);
                break;
            }

#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
            case PE_FRS_SRC_SNK_Evaluate_Swap: /* C */
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
                PD_PsmSetAutoPolicyState(pdInstance, PSM_RDY_EVAL_INIT);
#endif
                PD_PsmEvaluateSwap(pdInstance, PD_DPM_FR_SWAP_REQUEST, PE_FRS_SRC_SNK_Accept_Swap, PSM_HARD_RESET);
                break;
#endif

            case PSM_PE_PRS_SRC_SNK_ACCEPT_PR_SWAP: /* (C) */
                (void)PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(pdInstance, kPD_MsgAccept,
                                                                         PSM_PE_PRS_SRC_SNK_TRANSITION_TO_OFF);
                break;
            case PSM_PE_PRS_SNK_SRC_ACCEPT_PR_SWAP: /* (C) */
                (void)PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(pdInstance, kPD_MsgAccept,
                                                                         PSM_PE_PRS_SNK_SRC_TRANSITION_TO_OFF);
                break;
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
            case PE_FRS_SRC_SNK_Accept_Swap: /* C */
                (void)PD_PsmSendControlTransitionWithTry(pdInstance, kPD_MsgAccept, PE_FRS_SRC_SNK_Transition_to_off,
                                                         PSM_HARD_RESET, PSM_HARD_RESET);
                break;
#endif

            case PSM_PE_PRS_SRC_SNK_TRANSITION_TO_OFF: /* (C) */
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
                if (pdInstance->frsEnabled != 0U)
                {
                    PD_FRSControl(pdInstance, 0);
                    pdInstance->frsEnabled = 0;
                }
#endif
                PD_ConnectSetPowerProgress(pdInstance, kVbusPower_InPRSwap);

                /* pr swap transition to standby */
                /* 1. tSrcTransition */
                (void)PD_PsmTimerWait(pdInstance, tDelayTimer, T_SRC_TRANSITION, PD_TASK_EVENT_RECEIVED_HARD_RESET);
                /* 2. start enter to standby tSrcSwapStdby */
                (void)pdInstance->callbackFns->PD_SrcTurnOffVbus(pdInstance->callbackParam, kVbusPower_InPRSwap);
                PD_DpmDischargeVbus(pdInstance, 1);
                break;
            case PSM_PE_PRS_SNK_SRC_TRANSITION_TO_OFF: /* (C) */
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
                if (pdInstance->frsEnabled != 0U)
                {
                    PD_FRSControl(pdInstance, 0);
                    pdInstance->frsEnabled = 0;
                }
#endif
                pdInstance->psmExplicitContractExisted = 0;
                (void)PD_TimerStart(pdInstance, tPSSourceOffTimer, T_PS_SOURCE_OFF);
                PD_ConnectSetPowerProgress(pdInstance, kVbusPower_InPRSwap);
                /* sink transition to standby. */
                (void)pdInstance->callbackFns->PD_SnkStopDrawVbus(pdInstance->callbackParam, kVbusPower_InPRSwap);
                break;
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
            case PE_FRS_SNK_SRC_Transition_to_off: /* C */
                /* uint8_t enable = 0; */
                /* (void)PD_PhyControl(pdInstance, PD_PHY_CONTROL_FR_SWAP, &enable); */
                (void)PD_TimerStart(pdInstance, tPSSourceOffTimer, T_PS_SOURCE_OFF);
                break;
            case PE_FRS_SRC_SNK_Transition_to_off: /* C */
                PD_FRSControl(pdInstance, 0);
                pdInstance->frsEnabled = 0;
                (void)PD_TimerStart(pdInstance, timrFRSwapWaitPowerStable, T_FRSWAP_WAIT_POWER_STABLE);
                break;
#endif

            case PSM_PE_PRS_SRC_SNK_ASSERT_RD: /* (C) */
                PD_PsmPowerSwapAssertRdRp(pdInstance, kPD_PowerRoleSink, PSM_PE_PRS_SRC_SNK_WAIT_SOURCE_ON);
                break;
            case PSM_PE_PRS_SNK_SRC_ASSERT_RP: /* (C) */
                PD_PsmPowerSwapAssertRdRp(pdInstance, kPD_PowerRoleSource, PSM_PE_PRS_SNK_SRC_SOURCE_ON);
                break;
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
            case PE_FRS_SRC_SNK_Assert_Rd: /* C */
                PD_PsmPowerSwapAssertRdRp(pdInstance, kPD_PowerRoleSink, PE_FRS_SRC_SNK_Wait_Source_on);
                break;
            case PE_FRS_SNK_SRC_Assert_Rp: /* C */
                PD_PsmPowerSwapAssertRdRp(pdInstance, kPD_PowerRoleSource, PE_FRS_SNK_SRC_Source_on);
                break;
#endif

            case PSM_PE_PRS_SRC_SNK_WAIT_SOURCE_ON: /* (C) */
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
            case PE_FRS_SRC_SNK_Wait_Source_on: /* C */
#endif
                /* 0 Role stays as standby until we receive the PS_RDY message */
                (void)PD_PhyControl(pdInstance, PD_PHY_SET_MSG_HEADER_INFO, NULL);
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
                if (PE_FRS_SRC_SNK_Wait_Source_on == pdInstance->psmCurState)
                {
                    if (PD_PsmSendControlTransitionWithTry(pdInstance, kPD_MsgPsRdy, PE_FRS_SRC_SNK_Wait_Source_on,
                                                           PSM_EXIT_TO_ERROR_RECOVERY_WITH_DELAY,
                                                           PSM_EXIT_TO_ERROR_RECOVERY_WITH_DELAY) != 0U)
                    {
                        (void)PD_TimerStart(pdInstance, tPSSourceOnTimer, T_PS_SOURCE_ON);
                    }
                }
                else
#endif
                {
                    if (PD_PsmSendControlTransitionWithErrorRecovery(pdInstance, kPD_MsgPsRdy,
                                                                     PE_PSM_STATE_NO_CHANGE) != 0U)
                    {
                        (void)PD_TimerStart(pdInstance, tPSSourceOnTimer, T_PS_SOURCE_ON);
                    }
                }
                break;
            case PSM_PE_PRS_SNK_SRC_SOURCE_ON: /* (C) */
                (void)pdInstance->callbackFns->PD_SrcTurnOnTypeCVbus(pdInstance->callbackParam, kVbusPower_InPRSwap);
                break;
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
            case PE_FRS_SNK_SRC_Source_on: /* C */
                PD_PsmPowerSwapSinkOpenVbus(pdInstance, PD_DPM_FR_SWAP_SUCCESS);
                break;
#endif

            case PSM_PE_PRS_SRC_SNK_SEND_PR_SWAP: /* (C) */
            case PSM_PE_PRS_SNK_SRC_SEND_PR_SWAP: /* (C) */
                (void)PD_PsmSendControlTransitionWithSendResponserTimeOut(
                    pdInstance, kPD_MsgPrSwap, PE_PSM_STATE_NO_CHANGE, prevState, PSM_SEND_SOFT_RESET);
                break;
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
            case PE_FRS_SNK_SRC_Send_Swap: /* C */
                pdInstance->fr5VOpened = 0;
                PD_FRSControl(pdInstance, 0);
                pdInstance->frsEnabled = 0;
                PD_PsmCheckFRS5V(pdInstance);
                if (PD_PsmSendControlTransitionWithSendResponserTimeOut(
                        pdInstance, kPD_MsgFrSwap, PE_PSM_STATE_NO_CHANGE, PSM_EXIT_TO_ERROR_RECOVERY_WITH_DELAY,
                        PSM_EXIT_TO_ERROR_RECOVERY_WITH_DELAY) != 0U)
                {
                    (void)PD_PsmStartCommand(pdInstance, PD_DPM_FAST_ROLE_SWAP, 0);
                }
                else
                {
                    pdInstance->psmNewState = PSM_EXIT_TO_ERROR_RECOVERY_WITH_DELAY;
                }
                break;
#endif

            case PSM_PE_PRS_SRC_SNK_REJECT_PR_SWAP: /* (C) */
            case PSM_PE_PRS_SNK_SRC_REJECT_PR_SWAP: /* (C) */
            {
                pd_command_result_t callbackResult = kCommandResult_Reject;

                (void)PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(
                    pdInstance,
                    ((pdInstance->commandEvaluateResult == kCommandResult_Reject) ? kPD_MsgReject : kPD_MsgWait),
                    PE_PSM_STATE_ROLE_RDY_STATE);

                (void)PD_DpmAppCallback(pdInstance, PD_DPM_PR_SWAP_FAIL, &callbackResult, 1);
                break;
            }
#endif

#if defined(PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
            case PSM_PE_DRS_EVALUATE_DR_SWAP: /* (C) */
            {
                pd_dpm_callback_event_t requestEvent = PD_DPM_DR_SWAP_REQUEST;

                pdInstance->psmDrSwapPrevState = prevState;
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
                PD_PsmSetAutoPolicyState(pdInstance, PSM_RDY_EVAL_INIT);
                if (PD_POLICY_SUPPORT(pdInstance) && (((pdInstance->curDataRole == kPD_DataRoleUFP) &&
                                                       PD_POLICY_GET_AUTO_ACCEPT_DRSWAP_AS_DFP_SUPPORT(pdInstance)) ||
                                                      ((pdInstance->curDataRole == kPD_DataRoleDFP) &&
                                                       PD_POLICY_GET_AUTO_ACCEPT_DRSWAP_AS_UFP_SUPPORT(pdInstance))))
                {
                    if (pdInstance->curDataRole == kPD_DataRoleUFP)
                    {
                        pdInstance->commandEvaluateResult =
                            (pd_command_result_t)PD_POLICY_GET_AUTO_ACCEPT_DRSWAP_AS_DFP(pdInstance);
                    }
                    else
                    {
                        pdInstance->commandEvaluateResult =
                            (pd_command_result_t)PD_POLICY_GET_AUTO_ACCEPT_DRSWAP_AS_UFP(pdInstance);
                    }
                    requestEvent = PD_DPM_EVENT_INVALID;
                }
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY_LOG) && (PD_CONFIG_ENABLE_AUTO_POLICY_LOG)
                if (requestEvent == PD_DPM_EVENT_INVALID)
                {
                    PD_PsmPrintAutoPolicyReplySwapRequestLog(pdInstance, (uint8_t)kPD_MsgDrSwap);
                }
#endif
#endif
                PD_PsmEvaluateSwap(pdInstance, requestEvent, PSM_PE_DRS_ACCEPT_DR_SWAP, PSM_PE_DRS_REJECT_DR_SWAP);
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
                pdInstance->drSwapResult = pdInstance->commandEvaluateResult;
#endif
                break;
            }

            case PSM_PE_DRS_EVALUATE_DR_SWAP_WAIT_TIMER_TIME_OUT: /* (C) */
            case PSM_PE_PRS_EVALUATE_PR_SWAP_WAIT_TIMER_TIME_OUT: /* (C) */
                break;

            case PSM_PE_DRS_REJECT_DR_SWAP: /* (C) */
            {
                (void)PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(
                    pdInstance,
                    ((pdInstance->commandEvaluateResult == kCommandResult_Reject) ? kPD_MsgReject : kPD_MsgWait),
                    (pd_psm_state_t)pdInstance->psmDrSwapPrevState);
                if (pdInstance->commandEvaluateResult != kCommandResult_Reject)
                {
                    pdInstance->commandEvaluateResult = kCommandResult_Wait;
                }
                (void)PD_DpmAppCallback(pdInstance, PD_DPM_DR_SWAP_FAIL, &pdInstance->commandEvaluateResult, 1);
                break;
            }

            case PSM_PE_DRS_SEND_DR_SWAP:                   /* (C) */
                pdInstance->psmDrSwapPrevState = prevState; /* snk_rdy or src_rdy */
                (void)PD_PsmSendControlTransitionWithSendResponserTimeOut(
                    pdInstance, kPD_MsgDrSwap, PE_PSM_STATE_NO_CHANGE, prevState, PSM_SEND_SOFT_RESET);
                break;

            case PSM_PE_DRS_ACCEPT_DR_SWAP: /* (C) */
            {
                (void)PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(pdInstance, kPD_MsgAccept,
                                                                         PSM_PE_DRS_CHANGE_TO_DFP_OR_UFP);
                if (pdInstance->psmNewState == PSM_PE_DRS_CHANGE_TO_DFP_OR_UFP)
                {
                    pdInstance->curDataRole =
                        (pdInstance->curDataRole == kPD_DataRoleUFP) ? kPD_DataRoleDFP : kPD_DataRoleUFP;
                    (void)PD_PhyControl(pdInstance, PD_PHY_SET_MSG_HEADER_INFO, NULL);
                }
                break;
            }

            case PSM_PE_DRS_CHANGE_TO_DFP_OR_UFP: /* (C) */
            {
                /* Exit any active alternate mode */

                pdInstance->pendingSOP = (uint8_t)kPD_MsgSOPMask;
                if (pdInstance->curDataRole == kPD_DataRoleDFP)
                {
                    (void)PD_DpmAppCallback(pdInstance, PD_DPM_DR_SWAP_SUCCESS, NULL, 1);
#if 0 /* even do the cable discovery identity, but cannot do SOP'/SOP'' communication after negotiation done */
#if (defined PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
                    pdInstance->pendingSOP = (uint8_t)kPD_MsgSOPMask | (uint8_t)kPD_MsgSOPpMask | (uint8_t)kPD_MsgSOPppMask;
#endif
#endif
                }
                else
                {
                    (void)PD_DpmAppCallback(pdInstance, PD_DPM_DR_SWAP_SUCCESS, NULL, 1);
                }

                /* Raise the interrupt */
                if ((pdInstance->curDataRole == kPD_DataRoleDFP) || (pdInstance->curPowerRole == kPD_PowerRoleSource))
                {
#if defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
                    /* Cable plug will need a soft reset */
                    pdInstance->psmCablePlugResetNeeded = 1;
#endif
                }
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
                pdInstance->drSwapResult = kCommandResult_Accept;
                PD_PsmSetAutoPolicyState(pdInstance, PSM_RDY_EVAL_INIT);
#endif
                /* return to previous state */
                pdInstance->psmNewState = pdInstance->psmDrSwapPrevState;
                break;
            }
#endif

#if defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
            case PSM_PE_VCS_SEND_SWAP: /* (C) */
                pdInstance->psmVconnSwapPrevState = prevState;
                (void)PD_PsmSendControlTransitionWithSendResponserTimeOut(
                    pdInstance, kPD_MsgVconnSwap, PE_PSM_STATE_NO_CHANGE, pdInstance->psmVconnSwapPrevState,
                    PSM_SEND_SOFT_RESET);
                break;

            case PSM_PE_VCS_WAIT_FOR_VCONN: /* (C) */
                (void)PD_TimerStart(pdInstance, tVconnOnTimer, T_VCONN_SOURCE_ON);
                break;

            case PSM_PE_VCS_TURN_OFF_VCONN: /* (C) */
                pdInstance->psmPresentlyVconnSource = kPD_NotVconnSource;
#if (defined PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
                pdInstance->cableDiscoverIdentityCounter = 0;
#endif
                /* Inform the DPM */
                PD_DpmSetVconn(pdInstance, 0);
                (void)PD_DpmAppCallback(pdInstance, PD_DPM_VCONN_SWAP_SUCCESS, NULL, 1);
                /* return to previous state */
                pdInstance->psmNewState = pdInstance->psmVconnSwapPrevState;
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
                pdInstance->vconnSwapResult = kCommandResult_Accept;
                PD_PsmSetAutoPolicyState(pdInstance, PSM_RDY_EVAL_INIT);
#endif
                break;

            case PSM_PE_VCS_TURN_ON_VCONN: /* (C) */
                pdInstance->psmPresentlyVconnSource = kPD_IsVconnSource;
                /* Inform the DPM */
                PD_DpmSetVconn(pdInstance, 1);
                (void)PD_TimerStart(pdInstance, tVconnOnTimer, T_VCONN_SOURCE_ON / 2);
                break;

            case PSM_PE_VCS_SEND_PS_RDY: /* (C) */
                if (PD_MsgSendControlTransition(pdInstance, kPD_MsgPsRdy, pdInstance->psmVconnSwapPrevState,
                                                PSM_SEND_SOFT_RESET, PSM_SEND_SOFT_RESET) != 0U)
                {
#if (defined PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
                    pdInstance->cableDiscoverIdentityCounter = 0;
#endif
                    (void)PD_DpmAppCallback(pdInstance, PD_DPM_VCONN_SWAP_SUCCESS, NULL, 1);
                }
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
                pdInstance->vconnSwapResult = kCommandResult_Accept;
                PD_PsmSetAutoPolicyState(pdInstance, PSM_RDY_EVAL_INIT);
#endif
                break;

            case PSM_PE_VCS_EVALUATE_SWAP: /* (C) */
            {
                pd_dpm_callback_event_t requestEvent = PD_DPM_VCONN_SWAP_REQUEST;

                pdInstance->psmVconnSwapPrevState = prevState;
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
                PD_PsmSetAutoPolicyState(pdInstance, PSM_RDY_EVAL_INIT);
                if (PD_POLICY_SUPPORT(pdInstance) &&
                    (((pdInstance->psmPresentlyVconnSource == kPD_NotVconnSource) &&
                      PD_POLICY_GET_AUTO_ACCEPT_VCONNSWAP_TURN_ON_SUPPORT(pdInstance)) ||
                     ((pdInstance->psmPresentlyVconnSource == kPD_IsVconnSource) &&
                      PD_POLICY_GET_AUTO_ACCEPT_VCONNSWAP_TURN_OFF_SUPPORT(pdInstance))))
                {
                    if (pdInstance->psmPresentlyVconnSource == kPD_NotVconnSource)
                    {
                        pdInstance->commandEvaluateResult =
                            (pd_command_result_t)PD_POLICY_GET_AUTO_ACCEPT_VCONNSWAP_TURN_ON(pdInstance);
                    }
                    else
                    {
                        pdInstance->commandEvaluateResult =
                            (pd_command_result_t)PD_POLICY_GET_AUTO_ACCEPT_VCONNSWAP_TURN_OFF(pdInstance);
                    }
                    requestEvent = PD_DPM_EVENT_INVALID;
                }
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY_LOG) && (PD_CONFIG_ENABLE_AUTO_POLICY_LOG)
                if (requestEvent == PD_DPM_EVENT_INVALID)
                {
                    PD_PsmPrintAutoPolicyReplySwapRequestLog(pdInstance, (uint8_t)kPD_MsgVconnSwap);
                }
#endif
#endif /* PD_CONFIG_ENABLE_AUTO_POLICY */
                PD_PsmEvaluateSwap(pdInstance, requestEvent, PSM_PE_VCS_ACCEPT_SWAP, PSM_PE_VCS_REJECT_SWAP);
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
                pdInstance->vconnSwapResult = pdInstance->commandEvaluateResult;
#endif
                break;
            }

            case PSM_PE_VCS_ACCEPT_SWAP: /* (C) */
                (void)PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(
                    pdInstance, kPD_MsgAccept,
                    (pdInstance->psmPresentlyVconnSource == kPD_IsVconnSource) ? PSM_PE_VCS_WAIT_FOR_VCONN :
                                                                                 PSM_PE_VCS_TURN_ON_VCONN);

                break;

            case PSM_PE_VCS_REJECT_SWAP: /* (C) */
                (void)PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(
                    pdInstance,
                    (pdInstance->commandEvaluateResult == kCommandResult_Reject) ? kPD_MsgReject : kPD_MsgWait,
                    (pd_psm_state_t)pdInstance->psmVconnSwapPrevState);
                if (pdInstance->commandEvaluateResult != kCommandResult_Reject)
                {
                    pdInstance->commandEvaluateResult = kCommandResult_Wait;
                }
                (void)PD_DpmAppCallback(pdInstance, PD_DPM_VCONN_SWAP_FAIL, &pdInstance->commandEvaluateResult, 1);
                break;
#endif /* PD_CONFIG_VCONN_SUPPORT */

#if defined(PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
            case PE_SNK_GET_SOURCE_CAP_EXT:    /* C */
            case PE_DR_SRC_GET_SOURCE_CAP_EXT: /* C */
                (void)PD_PsmSendControlTransitionWithSendResponserTimeOut(
                    pdInstance, kPD_MsgGetSourceCapExtended, PE_PSM_STATE_NO_CHANGE, prevState, PSM_SEND_SOFT_RESET);
                break;
            case PE_SRC_Get_Sink_Status:   /* C */
            case PE_SNK_Get_Source_Status: /* C */
                (void)PD_PsmSendControlTransitionWithSendResponserTimeOut(
                    pdInstance, kPD_MsgGetStatus, PE_PSM_STATE_NO_CHANGE, prevState, PSM_SEND_SOFT_RESET);
                break;
#if defined(PD_CONFIG_PD3_PPS_ENABLE) && (PD_CONFIG_PD3_PPS_ENABLE)
            case PE_SNK_Get_PPS_Status: /* C */
                (void)PD_PsmSendControlTransitionWithSendResponserTimeOut(
                    pdInstance, kPD_MsgGetPpsStatus, PE_PSM_STATE_NO_CHANGE, prevState, PSM_SEND_SOFT_RESET);
                break;
#endif
            case PE_Get_Battery_Cap:    /* C */
            case PE_Get_Battery_Status: /* C */
                if (PD_MsgSendExtTransition(pdInstance, kPD_MsgSOP,
                                            (PE_Get_Battery_Cap == pdInstance->psmCurState) ? kPD_MsgGetBatteryCap :
                                                                                              kPD_MsgGetBatteryStatus,
                                            1, &pdInstance->getBatteryCapDataBlock, PE_PSM_STATE_NO_CHANGE, prevState,
                                            PSM_SEND_SOFT_RESET) != 0U)
                {
                    (void)PD_TimerStart(pdInstance, tSenderResponseTimer, T_SENDER_RESPONSE);
                }
                break;
            case PE_Get_Manufacturer_Info: /* C */
                if (PD_MsgSendExtTransition(pdInstance, (start_of_packet_t)pdInstance->commandExtParam.sop,
                                            kPD_MsgGetManufacturerInfo, 2, pdInstance->commandExtParam.dataBuffer,
                                            PE_PSM_STATE_NO_CHANGE, prevState, PSM_SEND_SOFT_RESET) != 0U)
                {
                    (void)PD_TimerStart(pdInstance, tSenderResponseTimer, T_SENDER_RESPONSE);
                }
                break;

            case PE_SRC_Send_Source_Alert: /* C */
            case PE_SNK_Send_Sink_Alert:   /* C */
                if (PD_MsgSendDataTransition(pdInstance, kPD_MsgAlert, 1, &pdInstance->alertADO,
                                             PE_PSM_STATE_ROLE_RDY_STATE, prevState, PSM_SEND_SOFT_RESET) != 0U)
                {
                    pdInstance->alertWaitReply = 1u;
                    (void)PD_DpmAppCallback(pdInstance, PD_DPM_SEND_ALERT_SUCCESS, NULL, 1);
                }
                else
                {
                    pd_command_result_t errorResult = kCommandResult_Error;
                    (void)PD_DpmAppCallback(pdInstance, PD_DPM_SEND_ALERT_FAIL, &errorResult, 1);
                }
                break;

            case PE_SRC_GIVE_SOURCE_CAP_EXT:    /* C */
            case PE_DR_SNK_GIVE_SOURCE_CAP_EXT: /* C */
                pdInstance->commandExtParamCallback.resultStatus = (uint8_t)kCommandResult_NotSupported;
                (void)PD_DpmAppCallback(pdInstance, PD_DPM_GIVE_SRC_EXT_CAP, &pdInstance->commandExtParamCallback, 0);
                PD_StateSendReplyExtDataTransition(pdInstance, kPD_MsgSourceCapExtended);
                break;
            case PE_SRC_Give_Source_Status: /* C */
            case PE_SNK_Give_Sink_Status:   /* C */
                pdInstance->commandExtParamCallback.resultStatus = (uint8_t)kCommandResult_NotSupported;
                (void)PD_DpmAppCallback(pdInstance, PD_DPM_GIVE_STATUS, &pdInstance->commandExtParamCallback, 0);
                PD_StateSendReplyExtDataTransition(pdInstance, kPD_MsgStatus);
                break;
#if (defined PD_CONFIG_PD3_PPS_ENABLE) && (PD_CONFIG_PD3_PPS_ENABLE)
            case PE_SRC_Give_PPS_Status: /* C */
                pdInstance->commandExtParamCallback.resultStatus = (uint8_t)kCommandResult_NotSupported;
                (void)PD_DpmAppCallback(pdInstance, PD_DPM_GIVE_PPS_STATUS, &pdInstance->commandExtParamCallback, 0);
                PD_StateSendReplyExtDataTransition(pdInstance, kPD_MsgPpsStatus);
                break;
#endif
            case PE_Give_Battery_Cap: /* C */
                pdInstance->commandExtParamCallback.resultStatus = (uint8_t)kCommandResult_NotSupported;
                (void)PD_DpmAppCallback(pdInstance, PD_DPM_GIVE_BATTERY_CAP, &pdInstance->commandExtParamCallback, 0);
                PD_StateSendReplyExtDataTransition(pdInstance, kPD_MsgBatteryCapabilities);
                break;
            case PE_Give_Battery_Status: /* C */
                pdInstance->commandExtParamCallback.resultStatus = (uint8_t)kCommandResult_NotSupported;
                (void)PD_DpmAppCallback(pdInstance, PD_DPM_GIVE_BATTERY_STATUS, &pdInstance->commandExtParamCallback,
                                        0);
                PD_StateSendReplyExtDataTransition(pdInstance, kPD_MsgBatteryStatus);
                break;
            case PE_Give_Manufacturer_Info: /* C */
                pdInstance->commandExtParamCallback.resultStatus = (uint8_t)kCommandResult_NotSupported;
                (void)PD_DpmAppCallback(pdInstance, PD_DPM_GIVE_MANUFACTURER_INFO, &pdInstance->commandExtParamCallback,
                                        0);
                PD_StateSendReplyExtDataTransition(pdInstance, kPD_MsgManufacturerInfo);
                break;

            case PE_SNK_Source_Alert_Received: /* C */
            case PE_SRC_Sink_Alert_Received:   /* C */
                pdInstance->psmNewState = PE_PSM_STATE_ROLE_RDY_STATE;
                break;

#if 0
            case PE_Send_Security_Request: /* C */
            {
                pd_status_t sendStatus = kStatus_PD_Error;

                /* even this msg is chunked, it only needs one chunk. */
                if (!pdInstance->unchunkedFeature != 0U)
                {
                    sendStatus = PD_PsmChunkingLayerTXStateMachine(pdInstance,
                                                                   kPD_MsgSOP, kPD_MsgSecurityRequest,
                                                                   pdInstance->commandExtParam.dataBuffer,
                                                                   pdInstance->commandExtParam.dataLength, NULL);
                    if (sendStatus != kStatus_PD_Success)
                    {
                        uint8_t inCase32Tmp = kCommandResult_Error;
                        (void)PD_DpmAppCallback(pdInstance, PD_DPM_SECURITY_REQUEST_FAIL, &inCase32Tmp, 1);
                        PD_PsmTransitionOnMsgSendError(pdInstance, prevState, PSM_SEND_SOFT_RESET);
                    }
                    else
                    {
                        pdInstance->psmNewState = PE_PSM_STATE_ROLE_RDY_STATE;
                    }
                }
                else
                {
                    if (PD_MsgSendExtTransition(pdInstance, (start_of_packet_t)pdInstance->commandExtParam.sop,
                                                kPD_MsgSecurityRequest, pdInstance->commandExtParam.dataLength,
                                                pdInstance->commandExtParam.dataBuffer, PE_PSM_STATE_ROLE_RDY_STATE,
                                                prevState, PSM_SEND_SOFT_RESET) == 0U)
                    {
                        uint8_t inCase32Tmp = kCommandResult_Error;
                        (void)PD_DpmAppCallback(pdInstance, PD_DPM_SECURITY_REQUEST_FAIL, &inCase32Tmp, 1);
                    }
                }
                break;
            }

            case PE_Send_Security_Response: /* C */
            {
                pd_status_t sendStatus = kStatus_PD_Error;

                /* first time */
                (void)PD_DpmAppCallback(pdInstance, PD_DPM_RESPONSE_SECURITY_REQUEST, &pdInstance->commandExtParamCallback,
                                  0);
                if ((pdInstance->commandExtParamCallback.resultStatus != (uint8_t)kCommandResult_NotSupported) &&
                    (pdInstance->unchunkedFeature == 0U))
                {
                    sendStatus = PD_PsmChunkingLayerTXStateMachine(pdInstance,
                                                                   kPD_MsgSOP, kPD_MsgSecurityResponse,
                                                                   pdInstance->commandExtParamCallback.dataBuffer,
                                                                   pdInstance->commandExtParamCallback.dataLength, NULL);
                    if (sendStatus != kStatus_PD_Success)
                    {
                        PD_PsmTransitionOnMsgSendError(pdInstance, PSM_CHECK_ASYNC_RX, PSM_SEND_SOFT_RESET);
                    }
                    else
                    {
                        if (PD_PsmChunkingLayerCheckSentDone(pdInstance))
                        {
                            pdInstance->psmNewState = PE_PSM_STATE_ROLE_RDY_STATE;
                        }
                        else
                        {
                            /* wait chunk message sent done */
                        }
                    }
                }
                else
                {
                    if ((pdInstance->commandExtParamCallback.resultStatus != (uint8_t)kCommandResult_NotSupported) &&
                        (pdInstance->unchunkedFeature != 0U))
                    {
                        PD_MsgSendExtTransition(pdInstance, (start_of_packet_t)pdInstance->commandExtParam.sop,
                                                kPD_MsgSecurityResponse, pdInstance->commandExtParamCallback.dataLength,
                                                pdInstance->commandExtParamCallback.dataBuffer, PE_PSM_STATE_ROLE_RDY_STATE,
                                                PSM_CHECK_ASYNC_RX, PSM_SEND_SOFT_RESET);
                    }
                    else if (pdInstance->commandExtParamCallback.resultStatus == (uint8_t)kCommandResult_NotSupported)
                    {
                        (void)PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(pdInstance, kPD_MsgNotSupported,
                                                           PE_PSM_STATE_ROLE_RDY_STATE);
                    }
                    else
                    {
                        /* No action required. */
                    }
                }
                break;
            }

            case PE_Security_Response_Received: /* C */
                (void)PD_DpmAppCallback(pdInstance, PD_DPM_SECURITY_RESPONSE_RECEIVED, &pdInstance->commandExtParamCallback, 0);
                pdInstance->psmNewState = PE_PSM_STATE_ROLE_RDY_STATE;
                break;
#endif
#endif /* PD_CONFIG_EXTENDED_MSG_SUPPORT */

#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
            case PE_FRS_SRC_SNK_CC_Signal: /* C */
            {
                PD_MsgReceive(pdInstance);
                (void)PD_TimerStart(pdInstance, tFRSwapSignalTimer, 500);
                break;
            }

            case PE_FRS_SNK_SRC_Vbus_Applied: /* C */
                break;
#endif

            case PSM_PE_BIST_TEST_DATA_MODE: /* (C) */
                break;

            case PSM_PE_BIST_CARRIER_MODE_2: /* (C) */
            {
#if (defined PD_CONFIG_COMPLIANCE_TEST_ENABLE) && (PD_CONFIG_COMPLIANCE_TEST_ENABLE)
                pd_bist_mst_t bistMode = kBIST_CarrierMode2;

                (void)PD_TimerStart(pdInstance, tBISTContModeTimer, T_BIST_CONT_MODE);
                /* Delay for 1ms to make sure any previous GoodCRC tranmission has ended. */
                (void)PD_PsmTimerWait(pdInstance, tDelayTimer, mSec(1), 0U);
                (void)PD_PhyControl(pdInstance, PD_PHY_ENTER_BIST, &bistMode);
#endif
                break;
            }

            case PSM_CHECK_ASYNC_RX: /* (C) */
                break;

            case PSM_BYPASS: /* (C) */
                break;

            default:
                /* No action required. */
                break;
        }

        if (returnState != kSM_None)
        {
            return returnState;
        }
        PD_PsmEndCommand(pdInstance);
        PD_PsmSetNormalPower(pdInstance);
    }

    PD_PsmProcessImportEventBeforeNextStateMachine(pdInstance);
    if (didNothingC == 1U)
    {
        return kSM_WaitEvent;
    }
    else
    {
        return kSM_Continue;
    }
}

static pd_state_machine_state_t PD_PsmGetNewestEvent(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo)
{
    uint32_t taskEventSet = 0;
    uint8_t processMessage;

    (void)OSA_EventGet(pdInstance->taskEventHandle, PD_TASK_EVENT_ALL, &taskEventSet);

    if ((taskEventSet & (uint32_t)PD_TASK_EVENT_TIME_OUT) != 0U)
    {
        (void)OSA_EventClear(pdInstance->taskEventHandle, PD_TASK_EVENT_TIME_OUT);
#if ((defined PD_CONFIG_EXTENDED_CHUNK_LAYER_SUPPORT) && (PD_CONFIG_EXTENDED_CHUNK_LAYER_SUPPORT))
        if (PD_PsmChunkingLayerRXTimerWaiting(pdInstance) != 0U)
        {
            (void)PD_PsmChunkingLayerRXStateMachine(pdInstance, triggerInfo);
        }
#endif
#if (defined(PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)) && \
    (defined(PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE) && (PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE))
        if ((PD_TimerCheckInvalidOrTimeOut(pdInstance, tDpmComandRetryTimer) != 0U) &&
            (pdInstance->commandRetryCount > 0U))
        {
            PD_StackSetEvent(pdInstance, PD_TASK_EVENT_DPM_MSG);
            return kSM_Continue;
        }
#endif
    }
    else if ((taskEventSet & (uint32_t)PD_TASK_EVENT_RECEIVED_HARD_RESET) != 0U)
    {
        (void)OSA_EventClear(pdInstance->taskEventHandle, PD_TASK_EVENT_PD_MSG);
        (void)OSA_EventClear(pdInstance->taskEventHandle, PD_TASK_EVENT_RECEIVED_HARD_RESET);
        if (pdInstance->psmCurState != PSM_PE_BIST_CARRIER_MODE_2)
        {
            /* Hard reset is ignored when in BIST carrier mode */
            pdInstance->hardResetReceived = 0;
            triggerInfo->triggerEvent     = PSM_TRIGGER_RECEIVE_HARD_RESET;
        }
    }
    else if ((taskEventSet & (uint32_t)PD_TASK_EVENT_FR_SWAP_SINGAL) != 0U)
    {
        (void)OSA_EventClear(pdInstance->taskEventHandle, PD_TASK_EVENT_FR_SWAP_SINGAL);
        if (pdInstance->curPowerRole == kPD_PowerRoleSink)
        {
            pdInstance->psmNewState = PE_FRS_SNK_SRC_Send_Swap;
        }
    }
    else if ((taskEventSet & (uint32_t)PD_TASK_EVENT_PD_MSG) != 0U)
    {
        (void)OSA_EventClear(pdInstance->taskEventHandle, PD_TASK_EVENT_PD_MSG);

        if (pdInstance->psmCurState != PSM_PE_BIST_TEST_DATA_MODE)
        {
            processMessage = 0;
            if (PD_MsgGetReceiveResult(pdInstance) != 0U)
            {
                if (pdInstance->receivedSop == kPD_MsgSOP)
                {
                    processMessage = 1;
                }
#if (defined PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
                else if ((pdInstance->receivedSop == kPD_MsgSOPp) || (pdInstance->receivedSop == kPD_MsgSOPpp))
                {
                    PD_MsgReceive(pdInstance);
                    processMessage = 1;
                }
#endif
                else
                {
                    /* No action required. */
                }
            }

            if (processMessage != 0U)
            {
                uint8_t msgType;
                triggerInfo->msgHeader.msgHeaderVal = (uint16_t)(MSG_DATA_HEADER);
                triggerInfo->pdMsgSop               = pdInstance->receivedSop;
                triggerInfo->pdMsgDataBuffer        = MSG_DATA_BUFFER;
                triggerInfo->pdMsgDataLength =
                    (uint8_t)((MSG_DATA_HEADER & PD_MSG_HEADER_NUMBER_OF_DATA_OBJECTS_MASK) >>
                              PD_MSG_HEADER_NUMBER_OF_DATA_OBJECTS_POS);
                msgType = (uint8_t)triggerInfo->msgHeader.bitFields.messageType;

                if (triggerInfo->msgHeader.bitFields.extended == 0U)
                {
                    if (triggerInfo->msgHeader.bitFields.NumOfDataObjs == 0U)
                    {
                        triggerInfo->pdMsgType = (message_type_t)msgType;
                        /* There are three places: two message will be sent continuously by one port.
                         * 1. fast role swap.
                         * 2. sink beginning power role swap.
                         * 3. rdo request in source.
                         * 4. after soft reset, the partner will send source_caps.
                         */
                        if (triggerInfo->pdMsgType == kPD_MsgAccept)
                        {
                            if (pdInstance->psmCurState != PSM_SEND_SOFT_RESET)
                            {
                                PD_MsgReceive(pdInstance);
                            }
                        }
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
                        if (triggerInfo->pdMsgType == kPD_MsgFrSwap)
                        {
                            pdInstance->frSwapReceived = 1;
                        }
#endif
                    }
                    else
                    {
                        msgType |= PD_MSG_DATA_TYPE_MASK;
                        triggerInfo->pdMsgType = (message_type_t)msgType;

                        if (triggerInfo->pdMsgType == kPD_MsgRequest)
                        {
                            PD_PsmCheckRevision(pdInstance, triggerInfo->msgHeader);
                        }
                    }
                }
                else
                {
                    msgType |= PD_MSG_EXT_TYPE_MASK;
                    triggerInfo->pdMsgType = (message_type_t)msgType;

                    triggerInfo->pdExtMsgLength =
                        ((uint32_t)((uint8_t *)triggerInfo->pdMsgDataBuffer)[0] +
                         ((uint32_t)((uint8_t *)triggerInfo->pdMsgDataBuffer)[1] & 0x01U) * 256U);
                }
                triggerInfo->triggerEvent = PSM_TRIGGER_PD_MSG;

                if (triggerInfo->pdMsgType == kPD_MsgVendorDefined)
                {
                    triggerInfo->vdmHeader.structuredVdmHeaderVal =
                        USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)triggerInfo->pdMsgDataBuffer));

                    /* Vendor Defined message can be interrupted. */
                    PD_MsgReceive(pdInstance);
                    (void)OSA_EventGet(pdInstance->taskEventHandle, PD_TASK_EVENT_PD_MSG, &taskEventSet);
                    if (taskEventSet != 0U)
                    {
                        PD_PsmProcessImportEventBeforeNextStateMachine(pdInstance);
                        return kSM_Continue; /* process next message */
                    }
                }
#if ((defined PD_CONFIG_EXTENDED_CHUNK_LAYER_SUPPORT) && (PD_CONFIG_EXTENDED_CHUNK_LAYER_SUPPORT))
                if (PD_PsmChunkingLayerRXStateMachine(pdInstance, triggerInfo) == 0U)
                {
                    return kSM_Continue;
                }
#endif
            }
            else
            {
            }
        }
        else
        {
            /* Ignore everything except Hard Reset and disconnect */
        }
    }
    else if ((taskEventSet & (uint32_t)PD_TASK_EVENT_DPM_MSG) != 0U)
    {
        pd_command_t command = (pd_command_t)PD_DpmGetMsg(pdInstance);

        switch (command)
        {
            case PD_DPM_CONTROL_SOFT_RESET:
            case PD_DPM_CONTROL_HARD_RESET:
            case PD_DPM_FAST_ROLE_SWAP:
                break;

            default:
                if ((pdInstance->psmCurState != PSM_PE_SNK_READY) && (pdInstance->psmCurState != PSM_PE_SRC_READY))
                {
                    command = PD_DPM_INVALID;
                }
                break;
        }

        if (command != PD_DPM_INVALID)
        {
#if (defined(PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)) && \
    (defined(PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE) && (PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE))
            if ((PD_MsgSnkCheckStartCommand(pdInstance, command) == 0U) &&
                (pdInstance->curPowerRole == kPD_PowerRoleSink))
            {
                if (pdInstance->commandRetryCount > 0U)
                {
                    (void)PD_TimerStart(pdInstance, tDpmComandRetryTimer, T_DPM_COMMAND_RETRY);
                    pdInstance->commandRetryCount--;
                }
                else
                {
                    PD_PsmCommandFail(pdInstance, command);
                    PD_DpmClearMsg(pdInstance, command);
                }
            }
            else
            {
#endif
                triggerInfo->triggerEvent = PSM_TRIGGER_DPM_MSG;
                triggerInfo->dpmMsg       = command;
                PD_DpmClearMsg(pdInstance, command);

#if (defined(PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)) && \
    (defined(PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE) && (PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE))
                pdInstance->commandRetryCount = 0;
            }
#endif
        }
        else
        {
#if (defined(PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)) && \
    (defined(PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE) && (PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE))
            pdInstance->commandRetryCount = 0;
#endif
            PD_PsmCommandFail(pdInstance, command);
            PD_DpmClearMsg(pdInstance, command);
        }
        (void)OSA_EventClear(pdInstance->taskEventHandle, PD_TASK_EVENT_DPM_MSG);
    }
    else if ((taskEventSet & (uint32_t)PD_TASK_EVENT_SEND_DONE) != 0U)
    {
        (void)OSA_EventClear(pdInstance->taskEventHandle, PD_TASK_EVENT_SEND_DONE);
    }
    else
    {
        PD_PortTaskEventProcess(pdInstance, taskEventSet);
        triggerInfo->triggerEvent = PSM_TRIGGER_NONE;
    }

    return kSM_None;
}

static uint8_t PD_PsmCheckTimeOutHardReset(pd_instance_t *pdInstance)
{
    uint8_t needHardReset = 0;
    if ((pdInstance->psmHardResetCount <= N_HARD_RESET_COUNT) && (pdInstance->curPowerRole == kPD_PowerRoleSink))
    {
        if (PD_TimerCheckValidTimeOut(pdInstance, tSinkWaitCapTimer) != 0U)
        {
            (void)PD_TimerClear(pdInstance, tSinkWaitCapTimer);
            needHardReset = 1;
        }
        if (PD_TimerCheckValidTimeOut(pdInstance, tPSTransitionTimer) != 0U)
        {
            (void)PD_TimerClear(pdInstance, tPSTransitionTimer);
            needHardReset = 1;
        }
        if (PD_TimerCheckValidTimeOut(pdInstance, tNoResponseTimer) != 0U)
        {
            (void)PD_TimerClear(pdInstance, tNoResponseTimer);
            needHardReset = 1;
        }
    }

    return needHardReset;
}

static uint8_t PD_PsmProcessHardResetState(pd_instance_t *pdInstance)
{
#if (defined PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
    if (pdInstance->curPowerRole == kPD_PowerRoleSource)
    {
        /* source send hard_reset, 2. after tpstimer. */
        if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tPSHardResetTimer) != 0U)
        {
            /* Hard reset is complete */
            pdInstance->psmNewState = PSM_PE_SRC_TRANSITION_TO_DEFAULT;
        }
        else
        {
            return 1;
        }
    }
    else
#endif
#if (defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
        if (pdInstance->curPowerRole == kPD_PowerRoleSink)
    {
        /* sink send hard_reset */
        pdInstance->psmNewState = PD_PsmSinkHardResetfunction(pdInstance);
    }
    else
#endif
    {
    }
    return 0;
}

static uint8_t PD_PsmProcessSendSoftResetState(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo)
{
    if ((triggerInfo->triggerEvent == PSM_TRIGGER_PD_MSG) && (triggerInfo->pdMsgSop == kPD_MsgSOP) &&
        (triggerInfo->pdMsgType == kPD_MsgAccept))
    {
        (void)PD_TimerClear(pdInstance, tSenderResponseTimer);
        (void)PD_DpmAppCallback(pdInstance, PD_DPM_SOFT_RESET_SUCCESS, NULL,
                                (pdInstance->commandProcessing == PD_DPM_CONTROL_SOFT_RESET) ? 1U : 0U);

        if (pdInstance->curPowerRole == kPD_PowerRoleSource)
        {
            pdInstance->psmNewState = PSM_PE_SRC_IMPLICIT_CABLE_DISCOVERY;
        }
        else
        {
            pdInstance->psmNewState = PSM_PE_SNK_WAIT_FOR_CAPABILITIES;
        }
    }
    else if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tSenderResponseTimer) != 0U)
    {
        if (pdInstance->commandProcessing == PD_DPM_CONTROL_SOFT_RESET)
        {
            (void)PD_DpmAppCallback(pdInstance, PD_DPM_SOFT_RESET_FAIL, NULL, 1);
        }
        pdInstance->psmNewState = PSM_HARD_RESET;
    }
    else
    {
        return 1;
    }
    return 0;
}

static void PD_PsmCheckUnexpectedReceivedMsg(pd_instance_t *pdInstance,
                                             trigger_event_t triggerEvent,
                                             uint8_t *didNothingStepB)
{
    if (triggerEvent == PSM_TRIGGER_PD_MSG)
    {
        switch (pdInstance->psmCurState)
        {
            case PSM_PE_SRC_STARTUP:
            case PSM_PE_SRC_DISCOVERY:
                pdInstance->psmNewState = PSM_HARD_RESET;
                *didNothingStepB        = 0;
                break;
            case PSM_PE_PRS_SRC_SNK_TRANSITION_TO_OFF:
            case PSM_PE_PRS_SNK_SRC_SOURCE_ON:
                pdInstance->psmNewState = PSM_CHECK_ASYNC_RX;
                *didNothingStepB        = 0;
                break;

            case PSM_PE_SNK_WAIT_FOR_CAPABILITIES:
                pdInstance->psmNewState = PSM_SEND_SOFT_RESET;
                *didNothingStepB        = 0;
                break;

            default:
                /* No action required. */
                break;
        }
    }
}

static uint8_t PD_PsmProcessReadyState(pd_instance_t *pdInstance, psm_trigger_info_t *triggerInfo)
{
    uint8_t didNothingStepB = 0;
#if ((defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE))
    uint8_t didNothing = 1;

    didNothing &= PD_PsmSecondaryStateHandler(pdInstance, kPD_MsgSOP, triggerInfo);

#if (defined PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
    if ((PD_TimerCheckValidTimeOut(pdInstance, tDiscoverIdentityTimer) != 0U) &&
        (pdInstance->psmSecondaryState[1] == PSM_IDLE) &&
        (pdInstance->cableDiscoverIdentityCounter < N_DISCOVER_IDENTITY_COUNTER))
    {
        pdInstance->cableDiscoverIdentityTimeout = 1;
        (void)PD_TimerClear(pdInstance, tDiscoverIdentityTimer);
        pdInstance->psmNewSecondaryState[1] = PSM_PE_SRC_VDM_IDENTITY_REQUEST;
    }

    didNothing &= PD_PsmSecondaryStateHandler(pdInstance, kPD_MsgSOPp, triggerInfo);
    didNothing &= PD_PsmSecondaryStateHandler(pdInstance, kPD_MsgSOPpp, triggerInfo);

    /* second state is IDLE && don't get success && count is not max && DiscoverIdentityTimer is timeout */
    if ((pdInstance->psmSecondaryState[1] == PSM_IDLE) &&
        (PD_CheckWhetherInitiateCableDiscoveryIdentityOrNot(pdInstance) != 0U) &&
        (pdInstance->cableDiscoverIdentityTimeout != 0U))
    {
        pdInstance->cableDiscoverIdentityTimeout = 0;
        (void)PD_TimerStart(pdInstance, tDiscoverIdentityTimer, T_DISCOVER_IDENTITY);
    }
    /* cable discover success || beyond maximum count */
    else if (((pdInstance->psmCableIdentitiesDataCount > 0U) ||
              (pdInstance->cableDiscoverIdentityCounter >= N_DISCOVER_IDENTITY_COUNTER)) &&
             (pdInstance->pendingSOP != (uint8_t)kPD_MsgSOPMask))
    {
        pdInstance->pendingSOP   = (uint8_t)kPD_MsgSOPMask;
        pdInstance->receiveState = 0;
        PD_MsgReceive(pdInstance);
    }
    else
    {
        /* No action required. */
    }
#endif

    if ((pdInstance->psmSecondaryState[0] == PSM_IDLE) && (pdInstance->psmSecondaryState[1] == PSM_IDLE) &&
        (pdInstance->psmSecondaryState[2] == PSM_IDLE) && (didNothing != 0U))
    {
        didNothingStepB = 1;
    }
#endif

    if (triggerInfo->triggerEvent == PSM_TRIGGER_PD_MSG)
    {
        PD_PsmRdyProcessPdMessage(pdInstance, triggerInfo);
    }
    else if (triggerInfo->triggerEvent == PSM_TRIGGER_DPM_MSG)
    {
#if (defined PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
        if (pdInstance->psmCurState == PSM_PE_SRC_READY)
        {
            PD_PsmSourceRdyProcessDpmMessage(pdInstance, triggerInfo);
        }
#endif
#if (defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
        if (pdInstance->psmCurState == PSM_PE_SNK_READY)
        {
            PD_PsmSinkRdyProcessDpmMessage(pdInstance, triggerInfo);
        }
#endif
    }
    else
    {
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
        didNothingStepB = PD_PsmReadyAutoPolicyProcess(pdInstance);
#else
        didNothingStepB         = 1;
#endif
    }

    return didNothingStepB;
}

#if (defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)) || \
    ((defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE))
static inline uint8_t PD_PsmGetTypeCCurrent(pd_instance_t *pdInstance)
{
    uint8_t typeCurrent = (uint8_t)kCurrent_Invalid;
    (void)PD_PhyControl(pdInstance, PD_PHY_GET_TYPEC_CURRENT_CAP, &typeCurrent);
    return typeCurrent;
}
#endif

static pd_state_machine_state_t PD_PsmProcessState(pd_instance_t *pdInstance)
{
    psm_trigger_info_t triggerInfo;
    pd_state_machine_state_t returnState = kSM_None;
    uint8_t didNothingStepA              = 1;
    uint8_t didNothingStepB              = 1;
    uint8_t msgReceived                  = 0;

    if (pdInstance->psmCurState != PSM_UNKNOWN)
    {
        triggerInfo.triggerEvent                     = PSM_TRIGGER_NONE;
        triggerInfo.pdMsgType                        = kPD_MsgInvalid;
        triggerInfo.pdMsgSop                         = kPD_MsgSOPInvalid;
        triggerInfo.dpmMsg                           = PD_DPM_INVALID;
        triggerInfo.pdMsgDataBuffer                  = NULL;
        triggerInfo.vdmHeader.structuredVdmHeaderVal = 0U;

        /* get the newest events */
        if (PD_PsmGetNewestEvent(pdInstance, &triggerInfo) == kSM_Continue)
        {
            return kSM_Continue;
        }
        if (triggerInfo.triggerEvent == PSM_TRIGGER_PD_MSG)
        {
            msgReceived = 1;
        }

        if (((triggerInfo.triggerEvent == PSM_TRIGGER_PD_MSG) && (triggerInfo.pdMsgSop == kPD_MsgSOP)) ||
            (triggerInfo.triggerEvent == PSM_TRIGGER_RECEIVE_HARD_RESET))
        {
            didNothingStepA &= PD_PsmPrimaryStateProcessPdMsg(pdInstance, &pdInstance->psmNewState, &triggerInfo);
        }
        else if (triggerInfo.triggerEvent == PSM_TRIGGER_DPM_MSG)
        {
            didNothingStepA &= PD_PsmPrimaryStateProcessDpmMsg(pdInstance, &pdInstance->psmNewState, &triggerInfo);
        }
        else
        {
            /* No action required. */
        }

        if (didNothingStepA == 0U)
        {
            if ((msgReceived != 0U) && (PD_PsmCanPendingReceive(pdInstance) != 0U))
            {
                PD_MsgReceive(pdInstance); /* msg has been processed, receiving next msg */
            }
            PD_PsmProcessImportEventBeforeNextStateMachine(pdInstance);
            return kSM_Continue;
        }

        /* different timeout need hard_reset */
        if (PD_PsmCheckTimeOutHardReset(pdInstance) != 0U)
        {
            didNothingStepA         = 0;
            pdInstance->psmNewState = PSM_HARD_RESET;
        }
        /* alternate mode tAMETimeoutTimer */
        else if (PD_TimerCheckValidTimeOut(pdInstance, tAMETimeoutTimer) != 0U)
        {
            (void)PD_TimerClear(pdInstance, tAMETimeoutTimer);
            PD_ConnectAltModeEnterFail(pdInstance, pdInstance->psmPresentlyPdConnected);

#ifdef USBPD_ENABLE_USB_BILLBOARD
            UsbbSetAltModeConfigResult(AMR_CONFIG_FAILED);
#endif
        }
        /* Check if we are waiting for an event for state transition */
        else if (pdInstance->psmNewState == pdInstance->psmCurState)
        {
            didNothingStepB = 0;
            switch (pdInstance->psmCurState)
            {
                case PSM_EXIT_TO_ERROR_RECOVERY:            /* (B) */
                case PSM_EXIT_TO_ERROR_RECOVERY_WITH_DELAY: /* (B) */
                    returnState = kSM_ErrorRecovery;
                    break;

#if (defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)
                case PSM_INTERRUPTED_REQUEST: /* (B) */
                    /* Transitioned back here after VDM handling, re attempt the request */
                    pdInstance->psmNewState         = pdInstance->psmInterruptedState;
                    pdInstance->psmInterruptedState = PSM_UNKNOWN;
                    break;
#endif

                /* do hard_reset actively do in (C) */
                case PSM_HARD_RESET: /* (B) */
                    didNothingStepB = PD_PsmProcessHardResetState(pdInstance);
                    break;

                case PSM_SEND_SOFT_RESET: /* (B) */
                    didNothingStepB = PD_PsmProcessSendSoftResetState(pdInstance, &triggerInfo);
                    break;

                case PSM_SOFT_RESET: /* (B) */
                    /* If we get here, then recover with a hard reset */
                    pdInstance->psmNewState = PSM_HARD_RESET;
                    break;

                case PSM_CHECK_ASYNC_RX: /* (B) */
                    /* The async conditions at the top of the loop have been evaluated */
                    /* continue to soft reset */
                    pdInstance->psmNewState = PSM_SEND_SOFT_RESET;
                    break;

                case PSM_BYPASS: /* (B) */
                    /* We do nothing here. */
                    didNothingStepB = 1;
                    break;

#if (defined PD_CONFIG_SOURCE_ROLE_ENABLE) && (PD_CONFIG_SOURCE_ROLE_ENABLE)
                case PSM_PE_SRC_STARTUP: /* (B) */
                {
                    if (PD_TimerCheckValidTimeOut(pdInstance, tSwapSourceStartTimer) != 0U)
                    {
                        (void)PD_TimerClear(pdInstance, tSwapSourceStartTimer);
                        pdInstance->psmNewState = PSM_PE_SRC_IMPLICIT_CABLE_DISCOVERY;
                    }
                    else
                    {
                        didNothingStepB = 1;
                    }
                    break;
                }

                case PSM_PE_SRC_DISCOVERY: /* (B) */
                {
                    if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tSourceCapabilityTimer) != 0U)
                    {
                        if ((pdInstance->psmSendCapsCounter > N_CAPS_COUNT))
                        {
                            pdInstance->psmNewState = PSM_PE_SRC_DISABLED;
                        }
                        else
                        {
#if (defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)) && \
    (defined(PD_CONFIG_SRC_AUTO_DISCOVER_CABLE_PLUG) && (PD_CONFIG_SRC_AUTO_DISCOVER_CABLE_PLUG))
                            pdInstance->pendingSOP = (uint8_t)kPD_MsgSOPMask;
#endif
                            pdInstance->psmNewState = PSM_PE_SRC_SEND_CAPABILITIES;
                        }
                    }
#if (defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)) && \
    (defined(PD_CONFIG_SRC_AUTO_DISCOVER_CABLE_PLUG) && (PD_CONFIG_SRC_AUTO_DISCOVER_CABLE_PLUG))
                    else if (PD_CheckWhetherInitiateCableDiscoveryIdentityOrNot(pdInstance) != 0U)
                    {
                        /* cable plug didn't response data object on PE_SRC_STARTUP state */
                        (void)PD_PsmSecondaryStateHandler(pdInstance, kPD_MsgSOPp, &triggerInfo);
                    }
#endif
                    else if (PD_PsmNoResponseHardResetCountCheck(pdInstance) == 0U)
                    {
                        didNothingStepB = 1;
                    }
                    else
                    {
                    }
                    break;
                }

                case PSM_PE_SRC_IMPLICIT_CABLE_DISCOVERY: /* (B) */
                {
#if (defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)) && \
    (defined(PD_CONFIG_SRC_AUTO_DISCOVER_CABLE_PLUG) && (PD_CONFIG_SRC_AUTO_DISCOVER_CABLE_PLUG))
                    (void)PD_PsmSecondaryStateHandler(pdInstance, kPD_MsgSOPp, &triggerInfo);
                    if (pdInstance->psmSecondaryState[1] == PSM_IDLE)
                    {
                        pdInstance->pendingSOP  = (uint8_t)kPD_MsgSOPMask;
                        pdInstance->psmNewState = PSM_PE_SRC_SEND_CAPABILITIES;
                    }
#endif
                    break;
                }

                case PSM_PE_SRC_SEND_CAPABILITIES: /* (B) */
                {
                    if (triggerInfo.triggerEvent == PSM_TRIGGER_PD_MSG)
                    {
                        (void)PD_TimerClear(pdInstance, tSenderResponseTimer);
                        if (triggerInfo.pdMsgType == kPD_MsgRequest)
                        {
                            pdInstance->partnerRdoRequest.rdoVal =
                                USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)triggerInfo.pdMsgDataBuffer));
                            pdInstance->psmNewState = PSM_PE_SRC_NEGOTIATE_CAPABILITY;
                        }
                        else
                        {
                            /* discard vendor defined message at here */
                            if (triggerInfo.pdMsgType != kPD_MsgVendorDefined)
                            {
                                pdInstance->psmNewState = PSM_SEND_SOFT_RESET;
                            }
                        }
                    }
                    else if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tSenderResponseTimer) != 0U)
                    {
                        pdInstance->psmNewState = PSM_HARD_RESET;
                    }
                    else if (PD_PsmNoResponseHardResetCountCheck(pdInstance) == 0U)
                    {
                        didNothingStepB = 1;
                    }
                    else
                    {
                        /* No action required. */
                    }
                    break;
                }

                case PSM_PE_SRC_TRANSITION_SUPPLY: /* (B) */
                {
                    (void)PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(pdInstance, kPD_MsgPsRdy,
                                                                             PSM_PE_SRC_READY);
                    if (pdInstance->psmGotoMinTx != 0U)
                    {
                        pdInstance->psmGotoMinTx = 0;
                        (void)PD_DpmAppCallback(pdInstance, PD_DPM_SRC_GOTOMIN_SUCCESS, NULL, 1);
                    }
                    else if (pdInstance->psmNewState == PSM_PE_SRC_READY)
                    {
                        (void)PD_DpmAppCallback(pdInstance, PD_DPM_SRC_RDO_SUCCESS, NULL, 1);
                    }
                    else
                    {
                        /* No action required. */
                    }
                    break;
                }

                case PSM_PE_SRC_READY: /* (B) */
                {
                    if (PD_PsmProcessReadyState(pdInstance, &triggerInfo) != 0U)
                    {
#if (defined PD_CONFIG_PD3_PPS_ENABLE) && (PD_CONFIG_PD3_PPS_ENABLE)
                        if ((PD_TimerCheckValidTimeOut(pdInstance, tSourcePPSCommTimer) != 0U) &&
                            (PD_PsmSourceIsPPSRDO(pdInstance) != 0U))
                        {
                            pdInstance->psmNewState = PSM_HARD_RESET;
                        }
                        else
#endif
                        {
#if defined(PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE) && (PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE)
                            typec_current_val_t typecCurrent = kCurrent_Invalid;

                            (void)PD_PhyControl(pdInstance, PD_PHY_GET_TYPEC_CURRENT_CAP, &typecCurrent);
                            if (typecCurrent != kCurrent_3A)
                            {
                                typecCurrent = kCurrent_3A;
                                (void)PD_PhyControl(pdInstance, PD_PHY_SRC_SET_TYPEC_CURRENT_CAP, &typecCurrent);
                            }
#endif
                            didNothingStepB = 1;
                        }
                    }
                    break;
                }

                case PSM_PE_SRC_DISABLED: /* (B) */
                {
                    if (PD_PsmNoResponseHardResetCountCheck(pdInstance) == 0U)
                    {
                        didNothingStepB = 1;
                    }
                    break;
                }

                case PSM_PE_SRC_WAIT_NEW_CAPABILITIES: /* (B) */
                    /* The exit transition is handled by the global state transitions */
                    didNothingStepB = 1;
                    break;

                case PSM_PE_SRC_HARD_RESET_RECEIVED: /* (B) */
                {
                    if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tPSHardResetTimer) != 0U)
                    {
                        pdInstance->psmNewState = PSM_PE_SRC_TRANSITION_TO_DEFAULT;
                    }
                    else
                    {
                        didNothingStepB = 1;
                    }
                    break;
                }

                case PSM_PE_SRC_TRANSITION_TO_DEFAULT: /* (B) */
                {
                    if (pdInstance->psmHardResetNeedsVSafe0V != 0U)
                    {
                        /* source send hard_reset, 4. after tSrcRecover, open vsafe5v. */
                        /* source receive hard_reset, 4. after tSrcRecover, open vsafe5v. */
                        /* 2. wait tSrcRecover */
                        if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tSrcRecoverTimer) != 0U)
                        {
                            pdInstance->psmHardResetNeedsVSafe0V = 0;
                            (void)pdInstance->callbackFns->PD_SrcTurnOnTypeCVbus(pdInstance->callbackParam,
                                                                                 kVbusPower_InHardReset);
#if defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
                            PD_DpmDischargeVconn(pdInstance, 0);
                            PD_DpmSetVconn(pdInstance, 1);
#endif /* PD_CONFIG_VCONN_SUPPORT */
                        }
                        else
                        {
                            didNothingStepB = 1;
                        }
                    }
                    /* source send hard_reset, 5. wait vsafe5v. */
                    /* source receive hard_reset, 5. wait vsafe5v. */
                    else if (PD_PsmCheckVsafe5V(pdInstance) != 0U)
                    {
                        (void)PD_TimerStart(pdInstance, tNoResponseTimer, T_NO_RESPONSE);
                        pdInstance->enterSrcFromSwap = 0;
                        pdInstance->psmNewState      = PSM_PE_SRC_STARTUP;
                    }
                    else
                    {
                        /* Wait until the power supply transitions to default. */
                        didNothingStepB = 0;
                    }
                    break;
                }

#endif

#if (defined PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
                case PSM_PE_SNK_DISCOVERY: /* (B) */
                {
                    if (PD_PsmNoResponseHardResetCountCheck(pdInstance) != 0U)
                    {
                    }
                    else if (pdInstance->psmHardResetNeedsVSafe0V != 0U)
                    {
                        if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tPSHardResetTimer) != 0U)
                        {
                            pdInstance->psmHardResetNeedsVSafe0V = 0U;
                        }
                        else if (PD_PsmCheckVsafe5V(pdInstance) == 0U)
                        {
                            pdInstance->psmHardResetNeedsVSafe0V = 0U;
                        }
                        else
                        {
                            /* No change in state, we should stay in low power mode */
                            didNothingStepB = 1U;
                        }
                    }
                    else if (PD_PsmCheckVsafe5V(pdInstance) != 0U)
                    {
                        if (pdInstance->asmHardResetSnkProcessing != 0U)
                        {
                            pdInstance->asmHardResetSnkProcessing = 0U;
                            (void)pdInstance->callbackFns->PD_SnkDrawTypeCVbus(
                                pdInstance->callbackParam, (uint8_t)PD_PsmGetTypeCCurrent(pdInstance),
                                kVbusPower_InHardReset);
                        }

                        /* Hard Reset is no longer in progress, allow VBus monitoring */
                        /* kVbusPower_InHardReset is end */
                        PD_ConnectSetPowerProgress(pdInstance, kVbusPower_Stable);
                        pdInstance->psmHardResetNeedsVSafe0V = 0U;

                        pdInstance->psmNewState = PSM_PE_SNK_WAIT_FOR_CAPABILITIES;
                    }
                    else
                    {
                        /* need check vbus constantly */
                        didNothingStepB = 0;
                    }
                    break;
                }

                case PSM_PE_SNK_WAIT_FOR_CAPABILITIES: /* (B) */
                {
                    if (PD_PsmNoResponseHardResetCountCheck(pdInstance) == 0U)
                    {
                        didNothingStepB = 1;
                    }
                    else
                    {
                    }
                    break;
                }

                case PSM_PE_SNK_SELECT_CAPABILITY: /* (B) */
                {
                    if (triggerInfo.triggerEvent == PSM_TRIGGER_PD_MSG)
                    {
                        pd_command_result_t commandResultCallback;

                        switch (triggerInfo.pdMsgType)
                        {
                            case kPD_MsgAccept:
                                pdInstance->psmNewState = PSM_PE_SNK_TRANSITION_SINK;
                                break;

                            case kPD_MsgWait:
                                commandResultCallback = kCommandResult_Wait;
                                (void)PD_DpmAppCallback(pdInstance, PD_DPM_SNK_RDO_FAIL, &commandResultCallback, 1);
                                if (pdInstance->psmExplicitContractExisted != 0U)
                                {
                                    pdInstance->psmNewState               = PSM_PE_SNK_READY;
                                    pdInstance->psmSnkReceiveRdoWaitRetry = 1;
                                    (void)PD_TimerStart(pdInstance, tSinkRequestTimer, T_SINK_REQUEST);
                                }
                                else
                                {
                                    pdInstance->psmNewState = PSM_PE_SNK_WAIT_FOR_CAPABILITIES;
                                }
                                break;

                            case kPD_MsgReject:
                                commandResultCallback = kCommandResult_Reject;
                                (void)PD_DpmAppCallback(pdInstance, PD_DPM_SNK_RDO_FAIL, &commandResultCallback, 1);
                                if (pdInstance->psmExplicitContractExisted == 0U)
                                {
                                    pdInstance->psmNewState = PSM_PE_SNK_WAIT_FOR_CAPABILITIES;
                                }
                                else
                                {
                                    pdInstance->psmNewState = PSM_PE_SNK_READY;
                                }
                                break;

                            default:
                                if (triggerInfo.pdMsgType != kPD_MsgInvalid)
                                {
                                    /* SourceCapabilities and VendorDefined are handled in the global section. */
                                    /* soft reset other packets */
                                    pdInstance->psmNewState = PSM_CHECK_ASYNC_RX;
                                }
                                break;
                        }
                    }
                    else if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tSenderResponseTimer) != 0U)
                    {
                        pdInstance->psmNewState = PSM_HARD_RESET;
                    }
                    else
                    {
                        didNothingStepB = 1;
                    }
                    break;
                }

                case PSM_PE_SNK_SELECT_CAPABILITY_WAIT_TIMER_TIME_OUT: /* (B) */
                {
                    if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tSinkRequestTimer) != 0U)
                    {
                        pdInstance->psmNewState = PSM_PE_SNK_SELECT_CAPABILITY;
                    }
                    break;
                }

                case PSM_PE_SNK_TRANSITION_SINK: /* (B) */
                {
                    if (triggerInfo.triggerEvent == PSM_TRIGGER_PD_MSG)
                    {
                        /* SourceCapabilities and VendorDefined are handled in the global section. */
                        if ((triggerInfo.pdMsgSop == kPD_MsgSOP) && (triggerInfo.pdMsgType == kPD_MsgPing))
                        {
                            /* Remain in the same state */
                            /* pdInstance->psmNewState = PSM_PE_SNK_TRANSITION_SINK; */
                        }
                        else
                        {
                            (void)PD_TimerClear(pdInstance, tPSTransitionTimer);
                            if ((triggerInfo.pdMsgSop == kPD_MsgSOP) && (triggerInfo.pdMsgType == kPD_MsgPsRdy))
                            {
                                PD_MsgReceive(pdInstance);
                                if (pdInstance->psmGotoMinRx != 0U)
                                {
                                    pdInstance->psmGotoMinRx = 0;
                                    (void)PD_DpmAppCallback(pdInstance, PD_DPM_SNK_GOTOMIN_SUCCESS, NULL, 1);
                                }
                                else
                                {
                                    (void)pdInstance->callbackFns->PD_SnkDrawRequestVbus(pdInstance->callbackParam,
                                                                                         pdInstance->rdoRequest);
                                    (void)PD_DpmAppCallback(pdInstance, PD_DPM_SNK_RDO_SUCCESS, &pdInstance->rdoRequest,
                                                            1);
                                }
                                pdInstance->psmNewState = PSM_PE_SNK_READY;
                            }
                            else
                            {
                                pdInstance->psmNewState = PSM_HARD_RESET;
                            }
                        }
                    }
                    else
                    {
                        didNothingStepB = 1;
                    }
                    break;
                }

                case PSM_PE_SNK_READY: /* (B) */
                {
                    if (PD_PsmProcessReadyState(pdInstance, &triggerInfo) != 0U)
                    {
                        if (PD_TimerCheckValidTimeOut(pdInstance, tSinkRequestTimer) != 0U)
                        {
                            if (pdInstance->psmSnkReceiveRdoWaitRetry != 0U)
                            {
                                pdInstance->psmSnkReceiveRdoWaitRetry = 0U;
                                pdInstance->psmNewState               = PSM_PE_SNK_SELECT_CAPABILITY;
                            }
                        }
#if (defined PD_CONFIG_PD3_PPS_ENABLE) && (PD_CONFIG_PD3_PPS_ENABLE)
                        else if ((PD_TimerCheckValidTimeOut(pdInstance, tSinkPPSPeriodicTimer) != 0U) &&
                                 (PD_PsmSinkIsPPSRDO(pdInstance) != 0U))
                        {
                            pdInstance->psmNewState = PSM_PE_SNK_EVALUATE_CAPABILITY;
                        }
#endif
                        else
                        {
                            didNothingStepB = 1;
                        }
                    }
                    break;
                }

                case PSM_PE_SNK_TRANSITION_TO_DEFAULT: /* (B) */
                    break;
#endif

#if defined(PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
                case PSM_PE_DR_SRC_GET_SOURCE_CAP: /* (B) */
                {
                    if (triggerInfo.triggerEvent == PSM_TRIGGER_PD_MSG)
                    {
                        (void)PD_TimerClear(pdInstance, tSenderResponseTimer);
                        /* SourceCapabilities and VendorDefined are handled in the global section. */
                        if ((triggerInfo.pdMsgSop == kPD_MsgSOP) && ((triggerInfo.pdMsgType == kPD_MsgReject) ||
                                                                     (triggerInfo.pdMsgType == kPD_MsgNotSupported)))
                        {
                            pdInstance->psmNewState = PSM_PE_SRC_READY;
                            pd_command_result_t commandResultCallback;

                            if (triggerInfo.pdMsgType == kPD_MsgReject)
                            {
                                commandResultCallback = kCommandResult_Reject;
                            }
                            else
                            {
                                commandResultCallback = kCommandResult_NotSupported;
                            }
                            (void)PD_DpmAppCallback(pdInstance, PD_DPM_GET_PARTNER_SRC_CAP_FAIL, &commandResultCallback,
                                                    1);
                        }
                        else
                        {
                            pdInstance->psmNewState = PSM_SEND_SOFT_RESET;
                        }
                    }
                    else if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tSenderResponseTimer) != 0U)
                    {
                        pdInstance->psmNewState = PSM_PE_SRC_READY;
                    }
                    else
                    {
                        didNothingStepB = 1;
                    }
                    break;
                }

                case PSM_PE_PRS_SRC_SNK_TRANSITION_TO_OFF: /* (B) */
                {
                    /* check source is standby */
                    if (PD_PsmCheckVsafe0V(pdInstance) != 0U)
                    {
                        /* NOTE : DPM will actively discharge VBUS, and use a timer to ensure at least a min discharge
                         * time
                         */
                        /* have enter standby */
                        PD_DpmDischargeVbus(pdInstance, 0);
                        pdInstance->psmNewState = PSM_PE_PRS_SRC_SNK_ASSERT_RD;
                    }
                    else
                    {
                        didNothingStepB = 0; /* need check vbus constantly */
                    }
                    break;
                }
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
                case PE_FRS_SRC_SNK_Transition_to_off: /* B */
                {
                    /* wait vbus reach vSafe5V */
                    if (PD_TimerCheckInvalidOrTimeOut(pdInstance, timrFRSwapWaitPowerStable) != 0U)
                    {
                        if (PD_PsmCheckVsafe5V(pdInstance) != 0U)
                        {
                            pdInstance->psmNewState = PE_FRS_SRC_SNK_Assert_Rd;
                        }
                    }
                    else
                    {
                        /* need check vbus constantly */
                        didNothingStepB = 0;
                    }
                    break;
                }
#endif
                case PSM_PE_PRS_SNK_SRC_TRANSITION_TO_OFF: /* (B) */
                    PD_PsmPowerSwapSinkSourceTransitionOff(pdInstance, &triggerInfo, PSM_PE_PRS_SNK_SRC_ASSERT_RP,
                                                           &didNothingStepB);
                    break;
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
                case PE_FRS_SNK_SRC_Transition_to_off: /* B */
                    PD_PsmCheckFRS5V(pdInstance);
                    PD_PsmPowerSwapSinkSourceTransitionOff(pdInstance, &triggerInfo, PE_FRS_SNK_SRC_Vbus_Applied,
                                                           &didNothingStepB);
                    break;
#endif

                case PSM_PE_PRS_SRC_SNK_WAIT_SOURCE_ON: /* (B) */
#if (defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
                case PE_FRS_SRC_SNK_Wait_Source_on: /* B */
#endif
                {
                    if (triggerInfo.triggerEvent == PSM_TRIGGER_PD_MSG)
                    {
                        if ((triggerInfo.pdMsgSop == kPD_MsgSOP) && (triggerInfo.pdMsgType == kPD_MsgPsRdy))
                        {
                            pd_vbus_power_progress_t vbusPowerState;
                            pd_dpm_callback_event_t callbackEvent;

#if 0
                            /* SIP code doesn't do this */
                            PD_FRSControl(pdInstance, 1);
#endif
                            PD_MsgStopReceive(pdInstance);
                            (void)PD_TimerClear(pdInstance, tPSSourceOnTimer);
                            /* Swap from SOURCE to SINK */
                            /* pr swap end */
                            PD_ConnectSetPowerProgress(pdInstance, kVbusPower_Stable);
#if defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
                            /* Cable plug will need a soft reset */
                            pdInstance->psmCablePlugResetNeeded = 1;
#endif

#if (defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
                            if (pdInstance->psmCurState == PE_FRS_SRC_SNK_Wait_Source_on)
                            {
                                vbusPowerState = kVbusPower_InFRSwap;
                            }
                            else
#endif
                            {
                                vbusPowerState = kVbusPower_InPRSwap;
                            }
                            (void)pdInstance->callbackFns->PD_SnkDrawTypeCVbus(
                                pdInstance->callbackParam, (uint8_t)PD_PsmGetTypeCCurrent(pdInstance), vbusPowerState);
#if (defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
                            if (pdInstance->psmCurState == PE_FRS_SRC_SNK_Wait_Source_on)
                            {
                                callbackEvent = PD_DPM_FR_SWAP_SUCCESS;
                            }
                            else
#endif
                            {
                                callbackEvent = PD_DPM_PR_SWAP_SUCCESS;
                            }
                            (void)PD_DpmAppCallback(pdInstance, callbackEvent, NULL, 1);

                            pdInstance->psmNewState = PSM_PE_SNK_STARTUP;
                        }
                        else
                        {
                            /* A protocol error during power role swap triggers a Hard Reset */
                            pdInstance->psmNewState = PSM_HARD_RESET;
                        }
                    }
                    else if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tPSSourceOnTimer) != 0U)
                    {
                        pdInstance->psmNewState = PSM_EXIT_TO_ERROR_RECOVERY;
                    }
                    else
                    {
                        didNothingStepB = 1;
                    }
                    break;
                }

                case PSM_PE_PRS_SRC_SNK_SEND_PR_SWAP: /* (B) */
                case PSM_PE_PRS_SNK_SRC_SEND_PR_SWAP: /* (B) */
                    PD_PsmStateWaitReply(pdInstance, &triggerInfo, &didNothingStepB);
                    break;
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
                case PE_FRS_SNK_SRC_Send_Swap: /* B */
                {
                    PD_PsmCheckFRS5V(pdInstance);
                    if (triggerInfo.triggerEvent == PSM_TRIGGER_PD_MSG)
                    {
                        if (triggerInfo.pdMsgType == kPD_MsgAccept)
                        {
                            pdInstance->psmNewState = PE_FRS_SNK_SRC_Transition_to_off;
                        }
                        else
                        {
                            pdInstance->psmNewState = PSM_EXIT_TO_ERROR_RECOVERY_WITH_DELAY;
                        }
                    }
                    else if (PD_TimerCheckValidTimeOut(pdInstance, tSenderResponseTimer) != 0U)
                    {
                        pdInstance->psmNewState = PSM_EXIT_TO_ERROR_RECOVERY;
                    }
                    else
                    {
                        didNothingStepB = 1;
                    }
                    break;
                }
#endif

                case PSM_PE_PRS_SNK_SRC_SOURCE_ON: /* (B) */
                {
                    if (PD_PsmCheckVsafe5V(pdInstance) != 0U)
                    {
                        PD_PsmPowerSwapSinkOpenVbus(pdInstance, PD_DPM_PR_SWAP_SUCCESS);
                    }
                    else
                    {
                        didNothingStepB = 1;
                    }
                    break;
                }

                case PSM_PE_PRS_EVALUATE_PR_SWAP_WAIT_TIMER_TIME_OUT: /* (B) */
                {
                    if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tPrSwapWaitTimer) != 0U)
                    {
                        if (pdInstance->curPowerRole == kPD_PowerRoleSource)
                        {
                            pdInstance->psmNewState = PSM_PE_PRS_SRC_SNK_SEND_PR_SWAP;
                        }
                        else
                        {
                            pdInstance->psmNewState = PSM_PE_PRS_SNK_SRC_SEND_PR_SWAP;
                        }
                    }
                    break;
                }

                case PSM_PE_DR_SNK_GET_SINK_CAP: /* (B) */
                case PSM_PE_SRC_GET_SINK_CAP:    /* (B) */
                {
                    if (triggerInfo.triggerEvent == PSM_TRIGGER_PD_MSG)
                    {
                        pdInstance->psmNewState = PE_PSM_STATE_ROLE_RDY_STATE;

                        /* SourceCapabilities and VendorDefined are handled in the global section. */
                        (void)PD_TimerClear(pdInstance, tSenderResponseTimer);
                        if (triggerInfo.pdMsgType == kPD_MsgSinkCapabilities)
                        {
                            pd_capabilities_t sinkCapa;
                            /* Clear any pending message now that we have the latest */
                            (void)PD_DpmClearMsg(pdInstance, PD_DPM_CONTROL_GET_PARTNER_SINK_CAPABILITIES);
                            /* Update the PDOs for the EC and send interrupt */
                            sinkCapa.capabilities      = triggerInfo.pdMsgDataBuffer;
                            sinkCapa.capabilitiesCount = triggerInfo.pdMsgDataLength;
                            (void)PD_DpmAppCallback(pdInstance, PD_DPM_GET_PARTNER_SNK_CAP_SUCCESS, &sinkCapa, 1);
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
#if (defined PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
                            pdInstance->portPartnerSinkPDO1.PDOValue = sinkCapa.capabilities[0];
#endif
#endif
                        }
                        else if ((triggerInfo.pdMsgType == kPD_MsgReject) ||
                                 (triggerInfo.pdMsgType == kPD_MsgNotSupported))
                        {
                            pd_command_result_t commandResultCallback;
                            if (triggerInfo.pdMsgType == kPD_MsgReject)
                            {
                                commandResultCallback = kCommandResult_Reject;
                            }
                            else
                            {
                                commandResultCallback = kCommandResult_NotSupported;
                            }
                            (void)PD_DpmAppCallback(pdInstance, PD_DPM_GET_PARTNER_SNK_CAP_FAIL, &commandResultCallback,
                                                    1);
                        }
                        else
                        {
                            pdInstance->psmNewState = PSM_SEND_SOFT_RESET;
                        }
                    }
                    else if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tSenderResponseTimer) != 0U)
                    {
                        pdInstance->psmNewState = PE_PSM_STATE_ROLE_RDY_STATE;
                    }
                    else
                    {
                        didNothingStepB = 1;
                    }
                    break;
                }
#endif

#if defined(PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
                case PSM_PE_DRS_SEND_DR_SWAP: /* (B) */
                    PD_PsmStateWaitReply(pdInstance, &triggerInfo, &didNothingStepB);
                    break;

                case PSM_PE_DRS_EVALUATE_DR_SWAP_WAIT_TIMER_TIME_OUT: /* (B) */
                    if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tDrSwapWaitTimer) != 0U)
                    {
                        pdInstance->psmNewState = PSM_PE_DRS_SEND_DR_SWAP;
                    }
                    break;
#endif

#if defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
                case PSM_PE_VCS_SEND_SWAP: /* (B) */
                    PD_PsmStateWaitReply(pdInstance, &triggerInfo, &didNothingStepB);
                    break;

                case PSM_PE_VCS_WAIT_FOR_VCONN: /* (B) */
                {
                    if (triggerInfo.triggerEvent == PSM_TRIGGER_PD_MSG)
                    {
                        /* SourceCapabilities and VendorDefined are handled in the global section. */
                        (void)PD_TimerClear(pdInstance, tVconnOnTimer);
                        if ((triggerInfo.pdMsgSop == kPD_MsgSOP) && (triggerInfo.pdMsgType == kPD_MsgPsRdy))
                        {
                            pdInstance->psmNewState = PSM_PE_VCS_TURN_OFF_VCONN;
                        }
                        else
                        {
                            pdInstance->psmNewState = PSM_SEND_SOFT_RESET;
                        }
                    }
                    else if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tVconnOnTimer) != 0U)
                    {
                        pdInstance->psmNewState = PSM_HARD_RESET;
                    }
                    else
                    {
                        didNothingStepB = 1;
                    }
                    break;
                }

                case PSM_PE_VCS_TURN_ON_VCONN: /* (B) */
                {
                    if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tVconnOnTimer) != 0U)
                    {
                        pdInstance->psmNewState = PSM_PE_VCS_SEND_PS_RDY;
                    }
                    else
                    {
                        didNothingStepB = 1;
                    }
                    break;
                }
#endif

#if (defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
                case PE_DR_SRC_GET_SOURCE_CAP_EXT: /* B */
                case PE_SNK_GET_SOURCE_CAP_EXT:    /* B */
                    PD_StateWaitReplyExtDataProcess(pdInstance, &triggerInfo, kPD_MsgSourceCapExtended,
                                                    PD_DPM_GET_SRC_EXT_CAP_SUCCESS, PD_DPM_GET_SRC_EXT_CAP_FAIL,
                                                    &didNothingStepB);
                    break;

                case PE_SRC_Get_Sink_Status:   /* B */
                case PE_SNK_Get_Source_Status: /* B */
                    PD_StateWaitReplyExtDataProcess(pdInstance, &triggerInfo, kPD_MsgStatus, PD_DPM_GET_STATUS_SUCCESS,
                                                    PD_DPM_GET_STATUS_FAIL, &didNothingStepB);
                    break;

#if defined(PD_CONFIG_PD3_PPS_ENABLE) && (PD_CONFIG_PD3_PPS_ENABLE)
                case PE_SNK_Get_PPS_Status: /* B */
                    PD_StateWaitReplyExtDataProcess(pdInstance, &triggerInfo, kPD_MsgPpsStatus,
                                                    PD_DPM_GET_PPS_STATUS_SUCCESS, PD_DPM_GET_PPS_STATUS_FAIL,
                                                    &didNothingStepB);
                    break;
#endif

                case PE_Get_Battery_Cap: /* B */
                    PD_StateWaitReplyExtDataProcess(pdInstance, &triggerInfo, kPD_MsgBatteryCapabilities,
                                                    PD_DPM_GET_BATTERY_CAP_SUCCESS, PD_DPM_GET_BATTERY_CAP_FAIL,
                                                    &didNothingStepB);
                    break;

                case PE_Get_Battery_Status: /* B */
                    PD_StateWaitReplyExtDataProcess(pdInstance, &triggerInfo, kPD_MsgBatteryStatus,
                                                    PD_DPM_GET_BATTERY_STATUS_SUCCESS, PD_DPM_GET_BATTERY_STATUS_FAIL,
                                                    &didNothingStepB);
                    break;

                case PE_Get_Manufacturer_Info: /* B */
                    PD_StateWaitReplyExtDataProcess(pdInstance, &triggerInfo, kPD_MsgManufacturerInfo,
                                                    PD_DPM_GET_MANUFACTURER_INFO_SUCCESS,
                                                    PD_DPM_GET_MANUFACTURER_INFO_FAIL, &didNothingStepB);
                    break;
#endif

#if (defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
                case PE_FRS_SRC_SNK_CC_Signal: /* B */
                    didNothingStepB = 1;
                    if (((triggerInfo.triggerEvent == PSM_TRIGGER_PD_MSG) &&
                         (triggerInfo.msgHeader.bitFields.NumOfDataObjs == 0U) &&
                         (triggerInfo.msgHeader.bitFields.messageType == (uint8_t)kPD_MsgFrSwap)) ||
                        (pdInstance->frSwapReceived != 0U))
                    {
                        PD_ConnectSetPowerProgress(pdInstance, kVbusPower_InFRSwap);
                        pdInstance->frSwapReceived = 0;
                        pdInstance->psmNewState    = PE_FRS_SRC_SNK_Evaluate_Swap;
                        didNothingStepB            = 0;
                    }
                    else if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tFRSwapSignalTimer) != 0U)
                    {
                        /* In normal situation, this system cannot execute to here because the system has no power now
                         */
                        PD_ConnectSetPowerProgress(pdInstance, kVbusPower_Stable);
                    }
                    else
                    {
                        /* No action required. */
                    }
                    break;

                case PE_FRS_SNK_SRC_Vbus_Applied: /* B */
                {
                    uint8_t checkVal;
                    PD_PsmCheckFRS5V(pdInstance);
                    (void)PD_PhyControl(pdInstance, PD_PHY_FR_SWAP_CHECK_VBUS_APPLIED, &checkVal);
                    if ((pdInstance->fr5VOpened != 0U) || (checkVal != 0U))
                    {
                        pdInstance->fr5VOpened  = 1;
                        pdInstance->psmNewState = PE_FRS_SNK_SRC_Assert_Rp;
                    }
                    else
                    {
                        /* need check vbus constantly */
                        didNothingStepB = 0;
                    }
                    break;
                }
#endif
                case PSM_PE_BIST_TEST_DATA_MODE: /* (B) */
                    didNothingStepB = 1;
                    break;

                case PSM_PE_BIST_CARRIER_MODE_2: /* (B) */
#if (defined PD_CONFIG_COMPLIANCE_TEST_ENABLE) && (PD_CONFIG_COMPLIANCE_TEST_ENABLE)
                    if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tBISTContModeTimer) != 0U)
                    {
                        (void)PD_PhyControl(pdInstance, PD_PHY_RESET_MSG_FUNCTION, NULL);
                        if (pdInstance->curPowerRole == kPD_PowerRoleSource)
                        {
                            pdInstance->psmNewState = PSM_PE_SRC_TRANSITION_TO_DEFAULT;
                        }
                        if (pdInstance->curPowerRole == kPD_PowerRoleSink)
                        {
                            pdInstance->psmNewState = PSM_PE_SNK_TRANSITION_TO_DEFAULT;
                        }
                        /* (void)PD_PhyControl(pdInstance, PD_PHY_EXIT_BIST, NULL); */
                    }
                    else
                    {
                        /* We do nothing here, it's all done by the hardware until the device is power cycled. */
                        didNothingStepB = 1;
                    }
#endif
                    break;

#if 0
                case PE_Send_Security_Response: /* (B) */
                {
                    if (PD_PsmChunkingLayerTXStateMachine(pdInstance,
                                                          kPD_MsgSOP, kPD_MsgSecurityResponse,
                                                          pdInstance->commandExtParam.dataBuffer,
                                                          pdInstance->commandExtParam.dataLength, &triggerInfo)
                        != kStatus_PD_Success)
                    {
                        PD_PsmTransitionOnMsgSendError(pdInstance, PSM_CHECK_ASYNC_RX, PSM_SEND_SOFT_RESET);
                    }
                    else
                    {
                        if (PD_PsmChunkingLayerCheckSentDone(pdInstance))
                        {
                            pdInstance->psmNewState = PE_PSM_STATE_ROLE_RDY_STATE;
                        }
                        else
                        {
                            /* wait chunk message sent done */
                        }
                    }
                    break;
                }
#endif

                default:
                    didNothingStepB = 1;
                    break;
            }
        }
        else
        {
            /* No action required. */
        }

        if (returnState != kSM_None)
        {
            return returnState;
        }
        if (triggerInfo.triggerEvent == PSM_TRIGGER_DPM_MSG)
        {
            PD_PsmCommandFail(pdInstance, triggerInfo.dpmMsg);
        }
        /* put this type exception process codes here, then the normal process codes are simple */
        PD_PsmCheckUnexpectedReceivedMsg(pdInstance, triggerInfo.triggerEvent, &didNothingStepB);
    }

    if ((msgReceived != 0U) && (PD_PsmCanPendingReceive(pdInstance) != 0U))
    {
        PD_MsgReceive(pdInstance); /* msg has been processed, receiving next msg */
    }

    PD_PsmProcessImportEventBeforeNextStateMachine(pdInstance);
    if ((didNothingStepB == 0U) || (didNothingStepA == 0U))
    {
        return kSM_Continue;
    }

    return kSM_WaitEvent;
}

/*! ***************************************************************************
   VMD process related functions
******************************************************************************/
#if ((defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)) || \
    (defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE))
static void PD_PsmEndVdmCommand(pd_instance_t *pdInstance, pd_psm_state_t secondState)
{
    if (secondState != PSM_IDLE)
    {
        return;
    }

    if (pdInstance->commandProcessing <= PD_DPM_ALERT)
    {
        return;
    }
    else
    {
        PD_PsmCommandFail(pdInstance, pdInstance->commandProcessing);
    }
}
#endif

#if (defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)

static pd_psm_state_t PD_PsmVdmCheckkVDMBusyTimer(pd_instance_t *pdInstance, pd_psm_state_t newState)
{
    if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tVDMBusyTimer) != 0U)
    {
        return newState;
    }
    else
    {
        pdInstance->psmVDMBusyWaitDpmMsg = newState;
        return PSM_PE_DFP_UFP_VDM_VDM_BUSY_WAIT;
    }
}

static pd_psm_state_t PD_PsmVdmIdleProcessDpmMessage(pd_instance_t *pdInstance,
                                                     start_of_packet_t statIndex,
                                                     psm_trigger_info_t *triggerInfo)
{
    pd_psm_state_t secondNewState = PSM_UNKNOWN;

    if (triggerInfo->dpmMsg >= PD_DPM_CONTROL_DISCOVERY_IDENTITY)
    {
        /* special process */
        if (pdInstance->structuredVdmCommandParameter.vdmSop != (uint8_t)statIndex)
        {
            return PSM_UNKNOWN;
        }
        switch (triggerInfo->dpmMsg)
        {
            case PD_DPM_CONTROL_CABLE_RESET:
                if (statIndex == kPD_MsgSOP)
                {
                    return PSM_UNKNOWN;
                }
                break;

            default:
                /* No action required. */
                break;
        }

        /* common process */
        triggerInfo->triggerEvent = PSM_TRIGGER_NONE;
        if (PD_PsmStartCommand(pdInstance, triggerInfo->dpmMsg, 1) == 0U)
        {
            PD_PsmCommandFail(pdInstance, triggerInfo->dpmMsg);
            return PSM_IDLE;
        }
        switch (triggerInfo->dpmMsg)
        {
            case PD_DPM_CONTROL_CABLE_RESET:
            {
                secondNewState = PSM_PE_DFP_CBL_SEND_CABLE_RESET;
                break;
            }
            case PD_DPM_CONTROL_DISCOVERY_IDENTITY:
                secondNewState = PD_PsmVdmCheckkVDMBusyTimer(pdInstance, PSM_PE_DFP_UFP_VDM_IDENTITY_REQUEST);
                break;
            case PD_DPM_CONTROL_DISCOVERY_SVIDS:
                secondNewState = PD_PsmVdmCheckkVDMBusyTimer(pdInstance, PSM_PE_DFP_VDM_SVIDS_REQUEST);
                break;
            case PD_DPM_CONTROL_DISCOVERY_MODES:
                secondNewState = PD_PsmVdmCheckkVDMBusyTimer(pdInstance, PSM_PE_DFP_VDM_MODES_REQUEST);
                break;
            case PD_DPM_CONTROL_ENTER_MODE:
            {
                secondNewState = PSM_PE_DFP_VDM_MODE_ENTRY_REQUEST;
                break;
            }
            case PD_DPM_CONTROL_EXIT_MODE:
            {
                secondNewState = PSM_PE_DFP_VDM_MODE_EXIT_REQUEST;
                break;
            }
            case PD_DPM_CONTROL_SEND_ATTENTION:
            {
                secondNewState = PSM_PE_DFP_VDM_ATTENTION_REQUEST;
                break;
            }
            case PD_DPM_SEND_VENDOR_STRUCTURED_VDM:
            {
                secondNewState = PSM_PE_VENDOR_STRUCTURED_VDM_REQUEST;
                break;
            }
            case PD_DPM_SEND_UNSTRUCTURED_VDM:
            {
                secondNewState = PSM_PD_SEND_UNSTRUCTURED_VDM;
                break;
            }

            default:
                /* No action required. */
                break;
        }
    }

    return secondNewState;
}

#if 0
static uint8_t PDPsmVdmEnterExitModePdMsgProcess(pd_instance_t *pdInstance,
                                                 uint8_t statIndex,
                                                 psm_trigger_info_t *triggerInfo,
                                                 pd_svdm_command_request_t *commandVdmRequest)
{
    if (pdInstance->curDataRole == kPD_DataRoleDFP)
    {
        pd_structured_vdm_header_t reponseVdmHeader;
        reponseVdmHeader.structuredVdmHeaderVal = triggerInfo->vdmHeader.structuredVdmHeaderVal;
        /* not supported */
        reponseVdmHeader.bitFields.commandType = kVDM_ResponderNAK;
        (void)PD_MsgSendStructuredVDM(pdInstance, (start_of_packet_t)statIndex, reponseVdmHeader, 0, NULL);
    }
    else
    {
        commandVdmRequest->vdoSop                           = (uint8_t)triggerInfo->pdMsgSop;
        commandVdmRequest->vdmHeader.structuredVdmHeaderVal = triggerInfo->vdmHeader.structuredVdmHeaderVal;
        if (triggerInfo->vdmHeader.bitFields.command == kVDM_EnterMode)
        {
            if (triggerInfo->msgHeader.bitFields.NumOfDataObjs >= 1)
            {
                commandVdmRequest->vdoCount = 1;
                commandVdmRequest->vdoData  = triggerInfo->pdMsgDataBuffer + 1U;
            }
        }
        if (((statIndex == 0) && (pdInstance->curDataRole == kPD_DataRoleUFP)) || (statIndex >= 1))
        {
            (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_REQUEST, commandVdmRequest, 0);
            return 1;
        }
    }
    return 0;
}
#endif

static void PD_PsmVdmIdleProcessPdMessage(pd_instance_t *pdInstance,
                                          start_of_packet_t statIndex,
                                          psm_trigger_info_t *triggerInfo)
{
    /* note: vdm message will be processed before this */
    if (triggerInfo->pdMsgType == kPD_MsgVendorDefined)
    {
        if (triggerInfo->pdMsgSop != statIndex)
        {
            return;
        }

        triggerInfo->triggerEvent = PSM_TRIGGER_NONE;

        /* structured vdm */
        if (triggerInfo->vdmHeader.bitFields.vdmType != 0U)
        {
            if (triggerInfo->vdmHeader.bitFields.commandType == (uint8_t)kVDM_Initiator)
            {
                pd_svdm_command_request_t commandVdmRequest;
                uint8_t needReply = 0;

                if ((triggerInfo->vdmHeader.bitFields.command >= (uint8_t)kVDM_DiscoverIdentity) &&
                    (triggerInfo->vdmHeader.bitFields.command <= (uint8_t)kVDM_ExitMode) &&
                    (pdInstance->curDataRole == kPD_DataRoleDFP))
                {
                    /* PD2.0 spec: When the UUT working as DFP receives Identity, SVID, Mode, Enter Mode and Exit Mode,
                       the UUT must ignore these message. */
                    if (pdInstance->revision < PD_SPEC_REVISION_30)
                    {
                        if ((triggerInfo->vdmHeader.bitFields.SVID != PD_STANDARD_ID) &&
                            ((triggerInfo->vdmHeader.bitFields.command == (uint8_t)kVDM_DiscoverIdentity) ||
                             (triggerInfo->vdmHeader.bitFields.command == (uint8_t)kVDM_DiscoverSVIDs)))
                        {
                            /* Ellisys compliance test TEST.PD.VDM.SRC.2, need reply NAK */
                        }
                        else
                        {
                            return; /* ignore these commands */
                        }
                    }
                    else
                    {
                        /* PD3.0 spec: When the UUT working as DFP receives Enter Mode and Exit Mode,
                           the UUT must reply with Not Supported. */
                        if ((triggerInfo->vdmHeader.bitFields.command == (uint8_t)kVDM_EnterMode) ||
                            (triggerInfo->vdmHeader.bitFields.command == (uint8_t)kVDM_ExitMode))
                        {
                            (void)PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(
                                pdInstance, PD_NOT_SUPPORT_REPLY_MSG, pdInstance->psmCurState);
                            return;
                        }
                    }
                }

                commandVdmRequest.requestResultStatus              = (uint8_t)kCommandResult_NotSupported;
                commandVdmRequest.vdoSop                           = (uint8_t)triggerInfo->pdMsgSop;
                commandVdmRequest.vdmHeader.structuredVdmHeaderVal = triggerInfo->vdmHeader.structuredVdmHeaderVal;

                /* process the message and get the reply data */
                /* special process */
                switch ((pd_vdm_command_t)triggerInfo->vdmHeader.bitFields.command)
                {
                    case kVDM_DiscoverIdentity:
                        needReply = 1;
                        if (triggerInfo->vdmHeader.bitFields.SVID == PD_STANDARD_ID)
                        {
                            commandVdmRequest.vdoData  = NULL;
                            commandVdmRequest.vdoCount = 0;
                            (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_REQUEST, &commandVdmRequest, 0);

                            if ((commandVdmRequest.vdoData != NULL) && (pdInstance->revision < PD_SPEC_REVISION_30))
                            {
                                pd_id_header_vdo_t *idHeader =
                                    (pd_id_header_vdo_t *)(void *)&pdInstance->sendingData[2];

                                (void)memcpy((void *)idHeader, (void *)commandVdmRequest.vdoData,
                                             (uint32_t)commandVdmRequest.vdoCount << 2U);

                                /* Ellisys compliance test TEST.PD.VDM.SNK.1, For PD2.0, ID Header Bits 25-16 shall be
                                   all 0 (Reserved) (TEST.PD.VDM.SNK.1#3) */
                                idHeader->bitFields.connectorType  = 0U;
                                idHeader->bitFields.productTypeDFP = 0U;

                                commandVdmRequest.vdoData = (uint32_t *)(void *)idHeader;

                                /* Ellisys compliance test TD.PD.VDMU.E1. For VDM1.0, if the Product Type declared in
                                   the ID Header (VDO #2) is Hub (1) or Peripheral (2) and the Number of Data Objects
                                   must be 4. */
                                if (commandVdmRequest.vdoCount > 3U)
                                {
                                    if ((idHeader->bitFields.productTypeUFPOrCablePlug >
                                         (uint8_t)kProductTypeUfp_AMA) ||
                                        (idHeader->bitFields.productTypeUFPOrCablePlug <
                                         (uint8_t)kProductTypeCable_PassiveCable))
                                    {
                                        commandVdmRequest.vdoCount = 3U;
                                    }
                                }
                            }
                        }
                        else
                        {
                            commandVdmRequest.requestResultStatus = (uint8_t)kCommandResult_VDMNAK;
                        }
                        break;

                    case kVDM_DiscoverSVIDs:
                        needReply = 1;
                        if (triggerInfo->vdmHeader.bitFields.SVID == PD_STANDARD_ID)
                        {
                            (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_REQUEST, &commandVdmRequest, 0);
                        }
                        else
                        {
                            commandVdmRequest.requestResultStatus = (uint8_t)kCommandResult_VDMNAK;
                        }
                        break;

                    case kVDM_DiscoverModes:
                        needReply = 1;
                        (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_REQUEST, &commandVdmRequest, 0);
                        break;

                    case kVDM_EnterMode:
                        if (triggerInfo->msgHeader.bitFields.NumOfDataObjs >= 1U)
                        {
                            commandVdmRequest.vdoCount = 1;
                            commandVdmRequest.vdoData  = triggerInfo->pdMsgDataBuffer + 1U;
                        }
                        needReply = 1;
                        (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_REQUEST, &commandVdmRequest, 0);
                        break;

                    case kVDM_ExitMode:
                        pdInstance->psmVdmActiveModeValidMask &=
                            ~((uint8_t)0x01U << triggerInfo->vdmHeader.bitFields.objPos);
                        pdInstance->vdmExitReceived[statIndex] = 0;
                        needReply                              = 1;
                        (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_REQUEST, &commandVdmRequest, 0);

#if (defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT)
                        /* UFP issues DR Swap to trigger Exit mode(Attention)? */
                        if ((pdInstance->drSwapTriggerExitMode != 0U) && (pdInstance->psmVdmActiveModeValidMask == 0U))
                        {
                            /* After exiting active mode, need to trigger DR Swap again. */
                            pdInstance->drSwapTriggerExitMode = 0;
                            (void)PD_Command(pdInstance, (uint32_t)PD_DPM_CONTROL_DR_SWAP, NULL);
                        }
#endif
                        break;

                    case kVDM_Attention:
                        if (triggerInfo->msgHeader.bitFields.NumOfDataObjs >= 1U)
                        {
                            commandVdmRequest.vdoCount = 1;
                            commandVdmRequest.vdoData  = triggerInfo->pdMsgDataBuffer + 1U;
                        }
                        needReply = 0;
                        (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_REQUEST, &commandVdmRequest, 0);
                        break;

                    default:
                    {
                        /* vendor structured vdm */
                        if (triggerInfo->vdmHeader.bitFields.command >= 16U)
                        {
                            needReply                  = 1;
                            commandVdmRequest.vdoCount = (uint8_t)(triggerInfo->pdMsgDataLength - 1U);
                            commandVdmRequest.vdmHeader.structuredVdmHeaderVal =
                                USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)triggerInfo->pdMsgDataBuffer));
                            commandVdmRequest.vdoData = triggerInfo->pdMsgDataBuffer + 1U;
                            (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_REQUEST, &commandVdmRequest, 0);
                        }
                        break;
                    }
                }

                /* reply response mseeage */
                if (needReply != 0U)
                {
                    pd_structured_vdm_header_t reponseVdmHeader;
                    reponseVdmHeader.structuredVdmHeaderVal = triggerInfo->vdmHeader.structuredVdmHeaderVal;
                    if (reponseVdmHeader.bitFields.vdmVersion > PD_CONFIG_STRUCTURED_VDM_VERSION)
                    {
                        if (pdInstance->revision < PD_SPEC_REVISION_30)
                        {
                            reponseVdmHeader.bitFields.vdmVersion = PD_SPEC_STRUCTURED_VDM_VERSION_10;
                        }
                        else
                        {
                            reponseVdmHeader.bitFields.vdmVersion = PD_CONFIG_STRUCTURED_VDM_VERSION;
                        }
                    }

                    if (commandVdmRequest.requestResultStatus == (uint8_t)kCommandResult_NotSupported)
                    {
                        /* PD2.0 spec: If Structured VDMs are not supported,
                         * a Structured VDM Command received by a DFP or UFP
                         * Shall be ignored */
                        if (pdInstance->revision < PD_SPEC_REVISION_30)
                        {
                            return; /* ignore these commands */
                        }
                        else
                        {
                            /* PD3.0 spec: If Structured VDMs are not supported,
                             * the DFP or UFP receiving a VDM Command Shall send
                             * a Not_Supported Message in response */
                            (void)PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(
                                pdInstance, PD_NOT_SUPPORT_REPLY_MSG, pdInstance->psmCurState);
                            return;
                        }
                    }
                    else if (commandVdmRequest.requestResultStatus == (uint8_t)kCommandResult_VDMACK)
                    {
                        reponseVdmHeader.bitFields.commandType = (uint32_t)kVDM_ResponderACK;
                        if (PD_MsgSendStructuredVDM(pdInstance, (start_of_packet_t)statIndex, reponseVdmHeader,
                                                    commandVdmRequest.vdoCount,
                                                    (uint32_t *)commandVdmRequest.vdoData) == kStatus_PD_Success)
                        {
                            if (triggerInfo->vdmHeader.bitFields.command == (uint8_t)kVDM_EnterMode)
                            {
                                pdInstance->psmVdmActiveModeValidMask |=
                                    ((uint8_t)0x01U << triggerInfo->vdmHeader.bitFields.objPos);
                            }
                        }
                    }
                    else
                    {
                        switch ((pd_vdm_command_t)triggerInfo->vdmHeader.bitFields.command)
                        {
                            case kVDM_EnterMode:
                            case kVDM_ExitMode:
                                reponseVdmHeader.bitFields.commandType = (uint8_t)kVDM_ResponderNAK;
                                break;

                            case kVDM_DiscoverIdentity:
                            case kVDM_DiscoverSVIDs:
                            case kVDM_DiscoverModes:
                            default:
                                if (commandVdmRequest.requestResultStatus == (uint8_t)kCommandResult_VDMNAK)
                                {
                                    reponseVdmHeader.bitFields.commandType = (uint8_t)kVDM_ResponderNAK;
                                }
                                else
                                {
                                    reponseVdmHeader.bitFields.commandType = (uint8_t)kVDM_ResponderBUSY;
                                }
                                break;
                        }
                        (void)PD_MsgSendStructuredVDM(pdInstance, statIndex, reponseVdmHeader, 0, NULL);
                    }
                }
            }
            else
            {
                /* should not reach here */
            }
        }
        else
        {
            /* unstructured vdm */
            pd_unstructured_vdm_command_param_t unstructuredVDMParam;
            unstructuredVDMParam.vdmSop                = (uint8_t)triggerInfo->pdMsgSop;
            unstructuredVDMParam.vdmHeaderAndVDOsData  = triggerInfo->pdMsgDataBuffer;
            unstructuredVDMParam.vdmHeaderAndVDOsCount = triggerInfo->pdMsgDataLength;
            unstructuredVDMParam.resultStatus          = (uint8_t)kCommandResult_None;
            (void)PD_DpmAppCallback(pdInstance, PD_DPM_UNSTRUCTURED_VDM_RECEIVED, &unstructuredVDMParam, 0);

#if defined(PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30)
            if (unstructuredVDMParam.resultStatus == (uint8_t)kCommandResult_NotSupported)
            {
                if (pdInstance->revision >= PD_SPEC_REVISION_30)
                {
                    (void)PD_PsmSendControlTransitionWithAsyncRxAndSoftReset(pdInstance, kPD_MsgNotSupported,
                                                                             pdInstance->psmCurState);
                }
                else
                {
                    /* PD2.0 spec: ignore this message. */
                }
            }
#endif
        }
    }
}
#endif

#if (defined(PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)) || \
    (defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE))

/*! ***************************************************************************
   \brief Common Handler for structured VDM initiator requests
   \param reject_flags will perSopPartner[kPD_MsgSOP].flags on failure
   \retval PSM_IDLE on failure
   \retval org_state on success
   \note Only called from CLP task context
******************************************************************************/
static pd_psm_state_t PD_PsmStructuredVdmInitiatorRequest(pd_instance_t *pdInstance,
                                                          start_of_packet_t sopIndex,
                                                          uint16_t svid,
                                                          uint8_t position,
                                                          pd_vdm_command_t command,
                                                          uint8_t vdo_count,
                                                          uint32_t *vdos)
{
    pd_structured_vdm_header_t vdmHeader;

    vdmHeader.structuredVdmHeaderVal = 0;
    vdmHeader.bitFields.command      = (uint8_t)command;
    vdmHeader.bitFields.commandType  = (uint8_t)kVDM_Initiator;
    vdmHeader.bitFields.objPos       = position;
    vdmHeader.bitFields.SVID         = svid;
    vdmHeader.bitFields.vdmType      = 1;
#if defined(PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30)
    vdmHeader.bitFields.vdmVersion = (pdInstance->revision < PD_SPEC_REVISION_30) ? PD_SPEC_STRUCTURED_VDM_VERSION_10 :
                                                                                    PD_CONFIG_STRUCTURED_VDM_VERSION;
#else
    vdmHeader.bitFields.vdmVersion = PD_SPEC_STRUCTURED_VDM_VERSION_10;
#endif
    /* Send Discover Modes request */
    if (PD_MsgSendStructuredVDM(pdInstance, sopIndex, vdmHeader, vdo_count, vdos) == kStatus_PD_Success)
    {
        if (command == kVDM_EnterMode)
        {
            (void)PD_TimerStart(pdInstance, tVDMModeEntryTimer, T_VDM_WAIT_MODE_ENTRY);
        }
        else if (command == kVDM_ExitMode)
        {
            (void)PD_TimerStart(pdInstance, tVDMModeExitTimer, T_VDM_WAIT_MODE_EXIT);
        }
        else
        {
            (void)PD_TimerStart(pdInstance, tVDMResponseTimer, T_VDM_SENDER_RESPONSE);
        }
        /* Event driven -> wait for response packet or timeout */
        return pdInstance->psmSecondaryState[sopIndex];
    }
    else
    {
        return PSM_IDLE;
    }
}

static pd_psm_state_t PD_PsmVdmResponseHandler(pd_instance_t *pdInstance,
                                               start_of_packet_t sop,
                                               uint8_t vdmMsgType,
                                               psm_trigger_info_t *triggerInfo,
                                               tTimer_t timr,
                                               pd_psm_state_t ackState,
                                               pd_psm_state_t nakState)
{
    uint8_t expected           = 0;
    pd_psm_state_t newStateTmp = PSM_UNKNOWN;

    if ((triggerInfo->triggerEvent == PSM_TRIGGER_PD_MSG) && (triggerInfo->pdMsgType == kPD_MsgVendorDefined) &&
        (triggerInfo->vdmHeader.bitFields.command == vdmMsgType) &&
        (triggerInfo->vdmHeader.bitFields.commandType != (uint8_t)kVDM_Initiator) && (triggerInfo->pdMsgSop == sop))
    {
        if ((vdmMsgType == (uint8_t)kVDM_DiscoverIdentity) || (vdmMsgType == (uint8_t)kVDM_DiscoverSVIDs))
        {
            if (triggerInfo->vdmHeader.bitFields.SVID == PD_STANDARD_ID)
            {
                expected = 1;
            }
        }
        else
        {
            expected = 1;
        }
    }

    if (expected != 0U)
    {
        (void)PD_TimerClear(pdInstance, timr);
        triggerInfo->triggerEvent  = PSM_TRIGGER_NONE;
        pdInstance->amsVdmReplyMsg = (pd_vdm_command_type_t)triggerInfo->vdmHeader.bitFields.commandType;
        switch (pdInstance->amsVdmReplyMsg)
        {
            case kVDM_ResponderACK:
                newStateTmp = ackState;
                break;

            case kVDM_ResponderBUSY:
                newStateTmp = nakState;
                break;

            case kVDM_ResponderNAK:
                if (vdmMsgType == (uint8_t)kVDM_ExitMode)
                {
                    newStateTmp = ackState;
                }
                else
                {
                    newStateTmp = nakState;
                }
                break;

            default:
                newStateTmp = nakState;
                break;
        }
    }
    else if (PD_TimerCheckInvalidOrTimeOut(pdInstance, timr) != 0U)
    {
        pdInstance->amsVdmReplyMsg = kVDM_ComandTypeInvalid;
        newStateTmp                = nakState;
    }
    else
    {
        /* TODO: AMS interrupt process */
    }

    return newStateTmp;
}

/*! ***************************************************************************
   \brief Meet the requirement to terminate the msgSopP and kPD_MsgSOPp communications on
******************************************************************************/
#if defined(PD_CONFIG_SINK_ROLE_ENABLE) && (PD_CONFIG_SINK_ROLE_ENABLE)
static void PD_PsmSecondaryStateHandlerTerminate(pd_instance_t *pdInstance, start_of_packet_t sop)
{
    uint8_t i;

    if (sop == kPD_MsgSOPInvalid)
    {
        for (i = 0; i < 3U; i++)
        {
            pdInstance->psmNewSecondaryState[i] = PSM_IDLE;
        }
    }
    else
    {
        if (pdInstance->psmSecondaryState[sop] != PSM_IDLE)
        {
            pdInstance->psmNewSecondaryState[sop] = PSM_IDLE;
        }
    }
}
#endif

/*! ***************************************************************************
   \brief
   \note Only called from CLP task context
******************************************************************************/
static uint8_t PD_PsmSecondaryStateHandler(pd_instance_t *pdInstance,
                                           start_of_packet_t statIndex,
                                           psm_trigger_info_t *triggerInfo)
{
    uint8_t didNothingSecond      = 0;
    pd_psm_state_t secondNewState = PSM_UNKNOWN;
    uint8_t index;

    while (true)
    {
        /* global transition */
        if (triggerInfo->triggerEvent == PSM_TRIGGER_PD_MSG)
        {
            /* sop has high priority */
            if (statIndex != kPD_MsgSOP)
            {
                if (
#if defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
                    (pdInstance->psmNewSecondaryState[statIndex] != PSM_PE_DFP_CBL_SEND_CABLE_RESET) &&
                    (pdInstance->psmSecondaryState[statIndex] != PSM_PE_DFP_CBL_SEND_CABLE_RESET) &&
                    (pdInstance->psmSecondaryState[statIndex] != PSM_PE_DFP_CBL_SEND_SOFT_RESET) &&
#endif
                    ((pdInstance->psmNewSecondaryState[statIndex] != PSM_UNKNOWN) ||
                     (pdInstance->psmSecondaryState[statIndex] != PSM_IDLE)))
                {
                    /* During PD Connection (Explicit Contract): */
                    /* o The DFP can communicate with a Cable Plug, using SOP' Packets or SOP" Packets, at any time it
                     */
                    /* is not engaged in any other SOP Communications. */
                    /* o If SOP Packets are received by the DFP, during SOP' or SOP" Communication, the SOP' or SOP" */
                    /* Communication is immediately terminated (the Cable Plug times out and does not retry) */
                    /* o If the DFP needs to initiate an SOP Communication during an ongoing SOP' or SOP" Communication
                     */
                    /* (e.g.for a Capabilities change) then the SOP' or SOP" Communications will be interrupted. */

                    /* Check for pending message on SOP */
                    /* Primary state machine needs to handle kPD_MsgSOP */
                    if (triggerInfo->pdMsgSop == kPD_MsgSOP)
                    {
                        /* Abort the current action */
                        pdInstance->psmNewSecondaryState[statIndex] = PSM_IDLE;
                        didNothingSecond                            = 1;
                    }
                }
                /* Allow soft reset to also interrupt with priority over cable resets */
                if (triggerInfo->pdMsgType == kPD_MsgSoftReset)
                {
                    /* Abort the current action */
                    pdInstance->psmNewSecondaryState[statIndex] = PSM_IDLE;
                    didNothingSecond                            = 1;
                }

                if (didNothingSecond != 0U)
                {
                    return didNothingSecond;
                }
            }
        }

        /* global transition for dpm message */
        if ((triggerInfo->triggerEvent == PSM_TRIGGER_DPM_MSG) &&
            (triggerInfo->dpmMsg >= PD_DPM_CONTROL_DISCOVERY_IDENTITY))
        {
            if (((pdInstance->psmNewSecondaryState[statIndex] != PSM_UNKNOWN) ||
                 (pdInstance->psmSecondaryState[statIndex] != PSM_IDLE)) &&
                (pdInstance->structuredVdmCommandParameter.vdmSop == (uint8_t)statIndex))
            {
                triggerInfo->triggerEvent = PSM_TRIGGER_NONE;
                PD_PsmCommandFail(pdInstance, triggerInfo->dpmMsg);
            }
        }

        /* State C */
        if (pdInstance->psmNewSecondaryState[statIndex] != PSM_UNKNOWN)
        {
            pd_svdm_command_result_t commandVdmResult;

            pdInstance->psmSecondaryState[statIndex]    = pdInstance->psmNewSecondaryState[statIndex];
            pdInstance->psmNewSecondaryState[statIndex] = PSM_UNKNOWN;
            secondNewState                              = pdInstance->psmSecondaryState[statIndex];
            commandVdmResult.vdmHeader.structuredVdmHeaderVal =
                pdInstance->structuredVdmCommandParameter.vdmHeader.structuredVdmHeaderVal;
            commandVdmResult.vdoSop = (uint8_t)triggerInfo->pdMsgSop;

            switch (pdInstance->psmSecondaryState[statIndex])
            {
                case PSM_IDLE: /* C */
                    /* dpm message will callback */
                    break;

                case PSM_PE_SRC_IMPLICIT_CABLE_SOFT_RESET: /* C */
                    if (PD_MsgSend(pdInstance, kPD_MsgSOPp, kPD_MsgSoftReset, 2, NULL) == kStatus_PD_Success)
                    {
                        (void)PD_TimerStart(pdInstance, tSenderResponseTimer, T_SENDER_RESPONSE);
                    }
                    else
                    {
                        secondNewState = PSM_IDLE;
                    }
                    break;

                case PSM_PE_SRC_VDM_IDENTITY_REQUEST: /* C , only for sopp */
#if defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
                    pdInstance->cableDiscoverIdentityCounter++;
                    secondNewState = PD_PsmStructuredVdmInitiatorRequest(pdInstance, statIndex, PD_STANDARD_ID, 0,
                                                                         kVDM_DiscoverIdentity, 0, NULL);
#endif
                    break;

                case PSM_PE_DFP_VDM_SVIDS_REQUEST: /* C */
                    secondNewState = PD_PsmStructuredVdmInitiatorRequest(pdInstance, statIndex, PD_STANDARD_ID, 0,
                                                                         kVDM_DiscoverSVIDs, 0, NULL);
                    break;

                case PSM_PE_DFP_VDM_MODES_REQUEST: /* C */
                    secondNewState = PD_PsmStructuredVdmInitiatorRequest(
                        pdInstance, statIndex,
                        (uint16_t)pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.SVID, 0,
                        kVDM_DiscoverModes, 0, NULL);
                    break;

                case PSM_PE_DFP_VDM_MODE_ENTRY_REQUEST: /* C */
                    secondNewState = PD_PsmStructuredVdmInitiatorRequest(
                        pdInstance, statIndex,
                        (uint16_t)pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.SVID,
                        (uint8_t)pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.objPos, kVDM_EnterMode,
                        (pdInstance->structuredVdmCommandParameter.vdoCount != 0U) ? 1U : 0U,
                        (pdInstance->structuredVdmCommandParameter.vdoCount != 0U) ?
                            (pdInstance->structuredVdmCommandParameter.vdoData) :
                            NULL);
                    break;

                case PSM_PE_DFP_VDM_MODE_EXIT_REQUEST: /* C */
                    secondNewState = PD_PsmStructuredVdmInitiatorRequest(
                        pdInstance, statIndex,
                        (uint16_t)pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.SVID,
                        (uint8_t)pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.objPos, kVDM_ExitMode, 0,
                        NULL);
                    break;

                case PSM_PE_DFP_VDM_MODE_EXIT_HARD_RESET: /* C */
                    commandVdmResult.vdmCommand = (uint8_t)kVDM_ExitMode;
                    commandVdmResult.vdoData    = NULL;
                    commandVdmResult.vdoCount   = 0;
                    if (pdInstance->amsVdmReplyMsg == kVDM_ResponderBUSY)
                    {
                        commandVdmResult.vdmCommandResult = (uint8_t)kCommandResult_VDMBUSY;
                    }
                    else
                    {
                        /* time out or receive non-right message */
                        commandVdmResult.vdmCommandResult = (uint8_t)kCommandResult_Error;
                    }
                    (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_FAIL, &commandVdmResult, 1);
                    secondNewState          = PSM_IDLE;
                    pdInstance->psmNewState = PSM_HARD_RESET;
                    /* TODO: it may be CABLE. */
                    break;

                case PSM_PE_DFP_VDM_ATTENTION_REQUEST: /* C */
                    secondNewState = PD_PsmStructuredVdmInitiatorRequest(
                        pdInstance, statIndex,
                        (uint16_t)pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.SVID,
                        (uint8_t)pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.objPos, kVDM_Attention,
                        (pdInstance->structuredVdmCommandParameter.vdoCount != 0U) ? 1U : 0U,
                        (pdInstance->structuredVdmCommandParameter.vdoCount != 0U) ?
                            (pdInstance->structuredVdmCommandParameter.vdoData) :
                            NULL);

                    commandVdmResult.vdmCommand = (uint8_t)kVDM_Attention;
                    commandVdmResult.vdoData    = NULL;
                    commandVdmResult.vdoCount   = 0;
                    if (secondNewState == PSM_IDLE)
                    {
                        commandVdmResult.vdmCommandResult = (uint8_t)kCommandResult_Error;
                        (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_FAIL, &commandVdmResult, 1);
                    }
                    else
                    {
                        (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_SUCCESS, &commandVdmResult, 1);
                    }
                    break;

                case PSM_PE_DFP_CBL_SEND_CABLE_RESET: /* C */
                    PD_MsgSendHardOrCableReset(pdInstance, 1);
                    secondNewState = PSM_IDLE;
                    break;

                case PSM_PE_VENDOR_STRUCTURED_VDM_REQUEST: /* C */
                    secondNewState = PD_PsmStructuredVdmInitiatorRequest(
                        pdInstance, statIndex,
                        (uint16_t)pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.SVID,
                        (uint8_t)pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.objPos,
                        (pd_vdm_command_t)pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.command,
                        pdInstance->structuredVdmCommandParameter.vdoCount,
                        pdInstance->structuredVdmCommandParameter.vdoData);

                    if (pdInstance->structuredVdmCommandParameter.vendorVDMNeedResponse == 0U)
                    {
                        commandVdmResult.vdmCommand =
                            (uint8_t)pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.command;
                        commandVdmResult.vdoData  = NULL;
                        commandVdmResult.vdoCount = 0;
                        if (secondNewState == pdInstance->psmSecondaryState[statIndex])
                        {
                            (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_SUCCESS, &commandVdmResult, 1);
                        }
                        else
                        {
                            commandVdmResult.vdmCommandResult = (uint8_t)kCommandResult_Error;
                            (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_FAIL, &commandVdmResult, 1);
                        }
                        secondNewState = PSM_IDLE;
                    }
                    break;

                case PSM_PE_SRC_VDM_IDENTITY_ACKED: /* C */
                {
#if defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
                    pd_id_header_vdo_t *headerVDO = (pd_id_header_vdo_t *)((void *)&pdInstance->psmCableIdentities[1]);

                    pdInstance->psmCableIdentitiesDataCount = triggerInfo->pdMsgDataLength;
                    for (index = 0; index < triggerInfo->pdMsgDataLength; ++index)
                    {
                        pdInstance->psmCableIdentities[index] =
                            USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)(triggerInfo->pdMsgDataBuffer + index)));
                    }

                    if ((headerVDO->bitFields.productTypeUFPOrCablePlug == VDM_ID_HEADER_VDO_PASSIVE_CABLE_VAL) ||
                        (headerVDO->bitFields.productTypeUFPOrCablePlug == VDM_ID_HEADER_VDO_ACTIVE_CABLE_VAL))
                    {
                        if (pdInstance->psmCableIdentitiesDataCount > 4U)
                        {
                            pd_passive_cable_vdo_vdm20_t *cableVDO =
                                (pd_passive_cable_vdo_vdm20_t *)((void *)&pdInstance->psmCableIdentities[4]);
                            switch (cableVDO->bitFields.vbusCurrentHandlingCapability)
                            {
                                case 0x01u:
                                    pdInstance->dpmCableMaxCurrent = 3000 / 10; /* 3A */
                                    break;
                                case 0x02u:
                                    pdInstance->dpmCableMaxCurrent = 5000 / 10; /* 5A */
                                    break;
                                default:
                                    pdInstance->dpmCableMaxCurrent = 3000 / 10; /* 3A */
                                    break;
                            }
                        }
                    }
                    secondNewState = PSM_IDLE;
#endif
                    break;
                }

                case PSM_PE_SRC_VDM_IDENTITY_NAKED: /* C */
                    for (index = 0; index < 7U; ++index)
                    {
                        pdInstance->psmCableIdentities[index] = 0;
                    }
                    secondNewState = PSM_IDLE;
                    break;

                case PSM_PE_DFP_UFP_VDM_IDENTITY_REQUEST: /* C */
                    secondNewState = PD_PsmStructuredVdmInitiatorRequest(pdInstance, statIndex, PD_STANDARD_ID, 0,
                                                                         kVDM_DiscoverIdentity, 0, NULL);
                    break;

                case PSM_PE_DFP_UFP_VDM_IDENTITY_ACKED:  /* C */
                case PSM_PE_DFP_VDM_SVIDS_ACKED:         /* C */
                case PSM_PE_DFP_VDM_MODES_ACKED:         /* C */
                case PSM_PE_VENDOR_STRUCTURED_VDM_ACKED: /* C */
                    if (pdInstance->psmSecondaryState[statIndex] == PSM_PE_DFP_UFP_VDM_IDENTITY_ACKED)
                    {
                        commandVdmResult.vdmCommand = (uint8_t)kVDM_DiscoverIdentity;
                    }
                    else if (pdInstance->psmSecondaryState[statIndex] == PSM_PE_DFP_VDM_SVIDS_ACKED)
                    {
                        commandVdmResult.vdmCommand = (uint8_t)kVDM_DiscoverSVIDs;
                    }
                    else if (pdInstance->psmSecondaryState[statIndex] == PSM_PE_DFP_VDM_MODES_ACKED)
                    {
                        commandVdmResult.vdmCommand = (uint8_t)kVDM_DiscoverModes;
                    }
                    else if (pdInstance->psmSecondaryState[statIndex] == PSM_PE_VENDOR_STRUCTURED_VDM_ACKED)
                    {
                        commandVdmResult.vdmCommand = (uint8_t)commandVdmResult.vdmHeader.bitFields.command;
                    }
                    else
                    {
                        /* No action required. */
                    }
                    commandVdmResult.vdmHeader.structuredVdmHeaderVal =
                        USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)triggerInfo->pdMsgDataBuffer));
                    commandVdmResult.vdoData  = triggerInfo->pdMsgDataBuffer + 1U;
                    if (triggerInfo->pdMsgDataLength < 1U)
                    {
                        secondNewState = PSM_IDLE;
                        break;
                    }
                    commandVdmResult.vdoCount = triggerInfo->pdMsgDataLength - 1U;
                    (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_SUCCESS, &commandVdmResult, 1);
                    secondNewState = PSM_IDLE;
                    break;

                case PSM_PE_DFP_VDM_MODE_ENTRY_ACKED: /* C */
                    commandVdmResult.vdmHeader.structuredVdmHeaderVal =
                        USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)triggerInfo->pdMsgDataBuffer));
                    commandVdmResult.vdmCommand = (uint8_t)kVDM_EnterMode;
                    commandVdmResult.vdoData    = NULL;
                    commandVdmResult.vdoCount   = 0;
                    pdInstance->psmVdmActiveModeValidMask |= (uint8_t)0x01U
                                                             << (((uint8_t *)triggerInfo->pdMsgDataBuffer)[1] & 0x07U);
                    (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_SUCCESS, &commandVdmResult, 1);
                    secondNewState = PSM_IDLE;
                    break;

                case PSM_PE_DFP_VDM_MODE_EXIT_ACKED: /* C */
                    commandVdmResult.vdmCommand = (uint8_t)kVDM_ExitMode;
                    commandVdmResult.vdoData    = NULL;
                    commandVdmResult.vdoCount   = 0;
                    pdInstance->psmVdmActiveModeValidMask &=
                        ~((uint8_t)0x01U << pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.objPos);
                    if (pdInstance->amsVdmReplyMsg == kVDM_ResponderACK)
                    {
                        commandVdmResult.vdmHeader.structuredVdmHeaderVal =
                            USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)triggerInfo->pdMsgDataBuffer));
                        (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_SUCCESS, &commandVdmResult, 1);
                    }
                    else
                    {
                        commandVdmResult.vdmCommandResult = (uint8_t)kCommandResult_VDMNAK;
                        (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_FAIL, &commandVdmResult, 1);
                    }
#if (defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT)
                    /* DFP issues DR Swap to trigger Exit mode? */
                    if ((pdInstance->drSwapTriggerExitMode != 0U) && (pdInstance->psmVdmActiveModeValidMask == 0U))
                    {
                        /* After exiting active mode, need to trigger DR Swap again. */
                        pdInstance->drSwapTriggerExitMode = 0;
                        (void)PD_Command(pdInstance, (uint32_t)PD_DPM_CONTROL_DR_SWAP, NULL);
                    }
#endif
                    secondNewState = PSM_IDLE;
                    break;

                case PSM_PE_DFP_UFP_VDM_IDENTITY_NAKED:  /* C */
                case PSM_PE_DFP_VDM_SVIDS_NAKED:         /* C */
                case PSM_PE_DFP_VDM_MODES_NAKED:         /* C */
                case PSM_PE_DFP_VDM_MODE_ENTRY_NAKED:    /* C */
                case PSM_PE_VENDOR_STRUCTURED_VDM_NAKED: /* C */
                    switch (pdInstance->psmSecondaryState[statIndex])
                    {
                        case PSM_PE_DFP_UFP_VDM_IDENTITY_NAKED:
                            commandVdmResult.vdmCommand = (uint8_t)kVDM_DiscoverIdentity;
                            break;
                        case PSM_PE_DFP_VDM_SVIDS_NAKED:
                            commandVdmResult.vdmCommand = (uint8_t)kVDM_DiscoverSVIDs;
                            break;
                        case PSM_PE_DFP_VDM_MODES_NAKED:
                            commandVdmResult.vdmCommand = (uint8_t)kVDM_DiscoverModes;
                            break;
                        case PSM_PE_DFP_VDM_MODE_ENTRY_NAKED:
                            commandVdmResult.vdmCommand = (uint8_t)kVDM_EnterMode;
                            break;
                        case PSM_PE_VENDOR_STRUCTURED_VDM_NAKED:
                            commandVdmResult.vdmCommand =
                                (uint8_t)pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.command;
                            break;
                        default:
                            /* No action required. */
                            break;
                    }

                    commandVdmResult.vdoData  = NULL;
                    commandVdmResult.vdoCount = 0;
                    if (pdInstance->amsVdmReplyMsg == kVDM_ResponderNAK)
                    {
                        commandVdmResult.vdmCommandResult = (uint8_t)kCommandResult_VDMNAK;
                    }
                    else if (pdInstance->amsVdmReplyMsg == kVDM_ResponderBUSY)
                    {
                        (void)PD_TimerStart(pdInstance, tVDMBusyTimer, T_VDM_BUSY);
                        commandVdmResult.vdmCommandResult = (uint8_t)kCommandResult_VDMBUSY;
                    }
                    else
                    {
                        /* time out or receive non-right message */
                        commandVdmResult.vdmCommandResult = (uint8_t)kCommandResult_Error;
                    }
                    (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_FAIL, &commandVdmResult, 1);
                    secondNewState = PSM_IDLE;
                    break;

#if ((defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE))
                case PSM_PD_SEND_UNSTRUCTURED_VDM: /* C */
                {
                    pd_status_t status = PD_MsgSend(
                        pdInstance, (start_of_packet_t)pdInstance->unstructuredVdmCommandParameter.vdmSop,
                        kPD_MsgVendorDefined,
                        (uint32_t)pdInstance->unstructuredVdmCommandParameter.vdmHeaderAndVDOsCount * 4U + 2U,
                        (uint8_t *)pdInstance->unstructuredVdmCommandParameter.vdmHeaderAndVDOsData);
                    if (status == kStatus_PD_Success)
                    {
                        (void)PD_DpmAppCallback(pdInstance, PD_DPM_SEND_UNSTRUCTURED_VDM_SUCCESS, NULL, 1);
                    }
                    else
                    {
                        pd_command_result_t commandResultCallback = kCommandResult_Error;

                        (void)PD_DpmAppCallback(pdInstance, PD_DPM_SEND_UNSTRUCTURED_VDM_FAIL, &commandResultCallback,
                                                1);
                    }
                    secondNewState = PSM_IDLE;
                    break;
                }
#endif

                default:
                    /* No action required. */
                    break;
            }

            PD_PsmEndVdmCommand(pdInstance, pdInstance->psmSecondaryState[statIndex]);

            if (secondNewState != pdInstance->psmSecondaryState[statIndex])
            {
                pdInstance->psmNewSecondaryState[statIndex] = secondNewState;
                continue;
            }
        }

        /* state B */
        secondNewState = pdInstance->psmSecondaryState[statIndex];
        switch (pdInstance->psmSecondaryState[statIndex])
        {
            case PSM_IDLE: /* B */
                if (triggerInfo->triggerEvent == PSM_TRIGGER_PD_MSG)
                {
                    PD_PsmVdmIdleProcessPdMessage(pdInstance, statIndex, triggerInfo);
                }
                else if (triggerInfo->triggerEvent == PSM_TRIGGER_DPM_MSG)
                {
                    secondNewState = PD_PsmVdmIdleProcessDpmMessage(pdInstance, statIndex, triggerInfo);
                }
                else
                {
                    /* No action required. */
                }

                /* Used to response to a ACK/NAK VDM message.
                   Prevent that psmSecondaryState is not equal to PSM_IDLE when receiving a Exit Mode command. */
                if (pdInstance->vdmExitReceived[statIndex] != 0U)
                {
                    pd_svdm_command_request_t commandVdmRequest;
                    pd_structured_vdm_header_t reponseVdmHeader;
                    commandVdmRequest.vdmHeader.structuredVdmHeaderVal = pdInstance->vdmExitReceived[statIndex];
                    pdInstance->vdmExitReceived[statIndex]             = 0;
                    commandVdmRequest.vdoSop                           = (uint8_t)triggerInfo->pdMsgSop;
                    (void)PD_DpmAppCallback(pdInstance, PD_DPM_STRUCTURED_VDM_REQUEST, &commandVdmRequest, 0);

                    reponseVdmHeader.structuredVdmHeaderVal = commandVdmRequest.vdmHeader.structuredVdmHeaderVal;
                    if (commandVdmRequest.requestResultStatus == (uint8_t)kCommandResult_VDMACK)
                    {
                        reponseVdmHeader.bitFields.commandType = (uint32_t)kVDM_ResponderACK;
                        (void)PD_MsgSendStructuredVDM(pdInstance, statIndex, reponseVdmHeader,
                                                      commandVdmRequest.vdoCount,
                                                      (uint32_t *)commandVdmRequest.vdoData);
                    }
                    else
                    {
                        reponseVdmHeader.bitFields.commandType = (uint32_t)kVDM_ResponderNAK;
                        (void)PD_MsgSendStructuredVDM(pdInstance, statIndex, reponseVdmHeader, 0, NULL);
                    }
                }
                break;

            case PSM_PE_DFP_UFP_VDM_VDM_BUSY_WAIT: /* B */
                if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tVDMBusyTimer) != 0U)
                {
                    secondNewState = pdInstance->psmVDMBusyWaitDpmMsg;
                }
                break;

            case PSM_PE_SRC_IMPLICIT_CABLE_SOFT_RESET: /* B */
                if ((triggerInfo->triggerEvent == PSM_TRIGGER_PD_MSG) && (triggerInfo->pdMsgSop == statIndex) &&
                    (triggerInfo->pdMsgType == kPD_MsgAccept))
                {
                    (void)PD_TimerClear(pdInstance, tSenderResponseTimer);
                    secondNewState = PSM_PE_SRC_VDM_IDENTITY_REQUEST;
                }
                else if (PD_TimerCheckInvalidOrTimeOut(pdInstance, tSenderResponseTimer) != 0U)
                {
                    secondNewState = PSM_IDLE;
                }
                else
                {
                    didNothingSecond = 1;
                }
                break;

            case PSM_PE_SRC_VDM_IDENTITY_REQUEST: /* B */
#if defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
                secondNewState = PD_PsmVdmResponseHandler(pdInstance, statIndex, (uint8_t)kVDM_DiscoverIdentity,
                                                          triggerInfo, tVDMResponseTimer, PSM_PE_SRC_VDM_IDENTITY_ACKED,
                                                          PSM_PE_SRC_VDM_IDENTITY_NAKED);
#endif
                break;

            case PSM_PE_DFP_UFP_VDM_IDENTITY_REQUEST: /* B */
                secondNewState = PD_PsmVdmResponseHandler(
                    pdInstance, statIndex, (uint8_t)kVDM_DiscoverIdentity, triggerInfo, tVDMResponseTimer,
                    PSM_PE_DFP_UFP_VDM_IDENTITY_ACKED, PSM_PE_DFP_UFP_VDM_IDENTITY_NAKED);
                break;

            case PSM_PE_DFP_VDM_SVIDS_REQUEST: /* B */
                secondNewState =
                    PD_PsmVdmResponseHandler(pdInstance, statIndex, (uint8_t)kVDM_DiscoverSVIDs, triggerInfo,
                                             tVDMResponseTimer, PSM_PE_DFP_VDM_SVIDS_ACKED, PSM_PE_DFP_VDM_SVIDS_NAKED);
                break;

            case PSM_PE_DFP_VDM_MODES_REQUEST: /* B */
                secondNewState =
                    PD_PsmVdmResponseHandler(pdInstance, statIndex, (uint8_t)kVDM_DiscoverModes, triggerInfo,
                                             tVDMResponseTimer, PSM_PE_DFP_VDM_MODES_ACKED, PSM_PE_DFP_VDM_MODES_NAKED);
                break;

            case PSM_PE_DFP_VDM_MODE_ENTRY_REQUEST: /* B */
                secondNewState = PD_PsmVdmResponseHandler(pdInstance, statIndex, (uint8_t)kVDM_EnterMode, triggerInfo,
                                                          tVDMModeEntryTimer, PSM_PE_DFP_VDM_MODE_ENTRY_ACKED,
                                                          PSM_PE_DFP_VDM_MODE_ENTRY_NAKED);
                break;

            /* The Responder shall not return a BUSY acknowledgement and shall
               only return a NAK acknowledgement to a request not containing an Active Mode (i.e. Invalid object
               position). An
               Initiator which fails to receive an ACK within tVDMWaitModeExit or receives a NAK or BUSY response shall
               exit its
               Active Mode. */
            case PSM_PE_DFP_VDM_MODE_EXIT_REQUEST: /* B */
                secondNewState = PD_PsmVdmResponseHandler(pdInstance, statIndex, (uint8_t)kVDM_ExitMode, triggerInfo,
                                                          tVDMModeExitTimer, PSM_PE_DFP_VDM_MODE_EXIT_ACKED,
                                                          PSM_PE_DFP_VDM_MODE_EXIT_HARD_RESET);
                break;

            case PSM_PE_DFP_VDM_ATTENTION_REQUEST: /* B */
                secondNewState = PSM_IDLE;
                break;

            case PSM_PE_VENDOR_STRUCTURED_VDM_REQUEST: /* B */
                secondNewState = PD_PsmVdmResponseHandler(
                    pdInstance, statIndex,
                    (uint8_t)pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.command, triggerInfo,
                    tVDMResponseTimer, PSM_PE_VENDOR_STRUCTURED_VDM_ACKED, PSM_PE_VENDOR_STRUCTURED_VDM_NAKED);
                break;

            default:
                /* No action required. */
                break;
        }

        if ((secondNewState != PSM_UNKNOWN) && (secondNewState != pdInstance->psmSecondaryState[statIndex]))
        {
            pdInstance->psmNewSecondaryState[statIndex] = secondNewState;
            continue;
        }

        return didNothingSecond;
    }
}

#endif

#if (defined PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
static uint8_t PD_CheckWhetherInitiateCableDiscoveryIdentityOrNot(pd_instance_t *pdInstance)
{
    if (((pdInstance->raPresent != 0U) || (pdInstance->initialPowerRole == kPD_PowerRoleSink)) &&
        (pdInstance->psmPresentlyVconnSource == kPD_IsVconnSource) && (pdInstance->psmCableIdentitiesDataCount == 0U) &&
        (pdInstance->cableDiscoverIdentityCounter < N_DISCOVER_IDENTITY_COUNTER))
    {
        return 1;
    }
    return 0;
}
#endif

static pd_state_machine_state_t PD_PsmStateMachine(pd_instance_t *pdInstance)
{
    pd_state_machine_state_t returnVal = kSM_Continue;

    /* Did we do anything in Step A or B or C */
    /* Step A: common process */
    /* Step B: state is not changed */
    /* Step C: change to new state */
    do
    {
        returnVal = PD_PsmEnterState(pdInstance);
        returnVal = PD_PsmDisconnectCheck(pdInstance, returnVal);
        if ((returnVal != kSM_ErrorRecovery) && (returnVal != kSM_Detach))
        {
            returnVal = PD_PsmProcessState(pdInstance);
            returnVal = PD_PsmDisconnectCheck(pdInstance, returnVal);
        }
    } while (((pdInstance->psmCurState != pdInstance->psmNewState) || (returnVal == kSM_Continue)) &&
             (returnVal != kSM_ErrorRecovery) && (returnVal != kSM_Detach));

    if ((returnVal == kSM_ErrorRecovery) || (returnVal == kSM_Detach))
    {
        PD_TimerCancelAllTimers(pdInstance, _tStartTimer, _tMaxDpmTimer);
    }

    return returnVal;
}

static pd_status_t PD_DpmAppCallback(pd_instance_t *pdInstance,
                                     pd_dpm_callback_event_t event,
                                     void *param,
                                     uint8_t done)
{
    pd_status_t status1 = kStatus_PD_Error;
#if (defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT)
    pd_status_t status2 = kStatus_PD_Error;
#endif

    if (done != 0U)
    {
#if defined(PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE) && (PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE)
        PD_MsgSrcEndCommand(pdInstance);
#endif

/* in case the power is not reset to normal operation */
#if (defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
        if (pdInstance->inProgress != kVbusPower_InFRSwap)
#endif
        {
            PD_ConnectSetPowerProgress(pdInstance, kVbusPower_Stable);
        }
        pdInstance->commandProcessing = PD_DPM_INVALID;
    }

#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
    if (pdInstance->dumpResponse != 0U)
    {
        pdInstance->dumpResponse = 0;
    }
    else
#endif
    {
#if (defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT)
        status2 = PD_AltModeCallback(pdInstance, (uint32_t)event, param);
#endif
        status1 = pdInstance->pdCallback(pdInstance->callbackParam, (uint32_t)event, param);

#if (defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT)
        if ((status1 == kStatus_PD_Success) || (status2 == kStatus_PD_Success))
        {
            return kStatus_PD_Success;
        }
#endif
    }

    return status1;
}

#if defined(PD_CONFIG_PHY_LOW_POWER_LEVEL) && (PD_CONFIG_PHY_LOW_POWER_LEVEL)
static void PD_LowPowerTransition(pd_instance_t *pdInstance)
{
    pd_stack_state_t getInfo = kState_Busy;

    (void)PD_Control(pdInstance, (uint32_t)PD_CONTROL_GET_PD_STATE, &getInfo);
    if ((getInfo == kState_Idle) && (pdInstance->lowPowerState == 0u))
    {
        /* PD stack idle && not in low power state */
        (void)pdInstance->phyInterface->pdPhyControl(pdInstance, PD_PHY_ENTER_LOW_POWER_STATE, NULL);
    }
    else if ((getInfo == kState_Busy) && (pdInstance->lowPowerState != 0u))
    {
        /* PD stack busy && in low power state */
        (void)pdInstance->phyInterface->pdPhyControl(pdInstance, PD_PHY_EXIT_LOW_POWER_STATE, NULL);
    }
    else
    {
        /* No action required. */
    }
}
#endif

static pd_status_t PD_InitiateAmsCheck(pd_instance_t *pdInstance, pd_command_t command)
{
    pd_status_t status = kStatus_PD_Busy;

    if ((pdInstance->psmCurState != PSM_PE_SRC_READY) && (pdInstance->psmCurState != PSM_PE_SNK_READY))
    {
    }
    else if (pdInstance->commandProcessing != PD_DPM_INVALID)
    {
    }
    else if (pdInstance->inProgress != kVbusPower_Stable)
    {
    }
    else
    {
        status = kStatus_PD_Success;
    }

    return status;
}

uint8_t PD_PsmCheckVsafe0V(pd_instance_t *pdInstance)
{
    uint8_t powerState = PD_VBUS_POWER_STATE_VSAFE0V_MASK;
    (void)PD_PhyControl(pdInstance, PD_PHY_GET_VBUS_POWER_STATE, &powerState);
    if ((powerState & PD_VBUS_POWER_STATE_VSAFE0V_MASK) != 0U)
    {
        return 1;
    }
    return 0;
}

uint8_t PD_PsmCheckVsafe5V(pd_instance_t *pdInstance)
{
    uint8_t powerState = (PD_VBUS_POWER_STATE_VSAFE5V_MASK | PD_VBUS_POWER_STATE_VBUS_MASK);
    (void)PD_PhyControl(pdInstance, PD_PHY_GET_VBUS_POWER_STATE, &powerState);
    if ((powerState & (PD_VBUS_POWER_STATE_VSAFE5V_MASK | PD_VBUS_POWER_STATE_VBUS_MASK)) != 0U)
    {
        return 1;
    }
    return 0;
}

/* 0 - timeout, other values - the remaining time. */
uint16_t PD_PsmTimerWait(pd_instance_t *pdInstance, tTimer_t timrName, uint16_t timrTime, uint32_t checkEvent)
{
    uint32_t eventSet = 0;

    (void)PD_TimerStart(pdInstance, timrName, timrTime);
    do
    {
        (void)OSA_EventWait(pdInstance->taskEventHandle, PD_TASK_EVENT_ALL, 0, (uint32_t)timrTime, &eventSet);
        PD_PortTaskEventProcess(pdInstance, eventSet);

        if ((PD_TimerCheckInvalidOrTimeOut(pdInstance, timrName) != 0U) && ((eventSet & PD_TASK_EVENT_TIME_OUT) != 0U))
        {
            break;
        }
        else if ((eventSet & checkEvent) != 0U)
        {
            break;
        }
        /* Disconnected */
        else if (PD_ConnectCheck(pdInstance) == kConnectState_Disconnected)
        {
            break;
        }
        else
        {
            /* No action required. */
        }
    } while (true);
    (void)PD_TimerClear(pdInstance, timrName);

    return pdInstance->timrsTimeValue[(uint8_t)timrName];
}

/*! ***************************************************************************
   task process related functions
******************************************************************************/
void PD_PortTaskEventProcess(pd_instance_t *pdInstance, uint32_t eventSet)
{
    if ((eventSet & (uint32_t)PD_TASK_EVENT_PHY_STATE_CHAGNE) != 0U)
    {
        (void)PD_PhyControl(pdInstance, PD_PHY_UPDATE_STATE, NULL);
    }

    if (pdInstance->dpmStateMachine == 0U)
    {
        /* clear events, and only respond to PHY event and FR swap event in the detached state. */
        eventSet &= ~(uint32_t)((uint32_t)PD_TASK_EVENT_PHY_STATE_CHAGNE | (uint32_t)PD_TASK_EVENT_FR_SWAP_SINGAL);

        if (eventSet != 0U)
        {
            (void)OSA_EventClear(pdInstance->taskEventHandle, eventSet);
        }
    }
    else
    {
        /* If PD state machine is running, only clear other event, and other events will be handled later. */
        if ((eventSet & (uint32_t)PD_TASK_EVENT_OTHER) != 0U)
        {
            (void)OSA_EventClear(pdInstance->taskEventHandle, PD_TASK_EVENT_OTHER);
        }
    }
}

void PD_StackStateMachine(pd_instance_t *pdInstance)
{
    uint32_t taskEventSet = 0;
    TypeCState_t typecState;
    pd_connect_state_t connectState;
    pd_state_machine_state_t smState = kSM_None;
    uint8_t connected;

    if (pdInstance->initializeLabel == 0U)
    {
        pdInstance->initializeLabel = 1;
        pdInstance->isConnected     = 0;
        /* We want to override any existing MTP-based connection */
        PD_ConnectInitRole(pdInstance, 0);
        pdInstance->connectedResult = PD_ConnectGetStateMachine(pdInstance);
#if defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
        PD_DpmSetVconn(pdInstance, 0);
#endif /* PD_CONFIG_VCONN_SUPPORT */
    }

#if defined(PD_CONFIG_PHY_LOW_POWER_LEVEL) && (PD_CONFIG_PHY_LOW_POWER_LEVEL)
    PD_LowPowerTransition(pdInstance);
#endif

#if (defined PD_CONFIG_COMMON_TASK) && (PD_CONFIG_COMMON_TASK)
    (void)OSA_EventWait(pdInstance->taskEventHandle, PD_TASK_EVENT_ALL, 0, 0, &taskEventSet);
#else
    (void)OSA_EventWait(pdInstance->taskEventHandle, PD_TASK_EVENT_ALL, 0, pdInstance->taskWaitTime, &taskEventSet);
#endif
/* if waiting for ever and no event */
#ifndef SDK_OS_FREE_RTOS
    if ((pdInstance->taskWaitTime == osaWaitForever_c) && (taskEventSet == 0u))
    {
        return;
    }
#endif

#if defined(PD_CONFIG_PHY_LOW_POWER_LEVEL) && (PD_CONFIG_PHY_LOW_POWER_LEVEL)
    PD_LowPowerTransition(pdInstance);
#endif

    PD_PortTaskEventProcess(pdInstance, taskEventSet);

    /* Process Type-C state change by PD stack */
    typecState = PD_ConnectGetStateMachine(pdInstance);
    if (typecState != pdInstance->connectedResult)
    {
        pdInstance->connectedResult = typecState;
    }

    /* Process Type-C state change by Type-C state machine */
    connectState = PD_ConnectCheck(pdInstance);
    typecState   = PD_ConnectGetStateMachine(pdInstance);
    if (connectState != kConnectState_NotStable)
    {
        if (connectState == kConnectState_Connected)
        {
            connected = 1;
        }
        else
        {
            connected = 0;
        }
        /* connect state change */
        if ((connected != pdInstance->isConnected) ||
            ((pdInstance->isConnected != 0U) && (typecState != pdInstance->connectedResult)))
        {
            if (connected != 0U)
            {
                switch (typecState)
                {
                    case TYPEC_ATTACHED_SRC:
                    case TYPEC_ATTACHED_SNK:
                        pdInstance->pendingSOP = (uint8_t)kPD_MsgSOPMask;
                        if (typecState == TYPEC_ATTACHED_SRC)
                        {
                            pdInstance->curPowerRole            = kPD_PowerRoleSource;
                            pdInstance->initialPowerRole        = kPD_PowerRoleSource;
                            pdInstance->curDataRole             = kPD_DataRoleDFP;
                            pdInstance->psmPresentlyVconnSource = kPD_IsVconnSource;
                        }
                        else
                        {
                            pdInstance->curPowerRole            = kPD_PowerRoleSink;
                            pdInstance->initialPowerRole        = kPD_PowerRoleSink;
                            pdInstance->curDataRole             = kPD_DataRoleUFP;
                            pdInstance->psmPresentlyVconnSource = kPD_NotVconnSource;
                        }
                        break;

                    case TYPEC_AUDIO_ACCESSORY:
                    case TYPEC_UNORIENTED_DEBUG_ACCESSORY_SRC:
                    case TYPEC_DEBUG_ACCESSORY_SNK:
                    case TYPEC_ORIENTED_DEBUG_ACCESSORY_SNK:
                    case TYPEC_POWERED_ACCESSORY:
                        /* do nothing */
                        /* pdInstance->curPowerRole = kPD_PowerRoleSource; */
                        /* pdInstance->curDataRole = kPD_DataRoleDFP; */
                        break;

#if (defined PD_CONFIG_DEBUG_ACCESSORY_SUPPORT) && (PD_CONFIG_DEBUG_ACCESSORY_SUPPORT)
                    case TYPEC_ORIENTED_DEBUG_ACCESSORY_SRC:
                        if (PD_CONFIG_DEBUG_ACCESSORY_ROLE == CONFIG_DEBUG_ACCESSORY_NONE)
                        {
                            /* Wait for disconnection */
                        }
                        else
                        {
                            pdInstance->curPowerRole            = kPD_PowerRoleSource;
                            pdInstance->initialPowerRole        = kPD_PowerRoleSource;
                            pdInstance->curDataRole             = kPD_DataRoleDFP;
                            pdInstance->psmPresentlyVconnSource = kPD_IsVconnSource;
                        }
                        break;
#endif
                    default:
                        /* No action required. */
                        break;
                }

                /* change from disconnect to connect */
                if (pdInstance->isConnected == 0U)
                {
                    (void)PD_DpmAppCallback(pdInstance, PD_CONNECTED, NULL, 0);
                    PD_PsmConnect(pdInstance);
                }
                /* still connect, but role change. For example: Try.SRC */
                else if (typecState != pdInstance->connectedResult)
                {
                    pdInstance->psmNewState = PSM_IDLE;
                    (void)PD_DpmAppCallback(pdInstance, PD_CONNECT_ROLE_CHANGE, NULL, 0);
                }
                else
                {
                    /* No action required. */
                }
            }
            /* change from connect to disconnect */
            else if (pdInstance->isConnected != 0U)
            {
                PD_PsmDisconnect(pdInstance);
                (void)PD_DpmAppCallback(pdInstance, PD_DISCONNECTED, NULL, 0);
                pdInstance->initializeLabel = 0;
                PD_StackSetEvent(pdInstance, PD_TASK_EVENT_RESET_CONFIGURE);
            }
            else
            {
                /* No action required. */
            }

            pdInstance->isConnected = connected;
        }
        else if (pdInstance->isConnected != 0U) /* connect result stable and connected */
        {
            pdInstance->noConnectButVBusExist = 0;
            if (pdInstance->dpmStateMachine != 0U)
            {
                smState = PD_PsmStateMachine(pdInstance);
            }
            else
            {
                /* wait vbus charge */
                /* Neither Source nor Sink should enter PSM before VSafe5V is available */
                if (PD_PsmCheckVsafe5V(pdInstance) != 0U)
                {
                    pdInstance->dpmStateMachine = 1;
                    pdInstance->psmNewState     = PSM_IDLE;
                    smState                     = PD_PsmStateMachine(pdInstance);
                }
            }

            /* Force a partner disconnect if the PSM has requested a error recovery */
            if ((smState == kSM_ErrorRecovery) || (smState == kSM_Detach))
            {
                pdInstance->isConnected = 0;
                PD_PsmDisconnect(pdInstance);
                if (smState == kSM_ErrorRecovery)
                {
                    PD_ConnectInitRole(pdInstance, 1);
                    pdInstance->initializeLabel = 1;
                }
                else
                {
                    pdInstance->initializeLabel = 0;
                }
                (void)PD_DpmAppCallback(pdInstance, PD_DISCONNECTED, NULL, 0);

                PD_StackSetEvent(pdInstance, PD_TASK_EVENT_RESET_CONFIGURE);
            }
            else if (smState == kSM_Continue)
            {
                pdInstance->taskWaitTime = PD_WAIT_EVENT_TIME;
            }
            else
            {
                pdInstance->taskWaitTime = osaWaitForever_c; /* wait forever */
            }
        }
        else
        {
            /* No action required. */
        }

#if (defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
        /* check FR_Swap signal */
        (void)OSA_EventGet(pdInstance->taskEventHandle, PD_TASK_EVENT_FR_SWAP_SINGAL, &taskEventSet);
        if (taskEventSet != 0U)
        {
            if (pdInstance->isConnected == 0U)
            {
                (void)OSA_EventClear(pdInstance->taskEventHandle, PD_TASK_EVENT_FR_SWAP_SINGAL);

                /* when receiving FR_Swap signal, PTN5110 will open source vbus defaultly.  */
                if (pdInstance->pdConfig->phyType == (uint8_t)kPD_PhyPTN5110)
                {
                    (void)pdInstance->callbackFns->PD_SrcTurnOffVbus(pdInstance->callbackParam, kVbusPower_Stable);
                }
            }
        }

        /* check unreasonable situation */
        if (pdInstance->isConnected == 0U)
        {
            if (PD_PsmCheckVsafe5V(pdInstance) != 0U)
            {
                pdInstance->noConnectButVBusExist++;
                PD_PsmTurnOffVconnAndVbus(pdInstance, kVbusPower_Stable);
                if (pdInstance->noConnectButVBusExist >= 100U)
                {
                    connected                         = 1; /* connected means enablement here. */
                    pdInstance->noConnectButVBusExist = 0;
                    (void)pdInstance->phyInterface->pdPhyDeinit(pdInstance);
                    (void)pdInstance->phyInterface->pdPhyInit(pdInstance);
                    (void)pdInstance->phyInterface->pdPhyControl(pdInstance, PD_PHY_CONTROL_ALERT_INTERRUPT,
                                                                 &connected);
                }
            }
        }
        else
        {
            pdInstance->noConnectButVBusExist = 0;
        }
#endif
    }
    pdInstance->connectedResult = typecState;

#if 0
    /* unclear error recovery */
    connectStatus = PD_ConnectGetStateMachine(pdInstance);
    if (connectStatus == PD_ConnectGetInitRoleState(pdInstance))
    {
        pdInstance->typeCStateNeedRecovery++;
    }
    else
    {
        pdInstance->typeCStateNeedRecovery = 0;
    }
    if ((!(pdInstance->isConnected)) && (pdInstance->typeCStateNeedRecovery >= 100))
    {
        pdInstance->typeCStateNeedRecovery = 0;
        PD_ConnectInitRole(pdInstance, 1);
    }
#endif
}

void PD_DpmDischargeVbus(pd_instance_t *pdInstance, uint8_t enable)
{
    (void)PD_PhyControl(pdInstance, PD_PHY_DISCHARGE_VBUS, &enable);
}

#if defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
void PD_DpmDischargeVconn(pd_instance_t *pdInstance, uint8_t enable)
{
    (void)PD_PhyControl(pdInstance, PD_PHY_DISCHARGE_VCONN, &enable);
}

void PD_DpmSetVconn(pd_instance_t *pdInstance, uint8_t enable)
{
    if ((pdInstance->pdPowerPortConfig->vconnSupported != 0U) && (pdInstance->callbackFns->PD_ControlVconn != NULL))
    {
        (void)pdInstance->callbackFns->PD_ControlVconn(pdInstance->callbackParam, enable);
    }
}
#endif /* PD_CONFIG_VCONN_SUPPORT */

/* change to command and is async */
pd_status_t PD_Command(pd_handle pdHandle, uint32_t command, void *param)
{
    pd_status_t status;
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;

    /* Check if an AMS can be initiated */
    status = PD_InitiateAmsCheck(pdInstance, (pd_command_t)command);

#if (defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)
    /* common process */
    switch ((pd_command_t)command)
    {
        case PD_DPM_CONTROL_DISCOVERY_IDENTITY:
        case PD_DPM_CONTROL_DISCOVERY_SVIDS:
        case PD_DPM_CONTROL_DISCOVERY_MODES:
        {
            pdInstance->structuredVdmCommandParameter = *((pd_svdm_command_param_t *)param);

            pdInstance->structuredVdmCommandParameter.vdoCount                        = 0;
            pdInstance->structuredVdmCommandParameter.vdoData                         = NULL;
            pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.commandType = (uint8_t)kVDM_Initiator;
            pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.objPos      = 0;
#if defined(PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30)
            pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.vdmVersion =
                (pdInstance->revision < PD_SPEC_REVISION_30) ? PD_SPEC_STRUCTURED_VDM_VERSION_10 :
                                                               PD_CONFIG_STRUCTURED_VDM_VERSION;
#else
            pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.vdmVersion =
                PD_SPEC_STRUCTURED_VDM_VERSION_10;
#endif
            pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.vdmType = 1;
            if ((command == (uint32_t)PD_DPM_CONTROL_DISCOVERY_IDENTITY) ||
                (command == (uint32_t)PD_DPM_CONTROL_DISCOVERY_SVIDS))
            {
                pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.SVID = PD_STANDARD_ID;
            }
            break;
        }

        case PD_DPM_SEND_VENDOR_STRUCTURED_VDM:
        case PD_DPM_CONTROL_ENTER_MODE:
        case PD_DPM_CONTROL_EXIT_MODE:
        case PD_DPM_CONTROL_SEND_ATTENTION:
        {
            pdInstance->structuredVdmCommandParameter = *((pd_svdm_command_param_t *)param);

            pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.commandType = (uint8_t)kVDM_Initiator;
#if defined(PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30)
            pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.vdmVersion =
                (pdInstance->revision < PD_SPEC_REVISION_30) ? PD_SPEC_STRUCTURED_VDM_VERSION_10 :
                                                               PD_CONFIG_STRUCTURED_VDM_VERSION;
#else
            pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.vdmVersion =
                PD_SPEC_STRUCTURED_VDM_VERSION_10;
#endif
            pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.vdmType = 1;
            if (command == (uint32_t)PD_DPM_CONTROL_EXIT_MODE)
            {
                pdInstance->structuredVdmCommandParameter.vdoCount = 0;
                pdInstance->structuredVdmCommandParameter.vdoData  = NULL;
            }
            break;
        }

        default:
            /* No action required. */
            break;
    }
#endif

    /* special process */
    switch ((pd_command_t)command)
    {
        case PD_DPM_CONTROL_POWER_NEGOTIATION:
        {
            if (pdInstance->curPowerRole != kPD_PowerRoleSource)
            {
                status = kStatus_PD_Error;
            }
            break;
        }

        case PD_DPM_CONTROL_REQUEST:
        {
            if (pdInstance->curPowerRole == kPD_PowerRoleSink)
            {
                pdInstance->rdoRequest.rdoVal = *((uint32_t *)param);
            }
            else
            {
                status = kStatus_PD_Error;
            }
            break;
        }

        case PD_DPM_CONTROL_GOTO_MIN:
        {
            if (pdInstance->psmCurState != PSM_PE_SRC_READY)
            {
                status = kStatus_PD_Error;
            }
            break;
        }

#if (defined PD_CONFIG_DUAL_POWER_ROLE_ENABLE) && (PD_CONFIG_DUAL_POWER_ROLE_ENABLE)
        case PD_DPM_CONTROL_PR_SWAP:
        {
            if (PD_PsmIsDualRole(pdInstance) == 0U)
            {
                status = kStatus_PD_Error;
            }
            break;
        }
#endif

#if (defined PD_CONFIG_DUAL_DATA_ROLE_ENABLE) && (PD_CONFIG_DUAL_DATA_ROLE_ENABLE)
        case PD_DPM_CONTROL_DR_SWAP:
        {
            if (pdInstance->pdPowerPortConfig->dataFunction != (uint8_t)kDataConfig_DRD)
            {
                status = kStatus_PD_Error;
            }
            break;
        }
#endif

#if (defined PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
        case PD_DPM_CONTROL_VCONN_SWAP:
        {
            if (pdInstance->pdPowerPortConfig->vconnSupported == 0U)
            {
                status = kStatus_PD_Error;
            }
            break;
        }
#endif

#if (defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
        case PD_DPM_GET_SRC_EXT_CAP:
#endif
        case PD_DPM_CONTROL_GET_PARTNER_SOURCE_CAPABILITIES:
        {
            if (pdInstance->pdPowerPortConfig->typecRole == (uint8_t)kPowerConfig_SourceOnly)
            {
                status = kStatus_PD_Error;
            }
            break;
        }

        case PD_DPM_CONTROL_GET_PARTNER_SINK_CAPABILITIES:
        {
            if (pdInstance->pdPowerPortConfig->typecRole == (uint8_t)kPowerConfig_SinkOnly)
            {
                status = kStatus_PD_Error;
            }
            break;
        }

        case PD_DPM_CONTROL_SOFT_RESET:
            pdInstance->psmSoftResetSop = *((start_of_packet_t *)param);
            break;

        case PD_DPM_CONTROL_HARD_RESET:
            /* Hard Reset is always success, whether PD stack is idle or not. */
            status = kStatus_PD_Success;
            break;

#if (defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)
        case PD_DPM_CONTROL_DISCOVERY_IDENTITY:
        {
            pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.command = (uint8_t)kVDM_DiscoverIdentity;
            break;
        }

        case PD_DPM_CONTROL_DISCOVERY_SVIDS:
        {
            pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.command = (uint8_t)kVDM_DiscoverSVIDs;
            break;
        }

        case PD_DPM_CONTROL_DISCOVERY_MODES:
        {
            pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.command = (uint8_t)kVDM_DiscoverModes;
            break;
        }

        case PD_DPM_CONTROL_SEND_ATTENTION:
        {
            pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.command = (uint8_t)kVDM_Attention;
            break;
        }

        case PD_DPM_SEND_VENDOR_STRUCTURED_VDM:
        {
            break;
        }

        case PD_DPM_CONTROL_ENTER_MODE:
        {
            pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.command = (uint8_t)kVDM_EnterMode;

            if (pdInstance->curDataRole != kPD_DataRoleDFP)
            {
                status = kStatus_PD_Error;
            }
            break;
        }

        case PD_DPM_CONTROL_EXIT_MODE:
        {
            pdInstance->structuredVdmCommandParameter.vdmHeader.bitFields.command = (uint8_t)kVDM_ExitMode;

            if (pdInstance->curDataRole != kPD_DataRoleDFP)
            {
                status = kStatus_PD_Error;
            }
            break;
        }

        case PD_DPM_SEND_UNSTRUCTURED_VDM:
        {
            pd_unstructured_vdm_header_t *unstructuredVDMHeader;
            pdInstance->unstructuredVdmCommandParameter = *((pd_unstructured_vdm_command_param_t *)param);
            unstructuredVDMHeader =
                (pd_unstructured_vdm_header_t *)((void *)&pdInstance->unstructuredVdmCommandParameter
                                                     .vdmHeaderAndVDOsData[0]);
            unstructuredVDMHeader->bitFields.vdmType = 0;
            break;
        }
#endif

        case PD_DPM_CONTROL_CABLE_RESET:
            break;

#if (defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
        case PD_DPM_GET_STATUS:
            break;

#if (defined PD_CONFIG_PD3_PPS_ENABLE) && (PD_CONFIG_PD3_PPS_ENABLE)
        case PD_DPM_GET_PPS_STATUS:
            if (pdInstance->curPowerRole != kPD_PowerRoleSink)
            {
                status = kStatus_PD_Error;
            }
            break;
#endif

        case PD_DPM_GET_BATTERY_CAP:
        case PD_DPM_GET_BATTERY_STATUS:
            pdInstance->getBatteryCapDataBlock = *((uint8_t *)param);
            break;

        case PD_DPM_GET_MANUFACTURER_INFO:
#if 0
        case PD_DPM_SECURITY_REQUEST:
        case PD_DPM_FIRMWARE_UPDATE_REQUEST:
#endif
            pdInstance->commandExtParam = *((pd_command_data_param_t *)param);
            break;

        case PD_DPM_ALERT:
            pdInstance->alertADO = *((uint32_t *)param);
            break;
#endif

#if (defined PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
        case PD_DPM_FAST_ROLE_SWAP:
            if ((pdInstance->psmCurState != PSM_PE_SRC_READY) || (pdInstance->curPowerRole != kPD_PowerRoleSource) ||
                (PD_PsmIsDualRole(pdInstance) == 0U))
            {
                status = kStatus_PD_Error;
            }
            else
            {
                /* Fast Role Swap is always success, whether PD stack is idle or not. */
                status = kStatus_PD_Success;
            }
            break;
#endif

        default:
            status = kStatus_PD_Error;
            break;
    }

    if (status == kStatus_PD_Success)
    {
/* (1). PTN5110 need signal fr_swap before turn off vbus, otherwise signal will fail
 * (2). "enter the PE_FRS_SRC_SNK_CC_Signal state" after "signal fr_swap" need be atomic.
 *  the psmNewState need change in the same task context, it is the PD task. here
 *  will not change it. So the (2) is not satisfied.
 */
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
        OSA_SR_ALLOC();

        if (command == (uint32_t)PD_DPM_FAST_ROLE_SWAP)
        {
            /* fr swap msg is received but the state may not be in PE_FRS_SRC_SNK_CC_Signal,
             * frSwapReceived avoid lost the fr swap msg if not in the PE_FRS_SRC_SNK_CC_Signal state */
            pdInstance->frSwapReceived = 0;
            /* frs signal is sent and wait the frs msg.
             * "(1) The PD_DPM_FAST_ROLE_SWAP is processed by PD task" and "(2) received the fr swap msg"
             * (1) may be ahead (2); (2) may be ahead (1).
             * frSignaledWaitFrSwap is used to enter PE_FRS_SRC_SNK_CC_Signal only in one place.
             */
            pdInstance->frSignaledWaitFrSwap = 1;
            PD_ConnectSetPowerProgress(pdInstance, kVbusPower_InFRSwap);

            OSA_ENTER_CRITICAL();
            (void)PD_PhyControl(pdInstance, PD_PHY_SIGNAL_FR_SWAP, NULL);
        }
#endif
        PD_DpmSendMsg(pdHandle, (uint8_t)command);

#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
        if (command == (uint32_t)PD_DPM_FAST_ROLE_SWAP)
        {
            OSA_EXIT_CRITICAL();
        }
#endif
    }

    return status;
}

pd_status_t PD_Control(pd_handle pdHandle, uint32_t controlCode, void *param)
{
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;
    pd_status_t status        = kStatus_PD_Success;

    switch ((pd_control_t)controlCode)
    {
        case PD_CONTROL_GET_PD_STATE:
            /* The following caditions indicate PD or Type-C state machine is busy: */
            /* 1. There are the running timers. */
            /* 2. There are the pending events. */
            /* 3. There is the AMS being executed. */
            /* 4. VBUS power is not stable. */
            /* 5. Type-C state machine is not in the stable state. */
            /* 6. Type-C is in the attached state, but PD state machine is not in the PE_SNK_READY, PE_SRC_READY,
                  PE_SRC_DISABLED and PE_SNK_WAIT_FOR_CAPABILITIES state.
                  a. PE_SNK_READY and PE_SRC_READY state indicate that an explict contact has been established.
                  b. PE_SRC_DISABLED state indicates the partner sink doesn't reply GoodCRC in response to SrcCapability
                    message. There isn't PD contact establishment or the partner sink doesn't support PD communication.
                  c. PE_SNK_WAIT_FOR_CAPABILITIES state indicates the self-sink doesn't receive SrcCapability message.
                    There isn't PD contact establishment or the partner source doesn't support PD communication. */
            if ((PD_TimerBusy(pdHandle) != 0U) || (PD_StackHasPendingEvent(pdInstance) != 0U) ||
                (pdInstance->commandProcessing != PD_DPM_INVALID) || (pdInstance->inProgress != kVbusPower_Stable) ||
                (PD_ConnectState(pdInstance) == kConnectState_NotStable) ||
                ((pdInstance->isConnected != 0U) && (pdInstance->psmCurState != PSM_PE_SNK_READY) &&
                 (pdInstance->psmCurState != PSM_PE_SRC_READY) && (pdInstance->psmCurState != PSM_PE_SRC_DISABLED) &&
                 (pdInstance->psmCurState != PSM_PE_SNK_WAIT_FOR_CAPABILITIES)))
            {
                *((uint8_t *)param) = (uint8_t)kState_Busy;
            }
            else
            {
                *((uint8_t *)param) = (uint8_t)kState_Idle;
            }
            break;

        case PD_CONTROL_GET_TYPEC_ORIENTATION:
            if (pdInstance->ccUsed == kPD_CC1)
            {
                *((uint8_t *)param) = 0;
            }
            else
            {
                *((uint8_t *)param) = 1;
            }
            break;

        case PD_CONTROL_GET_POWER_ROLE:
            *((uint8_t *)param) = (uint8_t)pdInstance->curPowerRole;
            break;

        case PD_CONTROL_GET_DATA_ROLE:
            *((uint8_t *)param) = (uint8_t)pdInstance->curDataRole;
            break;

        case PD_CONTROL_GET_TYPEC_CONNECT_STATE:
            *((uint8_t *)param) = (uint8_t)pdInstance->connectState;
            break;

        case PD_CONTROL_GET_VCONN_ROLE:
            *((uint8_t *)param) = (uint8_t)pdInstance->psmPresentlyVconnSource;
            break;

        case PD_CONTROL_GET_TYPEC_CURRENT_VALUE:
        case PD_CONTROL_GET_SNK_TYPEC_CURRENT_CAP:
            status = PD_PhyControl(pdInstance, PD_PHY_GET_TYPEC_CURRENT_CAP, param);
            break;

        case PD_CONTROL_PHY_POWER_PIN:
            (void)PD_PhyControl(pdInstance, PD_PHY_CONTROL_POWER_PIN, param);
            break;

#if defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
        case PD_CONTROL_VCONN:
            (void)PD_PhyControl(pdInstance, PD_PHY_CONTROL_VCONN, param);
            break;
#endif

        case PD_CONTROL_DISCHARGE_VBUS:
        {
            uint32_t eventSet = 0U;

            PD_DpmDischargeVbus(pdInstance, 1);
#if (defined PD_CONFIG_MIN_DISCHARGE_TIME_ENABLE) && (PD_CONFIG_MIN_DISCHARGE_TIME_ENABLE)
            (void)PD_TimerStart(pdInstance, tTypeCVbusMinDischargeTimer, T_MIN_VBUS_DISCHARGE);
            while (PD_TimerCheckValidTimeOut(pdInstance, tTypeCVbusMinDischargeTimer) == 0U)
#else
            (void)PD_TimerStart(pdInstance, tTypeCVbusMaxDischargeTimer, T_MAX_VBUS_DISCHARGE);
            while (PD_TimerCheckValidTimeOut(pdInstance, tTypeCVbusMaxDischargeTimer) == 0U)
#endif
            {
                if (PD_PsmCheckVsafe0V(pdInstance) != 0U)
                {
                    break;
                }
                /* When VBUS decreases to vSafe0V, the EXTENDED_STATUS.vSafe0V register will be set and an interrupt
                   will be asserted, so software does not need to check vSafe0V constantly. */
                (void)OSA_EventWait(pdInstance->taskEventHandle, PD_TASK_EVENT_ALL, 0, 5, &eventSet);
            }
            PD_DpmDischargeVbus(pdInstance, 0);
            break;
        }

        case PD_CONTROL_INFORM_VBUS_VOLTAGE_RANGE:
#if !(defined(PD_CONFIG_VBUS_ALARM_SUPPORT) && (PD_CONFIG_VBUS_ALARM_SUPPORT))
            (void)PD_PhyControl(pdInstance, PD_PHY_INFORM_VBUS_VOLTAGE_RANGE, param);
#endif
            break;

        case PD_CONTROL_GET_CABLE_INFO:
        {
            pd_cable_plug_info_t *plugInfo;

            if (param == NULL)
            {
                break;
            }

            plugInfo = (pd_cable_plug_info_t *)param;
            if (pdInstance->psmCableIdentitiesDataCount > 0U)
            {
                pd_id_header_vdo_t *headerVDO = (pd_id_header_vdo_t *)((void *)&pdInstance->psmCableIdentities[1]);
                pd_structured_vdm_header_t *vdmHeader =
                    (pd_structured_vdm_header_t *)((void *)&pdInstance->psmCableIdentities[0]);
                plugInfo->vdmVersion = (uint8_t)vdmHeader->bitFields.vdmVersion;

                if (headerVDO->bitFields.productTypeUFPOrCablePlug == VDM_ID_HEADER_VDO_PASSIVE_CABLE_VAL)
                {
                    plugInfo->cableType = (uint8_t)kCableType_PassiveCable;
                }
                else if (headerVDO->bitFields.productTypeUFPOrCablePlug == VDM_ID_HEADER_VDO_ACTIVE_CABLE_VAL)
                {
                    plugInfo->cableType = (uint8_t)kCableType_ActiveCable;
                }
                else
                {
                    plugInfo->cableType = (uint8_t)kCableType_Invalid;
                }
                plugInfo->vdoValue = pdInstance->psmCableIdentities[4];
            }
            else
            {
                plugInfo->cableType = (uint8_t)kCableType_Invalid;
            }
            break;
        }

#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
        case PD_CONTROL_INFORM_EXTERNAL_POWER_STATE:
            if (pdInstance->rdySeenExtPower != *((uint8_t *)param))
            {
                PD_PsmExternalPowerChange(pdInstance);
            }
            break;
#endif

#if (defined PD_CONFIG_ALT_MODE_DP_SUPPORT) && (PD_CONFIG_ALT_MODE_DP_SUPPORT)
        case PD_CONTROL_ALT_MODE:
            PD_AltModeControl(pdInstance->altModeHandle, param);
            break;
#endif

#if (defined PD_CONFIG_PHY_LOW_POWER_LEVEL) && (PD_CONFIG_PHY_LOW_POWER_LEVEL)
        case PD_CONTROL_ENTER_LOW_POWER:
            (void)PD_PhyControl(pdInstance, PD_PHY_ENTER_LOW_POWER_STATE, NULL);
            break;

        case PD_CONTROL_EXIT_LOW_POWER:
            (void)PD_PhyControl(pdInstance, PD_PHY_EXIT_LOW_POWER_STATE, NULL);
            break;
        case PD_CONTROL_GET_PD_LOW_POWER_STATE:
            *((uint8_t *)param) = (pdInstance->lowPowerState != 0U) ? 1U : 0U;
            break;
#endif

        default:
            /* No action required. */
            break;
    }

    return status;
}

void PD_DpmAltModeCallback(pd_handle pdHandle, pd_dpm_callback_event_t event, void *param)
{
    (void)((pd_instance_t *)(pdHandle))->pdCallback(((pd_instance_t *)(pdHandle))->callbackParam, event, param);
}
