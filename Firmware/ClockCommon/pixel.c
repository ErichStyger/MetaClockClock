/*
 * pixel.c
 *
 * Copyright (c) 2020: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_LED_PIXEL && PL_CONFIG_USE_NEO_PIXEL_HW
#include "pixel.h"
#include "matrix.h"
#include "matrixpixel.h"
#include "McuRTOS.h"
#include "McuLog.h"
#include "McuUtility.h"
#include "stepper.h"
#include "StepperBoard.h"
#include <assert.h>

static void PIXEL_Zero(uint8_t x, uint8_t y, uint8_t z) {
  assert(x<MATRIX_NOF_STEPPERS_X && y<MATRIX_NOF_STEPPERS_Y && z<MATRIX_NOF_STEPPERS_Z);
  STEPPER_MoveMotorStepsRel(MATRIX_GetStepper(x, y, z), STEPPER_FULL_RANGE_NOF_STEPS, 2); /* forward */
  STEPPER_StartTimer();
  STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 10);

  STEPPER_MoveMotorStepsRel(MATRIX_GetStepper(x, y, z), -STEPPER_FULL_RANGE_NOF_STEPS, 2); /* backward */
  STEPPER_StartTimer();
  STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 10);
}

static void PIXEL_ZeroAll(void) {
  for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
    for(int y=0; x<MATRIX_NOF_STEPPERS_Y; y++) {
      for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
        PIXEL_Zero(x, y, z);
      }
    }
  }
}

#if PL_CONFIG_USE_SHELL
static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  McuShell_SendStatusStr((unsigned char*)"pixel", (unsigned char*)"Pixel settings\r\n", io->stdOut);
  return ERR_OK;
}
#endif /* PL_CONFIG_USE_SHELL */

#if PL_CONFIG_USE_SHELL
static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"pixel", (unsigned char*)"Group of pixel commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  zp all", (unsigned char*)"Move all to zero position\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  zp <xyz>", (unsigned char*)"Move a single pixel to zero position (comma separated)\r\n", io->stdOut);
  return ERR_OK;
}
#endif /* PL_CONFIG_USE_SHELL */

#if PL_CONFIG_USE_SHELL
uint8_t PIXEL_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  int32_t x, y, z;
  const unsigned char *p;
  uint8_t res;

  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "pixel help")==0) {
    *handled = true;
    return PrintHelp(io);
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "pixel status")==0)) {
    *handled = true;
    return PrintStatus(io);
  } else if (McuUtility_strcmp((char*)cmd, "pixel zp all")==0) {
    *handled = true;
    PIXEL_ZeroAll();
    return ERR_OK;
  } else if (McuUtility_strncmp((char*)cmd, "pixel zp ", sizeof("pixel zp ")-1)==0) { /* example: pixel zp 0 0 0,1 0 0 */
    *handled = TRUE;
    p = cmd + sizeof("pixel zp ")-1;
    do {
      res = ERR_OK;
      if (*p==',') { /* skip comma (multiple commands) */
        p++;
      }
      if (   McuUtility_xatoi(&p, &x)==ERR_OK && x>=0 && x<MATRIX_NOF_STEPPERS_X
          && McuUtility_xatoi(&p, &y)==ERR_OK && y>=0 && y<MATRIX_NOF_STEPPERS_Y
          && McuUtility_xatoi(&p, &z)==ERR_OK && z>=0 && z<MATRIX_NOF_STEPPERS_Z
         )
      {
        PIXEL_Zero(x, y, z);
      } else {
        res = ERR_FAILED;
      }
    } while(res==ERR_OK && *p==',');
    return res;
  }
  return ERR_OK;
}
#endif /* PL_CONFIG_USE_SHELL */

void PIXEL_Init(void) {
}

#endif /* PL_CONFIG_USE_LED_PIXEL */
