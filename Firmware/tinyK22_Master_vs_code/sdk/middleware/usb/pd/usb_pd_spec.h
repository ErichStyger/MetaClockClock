/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PD_SPEC_H__
#define __PD_SPEC_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define PD_MSG_HEADER_NUMBER_OF_DATA_OBJECTS_POS  (12U)
#define PD_MSG_HEADER_NUMBER_OF_DATA_OBJECTS_MASK (0x7000u)
#define PD_MSG_HEADER_PORT_POWER_ROLE_POS         (8U)
#define PD_MSG_HEADER_PORT_POWER_ROLE_MASK        (0x0100u)
#define PD_MSG_HEADER_SPEC_REV_POS                (6U)
#define PD_MSG_HEADER_PORT_DATA_ROLE_POS          (5U)
#define PD_MSG_HEADER_PORT_DATA_ROLE_MASK         (0x0020u)
#define PD_MSG_HEADER_MESSAGE_TYPE_POS            (0U)
#define PD_MSG_HEADER_MESSAGE_TYPE_MASK           (0x001Fu)
#define PD_MSG_HEADER_EXTENDED_MASK               (0x8000u)
#define PD_MSG_HEADER_EXTENDED_POS                (15U)

#define PD_MSG_EXT_HEADER_CHUNKED_MASK       (0x8000u)
#define PD_MSG_EXT_HEADER_CHUNKED_POS        (15U)
#define PD_MSG_EXT_HEADER_CHUNK_NUMBER_MASK  (0x7800u)
#define PD_MSG_EXT_HEADER_CHUNK_NUMBER_POS   (11U)
#define PD_MSG_EXT_HEADER_REQUEST_CHUNK_MASK (0x0400u)
#define PD_MSG_EXT_HEADER_REQUEST_CHUNK_POS  (10U)
#define PD_MSG_EXT_HEADER_DATA_SIZE_MASK     (0x01FFu)
#define PD_MSG_EXT_HEADER_DATA_SIZE_POS      (0U)

#define PD_MSG_CONTROL_TYPE_MASK (0x00u)
#define PD_MSG_DATA_TYPE_MASK    (0x80u)
#define PD_MSG_EXT_TYPE_MASK     (0xC0u)
#define PD_MSG_TYPE_BITS_MASK    (0xC0u)
#define PD_MSG_TYPE_VALUE_MASK   (0x3Fu)

typedef enum _message_type
{
    kPD_MsgInvalid              = 0x00u,
    kPD_MsgGoodCRC              = 0x01u,
    kPD_MsgGotoMin              = 0x02u,
    kPD_MsgAccept               = 0x03u,
    kPD_MsgReject               = 0x04u,
    kPD_MsgPing                 = 0x05u,
    kPD_MsgPsRdy                = 0x06u,
    kPD_MsgGetSourceCap         = 0x07u,
    kPD_MsgGetSinkCap           = 0x08u,
    kPD_MsgDrSwap               = 0x09u,
    kPD_MsgPrSwap               = 0x0Au,
    kPD_MsgVconnSwap            = 0x0Bu,
    kPD_MsgWait                 = 0x0Cu,
    kPD_MsgSoftReset            = 0x0Du,
    kPD_MsgReserved1            = 0x0Eu,
    kPD_MsgReserved2            = 0x0Fu,
    kPD_MsgNotSupported         = 0x10u,
    kPD_MsgGetSourceCapExtended = 0x11u,
    kPD_MsgGetStatus            = 0x12u,
    kPD_MsgFrSwap               = 0x13u,
    kPD_MsgGetPpsStatus         = 0x14u,
    kPD_MsgSourceCapabilities   = (PD_MSG_DATA_TYPE_MASK | 0x01u),
    kPD_MsgRequest              = (PD_MSG_DATA_TYPE_MASK | 0x02u),
    kPD_MsgBIST                 = (PD_MSG_DATA_TYPE_MASK | 0x03u),
    kPD_MsgSinkCapabilities     = (PD_MSG_DATA_TYPE_MASK | 0x04u),
    kPD_MsgBatteryStatus        = (PD_MSG_DATA_TYPE_MASK | 0x05u),
    kPD_MsgAlert                = (PD_MSG_DATA_TYPE_MASK | 0x06u),
    kPD_MsgVendorDefined        = (PD_MSG_DATA_TYPE_MASK | 0x0Fu),

    kPD_MsgSourceCapExtended      = (PD_MSG_EXT_TYPE_MASK | 0x01u),
    kPD_MsgStatus                 = (PD_MSG_EXT_TYPE_MASK | 0x02u),
    kPD_MsgGetBatteryCap          = (PD_MSG_EXT_TYPE_MASK | 0x03u),
    kPD_MsgGetBatteryStatus       = (PD_MSG_EXT_TYPE_MASK | 0x04u),
    kPD_MsgBatteryCapabilities    = (PD_MSG_EXT_TYPE_MASK | 0x05u),
    kPD_MsgGetManufacturerInfo    = (PD_MSG_EXT_TYPE_MASK | 0x06u),
    kPD_MsgManufacturerInfo       = (PD_MSG_EXT_TYPE_MASK | 0x07u),
    kPD_MsgSecurityRequest        = (PD_MSG_EXT_TYPE_MASK | 0x08u),
    kPD_MsgSecurityResponse       = (PD_MSG_EXT_TYPE_MASK | 0x09u),
    kPD_MsgFirmwareUpdateRequest  = (PD_MSG_EXT_TYPE_MASK | 0x0Au),
    kPD_MsgFirmwareUpdaetResponse = (PD_MSG_EXT_TYPE_MASK | 0x0Bu),
    kPD_MsgPpsStatus              = (PD_MSG_EXT_TYPE_MASK | 0x0Cu),
} message_type_t;

typedef enum _extended_message_type
{
    kPD_ExtMsgSourceCapExtended = 1u,
    kPD_ExtMsgStatus,
    kPD_ExtMsgGetBatteryCap,
    kPD_ExtMsgGetBatteryStatus,
    kPD_ExtMsgBatteryCapabilities,
    kPD_ExtMsgGetManufacturerInfo,
    kPD_ExtmsgManufacturerInfo,
    kPD_ExtMsgSecurityRequest,
    kPD_ExtMsgSecurityResponse,
    kPD_ExtMsgFirmwareUpdateRequest,
    kPD_ExtMsgFirmwareUpdaetResponse,
} extended_message_type_t;

typedef enum _pd_psm_state
{
    PSM_UNKNOWN                      = 0, /* Internal state */
    PSM_IDLE                         = 1, /* Internal state */
    PSM_PE_SRC_STARTUP               = 2,
    PSM_PE_SRC_DISCOVERY             = 3,
    PSM_PE_SRC_SEND_CAPABILITIES     = 4,
    PSM_PE_SRC_NEGOTIATE_CAPABILITY  = 5,
    PSM_PE_SRC_TRANSITION_SUPPLY     = 6,
    PSM_PE_SRC_READY                 = 7,
    PSM_PE_SRC_DISABLED              = 8,
    PSM_PE_SRC_CAPABILITY_RESPONSE   = 9,
    PSM_PE_SRC_WAIT_NEW_CAPABILITIES = 10,

    PSM_HARD_RESET                   = 11, /* Source and Sink */
    PSM_PE_SRC_TRANSITION_TO_DEFAULT = 12,
    PSM_PE_SRC_GIVE_SOURCE_CAP       = 13,
    PSM_PE_SRC_GET_SINK_CAP          = 14,

    PSM_PE_DR_SRC_GET_SOURCE_CAP = 15,
    PSM_PE_DR_SRC_GIVE_SINK_CAP  = 16,

    PSM_SEND_SOFT_RESET = 17, /* source and sink */
    PSM_SOFT_RESET      = 18, /* source and sink */

    PSM_PE_PRS_SRC_SNK_EVALUATE_PR_SWAP,
    PSM_PE_PRS_SRC_SNK_ACCEPT_PR_SWAP,
    PSM_PE_PRS_SRC_SNK_TRANSITION_TO_OFF,
    PSM_PE_PRS_SRC_SNK_ASSERT_RD,
    PSM_PE_PRS_SRC_SNK_WAIT_SOURCE_ON,
    PSM_PE_PRS_SRC_SNK_SEND_PR_SWAP,
    PSM_PE_PRS_SRC_SNK_REJECT_PR_SWAP,

    PSM_PE_SNK_STARTUP,
    PSM_PE_SNK_DISCOVERY,
    PSM_PE_SNK_WAIT_FOR_CAPABILITIES,
    PSM_PE_SNK_EVALUATE_CAPABILITY,
    PSM_PE_SNK_SELECT_CAPABILITY,
    PSM_PE_SNK_SELECT_CAPABILITY_WAIT_TIMER_TIME_OUT,
    PSM_PE_SNK_TRANSITION_SINK,
    PSM_PE_SNK_READY,
    PSM_PE_SNK_TRANSITION_TO_DEFAULT,
    PSM_PE_SNK_GIVE_SINK_CAP,
    PSM_PE_SNK_GET_SOURCE_CAP,

    PSM_PE_DR_SNK_GET_SINK_CAP,
    PSM_PE_DR_SNK_GIVE_SOURCE_CAP,

    PSM_PE_PRS_EVALUATE_PR_SWAP_WAIT_TIMER_TIME_OUT,
    PSM_PE_PRS_SNK_SRC_EVALUATE_PR_SWAP,
    PSM_PE_PRS_SNK_SRC_ACCEPT_PR_SWAP,
    PSM_PE_PRS_SNK_SRC_TRANSITION_TO_OFF,
    PSM_PE_PRS_SNK_SRC_ASSERT_RP,
    PSM_PE_PRS_SNK_SRC_SOURCE_ON,
    PSM_PE_PRS_SNK_SRC_SEND_PR_SWAP,
    PSM_PE_PRS_SNK_SRC_REJECT_PR_SWAP,

    /* BIST STATES */
    PSM_PE_BIST_CARRIER_MODE_2,
    PSM_PE_BIST_TEST_DATA_MODE,
    /* New States */
    PSM_CHECK_ASYNC_RX, /* Internal state: Cover the case where an unexpected packet was received during transmit, check
                         */
    /* for valid packets. */
    PSM_CHECK_SINK_SOURCE_CAP_RX, /* Internal state: cover the case where an unexpected packet is received in a sink
                                     state. */
    PSM_BYPASS,                   /* Internal state: PD operation bypassed */
    /* Type-C additions */

    PSM_PE_DRS_EVALUATE_DR_SWAP,
    PSM_PE_DRS_EVALUATE_DR_SWAP_WAIT_TIMER_TIME_OUT,
    PSM_PE_DRS_EVALUATE_DR_SWAP_WAIT_ALT_MODE_EXIT,
    PSM_PE_DRS_REJECT_DR_SWAP,
    PSM_PE_DRS_SEND_DR_SWAP,
    PSM_PE_DRS_ACCEPT_DR_SWAP,
    PSM_PE_DRS_CHANGE_TO_DFP_OR_UFP,

    PSM_EXIT_TO_ERROR_RECOVERY,
    PSM_EXIT_TO_ERROR_RECOVERY_WITH_DELAY,

    /* NOTE: unattached UFP, DFP states are return to CLP */
    /* Type-C VCONN Swap */
    PSM_PE_VCS_SEND_SWAP,
    PSM_PE_VCS_EVALUATE_SWAP,
    PSM_PE_VCS_ACCEPT_SWAP,
    PSM_PE_VCS_REJECT_SWAP,
    PSM_PE_VCS_WAIT_FOR_VCONN,
    PSM_PE_VCS_TURN_ON_VCONN,
    PSM_PE_VCS_TURN_OFF_VCONN,
    PSM_PE_VCS_SEND_PS_RDY,

    /* UFP VDM */
    PSM_PE_UFP_VDM_GET_IDENTITY,
    PSM_PE_UFP_VDM_SEND_IDENTITY,
    PSM_PE_UFP_VDM_GET_SVIDS,
    PSM_PE_UFP_VDM_SEND_SVIDS,
    PSM_PE_UFP_VDM_GET_MODES,
    PSM_PE_UFP_VDM_SEND_MODES,
    PSM_PE_UFP_VDM_EVALUATE_MODE_ENTRY,
    PSM_PE_UFP_VDM_MODE_ENTRY_ACK,
    PSM_PE_UFP_VDM_MODE_ENTRY_NAK,
    PSM_PE_UFP_VDM_MODE_EXIT,
    PSM_PE_UFP_VDM_MODE_EXIT_ACK,
    PSM_PE_UFP_VDM_MODE_EXIT_NAK,
    /* UFP VDM Attention */
    NO_PE_UFP_VDM_ATTENTION_REQUEST,
    /* DFP to UFP VDM Discover Identity */
    PSM_PE_DFP_UFP_VDM_IDENTITY_REQUEST,
    PSM_PE_DFP_UFP_VDM_IDENTITY_ACKED,
    PSM_PE_DFP_UFP_VDM_IDENTITY_NAKED,
    PSM_PE_DFP_UFP_VDM_VDM_BUSY_WAIT,

    /* DFP to Cable Plug VDM Discover Identity */
    PSM_PE_DFP_CBL_VDM_IDENTITY_REQUEST,
    DEP_PE_DFP_CBL_VDM_IDENTITY_ACKED,
    DEP_PE_DFP_CBL_VDM_IDENTITY_NAKED,

    /* DFP VDM Discover SVIDs */
    PSM_PE_DFP_VDM_SVIDS_REQUEST,
    PSM_PE_DFP_VDM_SVIDS_ACKED,
    PSM_PE_DFP_VDM_SVIDS_NAKED,
    /* DFP VDM Discover Modes */
    PSM_PE_DFP_VDM_MODES_REQUEST,
    PSM_PE_DFP_VDM_MODES_ACKED,
    PSM_PE_DFP_VDM_MODES_NAKED,
    /* DFP VDM Mode Entry */
    PSM_PE_DFP_VDM_MODE_ENTRY_REQUEST,
    PSM_PE_DFP_VDM_MODE_ENTRY_ACKED,
    PSM_PE_DFP_VDM_MODE_ENTRY_NAKED,
    /* DFP VDM Mode Exit */
    PSM_PE_DFP_VDM_MODE_EXIT_REQUEST,
    PSM_PE_DFP_VDM_MODE_EXIT_ACKED,
    PSM_PE_DFP_VDM_MODE_EXIT_HARD_RESET,

    /* Source Startup VDM Discover Identity */
    PSM_PE_SRC_VDM_IDENTITY_REQUEST,
    PSM_PE_SRC_VDM_IDENTITY_ACKED,
    PSM_PE_SRC_VDM_IDENTITY_NAKED,

    /* DFP VDM Attention */
    PSM_PE_DFP_VDM_ATTENTION_REQUEST,

    PSM_PE_DFP_CBL_SEND_SOFT_RESET,
    PSM_PE_DFP_CBL_SEND_CABLE_RESET,
    PSM_PE_UFP_CBL_SEND_SOFT_RESET,

    PSM_PE_SRC_HARD_RESET_RECEIVED,

    PSM_INTERRUPTED_REQUEST,

    PSM_PE_DFP_CBL_VDM_SVIDS_REQUEST,
    PSM_PE_DFP_CBL_VDM_MODES_REQUEST,
    PSM_PE_DFP_CBL_P_VDM_MODE_ENTRY_REQUEST,
    PSM_PE_DFP_CBL_PP_VDM_MODE_ENTRY_REQUEST,
    PSM_PE_DFP_CBL_P_VDM_MODE_EXIT_REQUEST,
    PSM_PE_DFP_CBL_PP_VDM_MODE_EXIT_REQUEST,

    PSM_PE_DFP_UFP_VDM_BUSY,

    PSM_PE_SRC_IMPLICIT_CABLE_SOFT_RESET,
    PSM_PE_SRC_IMPLICIT_CABLE_DISCOVERY,

    PSM_INTERRUPTED_VDM_RESPONSE,

    /* vendor structured vdm */
    PSM_PE_VENDOR_STRUCTURED_VDM_REQUEST,
    PSM_PE_VENDOR_STRUCTURED_VDM_ACKED,
    PSM_PE_VENDOR_STRUCTURED_VDM_NAKED,

    /* unstructured vdm */
    PSM_PD_SEND_UNSTRUCTURED_VDM,

    /* get source extended cap */
    PE_SNK_GET_SOURCE_CAP_EXT,
    PE_DR_SRC_GET_SOURCE_CAP_EXT,
    PE_SRC_GIVE_SOURCE_CAP_EXT,
    PE_DR_SNK_GIVE_SOURCE_CAP_EXT,

    /* get status */
    PE_SNK_Get_Source_Status,
    PE_SRC_Give_Source_Status,
    PE_SRC_Get_Sink_Status,
    PE_SNK_Give_Sink_Status,

    /* get pps status */
    PE_SNK_Get_PPS_Status,
    PE_SRC_Give_PPS_Status,

    /* get battery cap */
    PE_Get_Battery_Cap,
    PE_Give_Battery_Cap,

    /* get battery status */
    PE_Get_Battery_Status,
    PE_Give_Battery_Status,

    /* get manufacturer info */
    PE_Get_Manufacturer_Info,
    PE_Give_Manufacturer_Info,

    /* security request */
    PE_Send_Security_Request,
    PE_Send_Security_Response,
    PE_Security_Response_Received,

    /* alert */
    PE_SRC_Send_Source_Alert,
    PE_SNK_Source_Alert_Received,
    PE_SNK_Send_Sink_Alert,
    PE_SRC_Sink_Alert_Received,

    /* fast role swap */
    PE_FRS_SRC_SNK_CC_Signal,
    PE_FRS_SRC_SNK_Evaluate_Swap,
    PE_FRS_SRC_SNK_Accept_Swap,
    PE_FRS_SRC_SNK_Transition_to_off,
    PE_FRS_SRC_SNK_Assert_Rd,
    PE_FRS_SRC_SNK_Wait_Source_on,

    PE_FRS_SNK_SRC_Send_Swap,
    PE_FRS_SNK_SRC_Transition_to_off,
    PE_FRS_SNK_SRC_Vbus_Applied,
    PE_FRS_SNK_SRC_Assert_Rp,
    PE_FRS_SNK_SRC_Source_on,

    PE_PSM_STATE_ROLE_RDY_STATE,
    PE_PSM_STATE_NO_CHANGE,
} pd_psm_state_t;

typedef enum
{
    TYPEC_DISABLED        = 0,
    TYPEC_ERROR_RECOVERY  = 1,
    TYPEC_UNATTACHED_SRC  = 2,
    TYPEC_UNATTACHED_SNK  = 3,
    TYPEC_ATTACH_WAIT_SRC = 4,
    TYPEC_ATTACH_WAIT_SNK = 5,
    TYPEC_ATTACHED_SRC    = 6,
    TYPEC_ATTACHED_SNK    = 7,

    TYPEC_TRY_SRC               = 8,
    TYPEC_TRY_WAIT_SNK          = 9,
    TYPEC_TRY_SNK               = 10,
    TYPEC_TRY_WAIT_SRC          = 11,
    TYPEC_AUDIO_ACCESSORY       = 12,
    TYPEC_UNATTACHED_ACCESSORY  = 13,
    TYPEC_ATTACH_WAIT_ACCESSORY = 14,
    TYPEC_POWERED_ACCESSORY     = 15,
    TYPEC_UNSUPPORTED_ACCESSORY = 16,

    TYPEC_UNORIENTED_DEBUG_ACCESSORY_SRC = 17,
    TYPEC_ORIENTED_DEBUG_ACCESSORY_SRC   = 18,
    TYPEC_DEBUG_ACCESSORY_SNK            = 19,
    TYPEC_ORIENTED_DEBUG_ACCESSORY_SNK   = 20,
    TYPEC_TOGGLE_SRC_FIRST               = 21,
    TYPEC_TOGGLE_SNK_FIRST               = 22,
    TYPEC_DEAD_BATTERY_SNK               = 23,

    TYPEC_INVALID_STATE = 0xFFU,
} TypeCState_t;

typedef enum
{
    RCH_Wait_For_Message_From_Protocol_Layer,
    RCH_Pass_Up_Message,
    RCH_Processing_Extended_Message,
    RCH_Requesting_Chunk,
    RCH_Waiting_Chunk,
    RCH_Report_Error,
    TCH_Wait_For_Message_Request_From_Policy_Engine,
    TCH_Report_Error,
    TCH_Prepare_To_Send_Chunked_Message,
    TCH_Construct_Chunked_Message,
    TCH_Sending_Chunked_Message,
    TCH_Wait_Chunk_Request,
    TCH_Message_Received,
    TCH_Message_Sent,
} pd_chunking_layer_state_t;

#endif
