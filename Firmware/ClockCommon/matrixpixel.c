/*
 * matrixpixel.c
 *
 * Copyright (c) 2020: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_LED_PIXEL
#include "matrixpixel.h"
#include "matrix.h"
#include "NeoPixel.h"
#include "assert.h"

void MPIXEL_ClearAll(void) {
  NEO_ClearAllPixel();
}

/* LEDS start from the left lower corner, left to right, bottom to top:
 *   64 65 ....       79
 *   48 ..            63
 *   32 ...           47
 *   16 17 18 ....... 31
 *   0 1 2 3 ........ 15
 *   x/y has its origin in the left top corner
 */
void MPIXEL_SetColor(int32_t x, int32_t y, int32_t z, uint8_t red, uint8_t green, uint8_t blue) {
  #if PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_SMARTWALL_16x5
    int32_t pos;

    /* assumption: all boards stacked together in y direction */
    pos = (MPIXEL_NOF_X*MPIXEL_NOF_Y)-MPIXEL_NOF_X; /* zero point left top corner */
    pos -= y*MPIXEL_NOF_X; /* add y row offset */
    pos += x%MPIXEL_NOF_X; /* add x column offset */
    NEO_SetPixelRGB(NEOC_LANE_START, pos, red, green, blue); /* pixel one */
  #else
    /* \todo assuming 8x2 pixels in line */
    NEO_SetPixelRGB(NEOC_LANE_START, NEOC_NOF_LEDS_IN_LANE-1-x*2, red, green, blue); /* pixel one */
    NEO_SetPixelRGB(NEOC_LANE_START, NEOC_NOF_LEDS_IN_LANE-1-(x*2)-1, red, green, blue); /* pixel two */
  #endif
}
uint32_t MPIXEL_GetColor(int32_t x, int32_t y, int32_t z) {
	uint8_t red, green, blue;
  int32_t pos;
#if PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_SMARTWALL_16x5
  /* assumption: all boards stacked together in y direction */
  pos = (MPIXEL_NOF_X*MPIXEL_NOF_Y)-MPIXEL_NOF_X; /* zero point left top corner */
  pos -= y*MPIXEL_NOF_X; /* add y row offset */
  pos += x%MPIXEL_NOF_X; /* add x column offset */
#else
  /* \todo assuming 8x2 pixels in line */
  pos = x + 2*y;
#endif
  NEO_GetPixelRGB(NEOC_LANE_START, pos, &red, &green, &blue);
  return NEO_COMBINE_RGB(red, green, blue);
}

#endif /* PL_CONFIG_USE_LED_PIXEL */
