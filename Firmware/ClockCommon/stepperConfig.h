/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STEPPERCONFIG_H_
#define STEPPERCONFIG_H_

#include "McuLibconfig.h"

#include "platform.h"

#if PL_CONFIG_IS_ANALOG_CLOCK
  /* number of steps for a full round */
  #if PL_CONFIG_USE_VIRTUAL_STEPPER
    #define STEPPER_CLOCK_360_STEPS   (360) /* number of steps for 360 degrees */
  #else
    #define STEPPER_CLOCK_360_STEPS   (4320) /* number of steps for 360 degrees */
  #endif

  /* time for a full round */
  #define STEPPER_TIME_360_DEGREE_MS  (3000)

  /* period needed for a single step in microseconds */
  #define STEPPER_TIME_STEP_US        ((STEPPER_TIME_360_DEGREE_MS*1000U)/STEPPER_CLOCK_360_STEPS)
#else /* not an analog clock: it is a linear stepper */
  #define STEPPER_FULL_RANGE_NOF_STEPS     (7350) /* number of steps for the full motion */
  #define STEPPER_TIME_FULL_RANGE_MS       (2000) /* time for a full movement of the stepper */
  #define STEPPER_TIME_STEP_US        ((STEPPER_TIME_FULL_RANGE_MS*1000U)/STEPPER_FULL_RANGE_NOF_STEPS)
#endif

#ifndef STEPPER_CONFIG_USE_FREERTOS_HEAP
  #define STEPPER_CONFIG_USE_FREERTOS_HEAP   (McuLib_CONFIG_SDK_USE_FREERTOS)
    /*!< 1: use FreeRTOS Heap (default), 0: use stdlib malloc() and free() */
#endif

#endif /* STEPPERCONFIG_H_ */
