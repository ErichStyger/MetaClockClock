/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "application.h"
#include "clock_config.h"
#include "pin_mux.h"
#include "peripherals.h"

int main(void) {
  /* Init board hardware. */
  BOARD_InitBootPins();
  BOARD_InitBootClocks();
  BOARD_InitBootPeripherals();

  typedef void (*fp_t)(void); /* function pointer */
  fp_t fp;
  fp = (fp_t)0x1246; /* thumb bit (LSB) not bit set */
  fp();

  APP_Run();
  return 0;
}
