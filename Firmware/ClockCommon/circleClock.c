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

/* build a circle around the clock */
static const uint16_t circleHourAngles[12 /* nof digits */][2 /* Z dimension */] = {
           /* low  high */
  /* 1 */  {300-5, 120+5},
  /* 2 */  {330-5, 150+5},
  /* 3 */  {360-5, 180+5},
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

/* build a rays around the clock */
static const uint16_t raysHourAngles[12 /* nof digits */][2 /* Z dimension */] = {
           /* low    high */
  /* 1 */  {0+30,  180+30},
  /* 2 */  {0+60,  180+60},
  /* 3 */  {0+90,  180+90},
  /* 4 */  {0+120, 180+120},
  /* 5 */  {0+150, 180+150},
  /* 6 */  {0+180, 180+180},
  /* 7 */  {0+210, 180+210},
  /* 8 */  {0+240, 180+240},
  /* 9 */  {0+270, 180+270},
  /* 10 */ {0+300, 180+300},
  /* 11 */ {0+330, 180+330},
  /* 12 */ {0+360, 180+360},
};

/* build a rays around the clock, only outside */
static const uint16_t raysOutsideHourAngles[12 /* nof digits */][2 /* Z dimension */] = {
           /* low    high */
  /* 1 */  {0+30,  0+30},
  /* 2 */  {0+60,  0+60},
  /* 3 */  {0+90,  0+90},
  /* 4 */  {0+120, 0+120},
  /* 5 */  {0+150, 0+150},
  /* 6 */  {0+180, 0+180},
  /* 7 */  {0+210, 0+210},
  /* 8 */  {0+240, 0+240},
  /* 9 */  {0+270, 0+270},
  /* 10 */ {0+300, 0+300},
  /* 11 */ {0+330, 0+330},
  /* 12 */ {0+360, 0+360},
};

/* build a rays around the clock, only inside */
static const uint16_t raysInsideHourAngles[12 /* nof digits */][2 /* Z dimension */] = {
           /* low    high */
  /* 1 */  {180+30,  180+30},
  /* 2 */  {180+60,  180+60},
  /* 3 */  {180+90,  180+90},
  /* 4 */  {180+120, 180+120},
  /* 5 */  {180+150, 180+150},
  /* 6 */  {180+180, 180+180},
  /* 7 */  {180+210, 180+210},
  /* 8 */  {180+240, 180+240},
  /* 9 */  {180+270, 180+270},
  /* 10 */ {180+300, 180+300},
  /* 11 */ {180+330, 180+330},
  /* 12 */ {180+360, 180+360},
};

/* build a rays around the clock, marking 3, 6, 9 and 12 */
static const uint16_t raysMarekdHourAngles[12 /* nof digits */][2 /* Z dimension */] = {
           /* low    high */
  /* 1 */  {0+30,   0+30},
  /* 2 */  {0+60,   0+60},
  /* 3 */  {270,      90},
  /* 4 */  {0+120, 0+120},
  /* 5 */  {0+150, 0+150},
  /* 6 */  {0,       180},
  /* 7 */  {0+210, 0+210},
  /* 8 */  {0+240, 0+240},
  /* 9 */  {270,      90},
  /* 10 */ {0+300, 0+300},
  /* 11 */ {0+330, 0+330},
  /* 12 */ {0,       180},
};


static void CC_DrawAngles(const uint16_t angles[12][2]) {
  int hour = 0; /* index 0 is hour 1 */

  for(int pos=CIRCLE_CLOCK_CIRCLE_START_X_POS; pos<=CIRCLE_CLOCK_CIRCLE_END_X_POS; pos++) {
    MPOS_SetAngle(pos, CIRCLE_CLOCK_MIDDLE_Y_POS, CIRCLE_CLOCK_HOUR_Z_POS, angles[hour][0]%360); /* hour */
    MPOS_SetAngle(pos, CIRCLE_CLOCK_MIDDLE_Y_POS, CIRCLE_CLOCK_MINUTE_Z_POS, angles[hour][1]%360); /* minute */
    hour++;
  }
}
void CC_DrawCircle(void) {
  /* Draw a circle with the hands of the outside ring
   * Outside clocks are from (1,0) to (12,0) */
  CC_DrawAngles(circleHourAngles);
}

void CC_DrawRays(void) {
  CC_DrawAngles(raysHourAngles);
}

void CC_ShowTime(uint8_t hour, uint8_t minute) {
  static uint8_t whichOutside = 0;
  int32_t angleHour, angleMinute;

  /* center clock is at (0,0) */
  minute %= 60; /* make it 0..59 */
  hour %= 12; /* make it 0..11 */
  angleMinute = (360/60)*minute;
  angleHour = (360/12)*hour + ((360/12)*minute)/60;
  MPOS_SetAngle(CIRCLE_CLOCK_MIDDLE_X_POS, CIRCLE_CLOCK_MIDDLE_Y_POS, CIRCLE_CLOCK_HOUR_Z_POS, angleHour); /* hour */
  MPOS_SetAngle(CIRCLE_CLOCK_MIDDLE_X_POS, CIRCLE_CLOCK_MIDDLE_Y_POS, CIRCLE_CLOCK_MINUTE_Z_POS, angleMinute); /* minute */

  if (whichOutside==0) {
    CC_DrawAngles(circleHourAngles);
  } else if (whichOutside==1) {
    CC_DrawAngles(raysHourAngles);
  } else if (whichOutside==2) {
    CC_DrawAngles(raysOutsideHourAngles);
  } else if (whichOutside==3) {
    CC_DrawAngles(raysInsideHourAngles);
  } else if (whichOutside==4) {
    CC_DrawAngles(raysMarekdHourAngles);
  }
  whichOutside++;
  if (whichOutside==5) {
    whichOutside = 0; /* reset */
  }

  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

void CC_Deinit(void) {
}

void CC_Init(void) {
}
#endif /* PL_CONFIG_HAS_CIRCLE_CLOCK */
