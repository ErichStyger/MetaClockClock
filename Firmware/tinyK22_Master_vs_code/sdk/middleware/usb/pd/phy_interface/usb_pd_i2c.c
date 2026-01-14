/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_adapter_i2c.h"
#include "usb_pd.h"
#include "usb_pd_i2c.h"
#include "fsl_os_abstraction.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef struct _i2c_driver_adpter
{
    HAL_I2C_MASTER_HANDLE_DEFINE(i2cMasterHandleBuffer);
    hal_i2c_master_config_t i2cMasterConfig;
    OSA_MUTEX_HANDLE_DEFINE(i2cMutex);
    PD_I2cReleaseBus i2cReleaseBus;
    uint8_t refCount;
} i2c_driver_adapter_t;

/*******************************************************************************
 * Variables
 ******************************************************************************/

static i2c_driver_adapter_t s_I2CDriverInstance[USB_PD_I2C_INSTANCE_COUNT];

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#if defined(SDK_OS_FREE_RTOS)
static uint8_t PD_CheckInterruptContext(void);
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

pd_status_t PD_I2cInit(usb_pd_i2c_handle *i2cHandle,
                       uint8_t i2cInstance,
                       uint32_t i2cSrcFreq,
                       PD_I2cReleaseBus i2cReleaseBus)
{
    uint8_t index;
    i2c_driver_adapter_t *i2cAdapter = NULL;
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    for (index = 0; index < USB_PD_I2C_INSTANCE_COUNT; index++)
    {
        if ((s_I2CDriverInstance[index].refCount != 0U) &&
            (s_I2CDriverInstance[index].i2cMasterConfig.instance == i2cInstance))
        {
            s_I2CDriverInstance[index].refCount++;
            *i2cHandle = &s_I2CDriverInstance[index];
            OSA_EXIT_CRITICAL();
            return kStatus_PD_Success;
        }
    }

    for (index = 0; index < USB_PD_I2C_INSTANCE_COUNT; index++)
    {
        if (s_I2CDriverInstance[index].refCount == 0U)
        {
            (void)memset(&s_I2CDriverInstance[index], 0, sizeof(i2c_driver_adapter_t));
            s_I2CDriverInstance[index].refCount = 1;
            i2cAdapter                          = &s_I2CDriverInstance[index];
            break;
        }
    }
    OSA_EXIT_CRITICAL();

    if (i2cAdapter == NULL)
    {
        return kStatus_PD_Error;
    }

    i2cAdapter->i2cReleaseBus                = i2cReleaseBus;
    i2cAdapter->i2cMasterConfig.srcClock_Hz  = i2cSrcFreq;
    i2cAdapter->i2cMasterConfig.baudRate_Bps = 400000UL;
    i2cAdapter->i2cMasterConfig.enableMaster = true;
    i2cAdapter->i2cMasterConfig.instance     = i2cInstance;
    if (HAL_I2cMasterInit(&i2cAdapter->i2cMasterHandleBuffer[0], &i2cAdapter->i2cMasterConfig) !=
        kStatus_HAL_I2cSuccess)
    {
        return kStatus_PD_Error;
    }

    if (OSA_MutexCreate(&i2cAdapter->i2cMutex[0]) != KOSA_StatusSuccess)
    {
        return kStatus_PD_Error;
    }

    *i2cHandle = i2cAdapter;

    return kStatus_PD_Success;
}

pd_status_t PD_I2cDeinit(usb_pd_i2c_handle i2cHandle)
{
    i2c_driver_adapter_t *i2cAdapter = (i2c_driver_adapter_t *)i2cHandle;
    OSA_SR_ALLOC();

    if (i2cHandle == NULL)
    {
        return kStatus_PD_Error;
    }

    OSA_ENTER_CRITICAL();
    if (i2cAdapter->refCount > 0U)
    {
        i2cAdapter->refCount--;
        if (i2cAdapter->refCount > 0U)
        {
            OSA_EXIT_CRITICAL();
            return kStatus_PD_Success;
        }
    }
    OSA_EXIT_CRITICAL();

    if (HAL_I2cMasterDeinit(&i2cAdapter->i2cMasterHandleBuffer[0]) != kStatus_HAL_I2cSuccess)
    {
        return kStatus_PD_Error;
    }

    (void)OSA_MutexDestroy(&i2cAdapter->i2cMutex[0]);

    return kStatus_PD_Success;
}

static pd_status_t PD_I2cTansferBlocking(usb_pd_i2c_handle i2cHandle,
                                         uint32_t slave,
                                         uint32_t registerAddr,
                                         uint8_t registerLen,
                                         uint8_t *data,
                                         uint32_t num,
                                         hal_i2c_direction_t direction)
{
    i2c_driver_adapter_t *i2cAdapter = (i2c_driver_adapter_t *)i2cHandle;
    hal_i2c_master_transfer_t xfer;
    hal_i2c_status_t i2cStatus;
    volatile uint8_t retryCount = I2C_TRANSFER_RETRY_COUNT;

    if (i2cHandle == NULL)
    {
        return kStatus_PD_Error;
    }

#if defined(SDK_OS_FREE_RTOS)
    if (PD_CheckInterruptContext() == 0U)
#endif
    {
        if (OSA_MutexLock(&i2cAdapter->i2cMutex[0], osaWaitForever_c) != KOSA_StatusSuccess)
        {
            return kStatus_PD_Error;
        }
    }

    xfer.data           = data;
    xfer.dataSize       = (size_t)num;
    xfer.flags          = (uint32_t)kHAL_I2cTransferDefaultFlag;
    xfer.subaddress     = registerAddr;
    xfer.subaddressSize = registerLen;
    xfer.slaveAddress   = (uint8_t)slave;
    xfer.direction      = direction;
    do
    {
        i2cStatus = HAL_I2cMasterTransferBlocking(&i2cAdapter->i2cMasterHandleBuffer[0], &xfer);
        if (i2cStatus == kStatus_HAL_I2cSuccess)
        {
            break;
        }
        else
        {
            (void)HAL_I2cMasterDeinit(&i2cAdapter->i2cMasterHandleBuffer[0]);
            i2cAdapter->i2cReleaseBus();
            (void)HAL_I2cMasterInit(&i2cAdapter->i2cMasterHandleBuffer[0], &i2cAdapter->i2cMasterConfig);
        }
    } while (--retryCount > 0U);

#if defined(SDK_OS_FREE_RTOS)
    if (PD_CheckInterruptContext() == 0U)
#endif
    {
        if (OSA_MutexUnlock(&i2cAdapter->i2cMutex[0]) != KOSA_StatusSuccess)
        {
            return kStatus_PD_Error;
        }
    }

    if (i2cStatus == kStatus_HAL_I2cSuccess)
    {
        return kStatus_PD_Success;
    }
    else
    {
        return kStatus_PD_Error;
    }
}

pd_status_t PD_I2cReadBlocking(usb_pd_i2c_handle i2cHandle,
                               uint32_t slave,
                               uint32_t registerAddr,
                               uint8_t registerLen,
                               uint8_t *data,
                               uint32_t num)
{
    return PD_I2cTansferBlocking(i2cHandle, slave, registerAddr, registerLen, data, num, kHAL_I2cRead);
}

pd_status_t PD_I2cWriteBlocking(usb_pd_i2c_handle i2cHandle,
                                uint32_t slave,
                                uint32_t registerAddr,
                                uint8_t registerLen,
                                uint8_t *data,
                                uint32_t num)
{
    return PD_I2cTansferBlocking(i2cHandle, slave, registerAddr, registerLen, data, num, kHAL_I2cWrite);
}

#if defined(SDK_OS_FREE_RTOS)
static uint8_t PD_CheckInterruptContext(void)
{
#if defined(__GIC_PRIO_BITS)
    if ((__get_CPSR() & CPSR_M_Msk) == 0x13)
#else
    if (__get_IPSR() != 0U)
#endif
    {
        return 1;
    }
    return 0;
}
#endif
