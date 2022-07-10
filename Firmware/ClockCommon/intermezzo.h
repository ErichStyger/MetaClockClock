/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERMEZZO_H_
#define INTERMEZZO_H_

#include <stdbool.h>
#include "McuShell.h"
#include "McuRTOS.h"

uint8_t INTERMEZZO_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

void INTERMEZZO_Play(TickType_t lastClockUpdateTickCount, bool *intermezzoShown);

void INTERMEZZO_PlaySpecific(uint8_t nr);

bool INTERMEZZO_IsOn(void);

void INTERMEZZO_Init(void);

#endif /* INTERMEZZO_H_ */
