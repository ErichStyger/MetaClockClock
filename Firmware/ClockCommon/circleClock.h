/*
 * Copyright (c) 2021, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CIRCLECLOCK_H_
#define CIRCLECLOCK_H_

#include "circleClock_config.h"
#include <stdint.h>

void CC_ShowTime(uint8_t hour, uint8_t minute);

void CC_Deinit(void);
void CC_Init(void);

#endif /* CIRCLECLOCK_H_ */
