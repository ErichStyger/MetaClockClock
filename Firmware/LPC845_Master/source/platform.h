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

#if 1
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_8x3 /* original meta clock with 24 clocks */
  #define PL_CONFIG_USE_CLOCK             (1)
  #define PL_CONFIG_USE_INTERMEZZO        (1)
#elif 1  /* note: not enough RAM */
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_12x5_60B  /* 60 billion lights */
  #define PL_CONFIG_USE_VIRTUAL_STEPPER   (0) /* not have the hardware on LPC845-BRK */
#elif 1
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_12x5_MOD  /* modular round clock version */
  #define PL_CONFIG_USE_MOTOR_ON_OFF      (1) /* has motor on/off hardware */
#elif 0
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_12x9_ALEXIS
#elif 0
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_SMARTWALL_8x5 /* new Sm(A)rtWall project */
  #define PL_CONFIG_USE_MOTOR_ON_OFF      (1) /* has motor on/off hardware */
#else
  #error
#endif
/* ********************************************************************* */
/* override settings in platform_common.h: */
#include "platform_common.h"

void PL_InitFromTask(void);
void PL_Init(void);

#endif /* PLATFORM_H_ */
