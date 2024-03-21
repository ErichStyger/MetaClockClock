/*
 * Copyright (c) 2022-2024, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MININIKEYS_H_
#define MININIKEYS_H_

/* strings and names used in minINI configuration file */

#include "platform.h"

#define NVMC_MININI_FILE_NAME       "settings.ini" /* 'file' name used */

#if PL_CONFIG_USE_CLOCK
  #define NVMC_MININI_SECTION_CLOCK             "Clock"
  #define NVMC_MININI_KEY_CLOCK_ON                  "on"          /* bool, 0: on, 1: off */
  #define NVMC_MININI_KEY_CLOCK_ON_OFF              "onoff"       /* bool, 0: on/off disabled, 1: on/off enabled */
  #define NVMC_MININI_KEY_CLOCK_OFF_START_HH        "offStartHH"  /* off start hour, e.g. 15 */
  #define NVMC_MININI_KEY_CLOCK_OFF_START_MM        "offStartMM"  /* off start minute, e.g. 0 */
  #define NVMC_MININI_KEY_CLOCK_OFF_END_HH          "offEndHH"    /* off end hour, e.g. 5 */
  #define NVMC_MININI_KEY_CLOCK_OFF_END_MM          "offEndMM"    /* off end minute, e.g. 15 */
#endif

#if PL_CONFIG_USE_INTERMEZZO
#define NVMC_MININI_SECTION_INTERMEZZO          "Intermezzo"
  #define NVMC_MININI_KEY_INTERMEZZO_ON             "on"       /* bool, 0: off, 1: on */
#endif

#endif /* MININIKEYS_H_ */
