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
#if 0 /* 60 billion time */
  #define PL_CONFIG_BOARD_ID            (PL_CONFIG_BOARD_ID_CLOCK_LPC845_1X4)
#else /* 'world stepper clock */
  #define PL_CONFIG_BOARD_ID            (PL_CONFIG_BOARD_ID_CLOCK_LPC845_2X2)
#endif
/* ********************************************************************* */
/* override settings in platform_common.h: */
#define PL_CONFIG_USE_MAG_SENSOR        (1) /* if using magnets */

#include "platform_common.h"

void PL_InitFromTask(void);
void PL_Init(void);

#endif /* PLATFORM_H_ */
