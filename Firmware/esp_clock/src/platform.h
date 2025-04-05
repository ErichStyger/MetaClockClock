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

#define PL_CONFIG_USE_PICO_W            ((PL_CONFIG_HW_VERSION==PL_CONFIG_HW_ADIS_PICO_W_CONSOLE_V0_1) || (PL_CONFIG_HW_VERSION==PL_CONFIG_HW_ADIS_PICO_W_CONSOLE_V0_2))
 /* if using Pico W board */
  /* Note:
   * if using Pico-W:
   * a) enable 'library pico_cyw43_arch_lwip_sys_freertos' in CMakeLists.txt of src folder
   * b) Enable set(PICO_BOARD pico_w) in CMakeLists.txt in project root
   */

/* platform configuration macros: set to 1 to enable functionality */
#define PL_CONFIG_USE_LEDS              (1)  /* if using LEDs */
#define PL_CONFIG_USE_BLINKY            (1 && PL_CONFIG_USE_LEDS)  /*!< if using blinky */
#define PL_CONFIG_USE_BUTTONS           (1) /* using nav switch buttons */
#define PL_CONFIG_USE_BUTTONS_IRQ       (0 && !(McuLib_CONFIG_CPU_IS_ESP32 && PL_CONFIG_USE_WIFI)) /* if using button interrupts */
  /* Note: on ESP32, there is a hardware bug, triggering interrupts on GPIO36 (right) and GPIO39 (down).
   * See https://github.com/espressif/esp-idf/commit/d890a516a1097f0a07788e203fdb1a82bb83520e
   * and 3.11 in https://www.espressif.com/sites/default/files/documentation/esp32_errata_en.pdf  */

/* Wireless related settings */
#define PL_CONFIG_USE_WIFI              (0) /* if using WiFi functionality */
#define PL_CONFIG_USE_IDENTIFY          (1 && PL_CONFIG_USE_WIFI) /*!< used to identify MAC, needed for EEE network */
#define PL_CONFIG_USE_UDP_SERVER        (1 && PL_CONFIG_USE_WIFI) /*!< UDP server, used for communication to robot */
#define PL_CONFIG_USE_UDP_CLIENT        (0 && PL_CONFIG_USE_WIFI) /*!< UDP client, optionally available for tests */
#define PL_CONFIG_USE_PING              (0 && PL_CONFIG_USE_WIFI) /*!< shell command with ping, to test network connection */
#define PL_CONFIG_USE_SNTP_TIME         (1 && PL_CONFIG_USE_WIFI) /*!< getting time from an NTP server */

#define PL_CONFIG_USE_TIME_DATE         (1) /*!< if using Time and Date information */

#define PL_CONFIG_USE_SPI               (0) /* if using SPI bus */
#define PL_HAS_RADIO                    (1 && PL_CONFIG_USE_SPI && McuRNET_CONFIG_IS_ENABLED && McuNRF24L01_CONFIG_IS_ENABLED) /* RNET with nRF24L01+ */

#define PL_CONFIG_USE_SHELL             (1) /* command line shell */
#define PL_CONFIG_USE_RTT               (0) /* SEGGER RTT */
#define PL_CONFIG_USE_USB_CDC           (0 && PL_CONFIG_USE_SHELL) /* if using USB CDC with shell */
#define PL_CONFIG_USE_RS485             (0 && McuUart485_CONFIG_USE_RS_485)
#define PL_CONFIG_USE_RS485_SHELL       (1 && PL_CONFIG_USE_RS485)

/* I2C: OLED and Sensor */
#define PL_CONFIG_USE_I2C               (0) /* if using I2C bus */
#define PL_CONFIG_USE_HW_I2C            (1 && PL_CONFIG_USE_I2C && CONFIG_USE_HW_I2C)
#define PL_CONFIG_USE_OLED              (0 && PL_CONFIG_USE_I2C)
#define PL_CONFIG_HAS_LCD               (1 && PL_CONFIG_USE_I2C && !PL_CONFIG_USE_OLED && PL_HAS_RADIO)
#define PL_CONFIG_HAS_LCD_MENU          (1 && PL_CONFIG_HAS_LCD) /* experimental */
#define PL_CONFIG_USE_SENSOR            (0 && PL_CONFIG_USE_I2C)
#define PL_CONFIG_USE_SHT31             (0 && PL_CONFIG_USE_SENSOR) /* board is using SHT31 */
#define PL_CONFIG_USE_SHT40             (!PL_CONFIG_USE_SHT31 && PL_CONFIG_USE_SENSOR) /* board is using SHT31 */

#define PL_CONFIG_USE_NVMC              (0) /* if using non-volatile memory for storing settings */
#define PL_CONFIG_USE_MININI            (1 && PL_CONFIG_USE_NVMC)

/* robot specific */
#define PL_CONFIG_USE_ROBOT_HAT         (0) /* if using RobotHAT */
#define PL_CONFIG_USE_ROBO_REMOTE       (0 && PL_CONFIG_USE_ROBOT_HAT && PL_CONFIG_USE_UDP_SERVER) /* UDP Remote controller for robot */

/* the following ones are not implemented yet: */
#define PL_CONFIG_USE_WDT               (0) /* watchdog */

/*! \brief
 * Platform initialization
 */
void PL_Init(void);

#endif /* PLATFORM_H_ */
