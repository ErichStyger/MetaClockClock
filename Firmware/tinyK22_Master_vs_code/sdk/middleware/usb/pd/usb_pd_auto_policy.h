/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_PD_AUTO_POLICY_H__
#define __USB_PD_AUTO_POLICY_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @addtogroup usb_pd_stack
 * @{
 */

/**
 * @brief pd auto policy accept or reject values.
 */
typedef enum _usb_pd_auto_accept_value
{
    /*! don't support */
    kAutoRequestProcess_NotSupport = 0x00u,
    /*! auto accept request */
    kAutoRequestProcess_Accept = 0x01u,
    /*! auto reject request */
    kAutoRequestProcess_Reject = 0x02u,
    /*! auto reply wait for request */
    kAutoRequestProcess_Wait = 0x03u,
} usb_pd_auto_accept_value_t;

/**
 * @brief pd auto policy configuration.
 */
typedef struct _usb_pd_auto_policy
{
    /*! 0 - don't support;
     *  1 - auto request pr_swap when current power role is source.
     */
    uint32_t autoRequestPRSwapAsSource : 1;
    /*! 0 - don't support;
     *  1 - auto request pr_swap when current power role is sink.
     */
    uint32_t autoRequestPRSwapAsSink : 1;
    /*! accept swap or not when current role is source
     * kAutoRequestProcess_Accept or kAutoRequestProcess_Reject
     */
    uint32_t autoAcceptPRSwapAsSource : 2;
    /*! accept swap or not when current role is sink
     * kAutoRequestProcess_Accept or kAutoRequestProcess_Reject
     */
    uint32_t autoAcceptPRSwapAsSink : 2;
    /*! #pd_data_role_t values
     * kPD_DataRoleUFP : auto request to UFP.
     * kPD_DataRoleDFP : auto request to DFP.
     * kPD_DataRoleNone : don't support auto request.
     */
    uint32_t autoRequestDRSwap : 2;
    /*! accept swap to DFP or not */
    uint32_t autoAcceptDRSwapToDFP : 2;
    /*! accept swap to UFP or not */
    uint32_t autoAcceptDRSwapToUFP : 2;
    /*! #pd_vconn_role_t values
     * kPD_NotVconnSource : auto request to turn off vconn.
     * kPD_IsVconnSource : auto request to turn on vconn.
     * kPD_VconnNone : don't support auto request.
     */
    uint32_t autoRequestVConnSwap : 2;
    /*! accept swap to trun on Vconn or not */
    uint32_t autoAcceptVconnSwapToOn : 2;
    /*! accept swap to trun off Vconn or not */
    uint32_t autoAcceptVconnSwapToOff : 2;
    /*! sink request the max power that statify self's sink caps; 1 - enable, 0 - don't enable */
    uint32_t autoSinkNegotiation : 1;
    /*! reserved bits */
    uint32_t reserved : 13;
    uint32_t reserved1 : 32;
} pd_auto_policy_t;

/*! @}*/

/*******************************************************************************
 * API
 ******************************************************************************/

#define PD_POLICY_SUPPORT(pdHandle) (((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig != NULL)

#define PD_POLICY_GET_AUTO_REQUEST_PRSWAP_AS_SOURCE(pdHandle) \
    ((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig))->autoRequestPRSwapAsSource
#define PD_POLICY_GET_AUTO_REQUEST_PRSWAP_AS_SINK(pdHandle) \
    ((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig))->autoRequestPRSwapAsSink

#define PD_POLICY_GET_AUTO_ACCEPT_PRSWAP_AS_SOURCE_SUPPORT(pdHandle)                          \
    (((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig)) \
         ->autoAcceptPRSwapAsSource != (uint8_t)kAutoRequestProcess_NotSupport)
#define PD_POLICY_GET_AUTO_ACCEPT_PRSWAP_AS_SOURCE(pdHandle) \
    ((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig))->autoAcceptPRSwapAsSource
#define PD_POLICY_GET_AUTO_ACCEPT_PRSWAP_AS_SINK_SUPPORT(pdHandle)                            \
    (((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig)) \
         ->autoAcceptPRSwapAsSink != (uint8_t)kAutoRequestProcess_NotSupport)
#define PD_POLICY_GET_AUTO_ACCEPT_PRSWAP_AS_SINK(pdHandle) \
    ((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig))->autoAcceptPRSwapAsSink
#define PD_POLICY_SET_AUTO_ACCEPT_PRSWAP_AS_SOURCE(pdHandle, val)                            \
    ((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig)) \
        ->autoAcceptPRSwapAsSource = val
#define PD_POLICY_SET_AUTO_ACCEPT_PRSWAP_AS_SINK(pdHandle, val)                                                        \
    ((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig))->autoAcceptPRSwapAsSink = \
        val

#define PD_POLICY_GET_AUTO_REQUEST_DRSWAP(pdHandle) \
    ((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig))->autoRequestDRSwap
#define PD_POLICY_GET_AUTO_ACCEPT_DRSWAP_AS_DFP_SUPPORT(pdHandle)                             \
    (((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig)) \
         ->autoAcceptDRSwapToDFP != (uint8_t)kAutoRequestProcess_NotSupport)
#define PD_POLICY_GET_AUTO_ACCEPT_DRSWAP_AS_DFP(pdHandle) \
    ((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig))->autoAcceptDRSwapToDFP
#define PD_POLICY_GET_AUTO_ACCEPT_DRSWAP_AS_UFP_SUPPORT(pdHandle)                             \
    (((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig)) \
         ->autoAcceptDRSwapToUFP != (uint8_t)kAutoRequestProcess_NotSupport)
#define PD_POLICY_GET_AUTO_ACCEPT_DRSWAP_AS_UFP(pdHandle) \
    ((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig))->autoAcceptDRSwapToUFP
#define PD_POLICY_SET_AUTO_REQUEST_DRSWAP(pdHandle, val) \
    ((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig))->autoRequestDRSwap = val
#define PD_POLICY_SET_AUTO_ACCEPT_DRSWAP_AS_DFP(pdHandle, val)                                                        \
    ((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig))->autoAcceptDRSwapToDFP = \
        val
#define PD_POLICY_SET_AUTO_ACCEPT_DRSWAP_AS_UFP(pdHandle, val)                                                        \
    ((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig))->autoAcceptDRSwapToUFP = \
        val

#define PD_POLICY_GET_AUTO_REQUEST_VCONNSWAP(pdHandle) \
    ((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig))->autoRequestVConnSwap
#define PD_POLICY_GET_AUTO_ACCEPT_VCONNSWAP_TURN_ON_SUPPORT(pdHandle)                         \
    (((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig)) \
         ->autoAcceptVconnSwapToOn != (uint8_t)kAutoRequestProcess_NotSupport)
#define PD_POLICY_GET_AUTO_ACCEPT_VCONNSWAP_TURN_OFF_SUPPORT(pdHandle)                        \
    (((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig)) \
         ->autoAcceptVconnSwapToOff != (uint8_t)kAutoRequestProcess_NotSupport)
#define PD_POLICY_GET_AUTO_SINK_NEGOTIATION_SUPPORT(pdHandle)                                                         \
    (((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig))->autoSinkNegotiation != \
     (uint8_t)kAutoRequestProcess_NotSupport)

#define PD_POLICY_GET_AUTO_ACCEPT_VCONNSWAP_TURN_ON(pdHandle) \
    ((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig))->autoAcceptVconnSwapToOn
#define PD_POLICY_GET_AUTO_ACCEPT_VCONNSWAP_TURN_OFF(pdHandle) \
    ((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig))->autoAcceptVconnSwapToOff
#define PD_POLICY_SET_AUTO_REQUEST_VCONNSWAP(pdHandle, val) \
    ((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig))->autoRequestVConnSwap = val
#define PD_POLICY_SET_AUTO_ACCEPT_VCONNSWAP_TURN_ON(pdHandle, val)                           \
    ((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig)) \
        ->autoAcceptVconnSwapToOn = val
#define PD_POLICY_SET_AUTO_ACCEPT_VCONNSWAP_TURN_OFF(pdHandle, val)                          \
    ((pd_auto_policy_t *)(((pd_instance_t *)pdHandle)->pdPowerPortConfig->autoPolicyConfig)) \
        ->autoAcceptVconnSwapToOff = val

#endif
