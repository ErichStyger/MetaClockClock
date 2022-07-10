/*
 * matrixpixel.h
 *
 * Copyright (c) 2020: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#ifndef MATRIXPIXEL_H_
#define MATRIXPIXEL_H_

#include "platform.h"
#include <stdint.h>

#if PL_CONFIG_USE_LED_PIXEL
  #include "NeoConfig.h"

  #define MPIXEL_NOF_X    (NEOC_NOF_LEDS_IN_ROW)
  #define MPIXEL_NOF_Y    (NEOC_NOF_LEDS_IN_COL)

  void MPIXEL_SetColor(int32_t x, int32_t y, int32_t z, uint8_t red, uint8_t green, uint8_t blue);

  uint32_t MPIXEL_GetColor(int32_t x, int32_t y, int32_t z);

  void MPIXEL_ClearAll(void);
#endif

#endif /* MATRIXPIXEL_H_ */
