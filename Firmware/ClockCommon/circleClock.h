/*
 * Copyright (c) 2021, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CIRCLECLOCK_H_
#define CIRCLECLOCK_H_

#include "platform.h"
#if PL_CONFIG_HAS_CIRCLE_CLOCK

#include "circleClock_config.h"
#include <stdint.h>

#define CIRCLE_CLOCK_MIDDLE_X_POS           (0)  /* X position of middle clock */
#define CIRCLE_CLOCK_MIDDLE_Y_POS           (0)  /* Y position of middle clock */

#define CIRCLE_CLOCK_MINUTE_Z_POS           (0)  /* Z position of minute hand */
#define CIRCLE_CLOCK_HOUR_Z_POS             (1)  /* Z position of hour hand */

#define CIRCLE_CLOCK_CIRCLE_START_X_POS     (1)  /* x start position of outer circle */
#define CIRCLE_CLOCK_CIRCLE_END_X_POS       (12)  /* x end position of outer circle */

/* draw a circle on the outside */
void CC_DrawCircle(void);

/* draw rays outside */
void CC_DrawRays(void);

/* show time */
void CC_ShowTime(uint8_t hour, uint8_t minute);

void CC_Deinit(void);
void CC_Init(void);

#endif /* PL_CONFIG_HAS_CIRCLE_CLOCK */

#endif /* CIRCLECLOCK_H_ */
