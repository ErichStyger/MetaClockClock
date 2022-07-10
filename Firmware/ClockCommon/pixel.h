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

void PIXEL_Zero(uint8_t x, uint8_t y, uint8_t z);

void PIXEL_ZeroAll(void);

uint8_t PIXEL_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

void PIXEL_Init(void);

#endif /* PIXEL_H_ */
