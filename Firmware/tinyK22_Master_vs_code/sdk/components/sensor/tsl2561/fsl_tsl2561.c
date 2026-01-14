/*
 * Copyright  2017 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_tsl2561.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static status_t TSL_WriteReg(tsl_handle_t *handle, uint8_t reg, uint8_t val);
static status_t TSL_ReadReg(tsl_handle_t *handle, uint8_t reg, uint8_t *val);
/*******************************************************************************
 * Variables
 ******************************************************************************/
static tsl_gain_config_t selectedGain         = kTSL_GainX16;
static tsl_intergration_time_t selectedTiming = kTSL_IntergrationTime13ms;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*
 * Enable the sensor
 */
tsl_status_t TSL_EnableSensor(tsl_handle_t *handle)
{
    if (kStatus_Success != TSL_WriteReg(handle, TSL_COMMAND_BIT | TSL_REG_CONTROL, TSL_CONTROL_POWERON))
    {
        return kStatus_TSL_Error;
    }
    else
    {
        return kStatus_TSL_Success;
    }
}

/*
 * Disable the sensor
 */
tsl_status_t TSL_DisableSensor(tsl_handle_t *handle)
{
    if (kStatus_Success != TSL_WriteReg(handle, TSL_COMMAND_BIT | TSL_REG_CONTROL, TSL_CONTROL_POWEROFF))
    {
        return kStatus_TSL_Error;
    }
    else
    {
        return kStatus_TSL_Success;
    }
}

/*
 * Initialize the sensor
 * @param  handle  handle to be populated with sensor info
 * @param  config  user settings
 * @return  status flag
 */
tsl_status_t TSL_Init(tsl_handle_t *handle, tsl_config_t *config)
{
    assert((NULL != handle) && (NULL != config));
    tsl_status_t status = kStatus_TSL_Success;

    /* Initialize the I2C access function. */
    handle->I2C_SendFunc    = config->I2C_SendFunc;
    handle->I2C_ReceiveFunc = config->I2C_ReceiveFunc;
    /* Set Slave Address. */
    handle->slaveAddress = config->slaveAddress;

    /*Reset the sensor. */
    TSL_DisableSensor(handle);
    TSL_EnableSensor(handle);

    /* Set default integration time and gain. */
    status = TSL_SetGainAndTiming(handle, config);

    return status;
}

static status_t TSL_ReadReg(tsl_handle_t *handle, uint8_t reg, uint8_t *val)
{
    assert(handle);
    assert(val);

    if (!handle->I2C_ReceiveFunc)
    {
        return kStatus_Fail;
    }

    return handle->I2C_ReceiveFunc(handle->slaveAddress, reg, 1, val, 1);
}

static status_t TSL_WriteReg(tsl_handle_t *handle, uint8_t reg, uint8_t val)
{
    assert(handle);

    if (!handle->I2C_SendFunc)
    {
        return kStatus_Fail;
    }

    return handle->I2C_SendFunc(handle->slaveAddress, reg, 1, val);
}

/*!
 * Set sensor's gain and timing
 * @param  gain desired gain
 * @return      status flag
 */
tsl_status_t TSL_SetGainAndTiming(tsl_handle_t *handle, tsl_config_t *config)
{
    if (kStatus_Success ==
        TSL_WriteReg(handle, TSL_COMMAND_BIT | TSL_REG_TIMING, ((uint8_t)config->gain | (uint8_t)config->timing)))
    {
        return kStatus_TSL_Success;
    }
    else
    {
        return kStatus_TSL_Error;
    }
}

/*
 * read raw sensor data
 * @param  channel    TSL channel (red or infrared)
 * @param  sensorData sensor data to be filled
 * @return            status flag
 */
tsl_status_t TSL_ReadRawData(tsl_handle_t *handle, uint32_t channel, uint16_t *data)
{
    assert(NULL != data);

    uint8_t dataLow = 0, dataHigh = 0;

    /* Read ADC channel 0 (Visible and IR) */
    if (kTSL_Channel0 & channel)
    {
        if (kStatus_Success != TSL_ReadReg(handle, TSL_COMMAND_BIT | TSL_REG_CHAN0_LOW, &dataLow))
        {
            return kStatus_TSL_Error;
        }

        if (kStatus_Success != TSL_ReadReg(handle, TSL_COMMAND_BIT | TSL_REG_CHAN0_HIGH, &dataHigh))
        {
            return kStatus_TSL_Error;
        }

        *data++ = (dataLow | ((uint16_t)dataHigh << 8));
    }
    /* Read ADC channel 1 (IR only). */
    if (kTSL_Channel1 & channel)
    {
        if (kStatus_Success != TSL_ReadReg(handle, TSL_COMMAND_BIT | TSL_REG_CHAN1_LOW, &dataLow))
        {
            return kStatus_TSL_Error;
        }

        if (kStatus_Success != TSL_ReadReg(handle, TSL_COMMAND_BIT | TSL_REG_CHAN1_HIGH, &dataHigh))
        {
            return kStatus_TSL_Error;
        }

        *data = (dataLow | ((uint16_t)dataHigh << 8));
    }
    return kStatus_TSL_Success;
}

/*
 * simplified lux calculation
 * @param  channelValues ADC values from both channels
 * @param  luxVal        value in [lx]
 * @return               status flag
 */
tsl_status_t TSL_CalculateLux(uint16_t *channelValues, uint32_t *luxValue)
{
    assert((NULL != channelValues) && (NULL != luxValue));

    uint32_t chScale  = 0U;
    uint32_t ratio    = 0U;
    uint32_t ch0Value = 0U;
    uint32_t ch1Value = 0U;
    uint32_t ratio1   = 0U;
    int32_t tempValue = 0U;

    uint16_t b, m;

    uint16_t ch0 = channelValues[0], ch1 = channelValues[1];

    switch (selectedTiming)
    {
        /* 13.7 msec */
        case 0:
        {
            chScale = TSL_CHSCALE_TINT0;
            break;
        }

        /* 101 msec */
        case 1:
        {
            chScale = TSL_CHSCALE_TINT1;
            break;
        }

        /* no scaling ... integration time = 402ms */
        default:
        {
            chScale = 1 << TSL_CHSCALE;
            break;
        }
    }

    /* Scale for gain (1x or 16x) */
    if (0 == selectedGain)
    {
        chScale <<= 4;
    }

    /* Scale the channel values. */
    ch0Value = (ch0 * chScale) >> TSL_CHSCALE;
    ch1Value = (ch1 * chScale) >> TSL_CHSCALE;

    /*
     * find the ratio of the channel values (Channel1/Channel0)
     * protect against divide by zero
     */

    if (0 != ch0Value)
    {
        ratio1 = (ch1Value << (1 + TSL_RATIO_SCALE)) / ch0Value;
    }

    /* Round the ratio value. */
    ratio = (1 + ratio1) >> 1;

    if ((ratio > 0) && (ratio <= TSL_K1T))
    {
        b = TSL_B1T;
        m = TSL_M1T;
    }
    else if (ratio <= TSL_K2T)
    {
        b = TSL_B2T;
        m = TSL_M2T;
    }
    else if (ratio <= TSL_K3T)
    {
        b = TSL_B3T;
        m = TSL_M3T;
    }
    else if (ratio <= TSL_K4T)
    {
        b = TSL_B4T;
        m = TSL_M4T;
    }
    else if (ratio <= TSL_K5T)
    {
        b = TSL_B5T;
        m = TSL_M5T;
    }
    else if (ratio <= TSL_K6T)
    {
        b = TSL_B6T;
        m = TSL_M6T;
    }
    else if (ratio <= TSL_K7T)
    {
        b = TSL_B7T;
        m = TSL_M7T;
    }
    else if (ratio > TSL_K8T)
    {
        b = TSL_B8T;
        m = TSL_M8T;
    }
    tempValue = ((ch0Value * b) - (ch1Value * m));

    /* Do not allow negative lux value. */
    if (tempValue < 0)
    {
        tempValue = 0;
    }

    /* Round lsb ( 2^(LUX_SCALE-1) ). */
    tempValue = tempValue + (1 << (TSL_LUX_SCALE - 1));

    /* Strip off fractional portion. */
    *luxValue = tempValue >> TSL_LUX_SCALE;

    return kStatus_TSL_Success;
}

tsl_status_t TSL_GetAmbiLuxValue(tsl_handle_t *handle, uint32_t *value)
{
    assert((NULL != handle) && (NULL != value));
    tsl_status_t status = kStatus_TSL_Success;
    uint16_t buff[2]    = {0};

    /* Read raw data from sensor. */
    status |= TSL_ReadRawData(handle, kTSL_Channel0 | kTSL_Channel1, buff);
    /* Translate raw data to Lux value. */
    status |= TSL_CalculateLux(buff, value);
    if (kStatus_TSL_Success != status)
    {
        return kStatus_TSL_Error;
    }
    else
    {
        return kStatus_TSL_Success;
    }
}
