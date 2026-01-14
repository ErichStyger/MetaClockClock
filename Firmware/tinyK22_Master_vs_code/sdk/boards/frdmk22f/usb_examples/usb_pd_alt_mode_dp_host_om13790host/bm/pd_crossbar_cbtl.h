/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PD_CROSSBAR_CBTL_H__
#define __PD_CROSSBAR_CBTL_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef struct _pd_cbtl_crossbar_config
{
    uint8_t orientControlPort;
    uint8_t orientControlPin;
    uint8_t xsdnControlPort;
    uint8_t xsdnControlPin;
    uint8_t dp4laneControlPort;
    uint8_t dp4laneControlPin;
} pd_cbtl_crossbar_config_t;

typedef struct _pd_cbtl_crossbar_instance
{
    pd_handle pdHandle;
    GPIO_HANDLE_DEFINE(gpioDp4LaneHandle);
    GPIO_HANDLE_DEFINE(gpioXsdHandle);
    GPIO_HANDLE_DEFINE(gpioOrientHandle);
    const pd_cbtl_crossbar_config_t *config;
} pd_cbtl_crossbar_instance_t;

/*******************************************************************************
 * API
 ******************************************************************************/

/* 0 - intialize success; other values - fail */
uint8_t PD_CrossbarCBTLInit(pd_cbtl_crossbar_instance_t *cbtlCrossInstance, pd_handle pdHandle, void *config);
/* 0 - de-intialize success; other values - fail */
uint8_t PD_CrossbarCBTLDeinit(pd_cbtl_crossbar_instance_t *cbtlCrossInstance);

void PD_CrossbarCBTLSetMux(pd_cbtl_crossbar_instance_t *cbtlCrossInstance,
                           uint8_t orient,
                           uint8_t mux,
                           uint32_t modeVDO);
#endif
