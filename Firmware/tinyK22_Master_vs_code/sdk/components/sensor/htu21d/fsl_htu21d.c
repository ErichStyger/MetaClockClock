/*
 * Copyright  2017 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_htu21d.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Initialize HTU21D sensor and reset the module
 */
htu_status_t HTU_Init(htu_handle_t *handle, htu_config_t *config)
{
    assert(handle);
    assert(config);

    /* Reset device. */
    htu_status_t status = kStatus_HTU_Success;

    /* Initialize the I2C access function */
    handle->I2C_SendFunc    = config->I2C_SendFunc;
    handle->I2C_ReceiveFunc = config->I2C_ReceiveFunc;

    status |= HTU_SoftReset(handle);
    status |= HTU_SetUserRegister(handle, config->resolution);

    if (kStatus_HTU_Success != status)
    {
        return kStatus_HTU_Error;
    }

    else
    {
        return kStatus_HTU_Success;
    }
}

status_t HTU_ReadFromDevice(htu_handle_t *handle, uint8_t reg, uint8_t regSize, uint8_t *val, uint8_t bytesNumber)
{
    if (!handle->I2C_ReceiveFunc)
    {
        return kStatus_Fail;
    }

    return handle->I2C_ReceiveFunc(HTU_I2C_ADDRESS, reg, regSize, val, bytesNumber);
}

status_t HTU_WriteToDevice(htu_handle_t *handle, uint8_t reg, uint8_t regSize, uint8_t val)
{
    assert(handle);

    if (!handle->I2C_SendFunc)
    {
        return kStatus_Fail;
    }

    return handle->I2C_SendFunc(HTU_I2C_ADDRESS, reg, regSize, val);
}

/*
 * reset the sensor
 * @return status flag
 */
htu_status_t HTU_SoftReset(htu_handle_t *handle)
{
    /* Send command only, set with regSize = 0. */
    status_t status = HTU_WriteToDevice(handle, 0, 0, HTU21D_SOFT_RESET);
    if (kStatus_Success != status)
    {
        return kStatus_HTU_ProtocolError;
    }
    else
    {
        return kStatus_HTU_Success;
    }
}

/*
 * configure the measurement resolution
 * @return status flag
 */
htu_status_t HTU_SetUserRegister(htu_handle_t *handle, htu_resoultion_t resolution)
{
    uint8_t txBuff = 0U;

    /*
     * Set a byte to be written to user register
     * and also set the corresponding measurement time
     */
    switch (resolution)
    {
        /* RH: 12bit, TEMP: 14bit */
        case kHTU_BitTypeRH12Temp14:
        {
            txBuff = 0x2;
            break;
        }
        /* RH: 8bit, TEMP: 12bit */
        case kHTU_BitTypeRH8Temp12:
        {
            txBuff = 0x3;
            break;
        }
        /* RH: 10bit, TEMP: 13bit */
        case kHTU_BitTypeRH10Temp13:
        {
            txBuff = 0x82;
            break;
        }
        /* RH: 11bit, TEMP: 11bit */
        case kHTU_BitTypeRH11Temp11:
        {
            txBuff = 0x83;
            break;
        }
        default:
            break;
    }

    status_t status = HTU_WriteToDevice(handle, HTU21D_WRITE_USER_REG, 1, txBuff);
    if (kStatus_Success != status)
    {
        return kStatus_HTU_ProtocolError;
    }
    else
    {
        return kStatus_HTU_Success;
    }
}

/*
 * Get temperature data
 * @param  tempData  data to be filled
 * @return           status flag
 */
htu_status_t HTU_GetTempRawData(htu_handle_t *handle, int16_t *tempData)
{
    uint8_t rxBuff[3];
    *tempData = -1;

    /* Send command only, set with regSize = 0. */
    status_t status = HTU_WriteToDevice(handle, 0, 0, HTU21D_TRIGGER_TEMP_NOHOLD);
    if (kStatus_Success != status)
    {
        return kStatus_HTU_ProtocolError;
    }
    else
    {
        /* read temperature data. */
        while (1)
        {
            status_t status = HTU_ReadFromDevice(handle, 0, 0, rxBuff, 3);
            if (kStatus_Success == status)
            {
                break;
            }
        }

        /*
         * convert to final value
         * discard the third received byte (checksum)
         */
        *tempData = (rxBuff[0] << 8) | (rxBuff[1] & 0xFC);

        return kStatus_HTU_Success;
    }
}

/*
 * Get humidity data
 * @param  humData  data to be filled
 * @return          status flag
 */
htu_status_t HTU_GetHumRawData(htu_handle_t *handle, int16_t *humData)
{
    uint8_t rxBuff[3];

    status_t status = HTU_WriteToDevice(handle, 0, 0, HTU21D_TRIGGER_HUMD_NOHOLD);
    if (kStatus_Success != status)
    {
        return kStatus_HTU_ProtocolError;
    }
    else
    {
        /* read humidity data. */
        while (1)
        {
            status_t status = HTU_ReadFromDevice(handle, 0, 0, rxBuff, 3);
            if (kStatus_Success == status)
            {
                break;
            }
        }
        /*
         * convert to final value
         * discard the third received byte (checksum)
         */
        *humData = (rxBuff[0] << 8) | (rxBuff[1] & 0xFC);
        return kStatus_HTU_Success;
    }
}

/*
 * Get temperature and humidity data
 * @param  temperature address to place the temperature data to
 * @param  humidity    address to place the humidity data to
 * @return             status flag
 */
htu_status_t HTU_ReadRawData(htu_handle_t *handle, int16_t *temperature, int16_t *humidity)
{
    htu_status_t status = kStatus_HTU_Success;

    status |= HTU_GetTempRawData(handle, temperature);
    status |= HTU_GetHumRawData(handle, humidity);

    return status;
}

htu_status_t HTU_GetTempData(htu_handle_t *handle, void *tempData)
{
    float *floatTemp     = tempData;
    int16_t tempRawValue = 0U;

    if (kStatus_HTU_Success != HTU_GetTempRawData(handle, &tempRawValue))
    {
        return kStatus_HTU_Error;
    }
    else
    {
        *floatTemp = (float)(((175.72f * (float)tempRawValue) / 0x10000) - 46.85f);
        return kStatus_HTU_Success;
    }
}

htu_status_t HTU_GetHumData(htu_handle_t *handle, void *humData)
{
    float *humValue     = humData;
    int16_t humRawValue = 0U;

    if (kStatus_HTU_Success != HTU_GetHumRawData(handle, &humRawValue))
    {
        return kStatus_HTU_Error;
    }
    else
    {
        *humValue = 125 * ((float)humRawValue / 0x10000) - 6;
        return kStatus_HTU_Success;
    }
}
