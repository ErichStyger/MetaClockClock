/*
 * Copyright (c) 2021, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "circleClock.h"
#include "matrixposition.h"
#include "matrix.h"

#if PL_CONFIG_HAS_CIRCLE_CLOCK

#define CIRCLE_CLOCK_MINUTE_Z_POS           (0)  /* Z position of minute hand */
#define CIRCLE_CLOCK_HOUR_Z_POS             (1)  /* Z position of hour hand */

#define CIRCLE_CLOCK_CIRCLE_START_X_POS     (1)  /* x start position of outer circle */
#define CIRCLE_CLOCK_CIRCLE_END_X_POS       (12)  /* x end position of outer circle */

#define CIRCLE_CLOCK_MIDDLE_X_POS           (0)  /* X position of middle clock */
#define CIRCLE_CLOCK_MIDDLE_Y_POS           (0)  /* Y position of middle clock */

static const uint16_t circleHourAngles[12][2] = {
  /* 1 */  {300-5, 120+5},
  /* 2 */  {330-5, 150+5},
  /* 3 */  {  0-5, 180+5},
  /* 4 */  { 30-5, 210+5},
  /* 5 */  { 60-5, 240+5},
  /* 6 */  { 90-5, 270+5},
  /* 7 */  {120-5, 300+5},
  /* 8 */  {150-5, 330+5},
  /* 9 */  {180-5,   0+5},
  /* 10 */ {210-5,  30+5},
  /* 11 */ {240-5,  60+5},
  /* 12 */ {270-5,  90+5},
};

static void CC_DrawCircle(void) {
  /* Draw a circle with the hands of the outside ring
   * Outside clocks are from (1,0) to (12,0)
   */
  int hour = 0; /* 0 is hour 1 */

  for(int pos=CIRCLE_CLOCK_CIRCLE_START_X_POS; pos<=CIRCLE_CLOCK_CIRCLE_END_X_POS; pos++) {
    MPOS_SetAngle(pos, CIRCLE_CLOCK_MIDDLE_Y_POS, CIRCLE_CLOCK_HOUR_Z_POS, circleHourAngles[hour][0]); /* hour */
    MPOS_SetAngle(pos, CIRCLE_CLOCK_MIDDLE_Y_POS, CIRCLE_CLOCK_MINUTE_Z_POS, circleHourAngles[hour][1]); /* minute */
    hour++;
  }
}

void CC_ShowTime(uint8_t hour, uint8_t minute) {
  int32_t angleHour, angleMinute;

  /* center clock is at (0,0) */
  minute %= 60; /* make it 0..59 */
  hour %= 12; /* make it 0..11 */
  angleMinute = (360/60)*minute;
  angleHour = (360/12)*hour + ((360/12)*minute)/60;
  MPOS_SetAngle(CIRCLE_CLOCK_MIDDLE_X_POS, CIRCLE_CLOCK_MIDDLE_Y_POS, CIRCLE_CLOCK_HOUR_Z_POS, angleHour); /* hour */
  MPOS_SetAngle(CIRCLE_CLOCK_MIDDLE_X_POS, CIRCLE_CLOCK_MIDDLE_Y_POS, CIRCLE_CLOCK_MINUTE_Z_POS, angleMinute); /* minute */

  CC_DrawCircle();
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

void CC_Deinit(void) {
}

void CC_Init(void) {
}
#endif /* PL_CONFIG_HAS_CIRCLE_CLOCK */
