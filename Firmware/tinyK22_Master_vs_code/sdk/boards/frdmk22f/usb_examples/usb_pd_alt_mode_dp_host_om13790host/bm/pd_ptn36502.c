/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "pd_board_config.h"
#include "usb_pd.h"
#include "usb_pd_i2c.h"
#include "pd_ptn36502.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define REG_SET_FIELD(regVal, mask, bitsVal) (regVal) = (((regVal) & (~(mask))) | (bitsVal))

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#define ptn36502_I2cWriteRegister(ptn36502Instance, registerAdd, data, dataLen)                                      \
    PD_I2cWriteBlocking(ptn36502Instance->i2cHandle, ptn36502Instance->ptn36502SlaveAddress, registerAdd, 1, (data), \
                        dataLen);

uint32_t HW_I2CGetFreq(uint8_t i2cInstance);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void PD_PTN36502SetMode(pd_ptn36502_instance_t *ptn36502Instance, pd_ptn36502_mode_t mode)
{
    uint8_t modeControl1 = 0x00u;

    if (mode <= kMode_DP4Lane)
    {
        uint8_t getInfo;
        PD_Control(ptn36502Instance->pdHandle, PD_CONTROL_GET_TYPEC_ORIENTATION, &getInfo);
        REG_SET_FIELD(modeControl1, (uint8_t)REG_MODE_CONTROL1_ORIENT_MASK,
                      (getInfo ? (1 << REG_MODE_CONTROL1_ORIENT_SHIFT) : (0)));
        PD_Control(ptn36502Instance->pdHandle, PD_CONTROL_GET_DATA_ROLE, &getInfo);
        REG_SET_FIELD(modeControl1, (uint8_t)REG_MODE_CONTROL1_DATA_ROLE_MASK,
                      ((getInfo == kPD_DataRoleDFP) ? (0 << REG_MODE_CONTROL1_DATA_ROLE_SHIFT) :
                                                      (2 << REG_MODE_CONTROL1_DATA_ROLE_SHIFT)));

        switch (mode)
        {
            case kMode_DeepPowerSavingState:
                REG_SET_FIELD(modeControl1, (uint8_t)REG_MODE_CONTROL1_OPERATIONAL_MODE_MASK, ((uint8_t)mode));
                ptn36502_I2cWriteRegister(ptn36502Instance, REG_MODE_CONTROL1, &modeControl1, 1);
                break;

            case kMode_USB31Gen1Only:
                if ((ptn36502Instance->prevMode != kMode_DeepPowerSavingState) && (ptn36502Instance->prevMode != mode))
                {
                    PD_PTN36502SetMode(ptn36502Instance, kMode_DeepPowerSavingState);
                }
                else
                {
                    REG_SET_FIELD(modeControl1, (uint8_t)REG_MODE_CONTROL1_OPERATIONAL_MODE_MASK, ((uint8_t)mode));
                    ptn36502_I2cWriteRegister(ptn36502Instance, REG_MODE_CONTROL1, &modeControl1, 1);
                }
                break;

            case kMode_USB31Gen1AndDP2Lane:
                if ((ptn36502Instance->prevMode != kMode_DeepPowerSavingState) &&
                    (ptn36502Instance->prevMode != kMode_USB31Gen1Only) && (ptn36502Instance->prevMode != mode))
                {
                    PD_PTN36502SetMode(ptn36502Instance, kMode_DeepPowerSavingState);
                }
                else
                {
                    REG_SET_FIELD(modeControl1, (uint8_t)REG_MODE_CONTROL1_AUX_CROSSBAR_SWITCH_MASK,
                                  (1 << REG_MODE_CONTROL1_AUX_CROSSBAR_SWITCH_SHIFT));
                    REG_SET_FIELD(modeControl1, (uint8_t)REG_MODE_CONTROL1_OPERATIONAL_MODE_MASK, ((uint8_t)mode));
                    ptn36502_I2cWriteRegister(ptn36502Instance, REG_MODE_CONTROL1, &modeControl1, 1);
                    ptn36502_I2cWriteRegister(ptn36502Instance, REG_DP_LINK_CONTROL, (uint8_t *)"\x06", 1);
                }
                break;

            case kMode_DP4Lane:
                if ((ptn36502Instance->prevMode != kMode_DeepPowerSavingState) && (ptn36502Instance->prevMode != mode))
                {
                    PD_PTN36502SetMode(ptn36502Instance, kMode_DeepPowerSavingState);
                }
                else
                {
                    REG_SET_FIELD(modeControl1, (uint8_t)REG_MODE_CONTROL1_AUX_CROSSBAR_SWITCH_MASK,
                                  (1 << REG_MODE_CONTROL1_AUX_CROSSBAR_SWITCH_SHIFT));
                    ptn36502_I2cWriteRegister(ptn36502Instance, REG_MODE_CONTROL1, &modeControl1, 1);
                    REG_SET_FIELD(modeControl1, (uint8_t)REG_MODE_CONTROL1_OPERATIONAL_MODE_MASK, ((uint8_t)mode));
                    ptn36502_I2cWriteRegister(ptn36502Instance, REG_MODE_CONTROL1, &modeControl1, 1);
                    ptn36502_I2cWriteRegister(ptn36502Instance, REG_DP_LINK_CONTROL, (uint8_t *)"\x06", 1);
                }
                break;

            default:
                ptn36502_I2cWriteRegister(ptn36502Instance, REG_MODE_CONTROL1, &modeControl1, 1);
                break;
        }
        ptn36502Instance->prevMode = mode;
    }
}

static void PD_PTN36502Reset(pd_ptn36502_instance_t *ptn36502Instance)
{
    ptn36502Instance->prevMode = kMode_DeepPowerSavingState;

    ptn36502_I2cWriteRegister(ptn36502Instance, REG_USB_US_TXRX_CONTROL, (uint8_t *)"\x51", 1);
    ptn36502_I2cWriteRegister(ptn36502Instance, REG_USB_DS_TXRX_CONTROL, (uint8_t *)"\x51", 1);
    ptn36502_I2cWriteRegister(ptn36502Instance, REG_DP_LANE0_TXRX_CONTROL, (uint8_t *)"\x29", 1);
    ptn36502_I2cWriteRegister(ptn36502Instance, REG_DP_LANE1_TXRX_CONTROL, (uint8_t *)"\x29", 1);
    ptn36502_I2cWriteRegister(ptn36502Instance, REG_DP_LANE2_TXRX_CONTROL, (uint8_t *)"\x29", 1);
    ptn36502_I2cWriteRegister(ptn36502Instance, REG_DP_LANE3_TXRX_CONTROL, (uint8_t *)"\x29", 1);
    PD_PTN36502SetMode(ptn36502Instance, kMode_DeepPowerSavingState);
}

/* 0 - intialize success; other values - fail */
uint8_t PD_PTN36502Init(pd_ptn36502_instance_t *ptn36502Instance, pd_handle pdHandle, void *config)
{
    /* initialize crossbar enable pin */
    ptn36502Instance->pdHandle = pdHandle;
    ((pd_dp_redriver_config_t *)config)->ptn36502I2cSrcClock =
        HW_I2CGetFreq(((pd_dp_redriver_config_t *)config)->ptn36502I2CMaster);
    ptn36502Instance->ptn36502SlaveAddress = ((const pd_dp_redriver_config_t *)config)->ptn36502SlaveAddress;

    /* initialize I2C */
    PD_I2cInit(&ptn36502Instance->i2cHandle, ((const pd_dp_redriver_config_t *)config)->ptn36502I2CMaster,
               ((const pd_dp_redriver_config_t *)config)->ptn36502I2cSrcClock,
               (PD_I2cReleaseBus)((const pd_dp_redriver_config_t *)config)->ptn36502I2cReleaseBus);
    if (ptn36502Instance->i2cHandle == NULL)
    {
        return 1;
    }

    PD_PTN36502Reset(ptn36502Instance);
    return 0;
}

uint8_t PD_PTN36502Deinit(pd_ptn36502_instance_t *ptn36502Instance)
{
    PD_I2cDeinit(ptn36502Instance->i2cHandle);
    ptn36502Instance->i2cHandle = NULL;
    return 0;
}
