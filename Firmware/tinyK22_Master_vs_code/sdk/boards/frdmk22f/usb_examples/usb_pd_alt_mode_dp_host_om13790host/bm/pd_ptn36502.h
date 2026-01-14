/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PD_PTN36502_H__
#define __PD_PTN36502_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Register indexes */
#define REG_CHIP_ID               0x00U
#define REG_CHIP_REVISION         0x01U
#define REG_USB_US_TXRX_CONTROL   0x04U
#define REG_USB_DS_TXRX_CONTROL   0x05U
#define REG_DP_LINK_CONTROL       0x06U
#define REG_DP_LANE0_TXRX_CONTROL 0x07U
#define REG_DP_LANE1_TXRX_CONTROL 0x08U
#define REG_DP_LANE2_TXRX_CONTROL 0x09U
#define REG_DP_LANE3_TXRX_CONTROL 0x0AU
#define REG_MODE_CONTROL1         0x0BU
#define REG_SQUELCH_THRESHOLD     0x0CU
#define REG_DEVICE_CONTROL        0x0DU

/* chip id */
#define REG_CHIP_ID_VAL (0x02u)

/* reg mode control 1 */
#define REG_MODE_CONTROL1_OPERATIONAL_MODE_SHIFT     (0x00u)
#define REG_MODE_CONTROL1_OPERATIONAL_MODE_MASK      (0x07u)
#define REG_MODE_CONTROL1_DATA_ROLE_SHIFT            (0x06u)
#define REG_MODE_CONTROL1_DATA_ROLE_MASK             (0xC0u)
#define REG_MODE_CONTROL1_ORIENT_SHIFT               (0x05u)
#define REG_MODE_CONTROL1_ORIENT_MASK                (0x20u)
#define REG_MODE_CONTROL1_AUX_CROSSBAR_SWITCH_MASK   (0x08u)
#define REG_MODE_CONTROL1_AUX_CROSSBAR_SWITCH_SHIFT  (3U)
#define REG_MODE_CONTROL1_AUX_MONITOR_POLARITY_MASK  (0x10u)
#define REG_MODE_CONTROL1_AUX_MONITOR_POLARITY_SHIFT (4U)

typedef enum _pd_ptn36502_mode
{
    kMode_DeepPowerSavingState = 0,
    kMode_USB31Gen1Only,
    kMode_USB31Gen1AndDP2Lane,
    kMode_DP4Lane,
} pd_ptn36502_mode_t;

typedef struct _pd_dp_redriver_config
{
    uint16_t ptn36502I2CMaster;
    uint16_t ptn36502SlaveAddress;
    uint32_t ptn36502I2cSrcClock;
    void *ptn36502I2cReleaseBus;
} pd_dp_redriver_config_t;

typedef struct _pd_ptn36502_instance
{
    pd_handle pdHandle;
    usb_pd_i2c_handle i2cHandle;
    uint8_t prevMode;
    uint8_t ptn36502SlaveAddress;
} pd_ptn36502_instance_t;

/*******************************************************************************
 * API
 ******************************************************************************/

/* 0 - intialize success; other values - fail */
uint8_t PD_PTN36502Init(pd_ptn36502_instance_t *ptn36502Instance, pd_handle pdHandle, void *config);
/* 0 - de-intialize success; other values - fail */
uint8_t PD_PTN36502Deinit(pd_ptn36502_instance_t *ptn36502Instance);

void PD_PTN36502SetMode(pd_ptn36502_instance_t *ptn36502Instance, pd_ptn36502_mode_t mode);
#endif
