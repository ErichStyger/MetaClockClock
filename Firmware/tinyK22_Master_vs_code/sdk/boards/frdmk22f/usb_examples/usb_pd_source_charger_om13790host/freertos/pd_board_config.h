/*
 * Copyright 2017-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PD_BOARD_CONFIG_H__
#define __PD_BOARD_CONFIG_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

#define BOARD_PD_I2C_INDEX (0U)

#define BOARD_PD_SW_INPUT_SUPPORT (1U)

#define PD_DEMO_EXPECTED_VOLTAGE (5 * 1000)    /* 5V */
#define PD_DEMO_EXPECTED_CURRENT (3000)        /* 3A */
#define PD_DEMO_TYPEC_CURRENT    (kCurrent_3A) /* 3A */

#define BOARD_PD_EXTERNAL_POWER_SUPPORT (1U)

#define BOARD_PD_USB3_CROSS_SUPPORT (1U)

#define PD_COMPLIANCE_TEST_DRP (1U)

#define PD_COMPLIANCE_TEST_DRP_TRY_SNK (0U)

#define PD_COMPLIANCE_TEST_DRP_TRY_SRC (0U)

#define PD_COMPLIANCE_TEST_CONSUMER_PROVIDER (0U)

#define PD_COMPLIANCE_TEST_PROVIDER_CONSUMER (0U)

#if (defined PD_CONFIG_COMPLIANCE_TEST_ENABLE) && (PD_CONFIG_COMPLIANCE_TEST_ENABLE)
#ifndef PD_TEST_ENABLE_AUTO_POLICY
#define PD_TEST_ENABLE_AUTO_POLICY (1U)
#endif

#else
#if (PD_COMPLIANCE_TEST_DRP)
/* 0 - disable auto policy;
 * 1 - enable auto policy 1:
 * don't auto request, auto accept all request for pr swap, data swap and vconn swap;
 * 2 - enable auto policy 2:
 * auto request pr swap when works as sink, auto accept for swap to source/DFP/vconn source, auto reject for swap to
 * sink/UFP/not vconn source, auto request power when working as sink;
 * 3 - enable auto policy 3:
 * auto request data swap to DFP, auto accept for swap to sink/UFP/not vconn source, auto reject for swap to
 * source/DFP/vconn source;
 * 4 - enable auto policy 4:
 * auto request vconn swap to vconn source, auto reject all request for pr swap, data swap and vconn swap.
 */
#ifndef PD_TEST_ENABLE_AUTO_POLICY
#define PD_TEST_ENABLE_AUTO_POLICY (1U)
#endif

#endif
#endif

#define PD_DEMO_PORT1_ENABLE (1U)

#define PD_DEMO_PORT2_ENABLE (0U)

#define PD_DEMO_PORT3_ENABLE (0U)

#define PD_DEMO_PORT4_ENABLE (0U)

#define PD_DEMO_PORTS_COUNT (PD_DEMO_PORT1_ENABLE + PD_DEMO_PORT2_ENABLE + PD_DEMO_PORT3_ENABLE + PD_DEMO_PORT4_ENABLE)

#define PD_PORT1_PHY_INTERRUPT_PIN (19U)

#define PD_PORT1_PHY_INTERRUPT_PORT (1U)

#define PD_PORT1_SHIELD2_EXTERNAL_POWER_DETECT_PIN (3U)

#define PD_PORT1_SHIELD2_EXTERNAL_POWER_DETECT_PORT (2U)

#define PD_PORT1_EXTRA_SRC_PIN (4U)

#define PD_PORT1_EXTRA_SRC_PORT (0U)

#define PD_PORT1_SHIELD2_DP4LANE_PIN (16U)

#define PD_PORT1_SHIELD2_DP4LANE_PORT (1U)

#define PD_PORT1_SHIELD2_XSD_PIN (18U)

#define PD_PORT1_SHIELD2_XSD_PORT (1U)

#define PD_PORT1_SHIELD2_ORIENT_PIN (1U)

#define PD_PORT1_SHIELD2_ORIENT_PORT (0U)

#define PD_PORT1_SHIELD2_HPD_PIN (2U)

#define PD_PORT1_SHIELD2_HPD_PORT (0U)

#define PD_PORT1_POWER_REQUEST_SW_PIN (1U)

#define PD_PORT1_POWER_REQUEST_SW_PORT (2U)

#define PD_PORT1_POWER_CHANGE_SW_PIN (17U)

#define PD_PORT1_POWER_CHANGE_SW_PORT (1U)

#define PD_PORT1_EXTERNAL_POWER_DETECT_INTERRUPT_PRIORITY (4U)

#define PD_PORT1_PHY_INTERRUPT_PRIORITY (4U)

#define PD_PORT1_I2C_INTERRUPT_PRIORITY (3U)

#define PD_TIMER_INTERRUPT_PRIORITY (2U)

#define PD_PTN36502_SLAVE_ADDRESS  (0x12)
#define PD_NX20P3483_SLAVE_ADDRESS (0x72)

#define PD_TIMER_INSTANCE (0U)

/*******************************************************************************
 * API
 ******************************************************************************/

#endif
