/*
 * Copyright (c) 2023-2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "application.h"

int main(void) {
  APP_Run();
  PL_Init();
  return 0;
}
