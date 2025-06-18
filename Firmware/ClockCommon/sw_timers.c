/*
 * Copyright (c) 2025, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sw_timers.h"
#include "McuRTOS.h"
#include "McuLog.h"
#include "McuTimeDate.h"

static void vTimerCallbackTimeDate(TimerHandle_t pxTimer) {
  McuTimeDate_AddTick();
}

void Timers_Init(void) {
  TimerHandle_t timeDateTimerHandle; /* timer for McuTimeDate update */
  timeDateTimerHandle = xTimerCreate(
        "timeDate", /* name */
        pdMS_TO_TICKS(McuTimeDate_CONFIG_TICK_TIME_MS), /* period/time */
        pdTRUE, /* auto reload */
        (void*)0, /* timer ID */
        vTimerCallbackTimeDate); /* callback */
  if (timeDateTimerHandle==NULL) {
    McuLog_fatal("failed creating timer");
    for(;;); /* failure! */
  }
  xTimerStart(timeDateTimerHandle, pdMS_TO_TICKS(100));
}
