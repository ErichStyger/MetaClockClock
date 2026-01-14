/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FSL_FXAS_H_
#define _FSL_FXAS_H_

#include "fsl_common.h"

#define FXAS_GYRO_I2C_ADDRESS 0x20
#define FXAS_GYRO_FIFO_SIZE   32 /* FXAS21002 have 32 element FIFO */

/*
 *  STATUS Register
 */
#define FXAS_STATUS_00_REG 0x00

/*
 *  XYZ Data Registers
 */
#define FXAS_OUT_X_MSB_REG 0x01
#define FXAS_OUT_X_LSB_REG 0x02
#define FXAS_OUT_Y_MSB_REG 0x03
#define FXAS_OUT_Y_LSB_REG 0x04
#define FXAS_OUT_Z_MSB_REG 0x05
#define FXAS_OUT_Z_LSB_REG 0x06

/*
 *  DR Status Register
 */
#define FXAS_DR_STATUS_REG 0x07

/*
 *  DR Status Register
 */
#define FXAS_F_STATUS_REG 0x08
#define FXAS_F_CNT_MASK   0x3F
/*
 *  F_SETUP FIFO Setup Register
 */
#define FXAS_F_SETUP_REG 0x09

#define FXAS_F_MODE1_MASK 0x80
#define FXAS_F_MODE0_MASK 0x40
#define FXAS_F_WMRK5_MASK 0x20
#define FXAS_F_WMRK4_MASK 0x10
#define FXAS_F_WMRK3_MASK 0x08
#define FXAS_F_WMRK2_MASK 0x04
#define FXAS_F_WMRK1_MASK 0x02
#define FXAS_F_WMRK0_MASK 0x01
#define FXAS_F_MODE_MASK  0xC0
#define FXAS_F_WMRK_MASK  0x3F
#define FXAS_F_MODE_SHIFT 6

#define FXAS_F_MODE_DISABLED 0x00

/*
 *  F_EVENT Register
 */
#define FXAS_F_EVENT_REG 0x0A

/*
 *  INT_SOURCE System Interrupt Status Register
 */
#define FXAS_INT_SOURCE_REG 0x0B

#define FXAS_SRC_BOOTEND_MASK 0x08
#define FXAS_SRC_FIFO_MASK    0x04
#define FXAS_SRC_RT_MASK      0x02
#define FXAS_SRC_DRDY_MASK    0x01

/*
 *  WHO_AM_I Device ID Register
 */
#define FXAS_WHO_AM_I_REG 0x0C

/* Content */
#define kFXAS_WHO_AM_I_Device_ID 0xD7

/*
 *  FXAS_CTRL_REG0 Register
 */
#define FXAS_CTRL_REG0 0x0D

/* Rate Threshold configure Register */
#define FXAS_RT_CFG_REG 0x0E

#define FXAS_RT_XTEFE_MASK 0x01
#define FXAS_RT_YTEFE_MASK 0x02
#define FXAS_RT_ZTEFE_MASK 0x04
#define FXAS_RT_ELE_MASK   0x08
#define RT_ALLAXES_MASK    (FXAS_RT_XTEFE_MASK | FXAS_RT_YTEFE_MASK | FXAS_RT_ZTEFE_MASK)

/* Rate threshold event source Register */
#define FXAS_RT_SRC_REG 0x0F

#define FXAS_RT_EA_MASK      0x40
#define FXAS_RT_ZRT_MASK     0x20
#define FXAS_RT_ZRT_POL_MASK 0x10
#define FXAS_RT_YRT_MASK     0x08
#define FXAS_RT_YRT_POL_MASK 0x04
#define FXAS_RT_XRT_MASK     0x02
#define FXAS_RT_XRT_POL_MASK 0x01

/*
 *  RT_THS Register
 */
#define FXAS_RT_THS_REG 0x10

/*
 *  RT_COUNT Register
 */
#define FXAS_RT_COUNT_REG 0x11

/*
 *  TEMP Register
 */
#define FXAS_RT_TEMP_REG 0x12

/* FXAS_CTRL_REG1 System Control 1 Register */
#define FXAS_CTRL_REG1 0x13

#define FXAS_RFXAS_ST_MASK 0x40
#define FXAS_ST_MASK       0x20
#define FXAS_DR2_MASK      0x10
#define FXAS_DR1_MASK      0x08
#define FXAS_DR0_MASK      0x04
#define FXAS_OP_MASK       0x03
#define FXAS_ACTIVE_MASK   0x02
#define FXAS_DR_MASK       0x38

#define DATA_RATE_OFFSET (2)

#define ACTIVE       (FXAS_ACTIVE_MASK)
#define FXAS_STANDBY 0x00

/* FXAS_CTRL_REG2 System Control 2 Register */
#define FXAS_CTRL_REG2 0x14

#define FXAS_INT_CFG_FIFO_MASK 0x80
#define FXAS_INT_EN_FIFO_MASK  0x40
#define FXAS_INT_CFG_RT_MASK   0x20
#define FXAS_INT_EN_RT_MASK    0x10
#define FXAS_INT_CFG_DRDY_MASK 0x08
#define FXAS_INT_EN_DRDY_MASK  0x04
#define FXAS_IPOL_MASK         0x02
#define FXAS_PP_OD_MASK        0x01

/* FXAS_CTRL_REG3 System Control 3 Register */
#define FXAS_CTRL_REG3 0x15

#define FXAS_WRAPTDONE_MASK 0x08
#define FXAS_EXTCTRLEN_MASK 0x04
#define FXAS_FS_DOUBLE_MASK 0x01

/*! @brief Full-scale range definition.*/
typedef enum fxas_gfsr
{
    kFXAS_Gfsr_2000DPS,
    kFXAS_Gfsr_1000DPS,
    kFXAS_Gfsr_500DPS,
    kFXAS_Gfsr_250DPS
} fxas_gfsr_t;

typedef enum fxas_odr
{
    kFXAS_Godr_800Hz,
    kFXAS_Godr_400Hz,
    kFXAS_Godr_200Hz,
    kFXAS_Godr_100Hz,
    kFXAS_Godr_50Hz,
    kFXAS_Godr_25Hz,
    kFXAS_Godr_12_5Hz,
} fxas_odr_t;

typedef enum fxas_fifo
{
    kFXAS_FIFO_Disabled,
    kFXAS_FIFO_CircularMode,
    kFXAS_FIFO_StopMode
} fxas_fifo_t;

typedef struct fxas_data
{
    uint8_t gyroXMSB;
    uint8_t gyroXLSB;
    uint8_t gyroYMSB;
    uint8_t gyroYLSB;
    uint8_t gyroZMSB;
    uint8_t gyroZLSB;

} fxas_data_t;

/*! @brief Define I2C callback function. */
typedef status_t (*I2C_SendFunc_t)(uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint32_t txBuff);
typedef status_t (*I2C_ReceiveFunc_t)(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);

typedef struct fxas_config
{
    /* Pointer to the user-defined I2C Send Data function. */
    I2C_SendFunc_t I2C_SendFunc;
    /* Pointer to the user-defined I2C Receive Data function. */
    I2C_ReceiveFunc_t I2C_ReceiveFunc;
    fxas_gfsr_t fsrdps;
    fxas_odr_t odr;
    fxas_fifo_t fifo;
} fxas_config_t;

/*! @brief fxas21002cq configure definition. This structure should be global.*/
typedef struct _fxas_handle
{
    /* Pointer to the user-defined I2C Send Data function. */
    I2C_SendFunc_t I2C_SendFunc;
    /* Pointer to the user-defined I2C Receive Data function. */
    I2C_ReceiveFunc_t I2C_ReceiveFunc;
} fxas_handle_t;

typedef struct _fxas21002cq_data
{
    uint8_t accelXMSB;
    uint8_t accelXLSB;
    uint8_t accelYMSB;
    uint8_t accelYLSB;
    uint8_t accelZMSB;
    uint8_t accelZLSB;
} fxas21002cq_data_t;

/*!
 * @addtogroup fxos_common
 * @{
 */

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Verify and initialize fxas_handleice: Hybrid mode with ODR=50Hz, Mag
 * OSR=32, Acc OSR=Normal.
 *
 * @param fxas_handle The pointer to accel driver handle.
 * @param configure The configuration structure.
 *
 * @return kStatus_Success if success or kStatus_Fail if error.
 */
status_t FXAS_Init(fxas_handle_t *fxas_handle, fxas_config_t *configure);

/*!
 * @brief Read data from sensors, assumes hyb_autoinc_mode is set in
 * M_FXAS_CTRL_REG2
 *
 * @param fxas_handle The pointer to accel driver handle.
 * @param sensorData The pointer to the buffer to hold sensor data
 *
 * @return kStatus_Success if success or kStatus_Fail if error.
 */
status_t FXAS_ReadSensorData(fxas_handle_t *fxas_handle, fxas_data_t *sensorData);

/*!
 * @brief Write value to register of sensor.
 *
 * @param handle The pointer to fxas21002cq driver handle.
 * @param reg Register address.
 * @param val Data want to write.
 *
 * @return kStatus_Success if success or kStatus_Fail if error.
 */
status_t FXAS_WriteReg(fxas_handle_t *handle, uint8_t reg, uint8_t val);

/*!
 * @brief Read n bytes start at register from sensor.
 *
 * @param handle The pointer to fxas21002cq driver handle.
 * @param reg Register address.
 * @param val The pointer to address which store data.
 * @param bytesNumber Number of bytes receiver.
 *
 * @return kStatus_Success if success or kStatus_Fail if error.
 */
status_t FXAS_ReadReg(fxas_handle_t *handle, uint8_t reg, uint8_t *val, uint8_t bytesNumber);

/*!
 * @brief Format to float.
 *
 * @param input The input integrate data.
 * @param fsrdps The FSR mode.
 *
 * @return The formated float.
 */
float FXAS_FormatFloat(int16_t input, fxas_gfsr_t fsrdps);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _FSL_FXAS_H_ */
