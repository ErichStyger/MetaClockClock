/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PD_CONFIG_H__
#define __PD_CONFIG_H__

#include "usb_pd.h"

/*!
 * @brief PD PTN5110 PHY driver instance count, meantime it indicates PTN5110 PHY driver enable or disable.
 *        - if 0, PTN5110 driver is disable.
 *        - if greater than 0, PTN5110 driver is enable.
 */
#define PD_CONFIG_PTN5110_PORT (1)

/*!
 * @brief Use one task for all PD instances.
 */
#define PD_CONFIG_COMMON_TASK (1)

/*!
 * @brief Enable compliance test function.
 */
#define PD_CONFIG_COMPLIANCE_TEST_ENABLE (1)

/*!
 * @brief Enable PD auto policy.
 */
#define PD_CONFIG_ENABLE_AUTO_POLICY (1)

/*!
 * @brief Board can detect the external power state.
 */
#define PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT (1)

/*!
 * @brief print the auto policy releated logs in the PD stack.
 */
#define PD_CONFIG_ENABLE_AUTO_POLICY_LOG (0)

/*!
 * @brief PD stack instance max count.
 */
#define PD_CONFIG_MAX_PORT (PD_CONFIG_PTN5110_PORT)

/*! @brief PD revision that this stack support */
#define PD_CONFIG_REVISION (PD_SPEC_REVISION_30)

/*! @brief PD structured VDM version that this stack support */
#define PD_CONFIG_STRUCTURED_VDM_VERSION (PD_SPEC_STRUCTURED_VDM_VERSION_20)

/*!
 * @brief Enable PD stack source role function.
 */
#define PD_CONFIG_SOURCE_ROLE_ENABLE (1)

/*!
 * @brief Enable PD stack sink role function.
 */
#define PD_CONFIG_SINK_ROLE_ENABLE (1)

/*!
 * @brief Enable PD stack dual power role function.
 */
#define PD_CONFIG_DUAL_POWER_ROLE_ENABLE (1)

/*!
 * @brief Enable PD stack dual data role function.
 */
#define PD_CONFIG_DUAL_DATA_ROLE_ENABLE (1)

/*!
 * @brief Enable Vconn support (vconn_swap, vconn supply, vconn discharge).
 */
#define PD_CONFIG_VCONN_SUPPORT (1)

/*!
 * @brief Enable Vconn discharge function (The macro is deprecated. Combine the macro and PD_CONFIG_VCONN_SUPPORT
 * only to keep PD_CONFIG_VCONN_SUPPORT. Vconn discharge by default is enabled if PD_CONFIG_VCONN_SUPPORT is set to 1).
 */
#define USBPD_ENABLE_VCONN_DISCHARGE (1)

/*!
 * @brief Enable vendor defined message function.
 */
#define PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE (1)

/*!
 * @brief Enable alternate mode function.
 */
#define PD_CONFIG_ALT_MODE_SUPPORT (0)

/*!
 * @brief Enable alternate mode DisplayPort function.
 */
#define PD_CONFIG_ALT_MODE_DP_SUPPORT          (0)
#define PD_CONFIG_ALT_MODE_DP_AUTO_SELECT_MODE (0)

/*!
 * @brief Enable alternate mode DisplayPort slave function.
 */
#define PD_CONFIG_ALT_MODE_SLAVE_SUPPORT (0)

/*!
 * @brief Enable alternate mode DisplayPort host function.
 */
#define PD_CONFIG_ALT_MODE_HOST_SUPPORT (0)

/*!
 * @brief Enable auto discovery cable plug function.
 */
#define PD_CONFIG_SRC_AUTO_DISCOVER_CABLE_PLUG (1)

/*!
 * @brief Enable cable communication function.
 */
#define PD_CONFIG_CABLE_COMMUNICATION_ENABLE (1)

/*!
 * @brief Config the detach detect way
 *  - PD_SINK_DETACH_ON_VBUS_ABSENT: detach is detected when vubs absent.
 *  - PD_SINK_DETACH_ON_CC_OPEN: detach is detected when CC is open.
 */
#define PD_CONFIG_SINK_DETACH_DETECT_WAY (PD_SINK_DETACH_ON_VBUS_ABSENT | PD_SINK_DETACH_ON_CC_OPEN)

#if (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30)
/*!
 * @brief Enable PD3.0 extended message function.
 */
#define PD_CONFIG_EXTENDED_MSG_SUPPORT (1)

/*!
 * @brief Enable fast role swap function.
 */
#define PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE (1)

/*!
 * @brief Enable PD3.0 AMS collision avoid function.
 */
#define PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE (1)

/*!
 * @brief Enable PD3.0 PPS function.
 */
#define PD_CONFIG_PD3_PPS_ENABLE (1)

#else

/*!
 * @brief Enable PD3.0 extended message function.
 */
#define PD_CONFIG_EXTENDED_MSG_SUPPORT           (0)

/*!
 * @brief Enable fast role swap function.
 */
#define PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE      (0)

/*!
 * @brief Enable PD3.0 AMS collision avoid function.
 */
#define PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE (0)

/*!
 * @brief Enable PD3.0 PPS function.
 */
#define PD_CONFIG_PD3_PPS_ENABLE                 (0)

#endif

#if ((PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_COMPLIANCE_TEST_ENABLE))
#define PD_CONFIG_EXTENDED_CHUNK_LAYER_SUPPORT (1)
#else
#define PD_CONFIG_EXTENDED_CHUNK_LAYER_SUPPORT (0)
#endif

/*!
 * @brief Set the discharge time.
 * 1 - use the 14ms.
 * 0 - use the 650ms.
 */
#define PD_CONFIG_MIN_DISCHARGE_TIME_ENABLE (1)

/*!
 * @brief Enable Try.SNK function.
 */
#define PD_CONFIG_TRY_SNK_SUPPORT (1)

/*!
 * @brief Enable Try.SRC function.
 */
#define PD_CONFIG_TRY_SRC_SUPPORT (1)

/*!
 * @brief Enable sink accessory function.
 */
#define PD_CONFIG_SINK_ACCESSORY_SUPPORT (0)

/*!
 * @brief Enable audio accessory function.
 */
#define PD_CONFIG_AUDIO_ACCESSORY_SUPPORT (0)

/*!
 * @brief Enable debug accessory function.
 */
#define PD_CONFIG_DEBUG_ACCESSORY_SUPPORT (0)

/*!
 * @brief Enable low power function.
 * 0 - Diable low power function. All PD functions are enabled.
 * 1 - Low power level 1. When detecting the detached state, PD PHY will enter low power state.
 * 2 - Low power level 2. Include level 1 setting and PD PHY will enter low power state when Type-C
 *     is in the attached state and PD stack is idle. In this low power level, PD communicaton can
 *     be carried out and partner's message can wake up PD PHY but VBUS functions will be disabled.
 */
#define PD_CONFIG_PHY_LOW_POWER_LEVEL (0)

#endif
