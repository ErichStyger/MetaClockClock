/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_i2c.h"
#include "fsl_adapter_gpio.h"
#include "pd_board_config.h"
#include "usb_pd_alt_mode.h"
#include "usb_pd_alt_mode_dp.h"
#include "pd_dp_hpd_driver.h"
#include "pd_crossbar_cbtl.h"
#include "pd_ptn36502.h"
#include "pd_app_misc.h"
#include "pd_dp_board_chip.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef struct _pd_board_chip_instance
{
    pd_cbtl_crossbar_instance_t crossbarInstance;
    pd_hpd_driver_t hpdDriverInstance;
    pd_ptn36502_instance_t ptn36502Instance;
    pd_handle pdHandle;
    uint8_t occupied;
} pd_board_chip_instance_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static pd_board_chip_instance_t s_PDBoardChipInstances[PD_CONFIG_MAX_PORT];
static EventGroupHandle_t s_PDBoardChipTaskEvent;

/*******************************************************************************
 * Code
 ******************************************************************************/

pd_status_t PD_DpBoardChipsInit(void **interfaceHandle, pd_handle pdHandle, const void *param)
{
    uint32_t index                           = 0;
    pd_board_chip_instance_t *pdChipInstance = NULL;
    APP_CRITICAL_ALLOC();

    if (s_PDBoardChipTaskEvent == NULL)
    {
        s_PDBoardChipTaskEvent = xEventGroupCreate();
        if (NULL == s_PDBoardChipTaskEvent)
        {
            return kStatus_PD_Error;
        }
    }

    APP_ENTER_CRITICAL();
    for (index = 0; index < sizeof(s_PDBoardChipInstances) / sizeof(pd_board_chip_instance_t); ++index)
    {
        if (s_PDBoardChipInstances[index].occupied == 0)
        {
            s_PDBoardChipInstances[index].occupied = 1;
            pdChipInstance                         = &s_PDBoardChipInstances[index];
            break;
        }
    }

    if (pdChipInstance == NULL)
    {
        APP_EXIT_CRITICAL();
        return kStatus_PD_Error;
    }
    APP_EXIT_CRITICAL();

    pdChipInstance->pdHandle = pdHandle;
    if (PD_CrossbarCBTLInit(&pdChipInstance->crossbarInstance, pdChipInstance->pdHandle,
                            ((const pd_dp_peripheral_config_t *)param)->crossbarConfig) != 0)
    {
        pdChipInstance->occupied = 0;
        return kStatus_PD_Error;
    }

    if (PD_DpHpdDriverInit(&pdChipInstance->hpdDriverInstance, pdChipInstance->pdHandle,
                           ((const pd_dp_peripheral_config_t *)param)->hpdDriverConfig) != 0)
    {
        PD_CrossbarCBTLDeinit(&pdChipInstance->crossbarInstance);
        pdChipInstance->occupied = 0;
        return kStatus_PD_Error;
    }

    if (PD_PTN36502Init(&pdChipInstance->ptn36502Instance, pdChipInstance->pdHandle,
                        ((const pd_dp_peripheral_config_t *)param)->ptn36502Config) != 0)
    {
        PD_DpHpdDriverDeinit(&pdChipInstance->hpdDriverInstance);
        PD_CrossbarCBTLDeinit(&pdChipInstance->crossbarInstance);
        pdChipInstance->occupied = 0;
        return kStatus_PD_Error;
    }

    *interfaceHandle = pdChipInstance;
    return kStatus_PD_Success;
}

pd_status_t PD_DpBoardChipsDeinit(void *interfaceHandle)
{
    pd_board_chip_instance_t *pdChipInstance = (pd_board_chip_instance_t *)interfaceHandle;
    if (interfaceHandle != NULL)
    {
        PD_DpHpdDriverDeinit(&pdChipInstance->hpdDriverInstance);
        PD_CrossbarCBTLDeinit(&pdChipInstance->crossbarInstance);
        PD_PTN36502Deinit(&pdChipInstance->ptn36502Instance);
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpBoardChipsControl(void *interfaceHandle, uint32_t opCode, void *opParam)
{
    uint8_t ccOrient;
    pd_board_chip_instance_t *pdChipInstance = (pd_board_chip_instance_t *)interfaceHandle;
    if (interfaceHandle == NULL)
    {
        return kStatus_PD_Error;
    }

    PD_Control(pdChipInstance->pdHandle, PD_CONTROL_GET_TYPEC_ORIENTATION, &ccOrient);
    switch (opCode)
    {
        case kDPPeripheal_ControlHPDValue:
            PD_DpHpdDriverControl(&pdChipInstance->hpdDriverInstance, *((uint8_t *)opParam));
            break;

        case kDPPeripheal_ControlHPDSetLow:
            PD_DpHpdDriverSetLow(&pdChipInstance->hpdDriverInstance);
            break;

        case kDPPeripheal_ControlHPDReleaseLow:
            PD_DpHpdDriverReleaseLow(&pdChipInstance->hpdDriverInstance);
            break;

        case kDPPeripheal_ControlSetMuxShutDown:
        case kDPPeripheal_ControlSetMuxDisable:
        case kDPPeripheal_ControlSetMuxSaftMode:
            PD_CrossbarCBTLSetMux(&pdChipInstance->crossbarInstance, ccOrient, opCode, *((uint32_t *)opParam));
            PD_PTN36502SetMode(&pdChipInstance->ptn36502Instance, kMode_DeepPowerSavingState);
            break;

        case kDPPeripheal_ControlSetMuxUSB3Only:
            PD_CrossbarCBTLSetMux(&pdChipInstance->crossbarInstance, ccOrient, opCode, *((uint32_t *)opParam));
            PD_PTN36502SetMode(&pdChipInstance->ptn36502Instance, kMode_USB31Gen1Only);
            break;

        case kDPPeripheal_ControlSetMuxDP2LANENOUSB:
        case kDPPeripheal_ControlSetMuxDP4LANE:
            PD_CrossbarCBTLSetMux(&pdChipInstance->crossbarInstance, ccOrient, opCode, *((uint32_t *)opParam));
            PD_PTN36502SetMode(&pdChipInstance->ptn36502Instance, kMode_DP4Lane);
            break;

        case kDPPeripheal_ControlSetMuxDP2LANEUSB3:
            PD_CrossbarCBTLSetMux(&pdChipInstance->crossbarInstance, ccOrient, opCode, *((uint32_t *)opParam));
            PD_PTN36502SetMode(&pdChipInstance->ptn36502Instance, kMode_USB31Gen1AndDP2Lane);
            break;

        default:
            break;
    }

    return kStatus_PD_Success;
}

void PD_DpBoardChip1msIsr(void)
{
    for (uint8_t index = 0; index < sizeof(s_PDBoardChipInstances) / sizeof(pd_board_chip_instance_t); ++index)
    {
        if (s_PDBoardChipInstances[index].occupied)
        {
            PD_DpHpdDriver1msISR(&s_PDBoardChipInstances[index].hpdDriverInstance);
        }
    }
}

void PD_DpBoardChipTaskSetEvent(uint32_t bitMask)
{
    portBASE_TYPE taskToWake = pdFALSE;
    if (s_PDBoardChipTaskEvent)
    {
#if defined(__GIC_PRIO_BITS)
        if ((__get_CPSR() & CPSR_M_Msk) == 0x13)
#else
        if (__get_IPSR())
#endif
        {
            if (pdPASS == xEventGroupSetBitsFromISR(s_PDBoardChipTaskEvent, (EventBits_t)bitMask, &taskToWake))
            {
                MISRAC_DISABLE
                portYIELD_FROM_ISR(taskToWake);
                MISRAC_ENABLE
            }
        }
        else
        {
            xEventGroupSetBits(s_PDBoardChipTaskEvent, (EventBits_t)bitMask);
        }
    }
}

static uint32_t PD_DpBoardChipTaskGetEvent(void)
{
    EventBits_t bits;

    if (s_PDBoardChipTaskEvent)
    {
        bits = xEventGroupWaitBits(s_PDBoardChipTaskEvent, (EventBits_t)0x000000FFu, pdTRUE, 0, portMAX_DELAY);
        return bits;
    }
    return 0;
}

void PD_DpBoardChipTask(void)
{
    uint32_t events = PD_DpBoardChipTaskGetEvent();

    if (events & USB_PD_DP_BOARD_CHIP_TASK_HPD_DRIVER_EVENT)
    {
        for (uint8_t index = 0; index < sizeof(s_PDBoardChipInstances) / sizeof(pd_board_chip_instance_t); ++index)
        {
            if (s_PDBoardChipInstances[index].occupied)
            {
                PD_DpHpdDrvierProcess(&s_PDBoardChipInstances[index].hpdDriverInstance);
            }
        }
    }
}
