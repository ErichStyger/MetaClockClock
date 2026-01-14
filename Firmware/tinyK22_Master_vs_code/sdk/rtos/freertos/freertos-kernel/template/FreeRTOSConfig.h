/*
 * FreeRTOS Kernel V11.1.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#if defined(__ICCARM__)||defined(__CC_ARM)||defined(__GNUC__)
    #include <stdint.h>
    extern uint32_t SystemCoreClock;
#endif

#include "FreeRTOSConfig_Gen.h"

#ifdef CONFIG_FREERTOS_USE_CUSTOM_CONFIG_FRAGMENT
#include "FreeRTOSConfig_frag.h"
#endif

/* Handle macros required to be defined.
 * If these macros are not defined set them to 0.
 */
#if !defined(configENABLE_FPU)
#define configENABLE_FPU 0
#endif

#if !defined(configENABLE_MPU)
#define configENABLE_MPU 0
#endif

#if !defined(configENABLE_TRUSTZONE)
#define configENABLE_TRUSTZONE 0
#endif

#if !defined(configENABLE_MVE)
#define configENABLE_MVE 0
#endif

#if !defined(configUSE_16_BIT_TICKS)
#define configUSE_16_BIT_TICKS  0
#endif

#if !defined(configUSE_IDLE_HOOK)
#define configUSE_IDLE_HOOK  0
#endif

#if !defined(configUSE_TICK_HOOK)
#define configUSE_TICK_HOOK  0
#endif

#if !defined(configUSE_PREEMPTION)
#define configUSE_PREEMPTION  0
#endif

#if !defined(configUSE_TIME_SLICING)
#define configUSE_TIME_SLICING  0
#endif

#if !defined(configSUPPORT_DYNAMIC_ALLOCATION)
#define configSUPPORT_DYNAMIC_ALLOCATION  0
#endif

#if USE_PERCEPIO_TRACELYZER == 1
    /* Do not include if processing assembly file */
    #if (!defined(__IAR_SYSTEMS_ASM__) && !defined(__ASSEMBLER__))
    #   include "fsl_device_registers.h"
    #   include "trcKernelPort.h"
    #endif
#endif

#endif /* FREERTOS_CONFIG_H */
