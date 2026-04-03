/*!
 * Copyright (c) 2023-2024, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * \file
 * \brief Configuration of hardware and software.
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "IncludeMcuLibConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PL_CONFIG_USE_PICO_W            ((PL_CONFIG_HW_VERSION==PL_CONFIG_HW_ADIS_PICO_W_CONSOLE_V0_1) || (PL_CONFIG_HW_VERSION==PL_CONFIG_HW_ADIS_PICO_W_CONSOLE_V0_2))
 /* if using Pico W board */
  /* Note:
   * if using Pico-W:
   * a) enable 'library pico_cyw43_arch_lwip_sys_freertos' in CMakeLists.txt of src folder
   * b) Enable set(PICO_BOARD pico_w) in CMakeLists.txt in project root
   */

/* platform configuration macros: set to 1 to enable functionality */
#define PL_CONFIG_USE_LEDS              (1) /* if using LEDs */
#define PL_CONFIG_USE_APP_TASK          (1) /* if using app task */
#define PL_CONFIG_USE_BLINKY            (0 && PL_CONFIG_USE_LEDS)  /*!< if using blinky */
#define PL_CONFIG_USE_BUTTONS           (0) /* using nav switch buttons */
#define PL_CONFIG_USE_BUTTONS_IRQ       (1 && !(McuLib_CONFIG_CPU_IS_ESP32 && PL_CONFIG_USE_WIFI)) /* if using button interrupts */
  /* Note: on ESP32, there is a hardware bug, triggering interrupts on GPIO36 (right) and GPIO39 (down).
   * See https://github.com/espressif/esp-idf/commit/d890a516a1097f0a07788e203fdb1a82bb83520e
   * and 3.11 in https://www.espressif.com/sites/default/files/documentation/esp32_errata_en.pdf  */

/* Wireless related settings */
#define PL_CONFIG_USE_WIFI              (1) /* if using WiFi functionality */
#define PL_CONFIG_USE_IDENTIFY          (0 && PL_CONFIG_USE_WIFI) /*!< used to identify MAC, needed for EEE network */
#define PL_CONFIG_USE_UDP_SERVER        (0 && PL_CONFIG_USE_WIFI) /*!< UDP server, used for communication to robot */
#define PL_CONFIG_USE_UDP_CLIENT        (0 && PL_CONFIG_USE_WIFI) /*!< UDP client, optionally available for tests */
#define PL_CONFIG_USE_PING              (0 && PL_CONFIG_USE_WIFI) /*!< shell command with ping, to test network connection */
#define PL_CONFIG_USE_NTP_CLIENT        (1 && PL_CONFIG_USE_WIFI)
#define PL_CONFIG_USE_MQTT_CLIENT       (0 && PL_CONFIG_USE_WIFI)   /* if running MQTT client */

#define PL_CONFIG_USE_TIME_DATE         (1) /*!< if using Time and Date information */

#define PL_CONFIG_USE_WATCHDOG          (0 && McuWatchdog_CONFIG_USE_WATCHDOG) /* if using Watchdog timer or not, configured in IncludeMcuLibConfig.h */

#define PL_CONFIG_USE_SPI               (0) /* if using SPI bus */
#define PL_HAS_RADIO                    (0 && PL_CONFIG_USE_SPI && McuRNET_CONFIG_IS_ENABLED && McuNRF24L01_CONFIG_IS_ENABLED) /* RNET with nRF24L01+ */

#define PL_CONFIG_USE_SHELL             (1 && !PL_CONFIG_USE_UNIT_TESTS) /* command line shell: cannot use it with unit tests because of argument handling */
#define PL_CONFIG_USE_RTT               (1) /* SEGGER RTT */
#define PL_CONFIG_USE_UART              (0) /* if using UART for stdio */
#define PL_CONFIG_USE_TUD_CDC           (1) /* tinyUSB CDC device with McuShellCdcDevice */
#define PL_CONFIG_USE_SHELL_CDC         (1 && PL_CONFIG_USE_TUD_CDC) /* if using CDC as shell interface */
#define PL_CONFIG_USE_SEMIHOSTING       (0) /* if using semihosting for standard I/O */
#define PL_CONFIG_USE_RS485             (1 && McuUart485_CONFIG_USE_RS_485)
#define PL_CONFIG_USE_RS485_SHELL       (1 && PL_CONFIG_USE_RS485 && !McuUart485_CONFIG_USE_RAW)

#define PL_CONFIG_USE_BALBOA            (0 && McuUart485_CONFIG_USE_RS_485)

/* which MQTT client to use */
#define PL_CONFIG_USE_MQTT_SENSOR       (0)
#define PL_CONFIG_USE_MQTT_BALBOA       (PL_CONFIG_USE_BALBOA)
#define PL_CONFIG_USE_MQTT_HEIDELBERG   (0)

/* I2C: OLED and Sensor */
#define PL_CONFIG_USE_I2C               (0) /* if using I2C bus */
#define PL_CONFIG_USE_HW_I2C            (1 && PL_CONFIG_USE_I2C && CONFIG_USE_HW_I2C)
#define PL_CONFIG_USE_OLED              (1 && PL_CONFIG_USE_I2C)
#define PL_CONFIG_USE_SENSOR            (1 && PL_CONFIG_USE_I2C)
#define PL_CONFIG_USE_SHT31             (1 && PL_CONFIG_USE_SENSOR) /* board is using SHT31 */
#define PL_CONFIG_USE_SHT40             (!PL_CONFIG_USE_SHT31 && PL_CONFIG_USE_SENSOR) /* board is using SHT31 */

#define PL_CONFIG_USE_NVMC              (1) /* if using non-volatile memory for storing settings */
#define PL_CONFIG_USE_MININI            (1 && PL_CONFIG_USE_NVMC)

/* robot specific */
#define PL_CONFIG_USE_ROBOT_HAT         (0) /* if using RobotHAT */
#define PL_CONFIG_USE_CHALLENGE         (0 && PL_CONFIG_USE_ROTOT_HAT) /* HS2022 challenge */
#define PL_CONFIG_USE_ROBO_REMOTE       (0 && PL_CONFIG_USE_ROTOT_HAT && PL_CONFIG_USE_UDP_SERVER) /* UDP Remote controller for robot */

#define PL_CONFIG_USE_UNIT_TESTS        (1 && defined(ENABLE_UNIT_TESTS) && ENABLE_UNIT_TESTS==1) /* if using unit tests. ENABLE_UNIT_TESTS is set by the CMake file */
#if PL_CONFIG_USE_UNIT_TESTS && !defined(UNITY_OUTPUT_CHAR)
   #error "needs to be defined in IncludeMcuLibConfig.h!"
#endif

#define PL_CONFIG_USE_GCOV              (0 && McuRdimon_CONFIG_IS_ENABLED && McuCoverage_CONFIG_IS_ENABLED) /* if using gcov */
#define PL_CONFIG_USE_GPROF             (0) /* GNU gprof profiling */

/*!
 * \brief de-initializes the platform
 */
void PL_Deinit(void);

/*!
 * \brief initializes the platform
 */
void PL_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* PLATFORM_H_ */
