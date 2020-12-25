/*
 * matrixring.h
 *
 * Author: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#ifndef MATRIXRING_H_
#define MATRIXRING_H_

#include <stdint.h>
#include <stdbool.h>

#if PL_CONFIG_USE_LED_RING

void MRING_SetRingPixelColor(int32_t x, int32_t y, uint8_t pos, uint8_t red, uint8_t green, uint8_t blue);
void MRING_SetRingColor(int32_t x, int32_t y, int32_t z, uint8_t red, uint8_t green, uint8_t blue);
void MRING_SetRingColorAll(uint8_t red, uint8_t green, uint8_t blue);

void MRING_EnableRing(int32_t x, int32_t y, uint8_t z, bool on);
void MRING_EnableRingAll(bool on);

#endif

#endif /* MATRIXRING_H_ */
