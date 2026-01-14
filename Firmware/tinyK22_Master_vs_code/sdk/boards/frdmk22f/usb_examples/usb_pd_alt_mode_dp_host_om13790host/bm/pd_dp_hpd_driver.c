/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_i2c.h"
#include "fsl_adapter_gpio.h"
#include "pd_dp_hpd_driver.h"
#include "pd_app_misc.h"
#include "usb_pd_alt_mode_dp.h"
#include "pd_dp_board_chip.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void PD_WaitUs(uint32_t us);
void PD_DpHpdDrvierProcess(pd_hpd_driver_t *hpdDriver);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void PD_DpHpdDriver1msISR(pd_hpd_driver_t *hpdDriver)
{
    if (hpdDriver->hpdTime > 0)
    {
        hpdDriver->hpdTime--;
        if (hpdDriver->hpdTime == 0)
        {
            hpdDriver->hpdOperating = kDPHPDDriver_None;
        }
    }
}

/* 0 - success; other values - fail */
uint8_t PD_DpHpdDriverInit(pd_hpd_driver_t *hpdDriver, void *pdHandle, void *hpdConfig)
{
    hal_gpio_pin_config_t pinConfig;

    hpdDriver->hpdHeldLow          = 0;
    hpdDriver->hpdDriverQueueCount = 0;
    hpdDriver->hpdOperating        = kDPHPDDriver_None;
    hpdDriver->hpdConfig           = (const pd_hpd_driver_config_t *)hpdConfig;

    pinConfig.direction = kHAL_GpioDirectionOut;
    pinConfig.port      = hpdDriver->hpdConfig->hpdControlPort;
    pinConfig.pin       = hpdDriver->hpdConfig->hpdControlPin;
    pinConfig.level     = 0;
    HAL_GpioInit((hal_gpio_handle_t *)(&hpdDriver->gpioHpdHandle[0]), &pinConfig);

    PD_DpHpdDriverSetLow(hpdDriver);
    return 0;
}

uint8_t PD_DpHpdDriverDeinit(pd_hpd_driver_t *hpdDriver)
{
    return 0;
}

void PD_DpHpdDriverSetLow(pd_hpd_driver_t *hpdDriver)
{
    APP_CRITICAL_ALLOC();

    APP_ENTER_CRITICAL();
    if (!(hpdDriver->hpdHeldLow))
    {
        hpdDriver->hpdDriverQueueCount = 0;
        hpdDriver->hpdHeldLow          = 1;
        hpdDriver->hpdOperating        = kDPHPDDriver_Waiting;
        APP_EXIT_CRITICAL();
        hpdDriver->hpdTime = HPD_DRIVE_MIN_LOW_TICKS;

        HAL_GpioSetOutput((hal_gpio_handle_t)(&hpdDriver->gpioHpdHandle[0]), 0);
    }
    else
    {
        APP_EXIT_CRITICAL();
    }
}

void PD_DpHpdDriverReleaseLow(pd_hpd_driver_t *hpdDriver)
{
    /* keep least low for HPD_DRIVE_MIN_TICKS_BEFORE_DRIVER_RELEASE from this call,
     * And keep low least HPD_DRIVE_MIN_LOW_TICKS for LOW from drive LOW. */
    APP_CRITICAL_ALLOC();

    APP_ENTER_CRITICAL();
    if (hpdDriver->hpdHeldLow)
    {
        hpdDriver->hpdHeldLow = 0;
        APP_EXIT_CRITICAL();
        if (hpdDriver->hpdOperating == kDPHPDDriver_Waiting)
        {
            if (hpdDriver->hpdTime <= HPD_DRIVE_MIN_TICKS_BEFORE_DRIVER_RELEASE)
            {
                hpdDriver->hpdTime = HPD_DRIVE_MIN_TICKS_BEFORE_DRIVER_RELEASE;
            }
        }
        else
        {
            hpdDriver->hpdOperating = kDPHPDDriver_Waiting;
            hpdDriver->hpdTime      = HPD_DRIVE_MIN_TICKS_BEFORE_DRIVER_RELEASE;
        }
    }
    else
    {
        APP_EXIT_CRITICAL();
    }
}

static void PD_DpHpdDriverOps(pd_hpd_driver_t *hpdDriver, uint8_t driveVal)
{
    if (driveVal == kDPHPDDriver_IRQ)
    {
        APP_CRITICAL_ALLOC();

        HAL_GpioSetOutput((hal_gpio_handle_t)(&hpdDriver->gpioHpdHandle[0]), 0);
        /* wait HPD_DRIVE_IRQ_LOW_USEC, make sure the time is accurate */
        APP_ENTER_CRITICAL();
        PD_WaitUs(HPD_DRIVE_IRQ_LOW_USEC);
        APP_EXIT_CRITICAL();
        HAL_GpioSetOutput((hal_gpio_handle_t)(&hpdDriver->gpioHpdHandle[0]), 1);
        hpdDriver->hpdTime = HPD_DRIVE_MIN_HIGH_TICKS;
    }
    else if (driveVal == kDPHPDDriver_High)
    {
        HAL_GpioSetOutput((hal_gpio_handle_t)(&hpdDriver->gpioHpdHandle[0]), 1);
        hpdDriver->hpdTime = HPD_DRIVE_MIN_HIGH_TICKS;
    }
    else if (driveVal == kDPHPDDriver_Low)
    {
        HAL_GpioSetOutput((hal_gpio_handle_t)(&hpdDriver->gpioHpdHandle[0]), 0);
        hpdDriver->hpdTime = HPD_DRIVE_MIN_LOW_TICKS;
    }
    else
    {
    }
}

void PD_DpHpdDriverControl(pd_hpd_driver_t *hpdDriver, uint8_t driveVal)
{
    APP_CRITICAL_ALLOC();

    APP_ENTER_CRITICAL();
    if ((hpdDriver->hpdOperating != kDPHPDDriver_None) || (hpdDriver->hpdHeldLow))
    {
        if (driveVal == kDPHPDDriver_Low)
        {
            hpdDriver->hpdDriverQueueCount = 0;
            if (!(hpdDriver->hpdHeldLow))
            {
                hpdDriver->hpdDriverQueue[hpdDriver->hpdDriverQueueCount++] = driveVal;
            }
        }
        else
        {
            if (hpdDriver->hpdDriverQueueCount < HPD_DRIVE_QUEUE_SIZE)
            {
                hpdDriver->hpdDriverQueue[hpdDriver->hpdDriverQueueCount++] = driveVal;
            }
        }
        APP_EXIT_CRITICAL();
    }
    else
    {
        hpdDriver->hpdOperating = driveVal;
        hpdDriver->hpdOperating = kDPHPDDriver_Waiting;
        APP_EXIT_CRITICAL();
        PD_DpHpdDriverOps(hpdDriver, driveVal);
    }
}

void PD_DpHpdDrvierProcess(pd_hpd_driver_t *hpdDriver)
{
    uint8_t index = 0;
    uint8_t drive;

    APP_CRITICAL_ALLOC();

    APP_ENTER_CRITICAL();
    if ((hpdDriver->hpdHeldLow) || (hpdDriver->hpdOperating != kDPHPDDriver_None))
    {
        APP_EXIT_CRITICAL();
        return;
    }

    if (hpdDriver->hpdDriverQueueCount > 0)
    {
        drive = hpdDriver->hpdDriverQueue[0];
        hpdDriver->hpdDriverQueueCount--;
        for (index = 0; index < hpdDriver->hpdDriverQueueCount; ++index)
        {
            hpdDriver->hpdDriverQueue[index] = hpdDriver->hpdDriverQueue[index + 1];
        }
        hpdDriver->hpdOperating = drive;
        hpdDriver->hpdOperating = kDPHPDDriver_Waiting;
        APP_EXIT_CRITICAL();
        PD_DpHpdDriverOps(hpdDriver, drive);
    }
    else
    {
        APP_EXIT_CRITICAL();
    }
}
