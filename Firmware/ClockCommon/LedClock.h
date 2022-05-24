/*
 * Copyright (c) 2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LEDCLOCK_H_
#define LEDCLOCK_H_

#include "McuTimeDate.h"
#include "McuShell.h"

uint8_t LedClock_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

void LedClock_ShowTimeDate(TIMEREC *time, DATEREC *date);

void LedClock_Deinit(void);

void LedClock_Init(void);

#endif /* LEDCLOCK_H_ */
