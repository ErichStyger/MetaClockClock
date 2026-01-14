/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_NMH1000_H_
#define _FSL_NMH1000_H_

#include "fsl_common.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* I2C Address of NMH1000 */
#define NMH1000_I2C_ADDR (0x60)

/* Address of NMH1000 registers */
#define NMH1000_CONTROL_REG1 (0x01)
#define NMH1000_OUT_M_REG (0x03)
#define NMH1000_USER_ODR (0x06)
#define NMH1000_WHO_AM_I (0x08)

#define NMH1000_MEDIUM_SAMPLE_RATE (0x02)

#define NMH1000_AUTO_MASK ((uint8_t)0x08)
#define NMH1000_ONE_SHOT_MASK ((uint8_t)0x04)

#define NMH1000_MAG_DATA_SHIFT (2)


/*! @brief Define I2C access function. */
typedef status_t (*I2C_SendFunc_t)(uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint32_t txBuff);
typedef status_t (*I2C_ReceiveFunc_t)(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);

/*! @brief NMH1000 handle structure */
typedef struct _nmh_handle
{
    /* Pointer to the user-defined I2C Send Data function. */
    I2C_SendFunc_t I2C_SendFunc;
    /* Pointer to the user-defined I2C Receive Data function. */
    I2C_ReceiveFunc_t I2C_ReceiveFunc;
} nmh_handle_t;

/*! @brief NMH1000 config structure */
typedef struct _nmh_config
{
    /* Pointer to the user-defined I2C Send Data function. */
    I2C_SendFunc_t I2C_SendFunc;
    /* Pointer to the user-defined I2C Receive Data function. */
    I2C_ReceiveFunc_t I2C_ReceiveFunc;
} nmh_config_t;


#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Verify and initialize NMH1000 handle
 *
 * @param handle The pointer to NMH1000 driver handle.
 * @param config The configuration structure pointer to NMH1000.
 *
 * @return kStatus_Success if success or kStatus_Fail if error.
 */
status_t NMH_Init(nmh_handle_t *handle, nmh_config_t *config);

/*!
 * @brief Read data from sensors - a relative magnetic field strength
 *
 * @param handle The pointer to NMH1000 driver handle.
 * @param magData The pointer to the buffer to hold sensor data.
 *
 * @return kStatus_Success if success or kStatus_Fail if error.
 */
status_t NMH_ReadMagData(nmh_handle_t *handle, uint8_t *magData);

/*!
 * @brief Write value to register of sensor.
 *
 * @param handle The pointer to NMH1000 driver handle.
 * @param reg Register address.
 * @param val Data want to write.
 *
 * @return kStatus_Success if success or kStatus_Fail if error.
 */
status_t NMH_WriteReg(nmh_handle_t *handle, uint8_t reg, uint8_t val);

/*!
 * @brief Read n bytes start at register from sensor.
 *
 * @param handle The pointer to NMH1000 driver handle.
 * @param reg Register address.
 * @param val The pointer to address which store data.
 * @param bytesNumber Number of bytes receiver.
 *
 * @return kStatus_Success if success or kStatus_Fail if error.
 */
status_t NMH_ReadReg(nmh_handle_t *handle, uint8_t reg, uint8_t *val, uint8_t bytesNumber);


#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _FSL_NMH1000_H_ */
