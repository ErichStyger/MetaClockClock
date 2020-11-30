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
#define PL_CONFIG_BOARD_ID            (PL_CONFIG_BOARD_ID_PIXELUNIT_K02FN128)

/* assign matrix configuration used, only needed for master: */
#define PL_MATRIX_CONFIGURATION_ID    (PL_MATRIX_ID_SMARTWALL_8x5)

#include "platform_common.h"



void PL_InitFromTask(void);
void PL_Init(void);

#endif /* PLATFORM_H_ */
