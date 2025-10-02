/*
 * Copyright (c) 2019-2025, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "clock.h"
#if PL_CONFIG_USE_CLOCK
#include "McuWait.h"
#include "McuRTOS.h"
#if PL_CONFIG_HAS_BUTTONS
  #include "buttons.h"
  #include "McuButton.h"
#endif
#include "leds.h"
#if PL_CONFIG_USE_SHELL
  #include "McuShell.h"
  #include "shell.h"
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
#if PL_CONFIG_USE_WDT
  #include "watchdog.h"
#endif
#if PL_CONFIG_USE_INTERMEZZO
  #include "intermezzo.h"
#endif
#if PL_CONFIG_USE_EXT_I2C_RTC
  #include "McuExtRTC.h"
#endif
#if PL_CONFIG_HAS_CIRCLE_CLOCK
  #include "circleClock.h"
#endif
#include "application.h"
#include "McuLog.h"
#if PL_CONFIG_USE_MATRIX
  #include "StepperBoard.h"
  #include "matrix.h"
  #include "matrixposition.h"
  #include "matrixhand.h"
  #include "matrixring.h"
#endif
#include "McuLog.h"
#if PL_CONFIG_USE_FONT
  #include "mfont.h"
#endif
#if PL_CONFIG_USE_LED_CLOCK
  #include "LedClock.h"
#endif
#if PL_CONFIG_USE_LED_PIXEL
  #include "pixel.h"
#endif
#if PL_CONFIG_USE_DEMOS
  #include "demos.h"
#endif
#if PL_CONFIG_USE_MININI
  #include "minIni/McuMinINI.h"
  #include "MinIniKeys.h"
#endif
#if PL_CONFIG_USE_ESP_TIME
  #include "esp_time.h"
#endif

#define CLOCK_CONFIG_UPDATE_SW_RTC_FROM_HW_RTC_PERIOD_MINUTES    (20) /* update SW RTC from HW RTC every x minutes. Seen a drift of 3 seconds on K20 every hour. */

static bool CLOCK_ClockIsOn =
  #if PL_CONFIG_CLOCK_ON_BY_DEFAULT
    true;
  #else
    false;
  #endif
static bool CLOCK_ClockIs24h = true; /* if showing time in 24h format (17:35) or 12h format (5:35) */
static bool CLOCK_ClockIsParked = false;

#if PL_CONFIG_IS_CLOCK_CLOCK
#if PL_CONFIG_USE_FONT
  static MFONT_Size_e CLOCK_font = PL_CONFIG_CLOCK_DEFAULT_FONT; /* default font */
#endif
#if MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5
  static bool CLOCK_clockHasBorder = true; /* if clock has a border (if using small font) */
#endif
#if PL_CONFIG_USE_LED_RING
  static uint32_t CLOCK_HandColor = PL_CONFIG_CLOCK_DEFAULT_HAND_COLOR;
  static bool CLOCK_doRandomHandColor = false;
  static bool CLOCK_doFadingHands = false;
#endif
#if PL_CONFIG_USE_LED_DIMMING
  static uint8_t CLOCK_HandBrightness = 0xff; /* max */
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  static bool CLOCK_ShowSeconds = false;
  static uint32_t CLOCK_SecondColor = 0x050000;
#endif
#endif /* PL_CONFIG_IS_CLOCK_CLOCK */

#if PL_CONFIG_USE_CLOCK_TIME_OFF
  /* default values for off-hours */
  #define CONFIG_CLOCK_DEFAULT_ON_OFF         (false)
  #define CONFIG_CLOCK_DEFAULT_OFF_START_HH   (15)
  #define CONFIG_CLOCK_DEFAULT_OFF_START_MM   (0)
  #define CONFIG_CLOCK_DEFAULT_OFF_END_HH     (7)
  #define CONFIG_CLOCK_DEFAULT_OFF_END_MM     (15)
  #define CONFIG_CLOCK_DEFAULT_OFF_DAYS       (0) /* no off days. Bit 0 would be Sunday */
  static struct CLOCK_TimeOff {
    bool isTimeOnOffEnabled; /* if range for automatically turning on and off is enabled */
    TIMEREC offStartTime;
    TIMEREC offEndTime;
    bool offIsActive; /* if clock is off because of off-time */
    uint8_t offDays; /* bit set, with 0x1 as Sunday, 0x2 as Monday, 0x4 as Tuesday ... */
  } CLOCK_TimeOff =
  {
      .isTimeOnOffEnabled = CONFIG_CLOCK_DEFAULT_ON_OFF,
      .offStartTime.Hour = CONFIG_CLOCK_DEFAULT_OFF_START_HH,
      .offStartTime.Min = CONFIG_CLOCK_DEFAULT_OFF_START_MM,
      .offEndTime.Hour = CONFIG_CLOCK_DEFAULT_OFF_END_HH,
      .offEndTime.Min = CONFIG_CLOCK_DEFAULT_OFF_END_MM,
      .offIsActive = false,
      .offDays = CONFIG_CLOCK_DEFAULT_OFF_DAYS,
  };
#endif

/* direct task notification messages: */
#define CLOCK_TASK_NOTIFY_PARK_ON             (1<<0) /* request to park the motors */
#define CLOCK_TASK_NOTIFY_PARK_OFF            (1<<1) /* request to un-park the motors */
#define CLOCK_TASK_NOTIFY_PARK_TOGGLE         (1<<2) /* request to toggle parking */
#define CLOCK_TASK_NOTIFY_CLOCK_ON            (1<<3) /* request to turn the clock on */
#define CLOCK_TASK_NOTIFY_CLOCK_OFF           (1<<4) /* request to turn the clock off */
#define CLOCK_TASK_NOTIFY_CLOCK_TOGGLE        (1<<5) /* request to toggle clock on/off */
#define CLOCK_TASK_NOTIFY_BUTTON_USR          (1<<6) /* request to toggle clock on/off */
#define CLOCK_TASK_NOTIFY_BUTTON_USR_LONG     (1<<7) /* request to toggle clock on/off */
#define CLOCK_TASK_NOTIFY_UPDATE_CLOCK        (1<<8) /* request to update clock */
#if PL_CONFIG_HAS_SWITCH_7WAY
  #define CLOCK_TASK_NOTIFY_BUTTON_UP           (1<<9) /* up button */
  #define CLOCK_TASK_NOTIFY_BUTTON_DOWN         (1<<10) /* down button */
  #define CLOCK_TASK_NOTIFY_BUTTON_LEFT         (1<<11) /* left button */
  #define CLOCK_TASK_NOTIFY_BUTTON_RIGHT        (1<<12) /* right button */
  #define CLOCK_TASK_NOTIFY_BUTTON_MID          (1<<13) /* middle button */
  #define CLOCK_TASK_NOTIFY_BUTTON_RST          (1<<14) /* reset button */
  #define CLOCK_TASK_NOTIFY_BUTTON_SET          (1<<15) /* set button */
  #define CLOCK_TASK_NOTIFY_ALL                ((1<<16)-1) /* all notification bits */
#elif PL_CONFIG_HAS_SWITCH_2WAY
  #define CLOCK_TASK_NOTIFY_BUTTON_UP           (1<<9) /* up button */
  #define CLOCK_TASK_NOTIFY_BUTTON_DOWN         (1<<10) /* down button */
  #define CLOCK_TASK_NOTIFY_ALL                ((1<<11)-1) /* all notification bits */
#else
  #define CLOCK_TASK_NOTIFY_ALL                 ((1<<9)-1) /* all notification bits */
#endif

static TaskHandle_t clockTaskHndl;
#if 0 /* not implemented yet */
static uint8_t CLOCK_UpdatePeriodMinutes = 1; /* by default, update clock every minute */
#endif

#if PL_CONFIG_USE_CLOCK_TIME_OFF
static void SetOffDays(uint8 dayBits) { /* 0x1: Sunday, 0x2: Monday, ... */
#if PL_CONFIG_USE_MININI
  McuMinINI_ini_putl(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_OFF_DAYS, dayBits, NVMC_MININI_FILE_NAME);
#endif
  CLOCK_TimeOff.offDays = dayBits;
}
#endif /* PL_CONFIG_USE_CLOCK_TIME_OFF */

#if PL_CONFIG_USE_CLOCK_TIME_OFF
static uint8_t GetOffDays(void) { /* 0x1: Sunday, 0x2: Monday, ... */
  return CLOCK_TimeOff.offDays;
}
#endif /* PL_CONFIG_USE_CLOCK_TIME_OFF */

#if PL_CONFIG_USE_LED_RING
static void SetDoRandomHandColor(bool enable) {
#if PL_CONFIG_USE_MININI
  McuMinINI_ini_putl(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_RANDOM_HAND_COLOR, enable, NVMC_MININI_FILE_NAME);
#endif
  CLOCK_doRandomHandColor = enable;
}
#endif

#if PL_CONFIG_USE_LED_RING
static bool GetDoRandomHandColor(void) {
  return CLOCK_doRandomHandColor;
}
#endif

#if PL_CONFIG_USE_LED_RING
static void SetDoFadingHands(bool enable) {
#if PL_CONFIG_USE_MININI
  McuMinINI_ini_putl(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_FADING_HANDS, enable, NVMC_MININI_FILE_NAME);
#endif
  CLOCK_doFadingHands = enable;
}
#endif

#if PL_CONFIG_USE_LED_RING
static bool GetDoFadingHands(void) {
  return CLOCK_doFadingHands;
}
#endif

#if PL_CONFIG_USE_CLOCK_TIME_OFF
static uint8_t SetClock_OnOff(bool onOff) {
  CLOCK_TimeOff.isTimeOnOffEnabled = onOff;
#if PL_CONFIG_USE_MININI
  McuMinINI_ini_putl(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_ON_OFF, onOff, NVMC_MININI_FILE_NAME);
#endif
  return ERR_OK;
}
#endif

#if PL_CONFIG_USE_CLOCK_TIME_OFF
static uint8_t SetClock_OffStart(TIMEREC *time) {
  CLOCK_TimeOff.offStartTime.Hour = time->Hour;
  CLOCK_TimeOff.offStartTime.Min = time->Min;
#if PL_CONFIG_USE_MININI
  McuMinINI_ini_putl(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_OFF_START_HH, time->Hour, NVMC_MININI_FILE_NAME);
  McuMinINI_ini_putl(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_OFF_START_MM, time->Min, NVMC_MININI_FILE_NAME);
#endif
  return ERR_OK;
}
#endif

#if PL_CONFIG_USE_CLOCK_TIME_OFF
static uint8_t SetClock_OffEnd(TIMEREC *time) {
  CLOCK_TimeOff.offEndTime.Hour = time->Hour;
  CLOCK_TimeOff.offEndTime.Min = time->Min;
#if PL_CONFIG_USE_MININI
  McuMinINI_ini_putl(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_OFF_END_HH, time->Hour, NVMC_MININI_FILE_NAME);
  McuMinINI_ini_putl(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_OFF_END_MM, time->Min, NVMC_MININI_FILE_NAME);
#endif
  return ERR_OK;
}
#endif

bool CLOCK_GetClockIsOn(void) {
  return CLOCK_ClockIsOn;
}

static void CLOCK_SetClockIsOn(bool onOff) {
  CLOCK_ClockIsOn = onOff;
#if PL_CONFIG_USE_MININI
  McuMinINI_ini_putl(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_ON, onOff, NVMC_MININI_FILE_NAME);
#endif
}

#if PL_CONFIG_USE_FONT
static void SetClockFont(MFONT_Size_e font) {
  CLOCK_font = font;
#if PL_CONFIG_USE_MININI
  unsigned char buf[16];
  MFONT_FontToStr(font, buf, sizeof(buf));
  McuMinINI_ini_puts(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_FONT, (char*)buf, NVMC_MININI_FILE_NAME);
#endif
}
#endif

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

#if !PL_CONFIG_IS_SPLIT_FLAP && PL_CONFIG_IS_CLIENT && PL_CONFIG_USE_STEPPER
static void SetTime(int32_t x, int32_t y, uint8_t hour, uint8_t minute) {
  #define CLOCK_DEFAULT_DELAY  (5)
  STEPBOARD_Handle_t board = STEPBOARD_GetBoard();
  int32_t angleHour, angleMinute;

  minute %= 60; /* make it 0..59 */
  hour %= 12; /* make it 0..11 */
  angleMinute = (360/60)*minute;
  angleHour = (360/12)*hour + ((360/12)*minute)/60;
  STEPPER_MoveClockDegreeAbs(STEPBOARD_GetStepper(board, x, y, STEPPER_HAND_HH), angleHour, STEPPER_MOVE_MODE_CW, CLOCK_DEFAULT_DELAY, true, true);
  STEPPER_MoveClockDegreeAbs(STEPBOARD_GetStepper(board, x, y, STEPPER_HAND_MM), angleMinute, STEPPER_MOVE_MODE_CW, CLOCK_DEFAULT_DELAY, true, true);
}
#endif

#if PL_CONFIG_USE_LED_RING
static void clock_fade(bool out, uint32_t color) {
  uint32_t c;
  int curr;

  if (out) {
    curr = 0xff;
  } else {
    curr = 0x00;
  }
  do {
    c = NEO_BrightnessFactorColor(color, curr);
    MHAND_SetHandColorAll(c);
    MATRIX_RequestRgbUpdate();
    if (out) {
      curr--;
    } else {
      curr++;
    }
    vTaskDelay(pdMS_TO_TICKS(15));
  } while(curr>=0 && curr<=0xff);
}

static void clock_fadeIn(uint32_t color) {
  clock_fade(false, color);
}

static void clock_fadeOut(uint32_t color) {
  clock_fade(true, color);
}
#endif /* PL_CONFIG_USE_NEO_PIXEL_HW */

static void CLOCK_ShowTimeDate(TIMEREC *time, DATEREC *date) {
  uint8_t buf[8];

  McuLog_info("Time: %02d:%02d, Date: %02d-%02d-%04d", time->Hour, time->Min, date->Day, date->Month, date->Year);

#if PL_CONFIG_USE_LED_CLOCK
  LedClock_ShowTimeDate(time, date);
  return;
#endif

#if PL_CONFIG_USE_MATRIX
  MATRIX_SetMoveDelayAll(MATRIX_GetDefaultDelay());
  MPOS_SetMoveModeAll(STEPPER_MOVE_MODE_SHORT);
#if PL_CONFIG_USE_LED_RING
  uint32_t color;

  if (GetDoRandomHandColor()) {
    int32_t r, g, b;
    do {
      r = McuUtility_random(0, 128); /* limit range to avoid excessive current */
      g = McuUtility_random(0, 128);
      b = McuUtility_random(0, 128);
      CLOCK_HandColor = NEO_COMBINE_RGB(r, g, b);
    } while (CLOCK_HandColor<20 || (r+g+b)>200); /* just making sure it is not too dimm or too bright */
  }
  color = NEO_COMBINE_RGB((CLOCK_HandColor>>16)&0xff, (CLOCK_HandColor>>8)&0xff, CLOCK_HandColor&0xff);
  MHAND_SetHandColorAll(color);
#elif PL_CONFIG_USE_LED_RING
  MATRIX_GetHandColorBrightness(&color, NULL);
#endif
#endif /* PL_CONFIG_USE_MATRIX */

  buf[0] = '\0';
  if (CLOCK_ClockIs24h) {
    McuUtility_strcatNum16uFormatted(buf, sizeof(buf), time->Hour, '0', 2);
  } else {
    int hour = time->Hour%12;
    if (hour==0) {
      hour = 12;
    }
    McuUtility_strcatNum16uFormatted(buf, sizeof(buf), hour, '0', 2);
  }
  McuUtility_strcatNum16uFormatted(buf, sizeof(buf), time->Min, '0', 2);
#if PL_CONFIG_USE_FONT
  #if PL_CONFIG_USE_LED_RING
  if (CLOCK_doFadingHands) {
    clock_fadeOut(color);
  }
  #endif
  uint8_t res;

#if MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5
  res = MFONT_ShowFramedText(0, 0, buf, CLOCK_font, CLOCK_clockHasBorder, true);
#else
  res = MFONT_ShowFramedText(0, 0, buf, CLOCK_font, false, true);
#endif
  #if PL_CONFIG_USE_LED_RING
  if (CLOCK_doFadingHands) {
    clock_fadeIn(color);
  }
  #endif
  if (res!=ERR_OK) {
    McuLog_error("Failed showing time");
  }
#endif /* PL_CONFIG_USE_FONT */

#if PL_CONFIG_IS_SPLIT_FLAP
  STEPPER_ShowLocalString(buf);
#endif

#if PL_CONFIG_HAS_CIRCLE_CLOCK
  MATRIX_SetMoveDelayAll(MATRIX_GetDefaultDelay());
  CC_ShowTime(time->Hour, time->Min);
#endif

#if PL_CONFIG_WORLD_CLOCK
  uint8_t hour;

  hour = AdjustHourForTimeZone(time->Hour, -1); /* local time is GMT+1 */
  SetTime(0, 0, AdjustHourForTimeZone(hour, 0), time->Min); /* London, GMT+0, top left */
  SetTime(1, 0, AdjustHourForTimeZone(hour, -4), time->Min); /* New York,, GMT-4, top right */
  SetTime(0, 1, AdjustHourForTimeZone(hour, 8), time->Min); /* Beijing, GMT+8, bottom left */
  SetTime(1, 1, AdjustHourForTimeZone(hour, 1), time->Min); /* Lucerne, GMT+1, bottom right */
  STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 5);
#endif /* PL_CONFIG_WORLD_CLOCK */
}

#if PL_CONFIG_HAS_SWITCH_7WAY
static void GetTimeString(unsigned char *buf, size_t bufSize, TIMEREC *time, DATEREC *date) {
  buf[0] = '\0';
  McuTimeDate_AddDateString(buf, bufSize, date, (unsigned char*)McuTimeDate_CONFIG_DEFAULT_DATE_FORMAT_STR);
  McuUtility_chcat(buf, bufSize, ' ');
  McuTimeDate_AddTimeString(buf, bufSize, time, (unsigned char*)McuTimeDate_CONFIG_DEFAULT_TIME_FORMAT_STR);
}

static void CLOCK_ButtonMenu(uint32_t notification) {
  static int8_t currDigit = -1;
  TIMEREC time;
  DATEREC date;
  uint8_t buf[24];
  static uint32_t seconds;

  if (notification&CLOCK_TASK_NOTIFY_BUTTON_RST) {
    if (CLOCK_GetClockIsOn()) {
      CLOCK_ClockIsOn = false; /* disable clock */
    }
    McuTimeDate_GetTimeDateAdjustDST(&time, &date);
    CLOCK_ShowTimeDate(&time, &date);
    seconds = McuTimeDate_TimeDateToUnixSeconds(&time, &date, 0);
    currDigit = 3;
    GetTimeString(buf, sizeof(buf), &time, &date);
    McuLog_info("Start setting time: %s", buf);
  } else if (currDigit>=0 && notification&CLOCK_TASK_NOTIFY_BUTTON_UP) {
    switch(currDigit) {
      case 0: seconds += 10*60*60; break; /* 1x:xx (10x hour) */
      case 1: seconds += 60*60; break;    /* x1:xx (hour) */
      case 2: seconds += 10*60; break;    /* xx:1x (10x Minute) */
      case 3: seconds += 60; break;       /* xx:x1 (minute) */
      default: break;
    }
    McuTimeDate_UnixSecondsToTimeDate(seconds, 0, &time, &date);
    GetTimeString(buf, sizeof(buf), &time, &date);
    McuLog_info("Incremented: %s", buf);
    CLOCK_ShowTimeDate(&time, &date);
  } else if (currDigit>=0 && notification&CLOCK_TASK_NOTIFY_BUTTON_DOWN) {
    switch(currDigit) {
      case 0: seconds -= 10*60*60; break; /* 1x:xx (10x hour) */
      case 1: seconds -= 60*60; break;    /* x1:xx (hour) */
      case 2: seconds -= 10*60; break;    /* xx:1x (10x Minute) */
      case 3: seconds -= 60; break;       /* xx:x1 (minute) */
      default: break;
    }
    McuTimeDate_UnixSecondsToTimeDate(seconds, 0, &time, &date);
    GetTimeString(buf, sizeof(buf), &time, &date);
    McuLog_info("Decremented: %s", buf);
    CLOCK_ShowTimeDate(&time, &date);
  } else if (currDigit>=0 && notification&CLOCK_TASK_NOTIFY_BUTTON_LEFT) {
    currDigit--;
    currDigit %= 4;
    McuLog_info("Digit pos: %d", currDigit);
  } else if (currDigit>=0 && notification&CLOCK_TASK_NOTIFY_BUTTON_RIGHT) {
    currDigit++;
    currDigit %= 4;
    McuLog_info("Digit pos: %d", currDigit);
  } else if (currDigit>=0 && notification&CLOCK_TASK_NOTIFY_BUTTON_SET) { /* store it */
    McuTimeDate_UnixSecondsToTimeDate(seconds, 0, &time, &date);
    GetTimeString(buf, sizeof(buf), &time, &date);
    McuLog_info("Storing time: %s", buf);
    McuTimeDate_SetTimeDate(&time, &date);
    CLOCK_ClockIsOn = true; /* enable clock */
    currDigit = -1;
  }
}
#endif /* #if PL_CONFIG_HAS_SWITCH_7WAY */

#if PL_CONFIG_HAS_BUTTONS
void CLOCK_ButtonHandler(McuDbnc_EventKinds event, uint32_t buttons) {
  switch(event) {
    case MCUDBNC_EVENT_PRESSED:
      break;

    case MCUDBNC_EVENT_PRESSED_REPEAT:
      break;

    case MCUDBNC_EVENT_LONG_PRESSED:
      break;

    case MCUDBNC_EVENT_LONG_PRESSED_REPEAT:
      break;

    case MCUDBNC_EVENT_RELEASED:
   #if PL_CONFIG_HAS_SWITCH_USER
      if (buttons&BTN_BIT_USER) {
        CLOCK_Notify(CLOCK_NOTIFY_BUTTON_PRESSED_USR);
      }
   #endif
   #if PL_CONFIG_HAS_SWITCH_7WAY
      if (buttons&BTN_BIT_RST) {
        CLOCK_Notify(CLOCK_NOTIFY_BUTTON_PRESSED_RST);
      }
      if (buttons&BTN_BIT_UP) {
        CLOCK_Notify(CLOCK_NOTIFY_BUTTON_PRESSED_UP);
      }
      if (buttons&BTN_BIT_DOWN) {
        CLOCK_Notify(CLOCK_NOTIFY_BUTTON_PRESSED_DOWN);
      }
      if (buttons&BTN_BIT_LEFT) {
        CLOCK_Notify(CLOCK_NOTIFY_BUTTON_PRESSED_LEFT);
      }
      if (buttons&BTN_BIT_RIGHT) {
        CLOCK_Notify(CLOCK_NOTIFY_BUTTON_PRESSED_RIGHT);
      }
      if (buttons&BTN_BIT_MID) {
        CLOCK_Notify(CLOCK_NOTIFY_BUTTON_PRESSED_MID);
      }
      if (buttons&BTN_BIT_SET) {
        CLOCK_Notify(CLOCK_NOTIFY_BUTTON_PRESSED_SET);
      }
   #elif PL_CONFIG_HAS_SWITCH_2WAY
      if (buttons&BTN_BIT_UP) {
        CLOCK_Notify(CLOCK_NOTIFY_BUTTON_PRESSED_UP);
      }
      if (buttons&BTN_BIT_DOWN) {
        CLOCK_Notify(CLOCK_NOTIFY_BUTTON_PRESSED_DOWN);
      }
   #endif /* PL_CONFIG_HAS_SWITCH_7WAY */
      break;

    case MCUDBNC_EVENT_LONG_RELEASED:
      #if PL_CONFIG_HAS_SWITCH_USER
      if (buttons&BTN_BIT_USER) {
        CLOCK_Notify(CLOCK_NOTIFY_BUTTON_PRESSED_USR_LONG);
      }
      #endif
      break;

    default:
      break;
  }
}
#endif

void CLOCK_Notify(CLOCK_Notify_e msg) {
  switch(msg) {
    case CLOCK_NOTIFY_UPDATE_CLOCK:
      (void)xTaskNotify(clockTaskHndl, CLOCK_TASK_NOTIFY_UPDATE_CLOCK, eSetBits);
      break;
#if PL_CONFIG_HAS_SWITCH_USER
    case CLOCK_NOTIFY_BUTTON_PRESSED_USR:
      (void)xTaskNotify(clockTaskHndl, CLOCK_TASK_NOTIFY_BUTTON_USR, eSetBits);
      break;
    case CLOCK_NOTIFY_BUTTON_PRESSED_USR_LONG:
      (void)xTaskNotify(clockTaskHndl, CLOCK_TASK_NOTIFY_BUTTON_USR_LONG, eSetBits);
      break;
#endif
#if PL_CONFIG_HAS_SWITCH_7WAY
    case CLOCK_NOTIFY_BUTTON_PRESSED_RST:
      (void)xTaskNotify(clockTaskHndl, CLOCK_TASK_NOTIFY_BUTTON_RST, eSetBits);
      break;
    case CLOCK_NOTIFY_BUTTON_PRESSED_UP:
      (void)xTaskNotify(clockTaskHndl, CLOCK_TASK_NOTIFY_BUTTON_UP, eSetBits);
      break;
    case CLOCK_NOTIFY_BUTTON_PRESSED_DOWN:
      (void)xTaskNotify(clockTaskHndl, CLOCK_TASK_NOTIFY_BUTTON_DOWN, eSetBits);
      break;
    case CLOCK_NOTIFY_BUTTON_PRESSED_LEFT:
      (void)xTaskNotify(clockTaskHndl, CLOCK_TASK_NOTIFY_BUTTON_LEFT, eSetBits);
      break;
    case CLOCK_NOTIFY_BUTTON_PRESSED_RIGHT:
      (void)xTaskNotify(clockTaskHndl, CLOCK_TASK_NOTIFY_BUTTON_RIGHT, eSetBits);
      break;
    case CLOCK_NOTIFY_BUTTON_PRESSED_MID:
      (void)xTaskNotify(clockTaskHndl, CLOCK_TASK_NOTIFY_BUTTON_MID, eSetBits);
      break;
    case CLOCK_NOTIFY_BUTTON_PRESSED_SET:
      (void)xTaskNotify(clockTaskHndl, CLOCK_TASK_NOTIFY_BUTTON_SET, eSetBits);
      break;
#elif PL_CONFIG_HAS_SWITCH_2WAY
    case CLOCK_NOTIFY_BUTTON_PRESSED_UP:
      (void)xTaskNotify(clockTaskHndl, CLOCK_NOTIFY_BUTTON_PRESSED_UP, eSetBits);
      break;
    case CLOCK_NOTIFY_BUTTON_PRESSED_DOWN:
      (void)xTaskNotify(clockTaskHndl, CLOCK_NOTIFY_BUTTON_PRESSED_DOWN, eSetBits);
      break;
#endif
    default:
      break;
  }
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

#if PL_CONFIG_IS_SPLIT_FLAP
static void ShowTime(uint8_t hour, uint8_t minute) {
  uint8_t buf[16];

  buf[0] = '\0';
  McuUtility_strcatNum16uFormatted(buf, sizeof(buf), hour, '0', 2);
  McuUtility_strcatNum16uFormatted(buf, sizeof(buf), minute, '0', 2);
  STEPPER_ShowLocalString(buf);
}
#endif

#if PL_CONFIG_IS_CLOCK_CLOCK
static void ShowTime(int32_t x, int32_t y, uint8_t hour, uint8_t minute) {
  STEPBOARD_Handle_t board = STEPBOARD_GetBoard();

  SetTime(x, y, hour, minute);
  STEPBOARD_MoveAndWait(board, 5);
}
#endif

#endif /* PL_CONFIG_USE_STEPPER */

#if PL_CONFIG_USE_SHELL
static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  uint8_t res = ERR_OK;
  uint8_t buf[96];
  const unsigned char *clock_type = (unsigned char*)"\r\n";

  McuShell_SendStatusStr((unsigned char*)"clock", (unsigned char*)"Clock settings\r\n", io->stdOut);
#if PL_CONFIG_USE_MATRIX
  #if PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_8x3
    clock_type = (unsigned char*)"Clock 8x3\r\n";
  #elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_12x5_60B
    clock_type = (unsigned char*)"Clock 12x5 60 Billion Lights\r\n";
  #elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_12x5_MOD
    clock_type = (unsigned char*)"Clock 12x5 Modular\r\n";
  #elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_16x9_ALEXIS
    clock_type = (unsigned char*)"Clock 16x9 Alexis\r\n";
  #elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_8x3_V4
    clock_type = (unsigned char*)"Clock 8x3 V4\r\n";
  #elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CIRCULAR_CLOCK_1x12
    clock_type = (unsigned char*)"Circular Clock 1x12\r\n";
  #elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_SMARTWALL_16x5
    clock_type = (unsigned char*)"SmartWall 16x5\r\n";
  #else
    #error "unknown"
    clock_type = (unsigned char*)"unknown\r\n";
  #endif
#else
  clock_type = (unsigned char*)"split-flap\r\n";
#endif
  McuShell_SendStatusStr((unsigned char*)"  type", clock_type, io->stdOut);

#if PL_CONFIG_IS_MASTER
  McuShell_SendStatusStr((unsigned char*)"  mode", (unsigned char*)"master\r\n", io->stdOut);
#else
  McuShell_SendStatusStr((unsigned char*)"  mode", (unsigned char*)"client\r\n", io->stdOut);
#endif
#if PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_LPC845_1X4 || PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_LPC845_2X2
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"V");
  McuUtility_strcatNum8u(buf, sizeof(buf), PL_CONFIG_BOARD_VERSION/10);
  McuUtility_chcat(buf, sizeof(buf), '.');
  McuUtility_strcatNum8u(buf, sizeof(buf), PL_CONFIG_BOARD_VERSION%10);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  board", buf, io->stdOut);
#endif
  McuShell_SendStatusStr((unsigned char*)"  clock", CLOCK_GetClockIsOn()?(unsigned char*)"on\r\n":(unsigned char*)"off\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  parked", CLOCK_ClockIsParked?(unsigned char*)"yes\r\n":(unsigned char*)"no\r\n", io->stdOut);
#if PL_CONFIG_USE_CLOCK_TIME_OFF
  McuUtility_strcpy(buf, sizeof(buf), CLOCK_TimeOff.isTimeOnOffEnabled?(unsigned char*)"enabled:yes, ":(unsigned char*)"enabled:no, ");
  McuUtility_strcat(buf, sizeof(buf), CLOCK_TimeOff.offIsActive?(unsigned char*)"off is active":(unsigned char*)"off not active");
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)", off-day bits 0x");
  McuUtility_strcatNum8Hex(buf, sizeof(buf), GetOffDays());

  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"; off-hours ");
  McuTimeDate_AddTimeString(buf, sizeof(buf), &CLOCK_TimeOff.offStartTime, (uint8_t*)McuTimeDate_CONFIG_HH_MM_TIME_FORMAT_STR);
  McuUtility_chcat(buf, sizeof(buf), '-');
  McuTimeDate_AddTimeString(buf, sizeof(buf), &CLOCK_TimeOff.offEndTime, (uint8_t*)McuTimeDate_CONFIG_HH_MM_TIME_FORMAT_STR);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  on/off", buf, io->stdOut);
#endif
#if 0 /* not implemented yet */
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"every ");
  McuUtility_strcatNum8u(buf, sizeof(buf), CLOCK_UpdatePeriodMinutes);
  McuUtility_strcat(buf, sizeof(buf), CLOCK_UpdatePeriodMinutes==1?(unsigned char*)" minute\r\n":(unsigned char*)" minutes\r\n");
  McuShell_SendStatusStr((unsigned char*)"  period", buf, io->stdOut);
#endif
  McuShell_SendStatusStr((unsigned char*)"  24h", CLOCK_ClockIs24h?(unsigned char*)"on\r\n":(unsigned char*)"off\r\n", io->stdOut);
#if PL_CONFIG_USE_LED_RING
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"color: 0x");
  McuUtility_strcatNum24Hex(buf, sizeof(buf), CLOCK_HandColor);
#if PL_CONFIG_USE_LED_DIMMING
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)", brightness 0x");
  McuUtility_strcatNum8Hex(buf, sizeof(buf), CLOCK_HandBrightness);
#endif
  McuUtility_strcat(buf, sizeof(buf), GetDoRandomHandColor()?(unsigned char*)", random on":(unsigned char*)", random off");
  McuUtility_strcat(buf, sizeof(buf), GetDoFadingHands()?(unsigned char*)", fading on":(unsigned char*)", fading off");
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  hand", buf, io->stdOut);
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  McuShell_SendStatusStr((unsigned char*)"  seconds", CLOCK_ShowSeconds?(unsigned char*)"on\r\n":(unsigned char*)"off\r\n", io->stdOut);
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"color: 0x");
  McuUtility_strcatNum24Hex(buf, sizeof(buf), CLOCK_SecondColor);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  second", buf, io->stdOut);
#endif
#if MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5
  McuShell_SendStatusStr((unsigned char*)"  border", CLOCK_clockHasBorder?(unsigned char*)"on\r\n":(unsigned char*)"off\r\n", io->stdOut);
#endif
#if PL_CONFIG_USE_FONT
  MFONT_FontToStr(CLOCK_font, buf, sizeof(buf));
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  font", buf, io->stdOut);
#endif
  return res;
}
#endif

#if PL_CONFIG_USE_SHELL
static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"clock", (unsigned char*)"Group of clock commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  on|off|toggle", (unsigned char*)"Enable or disable the clock\r\n", io->stdOut);
#if PL_CONFIG_USE_CLOCK_TIME_OFF
  McuShell_SendHelpStr((unsigned char*)"  onoff on|off", (unsigned char*)"Enable time based automatic on/off\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  offstart <time>", (unsigned char*)"on/off start time\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  offend <time>", (unsigned char*)"on/off end time\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  offdays <bits>", (unsigned char*)"Set days off, Sun 0b1, Mon 0b10, Tue 0b100 ...\r\n", io->stdOut);
#endif
  McuShell_SendHelpStr((unsigned char*)"  24h on|off", (unsigned char*)"Show time in 24h (17:35) or 12h (5:35) format\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  park on|off|toggle", (unsigned char*)"Turns clock off and moves to park position, ready to power off\r\n", io->stdOut);
#if 0 /* not implemented yet */
  McuShell_SendHelpStr((unsigned char*)"  period <minute>", (unsigned char*)"Clock update period in minutes (>0)\r\n", io->stdOut);
#endif
#if PL_CONFIG_USE_LED_RING
  McuShell_SendHelpStr((unsigned char*)"  hand rgb random on|off", (unsigned char*)"Set hand random color mode\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  hand rgb <rgb>", (unsigned char*)"Set hand color\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  hand fading on|off", (unsigned char*)"Set hand fading on or off\r\n", io->stdOut);
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  McuShell_SendHelpStr((unsigned char*)"  seconds on|off", (unsigned char*)"Show seconds\r\n", io->stdOut);
#endif
#if PL_CONFIG_USE_LED_DIMMING
  McuShell_SendHelpStr((unsigned char*)"  brightness <v>", (unsigned char*)"Set hand brightness factor (0-255)\r\n", io->stdOut);
#endif
#if MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5
  McuShell_SendHelpStr((unsigned char*)"  border on|off", (unsigned char*)"Show clock with border\r\n", io->stdOut);
#endif
#if PL_CONFIG_USE_FONT
  McuShell_SendHelpStr((unsigned char*)"  font <f>", (unsigned char*)"Set clock font, e.g. 2x3\r\n", io->stdOut);
#endif
#if PL_CONFIG_IS_CLIENT && PL_CONFIG_USE_STEPPER
#if PL_CONFIG_IS_SPLIT_FLAP
  McuShell_SendHelpStr((unsigned char*)"  time <time>", (unsigned char*)"Show time\r\n", io->stdOut);
#endif
#if PL_CONFIG_IS_CLOCK_CLOCK
  McuShell_SendHelpStr((unsigned char*)"  time <x> <y> <time>", (unsigned char*)"Show time on clock at coordinate (x,y)\r\n", io->stdOut);
#endif
#endif
#if PL_CONFIG_WORLD_CLOCK
  McuShell_SendHelpStr((unsigned char*)"  clocks", (unsigned char*)"[0,0] London    [1,0] New York\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"",         (unsigned char*)"[0,1] Beijing   [1,1] Lucerne\r\n", io->stdOut);
#endif
  return ERR_OK;
}
#endif

#if PL_CONFIG_USE_SHELL
uint8_t CLOCK_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  const unsigned char *p;
#if PL_CONFIG_USE_CLOCK_TIME_OFF
  TIMEREC time;
#endif

  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "clock help")==0) {
    *handled = true;
    return PrintHelp(io);
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "clock status")==0)) {
    *handled = true;
    return PrintStatus(io);
  } else if (McuUtility_strcmp((char*)cmd, "clock on")==0) {
    *handled = true;
    CLOCK_SetClockIsOn(true);
#if PL_CONFIG_USE_DEMOS
    if (DEMO_IsOn()) {
      McuShell_SendStr((unsigned char*)"Demo is on, disable it first with 'demo off'.\r\n", io->stdErr);
    } else {
      CLOCK_On(CLOCK_MODE_ON);
    }
#else
    CLOCK_On(CLOCK_MODE_ON);
#endif
  } else if (McuUtility_strcmp((char*)cmd, "clock off")==0) {
    *handled = true;
    CLOCK_SetClockIsOn(false);
    CLOCK_On(CLOCK_MODE_OFF);
#if PL_CONFIG_USE_CLOCK_TIME_OFF
  } else if (McuUtility_strcmp((char*)cmd, "clock onoff on")==0) {
    *handled = true;
    return SetClock_OnOff(true);
  } else if (McuUtility_strcmp((char*)cmd, "clock onoff off")==0) {
    *handled = true;
    return SetClock_OnOff(false);
  } else if (McuUtility_strncmp((char*)cmd, "clock offstart ", sizeof("clock offstart ")-1)==0) {
    *handled = TRUE;
    p = cmd + sizeof("clock offstart ")-1;
    if (McuUtility_ScanTime(&p, &time.Hour, &time.Min, &time.Sec, &time.Sec100)!=ERR_OK) {
      return ERR_FAILED;
    }
    return SetClock_OffStart(&time);
  } else if (McuUtility_strncmp((char*)cmd, "clock offend ", sizeof("clock offend ")-1)==0) {
    *handled = TRUE;
    p = cmd + sizeof("clock offend ")-1;
    if (McuUtility_ScanTime(&p, &time.Hour, &time.Min, &time.Sec, &time.Sec100)!=ERR_OK) {
      return ERR_FAILED;
    }
    return SetClock_OffEnd(&time);
  } else if (McuUtility_strncmp((char*)cmd, "clock offdays ", sizeof("clock offdays ")-1)==0) {
    int32_t bits;
    *handled = TRUE;
    p = cmd + sizeof("clock offdays ")-1;
    if (McuUtility_xatoi(&p, &bits)!=ERR_OK) {
      return ERR_FAILED;
    }
    if (bits<0 || bits>0b1111111) {
      return ERR_RANGE; /* only bits 0 to 6. Bit 0 is Sunday, bit 1 is Monday and so on */
    }
    SetOffDays(bits);
    return ERR_OK;
#endif
  } else if (McuUtility_strcmp((char*)cmd, "clock 24h on")==0) {
    *handled = true;
    CLOCK_ClockIs24h = true;
  } else if (McuUtility_strcmp((char*)cmd, "clock 24h off")==0) {
    *handled = true;
    CLOCK_ClockIs24h = false;
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
#if 0 /* not implemented yet */
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
#endif
#if PL_CONFIG_USE_LED_RING
  } else if (McuUtility_strcmp((char*)cmd, "clock hand rgb random on")==0) {
    *handled = true;
    SetDoRandomHandColor(true);
  } else if (McuUtility_strcmp((char*)cmd, "clock hand rgb random off")==0) {
    *handled = true;
    SetDoRandomHandColor(false);
#endif
#if PL_CONFIG_USE_LED_RING
  } else if (McuUtility_strcmp((char*)cmd, "clock hand fading on")==0) {
    *handled = true;
    SetDoFadingHands(true);
  } else if (McuUtility_strcmp((char*)cmd, "clock hand fading off")==0) {
    *handled = true;
    SetDoFadingHands(false);
#endif
#if PL_CONFIG_USE_LED_RING
  } else if (McuUtility_strncmp((char*)cmd, "clock hand rgb ", sizeof("clock hand rgb ")-1)==0) {
    *handled = TRUE;
    p = cmd + sizeof("clock hand rgb ")-1;
    if (McuUtility_ScanRGB32(&p, &CLOCK_HandColor)==ERR_OK) {
      MHAND_SetHandColorAll(NEO_COMBINE_RGB((CLOCK_HandColor>>16)&0xff, (CLOCK_HandColor>>8)&0xff, CLOCK_HandColor&0xff));
#if PL_CONFIG_USE_NEO_PIXEL_HW
      APP_RequestUpdateLEDs();
#endif
      return ERR_OK;
    } else {
      return ERR_FAILED;
    }
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
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
#if PL_CONFIG_USE_FONT
  } else if (McuUtility_strncmp((char*)cmd, "clock font ", sizeof("clock font")-1)==0) {
    MFONT_Size_e font;

    *handled = true;
    p = cmd + sizeof("clock font ")-1;
    MFONT_ParseFontName(&p, &font);
    if (CLOCK_font==MFONT_SIZE_ERROR) {
      return ERR_FAILED;
    } else {
      SetClockFont(font);
    }
#endif
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

    *handled = TRUE;
    p = cmd + sizeof("clock time ")-1;
    #if PL_CONFIG_IS_SPLIT_FLAP
    if (McuUtility_ScanTime(&p, &hour, &minute, &second, &hsec)==ERR_OK) {
      ShowTime(hour, minute);
    #elif PL_CONFIG_IS_CLOCK_CLOCK
    int32_t x, y;
    if (
           McuUtility_xatoi(&p, &x)==ERR_OK && x>=0 && x<MATRIX_NOF_STEPPERS_X
        && McuUtility_xatoi(&p, &y)==ERR_OK && y>=0 && y<MATRIX_NOF_STEPPERS_Y
        && McuUtility_ScanTime(&p, &hour, &minute, &second, &hsec)==ERR_OK
       )
    {
      ShowTime(x, y, hour, minute);
    #endif
    } else {
      return ERR_FAILED;
    }
    return ERR_OK;
#endif /* PL_CONFIG_USE_STEPPER */
  }
  return ERR_OK;
}
#endif /* #if PL_CONFIG_USE_SHELL */

#if PL_CONFIG_USE_NEO_PIXEL_HW
static void ShowSeconds(const TIMEREC *time) {
  if (!CLOCK_ShowSeconds) {
    return; /* disabled */
  }
#if PL_CONFIG_USE_LED_RING /* showing second with LED rings */
  static uint8_t lastSecondShown = -1;

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
#endif
}
#endif

static void UpdateTimeDate(TickType_t *lastUpdateTickCount, uint32_t updatePeriodMinutes) {
  /* Because the SW RTC might run off, we update the SW RTC from the HW RTC every 'updatePeriodMinutes' */
  TickType_t tickCount = xTaskGetTickCount();
  TIMEREC time;
  DATEREC date;
  uint8_t res;

  /* update SW RTC from external RTC */
  if ((tickCount-*lastUpdateTickCount) > pdMS_TO_TICKS(updatePeriodMinutes*60*1000)) { /* Check if it is time to update from external RTC */
    unsigned char timeBuf[16];
    uint32_t oldSWRTC, newSWRTC;

    McuTimeDate_GetTimeDate(&time, &date); /* get current SW RTC values, not using DST for this */
    oldSWRTC = McuTimeDate_TimeDateToUnixSeconds(&time, &date, 0); /* remember the value we had */
    timeBuf[0] = '\0';
    McuTimeDate_AddTimeString(timeBuf, sizeof(timeBuf), &time, (unsigned char*)McuTimeDate_CONFIG_DEFAULT_TIME_FORMAT_STR);
    McuLog_info("Updating SW RTC from HW RTC after %d minutes", updatePeriodMinutes);
    res = McuTimeDate_SyncFromExternalRTC(); /* update SW RTC from external HW RTC  */
    if (res!=ERR_OK) {
      McuLog_error("Failed updating RTC from external RTC");
    } else {
      McuTimeDate_GetTimeDate(&time, &date); /* what is the new time now? */
      newSWRTC = McuTimeDate_TimeDateToUnixSeconds(&time, &date, 0);
      timeBuf[0] = '\0';
      McuTimeDate_AddTimeString(timeBuf, sizeof(timeBuf), &time, (unsigned char*)McuTimeDate_CONFIG_DEFAULT_TIME_FORMAT_STR);
      if (newSWRTC>oldSWRTC) {
        McuLog_info("SW RTC was behind %d secs: new time is %s", newSWRTC-oldSWRTC, timeBuf);
      } else if (newSWRTC==oldSWRTC) {
        McuLog_info("Updating software RTC: %s, no drift", timeBuf);
      } else { /* newSWRTC<oldSWRTC */
        McuLog_info("SW RTC was ahead %d secs: new time is %s", oldSWRTC-newSWRTC, timeBuf);
      }
    }
    *lastUpdateTickCount = tickCount;
  }
}

static void ClockTask(void *pv) {
  uint8_t res;
  bool doImmediateClockUpdate = true;
  TIMEREC time;
  DATEREC date;
  TickType_t lastTimeDateUpdatTickCount = 0; /* time stamp when last time the SW RTC has been updated */
#if PL_CONFIG_USE_INTERMEZZO
  TickType_t lastClockUpdateTickCount = -1; /* tick count when the clock has been updated the last time */
  bool intermezzoShown = true;
#endif
  uint32_t ulNotificationValue;

  McuLog_trace("Starting Clock Task");
#if PL_CONFIG_USE_LED_PIXEL
  PIXEL_ZeroAll();
#endif
  res = McuTimeDate_Init();
#if PL_CONFIG_USE_WDT
  WDT_SetTaskHandle(WDT_REPORT_ID_TASK_CLOCK, xTaskGetCurrentTaskHandle());
#endif
  vTaskDelay(pdMS_TO_TICKS(1000)); /* give external RTC and hardware time to power up */
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
  #elif McuLib_CONFIG_CPU_IS_KINETIS && PL_CONFIG_IS_MASTER && PL_CONFIG_USE_LED_PIXEL
  SHELL_SendString((unsigned char*)"\r\n***********************\r\n* MovingPixels Master *\r\n***********************\r\n");
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

#if PL_CONFIG_USE_LED_RING
  /* turn on the hand LEDs */
  MHAND_SetHandColorAll(NEO_COMBINE_RGB((CLOCK_HandColor>>16)&0xff, (CLOCK_HandColor>>8)&0xff, CLOCK_HandColor&0xff));
#if PL_CONFIG_USE_LED_DIMMING
  MATRIX_SetHandBrightnessAll(CLOCK_HandBrightness);
#endif
  /* toggle hands: it could be that only the master has reset, make sure all clocks get the update */
  (void)SHELL_ParseCommandIO((const unsigned char *)"matrix he all off", NULL, true);
  (void)SHELL_ParseCommandIO((const unsigned char *)"matrix he all on", NULL, true);
#if PL_CONFIG_USE_NEO_PIXEL_HW
  APP_RequestUpdateLEDs(); /* update LEDs */
#endif
#endif
#if PL_CONFIG_USE_EXT_I2C_RTC
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
#if PL_CONFIG_IS_CLOCK_CLOCK
#if PL_CONFIG_IS_MASTER && PL_CONFIG_USE_MOTOR_ON_OFF /* turn on motors */
  (void)SHELL_ParseCommandIO((const unsigned char *)"matrix motor on", NULL, true);
#endif
#endif /* PL_CONFIG_IS_CLOCK_CLOCK */
#if PL_CONFIG_IS_SPLIT_FLAP
#if PL_CONFIG_IS_SPLIT_FLAP && PL_CONFIG_CLOCK_ZERO_STEPPER
  (void)SHELL_ParseCommandIO((const unsigned char*)"stepper zero all", NULL, false);
  vTaskDelay(pdMS_TO_TICKS(3000)); /* give some time to zero motors */
#endif
#endif /* PL_CONFIG_IS_SPLIT_FLAP */

#if PL_CONFIG_USE_CLOCK_TIME_OFF
  CLOCK_TimeOff.offIsActive = false;
#endif
#if PL_CONFIG_USE_MININI && PL_CONFIG_USE_CLOCK_TIME_OFF
  CLOCK_TimeOff.isTimeOnOffEnabled = McuMinINI_ini_getbool(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_ON_OFF, CONFIG_CLOCK_DEFAULT_ON_OFF, NVMC_MININI_FILE_NAME);
  CLOCK_TimeOff.offStartTime.Hour = McuMinINI_ini_getl(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_OFF_START_HH, CONFIG_CLOCK_DEFAULT_OFF_START_HH, NVMC_MININI_FILE_NAME);
  CLOCK_TimeOff.offStartTime.Min = McuMinINI_ini_getl(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_OFF_START_MM, CONFIG_CLOCK_DEFAULT_OFF_START_MM, NVMC_MININI_FILE_NAME);
  CLOCK_TimeOff.offEndTime.Hour = McuMinINI_ini_getl(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_OFF_END_HH, CONFIG_CLOCK_DEFAULT_OFF_END_HH, NVMC_MININI_FILE_NAME);
  CLOCK_TimeOff.offEndTime.Min = McuMinINI_ini_getl(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_OFF_END_MM, CONFIG_CLOCK_DEFAULT_OFF_END_MM, NVMC_MININI_FILE_NAME);
  CLOCK_TimeOff.offDays = McuMinINI_ini_getl(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_OFF_DAYS, CONFIG_CLOCK_DEFAULT_OFF_DAYS, NVMC_MININI_FILE_NAME);
  #else
  CLOCK_TimeOff.isTimeOnOffEnabled = CONFIG_CLOCK_DEFAULT_ON_OFF;
  CLOCK_TimeOff.offStartTime.Hour = CONFIG_CLOCK_DEFAULT_OFF_START_HH;
  CLOCK_TimeOff.offStartTime.Min = CONFIG_CLOCK_DEFAULT_OFF_START_MM;
  CLOCK_TimeOff.offEndTime.Hour = CONFIG_CLOCK_DEFAULT_OFF_END_HH;
  CLOCK_TimeOff.offEndTime.Min = CONFIG_CLOCK_DEFAULT_OFF_END_MM;
  CLOCK_TimeOff.offDays = CONFIG_CLOCK_DEFAULT_OFF_DAYS;
#endif
#if PL_CONFIG_USE_MININI && PL_CONFIG_USE_FONT
  unsigned char buf[6], defaultFont[6];
  const unsigned char *p = buf;

  MFONT_FontToStr(PL_CONFIG_CLOCK_DEFAULT_FONT, defaultFont, sizeof(defaultFont));
  McuMinINI_ini_gets(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_FONT, (char*)defaultFont, (char*)buf, sizeof(buf), NVMC_MININI_FILE_NAME);
  MFONT_ParseFontName(&p, &CLOCK_font);
#elif PL_CONFIG_USE_FONT
  CLOCK_font = PL_CONFIG_CLOCK_DEFAULT_FONT;
#endif

#if PL_CONFIG_USE_MININI
  CLOCK_ClockIsOn = McuMinINI_ini_getbool(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_ON, PL_CONFIG_CLOCK_ON_BY_DEFAULT, NVMC_MININI_FILE_NAME);
#else
  CLOCK_ClockIsOn = PL_CONFIG_CLOCK_ON_BY_DEFAULT;
#endif
#if PL_CONFIG_USE_LED_RING
#if PL_CONFIG_USE_MININI
  CLOCK_doRandomHandColor = McuMinINI_ini_getbool(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_RANDOM_HAND_COLOR, PL_CONFIG_CLOCK_RANDOM_COLOR_ON, NVMC_MININI_FILE_NAME);
#else
  CLOCK_doRandomHandColor = PL_CONFIG_CLOCK_RANDOM_COLOR_ON;
#endif
#endif
#if PL_CONFIG_USE_LED_RING
#if PL_CONFIG_USE_MININI
  CLOCK_doFadingHands = McuMinINI_ini_getbool(NVMC_MININI_SECTION_CLOCK, NVMC_MININI_KEY_CLOCK_FADING_HANDS, false, NVMC_MININI_FILE_NAME);
#else
  CLOCK_doFadingHands = false;
#endif
#endif
#if PL_CONFIG_USE_INTERMEZZO
  Intermezzo_InitSettings();
#endif

  for(;;) {
    vTaskDelay(pdMS_TO_TICKS(200));
  #if PL_CONFIG_USE_WDT
    WDT_Report(WDT_REPORT_ID_TASK_CLOCK, 200);
  #endif
    /* check task notifications */
    res = xTaskNotifyWait(
       0, /* do not clear anything on enter */
       CLOCK_TASK_NOTIFY_ALL, /* clear all bits on exit */
       &ulNotificationValue,
       0);
    if (res==pdTRUE) { /* notification received */
      if (ulNotificationValue&CLOCK_TASK_NOTIFY_UPDATE_CLOCK) {
        McuLog_info("Notification: update clock");
        doImmediateClockUpdate = true;
      }
      if (ulNotificationValue&CLOCK_TASK_NOTIFY_BUTTON_USR) {
        McuLog_info("Notification: button pressed");
        SHELL_ParseCommandIO((unsigned char*)"clock toggle", McuShell_GetStdio(), true);
      }
    #ifdef CLOCK_TASK_NOTIFY_BUTTON_UP
      if (ulNotificationValue&CLOCK_TASK_NOTIFY_BUTTON_UP) {
        McuLog_info("Notification: up button pressed");
      }
    #endif
    #ifdef CLOCK_TASK_NOTIFY_BUTTON_DOWN
      if (ulNotificationValue&CLOCK_TASK_NOTIFY_BUTTON_DOWN) {
        McuLog_info("Notification: down button pressed");
      }
    #endif
      if (ulNotificationValue&CLOCK_TASK_NOTIFY_BUTTON_USR_LONG) {
        McuLog_info("Notification: button pressed long");
        SHELL_ParseCommandIO((unsigned char*)"intermezzo toggle", McuShell_GetStdio(), true);
      }
      if (ulNotificationValue&CLOCK_TASK_NOTIFY_PARK_ON) {
        McuLog_info("Start parking clock");
        SHELL_ParseCommandIO((unsigned char*)"matrix park on", McuShell_GetStdio(), true); /* move to 12-o-clock position */
        McuLog_info("Parking done.");
        CLOCK_ClockIsOn = false; /* disabled clock */
        CLOCK_ClockIsParked = true;
      }
      if (ulNotificationValue&CLOCK_TASK_NOTIFY_PARK_OFF) {
        McuLog_info("Start unparking clock");
        SHELL_ParseCommandIO((unsigned char*)"matrix park off", McuShell_GetStdio(), true); /* move to 12-o-clock position */
        McuLog_info("Unparking done.");
        CLOCK_ClockIsOn = false; /* disabled clock */
        CLOCK_ClockIsParked = false;
      }
      if (ulNotificationValue&CLOCK_TASK_NOTIFY_PARK_TOGGLE) {
        McuLog_info("toggle park");
        if (CLOCK_ClockIsParked) {
          McuLog_info("Start unparking clock");
          SHELL_ParseCommandIO((unsigned char*)"matrix park off", McuShell_GetStdio(), true); /* move to 12-o-clock position */
          McuLog_info("Unparking done.");
          CLOCK_ClockIsOn = false; /* disabled clock */
          CLOCK_ClockIsParked = false;
        } else {
          McuLog_info("Start parking clock");
          SHELL_ParseCommandIO((unsigned char*)"matrix park on", McuShell_GetStdio(), true); /* move to 12-o-clock position */
          McuLog_info("Parking done.");
          CLOCK_ClockIsOn = false; /* disabled clock */
          CLOCK_ClockIsParked = true;
        }
      }
      if (ulNotificationValue&CLOCK_TASK_NOTIFY_CLOCK_ON) {
        CLOCK_ClockIsOn = true; /* enable clock */
        doImmediateClockUpdate = true;
      #if PL_CONFIG_USE_LED_RING
        MHAND_SetHandColorAll(MATRIX_GetHandColorAdjusted()); /* default hand color */
        MATRIX_DrawAllRingColor(0x000000); /* ring color off */
        MHAND_HandEnableAll(true);
      #endif
      }
      if (ulNotificationValue&CLOCK_TASK_NOTIFY_CLOCK_OFF) {
        McuLog_info("Clock off");
        CLOCK_ClockIsOn = false; /* disable clock */
      #if PL_CONFIG_IS_ANALOG_CLOCK
        MATRIX_MoveAllto12(5000, NULL); /* this turns on the hands */
        #if PL_CONFIG_USE_LED_RING
            /* turn off LEDs */
            MRING_SetRingColorAll(0, 0, 0);
            MHAND_HandEnableAll(false);
          #if PL_CONFIG_USE_EXTENDED_HANDS
            MHAND_2ndHandEnableAll(false);
          #endif
            MATRIX_RequestRgbUpdate(); /* update LEDs */
        #elif PL_MATRIX_CONFIG_IS_RGB
            MATRIX_EnableDisableHandsAll(false);
        #endif
      #elif PL_CONFIG_USE_LED_CLOCK
            LedClock_ReleasePixelAll();
            // INTERMEZZO_PlaySpecific(1); /* show rainbow colors */
            NEO_ClearAllPixel(); /* clear all pixels */
            MATRIX_RequestRgbUpdate(); /* update LEDs */
            MATRIX_MoveAllToStartPosition(1000, NULL); /* Move all stepper to start position */
      #endif
      #if PL_CONFIG_IS_SPLIT_FLAP
        (void)SHELL_ParseCommandIO((const unsigned char*)"stepper display \"    \"", NULL, false);
        vTaskDelay(pdMS_TO_TICKS(3000)); /* give some time to zero motors */
      #endif
      }
      if (ulNotificationValue&CLOCK_TASK_NOTIFY_CLOCK_TOGGLE) {
        McuLog_info("Clock toggle");
        CLOCK_ClockIsOn = !CLOCK_ClockIsOn; /* toggle clock */
      #if PL_CONFIG_USE_NEO_PIXEL_HW
        if (!CLOCK_ClockIsOn) {
          /* turn off LEDs */
        #if PL_CONFIG_IS_ANALOG_CLOCK
          MRING_SetRingColorAll(0, 0, 0);
          MHAND_HandEnableAll(false);
          #if PL_CONFIG_USE_EXTENDED_HANDS
          MHAND_2ndHandEnableAll(false);
          #endif
        #elif PL_CONFIG_USE_LED_CLOCK
          LedClock_ReleasePixelAll();
          NEO_ClearAllPixel(); /* clear all pixels */
        #endif
          APP_RequestUpdateLEDs(); /* update LEDs */
        }
      #endif
        if (!CLOCK_ClockIsOn) {
          #if PL_CONFIG_IS_ANALOG_CLOCK
          MATRIX_MoveAllto12(5000, NULL);
          #elif PL_CONFIG_USE_LED_CLOCK
          MATRIX_MoveAllToStartPosition(1000, NULL);
		      #endif
        }
        if (CLOCK_ClockIsOn) {
          doImmediateClockUpdate = true;
        }
      }
  #if PL_CONFIG_HAS_SWITCH_7WAY
      CLOCK_ButtonMenu(ulNotificationValue);
  #endif /* PL_CONFIG_HAS_SWITCH_7WAY */
    } /* if notification received */
    /* ----------------------------------------------------------------------------------*/
    UpdateTimeDate(&lastTimeDateUpdatTickCount, CLOCK_CONFIG_UPDATE_SW_RTC_FROM_HW_RTC_PERIOD_MINUTES);
  #if PL_CONFIG_USE_INTERMEZZO
    /* ----------------------------------------------------------------------------------*/
    /* Intermezzo */
    /* ----------------------------------------------------------------------------------*/
    if (CLOCK_ClockIsOn) {
      if (!intermezzoShown) { /* not shown intermezzo? */
        INTERMEZZO_Play(lastClockUpdateTickCount, &intermezzoShown);
        if (intermezzoShown) {
          doImmediateClockUpdate = true;
        }
      }
    } /* if clock is on */
  #endif /* PL_CONFIG_USE_INTERMEZZO */
    /* ----------------------------------------------------------------------------------*/
    /* Clock */
    /* ----------------------------------------------------------------------------------*/
#if PL_CONFIG_USE_CLOCK_TIME_OFF
    if (CLOCK_TimeOff.isTimeOnOffEnabled) {
      res = McuTimeDate_GetTimeDateAdjustDST(&time, &date);
      if (res==ERR_OK) {
        typedef enum {
          On_Off_Action_Nothing,
          On_Off_Action_Day_Turn_Off, /* turn off clock on days marked as off-days. This takes precedence over off-hours */
          On_Off_Action_Hour_Turn_On,
          On_Off_Action_Hour_Turn_Off,
        } On_Off_Action_e;
        On_Off_Action_e action = On_Off_Action_Nothing;
        bool isOffDay = (GetOffDays()&(1<<McuUtility_WeekDay(date.Year, date.Month, date.Day))); /* McuUtility_WeekDay() gives 0 for Sunday, 1, Monday, ... */

        /* check if we have an off-day */
        if (isOffDay) {
          if (CLOCK_ClockIsOn) { /* we are in an off-day, and clock is on -> turn it off */
            action = On_Off_Action_Day_Turn_Off;
          }
        } else {
          /* otherwise, check the hour settings */
          uint32_t offStartMinutes = CLOCK_TimeOff.offStartTime.Hour*60 + CLOCK_TimeOff.offStartTime.Min;
          uint32_t offEndMinutes = CLOCK_TimeOff.offEndTime.Hour*60 + CLOCK_TimeOff.offEndTime.Min;
          uint32_t currMinutes = time.Hour*60 + time.Min;

          if (offStartMinutes <= offEndMinutes) { /* e.g. 10:00 - 11:30 */
            if (CLOCK_ClockIsOn && !CLOCK_TimeOff.offIsActive && currMinutes>=offStartMinutes && currMinutes<=offEndMinutes) {
              /* clock is on, and we are in the off time range */
              action = On_Off_Action_Hour_Turn_Off;
            } else if (!CLOCK_ClockIsOn && CLOCK_TimeOff.offIsActive && (currMinutes<offStartMinutes || currMinutes>offEndMinutes)) {
              /* clock is off, and we are in the on time range */
              action = On_Off_Action_Hour_Turn_On;
            }
          } else { /* e.g. 19:00 - 03:00 */
            if (CLOCK_ClockIsOn && !CLOCK_TimeOff.offIsActive && (currMinutes>=offStartMinutes || currMinutes<=offEndMinutes)) {
              /* clock is on, and we are in the off time range */
              action = On_Off_Action_Hour_Turn_Off;
            } else if (!CLOCK_ClockIsOn && CLOCK_TimeOff.offIsActive && (currMinutes>offEndMinutes && currMinutes<offStartMinutes)) {
              /* clock is off, and we are in the on time range */
              action = On_Off_Action_Hour_Turn_On;
            }
          }
        }
        /* perform action */
        switch(action) {
          case On_Off_Action_Hour_Turn_On:
            McuLog_info("Off-time/date: turning clock on");
            CLOCK_TimeOff.offIsActive = false;
            CLOCK_On(CLOCK_MODE_ON);
            break;
          case On_Off_Action_Day_Turn_Off:
          case On_Off_Action_Hour_Turn_Off:
            McuLog_info("Off-time/date: turning clock off");
            CLOCK_TimeOff.offIsActive = true;
            CLOCK_On(CLOCK_MODE_OFF);
            break;
          case On_Off_Action_Nothing:
          default:
            break;
        }
      }
    }
#endif
    if (CLOCK_ClockIsOn) { /* show time */
      (void)McuTimeDate_GetTimeDateAdjustDST(&time, &date);
      if (doImmediateClockUpdate) { /* if not immediate update: sync on beginning of minute */
        doImmediateClockUpdate = false;
        if (time.Sec<=40) { /* do only update right now if we have enough time: will do a sync after 55 secs below anyway */
          CLOCK_ShowTimeDate(&time, &date);
        }
      }
      if (time.Sec>=55) { /* sync on start of the minute */
        McuLog_trace("Sync on full minute");
        do {
          (void)McuTimeDate_GetTimeDateAdjustDST(&time, &date);
          if (time.Sec==0) {
            break; /* leave loop */
          }
          vTaskDelay(pdMS_TO_TICKS(200));
        } while(res==ERR_OK);
        res = McuTimeDate_GetTimeDateAdjustDST(&time, &date);
      #if PL_CONFIG_USE_NEO_PIXEL_HW
        ShowSeconds(&time);
      #endif
        CLOCK_ShowTimeDate(&time, &date);
      #if PL_CONFIG_USE_INTERMEZZO /* show intermezzo? */
        lastClockUpdateTickCount = xTaskGetTickCount();
        intermezzoShown = false;
      #endif
        McuLog_info("finished showing clock");
      } /* if close to minute */
    } /* if clock is on */
  } /* for(;;) */
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
