/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLOCK_H_
#define CLOCK_H_

#include <stdbool.h>
#include "McuShell.h"

typedef enum CLOCK_Mode_e {
  CLOCK_MODE_ON,
  CLOCK_MODE_OFF,
  CLOCK_MODE_TOGGLE,
} CLOCK_Mode_e;

void CLOCK_Park(CLOCK_Mode_e mode);
void CLOCK_On(CLOCK_Mode_e mode);

uint8_t CLOCK_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

bool CLOCK_GetClockIsOn(void);

void CLOCK_Init(void);

#endif /* CLOCK_H_ */
