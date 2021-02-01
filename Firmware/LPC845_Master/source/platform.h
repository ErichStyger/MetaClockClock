/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "McuLibConfig.h"
#include "platform_id.h"

/* select the board used: */
#define PL_CONFIG_BOARD_ID            (PL_CONFIG_BOARD_ID_MASTER_LPC845_BRK)

/* assign matrix configuration used, only needed for master: */
#if 0   /* V1: original meta clock with 24 clocks */
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_8x3
#elif 0 /* V2: 60 billion lights */
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_12x5_60B
  #define PL_CONFIG_USE_VIRTUAL_STEPPER   (0) /* not have the hardware on LPC845-BRK */
  #define PL_MATRIX_CONFIG_IS_RGB         (0) /* disable color to fit into available RAM */
#elif 0 /* V3: modular round clock version */
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_12x5_MOD
  #define PL_CONFIG_USE_MOTOR_ON_OFF      (1) /* has motor on/off hardware */
  #define PL_MATRIX_CONFIG_IS_RGB         (0) /* disable color to fit into available RAM */
  #define PL_CONFIG_USE_RTT               (0) /* optional: enable RTT as extra interface beside UART */
  #define PL_CONFIG_CHECK_LAST_ERROR      (0) /* not check for cmd errors to improve speed */
  #define PL_CONFIG_HAS_BUTTONS           (1) /* requires FreeRTOS timers enabled */
#elif 1 /* V4: 24 clock version with round PCBs */
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_8x3_V4
  #define PL_CONFIG_USE_MOTOR_ON_OFF      (1) /* has motor on/off hardware */
  #define PL_CONFIG_USE_LED_RING          (1) /* we do have LED rings */
  #define PL_CONFIG_USE_EXTENDED_HANDS    (0) /* using extended hand */
  #define PL_CONFIG_USE_RTT               (0) /* need to disable RTT to fit into available RAM */
  #define PL_CONFIG_HAS_BUTTONS           (1) /* requires FreeRTOS timers enabled */
#elif 0 /* Alexis clock: won't fit anymore into the available amount of RAM */
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_16x9_ALEXIS
  #define PL_CONFIG_USE_RELATIVE_MOVES    (0) /* disable relative moves to fit into available RAM */
  #define PL_CONFIG_USE_RTT               (0) /* need to disable RTT to fit into available RAM */
  #define PL_CONFIG_HAS_BUTTONS           (0) /* disable to fit into available of RAM: would need to disable FreeRTOS timers too and tweak HEAP size */
#elif 0 /* Sm(A)rtWall project */
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_SMARTWALL_8x5
  #define PL_CONFIG_USE_MOTOR_ON_OFF      (1) /* has motor on/off hardware */
  #define PL_CONFIG_USE_DEMOS             (0) /* no demos as they are for dual shaft motors */
  #define PL_CONFIG_USE_INTERMEZZO        (0) /* only implemented for dual shaft motors */
  #define PL_CONFIG_USE_CLOCK             (0) /* only implemented for dual shaft motors */
  #define PL_CONFIG_USE_FONT              (0) /* only implemented for dual shaft motors */
#else
  #error
#endif
/* ********************************************************************* */
/* override settings in platform_common.h: */
#include "platform_common.h"


void PL_InitFromTask(void);
void PL_Init(void);

#endif /* PLATFORM_H_ */
