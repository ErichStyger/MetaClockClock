/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PD_DP_HPD_DRIVER_H__
#define __PD_DP_HPD_DRIVER_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define HPD_DRIVE_MIN_HIGH_TICKS                  (2U)   /* ms */
#define HPD_DRIVE_MIN_LOW_TICKS                   (3U)   /* ms */
#define HPD_DRIVE_MIN_TICKS_BEFORE_DRIVER_RELEASE (1U)   /* ms, the value is 600us */
#define HPD_DRIVE_IRQ_LOW_USEC                    (700U) /* 700 us */

#define HPD_DRIVE_QUEUE_SIZE (5U)

typedef struct _pd_hpd_driver_config
{
    uint8_t hpdControlPort;
    uint8_t hpdControlPin;
} pd_hpd_driver_config_t;

typedef struct _pd_hpd_driver
{
    void *altModehandle;
    void *dpHandle;
    GPIO_HANDLE_DEFINE(gpioHpdHandle);
    volatile uint32_t hpdTime;
    const pd_hpd_driver_config_t *hpdConfig;
    volatile uint8_t hpdHeldLow;
    volatile uint8_t hpdOperating;
    volatile uint8_t hpdDriverQueue[HPD_DRIVE_QUEUE_SIZE];
    volatile uint8_t hpdDriverQueueCount;
} pd_hpd_driver_t;

/*******************************************************************************
 * API
 ******************************************************************************/

void PD_DpHpdDriver1msISR(pd_hpd_driver_t *hpdDriver);
/* 0 - success; other values - fail */
uint8_t PD_DpHpdDriverInit(pd_hpd_driver_t *hpdDriver, void *pdHandle, void *hpdConfig);
uint8_t PD_DpHpdDriverDeinit(pd_hpd_driver_t *hpdDriver);
void PD_DpHpdDriverSetLow(pd_hpd_driver_t *hpdDriver);
void PD_DpHpdDriverReleaseLow(pd_hpd_driver_t *hpdDriver);
void PD_DpHpdDriverControl(pd_hpd_driver_t *hpdDriver, uint8_t driveVal);
void PD_DpHpdDrvierProcess(pd_hpd_driver_t *hpdDriver);

#endif
