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

int main(void) {
  /* Init board hardware. */
  BOARD_InitBootPins();
  BOARD_InitBootClocks();
  BOARD_InitBootPeripherals();

  //void LP_Test(void);
  //LP_Test();
  APP_Run();
  return 0;
}

