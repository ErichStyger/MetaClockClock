/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_fxas.h"
/*******************************************************************************
 * Variables
 ******************************************************************************/
/* sensitivity coefficients */
static float gyro_sensCoefs[] = {0.0625, 0.03125, 0.015625, 0.0078125};

/******************************************************************************
 * Code
 ******************************************************************************/

status_t FXAS_Init(fxas_handle_t *fxas_handle, fxas_config_t *configure)
{
    uint8_t tmp[1] = {0};

    /* Initialize the I2C access function. */
    fxas_handle->I2C_SendFunc    = configure->I2C_SendFunc;
    fxas_handle->I2C_ReceiveFunc = configure->I2C_ReceiveFunc;
    /* Check device ID. */
    if (FXAS_ReadReg(fxas_handle, FXAS_WHO_AM_I_REG, tmp, 1) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    if (tmp[0] != kFXAS_WHO_AM_I_Device_ID)
    {
        return kStatus_Fail;
    }

    /* go to standby */
    if (FXAS_ReadReg(fxas_handle, FXAS_CTRL_REG1, tmp, 1) != kStatus_Success)
    {
        return kStatus_Fail;
    }
    if (FXAS_WriteReg(fxas_handle, FXAS_CTRL_REG1, tmp[0] & (uint8_t)~FXAS_OP_MASK) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    /* Read again to make sure we are in standby mode. */
    if (FXAS_ReadReg(fxas_handle, FXAS_CTRL_REG1, tmp, 1) != kStatus_Success)
    {
        return kStatus_Fail;
    }
    if ((tmp[0] & FXAS_OP_MASK) != 0)
    {
        return kStatus_Fail;
    }

    /* Set FIFO */
    if (FXAS_WriteReg(fxas_handle, FXAS_F_SETUP_REG, configure->fifo << FXAS_F_MODE_SHIFT) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    /* Setup the scale range. */
    if (FXAS_WriteReg(fxas_handle, FXAS_CTRL_REG0, configure->fsrdps) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    /* Setup data rate */
    if (FXAS_WriteReg(fxas_handle, FXAS_CTRL_REG1, (configure->odr << DATA_RATE_OFFSET)) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    /* Setup interrupt configure */
    if (FXAS_WriteReg(fxas_handle, FXAS_CTRL_REG2, (FXAS_INT_CFG_DRDY_MASK | FXAS_INT_EN_DRDY_MASK | FXAS_IPOL_MASK)) !=
        kStatus_Success)
    {
        return kStatus_Fail;
    }

    /* Setup rate threshold
     at max rate threshold = FSR;  rate threshold = THS*FSR/128.
     Enable rate threshold detection on all axes.
    */
    if (FXAS_WriteReg(fxas_handle, FXAS_RT_CFG_REG, RT_ALLAXES_MASK) != kStatus_Success)
    {
        return kStatus_Fail;
    }
    /* Unsigned 7-bit THS, set to one-tenth FSR; set clearing debounce counter. */
    if (FXAS_WriteReg(fxas_handle, FXAS_RT_THS_REG, 0x0D) != kStatus_Success)
    {
        return kStatus_Fail;
    }
    if (FXAS_WriteReg(fxas_handle, FXAS_RT_COUNT_REG, 0x4) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    /* Active mode set */
    if (FXAS_ReadReg(fxas_handle, FXAS_CTRL_REG1, tmp, 1) != kStatus_Success)
    {
        return kStatus_Fail;
    }
    if (FXAS_WriteReg(fxas_handle, FXAS_CTRL_REG1, tmp[0] | FXAS_ACTIVE_MASK) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    /* Read Control register again to ensure we are in active mode */
    if (FXAS_ReadReg(fxas_handle, FXAS_CTRL_REG1, tmp, 1) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    if ((tmp[0] & FXAS_ACTIVE_MASK) != FXAS_ACTIVE_MASK)
    {
        return kStatus_Fail;
    }

    return kStatus_Success;
}

status_t FXAS_ReadSensorData(fxas_handle_t *fxas_handle, fxas_data_t *sensorData)
{
    status_t status     = kStatus_Success;
    uint8_t tmp_buff[6] = {0};
    uint8_t i           = 0;

    if (!FXAS_ReadReg(fxas_handle, FXAS_OUT_X_MSB_REG, tmp_buff, 6) == kStatus_Success)
    {
        status = kStatus_Fail;
    }

    for (i = 0; i < 6; i++)
    {
        ((int8_t *)sensorData)[i] = tmp_buff[i];
    }

    return status;
}

status_t FXAS_ReadReg(fxas_handle_t *handle, uint8_t reg, uint8_t *val, uint8_t bytesNumber)
{
    assert(handle);
    assert(val);

    if (!handle->I2C_ReceiveFunc)
    {
        return kStatus_Fail;
    }

    return handle->I2C_ReceiveFunc(FXAS_GYRO_I2C_ADDRESS, reg, 1, val, bytesNumber);
}

status_t FXAS_WriteReg(fxas_handle_t *handle, uint8_t reg, uint8_t val)
{
    assert(handle);

    if (!handle->I2C_SendFunc)
    {
        return kStatus_Fail;
    }

    return handle->I2C_SendFunc(FXAS_GYRO_I2C_ADDRESS, reg, 1, val);
}

float FXAS_FormatFloat(int16_t input, fxas_gfsr_t fsrdps)
{
    /* Convert data. */
    return (float)input * gyro_sensCoefs[(uint8_t)fsrdps];
}
