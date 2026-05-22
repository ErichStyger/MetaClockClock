/*
 * Copyright (c) 2023-2024, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* header file is included with -include compiler option */

#ifndef MCULIB_CONFIG_CONFIG_H_
#define MCULIB_CONFIG_CONFIG_H_

/* ---------------------------------------------------------------------------------------*/
/* different HW/PCB versions: */
#define PL_CONFIG_HW_ADIS_ESP_ROBO_SHIELD         (1)  /* ESP32 shield on K22FX512 robot */
#define PL_CONFIG_HW_ADIS_PICO_W_CONSOLE_V0_1     (2)  /* Pico-W console board with OLED, sensor, nav switch, 1-Jul-2023 */
#define PL_CONFIG_HW_ADIS_PICO_W_CONSOLE_V0_2     (3)  /* same as PL_CONFIG_HW_ADIS_PICO_W_CONSOLE_V0_1, but with added nRF */
#define PL_CONFIG_HW_ADIS_ESP32_CONSOLE_V0_1      (4)  /* ESP32 DevKitC-32E with OLED, wired PCB, sensor, nav switch and RS-485, 5-Jul-2023, obsolete */
#define PL_CONFIG_HW_ADIS_ESP32_CONSOLE_V0_2      (5)  /* ESP32 DevKitC-32E with OLED, green PCB, sensor, nav switch, RS-485, nRF, 14-Jul-2023 */
#define PL_CONFIG_HW_ADIS_ESP32_CONSOLE_V1_0      (6)  /* ESP32 DevKitC-32E with OLED, black PCB, sensor, nav switch, RS-485, nRF, 07-Aug-2023 */

/* active PCB/Hardware version */
#define PL_CONFIG_HW_VERSION  (PL_CONFIG_HW_ADIS_PICO_W_CONSOLE_V0_2)
/* ---------------------------------------------------------------------------------------*/
/* SDK */
#define McuLib_CONFIG_CPU_IS_KINETIS                (0)
#define McuLib_CONFIG_CORTEX_M                      (0) /* RP2040 is a Cortex-M0+ */
#define McuLib_CONFIG_CPU_IS_RPxxxx                 (1)
#define McuLib_CONFIG_CPU_VARIANT                   McuLib_CONFIG_CPU_VARIANT_RP2040
#define McuLib_CONFIG_SDK_VERSION_USED              McuLib_CONFIG_SDK_RPI_PICO
/* ---------------------------------------------------------------------- */
/* FreeRTOS */
#define McuLib_CONFIG_SDK_USE_FREERTOS              (1)
#define configMINIMAL_STACK_SIZE                    (500/sizeof(StackType_t))
#define configTOTAL_HEAP_SIZE                       (60*1024)
#define configUSE_TIMERS                            (1) /* needed for debouncing and TimeDate (timer.c)*/
#define INCLUDE_xTimerPendFunctionCall              (1 && configUSE_TIMERS)
#define configTIMER_TASK_STACK_DEPTH                (1024/sizeof(StackType_t)) /* stack size for Timer Service task */
#define configTIMER_QUEUE_LENGTH                    (24)
#define configUSE_SEGGER_SYSTEM_VIEWER_HOOKS        (1)
#define INCLUDE_vTaskEndScheduler                   (1)
#define configUSE_IDLE_HOOK                         (1)
#define configUSE_TICK_HOOK                         (1)
#define configUSE_MALLOC_FAILED_HOOK                (1)
#define configCHECK_FOR_STACK_OVERFLOW              (1)
/* -------------------------------------------------*/
/* McuSystemView */
#define McuSystemView_CONFIG_RTT_BUFFER_SIZE            (4*1024)
#define McuSystemView_CONFIG_GENERATE_STEPTICK_EVENTS   (0)
#define McuSystemView_CONFIG_GENERATE_ISR_EVENTS        (1)
#define McuSystemView_CONFIG_GENERATE_TASK_READY_EVENTS (1)
#define McuSystemView_CONFIG_GENERATE_QUEUE_EVENTS      (1)
/* -------------------------------------------------*/
/* I2C */
#define CONFIG_USE_HW_I2C                           (1) /* if using HW I2C, otherwise use software bit banging */
#define MCUI2CLIB_CONFIG_I2C_DEVICE                 i2c0
#define MCUI2CLIB_CONFIG_SDA_GPIO_PIN               16u
#define MCUI2CLIB_CONFIG_SCL_GPIO_PIN               17u
#define MCUI2CLIB_CONFIG_ADD_DELAY_US               (0)
#define MCUI2CLIB_CONFIG_TIMEOUT_BYTE_US            (1000)
/* -------------------------------------------------*/
/* McuGenericI2C */
#define McuGenericI2C_CONFIG_USE_ON_ERROR_EVENT       (0)
#define McuGenericI2C_CONFIG_USE_ON_RELEASE_BUS_EVENT (0)
#define McuGenericI2C_CONFIG_USE_ON_REQUEST_BUS_EVENT (0)
#define McuGenericI2C_CONFIG_USE_MUTEX                (1 && McuLib_CONFIG_SDK_USE_FREERTOS)

#if CONFIG_USE_HW_I2C /* implementation in McuI2cLib.c */
  #define McuLib_CONFIG_MCUI2CLIB_ENABLED                       (1)
  #define McuGenericI2C_CONFIG_INTERFACE_HEADER_FILE            "McuI2cLib.h"
  #define McuGenericI2C_CONFIG_RECV_BLOCK                       McuI2cLib_RecvBlock
  #define McuGenericI2C_CONFIG_SEND_BLOCK                       McuI2cLib_SendBlock
  #if McuGenericI2C_CONFIG_SUPPORT_STOP_NO_START
  #define McuGenericI2C_CONFIG_SEND_BLOCK_CONTINUE              McuI2cLib_SendBlockContinue
  #endif
  #define McuGenericI2C_CONFIG_SEND_STOP                        McuI2cLib_SendStop
  #define McuGenericI2C_CONFIG_SELECT_SLAVE                     McuI2cLib_SelectSlave
  #define McuGenericI2C_CONFIG_RECV_BLOCK_CUSTOM_AVAILABLE      (0)
  #define McuGenericI2C_CONFIG_RECV_BLOCK_CUSTOM                McuI2cLib_RecvBlockCustom

  #define MCUI2CLIB_CONFIG_ADD_DELAY                            (0)
#else
  /* settings for McuGenericSWI2C */
  #define SDA1_CONFIG_PIN_NUMBER  (16)
  #define SCL1_CONFIG_PIN_NUMBER  (17)

  #define McuGenericSWI2C_CONFIG_DO_YIELD (0 && McuLib_CONFIG_SDK_USE_FREERTOS) /* because of Yield in GenericSWI2C */
  #define McuGenericSWI2C_CONFIG_DELAY_NS (0)
#endif
/* ---------------------------------------------------------------------------------------*/
/* McuSSD1306 */
#define McuSSD1306_CONFIG_SSD1306_DRIVER_TYPE         (1106)
#define McuSSD1306_CONFIG_DYNAMIC_DISPLAY_ORIENTATION  (0)
#define McuSSD1306_CONFIG_FIXED_DISPLAY_ORIENTATION    McuSSD1306_CONFIG_ORIENTATION_LANDSCAPE
/* -------------------------------------------------*/
/* RTT */
#define McuRTT_CONFIG_RTT_BUFFER_SIZE_DOWN            (256)
#define McuRTT_CONFIG_BLOCKING_SEND                   (0) /* 0: do not block if buffer full */
#define McuRTT_CONFIG_BLOCKING_SEND_TIMEOUT_MS        (5)
#define McuRTT_CONFIG_BLOCKING_SEND_WAIT_MS           (1)
#define McuRTT_CONFIG_RTT_BUFFER_SIZE_UP              (8*1024)
/* ---------------------------------------------------------------------- */
/* McuTimeDate */
#define McuTimeDate_CONFIG_TICK_TIME_MS               (100) /* less timer ticks, default is RTOS tick rate */
#define McuTimeDate_CONFIG_USE_SOFTWARE_RTC                        (1) /* enable software RTC */
#define McuTimeDate_CONFIG_USE_EXTERNAL_HW_RTC                     (1) /* enable external I2C RTC */
#define McuTimeDate_CONFIG_USE_INTERNAL_HW_RTC                     (0) /* no internal RTC */

#define McuTimeDate_CONFIG_INIT_SOFTWARE_RTC_METHOD                (McuTimeDate_INIT_SOFTWARE_RTC_FROM_EXTERNAL_RTC)
#define McuTimeDate_CONFIG_USE_GET_TIME_DATE_METHOD                (McuTimeDate_GET_TIME_DATE_METHOD_SOFTWARE_RTC)
#define McuTimeDate_CONFIG_SET_TIME_DATE_METHOD_USES_SOFTWARE_RTC  (1) /* if using software RTC */
#define McuTimeDate_CONFIG_SET_TIME_DATE_METHOD_USES_EXTERNAL_RTC  (0) /* if using external I2C RTC */
#define McuTimeDate_CONFIG_SET_TIME_DATE_METHOD_USES_INTERNAL_RTC  (0) /* if using internal HW RTC */
/* ---------------------------------------------------------------------- */
/* McuShell */
#define McuShell_CONFIG_PROJECT_NAME_STRING         "PicoW ClockMaster"
#define McuShell_CONFIG_PROMPT_STRING               "mstr> "
#define McuShell_CONFIG_MULTI_CMD_ENABLED           (1)
#define McuShell_CONFIG_MULTI_CMD_SIZE              (200)
#define McuShell_CONFIG_DEFAULT_SHELL_BUFFER_SIZE   (256)
/* ---------------------------------------------------------------------- */
/* McuFlash */
#define McuFlash_CONFIG_IS_ENABLED          (1) /* enable for MinINI with Flash FS */
/* ---------------------------------------------------------------------------------------*/
/* MinINI */
#define McuMinINI_CONFIG_FS                         (McuMinINI_CONFIG_FS_TYPE_FLASH_FS)
#define McuMinINI_CONFIG_FLASH_NVM_ADDR_START       ((XIP_BASE+2048*1024)-(McuMinINI_CONFIG_FLASH_NVM_NOF_BLOCKS*McuMinINI_CONFIG_FLASH_NVM_BLOCK_SIZE)) /* pico has 2 MB Flash, starting from XIP_BASE */
#define McuMinINI_CONFIG_FLASH_NVM_NOF_BLOCKS       (1)
#define McuMinINI_CONFIG_FLASH_NVM_BLOCK_SIZE       (0x1000) /* just use a single block */
#define McuMinINI_CONFIG_FLASH_NVM_MAX_DATA_SIZE    (0x1000) /* must be multiple of 4K */
#define McuMinINI_CONFIG_BUFFER_SIZE                (128) /* maximum line and path length */
/* ---------------------------------------------------------------------------------------*/
/* McuUart485 */
#define McuUart485_CONFIG_USE_RS_485          (1)
/* ---------------------------------------------------------------------------------------*/
/* McuSPI */
/* RP2040: nRF24L01+ on SPI1, GP12 (MISO), GP11 (MOSI), GP10 (CLK) */
#define MCUSPI_CONFIG_HW_TEMPLATE               MCUSPI_CONFIG_HW_TEMPLATE_RP2040_SPI1
#define MCUSPI_CONFIG_USE_CS                    (0) /* do not initialize CS pin */
#define MCUSPI_CONFIG_TRANSFER_BAUDRATE         (8*500000U)
/* ---------------------------------------------------------------------------------------*/
/* McuRNF24L01 */
#define McuNRF24L01_CONFIG_IS_ENABLED           (0)
#define McuNRF24L01_CONFIG_CE_PIN_NUMBER        (9)
#define McuNRF24L01_CONFIG_CSN_PIN_NUMBER       (13)
#define McuNRF24L01_CONFIG_USE_MUTEX            (0)
#define McuNRF24L01_CONFIG_IRQ_PIN_NUMBER       (8)
#define McuNRF24L01_CONFIG_IRQ_PIN_ENABLED      (1)
/* ---------------------------------------------------------------------------------------*/
/* McuRNET */
#define McuRNET_CONFIG_IS_ENABLED               (0)
#define RNet_App_CONFIG_DO_SANITY_CHECK         (0)
#define McuRNet_CONFIG_APPLICATION_HEADER_FILE  "RNet_AppConfig.h"
#define RNET_CONFIG_TRANSCEIVER_CHANNEL         (120) /* channel, default 81 */
#define RNET_CONFIG_NRF24_DATA_RATE             McuNRF24L01_RF_SETUP_RF_DR_2000
#define RSTDIO_CONFIG_QUEUE_LENGTH              (5*48) /* default is 48 */
#define RNET_CONFIG_MSG_QUEUE_NOF_RX_ITEMS      (32) /* default is 15 */
#define RNET_CONFIG_MSG_QUEUE_NOF_TX_ITEMS      (32) /* default is 15 */
/* ---------------------------------------------------------------------------------------*/
/* Unity */
#if ENABLE_UNIT_TESTS
    #if !__ASSEMBLER__ /* set if file is included by GNU as (assembler). Do not include normal C header files if running the assembler for example to assemble the pico startup code */
    #include "McuUnity.h"
    #define UNITY_OUTPUT_CHAR(a)                        McuUnity_putc(a)
    #define UNITY_OUTPUT_FLUSH()                        McuUnity_flush()
    #define UNITY_OUTPUT_START()                        McuUnity_start()
    #define UNITY_OUTPUT_COMPLETE()                     McuUnity_complete()
    #define UNITY_OUTPUT_COLOR                          /* use colored output */
    #endif
#endif
/* ---------------------------------------------------------------------------------------*/
/* McuSemihost */
#define McuSemihost_CONFIG_IS_ENABLED               (0)
#define McuSemihost_CONFIG_DEBUG_CONNECTION         McuSemihost_DEBUG_CONNECTION_SEGGER
#define McuSemihost_CONFIG_LOG_ENABLED              (0)
#define McuSemihost_CONFIG_RETARGET_STDLIB          (0)
/* ---------------------------------------------------------------------------------------*/
/* McuRdimon */
#define McuRdimon_CONFIG_IS_ENABLED                 (1 && McuSemihost_CONFIG_IS_ENABLED)       /* 1: RdiMon is enabled; 0: RdiMon is disabled*/
/* ---------------------------------------------------------------------------------------*/
/* McuCoverage */
#define McuCoverage_CONFIG_IS_ENABLED               (1)
#define McuCoverage_CONFIG_USE_FREESTANDING         (0 && McuCoverage_CONFIG_IS_ENABLED)
/* ---------------------------------------------------------------------------------------*/
/* McuLog */
#define McuLog_CONFIG_IS_ENABLED                (1)
#define McuLog_CONFIG_USE_FILE                  (0)
#define McuLog_CONFIG_USE_RTT_CONSOLE           (1)
#define McuLog_CONFIG_NOF_CONSOLE_LOGGER        (2) /* RTT and USB CDC */
#define McuLog_CONFIG_USE_COLOR                 (0)
#define McuLog_CONFIG_LOG_TIMESTAMP_DATE        (1)
#define McuLog_CONFIG_LOG_TIMESTAMP_TIME        (1)
/* ---------------------------------------------------------------------------------------*/
/* McuShellCdcDevice with tinyUSB */
#define McuShellCdcDevice_CONFIG_IS_ENABLED                 (1)
#define McuShellCdcDevice_CONFIG_BLOCKING_SEND              (0) /* 0: do not block if buffer full */
#define McuShellCdcDevice_CONFIG_BLOCKING_SEND_TIMEOUT_MS   (10) /* maximum timout value */
#define McuShellCdcDevice_CONFIG_BLOCKING_SEND_WAIT_MS      (1) /* waiting time in timeout loop */
/* ---------------------------------------------------------------------------------------*/
/* McuWatchdog */
#define McuWatchdog_CONFIG_USE_WATCHDOG                     (0)
#define McuWatchdog_CONFIG_REPORT_ID_INCLUDE_HEADER_FILE    "../../PicoW_ClockMaster/src/platform.h" /* path from McuLib/src/McuWatchdog.c to our platform.h */
#define McuWatchdog_CONFIG_REPORT_ID_INCLUDE_FILE           "../../PicoW_ClockMaster/src/McuWatchdog_IDs.inc" /* path from McuLib/src/McuWatchdog.c to our ID file */
#define McuWatchdog_CONFIG_HEALTH_CHECK_TIME_SEC            (15) /* longer time than usual, as WiFi network stack sometimes takes 10 secs to connect */
/* ---------------------------------------------------------------------------------------*/
/* McuPicoWiFi */
#define MCU_PICO_WIFI_CONFIG_ENABLED             (1) /* if McuPicoWiFi module is enabled, needed for onboard LED too on PicoW board */
/* ---------------------------------------------------------------------------------------*/
/* McuWiFi */
#define MCU_WIFI_CONFIG_ENABLED                  (1 && MCU_PICO_WIFI_CONFIG_ENABLED) /* if McuWiFi module is enabled */
/* ---------------------------------------------------------------------------------------*/
/* McuNtpClient */
#define MCU_NTP_CLIENT_CONFIG_ENABLED            (1 && MCU_PICO_WIFI_CONFIG_ENABLED) /* if McuNtpClient module is enabled */
/* ---------------------------------------------------------------------------------------*/
/* McuDnsResolver */
#define MCU_DNS_RESOLVER_CONFIG_ENABLED          (1 && MCU_PICO_WIFI_CONFIG_ENABLED) /* if McuDnsResolver module is enabled */
/* ---------------------------------------------------------------------------------------*/
/* McuMqttClient */
#define MCU_MQTT_CLIENT_CONFIG_ENABLED           (0 && MCU_PICO_WIFI_CONFIG_ENABLED) /* if McuMqttClient module is enabled */

#endif /* MCULIB_CONFIG_CONFIG_H_ */
