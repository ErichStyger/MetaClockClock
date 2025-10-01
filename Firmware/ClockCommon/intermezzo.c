/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_INTERMEZZO
#include "intermezzo.h"
#include "matrix.h"
#include "matrixposition.h"
#include "matrixhand.h"
#include "shell.h"
#include "McuUtility.h"
#include "McuExtRTC.h"
#include "McuTimeDate.h"
#include "McuLog.h"
#include "NeoPixel.h"
#include "mfont.h"
#include <math.h>
#if PL_CONFIG_HAS_CIRCLE_CLOCK
  #include "circleClock.h"
#endif
#if PL_CONFIG_USE_LED_PIXEL
  #include "matrixpixel.h"
  #include "LedClock.h"
  #include "stepperconfig.h"
#endif
#if PL_CONFIG_USE_MININI
  #include "minIni/McuMinINI.h"
  #include "MinIniKeys.h"
#endif
#if PL_CONFIG_USE_SHT31
  #include "McuSHT31.h"
#endif

static bool IntermezzoOn = /* if intermezzos are on by default or not */
#if PL_CONFIG_INTERMEZZO_ON_BY_DEFAULT
    true;
#else
    false;
#endif
#if PL_CONFIG_USE_LED_PIXEL
static uint8_t IntermezzoDelaySec = 0; /* this is the delay *after* forming the time on the clock has started to build up. It takes about 2 secs to build the time */
static uint8_t IntermezzoFadeSec = (STEPPER_TIME_FULL_RANGE_MS/2000);  /* this is the fading-time of color changes between intermezzos */
#define INTERMEZZO_BG_BRIGHTNESS 	(20)	/* Brightness of the intermezzo background color */
#define INTERMEZZO_FONT_BRIGHTNESS 	(100)	/* Brightness of the intermezzo font color */
#else
static uint8_t IntermezzoDelaySec = 15; /* this is the delay *after* forming the time on the clock has started to build up. It takes about 10 secs to build the time */
#endif

#define RTC_OFFSET_TEMPERATURE_DEFAULT   (-40)
static int8_t rtcOffsetTemperature = RTC_OFFSET_TEMPERATURE_DEFAULT; /* deci-celsius */

static void Intermezzo_SetIsOn(bool onOff) {
  IntermezzoOn = onOff;
#if PL_CONFIG_USE_MININI
  McuMinINI_ini_putl(NVMC_MININI_SECTION_INTERMEZZO, NVMC_MININI_KEY_INTERMEZZO_ON, onOff, NVMC_MININI_FILE_NAME);
#endif
}

static int8_t GetRtcOffsetTemperature_dC(void) {
  return rtcOffsetTemperature; /* deci-celsius */
}

static void SetRtcOffsetTemperature(int offset_dC) {
  rtcOffsetTemperature = offset_dC;
#if PL_CONFIG_USE_MININI
  McuMinINI_ini_putl(NVMC_MININI_SECTION_INTERMEZZO, NVMC_MININI_KEY_RTC_TEMP_OFFSET, offset_dC, NVMC_MININI_FILE_NAME);
#endif
}

void Intermezzo_InitSettings(void) {
#if PL_CONFIG_USE_MININI && PL_CONFIG_USE_CLOCK_TIME_OFF
  IntermezzoOn = McuMinINI_ini_getbool(NVMC_MININI_SECTION_INTERMEZZO, NVMC_MININI_KEY_INTERMEZZO_ON, PL_CONFIG_INTERMEZZO_ON_BY_DEFAULT, NVMC_MININI_FILE_NAME);
#else
  IntermezzoOn = PL_CONFIG_INTERMEZZO_ON_BY_DEFAULT;
#endif
#if PL_CONFIG_USE_MININI
  rtcOffsetTemperature = McuMinINI_ini_getl(NVMC_MININI_SECTION_INTERMEZZO, NVMC_MININI_KEY_RTC_TEMP_OFFSET, RTC_OFFSET_TEMPERATURE_DEFAULT, NVMC_MININI_FILE_NAME);
#else
  rtcOffsetTemperature = RTC_OFFSET_TEMPERATURE_DEFAULT;
#endif
}

/* #if-directive for all matrix configurations except SmartWall Matrix*/
/* =============================================================================================================*/
#if PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_CLOCK_8x3 || PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_CLOCK_12x5_60B || PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_CLOCK_12x5_MOD || PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_CLOCK_8x3_V4 || PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_CLOCK_16x9_ALEXIS || PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_CIRCULAR_CLOCK_1x12
static void Intermezzo0(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  MATRIX_SetMoveDelayZ0Z1All(2, 2);
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  MPOS_SetAngleZ0Z1All(0, 90);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MPOS_SetAngleZ0Z1All(270, 180);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void Intermezzo1(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  MATRIX_SetMoveDelayZ0Z1All(2, 2);
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  MPOS_SetAngleZ0Z1All(0, 180);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  vTaskDelay(pdMS_TO_TICKS(1000));
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CCW);
  MPOS_SetAngleZ0Z1All(180, 0);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void Intermezzo2(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  MATRIX_SetMoveDelayZ0Z1All(6, 6);
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  MPOS_SetAngleZ0Z1All(0, 180);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MATRIX_SetMoveDelayZ0Z1All(4, 4);
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CW);
  MPOS_SetAngleZ0Z1All(180, 0);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void Intermezzo3(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  MATRIX_SetMoveDelayZ0Z1All(4, 4);
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_CCW, STEPPER_MOVE_MODE_CCW);
  MPOS_SetAngleZ0Z1All(90, 270);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MPOS_SetAngleZ0Z1All(0, 0);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void Intermezzo4(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  MATRIX_SetMoveDelayZ0Z1All(6, 6);
  MPOS_SetAngleZ0Z1All(315, 45);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MATRIX_SetMoveDelayZ0Z1All(4, 4);
  MPOS_SetAngleZ0Z1All(180, 180);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void Intermezzo5(void) {
  int angle0, angle1;

#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  (void)MATRIX_SetMoveDelayZ0Z1All(6, 6);
  /* init at 12 */
  (void)MATRIX_MoveAllto12(10000, NULL);
  /* move all clocks to '|' position */
  angle0 = 0;
  angle1 = 180;
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetAngleZ0Z1(x, y, angle0, angle1);
    }
  }
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  (void)MATRIX_SendToRemoteQueue(); /* queue commands */
  /* rotate them to the next quadrant */
  (void)MATRIX_SetMoveDelayZ0Z1All(3, 5);
  for(int i=0; i<2; i++) {
    angle0 = (angle0+90)%360;
    angle1 = (angle1+90)%360;
    for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
      for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
        MPOS_SetAngleZ0Z1(x, y, angle0, angle1);
      }
    }
    (void)MATRIX_SendToRemoteQueue(); /* queue command */
  }
  /* execute */
  (void)MATRIX_ExecuteRemoteQueueAndWait(true);
}

static void Intermezzo6(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetMoveModeZ0Z1(x, y, STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
      MATRIX_SetMoveDelayZ0Z1Checked(x, y, 2, 2);
    }
  }
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetAngleZ0Z1(x, y, 0, 180);
    }
  }
  (void)MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetMoveModeZ0Z1(x, y, STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CW);
      MATRIX_SetMoveDelayZ0Z1Checked(x, y, 2+x/2, 2+y);
    }
  }
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetAngleZ0Z1(x, y, 180, 0);
    }
  }
  (void)MATRIX_SendToRemoteQueue(); /* queue commands */
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetAngleZ0Z1(x, y, 0, 180);
    }
  }
  MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
}

static void Intermezzo7(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  MATRIX_MoveAllto12(10000, NULL);
  MATRIX_SetMoveDelayZ0Z1All(4, 4);
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_CCW, STEPPER_MOVE_MODE_CW);
  MPOS_SetAngleZ0Z1All(180, 180);
  (void)MATRIX_SendToRemoteQueue(); /* queue commands */
  MPOS_SetAngleZ0Z1All(0, 0);
  (void)MATRIX_SendToRemoteQueue(); /* queue commands */

  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CW);
  MPOS_SetAngleZ0Z1All(0, 90);
  (void)MATRIX_SendToRemoteQueue(); /* queue commands */
  MPOS_SetAngleZ0Z1All(270, 0);
  (void)MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
}

static void Intermezzo8(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  MATRIX_SetMoveDelayZ0Z1All(4, 4);
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);

  for(int x=0; x<MATRIX_NOF_STEPPERS_X; x+=2) {
    for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y+=2) {
      MPOS_SetAngleZ0Z1(x, y,   135, 135);
      if (y+1<MATRIX_NOF_STEPPERS_Y) {
        MPOS_SetAngleZ0Z1(x, y+1,  45,  45);
      }

      if (x+1<MATRIX_NOF_STEPPERS_X) {
        MPOS_SetAngleZ0Z1(x+1, y  , 225, 225);
        if (y+1<MATRIX_NOF_STEPPERS_Y) {
          MPOS_SetAngleZ0Z1(x+1, y+1, 315, 315);
        }
      }
    }
  }
  (void)MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */

  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CCW);
  for(int x=0; x<MATRIX_NOF_STEPPERS_X; x+=2) {
    for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y+=2) {
      MPOS_SetAngleZ0Z1(x, y  , 135-1, 135+1);
      if (y+1<MATRIX_NOF_STEPPERS_Y) {
        MPOS_SetAngleZ0Z1(x, y+1,  45-1,  45+1);
      }

      if (x+1<MATRIX_NOF_STEPPERS_X) {
        MPOS_SetAngleZ0Z1(x+1, y  , 225-1, 225+1);
        if (y+1<MATRIX_NOF_STEPPERS_Y) {
          MPOS_SetAngleZ0Z1(x+1, y+1, 315-1, 315+1);
        }
      }
    }
  }
  (void)MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
}

static void Intermezzo9(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  MATRIX_SetMoveDelayZ0Z1All(2, 2);
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  MPOS_SetAngleZ0Z1All(0, 0);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);

  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CW);
  MPOS_SetAngleZ0Z1All(180, 0);
  MATRIX_SendToRemoteQueue();
  MPOS_SetAngleZ0Z1All(0, 0);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);

  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  MPOS_SetAngleZ0Z1All(0, 0);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void Intermezzo10(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  MATRIX_SetMoveDelayZ0Z1All(2, 2);
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);

  MPOS_SetAngleZ0Z1All(0, 0);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);

  MPOS_SetAngleZ0Z1All(270, 180);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);

  MPOS_SetAngleZ0Z1All(0, 0);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void Intermezzo11(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  (void)MATRIX_SetMoveDelayZ0Z1All(2, 2);
  MPOS_SetAngleZ0Z1All(0, 0);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);

  /* set move mode: */
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MATRIX_SetMoveDelayZ0Z1Checked(x, y, 2+y, 2+y);
    }
  }
  /* set movement: */
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetAngleZ0Z1(x, y, 90, 270);
    }
  }
  MATRIX_SendToRemoteQueue(); /* queue commands */

  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetAngleZ0Z1(x, y, 0, 180);
    }
  }
  MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */

  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  MPOS_SetAngleZ0Z1All(0, 0);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void Intermezzo12(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  (void)MATRIX_SetMoveDelayZ0Z1All(2, 2);
  MPOS_SetAngleZ0Z1All(270, 270);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);

  (void)MATRIX_SetMoveDelayZ0Z1All(2, 2);
#if MATRIX_NOF_STEPPERS_X>6 /* speed up for larger clocks */
  for(int x=0; x<MATRIX_NOF_STEPPERS_X; x+=2) {
    for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
      MPOS_SetAngleZ0Z1(x, y, 270, 90);
      if (x+1<MATRIX_NOF_STEPPERS_X) {
        MPOS_SetAngleZ0Z1(x+1, y, 270, 90);
      }
    }
    MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  }
#else
  for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
    for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
      MPOS_SetAngleZ0Z1(x, y, 270, 90);
    }
    MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  }
#endif
}

static void Intermezzo13(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  (void)MATRIX_SetMoveDelayZ0Z1All(2, 2);
  MPOS_SetAngleZ0Z1All(0, 0);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);

#if MATRIX_NOF_STEPPERS_Y>6 /* speed up for larger clocks */
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y+=2) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetAngleZ0Z1(x, y, 0, 180);
      if (y+1<MATRIX_NOF_STEPPERS_Y) {
        MPOS_SetAngleZ0Z1(x, y+1, 0, 180);
      }
    }
    MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  }
#else
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetAngleZ0Z1(x, y, 0, 180);
    }
    MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  }
#endif
}

static void Intermezzo14(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  (void)MATRIX_SetMoveDelayZ0Z1All(2, 2);
  MPOS_SetAngleZ0Z1All(0, 180);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);

  /* set move mode: */
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_CCW, STEPPER_MOVE_MODE_CW);
  for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
    for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
      MPOS_SetAngleZ0Z1(x, y, 180, 0);
    }
  }
  MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */

  MPOS_SetAngleZ0Z1All(0, 180);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void Intermezzo15(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  (void)MATRIX_SetMoveDelayZ0Z1All(2, 2);
  MPOS_SetAngleZ0Z1All(270, 90);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);

  /* set move mode: */
  for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
    for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
      if ((x%2)==0) {
        MPOS_SetMoveModeZ0Z1(x, y, STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CW);
        MPOS_SetAngleZ0Z1(x, y, 269, 89);
      } else {
        MPOS_SetMoveModeZ0Z1(x, y, STEPPER_MOVE_MODE_CCW, STEPPER_MOVE_MODE_CCW);
        MPOS_SetAngleZ0Z1(x, y, 271, 91);
      }
    }
  }
  MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
}

#if PL_CONFIG_USE_RELATIVE_MOVES && !PL_CONFIG_USE_VIRTUAL_STEPPER /* issue with LED virtual steppers following properly, disabled for now */
/* map of 'circles', limited for now to 10x10 quadrants. Same numbers correspond to the radius */
static const int circleMap[10][10] = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
    {1, 1, 2, 3, 4, 5, 6, 6, 8, 9},
    {2, 2, 2, 3, 4, 5, 6, 7, 8, 9},
    {3, 3, 4, 4, 5, 6, 6, 7, 8, 9},
    {4, 4, 4, 5, 6, 6, 7, 8, 9, 10},
    {5, 5, 5, 6, 6, 7, 8, 9, 10, 10},
    {6, 6, 6, 6, 7, 8, 9, 10, 10, 11},
    {7, 7, 7, 7, 8, 9, 10, 10, 11, 11},
    {8, 8, 8, 8, 9, 10, 10, 11, 11, 12},
    {9, 9, 9, 9, 8, 8, 9, 10, 11, 12},
};

static void MoveCircles(int x0, int y0, int r, int angle0, int angle1) {
  int xpos, ypos;
  for(int x=0; x<10; x++) {
    for (int y=0; y<10; y++) {
      if (circleMap[x][y]==r) {
        /* quadrant: right lower */
        xpos = x0+x;
        ypos = y0+y;
        MPOS_SetRelativeMoveZ0Z1Checked(xpos, ypos, angle0, angle1);

        /* quadrant: right upper */
        if (y!=0) {
          xpos = x0+x;
          ypos = y0-y;
          MPOS_SetRelativeMoveZ0Z1Checked(xpos, ypos, angle0, angle1);
        }

        /* quadrant: left upper */
        if (x!=0 && y!=0) {
          xpos = x0-x;
          ypos = y0-y;
          MPOS_SetRelativeMoveZ0Z1Checked(xpos, ypos, angle0, angle1);
        }

        /* quadrant: left lower */
        if (x!=0) {
          xpos = x0-x;
          ypos = y0+y;
          MPOS_SetRelativeMoveZ0Z1Checked(xpos, ypos, angle0, angle1);
        }
      }
    }
  }
}

static void BuildCircles(void) {
  int angleX, angleY;
  int x, y;

#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  (void)MATRIX_SetMoveDelayZ0Z1All(2, 2);

  /* middle lines */
  x = MATRIX_NOF_STEPPERS_X/2;
  for(y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    if (y<MATRIX_NOF_STEPPERS_Y/2) {
      MPOS_SetAngleZ0Z1(x, y, 90, 270);
    } else {
      MPOS_SetAngleZ0Z1(x, y, 270, 90);
    }
  }
  y = MATRIX_NOF_STEPPERS_Y/2;
  for(x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
    if (x<MATRIX_NOF_STEPPERS_X/2) {
      MPOS_SetAngleZ0Z1(x, y, 0, 180);
    } else {
      MPOS_SetAngleZ0Z1(x, y, 180, 0);
    }
  }

  /* right upper quadrant */
  angleY = 45;
  for(y=MATRIX_NOF_STEPPERS_Y/2 - 1; y>=0; y--) {
    angleX = angleY;
    for(x=MATRIX_NOF_STEPPERS_X/2 + 1; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetAngleZ0Z1(x, y, 180-angleX, 0-angleX);
      angleX /= 2;
    }
    angleY += angleY/2;
  }
  /* right lower quadrant */
  angleY = 45;
  for(y=MATRIX_NOF_STEPPERS_Y/2 + 1; y<MATRIX_NOF_STEPPERS_Y; y++) {
    angleX = angleY;
    for(x=MATRIX_NOF_STEPPERS_X/2 + 1; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetAngleZ0Z1(x, y, 180+angleX, 0+angleX);
      angleX /= 2;
    }
    angleY += angleY/2;
  }
  /* left lower quadrant */
  angleY = 45;
  for(y=MATRIX_NOF_STEPPERS_Y/2 + 1; y<MATRIX_NOF_STEPPERS_Y; y++) {
    angleX = angleY;
    for(x=MATRIX_NOF_STEPPERS_X/2 - 1; x>=0; x--) {
      MPOS_SetAngleZ0Z1(x, y, 0-angleX, 180-angleX);
      angleX /= 2;
    }
    angleY += angleY/2;
  }
  /* left upper quadrant */
  angleY = 45;
  for(y=MATRIX_NOF_STEPPERS_Y/2 - 1; y>=0; y--) {
    angleX = angleY;
    for(x=MATRIX_NOF_STEPPERS_X/2 - 1; x>=0; x--) {
      MPOS_SetAngleZ0Z1(x, y, 0+angleX, 180+angleX);
      angleX /= 2;
    }
    angleY += angleY/2;
  }
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}
#endif /* PL_CONFIG_USE_RELATIVE_MOVES */

#if PL_CONFIG_USE_RELATIVE_MOVES && !PL_CONFIG_USE_VIRTUAL_STEPPER /* issue with LED virtual steppers following properly, disabled for now */
static void Intermezzo16(void) {
  BuildCircles();
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  for (int r=0; r<MATRIX_NOF_STEPPERS_X/2+1; r++) {
    MoveCircles(MATRIX_NOF_STEPPERS_X/2, MATRIX_NOF_STEPPERS_Y/2, r, 360, 360);
    MATRIX_SendToRemoteQueueExecuteAndWait(false);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  MATRIX_WaitForIdle(10000); /* wait until it is finished */
}
#endif /* PL_CONFIG_USE_RELATIVE_MOVES */

#if PL_CONFIG_USE_RELATIVE_MOVES && !PL_CONFIG_USE_VIRTUAL_STEPPER /* issue with LED virtual steppers following properly, disabled for now */
static void Intermezzo17(void) {
  BuildCircles();
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CCW);
  for (int r=0; r<MATRIX_NOF_STEPPERS_X/2+1; r++) {
    MoveCircles(MATRIX_NOF_STEPPERS_X/2, MATRIX_NOF_STEPPERS_Y/2, r, 360, 360);
    MATRIX_SendToRemoteQueueExecuteAndWait(false);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  MATRIX_WaitForIdle(10000); /* wait until it is finished */
}
#endif /* PL_CONFIG_USE_RELATIVE_MOVES */

#if PL_CONFIG_USE_RELATIVE_MOVES && !PL_CONFIG_USE_VIRTUAL_STEPPER /* issue with LED virtual steppers following properly, disabled for now */
static void Intermezzo18(void) {
  BuildCircles();
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_CCW, STEPPER_MOVE_MODE_CW);
  for (int r=0; r<MATRIX_NOF_STEPPERS_X/2+1; r++) {
    MoveCircles(MATRIX_NOF_STEPPERS_X/2, MATRIX_NOF_STEPPERS_Y/2, r, 360, 360);
    MATRIX_SendToRemoteQueueExecuteAndWait(false);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  MATRIX_WaitForIdle(10000); /* wait until it is finished */
}
#endif /* PL_CONFIG_USE_RELATIVE_MOVES */

#if PL_CONFIG_USE_RELATIVE_MOVES && !PL_CONFIG_USE_VIRTUAL_STEPPER /* issue with LED virtual steppers following properly, disabled for now */
static void Intermezzo19(void) {
  BuildCircles();
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CW);
  MPOS_RelativeMoveAll(-360);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}
#endif /* PL_CONFIG_USE_RELATIVE_MOVES */

#if PL_CONFIG_USE_RELATIVE_MOVES && !PL_CONFIG_USE_VIRTUAL_STEPPER /* issue with LED virtual steppers following properly, disabled for now */
static void Intermezzo20(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  (void)MATRIX_SetMoveDelayZ0Z1All(2, 2);
  MPOS_SetAngleZ0Z1All(270, 90);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);

  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CW);
  MPOS_RelativeMoveAll(45);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);

  MPOS_RelativeMoveAll(-90);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);

  MPOS_SetAngleZ0Z1All(270, 90);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}
#endif /* PL_CONFIG_USE_RELATIVE_MOVES */

static void IntermezzoTime(void) {
  uint8_t res;
  TIMEREC time;

  /* show time on each clock */
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  res = McuTimeDate_GetTimeDate(&time, NULL);
  if (res!=ERR_OK) {
    McuLog_error("Intermezzo: failed getting time");
    return;
  }
  MATRIX_SetMoveDelayZ0Z1All(4, 4);
  time.Hour %= 12;
  MPOS_SetAngleZ0Z1All(time.Min*360/60, time.Hour*360/12);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

#if PL_CONFIG_USE_RELATIVE_MOVES && !PL_CONFIG_USE_VIRTUAL_STEPPER /* issue with LED virtual steppers following properly, disabled for now */
static void IntermezzoHalfHalf(void) {
  /* rotate left half CW, right half CCW */
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  MATRIX_SetMoveDelayAll(MATRIX_GetDefaultDelay());
  MPOS_SetAngleZ0Z1All(270, 90);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);

  for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
    for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
      for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
        MPOS_RelativeMove(x, y, z, x<MATRIX_NOF_STEPPERS_X/2?2*360:-2*360);
      }
    }
  }
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}
#endif

static void IntermezzoRandomHandsAllOn(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  (void)MATRIX_SetMoveDelayZ0Z1All(2, 5);
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
        MPOS_SetAngleZ0Z1(x, y, z,  McuUtility_random(0, 359));
      #if PL_MATRIX_CONFIG_IS_RGB
        MHAND_SetHandColor(x, y, z, NEO_COMBINE_RGB(McuUtility_random(0, 255), McuUtility_random(0, 255), McuUtility_random(0, 255)));
      #endif
      }
    }
  }
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
#if PL_MATRIX_CONFIG_IS_RGB
  /* restore normal hand color */
  MHAND_SetHandColorAll(MATRIX_GetHandColorAdjusted());
#endif
}

static void IntermezzoRandomHands(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  MATRIX_SetMoveDelayZ0Z1All(3, 7);
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetAngleZ0Z1(x, y, McuUtility_random(0, 359),  McuUtility_random(0, 359));
    }
  }
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void IntermezzoTemperature(void) {
  float temperature;
  uint8_t res;
  uint8_t buf[8];

#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  (void)MATRIX_SetMoveDelayZ0Z1All(3, 3);
  res = McuExtRTC_GetTemperature(&temperature);
  if (res!=ERR_OK) {
    McuLog_error("failed reading RTC temperature sensor");
    return;
  }
  temperature += 0.1f*GetRtcOffsetTemperature_dC(); /* offset */
  buf[0] = '\0';
  McuUtility_strcatNumFloat(buf, sizeof(buf), temperature, 0);
  McuUtility_chcat(buf, sizeof(buf), MFONT_CHAR_DEGREE);
  McuUtility_chcat(buf, sizeof(buf), 'C');
#if MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5
  (void)MFONT_ShowFramedText(0, 0, buf, MFONT_SIZE_3x5, true, true);
#elif MATRIX_NOF_STEPPERS_X>=8 && MATRIX_NOF_STEPPERS_Y>=3
  (void)MFONT_ShowFramedText(0, 0, buf, MFONT_SIZE_2x3, true, true);
#endif
}

#if PL_CONFIG_USE_SHT31
static void IntermezzoHumidity(void) {
  uint8_t res;
  uint8_t buf[8];
  float temperature, humidity;

#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  (void)MATRIX_SetMoveDelayZ0Z1All(3, 3);
  res = McuSHT31_ReadTempHum(&temperature, &humidity);
  if (res!=ERR_OK) {
    McuLog_error("failed reading SHT31 sensor");
    return;
  }
  buf[0] = '\0';
  McuUtility_strcatNumFloat(buf, sizeof(buf), humidity, 0);
  McuUtility_chcat(buf, sizeof(buf), MFONT_CHAR_DEGREE);
  McuUtility_chcat(buf, sizeof(buf), '%');
#if MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5
  (void)MFONT_ShowFramedText(0, 0, buf, MFONT_SIZE_3x5, true, true);
#elif MATRIX_NOF_STEPPERS_X>=8 && MATRIX_NOF_STEPPERS_Y>=3
  (void)MFONT_ShowFramedText(0, 0, buf, MFONT_SIZE_2x3, true, true);
#endif
}
#endif /* PL_CONFIG_USE_SHT31 */

static void DrawNestedRectangles(int xPos, int yPos, int width, int height) {
  int x, y, w, h;

#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  (void)MATRIX_SetMoveDelayZ0Z1All(2, 2);
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
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  DrawNestedRectangles(0, 0, MATRIX_NOF_STEPPERS_X, MATRIX_NOF_STEPPERS_Y);
}

static void IntermezzoRectangles2(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  DrawNestedRectangles(0, 0, MATRIX_NOF_STEPPERS_X/2, MATRIX_NOF_STEPPERS_Y);
  DrawNestedRectangles(MATRIX_NOF_STEPPERS_X/2, 0, MATRIX_NOF_STEPPERS_X/2, MATRIX_NOF_STEPPERS_Y);
}

static void IntermezzoRectangles3(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  const int w = 2;
  for(int x=0; x<(MATRIX_NOF_STEPPERS_X/w)*w; x+=w) { /* for odd number of X, make sure we are not drawing beyond */
    DrawNestedRectangles(x, 0, w, MATRIX_NOF_STEPPERS_Y);
  }
}

static void IntermezzoCharTextLarge(const char *txt, uint8_t xPos) {
#if MATRIX_NOF_STEPPERS_X>=(4*3) && MATRIX_NOF_STEPPERS_Y>=5
  MFONT_PrintString((unsigned char*)txt, xPos, 0, MFONT_SIZE_3x5);
#else
  MFONT_PrintString((unsigned char*)txt, xPos, 0, MFONT_SIZE_2x3);
#endif
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void IntermezzoHSLU(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  IntermezzoCharTextLarge("HSLU", 0);
}

static void IntermezzoCSEM(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  IntermezzoCharTextLarge("CSEM", 0);
}

static void IntermezzoHey(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  IntermezzoCharTextLarge("HEY!", 0);
}

const char *monthStr3[] =
{
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

static void IntermezzoDateBig(void) {
  DATEREC date;
  unsigned char buf[5];
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  McuTimeDate_GetDate(&date);
  buf[0] = '\0';
  McuUtility_strcatNum16uFormatted(buf, sizeof(buf), date.Day, '0', 2);
  McuUtility_chcat(buf, sizeof(buf), '.');
  MFONT_PositionAllToClear();
  IntermezzoCharTextLarge((char*)buf, 2);
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)monthStr3[date.Month-1]);
  MFONT_PositionAllToClear();
  IntermezzoCharTextLarge((char*)buf, 2);
  McuUtility_Num16uToStr(buf, sizeof(buf), date.Year);
  IntermezzoCharTextLarge((char*)buf, 0);
}

static void IntermezzoDateSmall(void) {
  DATEREC date;
  unsigned char buf[8];
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  McuTimeDate_GetDate(&date);
  buf[0] = '\0';
  McuUtility_strcatNum16uFormatted(buf, sizeof(buf), date.Day, '0', 2);
  McuUtility_chcat(buf, sizeof(buf), '.');
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)monthStr3[date.Month-1]);
  MFONT_PositionAllToClear();
  MFONT_PrintString((unsigned char*)buf, 0, 1, MFONT_SIZE_2x3);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  McuUtility_Num16uToStr(buf, sizeof(buf), date.Year);
  MFONT_PositionAllToClear();
  MFONT_PrintString((unsigned char*)buf, 2, 1, MFONT_SIZE_2x3);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

#if PL_CONFIG_HAS_CIRCLE_CLOCK
static void IntermezzoCircleCircle(void) {
  CC_DrawCircle();
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}
#endif /* PL_CONFIG_HAS_CIRCLE_CLOCK */

#if PL_CONFIG_HAS_CIRCLE_CLOCK
static void IntermezzoCircleRays(void) {
  CC_DrawRays();
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}
#endif /* PL_CONFIG_HAS_CIRCLE_CLOCK */

/* #elif-directive for SmartWall Matrix configuration */
/* =============================================================================================================*/
#elif PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_SMARTWALL_16x5

static uint32_t oldColor[PL_CONFIG_NOF_STEPPER_ON_BOARD_X][PL_CONFIG_NOF_STEPPER_ON_BOARD_Y][PL_CONFIG_NOF_STEPPER_ON_BOARD_Z];
static uint32_t newColor[PL_CONFIG_NOF_STEPPER_ON_BOARD_X][PL_CONFIG_NOF_STEPPER_ON_BOARD_Y][PL_CONFIG_NOF_STEPPER_ON_BOARD_Z];

/*!
 * \brief Returns a random background color
 * \param br: brightness of the color in percent
 */
static uint32_t randomBGColorGen(uint8_t br){
	int rgbSet = McuUtility_random(1,7);
	int bright = br;
	int min = 50; /* minimal color value */
	int max = 255; /* maximal color value */
	if(bright>100) bright=100;
	uint32_t color;
	uint8_t r, g, b;
	switch (rgbSet)
	{
		case 1:
			r = McuUtility_random(min,max);
			g = 0;
			b = 0;
			break;
		case 2:
			r = 0;
			g = McuUtility_random(min,max);
			b = 0;
			break;
		case 3:
			r = 0;
			g = 0;
			b = McuUtility_random(min,max);
			break;
		case 4:
			r = McuUtility_random(min,max);
			g = McuUtility_random(min,max);
			b = 0;
			break;
		case 5:
			r = McuUtility_random(min,max);
			g = 0;
			b = McuUtility_random(min,max);
			break;
		case 6:
			r = 0;
			g = McuUtility_random(min,max);
			b = McuUtility_random(min,max);
			break;
		case 7:
			r = McuUtility_random(min,max);
			g = McuUtility_random(min,max);
			b = McuUtility_random(min,max);
			break;
		default:
			r = 0;
			g = 0;
			b = 0;
			break;
	}
	color = NEO_COMBINE_RGB(r,g,b);
	color = NEO_BrightnessPercentColor(color, bright);
	color = NEO_GammaCorrect24(color);
	return color;
}

/*!
 * \brief Returns a random foreground color
 * \param br: brightness of the color in percent
 */
static uint32_t randomFGColorGen(uint8_t br){
	uint32_t color;
	uint8_t r, g, b;
	r = McuUtility_random(0, 255);
	g = McuUtility_random(0, 255);
	b = McuUtility_random(0, 255);
	switch (McuUtility_random(1, 3)) /* Choose red green or blue as main color and add some other colors */
	{
		case 1:
			color=0xff0000;
			color |= (g<<8);
			color |= b;
			break;
		case 2:
			color=0x00ff00;
			color |= (r<<16);
			color |= b;
			break;
		case 3:
			color=0x0000ff;
			color |= (r<<16);
			color |= (g<<8);
		break;
		default:
			break;
	}
	color = NEO_GammaCorrect24(color);
	return NEO_BrightnessPercentColor(color, br );
}

/*!
 * \brief Mixing colors depending on their position
 * \param colorA: rgb color value of color A
 * \param colorB: rgb color value of color B
 * \param steps: number of steps from point A to B (e.g. A-X-X-X-B steps is 4)
 * \param pos: distance between point A and the blend color to be calculated. (e.g. A-X-X-[X] <-pos is 3)
 */
static uint32_t mixColorPos(uint32_t colorA, uint32_t colorB, uint32_t steps, uint32_t pos){
	uint8_t rA, rB, rC, gA, gB, gC, bA, bB, bC;

	rA = (colorA>>16)&0xff;
	gA = (colorA>>8)&0xff;
	bA = colorA&0xff;

	rB = (colorB>>16)&0xff;
	gB = (colorB>>8)&0xff;
	bB = colorB&0xff;

	rC = (rA*((steps)-pos)+rB*pos)/(steps);
	gC = (gA*((steps)-pos)+gB*pos)/(steps);
	bC = (bA*((steps)-pos)+bB*pos)/(steps);

	return NEO_COMBINE_RGB(rC,gC,bC);
}

/*!
 * \brief Fades from a current color to a new color
 * \brief There are no function parameters, since the static arrays are used for this purpose.
 * \param oldColor[][][] Current color, starting from this color is faded.
 * \param newColor[][][] New color, to this color will be faded.
 */
static void intermezzo_FadeColors(void){
	int steps = 255; //100
	uint32_t setColor;
	for(int i = 0; i<=steps; i++){
		for(int x=0; x<PL_CONFIG_NOF_STEPPER_ON_BOARD_X; x++) {
			for(int y=0; y<PL_CONFIG_NOF_STEPPER_ON_BOARD_Y; y++) {
				for(int z=0; z<PL_CONFIG_NOF_STEPPER_ON_BOARD_Z; z++) {
					setColor = mixColorPos(oldColor[x][y][z], newColor[x][y][z], steps, i);
					MPIXEL_SetColor(x, y, z, NEO_SPLIT_RGB(setColor));
				}
			}
		}
		MATRIX_RequestRgbUpdate();
		vTaskDelay(pdMS_TO_TICKS((IntermezzoFadeSec * 1000)/255)); //20
	}
}

static void clearUnusedPixel(void){
	for(int y=0; y<PL_CONFIG_NOF_STEPPER_ON_BOARD_Y; y++) {
		for(int x=PL_CONFIG_NOF_STEPPER_ON_BOARD_X-1; x>=0; x--) {
			for(int z=0; z<PL_CONFIG_NOF_STEPPER_ON_BOARD_Z; z++) {
				if(!LedClock_IsPixelUsed(x, y, z)){
					MPIXEL_SetColor(x, y, z, NEO_SPLIT_RGB(0x0));
				}
			}
		}
	}
	MATRIX_RequestRgbUpdate();
}

static void Intermezzo0(void){
	uint32_t cf;
	uint8_t br = INTERMEZZO_BG_BRIGHTNESS;
	cf = randomFGColorGen(INTERMEZZO_FONT_BRIGHTNESS); /* font color */
	for(int x=0; x<PL_CONFIG_NOF_STEPPER_ON_BOARD_X; x++) {
		for(int y=0; y<PL_CONFIG_NOF_STEPPER_ON_BOARD_Y; y++) {
			for(int z=0; z<PL_CONFIG_NOF_STEPPER_ON_BOARD_Z; z++) {
				oldColor[x][y][z]=MPIXEL_GetColor(x,y,z);
				if(!LedClock_IsPixelUsed(x, y, z)){
					newColor[x][y][z]=randomBGColorGen(br*2); /* set background color */
					STEPPER_MoveMotorStepsAbs(MATRIX_GetStepper(x, y, z), 0, 0); /* set background position */
				}
				else{
					newColor[x][y][z]=cf; /* set font color */
					STEPPER_MoveMotorStepsAbs(MATRIX_GetStepper(x, y, z), STEPPER_FULL_RANGE_NOF_STEPS, 0); /* set font position */
				}
			}
		}
	}

	STEPPER_StartTimer();
	intermezzo_FadeColors();
}

static void Intermezzo1(void) {
	int br = INTERMEZZO_BG_BRIGHTNESS; /*Brightness of intermezzo color (0-100%)*/
	uint32_t c00, c10, c01, c11, cf, ca, cb, color;
	c00 = randomBGColorGen(100); /* background color 1 */
	c10 = randomBGColorGen(100); /* background color 2 */
	c01 = randomBGColorGen(100); /* background color 3 */
	c11 = randomBGColorGen(100); /* background color 4 */
	cf = randomFGColorGen(INTERMEZZO_FONT_BRIGHTNESS); /* font color */
	for(int x=0; x<PL_CONFIG_NOF_STEPPER_ON_BOARD_X; x++) {
		for(int y=0; y<PL_CONFIG_NOF_STEPPER_ON_BOARD_Y; y++) {
			for(int z=0; z<PL_CONFIG_NOF_STEPPER_ON_BOARD_Z; z++) {
				oldColor[x][y][z]=MPIXEL_GetColor(x,y,z);
				if(!LedClock_IsPixelUsed(x, y, z)){
					ca = mixColorPos(c00, c10, (PL_CONFIG_NOF_STEPPER_ON_BOARD_X-1), x);
					cb = mixColorPos(c01, c11, (PL_CONFIG_NOF_STEPPER_ON_BOARD_X-1), x);
					color = mixColorPos(ca, cb, (PL_CONFIG_NOF_STEPPER_ON_BOARD_Y-1), y);
					color = NEO_BrightnessPercentColor(color, br);
					newColor[x][y][z] = color; /* set background color */
					STEPPER_MoveMotorStepsAbs(MATRIX_GetStepper(x, y, z), 0, 0); /* set background position */
				}
				else{
					newColor[x][y][z]=cf; /* set font color */
					STEPPER_MoveMotorStepsAbs(MATRIX_GetStepper(x, y, z), STEPPER_FULL_RANGE_NOF_STEPS, 0); /* set font position */
				}
			}
		}
	}
	STEPPER_StartTimer();
	intermezzo_FadeColors();
}

static void Intermezzo2(void) {
	int br = INTERMEZZO_BG_BRIGHTNESS; /*Brightness of intermezzo color (0-100%)*/
	uint32_t c00, c10, c20, c01, c11, c21, cf, ca, cb, color;
	c00 = randomBGColorGen(100); /* background color 1 */
	c10 = randomBGColorGen(100); /* background color 2 */
	c20 = randomBGColorGen(100); /* background color 3 */
	c01 = randomBGColorGen(100); /* background color 4 */
	c11 = randomBGColorGen(100); /* background color 5 */
	c21 = randomBGColorGen(100); /* background color 6 */
	cf = randomFGColorGen(INTERMEZZO_FONT_BRIGHTNESS); /* font color */

	for(int x=0; x<PL_CONFIG_NOF_STEPPER_ON_BOARD_X; x++) {
		for(int y=0; y<PL_CONFIG_NOF_STEPPER_ON_BOARD_Y; y++) {
			for(int z=0; z<PL_CONFIG_NOF_STEPPER_ON_BOARD_Z; z++) {
				oldColor[x][y][z]=MPIXEL_GetColor(x,y,z);
				if(!LedClock_IsPixelUsed(x, y, z)){
					if(x<(PL_CONFIG_NOF_STEPPER_ON_BOARD_X/2)){
						ca = mixColorPos(c00, c10, (PL_CONFIG_NOF_STEPPER_ON_BOARD_X/2)-1, x);
						cb = mixColorPos(c01, c11, (PL_CONFIG_NOF_STEPPER_ON_BOARD_X/2)-1, x);
					}
					else{
						ca = mixColorPos(c10, c20, (PL_CONFIG_NOF_STEPPER_ON_BOARD_X/2)-1, x-(PL_CONFIG_NOF_STEPPER_ON_BOARD_X/2));
						cb = mixColorPos(c11, c21, (PL_CONFIG_NOF_STEPPER_ON_BOARD_X/2)-1, x-(PL_CONFIG_NOF_STEPPER_ON_BOARD_X/2));
					}
					color = mixColorPos(ca, cb, PL_CONFIG_NOF_STEPPER_ON_BOARD_Y, y);
					color = NEO_BrightnessPercentColor(color, br);
					newColor[x][y][z] = color; /* set background color */
					STEPPER_MoveMotorStepsAbs(MATRIX_GetStepper(x, y, z), 0, 0); /* set background position */
				}
				else{
					newColor[x][y][z]= cf; /* set font color */
					STEPPER_MoveMotorStepsAbs(MATRIX_GetStepper(x, y, z), STEPPER_FULL_RANGE_NOF_STEPS, 0); /* set font position */
				}
			}
		}
	}
	STEPPER_StartTimer();
	intermezzo_FadeColors();
}
static void Intermezzo3(void) {
	int br = INTERMEZZO_BG_BRIGHTNESS; /*Brightness of intermezzo color (0-100%)*/
	uint32_t c00, c10, c01, c11, cf, ca, cb, color;
	c00 = randomBGColorGen(100); /* background color 1 */
	c10 = randomBGColorGen(100); /* background color 2 */
	c01 = randomBGColorGen(100); /* background color 3 */
	c11 = randomBGColorGen(100); /* background color 4 */
	cf = randomFGColorGen(INTERMEZZO_FONT_BRIGHTNESS); /* font color */

	for(int x=0; x<PL_CONFIG_NOF_STEPPER_ON_BOARD_X; x++) {
		for(int y=0; y<PL_CONFIG_NOF_STEPPER_ON_BOARD_Y; y++) {
			for(int z=0; z<PL_CONFIG_NOF_STEPPER_ON_BOARD_Z; z++) {
				oldColor[x][y][z]=MPIXEL_GetColor(x,y,z);
				if(!LedClock_IsPixelUsed(x, y, z)){
					ca = mixColorPos(c00, c10, PL_CONFIG_NOF_STEPPER_ON_BOARD_X-1, x);
					cb = mixColorPos(c01, c11, PL_CONFIG_NOF_STEPPER_ON_BOARD_X-1, x);
					color = mixColorPos(ca, cb, PL_CONFIG_NOF_STEPPER_ON_BOARD_Y-1, y);
					color = NEO_BrightnessPercentColor(color, br);
					newColor[x][y][z] = color; /* set background color */
				}
				else{
					newColor[x][y][z] = cf; /* set font color */
				}
			}
		}
	}
	for(int x=0; x<PL_CONFIG_NOF_STEPPER_ON_BOARD_X; x++) {
		for(int y=0; y<PL_CONFIG_NOF_STEPPER_ON_BOARD_Y; y++) {
			for(int z=0; z<PL_CONFIG_NOF_STEPPER_ON_BOARD_Z; z++) {
				if(!LedClock_IsPixelUsed(x, y, z)){
					STEPPER_MoveMotorStepsAbs(MATRIX_GetStepper(x, y, z), (STEPPER_FULL_RANGE_NOF_STEPS/(PL_CONFIG_NOF_STEPPER_ON_BOARD_Y-1))*y, 0); /* set background position */
				}
				else{
					STEPPER_MoveMotorStepsAbs(MATRIX_GetStepper(x, y, z), STEPPER_FULL_RANGE_NOF_STEPS, 0); /* set font position */
				}
			}
		}
	}
	STEPPER_StartTimer();
	intermezzo_FadeColors();
}

static void Intermezzo4(void) {
	int br = INTERMEZZO_BG_BRIGHTNESS; /*Brightness of intermezzo color (0-100%)*/
	uint32_t c00, c10, c01, c11, cf, ca, cb, color;
	c00 = randomBGColorGen(100); /* background color 1 */
	c10 = randomBGColorGen(100); /* background color 2 */
	c01 = randomBGColorGen(100); /* background color 3 */
	c11 = randomBGColorGen(100); /* background color 4 */
	cf = randomFGColorGen(INTERMEZZO_FONT_BRIGHTNESS); /* font color */

	for(int x=0; x<PL_CONFIG_NOF_STEPPER_ON_BOARD_X; x++) {
		for(int y=0; y<PL_CONFIG_NOF_STEPPER_ON_BOARD_Y; y++) {
			for(int z=0; z<PL_CONFIG_NOF_STEPPER_ON_BOARD_Z; z++) {
				oldColor[x][y][z]=MPIXEL_GetColor(x,y,z);
				if(!LedClock_IsPixelUsed(x, y, z)){
					ca = mixColorPos(c00, c10, PL_CONFIG_NOF_STEPPER_ON_BOARD_X-1, x);
					cb = mixColorPos(c01, c11, PL_CONFIG_NOF_STEPPER_ON_BOARD_X-1, x);
					color = mixColorPos(ca, cb, PL_CONFIG_NOF_STEPPER_ON_BOARD_Y-1, y);
					color = NEO_BrightnessPercentColor(color, br);
					newColor[x][y][z] = color; /* set background color */
				}
				else{
					newColor[x][y][z] = cf; /* set font color */
				}
			}
		}
	}
	for(int x=0; x<PL_CONFIG_NOF_STEPPER_ON_BOARD_X; x++) {
		for(int y=0; y<PL_CONFIG_NOF_STEPPER_ON_BOARD_Y; y++) {
			for(int z=0; z<PL_CONFIG_NOF_STEPPER_ON_BOARD_Z; z++) {
				if(!LedClock_IsPixelUsed(x, y, z)){
					STEPPER_MoveMotorStepsAbs(MATRIX_GetStepper(x, y, z), STEPPER_FULL_RANGE_NOF_STEPS, 0); /* set background position */
				}
				else{
					STEPPER_MoveMotorStepsAbs(MATRIX_GetStepper(x, y, z), 0, 0); /*set font position*/
				}
			}
		}
	}
	STEPPER_StartTimer();
	intermezzo_FadeColors();
}

#endif

typedef void (*Intermezzofp)(void); /* intermezzo function pointer */

typedef struct {
  Intermezzofp fp; /* intermezzo code */
  const char *text; /* description text */
} IntermezzoDesc_t;

static const IntermezzoDesc_t intermezzos[] = {
#if   PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_CLOCK_8x3 \
   || PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_CLOCK_12x5_60B \
   || PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_CLOCK_12x5_MOD \
   || PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_CLOCK_8x3_V4 \
   || PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_CLOCK_16x9_ALEXIS \
   || PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_CIRCULAR_CLOCK_1x12 \
   || PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_SMARTWALL_16x5
  {.fp=Intermezzo0,                 .text="L to not"},
  {.fp=Intermezzo1,                 .text="I clap straight"},
  {.fp=Intermezzo2,                 .text="I rotate"},
  {.fp=Intermezzo3,                 .text="Balance to 12"},
  {.fp=Intermezzo4,                 .text="V rotate down"},
#endif
#if   PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_CLOCK_8x3 \
   || PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_CLOCK_12x5_60B \
   || PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_CLOCK_12x5_MOD \
   || PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_CLOCK_8x3_V4 \
   || PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_CLOCK_16x9_ALEXIS \
   || PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_CIRCULAR_CLOCK_1x12
  {.fp=Intermezzo5,                 .text="12 falling to 6"},
  {.fp=Intermezzo6,                 .text="6 gradually rotating"},
  {.fp=Intermezzo7,                 .text="12 falling and rotate"},
  {.fp=Intermezzo8,                 .text="stars and squares"},
  {.fp=Intermezzo9,                 .text="12 to 6 to 12"},
  {.fp=Intermezzo10,                .text="15 to 6 to 12"},
  {.fp=Intermezzo11,                .text="lines rotating to 12"},
  {.fp=Intermezzo12,                .text="line building from left"},
  {.fp=Intermezzo13,                .text="line building from the top"},
  {.fp=Intermezzo14,                .text="6 with clapping right"},
  {.fp=Intermezzo15,                .text="rotating clock lines"},
  #if PL_CONFIG_USE_RELATIVE_MOVES && !PL_CONFIG_USE_VIRTUAL_STEPPER /* issue with LED virtual steppers following properly, disabled for now */
  {.fp=Intermezzo16,                .text=""},
  {.fp=Intermezzo17,                .text=""},
  {.fp=Intermezzo18,                .text=""},
  {.fp=Intermezzo19,                .text=""},
  {.fp=Intermezzo20,                .text=""},
  {.fp=IntermezzoHalfHalf,          .text="Half-Half"},
  #endif
  {.fp=IntermezzoTime,              .text="Current time on each clock"},
  {.fp=IntermezzoRandomHands,       .text="Random hands"},
  {.fp=IntermezzoRandomHandsAllOn,  .text="Random hands colored"},
  {.fp=IntermezzoTemperature,       .text="Temperature"},
#if PL_CONFIG_USE_SHT31
  {.fp=IntermezzoHumidity,           .text="Humidity"},
#endif
  {.fp=IntermezzoRectangles,        .text="Nested horizontal rectangles"},
  {.fp=IntermezzoRectangles2,       .text="Nested dual rectangles"},
  {.fp=IntermezzoRectangles3,       .text="Left to right rectangles"},
  #if PL_CONFIG_HAS_CIRCLE_CLOCK
  {.fp=IntermezzoCircleCircle,      .text="Circle circle"},
  {.fp=IntermezzoCircleRays,        .text="Circle rays"},
  #endif
  {.fp=IntermezzoDateBig,           .text="Current date big font"},
  {.fp=IntermezzoDateSmall,         .text="Current date small font"},
  {.fp=IntermezzoHSLU,              .text="HSLU"},
  {.fp=IntermezzoCSEM,              .text="CSEM"},
  {.fp=IntermezzoHey,               .text="HEY!"},
#endif
};

#define NOF_INTERMEZZOS   (sizeof(intermezzos)/sizeof(intermezzos[0]))

void INTERMEZZO_Play(TickType_t lastClockUpdateTickCount, bool *intermezzoShown) {
  TickType_t tickCount;
  uint8_t intermezzo;

  if (IntermezzoOn) {
    tickCount = xTaskGetTickCount();
    if (tickCount-lastClockUpdateTickCount > pdMS_TO_TICKS(IntermezzoDelaySec*1000)) { /* after a delay: start intermezzo */
      intermezzo = McuUtility_random(0, NOF_INTERMEZZOS-1);
      McuLog_info("Intermezzo: starting %d", intermezzo);
#if PL_CONFIG_HAS_CIRCLE_CLOCK
      /* exclude center clock from Intermezzos */
      CC_EnableCenterClock(false);
#endif
      intermezzos[intermezzo].fp();
#if PL_CONFIG_HAS_CIRCLE_CLOCK
      /* exclude center clock from Intermezzos */
      CC_EnableCenterClock(true);
#endif
      McuLog_info("Intermezzo: %d done", intermezzo);
      *intermezzoShown = true;
    }
#if PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_SMARTWALL_16x5
  } else {
	clearUnusedPixel(); /*clear the unused pixels in the background */
#endif
  }
}

void INTERMEZZO_PlaySpecific(uint8_t nr) {
  if(nr > NOF_INTERMEZZOS) {
    nr = 0;
  }
  intermezzos[nr].fp();
}

bool INTERMEZZO_IsOn(void){
  return IntermezzoOn;
}

#if PL_CONFIG_USE_SHELL
static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  uint8_t buf[64];

  McuShell_SendStatusStr((unsigned char*)"intermezzo", (unsigned char*)"Intermezzo settings\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  on", IntermezzoOn?(unsigned char*)"yes\r\n":(unsigned char*)"no\r\n", io->stdOut);

  buf[0] = '\0';
  McuUtility_strcatNum32u(buf, sizeof(buf), NOF_INTERMEZZOS);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  nof", buf, io->stdOut);

  buf[0] = '\0';
  McuUtility_strcatNum32u(buf, sizeof(buf), IntermezzoDelaySec);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" s\r\n");
  McuShell_SendStatusStr((unsigned char*)"  delay", buf, io->stdOut);

  #if PL_CONFIG_USE_LED_PIXEL
  buf[0] = '\0';
  McuUtility_strcatNum32u(buf, sizeof(buf), IntermezzoFadeSec);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" s\r\n");
  McuShell_SendStatusStr((unsigned char*)"  fade", buf, io->stdOut);
  #endif

  buf[0] = '\0';
  McuUtility_strcatNum8s(buf, sizeof(buf), GetRtcOffsetTemperature_dC());
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" deci-C offset\r\n");
  McuShell_SendStatusStr((unsigned char*)"  rtc temp", buf, io->stdOut);

  return ERR_OK;
}
#endif

#if PL_CONFIG_USE_SHELL
static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"intermezzo", (unsigned char*)"Group of intermezzo commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  on|off|toggle", (unsigned char*)"Turn intermezzos on, off or toggle\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  delay <sec>", (unsigned char*)"Intermezzo delay in seconds\r\n", io->stdOut);
  #if PL_CONFIG_USE_LED_PIXEL
  McuShell_SendHelpStr((unsigned char*)"  fade <sec>", (unsigned char*)"Intermezzo color fade in seconds\r\n", io->stdOut);
  #endif
  McuShell_SendHelpStr((unsigned char*)"  <nr>", (unsigned char*)"Play Intermezzo (0-", io->stdOut);
  McuShell_SendNum32u(NOF_INTERMEZZOS-1, io->stdOut);
  McuShell_SendStr((unsigned char*)")\r\n", io->stdOut);
  for(int i=0; i<NOF_INTERMEZZOS; i++) {
    unsigned char buf[48];

    McuUtility_Num16uToStr(buf, sizeof(buf), i);
    McuUtility_strcat(buf, sizeof(buf),  (unsigned char*)": ");
    McuUtility_strcat(buf, sizeof(buf),  (unsigned char*)intermezzos[i].text);
    McuUtility_strcat(buf, sizeof(buf),  (unsigned char*)"\r\n");
    McuShell_SendHelpStr((unsigned char*)"", (unsigned char*)buf, io->stdOut);
  }
  McuShell_SendHelpStr((unsigned char*)"  rtc temp offset <dC>", (unsigned char*)"Set RTC temperature offset in deci-Celsius\r\n", io->stdOut);
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
  } else if (McuUtility_strcmp((char*)cmd, "intermezzo on")==0) {
    *handled = true;
    Intermezzo_SetIsOn(true);
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, "intermezzo off")==0) {
    *handled = true;
    Intermezzo_SetIsOn(false);
#if PL_MATRIX_CONFIGURATION_ID == PL_MATRIX_ID_SMARTWALL_16x5
    clearUnusedPixel(); /*clear the unused pixels in the background */
#endif
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, "intermezzo toggle")==0) {
    *handled = true;
    IntermezzoOn = !IntermezzoOn;
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
  #if PL_CONFIG_USE_LED_PIXEL
  } else if (McuUtility_strncmp((char*)cmd, "intermezzo fade ", sizeof("intermezzo fade ")-1)==0) {
	uint8_t sec;

	*handled = TRUE;
	p = cmd + sizeof("intermezzo fade ")-1;
	if (McuUtility_ScanDecimal8uNumber(&p, &sec)==ERR_OK) {
	  IntermezzoFadeSec = sec;
	  return ERR_OK;
	} else {
	  return ERR_FAILED;
	}
  #endif
  } else if (McuUtility_strncmp((char*)cmd, "intermezzo rtc temp offset ", sizeof("intermezzo rtc temp offset ")-1)==0) {
    int8_t offset;

    *handled = TRUE;
    p = cmd + sizeof("intermezzo rtc temp offset ")-1;
    if (McuUtility_ScanDecimal8sNumber(&p, &offset)==ERR_OK) {
      SetRtcOffsetTemperature(offset);
      return ERR_OK;
    } else {
      return ERR_FAILED;
    }
  } else if (McuUtility_strncmp((char*)cmd, "intermezzo ", sizeof("intermezzo ")-1)==0) {
    uint8_t nr;

    *handled = TRUE;
    p = cmd + sizeof("intermezzo ")-1;
    if (McuUtility_ScanDecimal8uNumber(&p, &nr)==ERR_OK && nr<NOF_INTERMEZZOS) {
      intermezzos[nr].fp();
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
