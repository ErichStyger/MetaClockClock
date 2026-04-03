/*
 * buttons_config.h
 *
 * Copyright (c) 2021-2023, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BUTTONS_CONFIG_H_
#define BUTTONS_CONFIG_H_

#include "McuLib.h"
#include "platform.h"

#if McuLib_CONFIG_CPU_IS_KINETIS
  #define BUTTONS_PINS_NAVUP_GPIO      GPIOB
  #define BUTTONS_PINS_NAVUP_PORT      PORTB
  #define BUTTONS_PINS_NAVUP_PIN       3u

  #define BUTTONS_PINS_NAVDOWN_GPIO    GPIOB
  #define BUTTONS_PINS_NAVDOWN_PORT    PORTB
  #define BUTTONS_PINS_NAVDOWN_PIN     2u

  #define BUTTONS_PINS_NAVLEFT_GPIO    GPIOA
  #define BUTTONS_PINS_NAVLEFT_PORT    PORTA
  #define BUTTONS_PINS_NAVLEFT_PIN     13u

  #define BUTTONS_PINS_NAVRIGHT_GPIO   GPIOA
  #define BUTTONS_PINS_NAVRIGHT_PORT   PORTA
  #define BUTTONS_PINS_NAVRIGHT_PIN    12u

  #define BUTTONS_PINS_NAVCENTER_GPIO  GPIOB
  #define BUTTONS_PINS_NAVCENTER_PORT  PORTB
  #define BUTTONS_PINS_NAVCENTER_PIN   16u

  #define BUTTONS_ENABLE_CLOCK() CLOCK_EnableClock(kCLOCK_PortA); CLOCK_EnableClock(kCLOCK_PortB) /* enable clocking */
#elif McuLib_CONFIG_CPU_IS_LPC
   /* user button on LPC845-BRK board: PIO0_4 */
  #define BUTTONS_PINS_USER_GPIO            GPIO
  #define BUTTONS_PINS_USER_PORT            0
  #define BUTTONS_PINS_USER_PIN             4
  #define BUTTONS_PINS_USER_IOCON           IOCON_INDEX_PIO0_4
  #define BUTTONS_PINS_USER_PINTSEL         kSYSCON_GpioPort0Pin4ToPintsel

  #define BUTTONS_PINS_NAVUP_GPIO        GPIO
  #define BUTTONS_PINS_NAVUP_PORT        0
  #define BUTTONS_PINS_NAVUP_PIN         8u
  #define BUTTONS_PINS_NAVUP_IOCON       IOCON_INDEX_PIO0_8
  #define BUTTONS_PINS_NAVUP_PINTSEL     kSYSCON_GpioPort0Pin8ToPintsel

  #define BUTTONS_PINS_NAVDOWN_GPIO      GPIO
  #define BUTTONS_PINS_NAVDOWN_PORT      0
  #define BUTTONS_PINS_NAVDOWN_PIN       9u
  #define BUTTONS_PINS_NAVDOWN_IOCON     IOCON_INDEX_PIO0_9
  #define BUTTONS_PINS_NAVDOWN_PINTSEL   kSYSCON_GpioPort0Pin9ToPintsel

  #define BUTTONS_PINS_NAVLEFT_GPIO      GPIO
  #define BUTTONS_PINS_NAVLEFT_PORT      0
  #define BUTTONS_PINS_NAVLEFT_PIN       29u
  #define BUTTONS_PINS_NAVLEFT_IOCON     IOCON_INDEX_PIO0_29
  #define BUTTONS_PINS_NAVLEFT_PINTSEL   kSYSCON_GpioPort0Pin29ToPintsel

  #define BUTTONS_PINS_NAVRIGHT_GPIO     GPIO
  #define BUTTONS_PINS_NAVRIGHT_PORT     0
  #define BUTTONS_PINS_NAVRIGHT_PIN      28u
  #define BUTTONS_PINS_NAVRIGHT_IOCON    IOCON_INDEX_PIO0_28
  #define BUTTONS_PINS_NAVRIGHT_PINTSEL  kSYSCON_GpioPort0Pin28ToPintsel

  #define BUTTONS_PINS_NAVCENTER_GPIO    GPIO
  #define BUTTONS_PINS_NAVCENTER_PORT    0
  #define BUTTONS_PINS_NAVCENTER_PIN     27u
  #define BUTTONS_PINS_NAVCENTER_IOCON   IOCON_INDEX_PIO0_27
  #define BUTTONS_PINS_NAVCENTER_PINTSEL kSYSCON_GpioPort0Pin27ToPintsel

  #define BUTTONS_ENABLE_CLOCK() GPIO_PortInit(GPIO, 0) /* ungate the clocks for GPIO0: used for user button */
#elif PL_CONFIG_HW_VERSION==PL_CONFIG_HW_ADIS_PICO_W_CONSOLE_V0_1
  #define BUTTONS_PINS_NAVUP_PIN         15u
  #define BUTTONS_PINS_NAVDOWN_PIN       14u
  #define BUTTONS_PINS_NAVLEFT_PIN       13u
  #define BUTTONS_PINS_NAVRIGHT_PIN      12u
  #define BUTTONS_PINS_NAVCENTER_PIN     11u

  #define BUTTONS_ENABLE_CLOCK() /* nothing */
#elif PL_CONFIG_HW_VERSION==PL_CONFIG_HW_ADIS_PICO_W_CONSOLE_V0_2
  #define BUTTONS_PINS_NAVUP_PIN         14u
  #define BUTTONS_PINS_NAVDOWN_PIN       15u
  #define BUTTONS_PINS_NAVLEFT_PIN       28u
  #define BUTTONS_PINS_NAVRIGHT_PIN      26u
  #define BUTTONS_PINS_NAVCENTER_PIN     27u

  #define BUTTONS_ENABLE_CLOCK() /* nothing */
#elif McuLib_CONFIG_CPU_IS_ESP32
  /* ESP32: Only pins that support both input & output have integrated pull-up and pull-down resistors. Input-only GPIOs 34-39 do not. */
  #define BUTTONS_PINS_NAVUP_PIN         GPIO_NUM_25
  #define BUTTONS_PINS_NAVDOWN_PIN       GPIO_NUM_39  /* hardware bug: with WiFi enabled, it triggers an interrupt if using interrupts! */
  #define BUTTONS_PINS_NAVLEFT_PIN       GPIO_NUM_35
  #define BUTTONS_PINS_NAVRIGHT_PIN      GPIO_NUM_36  /* hardware bug: with WiFi enabled, it triggers an interrupt if using interrupts! */
  #define BUTTONS_PINS_NAVCENTER_PIN     GPIO_NUM_34
  #define BUTTONS_ENABLE_CLOCK()         /* nothing */
#endif

typedef enum {
  BTN_NAV_UP,
  BTN_NAV_DOWN,
  BTN_NAV_LEFT,
  BTN_NAV_RIGHT,
  BTN_NAV_CENTER,
#if McuLib_CONFIG_CPU_IS_LPC
  BTN_USER,
#endif
  BTN_NOF_BUTTONS  /* sentinel, must be last in list! */
} BTN_Buttons_e;

/* bits of the buttons */
  #define BTN_BIT_NAV_UP        (1<<0)
  #define BTN_BIT_NAV_DOWN      (1<<1)
  #define BTN_BIT_NAV_LEFT      (1<<2)
  #define BTN_BIT_NAV_RIGHT     (1<<3)
  #define BTN_BIT_NAV_CENTER    (1<<4)
#if McuLib_CONFIG_CPU_IS_LPC
  #define BTN_BIT_USER          (1<<5)
#endif

#define BTN_CONFIG_ROTATION  (0) /* 180 for rotated nav 180 degree */

#endif /* BUTTONS_CONFIG_H_ */
