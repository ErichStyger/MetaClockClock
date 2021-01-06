/*
 * pixel.h
 *
 * Copyright (c) 2020: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#ifndef PIXEL_H_
#define PIXEL_H_

#include <stdint.h>
#include <stdbool.h>
#include "McuShell.h"

uint8_t PIXEL_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

void PIXEL_Init(void);

#endif /* PIXEL_H_ */
