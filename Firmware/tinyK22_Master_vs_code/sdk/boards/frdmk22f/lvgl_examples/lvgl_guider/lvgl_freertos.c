/*
 * Copyright 2022, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "lvgl_freertos.h"
#include "FreeRTOS.h"
#include "task.h"
#include "lvgl_demo_utils.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if (INCLUDE_xTaskGetIdleTaskHandle == 1)
#define OS_IsInIdleTask (xTaskGetCurrentTaskHandle() == xTaskGetIdleTaskHandle())
#elif (INCLUDE_xTaskGetCurrentTaskHandle == 1)
#define OS_IsInIdleTask (strcmp(pcTaskGetName(xTaskGetCurrentTaskHandle()), "IDLE") == 0)
#else
#error Can check whether current task is idle task, check the FreeRTOS_Config.h
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
#if DEMO_MEASURE_TASK_TIME
static volatile uint64_t idle_cnt; /* Counter in us for idle task. */
static volatile uint64_t busy_cnt; /* Counter in us for other non-idle tasks. */
static volatile uint32_t task_switched_in; /* Timestamp of task switch in. */
#endif

/*******************************************************************************
 * Prototype
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t getIdleTaskPct(void)
{
#if DEMO_MEASURE_TASK_TIME
    if (busy_cnt + idle_cnt == 0)
        return 0UL;

    uint32_t idle_pct = (idle_cnt * 100) / (idle_cnt + busy_cnt);

    busy_cnt = 0;
    idle_cnt = 0;

    return idle_pct;
#else
    return 0UL;
#endif
}

void traceTaskSwitchedOut(void)
{
#if DEMO_MEASURE_TASK_TIME
    uint32_t elaps = DEMO_GetUsElapsed(task_switched_in);

    if (OS_IsInIdleTask)
    {
        idle_cnt += elaps;
	}
    else
	{
        busy_cnt += elaps;
	}
#endif
}

void traceTaskSwitchedIn(void)
{
#if DEMO_MEASURE_TASK_TIME
    task_switched_in = DEMO_GetUsTimer();
#endif
}
