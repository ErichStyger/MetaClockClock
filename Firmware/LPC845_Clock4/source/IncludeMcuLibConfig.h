/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* header file is included with -include compiler option
Instructions:
 - Remove the 'Template_' from the name and place this file into your 'src' folder.
 - Include it with the -include compiler option with:  "${ProjDirPath}/source/IncludeMcuLibConfig.h"
 - Add the following to the -I compiler option:
../McuLib
../McuLib/config
../McuLib/config/fonts
../McuLib/fonts
../McuLib/src
../McuLib/FreeRTOS/Source/include
../McuLib/FreeRTOS/Source/portable/GCC/ARM_CM4F
../McuLib/SEGGER_RTT
../McuLib/SEGGER_Sysview
../McuLib/TraceRecorder/config
../McuLib/TraceRecorder/include
../McuLib/TraceRecorder/streamports/Jlink_RTT/include
../McuLib/HD44780
 */

#ifndef INCLUDEMCULIBCONFIG_H_
#define INCLUDEMCULIBCONFIG_H_

/* ------------------- SDK/Library ---------------------------*/
#define McuLib_CONFIG_SDK_VERSION_USED   McuLib_CONFIG_SDK_MCUXPRESSO_2_0
#define McuLib_CONFIG_CPU_IS_LPC         (1)  /* LPC845 */
#define McuLib_CONFIG_CPU_VARIANT        McuLib_CONFIG_CPU_VARIANT_NXP_LPC845
#define McuLib_CONFIG_CORTEX_M           (0)  /* LPC845 is a Cortex-M0+ */
#define McuLib_CONFIG_CPU_IS_KINETIS     (0)  /* disabling the default Kinetis */

/* ------------------- RTOS ---------------------------*/
#define McuLib_CONFIG_SDK_USE_FREERTOS       (1)
#define configTOTAL_HEAP_SIZE                (12*1024 + 256)
//#define configUSE_HEAP_SECTION_NAME        (1)
//#define configHEAP_SECTION_NAME_STRING     ".bss.$SRAM_LOWER.FreeRTOS"
#define configQUEUE_REGISTRY_SIZE            (16)
#define configMINIMAL_STACK_SIZE             (200/sizeof(StackType_t))
#define configTIMER_TASK_STACK_DEPTH         (400/sizeof(StackType_t))
#define configUSE_TIMERS                     (0)
#define INCLUDE_xTimerPendFunctionCall       (0)

#define configUSE_TRACE_FACILITY              (0)
#define configGENERATE_RUN_TIME_STATS         (0)

/* performance counter: */
#define configGENERATE_RUN_TIME_STATS_USE_TICKS     (0)
#define configGET_RUNTIMER_COUNTER_VALUE_FROM_ISR   AppGetRuntimeCounterValueFromISR
#define configCONFIGURE_TIMER_FOR_RUNTIME_STATS     AppConfigureTimerForRuntimeStats
/* -------------------------------------------------*/
/* Segger SystemViewer: */
#define configUSE_SEGGER_SYSTEM_VIEWER_HOOKS        (0)
#define SYSVIEW_APP_NAME                            "LPC845 Clock4"
#define SYSVIEW_DEVICE_NAME                         "LPC845"
#define McuSystemView_CONFIG_RTT_BUFFER_SIZE        (128)
#define McuSystemView_CONFIG_RTT_CHANNEL            (1)
/* -------------------------------------------------*/
/* Percepio Tracealyzer */
#define configUSE_PERCEPIO_TRACE_HOOKS                  (0)
#define McuPercepio_CONFIG_START_TRACE_IN_STARTUP_MODE  TRC_START
/* -------------------------------------------------*/
/* Shell */
#define McuShell_CONFIG_PROJECT_NAME_STRING           "LPC845 Clock4"
#define McuShell_CONFIG_MULTI_CMD_ENABLED             (1)
#define McuShell_CONFIG_DEFAULT_SHELL_BUFFER_SIZE     (256)
#define McuShell_CONFIG_MULTI_CMD_SIZE                (200) /* max size of each command */
#define McuShellUart_CONFIG_UART                      McuShellUart_CONFIG_UART_NONE
/* -------------------------------------------------*/
/* RTT */
#define McuRTT_CONFIG_RTT_BUFFER_SIZE_DOWN            (150)
#define McuRTT_CONFIG_BLOCKING_SEND                   (1)
#define McuRTT_CONFIG_BLOCKING_SEND_TIMEOUT_MS        (10)
#define McuRTT_CONFIG_BLOCKING_SEND_WAIT_MS           (5)
#define McuRTT_CONFIG_RTT_BUFFER_SIZE_UP              (128)
#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  #define McuRTT_CONFIG_RTT_MAX_NUM_UP_BUFFERS          (2)
  #define McuRTT_CONFIG_RTT_MAX_NUM_DOWN_BUFFERS        (2)
#else
  #define McuRTT_CONFIG_RTT_MAX_NUM_UP_BUFFERS          (1)
  #define McuRTT_CONFIG_RTT_MAX_NUM_DOWN_BUFFERS        (1)
#endif
/* ------------------- I2C ---------------------------*/
#define CONFIG_USE_HW_I2C                             (0) /* if using HW I2C, otherwise use software bit banging */
#define McuGenericI2C_CONFIG_USE_ON_ERROR_EVENT       (0)
#define McuGenericI2C_CONFIG_USE_ON_REQUEST_BUS_EVENT (0)
#define McuGenericI2C_CONFIG_USE_MUTEX                (0 && McuLib_CONFIG_SDK_USE_FREERTOS)
/* -------------------------------------------------*/
/* stepper motor driver */
#define McuX12_017_CONFIG_QUAD_DRIVER                 (1)  /* using quad driver */
/* -------------------------------------------------*/
/* McuLog */
#define McuLog_CONFIG_IS_ENABLED                      (0)
#define McuLog_CONFIG_LOG_TIMESTAMP_DATE              (0)
#define McuLog_CONFIG_LOG_TIMESTAMP_TIME              (0)
#define McuLog_CONFIG_USE_RTT_DATA_LOGGER             (0)
#define McuLog_CONFIG_RTT_DATA_LOGGER_BUFFER_SIZE     (128)
/* ---------------------------------------------------------------------------------------*/
/* MinINI */
#define McuMinINI_CONFIG_FS                         (McuMinINI_CONFIG_FS_TYPE_FLASH_FS)
#define McuMinINI_CONFIG_FLASH_NVM_ADDR_START       (0xFC00) /* LPC845 has 64k FLASH (0x10000), last 1k page is used for NVM */
#define McuMinINI_CONFIG_FLASH_NVM_BLOCK_SIZE       (0x400)   /* sector size is 1k */
#define McuMinINI_CONFIG_FLASH_NVM_MAX_DATA_SIZE    (256) /* must be 64, 128, 256, ... */
/* -------------------------------------------------*/
/* McuFlash */
#define McuFlash_CONFIG_IS_ENABLED                  (1)
/* -------------------------------------------------*/
/* McuUart485 */
#define McuUart485_CONFIG_USE_RS_485                (1)

#endif /* INCLUDEMCULIBCONFIG_H_ */
