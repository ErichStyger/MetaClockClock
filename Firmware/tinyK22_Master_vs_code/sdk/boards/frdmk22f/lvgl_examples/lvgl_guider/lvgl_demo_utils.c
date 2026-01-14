/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "lvgl_demo_utils.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t s_cpuClockFreqHz;

/*******************************************************************************
 * Code
 ******************************************************************************/

/* Initialize the timer to get microsecond. */
void DEMO_InitUsTimer(void)
{
    s_cpuClockFreqHz = SystemCoreClock;
#if defined(DWT)
    MSDK_EnableCpuCycleCounter();
#endif
}

/* Get current time in microsecond. */
uint32_t DEMO_GetUsTimer(void)
{
#if defined(DWT)
    uint32_t count = MSDK_GetCpuCycleCount();

    /*
     * Because core clock is faster than 1MHz, so after converted to microsecond,
     * the value is always smaller than UINT32_MAX.
     */
    return (uint32_t)COUNT_TO_USEC(count, s_cpuClockFreqHz);
#else
    return 0;
#endif
}

/* Get time elapsed in us. */
uint32_t DEMO_GetUsElapsed(uint32_t start)
{
    uint32_t stop    = DEMO_GetUsTimer();
    uint32_t elapsed = 0;

    /*If there is no overflow, simply count the delta time*/
    if (stop >= start)
    {
        elapsed = stop - start;
    }
    else
    {
        /*
         * We assume the overflow will not happen more than once as the
         * idle task switch out shall happen in 4.3sec (in case of 1GHz CPU).
         */
        uint64_t overflow = ((uint64_t)UINT32_MAX + 1) * 1000000U / DEMO_GetCpuClockFreq();
        elapsed           = overflow + stop - start;
    }
    return elapsed;
}

uint32_t DEMO_GetCpuClockFreq(void)
{
    return s_cpuClockFreqHz;
}
