/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DPM_EXTERNAL_H__
#define __DPM_EXTERNAL_H__

#include "pd_board_config.h"
#include "fsl_adapter_gpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define PD_CONFIG_APP_MODE (0x00000001u)
#define PD_CONFIG_PID (0x0100)
#define PD_VENDOR_VID (0x1FC9u)
#define PD_CONFIG_XID (0x0000u)
#define PD_CONFIG_FW_VER (0x01)
#define PD_CONFIG_HW_VER (0x01)
#define PD_CONFIG_BCD_DEVICE (0x0100)

#if defined(__CC_ARM)
#pragma anon_unions
#endif

typedef struct _pd_vdm_identity_data
{
    pd_id_header_vdo_t idHeaderVDO;
    uint32_t certStatVDO;
    uint16_t bcdDevice;
    uint16_t pid;
} pd_vdm_identity_data_t;

typedef struct _pd_app
{
    pd_handle pdHandle;
    GPIO_HANDLE_DEFINE(gpioExtraSrcHandle);
    pd_instance_config_t *pdConfigParam;
    pd_vdm_identity_data_t selfVdmIdentity;
    uint32_t selfVdmSVIDs;
    pd_rdo_t sinkRequestRDO; /* sink - the self requested RDO; source - the partner sink requested RDO */
    uint32_t sinkRequestVoltage;
    pd_source_pdo_t partnerSourceCaps[7];
    uint8_t partnerSourceCapNumber; /* partner */
    uint8_t msgSop;
    volatile uint8_t contractValid;
    volatile uint8_t batteryQuantity; /* 0 - 100 */
    volatile uint8_t batteryChange;
    uint8_t portNumber;
    uint8_t portShieldVersion;
} pd_app_t;

typedef struct _pd_demo_io_init
{
    uint8_t extraEnSrcPort;
    uint8_t extraEnSrcPin;
} pd_demo_io_init_t;

#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE > 0)
extern pd_app_t g_PDAppInstancePort1;
#endif
#if (defined PD_DEMO_PORT2_ENABLE) && (PD_DEMO_PORT2_ENABLE > 0)
extern pd_app_t g_PDAppInstancePort2;
#endif
#if (defined PD_DEMO_PORT3_ENABLE) && (PD_DEMO_PORT3_ENABLE > 0)
extern pd_app_t g_PDAppInstancePort3;
#endif
#if (defined PD_DEMO_PORT4_ENABLE) && (PD_DEMO_PORT4_ENABLE > 0)
extern pd_app_t g_PDAppInstancePort4;
#endif
extern pd_app_t *g_PDAppInstanceArray[];

/*******************************************************************************
 * API
 ******************************************************************************/

#endif
