/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_PCA954X_H_
#define _FSL_PCA954X_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_common.h"

/*
 * Change log:
 *
 *   1.0.0
 *     - Initial version
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief PCA954X I2C receive function. */
typedef status_t (*pca954x_i2c_receive_func_t)( void *base,
                                                uint8_t deviceAddress,
                                                uint32_t subAddress,
                                                uint8_t subaddressSize,
                                                uint8_t *rxBuff,
                                                uint8_t rxBuffSize,
                                                uint32_t flags);

/*! @brief PCA954X I2C send function. */
typedef status_t (*pca954x_i2c_send_func_t)( void *base,
                                             uint8_t deviceAddress,
                                             uint32_t subAddress,
                                             uint8_t subaddressSize,
                                             const uint8_t *txBuff,
                                             uint8_t txBuffSize,
                                             uint32_t flags);
/*! @brief i2c device id */
enum pca954x_id {
#if defined(MCUX_ENABLE_MAX7356)
    MAX7356_ID,
#endif
#if defined(MCUX_ENABLE_MAX7357)
    MAX7357_ID,
#endif
#if defined(MCUX_ENABLE_MAX7358)
    MAX7358_ID,
#endif
#if defined(MCUX_ENABLE_MAX7367)
    MAX7367_ID,
#endif
#if defined(MCUX_ENABLE_MAX7368)
    MAX7368_ID,
#endif
#if defined(MCUX_ENABLE_MAX7369)
    MAX7369_ID,
#endif
#if defined(MCUX_ENABLE_PCA9540)
    PCA9540_ID,
#endif
#if defined(MCUX_ENABLE_PCA9542)
    PCA9542_ID,
#endif
#if defined(MCUX_ENABLE_PCA9543)
    PCA9543_ID,
#endif
#if defined(MCUX_ENABLE_PCA9544)
    PCA9544_ID,
#endif
#if defined(MCUX_ENABLE_PCA9545)
    PCA9545_ID,
#endif
#if defined(MCUX_ENABLE_PCA9546)
    PCA9546_ID,
#endif
#if defined(MCUX_ENABLE_PCA9547)
    PCA9547_ID,
#endif
#if defined(MCUX_ENABLE_PCA9548)
    PCA9548_ID,
#endif
#if defined(MCUX_ENABLE_PCA9646)
    PCA9646_ID,
#endif
#if defined(MCUX_ENABLE_PCA9846)
    PCA9846_ID,
#endif
#if defined(MCUX_ENABLE_PCA9847)
    PCA9847_ID,
#endif
#if defined(MCUX_ENABLE_PCA9848)
    PCA9848_ID,
#endif
#if defined(MCUX_ENABLE_PCA9849)
    PCA9849_ID,
#endif
    UNKNOWN_ID,
};


/*! @brief describe chip feature */
typedef struct _pca954x_chip_desc {
    uint8_t nchans;
    uint8_t enable; /* used for muxes only */
    enum muxtype {
	    pca954x_ismux = 0,
	    pca954x_isswi,
    } muxtype;
} pca954x_chip_desc_t;

/*! @brief PCA954X configure structure.*/
typedef struct _pca954x_config
{
    uint8_t i2cAddr;                            /*!< I2C Device address. */
    enum pca954x_id id;                         /*!< I2C Device Id. */
    void *i2cBase;                              /*!< I2C Controller Base. */
    pca954x_i2c_send_func_t I2C_SendFunc;       /*!< Function to send I2C data. */
    pca954x_i2c_receive_func_t I2C_ReceiveFunc; /*!< Function to receive I2C data. */
} pca954x_config_t;

/*! @brief PCA954X driver handle. */
typedef struct _pca954x_handle
{
    uint8_t i2cAddr;                            /*!< I2C Device address. */
    uint8_t last_chan;                          /*!< Last chan. */
    enum pca954x_id id;                         /*!< I2C Device Id. */
    void *i2cBase;                              /*!< I2C Controller Base. */
    const pca954x_chip_desc_t * chip;     /*!< I2C Chip Spec */
    pca954x_i2c_send_func_t I2C_SendFunc;       /*!< Function to send I2C data. */
    pca954x_i2c_receive_func_t I2C_ReceiveFunc; /*!< Function to receive I2C data. */
} pca954x_handle_t;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

/*
 * brief Initializes the PCA654X driver handle.
 *
 * param handle Pointer to the PCA954x handle.
 * param config Pointer to the PCA954x configuration structure.
 */
void PCA954X_Init(pca954x_handle_t *handle, const pca954x_config_t *config);

/*! @brief Select channel. */
status_t PCA954X_SelectChan(pca954x_handle_t *handle, uint32_t chan);

/*! @brief Deselect chip. */
status_t PCA954X_DeselectMux(pca954x_handle_t *handle, uint32_t chan);

#if defined(__cplusplus)
}
#endif

#endif /* _FSL_PCA954X_H_ */

