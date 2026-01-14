/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_nmh1000.h"


/*******************************************************************************
 * Code
 ******************************************************************************/
status_t NMH_ReadReg(nmh_handle_t *handle, uint8_t reg, uint8_t *val, uint8_t bytesNumber)
{
    assert(handle != NULL);
    assert(val != NULL);

    if (handle->I2C_ReceiveFunc == NULL)
    {
        return kStatus_Fail;
    }
    else
    {
        return handle->I2C_ReceiveFunc(NMH1000_I2C_ADDR, reg, 1, val, bytesNumber);
    }
}

status_t NMH_WriteReg(nmh_handle_t *handle, uint8_t reg, uint8_t val)
{
    assert(handle != NULL);

    if (handle->I2C_SendFunc == NULL)
    {
        return kStatus_Fail;
    }
    else
    {
        return handle->I2C_SendFunc(NMH1000_I2C_ADDR, reg, 1, val);
    }
}

status_t NMH_Init(nmh_handle_t *handle, nmh_config_t *config)
{
    assert(handle != NULL);
    assert(config != NULL);
    assert(config->I2C_SendFunc != NULL);
    assert(config->I2C_ReceiveFunc != NULL);

    /* Initialize the I2C access functions. */
    handle->I2C_SendFunc    = config->I2C_SendFunc;
    handle->I2C_ReceiveFunc = config->I2C_ReceiveFunc;

    /* Set to USER_ODR to medium sample rate (1 Hz) */
    if (NMH_WriteReg(handle, NMH1000_USER_ODR, NMH1000_MEDIUM_SAMPLE_RATE) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    /* Set to autonomous mode */
    if (NMH_WriteReg(handle, NMH1000_CONTROL_REG1, NMH1000_AUTO_MASK) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    return kStatus_Success;
}

status_t NMH_ReadMagData(nmh_handle_t *handle, uint8_t *magData)
{
    assert(handle != NULL);
    assert(magData != NULL);

    uint8_t tmp;

    /* Read value of relative magnetic field strength from OUT_M_REG (MAG_DATA) */
    if (NMH_ReadReg(handle, NMH1000_OUT_M_REG, &tmp, 1u) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    *magData = (tmp >> NMH1000_MAG_DATA_SHIFT);

    return kStatus_Success;
}
