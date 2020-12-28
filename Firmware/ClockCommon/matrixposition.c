/*
 * matrixposition.c
 *
 * Author: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "matrixposition.h"
#include "matrix.h"
#include <assert.h>

#if PL_CONFIG_IS_MASTER // currently only for master configuration implemented */

void MPOS_SetAngle(uint8_t x, uint8_t y, uint8_t z, int16_t angle) {
  assert(x<MATRIX_NOF_STEPPERS_X && y<MATRIX_NOF_STEPPERS_Y && z<MATRIX_NOF_STEPPERS_Z);
#if PL_CONFIG_IS_MASTER
  matrix.angleMap[x][y][z] = angle;
#else
  assert(false);
#endif
}

void MPOS_SetAngleChecked(uint8_t x, uint8_t y, uint8_t z, int16_t angle) {
  if (x>=0 && x<MATRIX_NOF_STEPPERS_X && y>=0 && y<MATRIX_NOF_STEPPERS_Y && z<MATRIX_NOF_STEPPERS_Z) {
    MPOS_SetAngle(x, y, z, angle);
  }
}

void MPOS_SetAngleZ0Z1(uint8_t x, uint8_t y, int16_t z0Angle, int16_t z1Angle) {
  MPOS_SetAngle(x, y, 0, z0Angle);
  MPOS_SetAngle(x, y, 1, z1Angle);
}

void MPOS_SetAngleZ0Z1Checked(uint8_t x, uint8_t y, int16_t z0Angle, int16_t z1Angle) {
  MPOS_SetAngleChecked(x, y, 0, z0Angle);
  MPOS_SetAngleChecked(x, y, 1, z1Angle);
}

void MPOS_SetAngleZ0Z1All(int16_t z0Angle, int16_t z1Angle) {
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetAngleZ0Z1(x, y, z0Angle, z1Angle);
    }
  }
}

void MPOS_SetAngleAll(int16_t angle) {
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
        MPOS_SetAngle(x, y, z, angle);
      }
    }
  }
}

void MPOS_SetMoveMode(uint8_t x, uint8_t y, uint8_t z, STEPPER_MoveMode_e mode) {
  assert(x<MATRIX_NOF_STEPPERS_X && y<MATRIX_NOF_STEPPERS_Y && z<MATRIX_NOF_STEPPERS_Z);
  matrix.moveMap[x][y][z] = mode;
}

void MPOS_SetMoveModeChecked(uint8_t x, uint8_t y, uint8_t z, STEPPER_MoveMode_e mode) {
  if (x>=0 && x<MATRIX_NOF_STEPPERS_X && y>=0 && y<MATRIX_NOF_STEPPERS_Y && z<MATRIX_NOF_STEPPERS_Z) {
    matrix.moveMap[x][y][z] = mode;
  }
}

void MPOS_SetMoveModeZ0Z1(uint8_t x, uint8_t y, STEPPER_MoveMode_e mode0, STEPPER_MoveMode_e mode1) {
  MPOS_SetMoveMode(x, y, 0, mode0);
  MPOS_SetMoveMode(x, y, 1, mode1);
}

void MPOS_SetMoveModeZ0Z1Checked(uint8_t x, uint8_t y, STEPPER_MoveMode_e mode0, STEPPER_MoveMode_e mode1) {
  /* do not set mode if coordinate is outside of matrix */
  MPOS_SetMoveModeChecked(x, y, 0, mode0);
  MPOS_SetMoveModeChecked(x, y, 1, mode1);
}

void MPOS_SetMoveModeZ0Z1All(STEPPER_MoveMode_e mode0, STEPPER_MoveMode_e mode1) {
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetMoveModeZ0Z1(x, y, mode0, mode1);
    }
  }
}

void MPOS_SetMoveModeAll(STEPPER_MoveMode_e mode) {
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
        MPOS_SetMoveMode(x, y, z, mode);
      }
    }
  }
}

#if PL_CONFIG_USE_RELATIVE_MOVES
void MPOS_RelativeMove(uint8_t x, uint8_t y, uint8_t z, int16_t angle) {
  assert(x<MATRIX_NOF_STEPPERS_X && y<MATRIX_NOF_STEPPERS_Y && z<MATRIX_NOF_STEPPERS_Z);
  matrix.relAngleMap[x][y][z] = angle;
}

void MPOS_RelativeMoveZ0Z1(uint8_t x, uint8_t y, int16_t angle0, int16_t angle1) {
  MPOS_RelativeMove(x, y, 0, angle0);
  MPOS_RelativeMove(x, y, 1, angle1);
}

void MPOS_SetRelativeMoveZ0Z1Checked(int x, int y, int angleZ0, int angleZ1) {
  /* do not set angle if coordinate is outside of matrix */
  if (x>=0 && x<MATRIX_NOF_STEPPERS_X && y>=0 && y<MATRIX_NOF_STEPPERS_Y) {
    MPOS_RelativeMoveZ0Z1(x, y, angleZ0, angleZ1);
  }
}

void MPOS_RelativeMoveAll(int16_t angle) {
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
        MPOS_RelativeMove(x, y, z, angle);
      }
    }
  }
}
#endif

#endif /* PL_CONFIG_IS_MASTER */
