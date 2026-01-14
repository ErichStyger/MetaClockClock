/*
 * Copyright  2017 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_HTU21D_H_
#define _FSL_HTU21D_H_

#include "fsl_timer.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define HTU_I2C_ADDRESS (0x40)

#define HTU21D_TRIGGER_TEMP_HOLD   0xE3
#define HTU21D_TRIGGER_HUMD_HOLD   0xE5
#define HTU21D_TRIGGER_TEMP_NOHOLD 0xF3
#define HTU21D_TRIGGER_HUMD_NOHOLD 0xF5
#define HTU21D_WRITE_USER_REG      0xE6
#define HTU21D_READ_USER_REG       0xE7
#define HTU21D_SOFT_RESET          0xFE

#define HTU21D_END_OF_BATTERY_SHIFT 6
#define HTU21D_ENABLE_HEATER_SHIFT  2
#define HTU21D_DISABLE_OTP_RELOAD   1
#define HTU21D_RESERVED_MASK        0x31

#define HTU21D_STARTUP_DELAY  15000
#define HTU21D_TEMP_MAX_DELAY 50000

#define HTU_TEMP_CALIB_OFFSET_0 (0)
#define HTU_TEMP_CALIB_OFFSET_1 (4)
#define HTU_TEMP_CALIB_OFFSET_2 (6)

typedef enum _htu_status
{
    kStatus_HTU_Success        = 0U,
    kStatus_HTU_Error          = 0x01U,
    kStatus_HTU_ParameterError = 0x02U,
    kStatus_HTU_ProtocolError  = 0x04U,
    kStatus_HTU_TimeOut        = 0x08U
} htu_status_t;

typedef enum _htu_resoultion
{
    kHTU_BitTypeRH12Temp14 = 0x00U,
    kHTU_BitTypeRH8Temp12  = 0x01U,
    kHTU_BitTypeRH10Temp13 = 0x02U,
    kHTU_BitTypeRH11Temp11 = 0x03U
} htu_resoultion_t;

/*! @brief Define I2C access function. */
typedef status_t (*I2C_SendFunc_t)(uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint32_t txBuff);
typedef status_t (*I2C_ReceiveFunc_t)(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);

typedef struct _htu_config
{
    /* Pointer to the user-defined I2C Send Data function. */
    I2C_SendFunc_t I2C_SendFunc;
    /* Pointer to the user-defined I2C Receive Data function. */
    I2C_ReceiveFunc_t I2C_ReceiveFunc;
    htu_resoultion_t resolution;
} htu_config_t;

typedef struct _htu_handle
{
    /* Pointer to the user-defined I2C Send Data function. */
    I2C_SendFunc_t I2C_SendFunc;
    /* Pointer to the user-defined I2C Receive Data function. */
    I2C_ReceiveFunc_t I2C_ReceiveFunc;
} htu_handle_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* _cplusplus */

/*!
 * @brief Initialize HTU21D sensor and reset the module
 * @param  handle  I2C instance and transfer config for transfer.
 * @param  config  Configuration structure.
 * @return         status flag
 */
htu_status_t HTU_Init(htu_handle_t *handle, htu_config_t *config);

/*!
 * @brief Reset the sensor by software.
 * @param handle I2C instance and transfer config for transfer.
 * @return status flag
 */
htu_status_t HTU_SoftReset(htu_handle_t *handle);

/*!
 * @brief Set the user register.
 * @param  handle I2C isntance and trasnfer config for transfer.
 * @param  type Configure the measurement resolution type
 * @return        status flag
 */
htu_status_t HTU_SetUserRegister(htu_handle_t *handle, htu_resoultion_t resolution);

/*!
 * @brief Get temperature data
 * @param  handle    I2C isntance and transfer config for THU device.
 * @param  tempData  SensorData data to be filled.
 * @return           status flag
 */
htu_status_t HTU_GetTempRawData(htu_handle_t *handle, int16_t *tempData);

/*!
 * @brief Get humidity data
 * @param  handle   I2C instance and transfer config for HTU device.
 * @param  humData  data to be filled
 * @return          status flag
 */
htu_status_t HTU_GetHumRawData(htu_handle_t *handle, int16_t *humData);

/*!
 * @brief Get temperature and humidity data
 * @param  handle       I2C isntance and transfer config for THU device.
 * @param  temperature  address to place the temperature data to
 * @param  humidity     address to place the humidity data to
 * @return              status flag
 */
htu_status_t HTU_ReadRawData(htu_handle_t *handle, int16_t *temperature, int16_t *humidity);

/*!
 * @brief Get temperature data
 * @param  handle    I2C isntance and transfer config for THU device.
 * @param  tempData  SensorData data to be filled(type float).
 * @return           status flag
 */
htu_status_t HTU_GetTempData(htu_handle_t *handle, void *tempData);

/*!
 * @brief Get humidity data
 * @param  handle   I2C instance and transfer config for HTU device.
 * @param  humData  data to be filled(type float)
 * @return          status flag
 */
htu_status_t HTU_GetHumData(htu_handle_t *handle, void *humData);

#if defined(__cplusplus)
}
#endif /* _cplusplus */

#endif /* _FSL_HTU21D_H_ */
