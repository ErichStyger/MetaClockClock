/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LOWPOWER_H_
#define LOWPOWER_H_

#include "LowPowerconfig.h"

/*!
 * \brief LP_OnActivateFromISR function.
 */
void LP_OnActivateFromISR(void);

/*!
 * \brief LP_EnterWaitMode function.
 */
void LP_EnterWaitMode(void);
/*!
 * \brief LP_EnterStopMode function.
 */
void LP_EnterStopMode(void);

/*!
 * \brief Initializes the module.
 */
void LP_Init(void);

#endif /* LOWPOWER_H_ */
