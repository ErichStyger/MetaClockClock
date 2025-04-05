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
#define PL_CONFIG_HW_VERSION  (PL_CONFIG_HW_ADIS_ESP32_CONSOLE_V1_0)

/* -------------- Core and McuLib general settings -------------------------- */
#define McuLib_CONFIG_CPU_IS_ESP32          (1)  /* Set CPU as ESP32 */
#define McuLib_CONFIG_CPU_IS_ARM_CORTEX_M   (0)

/* -------------- FreeRTOS and McuRTOS settings -------------------------- */
#define configHEAP_SCHEME_IDENTIFICATION    (0)  /* ESP-IDF RTOS used */

/* -------------- McuShell settings -------------------------- */
#define McuShell_CONFIG_PROJECT_NAME_STRING       "ESP32 Shell"
#define McuShell_CONFIG_ECHO_ENABLED              (1)
#define McuShell_CONFIG_PROMPT_STRING             "ESP32> "
#define McuShell_CONFIG_MULTI_CMD_ENABLED         (1)
#define McuShell_CONFIG_MULTI_CMD_SIZE            (128)
#define McuShell_CONFIG_DEFAULT_SHELL_BUFFER_SIZE (128)
/* -------------- McuCriticalSections settings -------------------------- */
#define McuCriticalSection_CONFIG_USE_RTOS_CRITICAL_SECTION  (1)

/* -------------- McuTimeDate settings -------------------------- */
#define McuTimeDate_CONFIG_TICK_TIME_MS    (10)
/* ------------------- I2C ---------------------------*/
#define CONFIG_USE_HW_I2C                             (1)
/* -------------------------------------------------*/
/* McuGenericI2C */
#define McuGenericI2C_CONFIG_USE_ON_ERROR_EVENT       (0)
#define McuGenericI2C_CONFIG_USE_ON_RELEASE_BUS_EVENT (0)
#define McuGenericI2C_CONFIG_USE_ON_REQUEST_BUS_EVENT (0)
#define McuGenericI2C_CONFIG_USE_MUTEX                (1) /* just one, do not check on FreeRTOS, because we use built-in IDF one */
/* ------------------- I2C ---------------------------*/
#if CONFIG_USE_HW_I2C /* implementation in McuI2cLib.c */
#if (PL_CONFIG_HW_VERSION==PL_CONFIG_HW_ADIS_ESP32_CONSOLE_V0_1) || (PL_CONFIG_HW_VERSION==PL_CONFIG_HW_ADIS_ESP32_CONSOLE_V0_2) || (PL_CONFIG_HW_VERSION==PL_CONFIG_HW_ADIS_ESP32_CONSOLE_V1_0)
  #define MCUI2CLIB_CONFIG_SDA_GPIO_PIN     21u
  #define MCUI2CLIB_CONFIG_SCL_GPIO_PIN     22u
#elif PL_CONFIG_HW_VERSION==PL_CONFIG_HW_ADIS_ESP_ROBO_SHIELD
  #define MCUI2CLIB_CONFIG_SDA_GPIO_PIN     18u
  #define MCUI2CLIB_CONFIG_SCL_GPIO_PIN     19u
#endif
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
  #if (PL_CONFIG_HW_VERSION==PL_CONFIG_HW_ADIS_ESP32_CONSOLE_V0_1) || (PL_CONFIG_HW_VERSION==PL_CONFIG_HW_ADIS_ESP32_CONSOLE_V0_2) || (PL_CONFIG_HW_VERSION==PL_CONFIG_HW_ADIS_ESP32_CONSOLE_V1_0)
    #define SDA1_CONFIG_PIN_NUMBER  (21)
    #define SCL1_CONFIG_PIN_NUMBER  (22)
  #elif PL_CONFIG_HW_VERSION==PL_CONFIG_HW_ADIS_ESP_ROBO_SHIELD
    #define SDA1_CONFIG_PIN_NUMBER  (18)
    #define SCL1_CONFIG_PIN_NUMBER  (19)
  #endif

  #define McuGenericSWI2C_CONFIG_DO_YIELD (0 && McuLib_CONFIG_SDK_USE_FREERTOS) /* because of Yield in GenericSWI2C */
  #define McuGenericSWI2C_CONFIG_DELAY_NS (0)
#endif
/* -------------------------------------------------*/
/* McuSSD1306 */
#define McuSSD1306_CONFIG_SSD1306_DRIVER_TYPE           (1106) /* use SH1106 */
#define McuSSD1306_CONFIG_SSD1306_I2C_DELAY_US          (0)
#define McuSSD1306_CONFIG_DYNAMIC_DISPLAY_ORIENTATION   (0)
#define McuSSD1306_CONFIG_FIXED_DISPLAY_ORIENTATION      McuSSD1306_CONFIG_ORIENTATION_LANDSCAPE
/* -------------------------------------------------*/
/* McuLog */
#define McuLog_CONFIG_IS_ENABLED                (1)
#define McuLog_CONFIG_USE_FILE                  (0)
#define McuLog_CONFIG_USE_RTT_CONSOLE           (0)
#define McuLog_CONFIG_NOF_CONSOLE_LOGGER        (1)
#define McuLog_CONFIG_USE_COLOR                 (1)
#define McuLog_CONFIG_LOG_TIMESTAMP_DATE        (1)
/* ---------------------------------------------------------------------------------------*/
/* McuUart485 */
#define McuUart485_CONFIG_USE_RS_485            (1)
#define McuUart485_CONFIG_USE_MODBUS            (McuModbus_CONFIG_IS_ENABLED)
#define McuUart485_CONFIG_UART_BAUDRATE         (115200)
/* Rx: IO5
 * Tx: IO4
 * EN: IO17
 */
#define McuUart485_CONFIG_RTS_PIN               (GPIO_NUM_17)
/* ---------------------------------------------------------------------------------------*/
/* McuSPI */
/* ESP:  nRF24L01+ on SPI3 (VSPI), IO19 (MISO), IO23 (MOSI), IO18 (CLK) */
#define MCUSPI_CONFIG_HW_TEMPLATE               MCUSPI_CONFIG_HW_TEMPLATE_ESP32_SPI3
#define MCUSPI_CONFIG_USE_CS                    (0) /* do not initialize CS pin */
#define MCUSPI_CONFIG_TRANSFER_BAUDRATE         500000U
/* ---------------------------------------------------------------------------------------*/
/* McuRNF24L01 */
#define McuNRF24L01_CONFIG_IS_ENABLED           (0)
#define McuNRF24L01_CONFIG_CE_PIN_NUMBER        (GPIO_NUM_33)
#define McuNRF24L01_CONFIG_CSN_PIN_NUMBER       (GPIO_NUM_32)
#define McuNRF24L01_CONFIG_USE_MUTEX            (1)   /* mutex needed, both shell and radio task use bus */
#define McuNRF24L01_CONFIG_IRQ_PIN_ENABLED      (1)   /* IRQ on IO0 */
#define McuNRF24L01_CONFIG_IRQ_PIN_NUMBER       (GPIO_NUM_0)   /* IO0 */
/* ---------------------------------------------------------------------------------------*/
/* McuRNET */
#define McuRNET_CONFIG_IS_ENABLED               (0)
#define RNet_App_CONFIG_DO_SANITY_CHECK         (0)
#define RNET_CONFIG_TRANSCEIVER_CHANNEL         (120) /* channel, default 81 */
#define RNET_CONFIG_NRF24_DATA_RATE             McuNRF24L01_RF_SETUP_RF_DR_2000
#define McuRNet_CONFIG_APPLICATION_HEADER_FILE  "RNet_AppConfig.h"
/* ---------------------------------------------------------------------------------------*/

#endif /* MCULIB_CONFIG_CONFIG_H_ */
