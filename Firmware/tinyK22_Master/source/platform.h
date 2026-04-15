/*
 * Copyright (c) 2020-2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "McuLibconfig.h"
#include "platform_id.h"

/* select the board used: */
#define PL_CONFIG_BOARD_ID            (PL_CONFIG_BOARD_ID_MASTER_K22FN512)

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
  #define PL_CONFIG_HAS_SWITCH_USER                 (0 && PL_CONFIG_HAS_BUTTONS) /* single user button (long, short) to turn on/off clock */
  //#define PL_CONFIG_USE_MOTOR_ON_AT_STARTUP (0) /* \TODO */
#else
  #error
#endif
/* ********************************************************************* */
/* override settings in platform_common.h: */
#include "platform_common.h"


void PL_InitFromTask(void);
void PL_Init(void);

#endif /* PLATFORM_H_ */
