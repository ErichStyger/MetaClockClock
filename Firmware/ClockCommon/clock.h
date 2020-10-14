/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLOCK_H_
#define CLOCK_H_

#include <stdbool.h>
#include "McuShell.h"

uint8_t CLOCK_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

bool CLOCK_GetClockIsOn(void);

void CLOCK_Init(void);

#endif /* CLOCK_H_ */
