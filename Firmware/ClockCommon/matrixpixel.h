/*
 * matrixpixel.h
 *
 * Author: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#ifndef MATRIXPIXEL_H_
#define MATRIXPIXEL_H_

#include "platform.h"
#include <stdint.h>

#if PL_CONFIG_USE_LED_PIXEL
void MPIXEL_SetColor(int32_t x, int32_t y, int32_t z, uint8_t red, uint8_t green, uint8_t blue);
#endif

#endif /* MATRIXPIXEL_H_ */
