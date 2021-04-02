/*
 * buttons_config.h
 *
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef BUTTONS_CONFIG_H_
#define BUTTONS_CONFIG_H_

#include "McuLib.h"

/* button pins are defined in IncludeMcuLibConfig.h */
#if McuLib_CONFIG_CPU_IS_LPC
   /* user button on LPC845-BRK board: PIO0_4 */
  #define BUTTONS_USER_GPIO       GPIO
  #define BUTTONS_USER_PORT       0
  #define BUTTONS_USER_PIN        4
  #define BUTTONS_USER_IOCON      IOCON_INDEX_PIO0_4
  #define BUTTONS_USER_PINTSEL    kSYSCON_GpioPort0Pin4ToPintsel

#if PL_CONFIG_HAS_SWITCH_7WAY
  #define BUTTONS_UP_GPIO         GPIO
  #define BUTTONS_UP_PORT         0
  #define BUTTONS_UP_PIN          8
  #define BUTTONS_UP_IOCON        IOCON_INDEX_PIO0_8
  #define BUTTONS_UP_PINTSEL      kSYSCON_GpioPort0Pin8ToPintsel

  #define BUTTONS_DOWN_GPIO       GPIO
  #define BUTTONS_DOWN_PORT       0
  #define BUTTONS_DOWN_PIN        9
  #define BUTTONS_DOWN_IOCON      IOCON_INDEX_PIO0_9
  #define BUTTONS_DOWN_PINTSEL    kSYSCON_GpioPort0Pin9ToPintsel

  #define BUTTONS_LEFT_GPIO       GPIO
  #define BUTTONS_LEFT_PORT       0
  #define BUTTONS_LEFT_PIN        29
  #define BUTTONS_LEFT_IOCON      IOCON_INDEX_PIO0_29
  #define BUTTONS_LEFT_PINTSEL    kSYSCON_GpioPort0Pin29ToPintsel

  #define BUTTONS_RIGHT_GPIO      GPIO
  #define BUTTONS_RIGHT_PORT      0
  #define BUTTONS_RIGHT_PIN       28
  #define BUTTONS_RIGHT_IOCON     IOCON_INDEX_PIO0_28
  #define BUTTONS_RIGHT_PINTSEL   kSYSCON_GpioPort0Pin28ToPintsel

  #define BUTTONS_MID_GPIO        GPIO
  #define BUTTONS_MID_PORT        0
  #define BUTTONS_MID_PIN         27
  #define BUTTONS_MID_IOCON       IOCON_INDEX_PIO0_27
  #define BUTTONS_MID_PINTSEL     kSYSCON_GpioPort0Pin27ToPintsel

  #define BUTTONS_SET_GPIO        GPIO
  #define BUTTONS_SET_PORT        0
  #define BUTTONS_SET_PIN         26
  #define BUTTONS_SET_IOCON       IOCON_INDEX_PIO0_26
  #define BUTTONS_SET_PINTSEL     kSYSCON_GpioPort0Pin28ToPintsel

  #define BUTTONS_RST_GPIO        GPIO
  #define BUTTONS_RST_PORT        0
  #define BUTTONS_RST_PIN         23
  #define BUTTONS_RST_IOCON       IOCON_INDEX_PIO0_23
  #define BUTTONS_RST_PINTSEL     kSYSCON_GpioPort0Pin23ToPintsel
#endif

  #define BUTTONS_ENABLE_CLOCK() GPIO_PortInit(GPIO, 0) /* ungate the clocks for GPIO0 (Port 0): used for user button */
#endif

typedef enum {
  BTN_USER,
#if PL_CONFIG_HAS_SWITCH_7WAY
  BTN_UP,
  BTN_DOWN,
  BTN_LEFT,
  BTN_RIGHT,
  BTN_MID,
  BTN_SET,
  BTN_RST,
#endif
  BTN_NOF_BUTTONS  /* sentinel, must be last in list! */
} BTN_Buttons_e;

/* bits of the buttons */
#define BTN_BIT_USER          (1<<0)
#if PL_CONFIG_HAS_SWITCH_7WAY
  #define BTN_BIT_UP          (1<<1)
  #define BTN_BIT_DOWN        (1<<2)
  #define BTN_BIT_LEFT        (1<<3)
  #define BTN_BIT_RIGHT       (1<<4)
  #define BTN_BIT_MID         (1<<5)
  #define BTN_BIT_SET         (1<<6)
  #define BTN_BIT_RST         (1<<7)
#endif

#endif /* BUTTONS_CONFIG_H_ */
