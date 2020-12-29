/*
 * matrixhand.h
 *
 * Copyright (c) 2020: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#ifndef MATRIXHAND_H_
#define MATRIXHAND_H_

#include "platform.h"
#include <stdint.h>
#include <stdbool.h>

#if PL_MATRIX_CONFIG_IS_RGB && PL_CONFIG_IS_ANALOG_CLOCK
void MHAND_HandEnable(uint8_t x, uint8_t y, uint8_t z, bool enable);
void MHAND_HandEnableAll(bool enable);

#if PL_CONFIG_USE_DUAL_HANDS
void MHAND_2ndHandEnable(int32_t x, int32_t y, uint8_t z, bool enable);
void MHAND_2ndHandEnableAll(bool enable);
#endif /* PL_CONFIG_USE_DUAL_HANDS */

void MHAND_SetHandColor(uint8_t x, uint8_t y, uint8_t z, uint32_t color);
void MHAND_SetHandColorAll(uint32_t color);

#if PL_CONFIG_USE_DUAL_HANDS
void MHAND_Set2ndHandColor(uint8_t x, uint8_t y, uint8_t z, uint32_t color);
void MHAND_Set2ndHandColorAll(uint32_t color);
#endif

#endif /* PL_MATRIX_CONFIG_IS_RGB */

#endif /* MATRIXHAND_H_ */
