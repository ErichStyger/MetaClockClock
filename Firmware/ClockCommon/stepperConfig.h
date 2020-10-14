/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STEPPERCONFIG_H_
#define STEPPERCONFIG_H_

#include "McuLibconfig.h"

#include "platform.h"

/* number of steps for a full round */
#if PL_CONFIG_USE_LED_STEPPER
  #define STEPPER_CLOCK_360_STEPS   (360) /* number of steps for 360 degrees */
#else
  #define STEPPER_CLOCK_360_STEPS   (4320) /* number of steps for 360 degrees */
#endif

/* time for a full round */
#define STEPPER_TIME_360_DEGREE_MS  (3000)

/* period needed for a single step in microseconds */
#define STEPPER_TIME_STEP_US        ((STEPPER_TIME_360_DEGREE_MS*1000U)/STEPPER_CLOCK_360_STEPS)

#ifndef STEPPER_CONFIG_USE_FREERTOS_HEAP
  #define STEPPER_CONFIG_USE_FREERTOS_HEAP   (McuLib_CONFIG_SDK_USE_FREERTOS)
    /*!< 1: use FreeRTOS Heap (default), 0: use stdlib malloc() and free() */
#endif

#endif /* STEPPERCONFIG_H_ */
