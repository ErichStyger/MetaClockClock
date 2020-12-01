/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEOCONFIG_H_
#define NEOCONFIG_H_

#include "platform.h"

#if PL_CONFIG_USE_NEO_PIXEL_HW

#if PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN64 || PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN128
  #define NEOC_LANE_START        (5)
  #define NEOC_LANE_END          (6)
  #define NEOC_NOF_LANES         (7) /* \todo remove? */
  #define NEOC_NOF_LEDS_IN_LANE  (40) /* number of LEDs in each lane */
#elif PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_PIXELUNIT_K02FN64 || PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_PIXELUNIT_K02FN128
  #define NEOC_LANE_START        (5)
  #define NEOC_LANE_END          (5)
  #define NEOC_NOF_LANES         (6) /* \todo remove? */
  #define NEOC_NOF_LEDS_IN_LANE  (2*8) /* number of LEDs in each lane */
#else
  #if PL_MATRIX_CONFIG_IS_8x3
    #define NEOC_NOF_LANES         (3) /* number of data lanes. For a matrix it is assumed that the number of pixels are divided to the available lanes! */
    #define NEOC_NOF_LEDS_IN_LANE  (8*40) /* number of LEDs in each lane */
  #elif PL_MATRIX_CONFIG_IS_12x5
    #define NEOC_NOF_LANES         (5) /* number of data lanes. For a matrix it is assumed that the number of pixels are divided to the available lanes! */
    #define NEOC_NOF_LEDS_IN_LANE  (12*40) /* number of LEDs in each lane */
  #else
    #error "unknown"
  #endif
  #define NEOC_LANE_START (0)
  #define NEOC_LANE_END   (NEOC_NOF_LANES-1)
#endif

#define NEOC_NOF_PIXEL   ((NEOC_LANE_END+1-NEOC_LANE_START)*(NEOC_NOF_LEDS_IN_LANE)) /* number of pixels */

#endif /* PL_CONFIG_USE_NEO_PIXEL_HW */

#endif /* NEOCONFIG_H_ */
