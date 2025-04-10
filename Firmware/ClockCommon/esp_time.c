/*
 * Copyright (c) 20251, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_ESP_TIME
#include "esp_time.h"

uint8_t EspTime_GetTimeDate(TIMEREC *time, DATEREC *date) {
  if (time!=NULL) {

  }
  if (date!=NULL) {

  }
  return ERR_FAILED;
}

void EspTime_Deinit(void) {
}

void EspTime_Init(void) {
}

#endif
