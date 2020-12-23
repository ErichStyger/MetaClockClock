/*
 * matrixhand.c
 *
 * Author: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include <assert.h>
#include "matrixhand.h"
#include "matrix.h"
#include "NeoStepperRing.h"
#include "NeoPixel.h"

#if PL_MATRIX_CONFIG_IS_RGB
void MHAND_HandEnable(uint8_t x, uint8_t y, uint8_t z, bool enable) {
  assert(x<MATRIX_NOF_STEPPERS_X && y<MATRIX_NOF_STEPPERS_Y && z<MATRIX_NOF_STEPPERS_Z);
#if PL_CONFIG_USE_NEO_PIXEL_HW
  NEOSR_SetHandLedEnabled(MATRIX_GetLedRingDevice(x, y, z), enable);
#elif PL_CONFIG_IS_MASTER && PL_MATRIX_CONFIG_IS_RGB
  matrix.enabledHandMap[x][y][z] = enable;
#endif
#if 0
  uint8_t addr, xb, yb;
  unsigned char cmd[sizeof("matrix hand enable xx yy zz off")];

  addr = MATRIX_GetAddress(x, y, z);
  /* remap position */
  xb = clockMatrix[x][y].board.x;
  yb = clockMatrix[x][y].board.y;
  McuUtility_strcpy(cmd, sizeof(cmd), (unsigned char*)"matrix hand enable ");
  McuUtility_strcatNum8u(cmd, sizeof(cmd), xb);
  McuUtility_chcat(cmd, sizeof(cmd), ' ');
  McuUtility_strcatNum8u(cmd, sizeof(cmd), yb);
  McuUtility_chcat(cmd, sizeof(cmd), ' ');
  McuUtility_strcatNum8u(cmd, sizeof(cmd), z);
  McuUtility_chcat(cmd, sizeof(cmd), ' ');
  if (on) {
    McuUtility_strcat(cmd, sizeof(cmd), (unsigned char*)"on");
  } else {
    McuUtility_strcat(cmd, sizeof(cmd), (unsigned char*)"off");
  }
  MATRIX_SendCmdToBoard(addr, cmd);
#endif
}

void MHAND_HandEnableAll(bool enable) {
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
        MHAND_HandEnable(x, y, z, enable);
      }
    }
  }
#if 0 && PL_MATRIX_CONFIG_IS_RGB
  unsigned char *cmd;
  if (on) {
    cmd = (unsigned char*)"matrix hand enable all on";
  } else {
    cmd = (unsigned char*)"matrix hand enable all off";
  }
  MATRIX_SendCmdToBoard(RS485_BROADCAST_ADDRESS, cmd);
#endif
}

#if PL_CONFIG_USE_DUAL_HANDS
void MHAND_2ndHandEnable(int32_t x, int32_t y, uint8_t z, bool enable) {
  assert(x<MATRIX_NOF_STEPPERS_X && y<MATRIX_NOF_STEPPERS_Y && z<MATRIX_NOF_STEPPERS_Z);
#if PL_CONFIG_USE_NEO_PIXEL_HW
  NEOSR_Set2ndHandLedEnabled(MATRIX_GetLedRingDevice(x, y, z), enable);
#elif PL_CONFIG_IS_MASTER && PL_MATRIX_CONFIG_IS_RGB
  matrix.enabledHandMap[x][y][z] = enable;
#endif
}

void MHAND_2ndHandEnableAll(bool enable) {
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
        MHAND_2ndHandEnable(x, y, z, enable);
      }
    }
  }
}
#endif /* PL_CONFIG_USE_DUAL_HANDS */

void MHAND_SetHandColor(uint8_t x, uint8_t y, uint8_t z, uint32_t color) {
  assert(x<MATRIX_NOF_STEPPERS_X && y<MATRIX_NOF_STEPPERS_Y && z<MATRIX_NOF_STEPPERS_Z);
#if PL_CONFIG_USE_NEO_PIXEL_HW
  NEOSR_SetHandColor(MATRIX_GetLedRingDevice(x, y, z), NEO_SPLIT_RGB(color));
#elif PL_CONFIG_IS_MASTER && PL_MATRIX_CONFIG_IS_RGB
  matrix.colorHandMap[x][y][z] = color;
#endif
}

void MHAND_SetHandColorAll(uint32_t color) {
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
        MHAND_SetHandColor(x, y, z, color);
      }
    }
  }
}

#if PL_CONFIG_USE_DUAL_HANDS
// old MATRIX_Draw2ndHandColor
void MHAND_Set2ndHandColor(uint8_t x, uint8_t y, uint8_t z, uint32_t color) {
  assert(x<MATRIX_NOF_STEPPERS_X && y<MATRIX_NOF_STEPPERS_Y && z<MATRIX_NOF_STEPPERS_Z);
#if PL_CONFIG_USE_NEO_PIXEL_HW
  NEOSR_Set2ndHandColor(MATRIX_GetLedRingDevice(x, y, z), NEO_SPLIT_RGB(color));
#elif PL_CONFIG_IS_MASTER && PL_MATRIX_CONFIG_IS_RGB
  matrix.color2ndHandMap[x][y][z] = color;
#endif
}
#endif

#if PL_CONFIG_USE_DUAL_HANDS
void MHAND_Set2ndHandColorAll(uint32_t color) {
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
        MHAND_Set2ndHandColor(x, y, z, color);
      }
    }
  }
}
#endif
#endif /* PL_MATRIX_CONFIG_IS_RGB */

void MHAND_SetMoveMode(uint8_t x, uint8_t y, uint8_t z, STEPPER_MoveMode_e mode) {
  assert(x<MATRIX_NOF_STEPPERS_X && y<MATRIX_NOF_STEPPERS_Y && z<MATRIX_NOF_STEPPERS_Z);
  matrix.moveMap[x][y][z] = mode;
}

void MHAND_SetMoveModeZ0Z1(uint8_t x, uint8_t y, STEPPER_MoveMode_e mode0, STEPPER_MoveMode_e mode1) {
  MHAND_SetMoveMode(x, y, 0, mode0);
  MHAND_SetMoveMode(x, y, 1, mode1);
}

void MHAND_SetMoveModeZ0Z1All(STEPPER_MoveMode_e mode0, STEPPER_MoveMode_e mode1) {
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MHAND_SetMoveModeZ0Z1(x, y, mode0, mode1);
    }
  }
}

void MHAND_SetMoveModeAll(STEPPER_MoveMode_e mode) {
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
        MHAND_SetMoveMode(x, y, z, mode);
      }
    }
  }
}

void MHAND_SetRelativeMove(uint8_t x, uint8_t y, uint8_t z, bool isRelative) {
  assert(x<MATRIX_NOF_STEPPERS_X && y<MATRIX_NOF_STEPPERS_Y && z<MATRIX_NOF_STEPPERS_Z);
  matrix.isRelModeMap[x][y][z] = isRelative;
}

void MHAND_SetRelativeMoveAll(bool isRelative) {
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
        MHAND_SetRelativeMove(x, y, z, isRelative);
      }
    }
  }
}

