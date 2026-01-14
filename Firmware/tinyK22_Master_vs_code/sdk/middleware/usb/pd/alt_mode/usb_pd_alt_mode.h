/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_PD_ALT_MODE_H__
#define __USB_PD_ALT_MODE_H__

#if (defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT)
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define PD_ALT_MODE_MAX_MODULES         (7U)
#define PD_ALT_MODE_COMMAND_RETRY_COUNT (4U)

#define DP_SVID                           (0xFF01U)
#define PD_ALT_MODE_ERROR_RETRY_WAIT_TIME (10U) /* ms */

#define PD_ALT_MODE_EVENT_MODULES (0x000000FFu)
#define PD_ALT_MODE_EVENT_MODULE1 (0x00000001u)
#define PD_ALT_MODE_EVENT_MODULE2 (0x00000002u)
#define PD_ALT_MODE_EVENT_MODULE3 (0x00000004u)
#define PD_ALT_MODE_EVENT_MODULE4 (0x00000008u)
#define PD_ALT_MODE_EVENT_COMMAND (0x00000200u)

typedef void *pd_alt_mode_handle;

typedef enum _pd_alt_mode_callback_code
{
    kAltMode_EventInvalid = 0,
    kAltMode_Attach,
    kAltMode_Detach,
    kAltMode_HardReset,
    /* receive partner's vdm request msg */
    kAltMode_StructedVDMMsgReceivedProcess,
    /* slef send VMD message, and receive result (ACK) */
    kAltMode_StructedVDMMsgSuccess,
    /* slef send VMD message, (NAK, BUSY) or send success/fail */
    kAltMode_StructedVDMMsgFail,
    kAltMode_UnstructedVDMMsgReceived,
    kAltMode_UnstructedVDMMsgSentResult,
} pd_alt_mode_callback_code_t;

typedef struct _pd_alt_mode_state
{
    uint32_t mode;
    uint16_t SVID;
} pd_alt_mode_state_t;

/*!
 * @addtogroup usb_pd_stack
 * @{
 */

/**
 * @brief PD alternating mode control
 */
typedef enum _pd_alt_mode_control_code
{
    kAltMode_Invalid = 0,
    /*! start do the enter mode steps */
    kAltMode_TriggerEnterMode,
    /*! start do the exit mode steps */
    kAltMode_TriggerExitMode,
    /*! get the enter mode state */
    kAltMode_GetModeState,
    /*! there is new HPD detect evetns (event value is #pd_hpd_detect_type_t) */
    kDPControl_HPDDetectEvent,
} pd_alt_mode_control_code_t;

/**
 * @brief PD alternating mode's modules config
 */
typedef struct _pd_alt_mode_module
{
    /*! module's SVID */
    uint32_t SVID;
    /*! module specific configure parameter, for example: pd_alt_mode_dp_config_t for displayport host or slave */
    const void *config;
} pd_alt_mode_module_t;

/**
 * @brief PD alternating mode config
 */
typedef struct _pd_alt_mode_config
{
    /*! module's list, the first module has high priority */
    const pd_alt_mode_module_t *modules;
    /*! module count for modules */
    uint32_t moduleCount;
    /*! identity data buffer */
    uint32_t *identityData;
    /*! identity data size (unit is 4 bytes) */
    uint8_t identityObjectCount;
    /*! #typec_data_function_config_t values
     * kDataConfig_UFP: support alt mode and support UFP.
     * kDataConfig_DFP: support alt mode and support DFP.
     * kDataConfig_DRD: support alt mode and support DFP and UFP.
     * kDataConfig_None: don't support alt mode.
     */
    uint8_t altModeRole;
} pd_alt_mode_config_t;

/**
 * @brief PD alternating mode control used by PD_Control(PD_CONTROL_ALT_MODE)
 */
typedef struct _pd_alt_mode_control
{
    /*! #pd_alt_mode_control_code_t values */
    uint8_t controlCode;
    /*! conresponding to pd_alt_mode_config_t's modules' index;
     * 0 - the common alternating mode function;
     * 1 - the first moddule pd_alt_mode_config_t's modules;
     * n - the n module in pd_alt_mode_config_t's modules.
     */
    uint8_t altModeModuleIndex;
    /*! the control parameter */
    void *controlParam;
} pd_alt_mode_control_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* the whole alt mode instance task, it will trigger modules' task if call PD_AltModeModuleTaskWakeUp */
/*!
 * @brief PD stack Alternating Mode task function
 *
 * User need keep calling this function endlessly in the PD application using one task.
 */
void PD_AltModeTask(void);

#ifdef __cplusplus
}
#endif

/*! @}*/

/* initialize whole alt mode module */
pd_status_t PD_AltModeInit(void);
/* de-initialize whole alt mode module */
pd_status_t PD_AltModeDeinit(void);
/* initialize one alt mode instance related to one pd instance */
pd_status_t PD_AltModeInstanceInit(pd_handle pdHandle,
                                   const pd_alt_mode_config_t *altModeConfig,
                                   pd_alt_mode_handle *altModeHandle);
/* de-initialize one alt mode instance */
pd_status_t PD_AltModeInstanceDeinit(pd_alt_mode_handle altModeHandle);

/* In this implementation, use one task to process all modules' task. so module can use this API to wake up self task to
 * excute. */
void PD_AltModeModuleTaskWakeUp(pd_alt_mode_handle altModeHandle, void *moduleHandle);

/* this function will be called in one 1ms timer ISR */
void PD_AltModeTimer1msISR(void);

/* PD stack callback events */
pd_status_t PD_AltModeCallback(pd_handle pdHandle, uint32_t event, void *param);

/* get the alt mode state, 0 - not in alt mode; 1 - in alt mode */
pd_status_t PD_AltModeState(pd_handle pdHandle, uint8_t *state);

void PD_AltModeControl(void *altModeHandle, void *controlParam);

/* when pd policy do the dr swap, need check whether in alt mode;
 * If in alt mode, need call this function get the dr swap state.
 * 0 - not in alt mode, or has exited alt mode;
 * 1 - trigger exiting alt mode, then do dr swap;
 */
uint8_t PD_AltModeExitModeForDrSwap(pd_alt_mode_handle altModeHandle);
uint8_t PD_AltModeCheckModalOperationSupported(void *altModeHandle);

#endif

#endif
