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

  #define BUTTONS_ENABLE_CLOCK() GPIO_PortInit(GPIO, 0) /* ungate the clocks for GPIO0: used for user button */
#endif

typedef enum {
  BTN_USER,
  BTN_NOF_BUTTONS  /* sentinel, must be last in list! */
} BTN_Buttons_e;

/* bits of the buttons */
#define BTN_BIT_USER          (1<<0)

#endif /* BUTTONS_CONFIG_H_ */
