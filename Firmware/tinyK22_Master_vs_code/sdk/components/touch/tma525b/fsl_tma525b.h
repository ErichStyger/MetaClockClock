/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_TMA525B_H_
#define _FSL_TMA525B_H_

#include "fsl_common.h"

/*
 * Change log:
 *
 *   1.0.0
 *     - Initial version
 */

/*!
 * @addtogroup tma525b
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief TMA525B I2C address. */
#define TMA525B_I2C_ADDRESS (0x24U)

/*! @brief TMA525B maximum number of simultaneously detected touches. */
#define TMA525B_MAX_TOUCHES (4U)

/*! @brief TMA525B register address where touch data begin. */
#define TMA525B_TOUCH_DATA_SUBADDR (1)

/*! @brief TMA525B raw touch data length. */
#define TMA525B_TOUCH_DATA_LEN (264) // * 7+4*10 =47

/*! @brief TMA525B I2C receive function. */
typedef status_t (*tma525b_i2c_receive_func_t)(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);

/*! @brief TMA525B I2C send function. */
typedef status_t (*tma525b_i2c_send_func_t)(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize);

/*! @brief Function to pull reset pin up or down. */
typedef void (*tma525b_reset_pin_func_t)(bool pullUp);

/*! @brief Function to pull power pin up or down. */
typedef void (*tma525b_power_pin_func_t)(bool pullUp);

/*! @brief Touch event. */
typedef enum _touch_event
{
    kTouch_Reserved = 0, /*!< No touch event detected. */
    kTouch_Down     = 1, /*!< Touch down event detected. */
    kTouch_Contact  = 2, /*!< Touch point moving, indicating a scrolling happening. */
    kTouch_Up       = 3  /*!< Touch event finished. */
} touch_event_t;

/*! @brief Touch point information. */
typedef struct _touch_point
{
    touch_event_t TOUCH_EVENT; /*!< Indicates the state or event of the touch point. */
    uint8_t TOUCH_ID; /*!< ID of the touch point. This numeric value stays constant between down and up event. */
    uint16_t TOUCH_X; /*!< X coordinate of the touch point */
    uint16_t TOUCH_Y; /*!< Y coordinate of the touch point */
} touch_point_t;

/*! @brief FT3267 configure structure.*/
typedef struct _tma525b_config
{
    tma525b_i2c_send_func_t I2C_SendFunc;       /*!< Function to send I2C data. */
    tma525b_i2c_receive_func_t I2C_ReceiveFunc; /*!< Function to receive I2C data. */
    tma525b_reset_pin_func_t pullResetPinFunc;  /*!< Function to pull reset pin high or low. */
    tma525b_power_pin_func_t pullPowerPinFunc;  /*!< Function to pull power pin high or low. */
    void (*timeDelayMsFunc)(uint32_t delayMs);  /*!< Function to delay some MS. */
} tma525b_config_t;

/*! @brief FT3267 driver handle. */
typedef struct _tma525b_handle
{
    tma525b_i2c_send_func_t I2C_SendFunc;       /*!< Function to send I2C data. */
    tma525b_i2c_receive_func_t I2C_ReceiveFunc; /*!< Function to receive I2C data. */
    tma525b_reset_pin_func_t pullResetPinFunc;  /*!< Function to pull reset pin high or low. */
    tma525b_power_pin_func_t pullPowerPinFunc;  /*!< Function to pull power pin high or low. */
    uint8_t touchBuf[TMA525B_TOUCH_DATA_LEN];   /*!< Buffer to receive touch point raw data. */
} tma525b_handle_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initialize the driver.
 *
 * This function power on the touch controller, releases the touch controller
 * reset. After calling this function, the touch controller is ready to work.
 *
 * @param [in] handle Pointer to the driver.
 * @param [in] config Pointer to the configuration.
 * @return Returns @ref kStatus_Success if initialize success, otherwise return error code.
 */
status_t TMA525B_Init(tma525b_handle_t *handle, const tma525b_config_t *config);

/*!
 * @brief De-initialize the driver.
 *
 * After this function, the touch controller is powered off.
 *
 * @param [in] handle Pointer to the driver.
 * @return Returns @ref kStatus_Success if success, otherwise return error code.
 */
status_t TMA525B_Deinit(tma525b_handle_t *handle);

/*!
 * @brief Get single touch point coordinate.
 *
 * Get one touch point coordinate.
 *
 * @param [in] handle Pointer to the driver.
 * @param [out] touch_x X coordinate of the touch point.
 * @param [out] touch_y Y coordinate of the touch point.
 * @return Returns @ref kStatus_Success if success, otherwise return error code.
 */
status_t TMA525B_GetSingleTouch(tma525b_handle_t *handle, touch_event_t *touch_event, int *touch_x, int *touch_y);

/*!
 * @brief Get multiple touch points coordinate.
 *
 * When this function returns successfully, the @p touch_count shows how
 * many valid touch points there are in the @p touch_array.
 *
 * @param [in] handle Pointer to the driver.
 * @param [out] touch_count The actual touch point number.
 * @param [out] touch_array Array of touch points coordinate.
 * @return Returns @ref kStatus_Success if success, otherwise return error code.
 */
status_t TMA525B_GetMultiTouch(tma525b_handle_t *handle,
                               int *touch_count,
                               touch_point_t touch_array[TMA525B_MAX_TOUCHES]);

#if defined(__cplusplus)
}
#endif

/*! @} */

#endif
