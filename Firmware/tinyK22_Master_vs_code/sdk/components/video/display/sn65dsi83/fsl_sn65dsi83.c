/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_sn65dsi83.h"
#include "fsl_display.h"
#include "fsl_video_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SN65DSI83_I2C_WriteReg(handle, addr, reg, value)                              \
    VIDEO_I2C_WriteReg(addr, kVIDEO_RegAddr8Bit, (reg), kVIDEO_RegWidth8Bit, (value), \
                       ((const sn65dsi83_resource_t *)((handle)->resource))->i2cSendFunc)

#define SN65DSI83_I2C_ReadReg(handle, addr, reg, value)                              \
    VIDEO_I2C_ReadReg(addr, kVIDEO_RegAddr8Bit, (reg), kVIDEO_RegWidth8Bit, (value), \
                      ((const sn65dsi83_resource_t *)((handle)->resource))->i2cReceiveFunc)

#define SN65DSI83_I2C_ModifyReg(handle, addr, reg, mask, value)                               \
    VIDEO_I2C_ModifyReg(addr, kVIDEO_RegAddr8Bit, (reg), kVIDEO_RegWidth8Bit, mask, (value),  \
                        ((const sn65dsi83_resource_t *)((handle)->resource))->i2cReceiveFunc, \
                        ((const sn65dsi83_resource_t *)((handle)->resource))->i2cSendFunc)

#define SN65DSI83_CHECK_RET(x)         \
    do                                 \
    {                                  \
        status = (x);                  \
        if (kStatus_Success != status) \
        {                              \
            return status;             \
        }                              \
    } while (false)

/*******************************************************************************
 * Variables
 ******************************************************************************/

const display_operations_t sn65dsi83_ops = {
    .init   = SN65DSI83_Init,
    .deinit = SN65DSI83_Deinit,
    .start  = SN65DSI83_Start,
    .stop   = SN65DSI83_Stop,
};

/*******************************************************************************
 * Code
 ******************************************************************************/

status_t SN65DSI83_Init(display_handle_t *handle, const display_config_t *config)
{
    status_t status;
    uint8_t mainI2cAddr   = (((const sn65dsi83_resource_t *)(handle->resource))->i2cAddr);
    uint32_t chipRevision = 0;
    uint8_t lanes         = config->dsiLanes;

    if ((lanes <= 1U) || (lanes > 4U))
    {
        return kStatus_InvalidArgument;
    }
    /* Clear software reset and PLL */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_SOFT_RESET, 0x00U));
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_PLL_EN, 0x00U));
    VIDEO_DelayMs(3);
    /* Set LVDS clock range */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_LVDS_CLK, 0x05U));
    /* Set DSI clock divider */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_DSI_CLK_DIV, 0x10U));
    /* Set DSI line number */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_DSI_LINES, 0x20U));
    /* Set DSI clock range */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_DSI_CLK, 0x2AU));
    /* Set LVDS format */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_LVDS_FMT, 0x78U));
    /* Set LVDS vod swing */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_LVDS_VOD_SWING, 0x00U));
    /* Set LVDS cm adjust */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_LVDS_CM_ADJUST, 0x00U));
    /* Set CHA_ACTIVE_LINE_LENGTH_LOW */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_ACTIVE_LENGTH_LOW,
                                               (uint8_t)(FSL_VIDEO_EXTRACT_WIDTH(config->resolution))));
    /* Set CHA_ACTIVE_LINE_LENGTH_HIGH */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_ACTIVE_LENGTH_HIGH,
                                               (uint8_t)(FSL_VIDEO_EXTRACT_WIDTH(config->resolution) >> 8U)));
    /* Set CHA_ACTIVE_LINE_LENGTH_LOW */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_VERTICAL_SIZE_LOW,
                                               (uint8_t)(FSL_VIDEO_EXTRACT_HEIGHT(config->resolution))));
    /* Set CHA_ACTIVE_LINE_LENGTH_HIGH */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_VERTICAL_SIZE_HIGH,
                                               (uint8_t)(FSL_VIDEO_EXTRACT_HEIGHT(config->resolution) >> 8U)));
    /* Set CHA_SYNC_DELAY */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_SYNC_DELAY, 0x21U));
    /* Set CHA_HSYNC_PULSE_WIDTH_LOW */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_HSW, (uint8_t)(config->hsw)));
    /* Set CHA_VSYNC_PULSE_WIDTH_LOW */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_VSW, (uint8_t)(config->vsw)));
    /*Set CHA_HORIZONTAL_BACK_PORCH */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_HBP, (uint8_t)(config->hbp)));
    /* Set CHA_VERTICAL_BACK_PORCH */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_VBP, (uint8_t)(config->vbp)));
    /* set CHA_HORIZONTAL_FRONT_PORCH */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_HFP, (uint8_t)(config->hfp)));
    /* Set CHA_VERTICAL_FRONT_PORCH */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_VFP, (uint8_t)(config->vfp)));
    /* Set CHA_TEST_PATTERN */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_TEST_PATTERN, 0x00U));
    /* Enable PLL */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_PLL_EN, 0x01U));
    VIDEO_DelayMs(10);
    /* Enable Software reset */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_SOFT_RESET, 0x01U));
    VIDEO_DelayMs(10);

    /* Check LVDS_CLK reg, after enable pll and reset, the PLL_EN_STAT bit is auto
     * set to 1 */
    status = SN65DSI83_I2C_ReadReg(handle, mainI2cAddr, SN65DSI83_LVDS_CLK, &chipRevision);

    if (kStatus_Success != status)
    {
        return status;
    }

    status = SN65DSI83_I2C_ReadReg(handle, mainI2cAddr, SN65DSI83_ERROR_REG, &chipRevision);

    if (kStatus_Success != status)
    {
        return status;
    }
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_ERROR_REG, 0xFFU));

    VIDEO_DelayMs(1);

    status = SN65DSI83_I2C_ReadReg(handle, mainI2cAddr, SN65DSI83_ERROR_REG, &chipRevision);

    if (kStatus_Success != status)
    {
        return status;
    }

    VIDEO_DelayMs(10);

    return kStatus_Success;
}

status_t SN65DSI83_Deinit(display_handle_t *handle)
{
    status_t status;
    uint8_t mainI2cAddr = (((const sn65dsi83_resource_t *)(handle->resource))->i2cAddr);
    uint32_t chipRevision = 0;

    /* Disable PLL */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_PLL_EN, 0x00));
    /* Disable Software reset */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_SOFT_RESET, 0x00));

    status = SN65DSI83_I2C_ReadReg(handle, mainI2cAddr, SN65DSI83_LVDS_CLK, &chipRevision);

    if (kStatus_Success != status)
    {
        return status;
    }

    status = SN65DSI83_I2C_ReadReg(handle, mainI2cAddr, 0xE5, &chipRevision);

    if (kStatus_Success != status)
    {
        return status;
    }

    return kStatus_Success;
}

status_t SN65DSI83_Start(display_handle_t *handle)
{
    status_t status;
    uint8_t mainI2cAddr = (((const sn65dsi83_resource_t *)(handle->resource))->i2cAddr);
    /* Enable PLL */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_PLL_EN, 0x01));
    VIDEO_DelayMs(10);
    /* Enable Software reset */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_SOFT_RESET, 0x01));
    VIDEO_DelayMs(10);
    return kStatus_Success;
}

status_t SN65DSI83_Stop(display_handle_t *handle)
{
    status_t status;
    uint8_t mainI2cAddr = (((const sn65dsi83_resource_t *)(handle->resource))->i2cAddr);
    /* Disable PLL */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_PLL_EN, 0x00));
    /* Disable Software reset */
    SN65DSI83_CHECK_RET(SN65DSI83_I2C_WriteReg(handle, mainI2cAddr, SN65DSI83_SOFT_RESET, 0x00));
    return kStatus_Success;
}
