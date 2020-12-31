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

#if 0
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_8x3 /* original meta clock with 24 clocks */
  #define PL_CONFIG_USE_RTT               (1) /* optional: enable RTT as extra interface beside UART */
#elif 0
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_12x5_60B  /* 60 billion lights */
  #define PL_CONFIG_USE_VIRTUAL_STEPPER   (0) /* not have the hardware on LPC845-BRK */
  #define PL_MATRIX_CONFIG_IS_RGB         (0) /* disable color to fit into available RAM */
  #define PL_CONFIG_USE_RTT               (1) /* optional: enable RTT as extra interface beside UART */
#elif 0
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_12x5_MOD  /* modular round clock version */
  #define PL_CONFIG_USE_MOTOR_ON_OFF      (1) /* has motor on/off hardware */
  #define PL_MATRIX_CONFIG_IS_RGB         (0) /* disable color to fit into available RAM */
  #define PL_CONFIG_USE_RTT               (1) /* optional: enable RTT as extra interface beside UART */
#elif 1
  #define PL_MATRIX_CONFIGURATION_ID      PL_MATRIX_ID_CLOCK_16x9_ALEXIS
  #define PL_CONFIG_USE_RELATIVE_MOVES    (0) /* disable relative moves to fit into available RAM */
  #define PL_CONFIG_USE_RTT               (0) /* need to disable RTT to fit into available RAM */
#elif 1
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
