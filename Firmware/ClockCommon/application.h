/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "platform.h"
#include "McuShell.h"

uint8_t APP_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

#if PL_CONFIG_USE_NEO_PIXEL_HW
bool APP_RequestUpdateLEDsFromISR(void);
void APP_RequestUpdateLEDs(void);
#endif

void APP_Run(void);

#endif /* APPLICATION_H_ */
