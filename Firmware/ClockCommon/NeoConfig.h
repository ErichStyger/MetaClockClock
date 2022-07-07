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
  #define NEOC_NOF_LEDS_IN_COL   (PL_CONFIG_NOF_DRIVER_BOARDS) /* have 5 rows with 16 LEDs each */
  #define NEOC_NOF_LEDS_IN_ROW   (8) /* number of LEDs in rows */
  #define NEOC_NOF_LEDS_IN_LANE  (NEOC_NOF_LEDS_IN_COL*NEOC_NOF_LEDS_IN_ROW) /* number of LEDs, all in one line */
#elif PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_MASTER_K22FN512
  #if PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_SMARTWALL_16x5
    #define NEOC_LANE_START        (0) /* this is the first GPIO pin used */
    #define NEOC_LANE_END          (0) /* this is the last GPIO pin used */
    #define NEOC_NOF_LEDS_IN_COL   (PL_CONFIG_NOF_DRIVER_BOARDS) /* have the rows (boards) stacked with 16 LEDs each*/
    #define NEOC_NOF_LEDS_IN_ROW   (16) /* each row/board has 16 LEDs */
    #define NEOC_NOF_LEDS_IN_LANE  (NEOC_NOF_LEDS_IN_COL*NEOC_NOF_LEDS_IN_ROW) /* number of LEDs, all in one line */
  #elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_12x5_60B /* 60 billion lights, with master behind the canvas driving the LEDs directly */
    #define NEOC_LANE_START        (0) /* this is the first GPIO pin used */
    #define NEOC_LANE_END          (4) /* this is the last GPIO pin used */
    #define NEOC_NOF_LEDS_IN_LANE  (12*40) /* number of LEDs in each lane */
  #elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_VERKEHRSHAUS
    #define NEOC_LANE_START        (0) /* this is the first GPIO pin used */
    #define NEOC_LANE_END          (0) /* this is the last GPIO pin used */
    #define NEOC_NOF_LEDS_IN_COL   (1) /* just one stripe */
    #define NEOC_NOF_LEDS_IN_ROW   (30) /* total number of leds in stripe */
    #define NEOC_NOF_LEDS_IN_LANE  (NEOC_NOF_LEDS_IN_COL*NEOC_NOF_LEDS_IN_ROW) /* number of LEDs, all in one line */
    #define NEOC_NOF_COLORS        (4)  /* 3 for RGB, 4 for RGBW */
  #else
    #error "unknown configuration?"
  #endif
#else
  #error "unknown"
#endif

#ifndef NEOC_NOF_COLORS
  #define NEOC_NOF_COLORS (3)  /* 3 for RGB, 4 for RGBW */
#endif

#if (NEOC_NOF_LANES>8)
  #error "can only handle up to 8 lanes (8bit port)"
#endif

#define NEOC_NOF_PIXEL   ((NEOC_LANE_END+1-NEOC_LANE_START)*(NEOC_NOF_LEDS_IN_LANE)) /* number of pixels */

#endif /* PL_CONFIG_USE_NEO_PIXEL_HW */

#endif /* NEOCONFIG_H_ */
