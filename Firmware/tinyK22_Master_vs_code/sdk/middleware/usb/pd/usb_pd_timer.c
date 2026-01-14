/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_phy.h"
#include "usb_pd_timer.h"
#include "usb_pd_interface.h"
#if (defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT)
#include "usb_pd_alt_mode.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*
  1. timr has started & timr time out: timrsTimeOutState == 1;
  2. timr not start || timr time out: timrsRunningState == 0;
*/

static void PD_TimerCallback(pd_instance_t *pdInstance, uint8_t timrName)
{
    switch ((tTimer_t)timrName)
    {
        case tMsgHardResetCompleteTimer:
            /* do nothing */
            break;
#if 0
        /* if has USB function */
        case tUSBSuspendTimer:
        case tUSBTimer:
            break;
#endif

        default:
            /* do nothing, code will use _PD_TimerCheckInvalidOrTimeOut to check the timr */
            PD_StackSetEvent(pdInstance, PD_TASK_EVENT_TIME_OUT);
            break;
    }
}

static uint8_t PD_TimerCheckBit(volatile uint32_t *dataArray, tTimer_t timrName)
{
    if ((uint8_t)timrName >= PD_MAX_TIMER_COUNT)
    {
        return 0;
    }

    /* timr is not running */
    if ((dataArray[((uint8_t)timrName / 32U)] & (0x00000001UL << ((uint8_t)timrName & 0x1FU))) != 0U)
    {
        return 1;
    }
    return 0;
}

pd_status_t PD_TimerClear(pd_handle pdHandle, tTimer_t timrName)
{
    uint32_t bitMape;
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;
    OSA_SR_ALLOC();

    if ((uint8_t)timrName >= PD_MAX_TIMER_COUNT)
    {
        return kStatus_PD_Error;
    }

    bitMape = (uint32_t)(0x00000001UL << ((uint8_t)timrName & 0x1FU));

    OSA_ENTER_CRITICAL();
    pdInstance->timrsRunningState[((uint8_t)timrName / 32U)] &= (~bitMape);
    pdInstance->timrsTimeOutState[((uint8_t)timrName / 32U)] &= (~bitMape);
    OSA_EXIT_CRITICAL();

    return kStatus_PD_Success;
}

uint8_t PD_TimerCheckStarted(pd_handle pdHandle, tTimer_t timrName)
{
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;
    uint8_t retVal;

    if (PD_TimerCheckBit(&pdInstance->timrsRunningState[0], timrName) != 0U)
    {
        retVal = 1;
    }
    else if (PD_TimerCheckBit(&pdInstance->timrsTimeOutState[0], timrName) != 0U)
    {
        retVal = 1;
    }
    else
    {
        retVal = 0;
    }

    return retVal;
}

pd_status_t PD_TimerStart(pd_handle pdHandle, tTimer_t timrName, uint16_t timrTime)
{
    uint32_t bitMape;
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;
    OSA_SR_ALLOC();

    if ((uint8_t)timrName >= PD_MAX_TIMER_COUNT)
    {
        return kStatus_PD_Error;
    }
    if (timrTime == 0U)
    {
        return kStatus_PD_Error;
    }

    bitMape = (uint32_t)(0x00000001UL << ((uint8_t)timrName & 0x1FU));

    OSA_ENTER_CRITICAL();
    pdInstance->timrsTimeValue[timrName] = timrTime;
    pdInstance->timrsTimeOutState[((uint8_t)timrName / 32U)] &= (~(bitMape));
    pdInstance->timrsRunningState[((uint8_t)timrName / 32U)] |= bitMape;
    OSA_EXIT_CRITICAL();

    return kStatus_PD_Success;
}

uint8_t PD_TimerCheckInvalidOrTimeOut(pd_handle pdHandle, tTimer_t timrName)
{
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;

    return (PD_TimerCheckBit(&pdInstance->timrsRunningState[0], timrName) == 0U) ? 1U : 0U;
}

uint8_t PD_TimerCheckValidTimeOut(pd_handle pdHandle, tTimer_t timrName)
{
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;

    return PD_TimerCheckBit(&pdInstance->timrsTimeOutState[0], timrName);
}

void PD_TimerCancelAllTimers(pd_handle pdHandle, tTimer_t timrBegin, tTimer_t timrEnd)
{
    uint8_t index;
    uint32_t bitMape;
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;
    OSA_SR_ALLOC();

    if (((uint8_t)timrBegin >= PD_MAX_TIMER_COUNT) || ((uint8_t)timrEnd >= PD_MAX_TIMER_COUNT))
    {
        return;
    }
    OSA_ENTER_CRITICAL();
    for (index = (uint8_t)timrBegin; index <= (uint8_t)timrEnd; ++index)
    {
        bitMape = (uint32_t)(0x00000001UL << (index & 0x1FU));
        pdInstance->timrsRunningState[(index / 32U)] &= (~bitMape);
        pdInstance->timrsTimeOutState[(index / 32U)] &= (~bitMape);
    }
    OSA_EXIT_CRITICAL();
}

void PD_TimerIsrFunction(pd_handle pdHandle)
{
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;
    uint8_t index;
    uint8_t index8;
    uint32_t bitMape;
    uint8_t currentIndex;

    if (pdHandle == NULL)
    {
        return;
    }

    for (index = 0; index < (((uint8_t)tTimerCount + 31U) / 32U); ++index)
    {
        if (pdInstance->timrsRunningState[index] != 0U)
        {
            for (index8 = 0; index8 < 32U; ++index8)
            {
                bitMape      = (uint32_t)(0x00000001UL << index8);
                currentIndex = (uint8_t)((index * 32U) + index8);
                if (currentIndex < (uint8_t)tTimerCount)
                {
                    if ((pdInstance->timrsRunningState[index] & bitMape) != 0U)
                    {
                        if (((pdInstance->timrsTimeValue[currentIndex])--) == 0U)
                        {
                            PD_TimerCallback(pdInstance, currentIndex);
                            pdInstance->timrsTimeValue[currentIndex] = 0;
                            pdInstance->timrsRunningState[index] &= (~(bitMape));
                            pdInstance->timrsTimeOutState[index] |= (bitMape);
                        }
                    }
                }
            }
        }
    }
#if (defined PD_CONFIG_ALT_MODE_SUPPORT) && (PD_CONFIG_ALT_MODE_SUPPORT)
    PD_AltModeTimer1msISR();
#endif
}

void PD_TimerInit(pd_handle pdHandle)
{
    uint8_t index8;
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;

    for (index8 = 0; index8 < ((PD_MAX_TIMER_COUNT + 31U) / 32U); ++index8)
    {
        pdInstance->timrsRunningState[index8] = 0;
        pdInstance->timrsTimeOutState[index8] = 0;
    }
    for (index8 = 0; index8 < PD_MAX_TIMER_COUNT; ++index8)
    {
        pdInstance->timrsTimeValue[index8] = 0;
    }
}

uint8_t PD_TimerBusy(pd_handle pdHandle)
{
    pd_instance_t *pdInstance = (pd_instance_t *)pdHandle;
    uint32_t index32;

    for (index32 = 0; index32 < (((uint8_t)tTimerCount + 31U) / 32U); ++index32)
    {
        if (pdInstance->timrsRunningState[index32] != 0U)
        {
            return 1;
        }
    }
    return 0;
}
