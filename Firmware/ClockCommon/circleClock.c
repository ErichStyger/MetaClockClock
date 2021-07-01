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
  /* 1 */  {300, 120},
  /* 2 */  {330, 150},
  /* 3 */  {  0, 180},
  /* 4 */  { 30, 210},
  /* 5 */  { 60, 240},
  /* 6 */  { 90, 270},
  /* 7 */  {120, 300},
  /* 8 */  {150, 330},
  /* 9 */  {180,   0},
  /* 10 */ {210,  30},
  /* 11 */ {240,  60},
  /* 12 */ {270,  90},
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
