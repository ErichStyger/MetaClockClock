/*
 * Copyright (c) 2021, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_TIME_DATE
#include "timer.h"
#include "McuRTOS.h"
#include "McuLog.h"
#include "McuTimeDate.h"

static TimerHandle_t timer;

static void timerCallback(TimerHandle_t xTimer) {
  McuTimeDate_AddTick();
}

void TMR_Init(void) {
  /* create auto-reload timer to update software RTC */
  timer = xTimerCreate("timer", pdMS_TO_TICKS(McuTimeDate_CONFIG_TICK_TIME_MS), pdTRUE, NULL, timerCallback);
  if (timer==NULL) {
    McuLog_fatal("Failed creating timer"); //  GCOVR_EXCL_LINE
    return;                                //  GCOVR_EXCL_LINE
  }
  xTimerStart(timer, 0);
}
#endif /* PL_CONFIG_USE_TIME_DATE */
