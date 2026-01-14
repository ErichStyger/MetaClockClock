/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DPM_EXTERNAL_H__
#define __DPM_EXTERNAL_H__

#include "fsl_adapter_gpio.h"
#include "pd_power_interface.h"
#include "pd_board_config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define VDM_DP_SID           0xFF01
#define VDM_DP_MODE          0x00000001u
#define PD_CONFIG_PID        (0x0001u)
#define PD_VENDOR_VID        (0x1FC9u)
#define PD_CONFIG_XID        (0x0001u)
#define PD_CONFIG_FW_VER     (0x01U)
#define PD_CONFIG_HW_VER     (0x01U)
#define PD_CONFIG_BCD_DEVICE (0x0001)

typedef struct _pd_source_cap_ext_data_block
{
    uint16_t vid;
    uint16_t pid;
    uint32_t xid;
    uint8_t fwVersion;
    uint8_t hwVersion;
    uint8_t voltageRegulation;
    uint8_t holdupTime;
    uint8_t compliance;
    uint8_t touchCurrent;
    uint16_t peakCurrent1;
    uint16_t peakCurrent2;
    uint16_t peakCurrent3;
    uint8_t touchTemp;
    uint8_t sourceInputs;
    uint8_t batteries;
} pd_source_cap_ext_data_block_t;

typedef struct _pd_status_data_block
{
    uint8_t internalTemp;
    uint8_t presentInput;
    uint8_t presentBatteryInput;
} pd_status_data_block_t;

typedef struct _pd_alert_data_object
{
    union
    {
        uint32_t alertValue;
        struct
        {
            uint32_t reserved : 16;
            uint32_t hostSwappableBatteries : 4;
            uint32_t fixedBatteries : 4;
            uint32_t typeOfAlert : 8;
        } bitFields;
    };
} pd_alert_data_object_t;

typedef struct _pd_battery_cap_data_block
{
    uint16_t vid;
    uint16_t pid;
    uint16_t batteryDesignCap;
    uint16_t batteryLastFullChargeCap;
    uint8_t batteryType;
} pd_battery_cap_data_block_t;

typedef struct _pd_battery_status_data_object
{
    uint8_t reserved;
    uint8_t batterInfo;
    uint16_t batteryPC;
} pd_battery_status_data_object_t;

typedef struct _pd_manufac_info_data_block
{
    uint16_t vid;
    uint16_t pid;
    uint8_t manufacturerString[22];
} pd_manufac_info_data_block_t;

typedef struct _pd_vdm_identity_data
{
    pd_id_header_vdo_t idHeaderVDO;
    uint32_t certStatVDO;
    uint16_t bcdDevice;
    uint16_t pid;
} pd_vdm_identity_data_t;

typedef enum _pd_demo_sw_state
{
    kDEMO_SWIdle = 0,
    kDEMO_SWIsrTrigger,
    kDEMO_SWPending,
    kDEMO_SWShortPress,
    kDEMO_SWLongPress,
    kDEMO_SWProcessed,
} pd_demo_sw_state_t;

typedef struct _pd_app
{
    pd_handle pdHandle;
    pd_instance_config_t *pdConfigParam;
    uint32_t sourceVbusVoltage;
    pd_sink_pdo_t partnerSinkCaps[3];
    pd_rdo_t partnerRequestRDO;
    GPIO_HANDLE_DEFINE(gpioExtraSrcHandle);
    uint8_t partnerSinkCapNumber;
    uint8_t msgSop;
    uint8_t contractValid;
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
