/*
 * matrixring.c
 *
 * Copyright (c) 2020: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "matrix.h"
#include "matrixring.h"
#include "NeoPixel.h"
#include <assert.h>

#if PL_CONFIG_USE_LED_RING

void MRING_SetRingPixelColor(int32_t x, int32_t y, uint8_t pos, uint8_t red, uint8_t green, uint8_t blue) {
#if PL_CONFIG_USE_NEO_PIXEL_HW
  NEOSR_SetRingPixelColor(MATRIX_GetLedRingDevice(x, y, 0), pos, red, green, blue);
#endif
}

void MRING_SetRingColor(int32_t x, int32_t y, int32_t z, uint8_t red, uint8_t green, uint8_t blue) {
  assert(x<MATRIX_NOF_STEPPERS_X && y<MATRIX_NOF_STEPPERS_Y && z<MATRIX_NOF_STEPPERS_Z);
#if PL_CONFIG_USE_NEO_PIXEL_HW
  NEOSR_SetRingColor(MATRIX_GetLedRingDevice(x, y, z), red, green, blue);
#elif PL_CONFIG_IS_MASTER && PL_MATRIX_CONFIG_IS_RGB
  matrix.colorRingMap[x][y][z] = NEO_COMBINE_RGB(red, green, blue);
#endif
}

void MRING_SetRingColorAll(uint8_t red, uint8_t green, uint8_t blue) {
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
        MRING_SetRingColor(x, y, z, red, green, blue);
      }
    }
  }
}

void MRING_EnableRing(int32_t x, int32_t y, uint8_t z, bool on) {
  assert(x<MATRIX_NOF_STEPPERS_X && y<MATRIX_NOF_STEPPERS_Y && z<MATRIX_NOF_STEPPERS_Z);
#if PL_CONFIG_USE_NEO_PIXEL_HW
  NEOSR_SetRingLedEnabled(MATRIX_GetLedRingDevice(x, y, z), on);
#elif PL_CONFIG_IS_MASTER && PL_MATRIX_CONFIG_IS_RGB
  matrix.enabledRingMap[x][y][z] = on;
#endif
}

void MRING_EnableRingAll(bool on) {
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
        MRING_EnableRing(x, y, z, on);
      }
    }
  }
}

#endif /* PL_CONFIG_USE_LED_RING */

