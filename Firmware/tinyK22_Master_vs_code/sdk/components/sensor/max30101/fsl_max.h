/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FSL_MAX_H_
#define _FSL_MAX_H_

#include "fsl_common.h"

#define MAX_I2C_ADDRESS 0x57 /* I2C slave device address. */

#define MAXIM_BYTES_PER_ADC_VALUE (3)
#define MAXIM_FIFO_DEPTH          (32)
#define MAXIM_NEW_SAMPLE_CHUNK    (256)
#define MAXIM_INIT_SAMPLE_SIZE    (MAXIM_NEW_SAMPLE_CHUNK)
#define MAXIM_FINAL_SAMPLE_SIZE   (MAXIM_NEW_SAMPLE_CHUNK * 8)
#define MAXIM_HR_FREQ_MIN         (0.75)
#define MAXIM_HR_FREQ_MAX         (3)

/*
 *  STATUS Register
 */
#define INT_STATUS_REG1     (0x00)
#define INT_STATUS_REG2     (0x01)
#define INT_ENABLE_REG1     (0x02)
#define INT_ENABLE_REG2     (0x03)
#define INT_FIFO_READY_MASK (1 << 6)

/**
 * interrupt enable
 */

#define MAXIM_EN_IRQ_FIFO_ALMOST_FULL_MASK (1 << 7)
#define MAXIM_EN_IRQ_NEW_SAMPLE_RDY_MASK   (1 << 6)
#define MAXIM_EN_IRQ_AMBIENT_OVF_MASK      (1 << 5)
#define MAXIM_EN_IRQ_PROX_INT_MASK         (1 << 4)
#define MAXIM_EN_IRQ_INT_TEMP_RDY_MASK     (1 << 1)
#define MAXIM_PEAK_THRESHOLD               (1000)
#define MAXIM_FIFO_OVERFLOW                (-1)

#define MAXIM_PROXY_THR (1000)

/* FIFO register */
#define FIFO_WRPTR_REG (0x04)
#define FIFO_OVPTR_REG (0x05)
#define FIFO_RDPTR_REG (0x06)
#define FIFO_DATA_REG  (0x07)

#define FIFO_CFG_REG          (0x8)
#define FIFO_ROLLOVER_EN_MASK (1 << 4)
#define FIFO_ALMOST_FULL_MASK (0xF)

/* Mode configure */
#define MODE_CFG_REG        (0x9)
#define MODE_CFG_RST_MASK   (1 << 6)
#define MODE_MULTI_LED_MODE (0x07)

#define SPO2_CFG_REG        (0xA)
#define SPO2_SR_SHIFT       (2)
#define SPO2_ADC_RAGE_SHIFT (5)

#define LED_RED_PA_REG   (0xC)
#define LED_IR_PA_REG    (0xD)
#define LED_GREEN_PA_REG (0xE)
#define LED_PROXY_PA_REG (0x10)

#define LED_CURRENT_50   (0xFF)
#define LED_CURRENT_12_5 (0x3F)
#define LED_PROXY_6_4    (0x1F)

#define MULTI_MODE_REG1         (0x11)
#define MULTI_MODE_REG2         (0x12)
#define MULTI_MODE_GREEN_ACTIVE (0x03)
#define MULTI_MODE_NONE_ACTIVE  (0x00)

#define TEMP_INT_REG    (0x1F)
#define TEMP_FRAC_REG   (0x20)
#define TEMP_CONFIG_REG (0x21)

#define PROXY_INT_THR_REG (0x30)

#define ID_REV_REG              (0xFE)
#define ID_PART_REG             (0xFF)
#define kMAX_WHO_AM_I_Device_ID 0x15

typedef enum _max_mode
{
    kMAX_HrMode       = 0x2,
    kMAX_Spo2Mode     = 0x3,
    kMAX_MultiLedMode = 0x7
} max_mode_t;

typedef enum _max_ledpw
{
    kMAX_PW_69US_15Bits  = 0x00,
    kMAX_PW_118US_16Bits = 0x01,
    kMAX_PW_215US_17Bits = 0x02,
    kMAX_PW_411US_18Bits = 0x03
} max_ledpw_t;

typedef enum _max_adc_range
{
    kMAX_AdcRge_00 = 0x00,
    kMAX_AdcRge_01,
    kMAX_AdcRge_02,
    kMAX_Adcge_03
} max_adc_range_t;

typedef enum _max_samplerate
{
    kMAX_SR_50Hz = 0,
    kMAX_SR_100Hz,
    kMAX_SR_200Hz,
    kMAX_SR_400Hz,
    kMAX_SR_800Hz,
    kMAX_SR_1000Hz,
    kMAX_SR_1600Hz,
    kMAX_SR_3200Hz
} max_samplerate_t;

/*! @brief Define I2C access function. */
typedef status_t (*I2C_SendFunc_t)(uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint32_t txBuff);
typedef status_t (*I2C_ReceiveFunc_t)(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subaddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);

/*! @brief fxas21002cq configure definition. This structure should be global.*/
typedef struct _max_handle
{
    /* Pointer to the user-defined I2C Send Data function. */
    I2C_SendFunc_t I2C_SendFunc;
    /* Pointer to the user-defined I2C Receive Data function. */
    I2C_ReceiveFunc_t I2C_ReceiveFunc;
} max_handle_t;

typedef struct _max_config_t
{
    /* Pointer to the user-defined I2C Send Data function. */
    I2C_SendFunc_t I2C_SendFunc;
    /* Pointer to the user-defined I2C Receive Data function. */
    I2C_ReceiveFunc_t I2C_ReceiveFunc;
    max_mode_t mode;
    max_ledpw_t pulsewidth;
    max_samplerate_t samples;
    max_adc_range_t adcRange;
} max_config_t;
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif
/*!
 * @name MAX30101 Driver
 * @{
 */

/*!
 * @brief Verify and initialize max_handleice: Hybrid mode with ODR=50Hz, Mag OSR=32, Acc OSR=Normal.
 *
 * @param max_handle The pointer to accel driver handle.
 *
 * @return kStatus_Success if success or kStatus_Fail if error.
 */
status_t MAX_Init(max_handle_t *max_handle, max_config_t *configure);

/*!
 * @brief Read data from sensors, assumes hyb_autoinc_mode is set in M_CTRL_REG2
 *
 * @param max_handle The pointer to accel driver handle.
 * @param sensorData The pointer to the buffer to hold sensor data
 * @param sampleNum The number of read samples.
 * @return kStatus_Success if success or kStatus_Fail if error.
 */
status_t MAX_ReadSensorData(max_handle_t *max_handle, uint8_t *sampleData, uint8_t *sampleNum);

/*!
 * @brief Write value to register of sensor.
 *
 * @param handle The pointer to fxas21002cq driver handle.
 * @param reg Register address.
 * @param val Data want to write.
 *
 * @return kStatus_Success if success or kStatus_Fail if error.
 */
status_t MAX_WriteReg(max_handle_t *handle, uint8_t reg, uint8_t val);

/*!
 * @brief Read n bytes start at register from sensor.
 *
 * @param handle The pointer to fmax30101 driver handle.
 * @param reg Register address.
 * @param val The pointer to address which store data.
 * @param bytesNumber Number of bytes receiver.
 *
 * @return kStatus_Success if success or kStatus_Fail if error.
 */
status_t MAX_ReadReg(max_handle_t *handle, uint8_t reg, uint8_t *val, uint16_t bytesNumber);
/*! @} */

#if defined(__cplusplus)
}
#endif

/*! @}*/

#endif /* _FSL_MAX_H_ */
