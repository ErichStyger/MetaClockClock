/*!
 * Copyright (c) 2023-2024, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * \file
 * \brief Configuration of hardware and software.
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_


#include "McuLibconfig.h"
#include "platform_id.h"

/* select the board used: */
#define PL_CONFIG_BOARD_ID            (PL_CONFIG_BOARD_ID_MASTER_PICO_W)

/* assign matrix configuration used, only needed for master: */
#if 0   /* V1: original meta clock with 24 clocks */
  #define PL_MATRIX_CONFIGURATION_ID                PL_MATRIX_ID_CLOCK_8x3
#elif 0 /* V2: 60 billion lights with LPC and WS2812B, now at CSEM in oak enclosure */
  #define PL_MATRIX_CONFIGURATION_ID                PL_MATRIX_ID_CLOCK_12x5_60B
  #define PL_MATRIX_CONFIG_IS_RGB                   (1) /* enable or disable RGB functions */
  #define PL_CONFIG_USE_NEO_PIXEL_HW                (1) /* we drive the NeoPixels directly with hardware pins */
  #define PL_CONFIG_USE_LED_RING                    (1) /* we do have LED rings */
  #define PL_CONFIG_USE_VIRTUAL_STEPPER             (1) /* LED rings are driven by virtual stepper motors */
  #define PL_CONFIG_USE_BLE                         (1) /* if using Adafruit BLE module */
  #define PL_CONFIG_USE_SHELL_UART                  (1) /* if using shell UART to K20 */
#elif 0 /* V3: modular round clock version on canvas: replaced with the 13x6 (78) clock version */
  #define PL_MATRIX_CONFIGURATION_ID                PL_MATRIX_ID_CLOCK_12x5_MOD
  #define PL_CONFIG_USE_MOTOR_ON_OFF                (1) /* has motor on/off hardware */
  #define PL_CONFIG_USE_LED_RING                    (0) /* if we do have LED rings */
  #define PL_MATRIX_CONFIG_IS_RGB                   (PL_CONFIG_USE_LED_RING)
  #define PL_CONFIG_CHECK_LAST_ERROR                (0) /* not check for cmd errors to improve speed */
  #define PL_CONFIG_CLOCK_ON_BY_DEFAULT             (0) /* if clock mode is on by default */
  #define PL_CONFIG_INTERMEZZO_ON_BY_DEFAULT        (0) /* if intermezzo is on by default */
  #define PL_CONFIG_CLOCK_DEFAULT_FONT              MFONT_SIZE_3x5
  #define PL_CONFIG_MATRIX_DEFAULT_HAND_COLOR       (0xFF0000)
  #define PL_CONFIG_MATRIX_DEFAULT_HAND_BRIGHTNESS  (0x50)
  #define PL_CONFIG_CLOCK_RANDOM_COLOR_ON           (1) /* if clock hands are enabled with random color*/
#elif 0 /* V4: 24 clock version with round PCBs: wooden frame on walnut veneer: https://mcuoneclipse.com/2021/01/01/metaclockclock-v4-for-the-year-2021/ */
  #define PL_MATRIX_CONFIGURATION_ID                PL_MATRIX_ID_CLOCK_8x3_V4
  #define PL_CONFIG_USE_MOTOR_ON_OFF                (1) /* has motor on/off hardware */
  #define PL_MATRIX_CONFIG_IS_RGB                   (0) /* has RGB functionality */
  #define PL_CONFIG_USE_LED_RING                    (PL_MATRIX_CONFIG_IS_RGB) /* we do have LED rings */
  #define PL_CONFIG_USE_EXTENDED_HANDS              (0) /* using extended hand */
  #define PL_CONFIG_CLOCK_ON_BY_DEFAULT             (0) /* clock on by default */
  #define PL_CONFIG_INTERMEZZO_ON_BY_DEFAULT        (0) /* if intermezzo is on by default */
  #define PL_CONFIG_MATRIX_DEFAULT_HAND_COLOR       (0xFF0000)
  #define PL_CONFIG_MATRIX_DEFAULT_HAND_BRIGHTNESS  (0xff) /* 0..0xff */
  #define PL_CONFIG_CLOCK_RANDOM_COLOR_ON           (1) /* if clock hands are enabled with random color */
  #define PL_CONFIG_HAS_BUTTONS                     (1) /* has user button on master to turn on/off clock */
  #define PL_CONFIG_HAS_SWITCH_USER                 (1 && PL_CONFIG_HAS_BUTTONS) /* single user button (long, short) to turn on/off clock */
#elif 0 /* Alexis clock */
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_16x9_ALEXIS
  #define PL_CONFIG_USE_ESP32_UART        (1) /* using the 'ESP32' UART for the shell */
#elif 0 /* second Sm(A)rtWall project by Livio Stadelmann */
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_SMARTWALL_16x5
  #define PL_MATRIX_CONFIG_IS_RGB         (1) /* has RGB functionality */
  #define PL_CONFIG_USE_LED_RING          (PL_MATRIX_CONFIG_IS_RGB) /* we do have LED rings */
  #define PL_CONFIG_USE_LED_PIXEL         (1 && PL_CONFIG_USE_NEO_PIXEL_HW) /* 'moving' pixels */
  #define PL_CONFIG_USE_NEO_PIXEL_HW      (1) /* we drive the NeoPixels directly by hardware */
  #define PL_CONFIG_USE_MOTOR_ON_OFF      (0) /* has motor on/off hardware */
  #define PL_CONFIG_USE_MOTOR_ON_OFF_AUTO (1) /* automatically turn on/off motors */
  #define PL_CONFIG_IS_ANALOG_CLOCK       (0) /* we are not a (round) analog clock with hands */
  #define PL_CONFIG_USE_STEPPER           (1) /* we do use stepper, but through shift registers */
  #define PL_CONFIG_USE_DEMOS             (1) /* only implemented for dual shaft motors and smart wall */
  #define PL_CONFIG_DEMOS_ON_BY_DEFAULT	  (0) /* Demos on by default */
  #define PL_CONFIG_USE_INTERMEZZO        (1) /* only implemented for dual shaft motors and smart wall */
  #define PL_CONFIG_INTERMEZZO_ON_BY_DEFAULT (0) /* intermezzo on by default*/
  #define PL_CONFIG_USE_CLOCK             (1) /* only implemented for dual shaft motors and smart wall */
  #define PL_CONFIG_CLOCK_ON_BY_DEFAULT   (1) /* clock on by default */
  #define PL_CONFIG_USE_FONT              (0) /* only implemented for dual shaft motors */
  #define PL_CONFIG_USE_BLE               (0) /* no module installed, SPI1 on PTD5, PTD6 and PDTD7 used for shift register access */
  #define PL_CONFIG_USE_AUTOMATIC_DEMO_MODE (0) /* used to test the LEDs */
  #define PL_CONFIG_NOF_DRIVER_BOARDS     (5) /* number of motor/LED boards (with each 16 motors+LEDs on it */
#elif 0 /* circular clock: 12 for each digit plus on in the center: https://mcuoneclipse.com/2021/07/11/round-metaclockclock/ */
  #define PL_CONFIG_HAS_CIRCLE_CLOCK                (1)
  #define PL_MATRIX_CONFIGURATION_ID                PL_MATRIX_ID_CIRCULAR_CLOCK_1x12
  #define PL_CONFIG_MATRIX_DEFAULT_HAND_COLOR       (0x00FF00)
  #define PL_CONFIG_MATRIX_DEFAULT_HAND_BRIGHTNESS  (0xff)
  #define PL_CONFIG_USE_MOTOR_ON_OFF                (1) /* has motor on/off hardware */
  #define PL_MATRIX_CONFIG_IS_RGB                   (1) /* has RGB functionality */
  #define PL_CONFIG_USE_LED_RING                    (PL_MATRIX_CONFIG_IS_RGB) /* if have LED rings */
  #define PL_CONFIG_CHECK_LAST_ERROR                (0) /* not check for cmd errors to improve speed */
  #define PL_CONFIG_USE_BLE                         (0) /* Adafruit module not mounted on board */
  #define PL_CONFIG_CLOCK_ON_BY_DEFAULT             (0) /* clock on by default */
  #define PL_CONFIG_INTERMEZZO_ON_BY_DEFAULT        (0) /* intermezzos on by default */
  /* below things are not supported yet for this clock */
  #define PL_CONFIG_USE_FONT                        (0)
#elif 0 /* Cornelius 8x3 matrix, without LED rings, with tinyK22 plus Adafruit BLE module */
  #define PL_MATRIX_CONFIGURATION_ID                PL_MATRIX_ID_CLOCK_8x3_V4
  #define PL_CONFIG_USE_MOTOR_ON_OFF                (1) /* has motor on/off hardware */
  #define PL_MATRIX_CONFIG_IS_RGB                   (0) /* has RGB functionality */
  #define PL_CONFIG_USE_LED_RING                    (PL_MATRIX_CONFIG_IS_RGB) /* if have LED rings */
  #define PL_CONFIG_CLOCK_ON_BY_DEFAULT             (1) /* clock on by default */
  #define PL_CONFIG_INTERMEZZO_ON_BY_DEFAULT        (1) /* intermezzo on by default */
  #define PL_CONFIG_USE_BLE                         (1) /* Adafruit module mounted on board */
#elif 0 /* Cornelius 6x12 matrix */
  #define PL_MATRIX_CONFIGURATION_ID                PL_MATRIX_ID_CLOCK_12x6
  #define PL_CONFIG_USE_MOTOR_ON_OFF                (1) /* has motor on/off hardware */
  #define PL_MATRIX_CONFIG_IS_RGB                   (0) /* if has RGB functionality */
  #define PL_CONFIG_USE_LED_RING                    (0) /* if we have LED rings */
  #define PL_CONFIG_USE_BLE                         (1) /* Adafruit module mounted on board */
#elif 1 /* 6x13 Matrix with Oak wood front */
  #define PL_MATRIX_CONFIGURATION_ID                PL_MATRIX_ID_CLOCK_13x6
  #define PL_CONFIG_USE_MOTOR_ON_OFF                (1) /* has motor on/off hardware */
  #define PL_MATRIX_CONFIG_IS_RGB                   (1) /* if has RGB functionality */
  #define PL_CONFIG_USE_LED_RING                    (PL_MATRIX_CONFIG_IS_RGB) /* if we have LED rings */
  #define PL_CONFIG_USE_BLE                         (0) /* Adafruit module mounted on board */
  #define PL_CONFIG_HAS_BUTTONS                     (0) /* has user button on master to turn on/off clock */
  #define PL_CONFIG_HAS_SWITCH_USER                 (1 && PL_CONFIG_HAS_BUTTONS) /* single user button (long, short) to turn on/off clock */
  #define PL_CONFIG_USE_LEDS                        (0) /* \TODO */
  #define PL_CONFIG_USE_RTT                         (1) /* \TODO used as default shell IO */
  #define PL_CONFIG_USE_MOTOR_ON_AT_STARTUP (1) /* \TODO */
#else
  #error
#endif

/* ********************************************************************* */
/* override settings in platform_common.h: */
#include "platform_common.h"


void PL_InitFromTask(void);
void PL_Init(void);



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
//#define PL_CONFIG_USE_LEDS              (1) /* if using LEDs */
#define PL_CONFIG_USE_APP_TASK          (0) /* if using app task */
#define PL_CONFIG_USE_BLINKY            (0 && PL_CONFIG_USE_LEDS)  /*!< if using blinky */
#define PL_CONFIG_USE_BUTTONS           (0) /* using nav switch buttons */
#define PL_CONFIG_USE_BUTTONS_IRQ       (1 && !(McuLib_CONFIG_CPU_IS_ESP32 && PL_CONFIG_USE_WIFI)) /* if using button interrupts */
  /* Note: on ESP32, there is a hardware bug, triggering interrupts on GPIO36 (right) and GPIO39 (down).
   * See https://github.com/espressif/esp-idf/commit/d890a516a1097f0a07788e203fdb1a82bb83520e
   * and 3.11 in https://www.espressif.com/sites/default/files/documentation/esp32_errata_en.pdf  */

/* Wireless related settings */
#define PL_CONFIG_USE_WIFI              (MCU_WIFI_CONFIG_ENABLED)
#define PL_CONFIG_USE_IDENTIFY          (0 && PL_CONFIG_USE_WIFI) /*!< used to identify MAC, needed for EEE network */
#define PL_CONFIG_USE_UDP_SERVER        (0 && PL_CONFIG_USE_WIFI) /*!< UDP server, used for communication to robot */
#define PL_CONFIG_USE_UDP_CLIENT        (0 && PL_CONFIG_USE_WIFI) /*!< UDP client, optionally available for tests */
#define PL_CONFIG_USE_PING              (0 && PL_CONFIG_USE_WIFI) /*!< shell command with ping, to test network connection */
#define PL_CONFIG_USE_NTP_CLIENT        (1 && PL_CONFIG_USE_WIFI)
#define PL_CONFIG_USE_MQTT_CLIENT       (0 && PL_CONFIG_USE_WIFI)   /* if running MQTT client */

#define PL_CONFIG_USE_TIME_DATE         (0) /*!< if using Time and Date information */

#define PL_CONFIG_USE_WATCHDOG          (0 && McuWatchdog_CONFIG_USE_WATCHDOG) /* if using Watchdog timer or not, configured in IncludeMcuLibConfig.h */

#define PL_CONFIG_USE_SPI               (0) /* if using SPI bus */
#define PL_HAS_RADIO                    (0 && PL_CONFIG_USE_SPI && McuRNET_CONFIG_IS_ENABLED && McuNRF24L01_CONFIG_IS_ENABLED) /* RNET with nRF24L01+ */

//#define PL_CONFIG_USE_SHELL             (1 && !PL_CONFIG_USE_UNIT_TESTS) /* command line shell: cannot use it with unit tests because of argument handling */
//#define PL_CONFIG_USE_RTT               (1) /* SEGGER RTT */
#define PL_CONFIG_USE_UART              (0) /* if using UART for stdio */
#define PL_CONFIG_USE_TUD_CDC           (1) /* tinyUSB CDC device with McuShellCdcDevice */
#define PL_CONFIG_USE_SHELL_CDC         (1 && PL_CONFIG_USE_TUD_CDC) /* if using CDC as shell interface */
#define PL_CONFIG_USE_SEMIHOSTING       (0) /* if using semihosting for standard I/O */
//#define PL_CONFIG_USE_RS485             (1 && McuUart485_CONFIG_USE_RS_485)
//#define PL_CONFIG_USE_RS485_SHELL       (1 && PL_CONFIG_USE_RS485 && !McuUart485_CONFIG_USE_RAW)

#define PL_CONFIG_USE_BALBOA            (0 && McuUart485_CONFIG_USE_RS_485)

/* which MQTT client to use */
#define PL_CONFIG_USE_MQTT_SENSOR       (0)
#define PL_CONFIG_USE_MQTT_BALBOA       (PL_CONFIG_USE_BALBOA)
#define PL_CONFIG_USE_MQTT_HEIDELBERG   (0)

/* I2C: OLED and Sensor */
//#define PL_CONFIG_USE_I2C               (1) /* if using I2C bus */
//#define PL_CONFIG_USE_HW_I2C            (1 && PL_CONFIG_USE_I2C && CONFIG_USE_HW_I2C)
#define PL_CONFIG_USE_OLED              (0 && PL_CONFIG_USE_I2C)
#define PL_CONFIG_USE_SENSOR            (0 && PL_CONFIG_USE_I2C)
//#define PL_CONFIG_USE_SHT31             (0 && PL_CONFIG_USE_SENSOR) /* board is using SHT31 */
#define PL_CONFIG_USE_SHT40             (!PL_CONFIG_USE_SHT31 && PL_CONFIG_USE_SENSOR) /* board is using SHT31 */

//#define PL_CONFIG_USE_RTC               (1) /* if using an RTC */
#define PL_CONFIG_USE_EXT_I2C_RTC       (1 && PL_CONFIG_USE_RTC && PL_CONFIG_USE_I2C) /* DS3232M with internal memory */

//#define PL_CONFIG_USE_NVMC              (1) /* if using non-volatile memory for storing settings */
//#define PL_CONFIG_USE_MININI            (1 && PL_CONFIG_USE_NVMC)

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


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* PLATFORM_H_ */
