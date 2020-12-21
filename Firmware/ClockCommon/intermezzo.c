/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_INTERMEZZO
#include "intermezzo.h"
#include "matrix.h"
#include "hands.h"
#include "Shell.h"
#include "McuUtility.h"
#include "McuExtRTC.h"
#include "McuTimeDate.h"
#include "McuLog.h"

typedef enum Intermezzo_e {
  INTERMEZZO_NONE = 0,
  INTERMEZZO_FEW = 1,
  INTERMEZZO_MANY = 2, /* NYI! */
  INTERMEZZO_NOF /* must be last in list */
} Intermezzo_e;

static Intermezzo_e IntermezzoMode = INTERMEZZO_NONE;
//static Intermezzo_e IntermezzoMode = INTERMEZZO_FEW;
static uint8_t IntermezzoDelaySec = 15; /* this is the delay *after* forming the time on the clock has started to build up. It takes about 10 secs to build the time */


static void Intermezzo0(void) {
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  MATRIX_SetHandLedEnabledAll(true);
#endif
  MATRIX_DrawAllClockDelays(2, 2);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  MATRIX_DrawAllClockHands(0, 90);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MATRIX_DrawAllClockHands(270, 180);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void Intermezzo1(void) {
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  MATRIX_SetHandLedEnabledAll(true);
#endif
  MATRIX_DrawAllClockDelays(2, 2);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  MATRIX_DrawAllClockHands(0, 180);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CCW);
  MATRIX_DrawAllClockHands(180, 0);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void Intermezzo2(void) {
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  MATRIX_SetHandLedEnabledAll(true);
#endif
  MATRIX_DrawAllClockDelays(6, 6);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  MATRIX_DrawAllClockHands(0, 180);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MATRIX_DrawAllClockDelays(4, 4);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CW);
  MATRIX_DrawAllClockHands(180, 0);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void Intermezzo3(void) {
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  MATRIX_SetHandLedEnabledAll(true);
#endif
  MATRIX_DrawAllClockDelays(4, 4);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CCW, STEPPER_MOVE_MODE_CCW);
  MATRIX_DrawAllClockHands(90, 270);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MATRIX_DrawAllClockHands(0, 0);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void Intermezzo4(void) {
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  MATRIX_SetHandLedEnabledAll(true);
#endif
  MATRIX_DrawAllClockDelays(6, 6);
  MATRIX_DrawAllClockHands(315, 45);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MATRIX_DrawAllClockDelays(4, 4);
  MATRIX_DrawAllClockHands(180, 180);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void Intermezzo5(void) {
  int angle0, angle1;

#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  MATRIX_SetHandLedEnabledAll(true);
#endif
  (void)MATRIX_DrawAllClockDelays(6, 6);
  /* init at 12 */
  (void)MATRIX_MoveAllto12(10000, NULL);
  /* move all clocks to '|' position */
  angle0 = 0;
  angle1 = 180;
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      HAND_SetHandAngleBoth(x, y, angle0, angle1);
    }
  }
  (void)MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  (void)MATRIX_SendToRemoteQueue(); /* queue commands */
  /* rotate them to the next quadrant */
  (void)MATRIX_DrawAllClockDelays(3, 5);
  for(int i=0; i<2; i++) {
    angle0 = (angle0+90)%360;
    angle1 = (angle1+90)%360;
    for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
      for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
        HAND_SetHandAngleBoth(x, y, angle0, angle1);
      }
    }
    (void)MATRIX_SendToRemoteQueue(); /* queue command */
  }
  /* execute */
  (void)MATRIX_ExecuteRemoteQueueAndWait(true);
}

static void Intermezzo6(void) {
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  MATRIX_SetHandLedEnabledAll(true);
#endif
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      (void)MATRIX_DrawMoveMode(x, y, STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
      MATRIX_DrawClockDelays(x, y, 1, 1);
    }
  }
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      HAND_SetHandAngleBoth(x, y, 0, 180);
    }
  }
  (void)MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      (void)MATRIX_DrawMoveMode(x, y, STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CW);
      MATRIX_DrawClockDelays(x, y, 0+x/2, y);
    }
  }
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      HAND_SetHandAngleBoth(x, y, 180, 0);
    }
  }
  (void)MATRIX_SendToRemoteQueue(); /* queue commands */
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      HAND_SetHandAngleBoth(x, y, 0, 180);
    }
  }
  MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
}

static void Intermezzo7(void) {
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  MATRIX_SetHandLedEnabledAll(true);
#endif
  MATRIX_MoveAllto12(10000, NULL);
  MATRIX_DrawAllClockDelays(4, 4);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CCW, STEPPER_MOVE_MODE_CW);
  MATRIX_DrawAllClockHands(180, 180);
  (void)MATRIX_SendToRemoteQueue(); /* queue commands */
  MATRIX_DrawAllClockHands(0, 0);
  (void)MATRIX_SendToRemoteQueue(); /* queue commands */

  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CW);
  MATRIX_DrawAllClockHands(0, 90);
  (void)MATRIX_SendToRemoteQueue(); /* queue commands */
  MATRIX_DrawAllClockHands(270, 0);
  (void)MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
}

static void Intermezzo8(void) {
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  MATRIX_SetHandLedEnabledAll(true);
#endif
  MATRIX_DrawAllClockDelays(4, 4);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  for(int x=0; x<MATRIX_NOF_STEPPERS_X; x+=2) {
    for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y+=2) {
      HAND_SetHandAngleBoth(x, y,   135, 135);
      HAND_SetHandAngleBoth(x, y+1,  45,  45);

      HAND_SetHandAngleBoth(x+1, y  , 225, 225);
      HAND_SetHandAngleBoth(x+1, y+1, 315, 315);
    }
  }
  (void)MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */

  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CCW);
  for(int x=0; x<MATRIX_NOF_STEPPERS_X; x+=2) {
    for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y+=2) {
      HAND_SetHandAngleBoth(x, y  , 135-1, 135+1);
      HAND_SetHandAngleBoth(x, y+1,  45-1,  45+1);

      HAND_SetHandAngleBoth(x+1, y  , 225-1, 225+1);
      HAND_SetHandAngleBoth(x+1, y+1, 315-1, 315+1);
    }
  }
  (void)MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */

  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CCW, STEPPER_MOVE_MODE_CW);
  for(int x=0; x<MATRIX_NOF_STEPPERS_X; x+=2) {
    for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y+=2) {
      HAND_SetHandAngleBoth(x, y  , 135, 135);
      HAND_SetHandAngleBoth(x, y+1,  45,  45);

      HAND_SetHandAngleBoth(x+1, y  , 225, 225);
      HAND_SetHandAngleBoth(x+1, y+1, 315, 315);
    }
  }
  MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
}

static void Intermezzo9(void) {
  uint8_t res;
  TIMEREC time;

  /* show time on each clock */
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  MATRIX_SetHandLedEnabledAll(true);
#endif
  res = McuTimeDate_GetTimeDate(&time, NULL);
  if (res!=ERR_OK) {
    McuLog_error("Intermezzo: failed getting time");
    return;
  }
  MATRIX_DrawAllClockDelays(4, 4);
  time.Hour %= 12;
  MATRIX_DrawAllClockHands(time.Min*360/60, time.Hour*360/12);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void IntermezzoRandomHandsAllOn(void) {
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  MATRIX_SetHandLedEnabledAll(true);
#endif
  (void)MATRIX_DrawAllClockDelays(2, 5);
  (void)MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
        HAND_SetHandAngleBoth(x, y, z,  McuUtility_random(0, 359));
      #if PL_CONFIG_USE_NEO_PIXEL_HW
        MATRIX_SetHandColor(x, y, z, McuUtility_random(0, 255), McuUtility_random(0, 255), McuUtility_random(0, 255));
      #endif
      }
    }
  }
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void IntermezzoRandomHands(void) {
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  MATRIX_SetHandLedEnabledAll(true);
#endif
  MATRIX_DrawAllClockDelays(3, 7);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      HAND_SetHandAngleBoth(x, y, McuUtility_random(0, 359),  McuUtility_random(0, 359));
    }
  }
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void IntermezzoTemperature(void) {
  float temperature;
  uint8_t res;

  (void)MATRIX_DrawAllClockDelays(3, 3);
  res = McuExtRTC_GetTemperature(&temperature);
  if (res!=ERR_OK) {
    return ;
  }
  temperature -= 4.0; /* offset */
#if MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5
  MATRIX_ShowTemperatureLarge(temperature, false);
#elif MATRIX_NOF_STEPPERS_X>=8 && MATRIX_NOF_STEPPERS_Y>=3
  MATRIX_ShowTemperature(temperature, false);
#endif
}

static void DrawNestedRectangles(int xPos, int yPos, int width, int height) {
  int x, y, w, h;

  (void)MATRIX_DrawAllClockDelays(2, 2);
  x = xPos; y = yPos; w = width; h = height;
  while(w>0 && h>0) {
    if (h==1) {
      MATRIX_DrawHLine(x, y, w);
    } else if (w==1) {
      MATRIX_DrawVLine(x, y, h);
    } else {
      MATRIX_DrawRectangle(x, y, w, h);
    }
    x++; y++; w -= 2; h -= 2;
  }
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void IntermezzoRectangles(void) {
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  MATRIX_SetHandLedEnabledAll(true);
#endif
  DrawNestedRectangles(0, 0, MATRIX_NOF_STEPPERS_X, MATRIX_NOF_STEPPERS_Y);
}

static void IntermezzoRectangles2(void) {
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  MATRIX_SetHandLedEnabledAll(true);
#endif
  DrawNestedRectangles(0, 0, MATRIX_NOF_STEPPERS_X/2, MATRIX_NOF_STEPPERS_Y);
  DrawNestedRectangles(MATRIX_NOF_STEPPERS_X/2, 0, MATRIX_NOF_STEPPERS_X/2, MATRIX_NOF_STEPPERS_Y);
}

static void IntermezzoRectangles3(void) {
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  MATRIX_SetHandLedEnabledAll(true);
#endif
  DrawNestedRectangles(0, 0, MATRIX_NOF_STEPPERS_X/3, MATRIX_NOF_STEPPERS_Y);
  DrawNestedRectangles(MATRIX_NOF_STEPPERS_X/3, 0, MATRIX_NOF_STEPPERS_X/3, MATRIX_NOF_STEPPERS_Y);
  DrawNestedRectangles(2*MATRIX_NOF_STEPPERS_X/3, 0, MATRIX_NOF_STEPPERS_X/3, MATRIX_NOF_STEPPERS_Y);
}

typedef void (*Intermezzofp)(void); /* intermezzo function pointer */
static const Intermezzofp intermezzos[] = /* list of intermezzos */
{
    Intermezzo0,
    Intermezzo1,
    Intermezzo2,
    Intermezzo3,
    Intermezzo4,
    Intermezzo5,
    Intermezzo6,
    Intermezzo7,
    Intermezzo8,
    Intermezzo9,
    IntermezzoRandomHands,
    IntermezzoRandomHandsAllOn,
    IntermezzoTemperature,
    IntermezzoRectangles,
    IntermezzoRectangles2,
    IntermezzoRectangles3,
};
#define NOF_INTERMEZZOS   (sizeof(intermezzos)/sizeof(intermezzos[0]))

void INTERMEZZO_Play(TickType_t lastClockUpdateTickCount, bool *intermezzoShown) {
  TickType_t tickCount;
  uint8_t intermezzo;

  if (IntermezzoMode==INTERMEZZO_FEW) {
    tickCount = xTaskGetTickCount();
    if (tickCount-lastClockUpdateTickCount > pdMS_TO_TICKS(IntermezzoDelaySec*1000)) { /* after a delay: start intermezzo */

      intermezzo = McuUtility_random(0, NOF_INTERMEZZOS-1);
      McuLog_info("Intermezzo: starting %d", intermezzo);
      intermezzos[intermezzo]();
      McuLog_info("Intermezzo: %d done", intermezzo);
      *intermezzoShown = true;
    }
  } else if(IntermezzoMode==INTERMEZZO_MANY) {
    /* not implemented yet */
    *intermezzoShown = true;
  }
}

#if PL_CONFIG_USE_SHELL
static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  uint8_t res = ERR_OK;
  uint8_t buf[64];

  McuShell_SendStatusStr((unsigned char*)"intermezzo", (unsigned char*)"Intermezzo settings\r\n", io->stdOut);
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"mode: ");
  McuUtility_strcatNum32u(buf, sizeof(buf), IntermezzoMode);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" nof: ");
  McuUtility_strcatNum32u(buf, sizeof(buf), NOF_INTERMEZZOS);

  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" delay: ");
  McuUtility_strcatNum32u(buf, sizeof(buf), IntermezzoDelaySec);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" s");

  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  intermezzo", buf, io->stdOut);
  return res;
}
#endif

#if PL_CONFIG_USE_SHELL
static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"intermezzo", (unsigned char*)"Group of intermezzo commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  mode <nr>", (unsigned char*)"Set Intermezzo mode (0-2):\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"", (unsigned char*)"0: no intermezzos\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"", (unsigned char*)"1: few intermezzos\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"", (unsigned char*)"2: many intermezzos (not implemented yet!)\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  delay <sec>", (unsigned char*)"Intermezzo delay in seconds\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  <nr>", (unsigned char*)"Play Intermezzo (0-", io->stdOut);
  McuShell_SendNum32u(NOF_INTERMEZZOS-1, io->stdOut);
  McuShell_SendStr((unsigned char*)")\r\n", io->stdOut);
  return ERR_OK;
}
#endif

uint8_t INTERMEZZO_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  const unsigned char *p;

  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "intermezzo help")==0) {
    *handled = true;
    return PrintHelp(io);
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "intermezzo status")==0)) {
    *handled = true;
    return PrintStatus(io);
  } else if (McuUtility_strncmp((char*)cmd, "intermezzo mode ", sizeof("intermezzo mode ")-1)==0) {
    uint8_t val;

    *handled = TRUE;
    p = cmd + sizeof("intermezzo mode ")-1;
    if (McuUtility_ScanDecimal8uNumber(&p, &val)==ERR_OK && val<INTERMEZZO_NOF) {
      IntermezzoMode = val;
    } else {
      return ERR_FAILED;
    }
    return ERR_OK;
  } else if (McuUtility_strncmp((char*)cmd, "intermezzo delay ", sizeof("intermezzo delay ")-1)==0) {
    uint8_t sec;

    *handled = TRUE;
    p = cmd + sizeof("intermezzo delay ")-1;
    if (McuUtility_ScanDecimal8uNumber(&p, &sec)==ERR_OK) {
      IntermezzoDelaySec = sec;
      return ERR_OK;
    } else {
      return ERR_FAILED;
    }
  } else if (McuUtility_strncmp((char*)cmd, "intermezzo ", sizeof("intermezzo ")-1)==0) {
    uint8_t nr;

    *handled = TRUE;
    p = cmd + sizeof("intermezzo ")-1;
    if (McuUtility_ScanDecimal8uNumber(&p, &nr)==ERR_OK && nr<NOF_INTERMEZZOS) {
      intermezzos[nr]();
      return ERR_OK;
    } else {
      return ERR_FAILED;
    }
  }
  return ERR_OK;
}

void INTERMEZZO_Init(void) {
}


#endif /* PL_CONFIG_USE_INTERMEZZO */
