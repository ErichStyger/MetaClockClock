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
    #define STEPPER_CLOCK_360_STEPS       (360)  /* number of steps for 360 degrees */
    #define STEPPER_TIME_360_DEGREE_MS    (2500)  /* time for a full round, needs to match the physical stepper motors on LPC boards */
  #else
    #define STEPPER_CLOCK_360_STEPS       (4320)  /* number of steps for 360 degrees */
    #define STEPPER_TIME_360_DEGREE_MS    (3000)  /* time for a full round */
  #endif

  /* period needed for a single step in microseconds */
  #define STEPPER_TIME_STEP_US             ((STEPPER_TIME_360_DEGREE_MS*1000U)/STEPPER_CLOCK_360_STEPS)
#else /* not an analog clock: it is a linear stepper */
#if 0 /* 'old' SmArtWall */
  #define STEPPER_FULL_RANGE_NOF_STEPS     (7350) /* number of steps for the full motion */
#else
  #define STEPPER_FULL_RANGE_NOF_STEPS     (1240) /* number of steps for the full motion */
#endif
  #define STEPPER_TIME_FULL_RANGE_MS       (6000) /* time for a full movement of the stepper */
  #define STEPPER_TIME_STEP_US             ((STEPPER_TIME_FULL_RANGE_MS*1000U)/STEPPER_FULL_RANGE_NOF_STEPS)
  #define STEPPER_MAX_SIMULTAN_MOVE		     (80) /* Maximum number of motors that should move at the same time*/
#endif

#ifndef STEPPER_CONFIG_USE_FREERTOS_HEAP
  #define STEPPER_CONFIG_USE_FREERTOS_HEAP   (McuLib_CONFIG_SDK_USE_FREERTOS)
    /*!< 1: use FreeRTOS Heap (default), 0: use stdlib malloc() and free() */
#endif

#define STEPPER_CONFIG_USE_FREERTOS_TIMER (1 && STEPPER_TIME_STEP_US>=1000 && !PL_CONFIG_USE_VIRTUAL_STEPPER)
  /*!< using FreeRTOS timer if it would fit the frequency (equal or below 1 kHz). Note for virtual (LED) steppers 1 ms resolution of FreeRTOS timer is not good enough to match the LPC845 steppers */

#ifndef STEPPER_CONFIG_USE_FREERTOS_TIMER
  #define STEPPER_CONFIG_USE_FREERTOS_TIMER   (1 && McuLib_CONFIG_SDK_USE_FREERTOS)
    /*!< 1: use FreeRTOS timer (max 1 kHz), 0: use hardware timer */
#endif

#endif /* STEPPERCONFIG_H_ */
