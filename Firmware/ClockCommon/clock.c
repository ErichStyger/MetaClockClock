/*
 * Copyright (c) 2019, 2020, Erich Styger
  *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "clock.h"
#if PL_CONFIG_USE_CLOCK
#include "McuWait.h"
#include "McuRTOS.h"
#if PL_CONFIG_HAS_PARK_BUTTON
  #include "buttons.h"
#endif
#include "leds.h"
#if PL_CONFIG_USE_SHELL
  #include "McuShell.h"
  #include "Shell.h"
  #include "McuShellUart.h"
#endif
#include "McuTimeDate.h"
#if PL_CONFIG_USE_STEPPER
  #include "stepper.h"
#endif
#if PL_CONFIG_USE_MAG_SENSOR
  #include "magnets.h"
#endif
#if PL_CONFIG_USE_NVMC
  #if McuLib_CONFIG_CPU_IS_LPC  /* LPC845-BRK */
  #include "fsl_iap.h"
  #elif McuLib_CONFIG_CPU_IS_KINETIS /* K22FN512 */
  #include "fsl_flash.h"
  #endif
#endif
#include "matrix.h"
#if PL_CONFIG_USE_WDT
  #include "watchdog.h"
#endif
#if PL_CONFIG_USE_INTERMEZZO
  #include "intermezzo.h"
#endif
#if PL_CONFIG_USE_RTC
  #include "McuExtRTC.h"
#endif
#include "StepperBoard.h"
#include "application.h"
#include "matrixposition.h"
#include "matrixhand.h"
#include "matrixring.h"
#include "McuLog.h"
#include "mfont.h"

static bool CLOCK_ClockIsOn = false;
static bool CLOCK_ClockIsParked = false;
static MFONT_Size_e CLOCK_font = MFONT_SIZE_2x3; /* default font */
#if MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5
  static bool CLOCK_clockHasBorder = true; /* if clock has a border (if using small font) */
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  static uint32_t CLOCK_HandColor = 0x000030;
  static uint8_t CLOCK_HandBrightness = 0xff; /* max */
  static bool CLOCK_ShowSeconds = false;
  static uint32_t CLOCK_SecondColor = 0x050000;
#endif

/* direct task notification messages: */
#define CLOCK_TASK_NOTIFY_PARK_ON        (1<<0) /* request to park the motors */
#define CLOCK_TASK_NOTIFY_PARK_OFF       (1<<1) /* request to un-park the motors */
#define CLOCK_TASK_NOTIFY_PARK_TOGGLE    (1<<2) /* request to toggle parking */
#define CLOCK_TASK_NOTIFY_CLOCK_ON       (1<<3) /* request to turn the clock on */
#define CLOCK_TASK_NOTIFY_CLOCK_OFF      (1<<4) /* request to turn the clock off */
#define CLOCK_TASK_NOTIFY_CLOCK_TOGGLE   (1<<5) /* request to toggle clock on/off */

static TaskHandle_t clockTaskHndl;
static uint8_t CLOCK_UpdatePeriodMinutes = 1; /* by default, update clock every minute */

bool CLOCK_GetClockIsOn(void) {
  return CLOCK_ClockIsOn;
}

void CLOCK_Park(CLOCK_Mode_e mode) {
  switch(mode) {
    case CLOCK_MODE_ON:       (void)xTaskNotify(clockTaskHndl, CLOCK_TASK_NOTIFY_PARK_ON, eSetBits); break;
    case CLOCK_MODE_OFF:      (void)xTaskNotify(clockTaskHndl, CLOCK_TASK_NOTIFY_PARK_OFF, eSetBits); break;
    case CLOCK_MODE_TOGGLE:
      if (CLOCK_ClockIsParked) {
        (void)xTaskNotify(clockTaskHndl, CLOCK_TASK_NOTIFY_PARK_OFF, eSetBits);
      } else {
        (void)xTaskNotify(clockTaskHndl, CLOCK_TASK_NOTIFY_PARK_ON, eSetBits);
      }
      break;
    default: break;
  }
}

void CLOCK_On(CLOCK_Mode_e mode) {
  switch(mode) {
    case CLOCK_MODE_ON:       (void)xTaskNotify(clockTaskHndl, CLOCK_TASK_NOTIFY_CLOCK_ON, eSetBits); break;
    case CLOCK_MODE_OFF:      (void)xTaskNotify(clockTaskHndl, CLOCK_TASK_NOTIFY_CLOCK_OFF, eSetBits); break;
    case CLOCK_MODE_TOGGLE:
      if (CLOCK_ClockIsOn) {
        (void)xTaskNotify(clockTaskHndl, CLOCK_TASK_NOTIFY_CLOCK_OFF, eSetBits);
      } else {
        (void)xTaskNotify(clockTaskHndl, CLOCK_TASK_NOTIFY_CLOCK_ON, eSetBits);
      }
      break;
    default: break;
  }
}

#if PL_CONFIG_IS_CLIENT && PL_CONFIG_USE_STEPPER
static void SetTime(STEPPER_Clock_e clock, uint8_t hour, uint8_t minute) {
  #define CLOCK_DEFAULT_DELAY  (5)
  STEPBOARD_Handle_t board = STEPBOARD_GetBoard();
  int32_t angleHour, angleMinute;

  minute %= 60; /* make it 0..59 */
  hour %= 12; /* make it 0..11 */
  angleMinute = (360/60)*minute;
  angleHour = (360/12)*hour + ((360/12)*minute)/60;
  STEPPER_MoveClockDegreeAbs(STEPBOARD_GetStepper(board, clock, STEPPER_HAND_HH), angleHour, STEPPER_MOVE_MODE_CW, CLOCK_DEFAULT_DELAY, true, true);
  STEPPER_MoveClockDegreeAbs(STEPBOARD_GetStepper(board, clock, STEPPER_HAND_MM), angleMinute, STEPPER_MOVE_MODE_CW, CLOCK_DEFAULT_DELAY, true, true);
}
#endif /* PL_CONFIG_USE_STEPPER */

#if PL_CONFIG_IS_CLIENT && PL_CONFIG_USE_STEPPER
static void ShowTime(STEPPER_Clock_e clock, uint8_t hour, uint8_t minute) {
  STEPBOARD_Handle_t board = STEPBOARD_GetBoard();

  SetTime(clock, hour, minute);
  STEPBOARD_MoveAndWait(board, 5);
}
#endif /* PL_CONFIG_USE_STEPPER */

#if PL_CONFIG_USE_SHELL
static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  uint8_t res = ERR_OK;
  uint8_t buf[64];

  McuShell_SendStatusStr((unsigned char*)"clock", (unsigned char*)"Clock settings\r\n", io->stdOut);
#if PL_CONFIG_IS_MASTER
  McuShell_SendStatusStr((unsigned char*)"  mode", (unsigned char*)"master\r\n", io->stdOut);
#else
  McuShell_SendStatusStr((unsigned char*)"  mode", (unsigned char*)"client\r\n", io->stdOut);
#endif
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"V");
  McuUtility_strcatNum8u(buf, sizeof(buf), PL_CONFIG_BOARD_VERSION/10);
  McuUtility_chcat(buf, sizeof(buf), '.');
  McuUtility_strcatNum8u(buf, sizeof(buf), PL_CONFIG_BOARD_VERSION%10);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  board", buf, io->stdOut);

  McuShell_SendStatusStr((unsigned char*)"  clock", CLOCK_GetClockIsOn()?(unsigned char*)"on\r\n":(unsigned char*)"off\r\n", io->stdOut);

  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"every ");
  McuUtility_strcatNum8u(buf, sizeof(buf), CLOCK_UpdatePeriodMinutes);
  McuUtility_strcat(buf, sizeof(buf), CLOCK_UpdatePeriodMinutes==1?(unsigned char*)" minute\r\n":(unsigned char*)" minutes\r\n");
  McuShell_SendStatusStr((unsigned char*)"  period", buf, io->stdOut);

#if PL_CONFIG_USE_NEO_PIXEL_HW
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"color: 0x");
  McuUtility_strcatNum24Hex(buf, sizeof(buf), CLOCK_HandColor);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)", brightness 0x");
  McuUtility_strcatNum8Hex(buf, sizeof(buf), CLOCK_HandBrightness);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  hand", buf, io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  seconds", CLOCK_ShowSeconds?(unsigned char*)"on\r\n":(unsigned char*)"off\r\n", io->stdOut);
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"color: 0x");
  McuUtility_strcatNum24Hex(buf, sizeof(buf), CLOCK_SecondColor);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  second", buf, io->stdOut);
#endif
#if MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5
  McuShell_SendStatusStr((unsigned char*)"  border", CLOCK_clockHasBorder?(unsigned char*)"on\r\n":(unsigned char*)"off\r\n", io->stdOut);
#endif
  MFONT_FontToStr(CLOCK_font, buf, sizeof(buf));
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  font", buf, io->stdOut);

#if PL_CONFIG_USE_NVMC
#if McuLib_CONFIG_CPU_IS_LPC  /* LPC845-BRK */
  uint32_t val;
  uint32_t id[4];

  res = IAP_ReadPartID(&val);
  if (res == kStatus_IAP_Success) {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"0x");
    McuUtility_strcatNum32Hex(buf, sizeof(buf), val);
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  } else {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"ERROR\r\n");
  }
  McuShell_SendStatusStr((unsigned char*)"  PartID", buf, io->stdOut);

  res = IAP_ReadUniqueID(&id[0]);  /* \todo in McuArmTools now, could be removed */
  if (res == kStatus_IAP_Success) {
    buf[0] = '\0';
    for(int i=0; i<sizeof(id)/sizeof(id[0]); i++) {
      McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"0x");
      McuUtility_strcatNum32Hex(buf, sizeof(buf), id[i]);
      McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" ");
    }
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  } else {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"ERROR\r\n");
  }
  McuShell_SendStatusStr((unsigned char*)"  UID", buf, io->stdOut);

  res = IAP_ReadBootCodeVersion(&val);
  if (res == kStatus_IAP_Success) {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"Version 0x");
    McuUtility_strcatNum32Hex(buf, sizeof(buf), val);
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  } else {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"ERROR\r\n");
  }
  McuShell_SendStatusStr((unsigned char*)"  BootCode", buf, io->stdOut);
  #endif
#endif
  return res;
}
#endif

#if PL_CONFIG_USE_SHELL
static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"clock", (unsigned char*)"Group of clock commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  on|off|toggle", (unsigned char*)"Enable or disable the clock\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  park on|off|toggle", (unsigned char*)"Turns clock off and moves to park position, ready to power off\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  period <minute>", (unsigned char*)"Clock update period in minutes (>0)\r\n", io->stdOut);
#if PL_CONFIG_USE_NEO_PIXEL_HW
  McuShell_SendHelpStr((unsigned char*)"  hand rgb <rgb>", (unsigned char*)"Set hand color\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  seconds on|off", (unsigned char*)"Show seconds\r\n", io->stdOut);
#endif
#if PL_CONFIG_USE_LED_DIMMING
  McuShell_SendHelpStr((unsigned char*)"  brightness <v>", (unsigned char*)"Set hand brightness factor (0-255)\r\n", io->stdOut);
#endif
#if MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5
  McuShell_SendHelpStr((unsigned char*)"  border on|off", (unsigned char*)"Show clock with border\r\n", io->stdOut);
#endif
  McuShell_SendHelpStr((unsigned char*)"  font <f>", (unsigned char*)"Set clock font, e.g. 2x3\r\n", io->stdOut);
#if PL_CONFIG_IS_CLIENT && PL_CONFIG_USE_STEPPER
  McuShell_SendHelpStr((unsigned char*)"  time <c> <time>", (unsigned char*)"Show time on clock (0..3)\r\n", io->stdOut);
#endif
#if PL_CONFIG_WORLD_CLOCK
  McuShell_SendHelpStr((unsigned char*)"  clocks", (unsigned char*)"[0] London    [3] New York\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"",         (unsigned char*)"[1] Beijing   [2] Lucerne\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  hands",  (unsigned char*)"[0] MM,inner  [1] HH,outer\r\n", io->stdOut);
#endif
  return ERR_OK;
}
#endif

#if PL_CONFIG_USE_SHELL
uint8_t CLOCK_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  const unsigned char *p;

  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "clock help")==0) {
    *handled = true;
    return PrintHelp(io);
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "clock status")==0)) {
    *handled = true;
    return PrintStatus(io);
  } else if (McuUtility_strcmp((char*)cmd, "clock on")==0) {
    *handled = true;
    CLOCK_On(CLOCK_MODE_ON);
  } else if (McuUtility_strcmp((char*)cmd, "clock off")==0) {
    *handled = true;
    CLOCK_On(CLOCK_MODE_OFF);
  } else if (McuUtility_strcmp((char*)cmd, "clock toggle")==0) {
    *handled = true;
    CLOCK_On(CLOCK_MODE_TOGGLE);
  } else if (McuUtility_strcmp((char*)cmd, "clock park on")==0) {
    *handled = true;
    CLOCK_Park(CLOCK_MODE_ON);
  } else if (McuUtility_strcmp((char*)cmd, "clock park off")==0) {
    *handled = true;
    CLOCK_Park(CLOCK_MODE_OFF);
  } else if (McuUtility_strcmp((char*)cmd, "clock park toggle")==0) {
    *handled = true;
    CLOCK_Park(CLOCK_MODE_TOGGLE);
  } else if (McuUtility_strncmp((char*)cmd, "clock period ", sizeof("clock period ")-1)==0) {
    uint8_t val;

    *handled = TRUE;
    p = cmd + sizeof("clock period ")-1;
    if (McuUtility_ScanDecimal8uNumber(&p, &val)==ERR_OK && val>0) {
      CLOCK_UpdatePeriodMinutes = val;
    } else {
      return ERR_FAILED;
    }
    return ERR_OK;
#if PL_CONFIG_USE_NEO_PIXEL_HW
  } else if (McuUtility_strncmp((char*)cmd, "clock hand rgb ", sizeof("clock hand rgb ")-1)==0) {
    *handled = TRUE;
    p = cmd + sizeof("clock hand rgb ")-1;
    if (McuUtility_ScanRGB32(&p, &CLOCK_HandColor)==ERR_OK) {
      MHAND_SetHandColorAll(NEO_COMBINE_RGB((CLOCK_HandColor>>16)&0xff, (CLOCK_HandColor>>8)&0xff, CLOCK_HandColor&0xff));
      APP_RequestUpdateLEDs();
      return ERR_OK;
    } else {
      return ERR_FAILED;
    }
  } else if (McuUtility_strncmp((char*)cmd, "clock second rgb ", sizeof("clock second rgb ")-1)==0) {
    *handled = TRUE;
    p = cmd + sizeof("clock second rgb ")-1;
    if (McuUtility_ScanRGB32(&p, &CLOCK_SecondColor)==ERR_OK) {
      return ERR_OK;
    } else {
      return ERR_FAILED;
    }
  } else if (McuUtility_strcmp((char*)cmd, "clock seconds on")==0) {
    *handled = TRUE;
    CLOCK_ShowSeconds = true;
  } else if (McuUtility_strcmp((char*)cmd, "clock seconds off")==0) {
    *handled = TRUE;
    CLOCK_ShowSeconds = false;
#endif
#if PL_CONFIG_USE_LED_DIMMING
  } else if (McuUtility_strncmp((char*)cmd, "clock brightness ", sizeof("clock brightness ")-1)==0) {
    *handled = TRUE;
    p = cmd + sizeof("clock brightness ")-1;
    if (McuUtility_ScanDecimal8uNumber(&p, &CLOCK_HandBrightness)==ERR_OK) {
      MATRIX_SetHandBrightnessAll(CLOCK_HandBrightness);
      APP_RequestUpdateLEDs(); /* update LEDs */
    } else {
      return ERR_FAILED;
    }
    return ERR_OK;
#endif /* PL_CONFIG_USE_LED_DIMMING */
  } else if (McuUtility_strncmp((char*)cmd, "clock font ", sizeof("clock font")-1)==0) {
    *handled = true;
    p = cmd + sizeof("clock font ")-1;
    MFONT_ParseFontName(&p, &CLOCK_font);
    if (CLOCK_font==MFONT_SIZE_ERROR) {
      CLOCK_font = MFONT_SIZE_2x3;
      return ERR_FAILED;
    }
#if MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5
  } else if (McuUtility_strncmp((char*)cmd, "clock border ", sizeof("clock border ")-1)==0) {
    *handled = true;
    p = cmd + sizeof("clock border ")-1;
    if (McuUtility_strcmp((char*)p, "on")==0) {
      CLOCK_clockHasBorder = true;
      return ERR_OK;
    } else if (McuUtility_strcmp((char*)p, "off")==0) {
      CLOCK_clockHasBorder = false;
      return ERR_OK;
    }
    return ERR_FAILED;
#endif
#if PL_CONFIG_IS_CLIENT && PL_CONFIG_USE_STEPPER
  } else if (McuUtility_strncmp((char*)cmd, "clock time ", sizeof("clock time ")-1)==0) {
    uint8_t hour, minute, second, hsec;
    int32_t val;

    *handled = TRUE;
    p = cmd + sizeof("clock time ")-1;
    if (
           McuUtility_xatoi(&p, &val)==ERR_OK && val>=0 && val<=3
        && McuUtility_ScanTime(&p, &hour, &minute, &second, &hsec)==ERR_OK
       )
    {
      ShowTime(val, hour, minute);
    } else {
      return ERR_FAILED;
    }
    return ERR_OK;
#endif /* PL_CONFIG_USE_STEPPER */
  }
  return ERR_OK;
}
#endif /* #if PL_CONFIG_USE_SHELL */

#if PL_CONFIG_WORLD_CLOCK
static uint8_t AdjustHourForTimeZone(uint8_t hour, int8_t gmtDelta) {
  int h;

  h = (int)hour+gmtDelta;
  if (h<0) {
    h = 24+h;
  }
  h %= 24;
  return h;
}
#endif

#if PL_CONFIG_USE_NEO_PIXEL_HW
static void ShowSeconds(const TIMEREC *time) {
  static uint8_t lastSecondShown = -1;

  if (!CLOCK_ShowSeconds) {
    return; /* disabled */
  }
  if (time->Sec != lastSecondShown) {
    int x, y;
    uint8_t red, green, blue;

    if (lastSecondShown!=-1) { /* turn off previous second */
      x = lastSecondShown%12;
      y = lastSecondShown/12;
      MRING_EnableRing(x, y, 0, false);
      MRING_SetRingColor(x, y, 0, 0, 0, 0);
    }
    lastSecondShown = time->Sec;
    red = CLOCK_SecondColor>>16; green = (CLOCK_SecondColor>>8)&0xff; blue = CLOCK_SecondColor&0xff;
    x = lastSecondShown%12;
    y = lastSecondShown/12;
    MRING_EnableRing(x, y, 0, true);
    MRING_SetRingColor(x, y, 0, red, green, blue);
    APP_RequestUpdateLEDs();
  }
}
#endif

static void ClockTask(void *pv) {
#if PL_CONFIG_USE_RTC
  int32_t prevClockUpdateTimestampSec = 0; /* time of previous clock update time stamp, seconds since 1972 */
  TIMEREC time;
  DATEREC date;
  uint8_t res;
  TickType_t lastUpdateFromRTCtickCount; /* time stamp when last time the SW RTC has been update from HW RTC: it gets updated every hour */
#endif
#if PL_CONFIG_USE_INTERMEZZO
  TickType_t lastClockUpdateTickCount = -1; /* tick count when the clock has been updated the last time */
  bool intermezzoShown = false;
#endif
  uint32_t ulNotificationValue;

  res = McuTimeDate_Init();
  if(res==ERR_OK) { /* initialize time from external RTC if configured with McuTimeDate_INIT_SOFTWARE_RTC_FROM_EXTERNAL_RTC */
    lastUpdateFromRTCtickCount = xTaskGetTickCount(); /* remember last time we updated the RTC */
  } else{
    McuLog_error("Failed initializing time from RTC!");
    lastUpdateFromRTCtickCount = 0; /* set it to zero: will retry in the main loop below */
  }
  McuLog_trace("Starting Clock Task");
#if PL_CONFIG_USE_WDT
  WDT_SetTaskHandle(WDT_REPORT_ID_TASK_CLOCK, xTaskGetCurrentTaskHandle());
#endif
  vTaskDelay(pdMS_TO_TICKS(2000)); /* give external RTC and hardware time to power up */
#if PL_CONFIG_USE_WDT
  WDT_Report(WDT_REPORT_ID_TASK_CLOCK, 2000);
#endif
#if PL_CONFIG_USE_SHELL
  #if McuLib_CONFIG_CPU_IS_LPC && PL_CONFIG_IS_MASTER
  SHELL_SendString((unsigned char*)"\r\n*****************\r\n* LPC845 Master *\r\n*****************\r\n");
  #elif McuLib_CONFIG_CPU_IS_LPC && PL_CONFIG_IS_CLIENT
  SHELL_SendString((unsigned char*)"\r\n*****************\r\n* LPC845 Client *\r\n*****************\r\n");
  #elif McuLib_CONFIG_CPU_IS_KINETIS && PL_CONFIG_IS_CLIENT
  SHELL_SendString((unsigned char*)"\r\n******************\r\n* tinyK22 Client *\r\n******************\r\n");
  #elif McuLib_CONFIG_CPU_IS_KINETIS && PL_CONFIG_IS_MASTER
  SHELL_SendString((unsigned char*)"\r\n******************\r\n* tinyK22 Master *\r\n******************\r\n");
  #endif
#endif
#if 0 && PL_CONFIG_USE_STEPPER
  if (STEPPER_ZeroAllHands()!=ERR_OK) {
    McuLog_error("Failed to zero all hands!");
  }
#endif
#if PL_CONFIG_WORLD_CLOCK
  if (CLOCK_ClockIsOn) {
    vTaskDelay(pdMS_TO_TICKS(2000)); /* just some delay */
  #if PL_CONFIG_USE_WDT
    WDT_Report(WDT_REPORT_ID_TASK_CLOCK, 2000);
  #endif
  }
#endif

#if PL_CONFIG_USE_NEO_PIXEL_HW
  /* turn on the hand LEDs */
  MHAND_SetHandColorAll(NEO_COMBINE_RGB((CLOCK_HandColor>>16)&0xff, (CLOCK_HandColor>>8)&0xff, CLOCK_HandColor&0xff));
#if PL_CONFIG_USE_LED_DIMMING
  MATRIX_SetHandBrightnessAll(CLOCK_HandBrightness);
#endif
  MHAND_HandEnableAll(true);
  APP_RequestUpdateLEDs(); /* update LEDs */
#endif
#if PL_CONFIG_USE_RTC
  /* set new random seed based temperature */
  float temperature;

  if (McuExtRTC_GetTemperature(&temperature)!=ERR_OK) {
    McuLog_error("failed getting RTC temperature");
  } else {
    uint32_t seed;

    seed = (((int)temperature)<<16)+(int)(temperature)*64;
    McuUtility_randomSetSeed(seed);
  }
#endif
#if PL_CONFIG_IS_MASTER && PL_CONFIG_USE_MOTOR_ON_OFF /* turn on motors */
  (void)SHELL_ParseCommand((const unsigned char *)"matrix motor on", NULL, true);
#endif
  for(;;) {
    vTaskDelay(pdMS_TO_TICKS(200));
  #if PL_CONFIG_USE_WDT
    WDT_Report(WDT_REPORT_ID_TASK_CLOCK, 200);
  #endif
    /* check task notifications */
    res = xTaskNotifyWait(
       0, /* do not clear anything on enter */
        CLOCK_TASK_NOTIFY_PARK_ON|CLOCK_TASK_NOTIFY_PARK_OFF|CLOCK_TASK_NOTIFY_PARK_TOGGLE
       |CLOCK_TASK_NOTIFY_CLOCK_ON|CLOCK_TASK_NOTIFY_CLOCK_OFF|CLOCK_TASK_NOTIFY_CLOCK_TOGGLE, /* clear on exit */
       &ulNotificationValue,
       0);
    if (res==pdTRUE) { /* notification received */
      if (ulNotificationValue&CLOCK_TASK_NOTIFY_PARK_ON) {
        McuLog_info("Start parking clock");
        SHELL_ParseCommand((unsigned char*)"matrix park on", McuShell_GetStdio(), true); /* move to 12-o-clock position */
        McuLog_info("Parking done.");
        CLOCK_ClockIsOn = false; /* disabled clock */
        CLOCK_ClockIsParked = true;
      }
      if (ulNotificationValue&CLOCK_TASK_NOTIFY_PARK_OFF) {
        McuLog_info("Start unparking clock");
        SHELL_ParseCommand((unsigned char*)"matrix park off", McuShell_GetStdio(), true); /* move to 12-o-clock position */
        McuLog_info("Unparking done.");
        CLOCK_ClockIsOn = false; /* disabled clock */
        CLOCK_ClockIsParked = false;
      }
      if (ulNotificationValue&CLOCK_TASK_NOTIFY_CLOCK_ON) {
        McuLog_info("Clock on");
        CLOCK_ClockIsOn = true; /* enable clock */
        prevClockUpdateTimestampSec = 0; /* to make sure it will update */
      }
      if (ulNotificationValue&CLOCK_TASK_NOTIFY_CLOCK_OFF) {
        McuLog_info("Clock off");
        CLOCK_ClockIsOn = false; /* disable clock */
        prevClockUpdateTimestampSec = 0;
    #if PL_CONFIG_USE_NEO_PIXEL_HW
        /* turn off LEDs */
        MRING_SetRingColorAll(0, 0, 0);
        MHAND_HandEnableAll(false);
      #if PL_CONFIG_USE_EXTENDED_HANDS
        MHAND_2ndHandEnableAll(false);
      #endif
        APP_RequestUpdateLEDs(); /* update LEDs */
    #endif
      }
    }
  #if PL_CONFIG_USE_RTC
    /* ----------------------------------------------------------------------------------*/
    /* Because the SW RTC might run off, we update the SW RTC from the HW RTC every hour */
    TickType_t tickCount = xTaskGetTickCount();
    /* update SW RTC from external RTC */
    if ((tickCount-lastUpdateFromRTCtickCount) > pdMS_TO_TICKS(60*60*1000)) { /* every hour */
      McuLog_info("Updating RTC from external RTC");
      res = McuTimeDate_SyncWithExternalRTC(); /* update SW RTC with external HW RTC to avoid too much clock drift */
      if (res!=ERR_OK) {
        McuLog_error("Updating RTC from external RTC");
      }
      lastUpdateFromRTCtickCount = tickCount;
    }
    /* ----------------------------------------------------------------------------------*/
    /* Intermezzo */
    /* ----------------------------------------------------------------------------------*/
    if (CLOCK_ClockIsOn) {
      #if PL_CONFIG_USE_INTERMEZZO
      if (!intermezzoShown) { /* not shown intermezzo? */
        INTERMEZZO_Play(lastClockUpdateTickCount, &intermezzoShown);
        if (intermezzoShown) {
          prevClockUpdateTimestampSec = 0; /* if there is time after the intermezzo: trigger showing clock again */
        }
      }
      #endif
    } /* if clock is on */
    /* ----------------------------------------------------------------------------------*/
    /* Clock */
    /* ----------------------------------------------------------------------------------*/
    if (CLOCK_ClockIsOn) { /* show time */
      res = McuTimeDate_GetTimeDate(&time, &date);
    #if PL_CONFIG_USE_NEO_PIXEL_HW
      ShowSeconds(&time);
    #endif
      if (res==ERR_OK && (McuTimeDate_TimeDateToUnixSeconds(&time, &date, 0) >= prevClockUpdateTimestampSec+60*CLOCK_UpdatePeriodMinutes)) {
    #if PL_CONFIG_IS_MASTER
        uint8_t buf[8];

        McuLog_info("Time: %02d:%02d, Date: %02d-%02d-%04d", time.Hour, time.Min, date.Day, date.Month, date.Year);
        MATRIX_SetMoveDelayAll(5);
        MPOS_SetMoveModeAll(STEPPER_MOVE_MODE_SHORT);
      #if PL_CONFIG_USE_NEO_PIXEL_HW
        MHAND_SetHandColorAll(NEO_COMBINE_RGB((CLOCK_HandColor>>16)&0xff, (CLOCK_HandColor>>8)&0xff, CLOCK_HandColor&0xff));
      #endif
        buf[0] = '\0';
        McuUtility_strcatNum16uFormatted(buf, sizeof(buf), time.Hour, '0', 2);
        McuUtility_strcatNum16uFormatted(buf, sizeof(buf), time.Min, '0', 2);
      #if MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5
        res = MFONT_ShowFramedText(0, 0, buf, CLOCK_font, CLOCK_clockHasBorder, true);
      #else
        res = MFONT_ShowFramedText(0, 0, buf, CLOCK_font, false, true);
      #endif
        if (res!=ERR_OK) {
          McuLog_error("Failed showing time");
        }
    #endif /* PL_CONFIG_IS_MASTER */
    #if PL_CONFIG_WORLD_CLOCK
        uint8_t hour;

        hour = AdjustHourForTimeZone(time.Hour, -1); /* local time is GMT+1 */
        SetTime(STEPPER_CLOCK_0, AdjustHourForTimeZone(hour, 0), time.Min); /* London, GMT+0, top left */
        SetTime(STEPPER_CLOCK_1, AdjustHourForTimeZone(hour, 8), time.Min); /* Beijing, GMT+8, top left */
        SetTime(STEPPER_CLOCK_2, AdjustHourForTimeZone(hour, 1), time.Min); /* Lucerne, GMT+1, top left */
        SetTime(STEPPER_CLOCK_3, AdjustHourForTimeZone(hour, -4), time.Min); /* New York,, GMT-4, top left */
        STEPBOARD_MoveAndWait(board, 5);
    #endif /* PL_CONFIG_WORLD_CLOCK */
        prevClockUpdateTimestampSec = McuTimeDate_TimeDateToUnixSeconds(&time, &date, 0);
     #if PL_CONFIG_USE_INTERMEZZO /* show intermezzo? */
        lastClockUpdateTickCount = xTaskGetTickCount();
        intermezzoShown = false;
      #endif
        McuLog_info("finished showing clock");
      } /* if */
    } /* if clock is on */
  #endif /* PL_CONFIG_USE_RTC */
  } /* for */
}

void CLOCK_Init(void) {
  if (xTaskCreate(
      ClockTask,  /* pointer to the task */
      "Clock", /* task name for kernel awareness debugging */
      2500/sizeof(StackType_t), /* task stack size */
      (void*)NULL, /* optional task startup argument */
      tskIDLE_PRIORITY+1,  /* initial priority */
      &clockTaskHndl /* optional task handle to create */
    ) != pdPASS) {
     McuLog_fatal("failed creating Clock task");
     for(;;){} /* error! probably out of memory */
  }
}

#endif /* PL_CONFIG_USE_CLOCK */
