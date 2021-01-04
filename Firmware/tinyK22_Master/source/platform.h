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
#define PL_CONFIG_BOARD_ID            (PL_CONFIG_BOARD_ID_MASTER_K22FN512)

/* assign matrix configuration used, only needed for master: */
#if 0   /* V1: original meta clock with 24 clocks */
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_8x3
#elif 0 /* V2: 60 billion lights */
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_12x5_60B
  #define PL_CONFIG_USE_NEO_PIXEL_HW      (1) /* we drive the NeoPixels directly by hardware */
  #define PL_CONFIG_USE_LED_RING          (1) /* we do have LED rings */
  #define PL_CONFIG_USE_VIRTUAL_STEPPER   (1) /* LED rings are driven by virtual stepper motors */
  #define PL_CONFIG_USE_EXTENDED_HANDS    (0) /* using extended hand */
#elif 1 /* V3: modular round clock version */
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_12x5_MOD
  #define PL_CONFIG_USE_MOTOR_ON_OFF      (1) /* has motor on/off hardware */
  #define PL_CONFIG_USE_LED_RING          (1) /* we do have LED rings */
  #define PL_CONFIG_CHECK_LAST_ERROR      (0) /* not check for cmd errors to improve speed */
#elif 0 /* V4: 24 clock version with round PCBs */
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_8x3_V4
  #define PL_CONFIG_USE_MOTOR_ON_OFF      (1) /* has motor on/off hardware */
  #define PL_CONFIG_USE_LED_RING          (1) /* we do have LED rings */
  #define PL_CONFIG_USE_EXTENDED_HANDS    (1) /* using extended hand */
#elif 0 /* Alexis clock */
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_16x9_ALEXIS
#elif 1 /* Sm(A)rtWall project */
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
