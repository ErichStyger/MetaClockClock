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
#include <stdbool.h>

#define CIRCLE_CLOCK_MIDDLE_X_POS           (0)  /* X position of middle clock */
#define CIRCLE_CLOCK_MIDDLE_Y_POS           (0)  /* Y position of middle clock */

#define CIRCLE_CLOCK_MINUTE_Z_POS           (0)  /* Z position of minute hand */
#define CIRCLE_CLOCK_HOUR_Z_POS             (1)  /* Z position of hour hand */

#define CIRCLE_CLOCK_CIRCLE_START_X_POS     (1)  /* x start position of outer circle */
#define CIRCLE_CLOCK_CIRCLE_END_X_POS       (12)  /* x end position of outer circle */

/* draw a circle on the outside */
/*!
 * \brief Draws the circular clock outline.
 */
void CC_DrawCircle(void);

/* draw rays outside */
/*!
 * \brief Draws the circular clock ray markers.
 */
void CC_DrawRays(void);

/* show time */
/*!
 * \brief Shows the given time on the circular clock.
 * \param hour Hour value to display.
 * \param minute Minute value to display.
 */
void CC_ShowTime(uint8_t hour, uint8_t minute);

/* disable/enable center clock during intermezzos */
/*!
 * \brief Enables or disables the center clock in the circular layout.
 * \param enable True to enable or turn on, false to disable or turn off.
 */
void CC_EnableCenterClock(bool enable);

/*!
 * \brief Deinitializes the circular clock module.
 */
void CC_Deinit(void);
/*!
 * \brief Initializes the circular clock module.
 */
void CC_Init(void);

#endif /* PL_CONFIG_HAS_CIRCLE_CLOCK */

#endif /* CIRCLECLOCK_H_ */
