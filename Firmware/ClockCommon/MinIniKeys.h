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
  #define NVMC_MININI_KEY_CLOCK_ON                  "on"              /* bool, 0: on, 1: off */
  #define NVMC_MININI_KEY_CLOCK_ON_OFF              "onoff"           /* bool, 0: on/off disabled, 1: on/off enabled */
  #define NVMC_MININI_KEY_CLOCK_OFF_START_HH        "offStartHH"      /* off start hour, e.g. 15 */
  #define NVMC_MININI_KEY_CLOCK_OFF_START_MM        "offStartMM"      /* off start minute, e.g. 0 */
  #define NVMC_MININI_KEY_CLOCK_OFF_END_HH          "offEndHH"        /* off end hour, e.g. 5 */
  #define NVMC_MININI_KEY_CLOCK_OFF_END_MM          "offEndMM"        /* off end minute, e.g. 15 */
  #define NVMC_MININI_KEY_CLOCK_OFF_DAYS            "offDays"         /* off days as bitset, with bit 0 (0x1) as Sunday, bit 1 Monday, ... */
  #define NVMC_MININI_KEY_CLOCK_FONT                "font"            /* "2x3" or "3x5" */
  #define NVMC_MININI_KEY_CLOCK_RANDOM_HAND_COLOR   "randomHandColor" /* bool, 0: on, 1: off */
  #define NVMC_MININI_KEY_CLOCK_FADING_HANDS        "fadingHands"     /* bool, 0: on, 1: off */
#endif

#if PL_CONFIG_USE_INTERMEZZO
#define NVMC_MININI_SECTION_INTERMEZZO          "Intermezzo"
  #define NVMC_MININI_KEY_INTERMEZZO_ON             "on"              /* bool, 0: off, 1: on */
  #define NVMC_MININI_KEY_RTC_TEMP_OFFSET           "rtcTempOffset"   /* integer, temperature offset in deci-celcius degree */
  #define NVMC_MININI_KEY_INTERMEZZO_DISABLED_0     "disabled0"        /* integer, 32bits as bitset 0-31 of disabled intermezzos */
  #define NVMC_MININI_KEY_INTERMEZZO_DISABLED_1     "disabled1"        /* integer, 32bits as bitset 32-63 of disabled intermezzos */
#endif

#endif /* MININIKEYS_H_ */
