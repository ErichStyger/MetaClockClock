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
#define PL_CONFIG_BOARD_ID            (PL_CONFIG_BOARD_ID_PIXELUNIT_K02FN64)

/* *************************************************************************************** */
/* overwrite settings in platform_common.h */
#define PL_CONFIG_USE_NEO_PIXEL_HW      (1) /* has NeoPixel hardware */
#define PL_MATRIX_CONFIG_IS_RGB         (1) /* has RGB functionality */
#define PL_CONFIG_IS_ANALOG_CLOCK       (0) /* not an analog clock */
#define PL_CONFIG_USE_EXTENDED_HANDS    (0) /* disable dual hand */
#define PL_CONFIG_USE_LED_PIXEL         (1) /* enable LED pixel */
#define PL_CONFIG_USE_MOTOR_ON_OFF      (1) /* has motor on/off hardware */
#define PL_CONFIG_USE_FONT              (0) /* only implemented for dual shaft motors */

#include "platform_common.h"


void PL_InitFromTask(void);
void PL_Init(void);

#endif /* PLATFORM_H_ */
