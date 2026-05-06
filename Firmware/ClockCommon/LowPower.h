/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LOWPOWER_H_
#define LOWPOWER_H_

#include "LowPowerconfig.h"

/*!
 * \brief Handles low-power activation from an interrupt service routine.
 */
void LP_OnActivateFromISR(void);

/*!
 * \brief Enters the MCU wait low-power mode.
 */
void LP_EnterWaitMode(void);
/*!
 * \brief Enters the MCU stop low-power mode.
 */
void LP_EnterStopMode(void);

/*!
 * \brief Initializes low-power mode support.
 */
void LP_Init(void);

#endif /* LOWPOWER_H_ */
