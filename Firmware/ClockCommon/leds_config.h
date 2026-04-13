/*
 * Copyright (c) 2023, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LEDS_CONFIG_H_
#define LEDS_CONFIG_H_

#include "platform.h"

#if PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_MASTER_K22FN512 /* tinyK22 */ \
   || PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN64 /* clock board */ \
   || PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN128 /* clock board */
  #define LEDS_CONFIG_HAS_ONBOARD_LED        (1)
  #define LEDS_CONFIG_HAS_RED_LED            (0)
  #define LEDS_CONFIG_HAS_GREEN_LED          (0)
  #define LEDS_CONFIG_HAS_BLUE_LED           (0)
  #define LEDS_CONFIG_HAS_ORANGE_LED         (0)
#elif McuLib_CONFIG_CPU_IS_LPC /* LPC845 based boards */
  #define LEDS_CONFIG_HAS_ONBOARD_LED        (1)
  #define LEDS_CONFIG_HAS_RED_LED            (0)
  #define LEDS_CONFIG_HAS_GREEN_LED          (0)
  #define LEDS_CONFIG_HAS_BLUE_LED           (0)
  #define LEDS_CONFIG_HAS_ORANGE_LED         (0)
#elif PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_MASTER_PICO_W
  #define LEDS_CONFIG_HAS_ONBOARD_LED        (1)
  #define LEDS_CONFIG_HAS_RED_LED            (0)
  #define LEDS_CONFIG_HAS_GREEN_LED          (0)
  #define LEDS_CONFIG_HAS_BLUE_LED           (0)
  #define LEDS_CONFIG_HAS_ORANGE_LED         (0)
#endif

#if   PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_MASTER_K22FN512 /* tinyK22 */ \
   || PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN64 /* clock board */ \
   || PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN128 /* clock board */
  #define LEDS_CONFIG_ENABLE_CLOCK()    /* nothing */

  #if LEDS_CONFIG_HAS_ONBOARD_LED
    /* blue onboard LED on PTC2 */
    #define LEDS_CONFIG_ONBOARD_GPIO             (GPIOC)
    #define LEDS_CONFIG_ONBOARD_PORT             (PORTC)
    #define LEDS_CONFIG_ONBOARD_PIN              (2U)
    #define LEDS_CONFIG_ONBOARD_LOW_ACTIVE       (0)
  #endif
#elif PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_MASTER_PICO_W
  #define LEDS_CONFIG_ENABLE_CLOCK()    /* nothing */

  #if LEDS_CONFIG_HAS_ONBOARD_LED
    #if !PL_CONFIG_USE_PICO_W
      #define LEDS_CONFIG_ONBOARD_PIN          25 /* only for non-W! */
      #define LEDS_CONFIG_ONBOARD_LOW_ACTIVE   false
    #endif
  #endif
#elif McuLib_CONFIG_CPU_IS_LPC /* LPC845 based boards */
  #define LEDS_CONFIG_ENABLE_CLOCK()    /* nothing */
  #if PL_CONFIG_IS_CLIENT /* blue led */
    #define LEDS_CONFIG_ONBOARD_GPIO       GPIO
    #define LEDS_CONFIG_ONBOARD_PORT       0U
    #define LEDS_CONFIG_ONBOARD_PIN        19U
    #define LEDS_CONFIG_ONBOARD_IOCON      IOCON_INDEX_PIO0_19
    #define LEDS_CONFIG_ONBOARD_LOW_ACTIVE   false
  #elif PL_CONFIG_IS_MASTER
    /* LEDs on LPC845-BRK */
    #define LEDS_CONFIG_ONBOARD_GPIO      GPIO
    #define LEDS_CONFIG_ONBOARD_PORT      1U
    #define LEDS_CONFIG_ONBOARD_PIN       0U
    #define LEDS_CONFIG_ONBOARD_IOCON     IOCON_INDEX_PIO1_0
    #define LEDS_CONFIG_ONBOARD_LOW_ACTIVE   false
  #endif
#endif

#endif /* LEDS_CONFIG_H_ */
