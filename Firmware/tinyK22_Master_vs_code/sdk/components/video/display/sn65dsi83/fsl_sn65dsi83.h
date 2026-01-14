/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_SN65DSI83_H_
#define _FSL_SN65DSI83_H_

#include "fsl_display.h"
#include "fsl_common.h"
#include "fsl_video_i2c.h"

/*
 * Change log:
 *
 *   1.0.0
 *     - Initial version
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SN65DSI83_SOFT_RESET          (0x09)
#define SN65DSI83_PLL_EN              (0x0D)
#define SN65DSI83_LVDS_CLK            (0x0A)
#define SN65DSI83_DSI_CLK_DIV         (0x0B)
#define SN65DSI83_DSI_LINES           (0x10)
#define SN65DSI83_DSI_CLK             (0x12)
#define SN65DSI83_LVDS_FMT            (0x18)
#define SN65DSI83_LVDS_VOD_SWING      (0x19)
#define SN65DSI83_LVDS_CM_ADJUST      (0x1B)
#define SN65DSI83_ACTIVE_LENGTH_LOW   (0x20)
#define SN65DSI83_ACTIVE_LENGTH_HIGH  (0x21)
#define SN65DSI83_VERTICAL_SIZE_LOW   (0x24)
#define SN65DSI83_VERTICAL_SIZE_HIGH  (0x25)
#define SN65DSI83_SYNC_DELAY          (0x28)
#define SN65DSI83_HSW                 (0x2C)
#define SN65DSI83_VSW                 (0x30)
#define SN65DSI83_HBP                 (0x34)
#define SN65DSI83_VBP                 (0x36)
#define SN65DSI83_HFP                 (0x38)
#define SN65DSI83_VFP                 (0x3A)
#define SN65DSI83_TEST_PATTERN        (0x3C)
#define SN65DSI83_ERROR_REG           (0xE5)

/*!
 * @brief SN65DSI83 resource.
 */
typedef struct _sn65dsi83_resource
{
    video_i2c_send_func_t i2cSendFunc;       /*!< I2C Send function. */
    video_i2c_receive_func_t i2cReceiveFunc; /*!< I2C receive function. */
    uint8_t i2cAddr;                         /*!< I2C address for the main memory. */
} sn65dsi83_resource_t;

extern const display_operations_t sn65dsi83_ops;

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

status_t SN65DSI83_Init(display_handle_t *handle, const display_config_t *config);

status_t SN65DSI83_Deinit(display_handle_t *handle);

status_t SN65DSI83_Start(display_handle_t *handle);

status_t SN65DSI83_Stop(display_handle_t *handle);

#if defined(__cplusplus)
}
#endif

#endif /* _FSL_SN65DSI83_H_ */
