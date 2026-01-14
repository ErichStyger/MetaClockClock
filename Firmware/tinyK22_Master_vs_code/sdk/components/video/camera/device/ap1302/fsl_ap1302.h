/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_AP1302_H_
#define _FSL_AP1302_H_

#include "fsl_common.h"
#include "fsl_camera_device.h"
#include "fsl_video_i2c.h"

/*
 * Change log:
 *   1.0.0
 *     - Initial version
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @brief AP1302 resource.
 *
 * Before initialize the AP1302, the resource must be initialized that
 * I2C could start to work.
 */
typedef struct _ap1302_resource
{
    video_i2c_send_func_t i2cSendFunc;                           /*!< I2C send function, uin8_t tx buffersize. */
    video_i2c_send_txbuffersize_32bit_func_t i2cSendFunc32bit;   /*!< I2C send function, uin32_t tx buffersize. */
    video_i2c_receive_func_t i2cReceiveFunc;                     /*!< I2C receive function. */
    void (*pullResetPin)(bool pullUp);                           /*!< Function to pull reset pin high or low. */
    void (*pullPowerDownPin)(bool pullUp);                       /*!< Function to pull the power down pin high or low. */
    void (*ispBypassPin)(bool pullUp);                           /*!< Function to enable ISP by default. */
    void (*powerUp)(bool power);                                 /*!< Function to power up sensor and isp device. */
} ap1302_resource_t;

/*!
 * @brief AP1302 firmware resource.
 *
 * AP1302 firmware including essential setting for ISP core, it needed by load into AP1302 RAM.
 */
struct ap1302_firmware_t
{
    uint32_t crc;                            /*!< AP1302 crc check. */
    uint32_t checksum;                       /*!< AP1302 checksum. */
    uint32_t pll_init_size;                  /*!< Bootdata Pll init setting size. */
    uint32_t total_size;                     /*!< Bootdata all setting size. */
};

/*! @brief AP1302 operation functions. */
extern const camera_device_operations_t ap1302_ops;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
}
#endif

#endif /* _FSL_AP1302_H_ */
