/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_PD_ALT_MODE_DP_H__
#define __USB_PD_ALT_MODE_DP_H__

#if (defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT)
#if (defined PD_CONFIG_ALT_MODE_DP_SUPPORT) && (PD_CONFIG_ALT_MODE_DP_SUPPORT)

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define PD_ALT_MODE_LOG (0U)

#define PD_DP_DFP_ENABLE (1U)
#define PD_DP_UFP_ENABLE (1U)

/* #define DP_TASK_EVENT_DFP_RETRY_COMMAND (0x00000001u) */
/* #define DP_TASK_EVENT_DFP_DP_CHECK_UFP_STATUS (0x00000002u) */
/* #define DP_TASK_EVENT_DFP_SEND_COMMAND (0x00000004u) */
/* #define DP_TASK_EVENT_DFP_EXIT_MODE_TRIGGER (0x00000008u) */
#define DP_TASK_EVENT_DFP_HPD_PROCESS (0x00000010u)

/* displayport alt mode's state (enter, configured etc) */
typedef enum _pd_dp_mode_state
{
    kDPMode_Invalid = 0,      /* DFP/UFP */
    kDPMode_Exited,           /* DFP/UFP */
    kDPMode_EnterDPDone,      /* DFP/UFP */
    kDPMode_StatusUpdateDone, /* DFP/UFP */
    kDPMode_ConfigureDone,    /* DFP/UFP */
} pd_dp_mode_state_t;

/*!
 * @addtogroup usb_pd_stack
 * @{
 */

/**
 * @brief PD DisplayPort drive the HPD
 */
typedef enum _pd_dp_hpd_driver
{
    /*! invalid value */
    kDPHPDDriver_None = 0,
    /*! IRQ */
    kDPHPDDriver_IRQ,
    /*! LOW */
    kDPHPDDriver_Low,
    /*! HIGH */
    kDPHPDDriver_High,
    /*! wait */
    kDPHPDDriver_Waiting,
} pd_dp_hpd_driver_t;

#if (defined PD_CONFIG_ALT_MODE_SLAVE_SUPPORT) && (PD_CONFIG_ALT_MODE_SLAVE_SUPPORT)
/**
 * @brief PD DisplayPort HPD detection type
 */
typedef enum _pd_hpd_detect_type
{
    /*! invalid value */
    kDPHPDDetect_Empty = 0,
    /*! IRQ */
    kDPHPDDetect_IRQ,
    /*! LOW */
    kDPHPDDetect_Low,
    /*! HIGH */
    kDPHPDDetect_High,
} pd_hpd_detect_type_t;
#endif

/**
 * @brief PD DisplayPort VDM command
 */
typedef enum _pd_displayport_vdm_command
{
    /*! status update command */
    kDPVDM_StatusUpdate = 0x10,
    /*! dp configure */
    kDPVDM_Configure,
} pd_displayport_vdm_command_t;

/**
 * @brief PD DisplayPort connect state
 */
typedef enum _pd_status_connected_val
{
    /*! Neither DFP_D nor UFP_D is connected, or Adaptor is disabled */
    kDFP_D_NonConnected = 0,
    /*! DFP_D is connected */
    kDFP_D_Connected = 1,
    /*! UFP_D is connected */
    kUFP_D_Connected = 2,
    /*! Both DFP_D and UFP_D are connected */
    kUFP_D_BothConnected = 3,
} pd_status_connected_val_t;

/**
 * @brief PD DisplayPort select configuration
 */
typedef enum _pd_configure_set_config_val
{
    /*! Set configuration for USB */
    kDPConfig_USB = 0,
    /*! Set configuration for UFP_U as DFP_D */
    kDPConfig_DFPD = 1,
    /*! Set configuration for UFP_U as UFP_D */
    kDPConfig_UFPD = 2,
} pd_configure_set_config_val_t;

/**
 * @brief PD DisplayPort port capability
 */
typedef enum _dp_mode_port_cap_val
{
    /*! RESERVED */
    kDPPortCap_Reserved = 0,
    /*! UFP_D-capable (including Branch device */
    kDPPortCap_UFPD = 1,
    /*! DFP_D-capable (including Branch device) */
    kDPPortCap_DFPD = 2,
    /*! Both DFP_D and UFP_D-capable */
    kDPPortCap_Both = 3,
} dp_mode_port_cap_val_t;

/**
 * @brief Signaling for Transport of DisplayPort Protocol
 */
typedef enum _dp_mode_signal_val
{
    /*! Signaling unspecified (used only when Select Configuration field is set for USB Configuration) */
    kDPSignal_Unspecified = 0,
    /*! Select DP v1.3 signaling rates and electrical settings */
    kDPSignal_DP = 1,
    /*! Select Gen 2 signaling rates and electrical specifications */
    kDPSignal_USBGEN2 = 2,
} dp_mode_signal_t;

/**
 * @brief Configure Pin Assignment
 */
typedef enum _dp_mode_pin_assign_val
{
    /*! De-select pin assignment */
    kPinAssign_DeSelect = 0,
    /*! GEN2_BR signal, 4 lanes */
    kPinAssign_A = 0x01u,
    /*! GEN2_BR signal, 2 lanes */
    kPinAssign_B = 0x02u,
    /*! DP_BR signal, 4 lanes */
    kPinAssign_C = 0x04u,
    /*! DP_BR signal, 2 lanes */
    kPinAssign_D = 0x08u,
    /*! DP_BR signal, 4 lanes */
    kPinAssign_E = 0x10u,
    /*! Select Pin Assignment F */
    kPinAssign_F   = 0x20u,
    kPinAssign_VR  = 0x80u,
    kPinAssign_All = 0xFFu,
} dp_mode_pin_assign_val_t;

/**
 * @brief Receptacle Indication
 */
typedef enum _dp_mode_receptacle_indication
{
    /*! DisplayPort interface is presented on a USB Type-C Plug */
    kReceptacle_TypeCPlug = 0,
    /*! DisplayPort interface is presented on a USB Type-C Receptacle */
    kReceptacle_TypeCReceptacle = 1,
} dp_mode_receptacel_indication_t;

/**
 * @brief USB r2.0 Signaling Not Used
 */
typedef enum _dp_mode_usb20_signal
{
    /*! USB r2.0 signaling may be required on A6 ¨C A7 or B6 ¨C B7 while in DisplayPort Configuration */
    kUSB2_Required = 0,
    /*! USB r2.0 signaling is not required on A6 ¨C A7 or B6 ¨C B7 while in DisplayPort Configuration */
    kUSB2_NotRequired = 1,
} dp_mode_usb20_signal_t;

/**
 * @brief DisplayPort Capabilities (VDO in Responder Discover Modes VDM)
 */
typedef struct _pd_dp_mode_obj
{
    union
    {
        /*! union DP mode object */
        struct
        {
            /*! Port Capability */
            uint32_t portCap : 2;
            /*! Signaling for Transport of DisplayPort Protocol */
            uint32_t signalSupport : 4;
            /*! Receptacle Indication */
            uint32_t receptacleIndication : 1;
            /*! USB r2.0 Signaling Not Used */
            uint32_t usb2SignalNotUsed : 1;
            /*! DFP_D Pin Assignments Supported (reported by a DFP_D receptacle or UFP_D (direct attach) plug) */
            uint32_t DFPDPinSupport : 8;
            /*! UFP_D Pin Assignments Supported (reported by a UFP_D receptacle or DFP_D (direct attach) plug) */
            uint32_t UFPDPinSupport : 8;
            uint32_t reseerved : 8;
        } bitFields;
        /*! union 32bits value */
        uint32_t modeVal;
    };
} pd_dp_mode_obj_t;

/**
 * @brief DisplayPort Status
 */
typedef struct _pd_dp_status_obj
{
    union
    {
        /*! union DP status object */
        struct
        {
            /*! DFP_D/UFP_D Connected */
            uint32_t DFPDUFPDConnected : 2;
            /*! Power Low */
            uint32_t powerLow : 1;
            /*! Enabled */
            uint32_t enabled : 1;
            /*! Multi-function Preferred */
            uint32_t multiFunctionPreferred : 1;
            /*! USB Configuration Request */
            uint32_t USBConfigReq : 1;
            /*! Exit DisplayPort Mode Request */
            uint32_t exitDPModeReq : 1;
            /*! HPD State */
            uint32_t HPDState : 1;
            /*! IRQ_HPD */
            uint32_t HPDInterrupt : 1;
            uint32_t reserved : 23;
        } bitFields;
        /*! union 32bits value */
        uint32_t statusVal;
    };
} pd_dp_status_obj_t;

/**
 * @brief DisplayPort Configurations
 */
typedef struct _pd_dp_configure_obj
{
    union
    {
        /*! union DP configure object */
        struct
        {
            /*! Select Configuration */
            uint32_t setConfig : 2;
            /*! Signaling for Transport of DisplayPort Protocol */
            uint32_t setSignal : 4;
            /*! reserved */
            uint32_t reserved : 2;
            /*! Configure UFP_U Pin Assignment */
            uint32_t configureUFPUPin : 8;
            /*! reserved */
            uint32_t reserved1 : 16;
        } bitFields;
        /*! union 32bits value */
        uint32_t configureVal;
    };
} pd_dp_configure_obj_t;

/**
 * @brief DisplayPort peripheral control
 */
typedef enum _pd_dp_peripheral_control
{
    /*! control HPD */
    kDPPeripheal_ControlHPDValue,
    /*! set HPD low */
    kDPPeripheal_ControlHPDSetLow,
    /*! release HPD */
    kDPPeripheal_ControlHPDReleaseLow,
    /*! set mux */
    kDPPeripheal_ControlSetMux,
    /*! set mux safe mode */
    kDPPeripheal_ControlSetMuxSaftMode,
    /*! set mux usb3 only */
    kDPPeripheal_ControlSetMuxUSB3Only,
    /*! shut mux */
    kDPPeripheal_ControlSetMuxShutDown,
    /*! disable mux */
    kDPPeripheal_ControlSetMuxDisable,
    /*! set mux dp4lane */
    kDPPeripheal_ControlSetMuxDP4LANE,
    /*! set mux dp2land and usb3 */
    kDPPeripheal_ControlSetMuxDP2LANEUSB3,
    /*! set mux dp2lane no usb */
    kDPPeripheal_ControlSetMuxDP2LANENOUSB,
#ifdef USBPD_ENABLE_FAST_CENTER_ALT_MODE_CONTROL
    kDPPeripheal_ControlSetMuxDP4LANEUSB3,
#endif
    /*! start HPD detect */
    kDPPeripheal_ControlHPDDetectStart,
    /*! stop HPD detect */
    kDPPeripheal_ControlHPDDetectStop,
    /*! enable HPD queue */
    kDPPeripheal_ControlHPDQueueEnable,
    /*! disable HPD queue */
    kDPPeripheal_ControlHPDQueueDisable,
    /*! get HPD detect state value */
    kDPPeripheal_ControlHPDGetCurrentState,
} pd_dp_peripheral_control_t;

/**
 * @brief DisplayPort mode select callback parameter
 */
typedef struct _pd_altmode_dp_modes_sel
{
    /*! The all modes buffer */
    pd_dp_mode_obj_t *modes;
    /*! The all modes count */
    uint8_t modesCount;
    /*! application return this value to stack, which one application select
     *  0 - all modes are not supported and select none.
     *  1 - modesCount: the mode that is selected.
     */
    uint8_t selectIndex;
    /*! the pin assign selected by application.
     * it is based on the selected mode.
     */
    uint8_t selectPinAssign;
} pd_altmode_dp_modes_sel_t;

/**
 * @brief DisplayPort mode board peripheral interface
 */
typedef struct _pd_dp_board_peripheral_interface
{
    /*! displayport peripheral init */
    pd_status_t (*dpPeripheralInit)(void **interfaceHandle, pd_handle pdHandle, const void *param);
    /*! displayport peripheral de-init */
    pd_status_t (*dpPeripheralDeinit)(void *interfaceHandle);
    /*! displayport peripheral control */
    pd_status_t (*dpPeripheralControl)(void *interfaceHandle, uint32_t opCode, void *opParam);
} pd_dp_peripheral_interface_t;

/**
 * @brief DisplayPort mode config
 */
typedef struct _pd_alt_mode_displayport_config
{
    /*! customer can define this structure by self
     * For example: hpd/crossbar driver can differ for different customers' solution.
     */
    const void *peripheralConfig;
    /*! pd_dp_peripheral_interface_t */
    const pd_dp_peripheral_interface_t *peripheralInterface;
    /*! modes list */
    uint32_t *modesList;
    /*! modes count */
    uint32_t modesCount;
    /*! DisplayPort status update config, used by slave */
    pd_dp_status_obj_t dpStatusConfig;
    /*! DisplayPort configurations config, used by host */
    pd_dp_configure_obj_t dpConfigurationsConfig;
} pd_alt_mode_dp_config_t;

/*! @}*/

/*******************************************************************************
 * API
 ******************************************************************************/

pd_status_t PD_DPInit(pd_handle pdHandle, void *altModeHandle, const void *moduleConfig, void **moduleInstance);
pd_status_t PD_DPDeinit(void *moduleInstance);
pd_status_t PD_DPControl(void *moduleInstance, uint32_t controlCode, void *controlParam);
pd_status_t PD_DPCallbackEvent(void *moduleInstance, uint32_t processCode, uint16_t msgSVID, void *param);
void PD_DPTask(void *taskParam);
void PD_DPModule1msISR(void *moduleInstance);

#endif
#endif
#endif
