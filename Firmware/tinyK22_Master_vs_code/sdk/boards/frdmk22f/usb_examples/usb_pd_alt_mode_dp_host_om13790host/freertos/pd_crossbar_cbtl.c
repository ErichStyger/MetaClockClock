/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "usb_pd_config.h"
#include "pd_board_config.h"
#include "usb_pd.h"
#include "usb_pd_i2c.h"
#include "usb_pd_alt_mode.h"
#include "usb_pd_alt_mode_dp.h"
#include "fsl_adapter_gpio.h"
#include "pd_crossbar_cbtl.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void PD_CrossbarCBTLSetMux(pd_cbtl_crossbar_instance_t *cbtlCrossInstance,
                           uint8_t orient,
                           uint8_t mux,
                           uint32_t modeVDO);
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/* 0 - intialize success; other values - fail */
uint8_t PD_CrossbarCBTLInit(pd_cbtl_crossbar_instance_t *cbtlCrossInstance, pd_handle pdHandle, void *config)
{
    /* initialize crossbar enable pin */
    cbtlCrossInstance->pdHandle = pdHandle;
    cbtlCrossInstance->config   = (const pd_cbtl_crossbar_config_t *)config;
    hal_gpio_pin_config_t pinConfig;

    if ((cbtlCrossInstance->config->orientControlPort != 0xFFu) &&
        (cbtlCrossInstance->config->orientControlPin != 0xFFu))
    {
        pinConfig.direction = kHAL_GpioDirectionOut;
        pinConfig.port      = cbtlCrossInstance->config->orientControlPort;
        pinConfig.pin       = cbtlCrossInstance->config->orientControlPin;
        pinConfig.level     = 0;
        HAL_GpioInit((hal_gpio_handle_t *)(&cbtlCrossInstance->gpioOrientHandle[0]), &pinConfig);
    }

    if ((cbtlCrossInstance->config->xsdnControlPort != 0xFFu) && (cbtlCrossInstance->config->xsdnControlPin != 0xFFu))
    {
        pinConfig.direction = kHAL_GpioDirectionOut;
        pinConfig.port      = cbtlCrossInstance->config->xsdnControlPort;
        pinConfig.pin       = cbtlCrossInstance->config->xsdnControlPin;
        pinConfig.level     = 0;
        HAL_GpioInit((hal_gpio_handle_t *)(&cbtlCrossInstance->gpioXsdHandle[0]), &pinConfig);
    }

    if ((cbtlCrossInstance->config->dp4laneControlPort != 0xFFu) &&
        (cbtlCrossInstance->config->dp4laneControlPin != 0xFFu))
    {
        pinConfig.direction = kHAL_GpioDirectionOut;
        pinConfig.port      = cbtlCrossInstance->config->dp4laneControlPort;
        pinConfig.pin       = cbtlCrossInstance->config->dp4laneControlPin;
        pinConfig.level     = 0;
        HAL_GpioInit((hal_gpio_handle_t *)(&cbtlCrossInstance->gpioDp4LaneHandle[0]), &pinConfig);
    }

    PD_CrossbarCBTLSetMux(cbtlCrossInstance, 0, kDPPeripheal_ControlSetMuxDisable, 0);
    return 0;
}

uint8_t PD_CrossbarCBTLDeinit(pd_cbtl_crossbar_instance_t *cbtlCrossInstance)
{
    return 0;
}

void PD_CrossbarCBTLSetMux(pd_cbtl_crossbar_instance_t *cbtlCrossInstance,
                           uint8_t orient,
                           uint8_t mux,
                           uint32_t modeVDO)
{
    switch (mux)
    {
        case kDPPeripheal_ControlSetMuxUSB3Only:
            HAL_GpioSetOutput((hal_gpio_handle_t)(&cbtlCrossInstance->gpioOrientHandle[0]), (orient ? 1 : 0));
            HAL_GpioSetOutput((hal_gpio_handle_t)(&cbtlCrossInstance->gpioDp4LaneHandle[0]), 0);
            HAL_GpioSetOutput((hal_gpio_handle_t)(&cbtlCrossInstance->gpioXsdHandle[0]), 0);
            break;

        case kDPPeripheal_ControlSetMuxShutDown:
        case kDPPeripheal_ControlSetMuxDisable:
        case kDPPeripheal_ControlSetMuxSaftMode:
            HAL_GpioSetOutput((hal_gpio_handle_t)(&cbtlCrossInstance->gpioXsdHandle[0]), 1);
            HAL_GpioSetOutput((hal_gpio_handle_t)(&cbtlCrossInstance->gpioDp4LaneHandle[0]), 0);
            break;

        case kDPPeripheal_ControlSetMuxDP2LANEUSB3:
        case kDPPeripheal_ControlSetMuxDP2LANENOUSB:
            HAL_GpioSetOutput((hal_gpio_handle_t)(&cbtlCrossInstance->gpioOrientHandle[0]), (orient ? 1 : 0));
            HAL_GpioSetOutput((hal_gpio_handle_t)(&cbtlCrossInstance->gpioDp4LaneHandle[0]), 0);
            HAL_GpioSetOutput((hal_gpio_handle_t)(&cbtlCrossInstance->gpioXsdHandle[0]), 0);
            break;

        case kDPPeripheal_ControlSetMuxDP4LANE:
            HAL_GpioSetOutput((hal_gpio_handle_t)(&cbtlCrossInstance->gpioOrientHandle[0]), (orient ? 1 : 0));
            HAL_GpioSetOutput((hal_gpio_handle_t)(&cbtlCrossInstance->gpioDp4LaneHandle[0]), 1);
            HAL_GpioSetOutput((hal_gpio_handle_t)(&cbtlCrossInstance->gpioXsdHandle[0]), 0);
            break;

        default:
            break;
    }
}
