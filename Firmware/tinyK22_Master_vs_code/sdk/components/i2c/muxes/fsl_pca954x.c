/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_pca954x.h"

static const pca954x_chip_desc_t chips[] = {
#if defined(MCUX_ENABLE_MAX7356)
    [MAX7356_ID] = {
      .nchans = 8,
      .muxtype = pca954x_isswi,
    },
#endif
#if defined(MCUX_ENABLE_MAX7357)
    [MAX7357_ID] = {
      .nchans = 8,
      .muxtype = pca954x_isswi,
    },
#endif
#if defined(MCUX_ENABLE_MAX7358)
    [MAX7358_ID] = {
      .nchans = 8,
      .muxtype = pca954x_isswi,
    },
#endif
#if defined(MCUX_ENABLE_MAX7367)
    [MAX7367_ID] = {
      .nchans = 4,
      .muxtype = pca954x_isswi,
    },
#endif
#if defined(MCUX_ENABLE_MAX7368)
    [MAX7368_ID] = {
      .nchans = 4,
      .muxtype = pca954x_isswi,
    },
#endif
#if defined(MCUX_ENABLE_MAX7369)
    [MAX7369_ID] = {
      .nchans = 4,
      .enable = 0x4,
      .muxtype = pca954x_ismux,
    },
#endif
#if defined(MCUX_ENABLE_PCA9540)
    [PCA9540_ID] = {
      .nchans = 2,
      .enable = 0x4,
      .muxtype = pca954x_ismux,
    },
#endif
#if defined(MCUX_ENABLE_PCA9542)
    [PCA9542_ID] = {
      .nchans = 2,
      .enable = 0x4,
      .muxtype = pca954x_ismux,
    },
#endif
#if defined(MCUX_ENABLE_PCA9543)
    [PCA9543_ID] = {
      .nchans = 2,
      .muxtype = pca954x_isswi,
    },
#endif
#if defined(MCUX_ENABLE_PCA9544)
    [PCA9544_ID] = {
      .nchans = 4,
      .enable = 0x4,
      .muxtype = pca954x_ismux,
    },
#endif
#if defined(MCUX_ENABLE_PCA9545)
    [PCA9545_ID] = {
      .nchans = 4,
      .muxtype = pca954x_isswi,
    },
#endif
#if defined(MCUX_ENABLE_PCA9546)
    [PCA9546_ID] = {
      .nchans = 4,
      .muxtype = pca954x_isswi,
    },
#endif
#if defined(MCUX_ENABLE_PCA9547)
    [PCA9547_ID] = {
      .nchans = 8,
      .enable = 0x8,
      .muxtype = pca954x_ismux,
    },
#endif
#if defined(MCUX_ENABLE_PCA9548)
    [PCA9548_ID] = {
      .nchans = 8,
      .muxtype = pca954x_isswi,
    },
#endif
#if defined(MCUX_ENABLE_PCA9646)
    [PCA9646_ID] = {
      .nchans = 4,
      .muxtype = pca954x_isswi,
    },
#endif
#if defined(MCUX_ENABLE_PCA9846)
    [PCA9846_ID] = {
      .nchans = 4,
      .muxtype = pca954x_isswi,
    },
#endif
#if defined(MCUX_ENABLE_PCA9847)
    [PCA9847_ID] = {
      .nchans = 8,
      .enable = 0x8,
      .muxtype = pca954x_ismux,
    },
#endif
#if defined(MCUX_ENABLE_PCA9848)
    [PCA9848_ID] = {
      .nchans = 8,
      .muxtype = pca954x_isswi,
    },
#endif
#if defined(MCUX_ENABLE_PCA9849)
    [PCA9849_ID] = {
      .nchans = 4,
      .enable = 0x4,
      .muxtype = pca954x_ismux,
    },
#endif
};

/*
 * brief Initializes the PCA654X driver handle.
 *
 * param handle Pointer to the PCA954x handle.
 * param config Pointer to the PCA954x configuration structure.
 */
void PCA954X_Init(pca954x_handle_t *handle, const pca954x_config_t *config)
{
    assert(NULL != handle);
    assert(NULL != config);
    assert(config->id < UNKNOWN_ID);

    handle->i2cBase         = config->i2cBase;
    handle->id              = config->id;
    handle->i2cAddr         = config->i2cAddr;
    handle->I2C_SendFunc    = config->I2C_SendFunc;
    handle->I2C_ReceiveFunc = config->I2C_ReceiveFunc;
    handle->chip            = &chips[handle->id];
}

#if 0
/*! @brief Read register value. */
static status_t PCA954X_RegRead(pca954x_handle_t *handle, uint8_t *value)
{
    return handle->I2C_ReceiveFunc(handle->i2cBase, handle->i2cAddr, 0U, 0U, (uint8_t *)value, 1U, 0U);
}
#endif

/*! @brief Write port registers value. */
static status_t PCA954X_RegWrite(pca954x_handle_t *handle, uint8_t value)
{
    return handle->I2C_SendFunc(handle->i2cBase, handle->i2cAddr, 0U, 0U, (uint8_t *)&value, 1U, 0U);
}

/*! @brief Convert chan to register value. */
static uint8_t PCA954X_RegVal(pca954x_handle_t *handle, uint8_t chan)
{
    uint8_t regVal = 0;
    if (handle->chip->muxtype == pca954x_ismux)
    {
        regVal = chan | handle->chip->enable;
    }
    else
    {
        regVal = 1 << chan;
    }
    return regVal;
}

/*! @brief Select channel. */
status_t PCA954X_SelectChan(pca954x_handle_t *handle, uint32_t chan)
{
    uint8_t regVal = 0U;
    status_t status = kStatus_Success;

    regVal = PCA954X_RegVal(handle, chan);
    if (handle->last_chan != regVal)
    {
        status = PCA954X_RegWrite(handle, regVal);
	handle->last_chan = status != kStatus_Success ? 0 : regVal;
    }

    return status;
}

/*! @brief Deselect chip. */
status_t PCA954X_DeselectMux(pca954x_handle_t *handle, uint32_t chan)
{
    status_t status = kStatus_Success;

    /* Deselect active channel */
    handle->last_chan = 0;
    status = PCA954X_SelectChan(handle, handle->last_chan);

    return status;
}
