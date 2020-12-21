/*
 * hands.c
 *
 * Author: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "hands.h"
#include "matrix.h"
#include <assert.h>

// old: MATRIX_DrawClockHand()
void HAND_SetHandAngle(uint8_t x, uint8_t y, uint8_t z, int16_t angle) {
  assert(x<MATRIX_NOF_STEPPERS_X && y<MATRIX_NOF_STEPPERS_Y && z<MATRIX_NOF_STEPPERS_Z);
#if PL_CONFIG_IS_MASTER
  matrix.angleMap[x][y][z] = angle;
#endif
}

// old: MATRIX_DrawClockHands()
void HAND_SetHandAngleBoth(uint8_t x, uint8_t y, int16_t z0Angle, int16_t z1Angle) {
  HAND_SetHandAngle(x, y, 0, z0Angle);
  HAND_SetHandAngle(x, y, 1, z1Angle);
}
