/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "application.h"
#include "clock_config.h"
#include "pin_mux.h"
#include "peripherals.h"

#if 0
#include "McuTimeDate.h"
#include "fsl_pit.h"

#define PIT_BASEADDR       PIT
#define PIT_SOURCE_CLOCK   CLOCK_GetFreq(kCLOCK_BusClk)
#define PIT_CHANNEL        kPIT_Chnl_0
#define PIT_HANDLER        PIT0_IRQHandler
#define PIT_IRQ_ID         PIT0_IRQn

void PIT_HANDLER(void) {
  PIT_ClearStatusFlags(PIT_BASEADDR, PIT_CHANNEL, kPIT_TimerFlag);
  McuTimeDate_AddTick();
  __DSB();
}

static void AppConfigureTimer(void) {
  pit_config_t config;

  PIT_GetDefaultConfig(&config);
  config.enableRunInDebug = false;
  PIT_Init(PIT_BASEADDR, &config);
  PIT_SetTimerPeriod(PIT_BASEADDR, PIT_CHANNEL, USEC_TO_COUNT(McuTimeDate_CONFIG_TICK_TIME_MS*1000, PIT_SOURCE_CLOCK));
  PIT_EnableInterrupts(PIT_BASEADDR, PIT_CHANNEL, kPIT_TimerInterruptEnable);
  NVIC_SetPriority(PIT_IRQ_ID, 0);
  EnableIRQ(PIT_IRQ_ID);
  PIT_StartTimer(PIT_BASEADDR, PIT_CHANNEL);
}
#endif

int main(void) {
  /* Init board hardware. */
  BOARD_InitBootPins();
  BOARD_InitBootClocks();
  BOARD_InitBootPeripherals();

  //AppConfigureTimer();
  APP_Run();
  return 0;
}
