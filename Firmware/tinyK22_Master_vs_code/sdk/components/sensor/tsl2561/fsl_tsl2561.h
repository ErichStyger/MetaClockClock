/*
 * Copyright  2017 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_TSL2561_H_
#define _FSL_TSL2561_H_

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define TSL_COMMAND_BIT (0x80) /* Must be 1 */
#define TSL_CLEAR_BIT   (0x40) /* Clears any pending interrupt (write 1 to clear) */
#define TSL_WORD_BIT    (0x20) /* read/write word (rather than byte) */
#define TSL_BLOCK_BIT   (0x10) /* using block read/write */

#define TSL_CONTROL_POWERON  (0x03)
#define TSL_CONTROL_POWEROFF (0x00)

#define TSL_LUX_SCALE     (14)     /* Scale by 2^14 */
#define TSL_RATIO_SCALE   (9)      /* Scale ratio by 2^9 */
#define TSL_CHSCALE       (10)     /* Scale channel values by 2^10 */
#define TSL_CHSCALE_TINT0 (0x7517) /* 322/11 * 2^TSL_LUX_CHSCALE */
#define TSL_CHSCALE_TINT1 (0x0FE7) /* 322/81 * 2^TSL_LUX_CHSCALE */

/* T, FN and CL package coefficients */
#define TSL_K1T (0x0040) /* 0.125 * 2^RATIO_SCALE */
#define TSL_B1T (0x01f2) /* 0.0304 * 2^LUX_SCALE */
#define TSL_M1T (0x01be) /* 0.0272 * 2^LUX_SCALE */
#define TSL_K2T (0x0080) /* 0.250 * 2^RATIO_SCALE */
#define TSL_B2T (0x0214) /* 0.0325 * 2^LUX_SCALE */
#define TSL_M2T (0x02d1) /* 0.0440 * 2^LUX_SCALE */
#define TSL_K3T (0x00c0) /* 0.375 * 2^RATIO_SCALE */
#define TSL_B3T (0x023f) /* 0.0351 * 2^LUX_SCALE */
#define TSL_M3T (0x037b) /* 0.0544 * 2^LUX_SCALE */
#define TSL_K4T (0x0100) /* 0.50 * 2^RATIO_SCALE */
#define TSL_B4T (0x0270) /* 0.0381 * 2^LUX_SCALE */
#define TSL_M4T (0x03fe) /* 0.0624 * 2^LUX_SCALE */
#define TSL_K5T (0x0138) /* 0.61 * 2^RATIO_SCALE */
#define TSL_B5T (0x016f) /* 0.0224 * 2^LUX_SCALE */
#define TSL_M5T (0x01fc) /* 0.0310 * 2^LUX_SCALE */
#define TSL_K6T (0x019a) /* 0.80 * 2^RATIO_SCALE */
#define TSL_B6T (0x00d2) /* 0.0128 * 2^LUX_SCALE */
#define TSL_M6T (0x00fb) /* 0.0153 * 2^LUX_SCALE */
#define TSL_K7T (0x029a) /* 1.3 * 2^RATIO_SCALE */
#define TSL_B7T (0x0018) /* 0.00146 * 2^LUX_SCALE */
#define TSL_M7T (0x0012) /* 0.00112 * 2^LUX_SCALE */
#define TSL_K8T (0x029a) /* 1.3 * 2^RATIO_SCALE */
#define TSL_B8T (0x0000) /* 0.000 * 2^LUX_SCALE */
#define TSL_M8T (0x0000) /* 0.000 * 2^LUX_SCALE */

/* CS package values */
#define TSL_K1C (0x0043) /* 0.130 * 2^RATIO_SCALE */
#define TSL_B1C (0x0204) /* 0.0315 * 2^LUX_SCALE */
#define TSL_M1C (0x01ad) /* 0.0262 * 2^LUX_SCALE */
#define TSL_K2C (0x0085) /* 0.260 * 2^RATIO_SCALE */
#define TSL_B2C (0x0228) /* 0.0337 * 2^LUX_SCALE */
#define TSL_M2C (0x02c1) /* 0.0430 * 2^LUX_SCALE */
#define TSL_K3C (0x00c8) /* 0.390 * 2^RATIO_SCALE */
#define TSL_B3C (0x0253) /* 0.0363 * 2^LUX_SCALE */
#define TSL_M3C (0x0363) /* 0.0529 * 2^LUX_SCALE */
#define TSL_K4C (0x010a) /* 0.520 * 2^RATIO_SCALE */
#define TSL_B4C (0x0282) /* 0.0392 * 2^LUX_SCALE */
#define TSL_M4C (0x03df) /* 0.0605 * 2^LUX_SCALE */
#define TSL_K5C (0x014d) /* 0.65 * 2^RATIO_SCALE */
#define TSL_B5C (0x0177) /* 0.0229 * 2^LUX_SCALE */
#define TSL_M5C (0x01dd) /* 0.0291 * 2^LUX_SCALE */
#define TSL_K6C (0x019a) /* 0.80 * 2^RATIO_SCALE */
#define TSL_B6C (0x0101) /* 0.0157 * 2^LUX_SCALE */
#define TSL_M6C (0x0127) /* 0.0180 * 2^LUX_SCALE */
#define TSL_K7C (0x029a) /* 1.3 * 2^RATIO_SCALE */
#define TSL_B7C (0x0037) /* 0.00338 * 2^LUX_SCALE */
#define TSL_M7C (0x002b) /* 0.00260 * 2^LUX_SCALE */
#define TSL_K8C (0x029a) /* 1.3 * 2^RATIO_SCALE */
#define TSL_B8C (0x0000) /* 0.000 * 2^LUX_SCALE */
#define TSL_M8C (0x0000) /* 0.000 * 2^LUX_SCALE */

#define TSL_REG_CONTROL          (0x00)
#define TSL_REG_TIMING           (0x01)
#define TSL_REG_THRESHHOLDL_LOW  (0x02)
#define TSL_REG_THRESHHOLDL_HIGH (0x03)
#define TSL_REG_THRESHHOLDH_LOW  (0x04)
#define TSL_REG_THRESHHOLDH_HIGH (0x05)
#define TSL_REG_INTERRUPT        (0x06)
#define TSL_REG_CRC              (0x08)
#define TSL_REG_ID               (0x0A)
#define TSL_REG_CHAN0_LOW        (0x0C)
#define TSL_REG_CHAN0_HIGH       (0x0D)
#define TSL_REG_CHAN1_LOW        (0x0E)
#define TSL_REG_CHAN1_HIGH       (0x0F)

typedef enum _tsl_intergration_time
{
    kTSL_IntergrationTime13ms  = 0, /* Intergration time is 13.7ms. */
    kTSL_IntergrationTime101ms = 1, /* Intergration time is 101ms. */
    kTSL_IntergrationTime402ms = 2  /* Intergration time is 402ms. */
} tsl_intergration_time_t;

typedef enum _tsl_gain_config
{
    kTSL_GainX0  = 0,   /* No gain. */
    kTSL_GainX16 = 0x10 /* 16x gain. */
} tsl_gain_config_t;

enum _tsl_channel_select
{
    kTSL_Channel0 = 0x01,
    kTSL_Channel1 = 0x02
};

typedef enum _tsl_status
{
    kStatus_TSL_Success,
    kStatus_TSL_Error,
    kStatus_TSL_ProtocolError,
    kStatus_TSL_ParameterError,
} tsl_status_t;

typedef status_t (*I2C_SendFunc_t)(uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint32_t txBuff);
typedef status_t (*I2C_ReceiveFunc_t)(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);

typedef struct _tsl_config
{
    tsl_gain_config_t gain;
    tsl_intergration_time_t timing;
    /* Pointer to the user-defined I2C Send Data function. */
    I2C_SendFunc_t I2C_SendFunc;
    /* Pointer to the user-defined I2C Receive Data function. */
    I2C_ReceiveFunc_t I2C_ReceiveFunc;
    /* The I2C slave address . */
    uint8_t slaveAddress;
} tsl_config_t;

typedef struct _tsl_handle
{
    /* Pointer to the user-defined I2C Send Data function. */
    I2C_SendFunc_t I2C_SendFunc;
    /* Pointer to the user-defined I2C Receive Data function. */
    I2C_ReceiveFunc_t I2C_ReceiveFunc;
    /* The I2C slave address . */
    uint8_t slaveAddress;
} tsl_handle_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* _cplusplus */

/*!
 * @brief Initialize the sensor
 * @param  handle  handle to be populated with I2C info
 * @param  config  user settings for sensor
 * @return  status flag
 */
tsl_status_t TSL_Init(tsl_handle_t *handle, tsl_config_t *config);

/*!
 * @brief Enable the sensor
 * @param  handle  handle to be populated with I2C info
 */
tsl_status_t TSL_EnableSensor(tsl_handle_t *handle);

/*!
 * @brief Disable the sensor
 * @param  handle  handle to be populated with I2C info
 */
tsl_status_t TSL_DisableSensor(tsl_handle_t *handle);

/*!
 * @brief Set sensor integration time and gain value.
 * @param  handle  handle to be populated with I2C info
 * @param  config  integration time and gain value for sensor
 * @return         status flag
 */
tsl_status_t TSL_SetGainAndTiming(tsl_handle_t *handle, tsl_config_t *config);

/*!
 * @brief Read raw sensor data
 * @param  channel    TSL channel (red or infrared)
 * @param  sensorData sensor data to be filled
 * @return            status flag
 */
tsl_status_t TSL_ReadRawData(tsl_handle_t *handle, uint32_t channel, uint16_t *data);

/*!
 * @brief Calculate luminance
 * @param channelValues the raw data of channel values.
 * @param luxValue      the lux value of sensor.
 * @return            status flag
 */
tsl_status_t TSL_CalculateLux(uint16_t *channelValues, uint32_t *luxValue);

/*!
 * @brief Calculate luminance
 * @param  handle  handle to be populated with I2C info
 * @param value    the lux value of sensor.
 * @return         status flag
 */
tsl_status_t TSL_GetAmbiLuxValue(tsl_handle_t *handle, uint32_t *value);

#if defined(__cplusplus)
}
#endif /* _cplusplus */

#endif /* _FSL_TSL2561_H_ */
