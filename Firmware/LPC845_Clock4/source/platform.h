/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

#include "McuLibConfig.h"

#include "McuLibConfig.h"
#include "platform_id.h"

/* select the board used: */
#if 1 /* 60 billion time clock: 1x4 clock module */
  #define PL_CONFIG_BOARD_ID            (PL_CONFIG_BOARD_ID_CLOCK_LPC845_1X4)
#else /* 'world stepper clock with 4 clocks */
  #define PL_CONFIG_BOARD_ID            (PL_CONFIG_BOARD_ID_CLOCK_LPC845_2X2)
  #define PL_CONFIG_USE_RTC             (1) /* RTC (at least software), for hardware add PL_CONFIG_USE_EXT_I2C_RTC */
  #define PL_CONFIG_USE_CLOCK           (1) /* implements world clock with 4 clocks */
  #define PL_CONFIG_WORLD_CLOCK         (1 && PL_CONFIG_USE_CLOCK) /* showing four time zones */
  #define PL_CONFIG_USE_RS485           (0) /* to save RAM */
#endif
/* ********************************************************************* */
/* override settings in platform_common.h: */
#define PL_CONFIG_USE_MAG_SENSOR        (1) /* if using magnets */

#include "platform_common.h"

void PL_InitFromTask(void);
void PL_Init(void);

#endif /* PLATFORM_H_ */
