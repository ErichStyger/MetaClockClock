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
  #define NEOC_LANE_START        (5)  /* this is the first GPIO pin used */
  #define NEOC_LANE_END          (6)  /* this is the last GPIO pin used */
  #define NEOC_NOF_LEDS_IN_LANE  (40) /* number of LEDs in a lane */
#elif PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_PIXELUNIT_K02FN64 || PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_PIXELUNIT_K02FN128
  #define NEOC_LANE_START        (5) /* this is the first GPIO pin used */
  #define NEOC_LANE_END          (5) /* this is the last GPIO pin used */
  #define NEOC_NOF_LEDS_IN_LANE  (2*8) /* number of LEDs in a lane */
#elif PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_MASTER_K22FN512
  #if PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_SMARTWALL_16x5
    #define NEOC_LANE_START        (0) /* this is the first GPIO pin used */
    #define NEOC_LANE_END          (0) /* this is the last GPIO pin used */
    #define NEOC_NOF_LEDS_IN_LANE  (PL_CONFIG_NOF_DRIVER_BOARDS*16) /* number of LEDs in each lane */
  #else
    #define NEOC_LANE_START        (0) /* this is the first GPIO pin used */
    #define NEOC_LANE_END          (4) /* this is the last GPIO pin used */
    #define NEOC_NOF_LEDS_IN_LANE  (12*40) /* number of LEDs in each lane */
  #endif
#else
  #error "unknown"
#endif

#if (NEOC_NOF_LANES>8)
  #error "can only handle up to 8 lanes (8bit port)"
#endif

#define NEOC_NOF_PIXEL   ((NEOC_LANE_END+1-NEOC_LANE_START)*(NEOC_NOF_LEDS_IN_LANE)) /* number of pixels */

#endif /* PL_CONFIG_USE_NEO_PIXEL_HW */

#endif /* NEOCONFIG_H_ */
