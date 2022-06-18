/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "application.h"
#include "clock_config.h"
#include "pin_mux.h"
#include "peripherals.h"

void McuEvents_AppHandleEvent(uint8_t event) {
  /* used if -lto? */
}

#if McuUart485_CONFIG_UART_ISR_HOOK_ENABLED
#include "McuLED.h"
#include "leds.h"
#include "LowPower.h"

void McuUart485_CONFIG_UART_ISR_HOOK_NAME(void) {
  McuLED_Toggle(LEDS_Led);
  LP_OnActivateFromISR();
}
#endif

int main(void) {
  /* Init board hardware. */
  BOARD_InitBootPins();
  BOARD_InitBootClocks();
  BOARD_InitBootPeripherals();
  APP_Run();
  return 0;
}
