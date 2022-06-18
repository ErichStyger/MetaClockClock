/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LOWPOWER_H_
#define LOWPOWER_H_

#include "LowPowerconfig.h"

void LP_OnActivateFromISR(void);

void LP_EnterWaitMode(void);
void LP_EnterStopMode(void);

void LP_Init(void);

#endif /* LOWPOWER_H_ */
