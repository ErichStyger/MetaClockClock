/*
 * Copyright 2022, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _LVGL_FREERTOS_H_
#define _LVGL_FREERTOS_H_

#include <stdint.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if defined(SDK_OS_FREE_RTOS)
/* Define to 1, to enable task time measurement. */
#ifndef DEMO_MEASURE_TASK_TIME
#define DEMO_MEASURE_TASK_TIME 1
#endif
#endif

/*******************************************************************************
 * APIs
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

void traceTaskSwitchedOut(void);
void traceTaskSwitchedIn(void);

/*
 * Get Idle task percent.
 * This feature depends on micro-second timer, DEMO_InitUsTimer need be called first.
 */
uint32_t getIdleTaskPct(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

/*! @} */

#endif /*_LVGL_FREERTOS_H_*/
