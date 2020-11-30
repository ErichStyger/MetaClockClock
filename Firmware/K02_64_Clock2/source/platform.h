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
#define PL_CONFIG_BOARD_ID            (PL_CONFIG_BOARD_ID_CLOCK_K02FN64)

/* ********************************************************************* */
/* override settings in platform_common.h: */
#define PL_CONFIG_USE_MAG_SENSOR        (1)
#define PL_CONFIG_USE_NEO_PIXEL_HW      (1) /* has NeoPixels */
#define PL_CONFIG_USE_LED_RING          (1 && PL_CONFIG_USE_NEO_PIXEL_HW)
#define PL_CONFIG_USE_X12_LED_STEPPER   (1 && PL_CONFIG_USE_LED_RING) /* stepper motors with rings */

#include "platform_common.h"


void PL_InitFromTask(void);
void PL_Init(void);

#endif /* PLATFORM_H_ */
