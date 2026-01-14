/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_max.h"
#include "fsl_timer.h"
/*******************************************************************************
 * Variables
 ******************************************************************************/

/******************************************************************************
 * Code
 ******************************************************************************/

static status_t MAX_Reset(max_handle_t *max_handle)
{
    uint8_t tmp = 0;
    status_t result;

    /* Reset */
    result = MAX_WriteReg(max_handle, MODE_CFG_REG, MODE_CFG_RST_MASK);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Poll the RESET bit until it's cleared by hardware.*/
    while (1)
    {
        result = MAX_ReadReg(max_handle, MODE_CFG_REG, &tmp, 1);
        if (result != kStatus_Success)
        {
            return result;
        }

        if (!(tmp & MODE_CFG_RST_MASK))
        {
            break;
        }
    }

    return kStatus_Success;
}

status_t MAX_Init(max_handle_t *max_handle, max_config_t *configure)
{
    assert(max_handle);
    assert(configure);

    uint8_t tmp = 0;
    status_t result;

    /* Initialize the I2C access function. */
    max_handle->I2C_SendFunc    = configure->I2C_SendFunc;
    max_handle->I2C_ReceiveFunc = configure->I2C_ReceiveFunc;

    result = MAX_Reset(max_handle);
    if (result != kStatus_Success)
    {
        return result;
    }

    /*  Time delay */
    Timer_Delay_ms(50);

    if (MAX_ReadReg(max_handle, ID_PART_REG, &tmp, 1) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    if (tmp != kMAX_WHO_AM_I_Device_ID)
    {
        return kStatus_Fail;
    }

    /* Set LED Reg */
    if (MAX_WriteReg(max_handle, LED_RED_PA_REG, 0xff) != kStatus_Success)
    {
        return kStatus_Fail;
    }
    if (MAX_WriteReg(max_handle, LED_IR_PA_REG, 0x33) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    if (MAX_WriteReg(max_handle, LED_GREEN_PA_REG, 0xff) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    if (MAX_WriteReg(max_handle, LED_PROXY_PA_REG, 0x19) != kStatus_Success)
    {
        return kStatus_Fail;
    }
    /* Multi mode configure */
    if (MAX_WriteReg(max_handle, MULTI_MODE_REG1, MULTI_MODE_GREEN_ACTIVE) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    if (MAX_WriteReg(max_handle, MULTI_MODE_REG2, MULTI_MODE_NONE_ACTIVE) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    /* FIFO */
    if (MAX_WriteReg(max_handle, FIFO_CFG_REG, 0x06) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    tmp = (configure->adcRange << SPO2_ADC_RAGE_SHIFT) | (configure->samples << SPO2_SR_SHIFT) | configure->pulsewidth;
    if (MAX_WriteReg(max_handle, SPO2_CFG_REG, tmp) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    if (MAX_WriteReg(max_handle, PROXY_INT_THR_REG, 0x14) != kStatus_Success)
    {
        return kStatus_Fail;
    }
    if (MAX_WriteReg(max_handle, FIFO_WRPTR_REG, 0) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    if (MAX_WriteReg(max_handle, FIFO_OVPTR_REG, 0) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    if (MAX_WriteReg(max_handle, FIFO_RDPTR_REG, 0) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    /* Enable */
    if (MAX_WriteReg(max_handle, INT_ENABLE_REG1, (MAXIM_EN_IRQ_PROX_INT_MASK | (1 << 6))) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    if (MAX_WriteReg(max_handle, MODE_CFG_REG, configure->mode) != kStatus_Success)
    {
        return kStatus_Fail;
    }

    return kStatus_Success;
}

status_t MAX_ReadSensorData(max_handle_t *max_handle, uint8_t *sampleData, uint8_t *sampleNum)
{
    status_t status   = kStatus_Success;
    uint8_t fifoWrPtr = 0;
    uint8_t fifoRdPtr = 0;
    uint8_t fifoOvPtr = 0;
    /* Temporary buffer to read samples from the FIFO buffer.*/
    uint8_t tmpBuf[MAXIM_FIFO_DEPTH * MAXIM_BYTES_PER_ADC_VALUE];
    uint8_t numAvailSam;

    /* Read FIFO pointers. */
    if (MAX_ReadReg(max_handle, FIFO_WRPTR_REG, &fifoWrPtr, 1) != kStatus_Success)
    {
        status = kStatus_Fail;
    }

    if (MAX_ReadReg(max_handle, FIFO_OVPTR_REG, &fifoOvPtr, 1) != kStatus_Success)
    {
        status = kStatus_Fail;
    }

    if (MAX_ReadReg(max_handle, FIFO_RDPTR_REG, &fifoRdPtr, 1) != kStatus_Success)
    {
        status = kStatus_Fail;
    }

    if (fifoOvPtr > 0)
    {
        MAX_ReadReg(max_handle, FIFO_DATA_REG, &tmpBuf[0], MAXIM_FIFO_DEPTH * MAXIM_BYTES_PER_ADC_VALUE);

        numAvailSam = 0xFF;
    }
    else
    {
        if (fifoWrPtr > fifoRdPtr)
        {
            numAvailSam = fifoWrPtr - fifoRdPtr;
        }
        else if (fifoWrPtr < fifoRdPtr)
        {
            numAvailSam = fifoWrPtr + MAXIM_FIFO_DEPTH - fifoRdPtr;
        }
        else
        {
            numAvailSam = 0;
        }

        if (numAvailSam > 0)
        {
            MAX_ReadReg(max_handle, FIFO_DATA_REG, &tmpBuf[0], numAvailSam * MAXIM_BYTES_PER_ADC_VALUE);

            if (sampleData != NULL)
            {
                memcpy(sampleData, tmpBuf, numAvailSam * MAXIM_BYTES_PER_ADC_VALUE);
            }
        }
    }

    if (sampleNum != NULL)
    {
        *sampleNum = numAvailSam;
    }
    return status;
}

status_t MAX_ReadReg(max_handle_t *handle, uint8_t reg, uint8_t *val, uint16_t bytesNumber)
{
    assert(handle);
    assert(val);

    if (!handle->I2C_ReceiveFunc)
    {
        return kStatus_Fail;
    }

    return handle->I2C_ReceiveFunc(MAX_I2C_ADDRESS, reg, 1, val, bytesNumber);
}

status_t MAX_WriteReg(max_handle_t *handle, uint8_t reg, uint8_t val)
{
    assert(handle);

    if (!handle->I2C_SendFunc)
    {
        return kStatus_Fail;
    }

    return handle->I2C_SendFunc(MAX_I2C_ADDRESS, reg, 1, val);
}
