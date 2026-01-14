/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_PD_I2C_H__
#define __USB_PD_I2C_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*!
 * @addtogroup usb_pd_i2c_wrapper
 * @{
 */

/*! @brief I2C instance count */
#define USB_PD_I2C_INSTANCE_COUNT (4U)

/*! @brief I2C transfer retry count */
#define I2C_TRANSFER_RETRY_COUNT (5U)

/*! @brief usb pd I2C driver handle */
typedef void *usb_pd_i2c_handle;
typedef void (*PD_I2cReleaseBus)(void);

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief Initialize I2C driver adapter instance.
 *
 * This function return the #i2c_driver_adpter_t instance, the other API use this as the parameter.
 *
 * @param[out] i2cHandle       Return the handle.
 * @param[in] i2cInstance      the I2C instance index, see #pd_phy_interface_t
 *
 * @retval kStatus_PD_Success    initialization success.
 * @retval kStatus_PD_Error      error code.
 */
pd_status_t PD_I2cInit(usb_pd_i2c_handle *i2cHandle,
                       uint8_t i2cInstance,
                       uint32_t i2cSrcFreq,
                       PD_I2cReleaseBus i2cReleaseBus);

/*!
 * @brief De-initialize I2C driver adapter instance.
 *
 * @param[in] i2cHandle        The handle from PD_I2cInit
 *
 * @retval kStatus_PD_Success    initialization success.
 * @retval kStatus_PD_Error      error code.
 */
pd_status_t PD_I2cDeinit(usb_pd_i2c_handle i2cHandle);

/*!
 * @brief Read data from slave.
 *
 * @param[in] i2cHandle      The handle from PD_I2cInit.
 * @param[in] slave          For I2C it is slave address.
 * @param[in] registerAddr   The access register address. Transferred MSB(most significant byte) first.
 * @param[in] registerLen    The register addreess's length, normally it is one byte or two bytes.
 * @param[in] data           The data buffer.
 * @param[in] num            The data length.
 *
 * @retval kStatus_PD_Success    success.
 * @retval kStatus_PD_Error      error code.
 */
pd_status_t PD_I2cReadBlocking(usb_pd_i2c_handle i2cHandle,
                               uint32_t slave,
                               uint32_t registerAddr,
                               uint8_t registerLen,
                               uint8_t *data,
                               uint32_t num);

/*!
 * @brief Write data to slave.
 *
 * @param[in] i2cHandle      The handle from PD_I2cInit.
 * @param[in] slave          For I2C it is slave address.
 * @param[in] registerAddr   The access register address. Transferred MSB(most significant byte) first.
 * @param[in] registerLen    The register addreess's length, normally it is one byte or two bytes.
 * @param[in] data           The data buffer.
 * @param[in] num            The data length.
 *
 * @retval kStatus_PD_Success    success.
 * @retval kStatus_PD_Error      error code.
 */
pd_status_t PD_I2cWriteBlocking(usb_pd_i2c_handle i2cHandle,
                                uint32_t slave,
                                uint32_t registerAddr,
                                uint8_t registerLen,
                                uint8_t *data,
                                uint32_t num);

/*! @}*/

#endif
