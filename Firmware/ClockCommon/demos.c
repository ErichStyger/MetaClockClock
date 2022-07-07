/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_DEMOS
#include "demos.h"
#include "McuRTOS.h"
#include "stepper.h"
#include "matrix.h"
#include "matrixposition.h"
#include "matrixhand.h"
#include "matrixring.h"
#include "McuUtility.h"
#include "McuTimeDate.h"
#if PL_CONFIG_USE_NEO_PIXEL_HW
  #include "application.h"
#endif
#if PL_CONFIG_USE_CLOCK
  #include "clock.h"
#endif
#include "mfont.h"
#include "StepperBoard.h"
#include "NeoPixel.h"
#include "McuLog.h"

#if PL_CONFIG_IS_MASTER
static uint8_t DEMO_FailedDemo(uint8_t res) {
  return res; /* used to set a breakpoint in case of failure */
}
#endif

#if PL_MATRIX_CONFIG_IS_RGB
static void DEMO_LedDemo0(void) {
  MRING_EnableRingAll(true);
  MRING_SetRingColorAll(2, 0, 0);
  MATRIX_RequestRgbUpdate();
  vTaskDelay(pdMS_TO_TICKS(2000));

  MRING_SetRingColorAll(0, 2, 0);
  MATRIX_RequestRgbUpdate();
  vTaskDelay(pdMS_TO_TICKS(2000));

  MRING_SetRingColorAll(0, 0, 1);
  MATRIX_RequestRgbUpdate();
  vTaskDelay(pdMS_TO_TICKS(2000));

  MRING_SetRingColorAll(2, 2, 2);
  MATRIX_RequestRgbUpdate();
  vTaskDelay(pdMS_TO_TICKS(2000));

  MRING_SetRingColorAll(0, 0, 0);
  MATRIX_RequestRgbUpdate();
  vTaskDelay(pdMS_TO_TICKS(2000));
  MRING_EnableRingAll(false);
}
#endif /* PL_CONFIG_USE_NEO_PIXEL_HW */

#if PL_MATRIX_CONFIG_IS_RGB && PL_CONFIG_IS_MASTER && !PL_CONFIG_HAS_CIRCLE_CLOCK
typedef struct weather_ring_t {
  bool enabled;
  uint8_t r, g, b;
} weather_ring_t;

typedef struct weather_hand_t {
  bool enabled;
  int angle;
  uint8_t r, g, b;
} weather_hand_t;

typedef struct weather_clock_t {
  weather_hand_t hand[2];
  weather_ring_t ring[2];
} weather_clock_t;

#define BLUE_SKY      {.enabled=true, .r=0, .g=0, .b=1}
#define SNOW_SKY      {.enabled=true, .r=1, .g=1, .b=1}
#define RAIN_SKY      {.enabled=true, .r=1, .g=1, .b=1}
#define CLOUDY_SKY    {.enabled=true, .r=1, .g=1, .b=1}
#define YELLOW_SUN    {.enabled=true, .r=0x08, .g=0x08, .b=0}

#define BEAM_SUNNY    .enabled=true, .r=0x20, .g=0x20, .b=0x00
#define BEAM_CLOUDY   .enabled=true, .r=0x20, .g=0x20, .b=0x20
#define BEAM_BLUE     .enabled=true, .r=0x00, .g=0x00, .b=0x30
#define BEAM_ICY      .enabled=true, .r=0x15, .g=0x15, .b=0x15
#define BEAM_RAIN     .enabled=true, .r=0x00, .g=0x00, .b=0xa0

static const weather_clock_t weather_sunny[3][3] = {
  [0][0] = {  .hand[0] = {.angle=135, BEAM_SUNNY},
              .hand[1] = {.angle=315, BEAM_BLUE},
              .ring[0] = BLUE_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [0][1] = {  .hand[0] = {.angle=270, BEAM_BLUE},
              .hand[1] = {.angle= 90, BEAM_SUNNY},
              .ring[0] = BLUE_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [0][2] = {  .hand[0] = {.angle= 45, BEAM_SUNNY},
              .hand[1] = {.angle=225, BEAM_BLUE},
              .ring[0] = BLUE_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [1][0] = {  .hand[0] = {.angle=  0, BEAM_BLUE},
              .hand[1] = {.angle=180, BEAM_SUNNY},
              .ring[0] = BLUE_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [1][1] = {  .hand[0] = {.angle=  0, BEAM_SUNNY},
              .hand[1] = {.angle=180, BEAM_SUNNY},
              .ring[0] = YELLOW_SUN,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [1][2] = {  .hand[0] = {.angle=  0, BEAM_SUNNY},
              .hand[1] = {.angle=180, BEAM_BLUE},
              .ring[0] = BLUE_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [2][0] = {  .hand[0] = {.angle=225, BEAM_SUNNY},
              .hand[1] = {.angle= 45, BEAM_BLUE},
              .ring[0] = BLUE_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [2][1] = {  .hand[0] = {.angle=270, BEAM_SUNNY},
              .hand[1] = {.angle= 90, BEAM_BLUE},
              .ring[0] = BLUE_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [2][2] = {  .hand[0] = {.angle=315, BEAM_SUNNY},
              .hand[1] = {.angle=135, BEAM_BLUE},
              .ring[0] = BLUE_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
};
static const weather_clock_t weather_cloudy[3][3] = {
  [0][0] = {  .hand[0] = {.angle=135, BEAM_SUNNY},
              .hand[1] = {.angle=315, BEAM_BLUE},
              .ring[0] = BLUE_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [0][1] = {  .hand[0] = {.angle=270, BEAM_BLUE},
              .hand[1] = {.angle= 90, BEAM_SUNNY},
              .ring[0] = BLUE_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [0][2] = {  .hand[0] = {.angle= 45, BEAM_CLOUDY},
              .hand[1] = {.angle=225, BEAM_CLOUDY},
              .ring[0] = CLOUDY_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [1][0] = {  .hand[0] = {.angle=  0, BEAM_BLUE},
              .hand[1] = {.angle=180, BEAM_SUNNY},
              .ring[0] = BLUE_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [1][1] = {  .hand[0] = {.angle=  0, BEAM_SUNNY},
              .hand[1] = {.angle=180, BEAM_SUNNY},
              .ring[0] = YELLOW_SUN,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [1][2] = {  .hand[0] = {.angle=  0, BEAM_CLOUDY},
              .hand[1] = {.angle=180, BEAM_CLOUDY},
              .ring[0] = CLOUDY_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [2][0] = {  .hand[0] = {.angle=225, BEAM_SUNNY},
              .hand[1] = {.angle= 45, BEAM_BLUE},
              .ring[0] = BLUE_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [2][1] = {  .hand[0] = {.angle=270, BEAM_SUNNY},
              .hand[1] = {.angle= 90, BEAM_BLUE},
              .ring[0] = BLUE_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [2][2] = {  .hand[0] = {.angle=315, BEAM_CLOUDY},
              .hand[1] = {.angle=135, BEAM_CLOUDY},
              .ring[0] = CLOUDY_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
};

static const weather_clock_t weather_icy[3][3] = {
  [0][0] = {  .hand[0] = {.angle=135, BEAM_ICY},
              .hand[1] = {.angle=135, BEAM_ICY},
              .ring[0] = SNOW_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [0][1] = {  .hand[0] = {.angle= 90, BEAM_ICY},
              .hand[1] = {.angle= 90, BEAM_ICY},
              .ring[0] = SNOW_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [0][2] = {  .hand[0] = {.angle= 45, BEAM_ICY},
              .hand[1] = {.angle= 45, BEAM_ICY},
              .ring[0] = SNOW_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [1][0] = {  .hand[0] = {.angle=180, BEAM_ICY},
              .hand[1] = {.angle=180, BEAM_ICY},
              .ring[0] = SNOW_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [1][1] = {  .hand[0] = {.angle=270, BEAM_ICY},
              .hand[1] = {.angle= 90, BEAM_ICY},
              .ring[0] = SNOW_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [1][2] = {  .hand[0] = {.angle=  0, BEAM_ICY},
              .hand[1] = {.angle=  0, BEAM_ICY},
              .ring[0] = SNOW_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [2][0] = {  .hand[0] = {.angle=225, BEAM_ICY},
              .hand[1] = {.angle=225, BEAM_ICY},
              .ring[0] = SNOW_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [2][1] = {  .hand[0] = {.angle=270, BEAM_ICY},
              .hand[1] = {.angle=270, BEAM_ICY},
              .ring[0] = SNOW_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [2][2] = {  .hand[0] = {.angle=315, BEAM_ICY},
              .hand[1] = {.angle=315, BEAM_ICY},
              .ring[0] = SNOW_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
};

static const weather_clock_t weather_rainy[3][3] = {
  [0][0] = {  .hand[0] = {.angle=225, BEAM_RAIN},
              .hand[1] = {.angle=225, BEAM_RAIN},
              .ring[0] = RAIN_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [0][1] = {  .hand[0] = {.angle=225, BEAM_RAIN},
              .hand[1] = {.angle=225, BEAM_RAIN},
              .ring[0] = RAIN_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [0][2] = {  .hand[0] = {.angle=225, BEAM_RAIN},
              .hand[1] = {.angle=225, BEAM_RAIN},
              .ring[0] = RAIN_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [1][0] = {  .hand[0] = {.angle=225, BEAM_RAIN},
              .hand[1] = {.angle=225, BEAM_RAIN},
              .ring[0] = RAIN_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [1][1] = {  .hand[0] = {.angle=225, BEAM_RAIN},
              .hand[1] = {.angle=225, BEAM_RAIN},
              .ring[0] = RAIN_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [1][2] = {  .hand[0] = {.angle=225, BEAM_RAIN},
              .hand[1] = {.angle=225, BEAM_RAIN},
              .ring[0] = RAIN_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [2][0] = {  .hand[0] = {.angle=225, BEAM_RAIN},
              .hand[1] = {.angle=225, BEAM_RAIN},
              .ring[0] = RAIN_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [2][1] = {  .hand[0] = {.angle=225, BEAM_RAIN},
              .hand[1] = {.angle=225, BEAM_RAIN},
              .ring[0] = RAIN_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
  [2][2] = {  .hand[0] = {.angle=225, BEAM_RAIN},
              .hand[1] = {.angle=225, BEAM_RAIN},
              .ring[0] = RAIN_SKY,
              .ring[1] = {.enabled=false, .r=0, .g=0, .b=0}},
};

static void DEMO_ShowWeather(const weather_clock_t weather[3][3]) {
  int xPos, yPos;

  xPos = MATRIX_NOF_STEPPERS_X-3;
  yPos = 0;

#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
  (void)MATRIX_SetMoveDelayZ0Z1All(4, 4);
  MPOS_SetMoveModeAll(STEPPER_MOVE_MODE_SHORT);
  for(int x=0; x<3; x++) {
    for(int y=0; y<3; y++) {
      for(int z=0; z<2; z++) {
        MHAND_HandEnable(xPos+x, yPos+y, z, weather[x][y].hand[z].enabled);
        MHAND_SetHandColor(xPos+x, yPos+y, z, NEO_COMBINE_RGB(weather[x][y].hand[z].r, weather[x][y].hand[z].g, weather[x][y].hand[z].b));
        MRING_EnableRing(xPos+x, yPos+y, z, weather[x][y].ring[z].enabled);
        MRING_SetRingColor(xPos+x, yPos+y, z, weather[x][y].ring[z].r, weather[x][y].ring[z].g, weather[x][y].ring[z].b); /* yellow */
        MPOS_SetAngle(xPos+x, yPos+y, z, weather[x][y].hand[z].angle);
      }
    }
  }
  MATRIX_RequestRgbUpdate();
}
#endif

#if PL_MATRIX_CONFIG_IS_RGB
static NEO_PixelColor Rainbow(int32_t numOfSteps, int32_t step) {
  float r = 0.0;
  float g = 0.0;
  float b = 0.0;
  float h = (double)step / numOfSteps;
  int i = (int32_t)(h * 6);
  float f = h * 6.0 - i;
  float q = 1 - f;

  switch (i % 6)  {
      case 0:
          r = 1;
          g = f;
          b = 0;
          break;
      case 1:
          r = q;
          g = 1;
          b = 0;
          break;
      case 2:
          r = 0;
          g = 1;
          b = f;
          break;
      case 3:
          r = 0;
          g = q;
          b = 1;
          break;
      case 4:
          r = f;
          g = 0;
          b = 1;
          break;
      case 5:
          r = 1;
          g = 0;
          b = q;
          break;
      default:
        break;
  }
  r *= 255;
  g *= 255;
  b *= 255;
  return ((((int)r)<<16)|(((int)g)<<8))+(int)b;
}
#endif /* PL_MATRIX_CONFIG_IS_RGB */

#if PL_MATRIX_CONFIG_IS_RGB
static void DEMO_LedDemo1(void) {
  uint8_t rowStartStep[MATRIX_NOF_STEPPERS_Y]; /* rainbow color starting values */
  NEO_PixelColor color;
  uint8_t brightness = 2;
  int val = 0;

  /* fill in default row start values */
  for(int i=0; i<MATRIX_NOF_STEPPERS_Y;i++) {
    rowStartStep[i] = val;
    val+=20;
    if(val>=0xff) {
      val = 0;
    }
  }

  /* clear all */
  MRING_SetRingColorAll(0, 0, 0);
  MRING_EnableRingAll(true);
  MATRIX_RequestRgbUpdate();
  vTaskDelay(pdMS_TO_TICKS(100));

  for(int i=0; i<200; i++) {
    for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
      color = Rainbow(256, rowStartStep[y]);
      color = NEO_BrightnessPercentColor(color, brightness);
      rowStartStep[y]++;
      for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
        for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
          MRING_SetRingColor(x, y, z, NEO_SPLIT_RGB(color));
        }
      }
    }
    MATRIX_RequestRgbUpdate();
    vTaskDelay(pdMS_TO_TICKS(20));
  }
  MRING_EnableRingAll(false);
  MATRIX_RequestRgbUpdate();
}
#endif /* PL_MATRIX_CONFIG_IS_RGB */

#if PL_MATRIX_CONFIG_IS_RGB && PL_CONFIG_IS_MASTER && !PL_CONFIG_HAS_CIRCLE_CLOCK
/* ------------------ PONG Game ------------------------------- */
typedef struct {
  uint8_t r, g, b;
  int posX, posY;
  int dirX, dirY;
} PongBall_t;

typedef struct {
  uint8_t r, g, b;
  int posX, posY;
  int points;
} PongPlayer_t;

static void DrawBall(PongBall_t *b) {
  MRING_SetRingColor(b->posX, b->posY, 0, b->r, b->g, b->b);
}

static void ClearBall(PongBall_t *b) {
  MRING_SetRingColor(b->posX, b->posY, 0, 0, 0, 0);
}

static void DrawPlayer(PongPlayer_t *player) {
  MRING_SetRingColor(player->posX, player->posY, 0, player->r, player->g, player->b);
}

static void ClearPlayer(PongPlayer_t *player) {
  MRING_SetRingColor(player->posX, player->posY, 0, 0, 0, 0);
}

static void MovePlayer(PongPlayer_t *player, int deltaY) {
  ClearPlayer(player);
  player->posY += deltaY;
  if (player->posY<0) {
    player->posY = 0;
  } else if (player->posY>MATRIX_NOF_STEPPERS_Y-1) {
    player->posY =MATRIX_NOF_STEPPERS_Y-1;
  }
  DrawPlayer(player);
}

static void PongGameLost(PongPlayer_t *player, PongBall_t *ball) {
  /* mark ball in red */
  ball->r = 0x50;
  ball->g = 0;
  ball->b = 0;
  DrawBall(ball);
  DrawPlayer(player);
  MATRIX_RequestRgbUpdate();
  vTaskDelay(pdMS_TO_TICKS(100));
  for(int i=0; i<10; i++) {
    ClearBall(ball);
    MATRIX_RequestRgbUpdate();
    vTaskDelay(pdMS_TO_TICKS(100));
    DrawBall(ball);
    MATRIX_RequestRgbUpdate();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  MRING_SetRingColorAll(0, 0, 0);
  MRING_EnableRingAll(false);
  MFONT_PrintString((unsigned char*)"GAME", 0, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MFONT_PrintString((unsigned char*)"OVER", 0, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
}

static void ScorePoint(PongPlayer_t *player) {
  player->points++;
  /* blink it */
  player->r <<= 5;
  player->g <<= 5;
  player->b <<= 5;
  DrawPlayer(player);
  MATRIX_RequestRgbUpdate();
  vTaskDelay(pdMS_TO_TICKS(100));
  player->r >>= 5;
  player->g >>= 5;
  player->b >>= 5;
  DrawPlayer(player);
  MATRIX_RequestRgbUpdate();
  vTaskDelay(pdMS_TO_TICKS(100));
}

static bool MoveBall(PongBall_t *b, PongPlayer_t *left, PongPlayer_t *right) {
  bool gameOver = false;

  ClearBall(b);
  b->posX += b->dirX;
  b->posY += b->dirY;
  if (b->posY < 0) {
    b->posY = 1;
    b->dirY = -b->dirY;
  } else if (b->posY >= MATRIX_NOF_STEPPERS_Y) {
    b->posY = MATRIX_NOF_STEPPERS_Y-2;
    b->dirY = -b->dirY;
  }
  /* check collision with player */
  if (b->posX == left->posX) {
    if (b->posY != left->posY) {
      /* player left lost */
      PongGameLost(left, b);
      gameOver = true;
    } else {
      b->dirX = -b->dirX;
      b->dirY = -b->dirY;
      b->posX += b->dirX;
      b->posY += b->dirY;
      ScorePoint(left);
    }
  }
  if (b->posX == right->posX) {
    if (b->posY != right->posY) {
      /* player right lost */
      PongGameLost(right, b);
      gameOver = true;
    } else {
      b->dirX = -b->dirX;
      b->dirY = -b->dirY;
      b->posX += b->dirX;
      b->posY += b->dirY;
      ScorePoint(right);
    }
  }
  DrawBall(b);
  return gameOver;
}

static void DEMO_LedPong(void) {
  PongBall_t ball;
  PongPlayer_t playerL, playerR;
  bool gameOver;

  MHAND_SetHandColorAll(NEO_COMBINE_RGB(0x12, 0x12, 0x12));
  MRING_SetRingColorAll(0, 0, 0);
  MFONT_PrintString((unsigned char*)"    ", 0, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MFONT_PrintString((unsigned char*)"PONG", 0, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MFONT_PrintString((unsigned char*)"    ", 0, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);

  MHAND_HandEnableAll(false);
  MRING_EnableRingAll(true);
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
  /* ball: */
  ball.r = 0x4;
  ball.g = 0x4;
  ball.b = 0x4;
  ball.posX = 1;
  ball.posY = 1;
  ball.dirX = 1;
  ball.dirY = 1;
  /* player left: */
  playerL.r = 0;
  playerL.g = 0x4;
  playerL.b = 0;
  playerL.points = 0;
  playerL.posX = 0;
  playerL.posY = 0;
  /* player right: */
  playerR.r = 0;
  playerR.g = 0;
  playerR.b = 0x4;
  playerR.points = 0;
  playerR.posX = MATRIX_NOF_STEPPERS_X-1;
  playerR.posY = MATRIX_NOF_STEPPERS_Y/2;

  DrawPlayer(&playerL);
  DrawPlayer(&playerR);
  DrawBall(&ball);
  MATRIX_RequestRgbUpdate();
  vTaskDelay(pdMS_TO_TICKS(1000));

  for(int i=0; i<60; i++) {
    gameOver = MoveBall(&ball, &playerL, &playerR);
    DrawPlayer(&playerL);
    DrawPlayer(&playerR);
    MATRIX_RequestRgbUpdate();
    if (gameOver) {
      break;
    }
#if MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_X>=5
    if (i==1) {
      MovePlayer(&playerL, 1);
    } else if (i==3) {
      MovePlayer(&playerL, 1);
    } else if (i==7) {
      MovePlayer(&playerR, 1);
    } else if (i==9) {
      MovePlayer(&playerL, -1);
    } else if (i==15) {
      MovePlayer(&playerL, 1);
      MovePlayer(&playerR, -1);
    } else if (i==16) {
      MovePlayer(&playerL, -1);
    } else if (i==17) {
      MovePlayer(&playerL, -1);
    } else if (i==26) {
      MovePlayer(&playerR, -1);
    } else if (i==27) {
      MovePlayer(&playerR, 1);
    }
#elif MATRIX_NOF_STEPPERS_X>=8 && MATRIX_NOF_STEPPERS_Y>=3
    if (i==1) {
      MovePlayer(&playerL, 1);
    } else if (i==3) {
      MovePlayer(&playerR, 1);
    } else if (i==9) {
      MovePlayer(&playerL, -1);
    } else if (i==15) {
      MovePlayer(&playerL, -1);
      MovePlayer(&playerR, 1);
    }
#endif
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
#endif /* PL_MATRIX_CONFIG_IS_RGB */

#if PL_CONFIG_IS_MASTER && PL_MATRIX_CONFIG_IS_RGB && !PL_CONFIG_HAS_CIRCLE_CLOCK
static void evolve_univ(bool univ[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y], bool *changed) {
  /* see https://rosettacode.org/wiki/Conway%27s_Game_of_Life#C */
  unsigned newar[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y];

  for (int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for (int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      int n = 0; /* number of neighbors */
      for (int y1 = y - 1; y1 <= y + 1; y1++) {
        for (int x1 = x - 1; x1 <= x + 1; x1++) {
          if (univ[(x1 + MATRIX_NOF_STEPPERS_X) % MATRIX_NOF_STEPPERS_X][(y1 + MATRIX_NOF_STEPPERS_Y) % MATRIX_NOF_STEPPERS_Y]) {
            n++;
          }
        }
      }
      if (univ[x][y]) { /* we counted ourself twice */
        n--;
      }
      /* Any live cell with two or three live neighbors survives. Any dead cell with three live neighbors becomes a live cell. */
      newar[x][y] = (n == 3 || (n == 2 && univ[x][y]));
    }
  }
  for (int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for (int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      if (univ[x][y] != newar[x][y]) {
        *changed = true;
      }
      univ[x][y] = newar[x][y];
    }
  }
}

static void show_univ(bool univ[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y]) {
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MRING_EnableRing(x, y, 1, univ[x][y]);
    }
  }
  MATRIX_SendToRemoteQueueExecuteAndWait(false);
}

static void DEMO_GameOfLife(void) {
  bool univ[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y]; /* universum */
  bool changed;

  MHAND_SetHandColorAll(NEO_COMBINE_RGB(0x0, 0x10, 0x0));
  MRING_EnableRingAll(false);
  MFONT_PrintString((unsigned char*)"GAME", 0, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MFONT_PrintString((unsigned char*)" OF ", 0, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MFONT_PrintString((unsigned char*)"LIFE", 0, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MFONT_PrintString((unsigned char*)"    ", 0, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);

  MHAND_HandEnableAll(false);
  MRING_EnableRingAll(true);
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
  MRING_SetRingColorAll(0, 1, 0); /* color for each ring */
  /* seed random cells */
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      bool alive = McuUtility_random(0, 2)==1;
      univ[x][y] = alive;
      MRING_EnableRing(x, y, 0, alive);
      MRING_EnableRing(x, y, 1, false);
    }
  }
  MATRIX_SendToRemoteQueueExecuteAndWait(false); /* no need to wait as only changing LEDs */

  for(int i=0; i<50; i++) {
    changed = false;
    evolve_univ(univ, &changed);
    show_univ(univ);
    if (!changed) {
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
  MRING_EnableRingAll(false);
  MFONT_PrintString((unsigned char*)"OVER", 0, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(false); /* no need to wait as only changing LEDs */
}
#endif /* PL_CONFIG_IS_MASTER && PL_MATRIX_CONFIG_IS_RGB */


#if PL_CONFIG_IS_MASTER
static uint8_t DemoSquare(void) {
  uint8_t res;

#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
  (void)MATRIX_SetMoveDelayZ0Z1All(4, 4);
  MPOS_SetMoveModeAll(STEPPER_MOVE_MODE_SHORT);
  /* build initial squares */
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y+=2) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x+=2) {
      MPOS_SetAngleZ0Z1Checked(x,     y, 180,  90);
      MPOS_SetAngleZ0Z1Checked(x,   y+1,  90,   0);
      MPOS_SetAngleZ0Z1Checked(x+1,   y, 270, 180);
      MPOS_SetAngleZ0Z1Checked(x+1, y+1,   0, 270);
    }
  }
  /* build squares */
  res = MATRIX_SendToRemoteQueueExecuteAndWait(true);
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  return res;
}
#endif

#if PL_CONFIG_IS_MASTER
static uint8_t DemoSquareRotate(void) {
  uint8_t res;

  res = DemoSquare();
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  /* make two turns (2x360) */
  MPOS_SetMoveModeAll(STEPPER_MOVE_MODE_CW);
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y+=2) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x+=2) {
      MPOS_SetAngleZ0Z1Checked(x,     y, 180+2*360,  90+2*360);
      MPOS_SetAngleZ0Z1Checked(x,   y+1,  90+2*360,   0+2*360);
      MPOS_SetAngleZ0Z1Checked(x+1,   y, 270+2*360, 180+2*360);
      MPOS_SetAngleZ0Z1Checked(x+1, y+1,   0+2*360, 270+2*360);
    }
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  return res;
}
#endif

#if PL_CONFIG_IS_MASTER
static uint8_t DemoSquareClap(void) {
  uint8_t res;

  res = DemoSquare();
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  /* make two claps (2x360) */
  MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_CCW, STEPPER_MOVE_MODE_CW);
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y+=2) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x+=2) {
      MPOS_SetAngleZ0Z1Checked(x,     y, 180-2*360,  90+2*360);
      MPOS_SetAngleZ0Z1Checked(x,   y+1,  90-2*360,   0+2*360);
      MPOS_SetAngleZ0Z1Checked(x+1,   y, 270-2*360, 180+2*360);
      MPOS_SetAngleZ0Z1Checked(x+1, y+1,   0-2*360, 270+2*360);
    }
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  return res;
}
#endif

#if PL_CONFIG_IS_MASTER
static uint8_t DemoPropeller(void) {
  uint8_t res;

#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
  (void)MATRIX_SetMoveDelayZ0Z1All(4, 4);
  MPOS_SetMoveModeAll(STEPPER_MOVE_MODE_SHORT);
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetAngleZ0Z1(x, y, 0,  180);
    }
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  MPOS_SetMoveModeAll(STEPPER_MOVE_MODE_CW);
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetAngleZ0Z1(x, y, 0+2*360, 180+2*360);
    }
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  return ERR_OK;
}
#endif

#if PL_CONFIG_IS_MASTER
static uint8_t DemoFalling(void) {
  uint8_t res;

#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
  (void)MATRIX_SetMoveDelayZ0Z1All(4, 4);
  MPOS_SetMoveModeAll(STEPPER_MOVE_MODE_SHORT);
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetAngleZ0Z1(x, y, 0,  0);
    }
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y;y++) {
    MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_CCW, STEPPER_MOVE_MODE_CW);
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetAngleZ0Z1(x, y, 180, 180);
    }
    res = MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
    if (res!=ERR_OK) {
      return DEMO_FailedDemo(res);
    }
  }
  return ERR_OK;
}
#endif


#if PL_CONFIG_IS_ANALOG_CLOCK
static uint8_t DemoRandomHandsPos(void) {
#if PL_CONFIG_IS_MASTER
  uint8_t res;

#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(true);
#endif
  (void)MATRIX_SetMoveDelayZ0Z1All(2, 2);
  MPOS_SetMoveModeAll(STEPPER_MOVE_MODE_SHORT);
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetAngleZ0Z1(x, y, McuUtility_random(0, 359),  McuUtility_random(0, 359));
    }
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  return ERR_OK;
#else
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
        STEPPER_MoveClockDegreeAbs(MATRIX_GetStepper(x, y, z), McuUtility_random(0, 359), STEPPER_MOVE_MODE_SHORT, 2, true, true);
      }
    }
  }
  STEPPER_StartTimer();
  return ERR_OK;
#endif
}
#endif

#if PL_CONFIG_IS_MASTER

static uint8_t DEMO_Demo1(const McuShell_StdIOType *io) {
  int angle0, angle1;
  uint8_t res;

#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
  res = MATRIX_MoveAllto12(20000, io);
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  (void)MATRIX_SetMoveDelayZ0Z1All(2, 2);
  MPOS_SetAngleZ0Z1All(0, 180);
  res = MATRIX_SendToRemoteQueue(); /* queue command */
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  /* configure delays */
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MATRIX_SetMoveDelayZ0Z1Checked(x, y, 1+x, 1+x);
    }
  }

  /* rotate them to the next quadrant */
  angle0 = angle1 = 0;
  for(int i=0; i<3; i++) {
    angle0 = (angle0+90)%360;
    angle1 = (angle1+90)%360;
    MPOS_SetAngleZ0Z1All(angle0, angle1);
    res = MATRIX_SendToRemoteQueue(); /* queue command */
    if (res!=ERR_OK) {
      return DEMO_FailedDemo(res);
    }
  }
  /* execute */
  res = MATRIX_ExecuteRemoteQueueAndWait(true);
  return res;
}

#if PL_CONFIG_USE_FONT
static uint8_t DEMO_Demo2(const McuShell_StdIOType *io) {
  uint8_t res;
  uint8_t buf[8];

#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
  res = MATRIX_MoveAllto12(20000, io);
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  MATRIX_SetMoveDelayZ0Z1All(1, 1);
  MPOS_SetMoveModeAll(STEPPER_MOVE_MODE_SHORT);
  for(int i=2;i<12;i++) {
    buf[0] = '\0';
    McuUtility_strcatNum16uFormatted(buf,  sizeof(buf), 21, '0', 2);
    McuUtility_strcatNum16uFormatted(buf,  sizeof(buf), i, '0', 2);
    MFONT_ShowFramedText(0, 0, buf, MFONT_SIZE_2x3, true, true);
  }
  return MATRIX_MoveAllto12(10000, io);
}
#endif

#if PL_CONFIG_IS_MASTER && MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5 && PL_CONFIG_USE_FONT
static void DEMO_Nxp(void) {
  MATRIX_SetMoveDelayZ0Z1All(3, 3);
  MPOS_SetMoveModeAll(STEPPER_MOVE_MODE_SHORT);
#if PL_MATRIX_CONFIG_IS_RGB && PL_CONFIG_IS_ANALOG_CLOCK
  MHAND_HandEnableAll(true);
#endif
  DemoRandomHandsPos();
  MATRIX_Delay(2000);
  MFONT_PrintString((unsigned char*)"RUNS", 0, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MATRIX_Delay(1000);
#if PL_MATRIX_CONFIG_IS_RGB && PL_CONFIG_IS_ANALOG_CLOCK
  MHAND_HandEnableAll(true);
#endif
  DemoRandomHandsPos();
  MFONT_PrintString((unsigned char*)"WITH", 0, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MATRIX_Delay(1000);
#if PL_MATRIX_CONFIG_IS_RGB && PL_CONFIG_IS_ANALOG_CLOCK
  MHAND_HandEnableAll(true);
#endif
  DemoRandomHandsPos();
  MFONT_PrintString((unsigned char*)"    ", 0, 0, MFONT_SIZE_3x5);
  MFONT_PrintString((unsigned char*)"N", 1, 0, MFONT_SIZE_3x5);
  MFONT_PrintString((unsigned char*)"X", 4, 0, MFONT_SIZE_3x5);
  MFONT_PrintString((unsigned char*)"P", 7, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MATRIX_Delay(1000);
#if PL_MATRIX_CONFIG_IS_RGB && PL_CONFIG_IS_ANALOG_CLOCK
  MHAND_HandEnableAll(true);
#endif
  DemoRandomHandsPos();
  MFONT_PrintString((unsigned char*)"    ", 0, 0, MFONT_SIZE_3x5);
  MFONT_PrintString((unsigned char*)"L", 1, 0, MFONT_SIZE_3x5);
  MFONT_PrintString((unsigned char*)"P", 4, 0, MFONT_SIZE_3x5);
  MFONT_PrintString((unsigned char*)"C", 7, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MATRIX_Delay(1000);
#if PL_MATRIX_CONFIG_IS_RGB && PL_CONFIG_IS_ANALOG_CLOCK
  MHAND_HandEnableAll(true);
#endif
  DemoRandomHandsPos();
  MFONT_PrintString((unsigned char*)"    ", 0, 0, MFONT_SIZE_3x5);
  MFONT_PrintString((unsigned char*)"8", 1, 0, MFONT_SIZE_3x5);
  MFONT_PrintString((unsigned char*)"4", 4, 0, MFONT_SIZE_3x5);
  MFONT_PrintString((unsigned char*)"5", 7, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MATRIX_Delay(1000);
#if PL_MATRIX_CONFIG_IS_RGB && PL_CONFIG_IS_ANALOG_CLOCK
  MHAND_HandEnableAll(true);
#endif
  DemoRandomHandsPos();
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
}
#endif

static uint8_t DEMO_DemoCombined(const McuShell_StdIOType *io) {
  uint8_t res = ERR_OK;
#if PL_CONFIG_USE_FONT
  TIMEREC time;
  uint8_t buf[16];
#endif

#if PL_CONFIG_USE_FONT
  MATRIX_SetMoveDelayZ0Z1All(2, 2);
  McuTimeDate_GetTime(&time);
  buf[0] = '\0';
  McuUtility_strcatNum16uFormatted(buf,  sizeof(buf), time.Hour, '0', 2);
  McuUtility_strcatNum16uFormatted(buf,  sizeof(buf), time.Min, '0', 2);
#if MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5
  (void)MFONT_ShowFramedText(0, 0, buf, MFONT_SIZE_3x5, false, true);
#else
  (void)MFONT_ShowFramedText(0, 0, buf, MFONT_SIZE_2x3, false, true);
#endif
#endif

  (void)DemoRandomHandsPos();
  MATRIX_Delay(2000);
  (void)DemoRandomHandsPos();
  MATRIX_Delay(2000);
  (void)DemoRandomHandsPos();
  MATRIX_Delay(2000);

#if PL_CONFIG_USE_FONT
  MATRIX_SetMoveDelayZ0Z1All(2, 2);
  res = MFONT_ShowFramedText(0, 0, (unsigned char*)"2034", MFONT_SIZE_2x3, false, true);
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  MATRIX_Delay(3000);
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#endif

#if PL_CONFIG_USE_FONT
  MATRIX_SetMoveDelayZ0Z1All(2, 2);
  McuTimeDate_GetTime(&time);
  buf[0] = '\0';
  McuUtility_strcatNum16uFormatted(buf,  sizeof(buf), time.Hour, '0', 2);
  McuUtility_strcatNum16uFormatted(buf,  sizeof(buf), time.Min, '0', 2);
  (void)MFONT_ShowFramedText(0, 0, buf, MFONT_SIZE_2x3, false, true);
  MATRIX_Delay(3000);
#endif

#if PL_CONFIG_USE_FONT
  MATRIX_SetMoveDelayZ0Z1All(2, 2);
  res = MFONT_ShowFramedText(0, 0, (unsigned char*)"22" MFONT_STR_DEGREE "C", MFONT_SIZE_2x3, true, true);
  MATRIX_Delay(3000);
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
#if PL_CONFIG_USE_NEO_PIXEL_HW
  MHAND_HandEnableAll(true);
#endif
#endif

  res = DEMO_Demo1(io);
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }

#if PL_CONFIG_USE_FONT
  MATRIX_SetMoveDelayZ0Z1All(2, 2);
  McuTimeDate_GetTime(&time);
  buf[0] = '\0';
  McuUtility_strcatNum16uFormatted(buf,  sizeof(buf), time.Hour, '0', 2);
  McuUtility_strcatNum16uFormatted(buf,  sizeof(buf), time.Min, '0', 2);
  (void)MFONT_ShowFramedText(0, 0, buf, MFONT_SIZE_2x3, true, true);
  MATRIX_Delay(3000);
#endif

#if PL_CONFIG_USE_NEO_PIXEL_HW
  MHAND_HandEnableAll(true);
#endif
  MATRIX_Delay(3000);
  return MATRIX_MoveAllto12(10000, io);
}
#endif /* PL_CONFIG_IS_MASTER */

#if PL_CONFIG_USE_NEO_PIXEL_HW || PL_MATRIX_CONFIG_IS_RGB
static uint8_t DemoRandomHandsColor(void) {
#if PL_CONFIG_USE_NEO_PIXEL_HW
  uint32_t color;

  MHAND_HandEnableAll(true);
  for (int i=0; i<10; i++) {
    for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
      for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
        for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
          MHAND_SetHandColor(x, y, z, NEO_COMBINE_RGB(McuUtility_random(1, 255),  McuUtility_random(1, 255), McuUtility_random(1, 255)));
        }
      }
    }
    APP_RequestUpdateLEDs();
    vTaskDelay(pdMS_TO_TICKS(500));
  }
  color = MATRIX_GetHandColorAdjusted();
  MHAND_SetHandColorAll(color);
  return ERR_OK;
#elif PL_MATRIX_CONFIG_IS_RGB
  uint8_t res;

  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
        uint8_t r, g, b;

        r = McuUtility_random(1, 255);
        g = McuUtility_random(1, 255);
        b = McuUtility_random(1, 255);
        MHAND_SetHandColor(x, y, z, NEO_COMBINE_RGB(r,g,b));
      }
    }
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  return ERR_OK;
#else
  return ERR_FAILED;
#endif /* PL_CONFIG_USE_NEO_PIXEL_HW */
}
#endif

#if PL_CONFIG_USE_NEO_PIXEL_HW || PL_MATRIX_CONFIG_IS_RGB
static uint8_t DemoRandomRingColor(void) {
#if PL_CONFIG_USE_NEO_PIXEL_HW
  MHAND_HandEnableAll(false);
  MRING_EnableRingAll(true);
  for(int i=0; i<10; i++) {
    for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
      for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
        for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
          (void)MRING_SetRingColor(x, y, z, McuUtility_random(0, 0x5),  McuUtility_random(0, 0x5), McuUtility_random(0, 0x5)); /* limit brightness */
        }
      }
    }
    APP_RequestUpdateLEDs();
    vTaskDelay(pdMS_TO_TICKS(500));
  } /* for */
  MRING_EnableRingAll(false);
  MHAND_HandEnableAll(true);
  return ERR_OK;
#elif PL_MATRIX_CONFIG_IS_RGB
  uint8_t res;

  MHAND_HandEnableAll(false);
  MRING_EnableRingAll(true);
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      for(int z=0; z<MATRIX_NOF_STEPPERS_Z; z++) {
        uint8_t r, g, b;

        r = McuUtility_random(1, 5);
        g = McuUtility_random(1, 5);
        b = McuUtility_random(1, 5);
        MATRIX_DrawRingColor(x, y, z, NEO_COMBINE_RGB(r,g,b));
      }
    }
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  return ERR_OK;
#else
  return ERR_FAILED;
#endif /* PL_CONFIG_USE_NEO_PIXEL_HW */
}
#endif

#if PL_CONFIG_IS_MASTER || PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN128
static uint8_t DemoClap(void) {
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
#endif
#if PL_CONFIG_IS_MASTER
  (void)MATRIX_SetMoveDelayZ0Z1All(4, 4);
  MPOS_SetMoveModeAll(STEPPER_MOVE_MODE_SHORT);
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      MPOS_SetAngleZ0Z1(x, y, 0,  180);
    }
  }
  uint8_t res;

  res = MATRIX_SendToRemoteQueueExecuteAndWait(true);
  if (res!=ERR_OK) {
    McuLog_error("failed executing: %d", res);
  }
  for (int i=0; i<2; i++) {
    MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CCW);
    for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
      for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
        MPOS_SetAngleZ0Z1(x, y, 90, 90);
      }
    }
    (void)MATRIX_SendToRemoteQueue();
    MPOS_SetMoveModeZ0Z1All(STEPPER_MOVE_MODE_CCW, STEPPER_MOVE_MODE_CW);
    for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
      for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
        MPOS_SetAngleZ0Z1(x, y, 0, 180);
      }
    }
    (void)MATRIX_SendToRemoteQueue();
  }
  return MATRIX_ExecuteRemoteQueueAndWait(true);
#else
  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      STEPPER_MoveClockDegreeAbs(MATRIX_GetStepper(x, y, 0), 0, STEPPER_MOVE_MODE_SHORT, 4, true, true);
      STEPPER_MoveClockDegreeAbs(MATRIX_GetStepper(x, y, 1), 180, STEPPER_MOVE_MODE_SHORT, 4, true, true);
    }
  }
  STEPPER_StartTimer();
  while(!STEPBOARD_IsIdle(STEPBOARD_GetBoard())) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      STEPPER_MoveClockDegreeAbs(MATRIX_GetStepper(x, y, 0), 90, STEPPER_MOVE_MODE_CW, 4, true, true);
      STEPPER_MoveClockDegreeAbs(MATRIX_GetStepper(x, y, 1), 90, STEPPER_MOVE_MODE_CCW, 4, true, true);
    }
  }
  STEPPER_StartTimer();
  while(!STEPBOARD_IsIdle(STEPBOARD_GetBoard())) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  for(int y=0; y<MATRIX_NOF_STEPPERS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
      STEPPER_MoveClockDegreeAbs(MATRIX_GetStepper(x, y, 0), 0, STEPPER_MOVE_MODE_CCW, 4, true, true);
      STEPPER_MoveClockDegreeAbs(MATRIX_GetStepper(x, y, 1), 180, STEPPER_MOVE_MODE_CW, 4, true, true);
    }
  }
  STEPPER_StartTimer();
  while(!STEPBOARD_IsIdle(STEPBOARD_GetBoard())) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  return ERR_OK;
#endif
}
#endif

#if PL_CONFIG_IS_MASTER
static uint8_t DemoMiddle(void) {
  int x, y;

  /* top and bottom row */
  for(x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
    MPOS_SetAngleZ0Z1(x, 0, 180, 180);
#if PL_CONFIG_USE_EXTENDED_HANDS
    MHAND_2ndHandEnable(x, 0, 0, false);
    MHAND_2ndHandEnable(x, 0, 1, false);
#endif

    MPOS_SetAngleZ0Z1(x, MATRIX_NOF_STEPPERS_Y-1, 0, 0);
#if PL_CONFIG_USE_EXTENDED_HANDS
    MHAND_2ndHandEnable(x, MATRIX_NOF_STEPPERS_Y-1, 0, false);
    MHAND_2ndHandEnable(x, MATRIX_NOF_STEPPERS_Y-1, 1, false);
#endif
  }

  /* remaining rows in the middle */
  for(x=0; x<MATRIX_NOF_STEPPERS_X; x++) {
    for(y=1; y<MATRIX_NOF_STEPPERS_Y-1; y++) {
      MPOS_SetAngleZ0Z1(x, y, 0, 0);
  #if PL_CONFIG_USE_EXTENDED_HANDS
      MHAND_2ndHandEnable(x, y, 0, true);
      MHAND_2ndHandEnable(x, y, 1, true);
  #endif
    }
  }

  (void)MATRIX_SendToRemoteQueue();
  return MATRIX_ExecuteRemoteQueueAndWait(true);
}
#endif

#if PL_CONFIG_USE_SHELL
static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"demo", (unsigned char*)"Group of demo commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
  #if PL_CONFIG_IS_ANALOG_CLOCK
  McuShell_SendHelpStr((unsigned char*)"  hands random pos", (unsigned char*)"Randomize hand position\r\n", io->stdOut);
#if PL_MATRIX_CONFIG_IS_RGB
  McuShell_SendHelpStr((unsigned char*)"  hands random color", (unsigned char*)"Randomize hand color\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  ring random color", (unsigned char*)"Randomize ring color\r\n", io->stdOut);
#endif
  McuShell_SendHelpStr((unsigned char*)"  clap", (unsigned char*)"Clapping hands\r\n", io->stdOut);
  #endif
  #if PL_CONFIG_IS_MASTER
  McuShell_SendHelpStr((unsigned char*)"  square", (unsigned char*)"Building squares\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  square rotate", (unsigned char*)"Rotating squares\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  square clap", (unsigned char*)"Squares clapping\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  falling", (unsigned char*)"Falling hands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  propeller", (unsigned char*)"Rotating propeller\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  demo combined", (unsigned char*)"Combined demo\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  1", (unsigned char*)"Demo with changing angles\r\n", io->stdOut);
#if PL_CONFIG_USE_FONT
  McuShell_SendHelpStr((unsigned char*)"  2", (unsigned char*)"Demo fast clock\r\n", io->stdOut);
#endif
  McuShell_SendHelpStr((unsigned char*)"  delay <delay>", (unsigned char*)"Set default delay\r\n", io->stdOut);
  #if PL_CONFIG_IS_MASTER && MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5 && PL_CONFIG_USE_FONT
  McuShell_SendHelpStr((unsigned char*)"  nxp", (unsigned char*)"NXP demo\r\n", io->stdOut);
  #endif
  McuShell_SendHelpStr((unsigned char*)"  middle", (unsigned char*)"middle demo\r\n", io->stdOut);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  McuShell_SendHelpStr((unsigned char*)"  led 0", (unsigned char*)"LED color demo\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  led 1", (unsigned char*)"LED rainbow demo\r\n", io->stdOut);
#endif
#if PL_MATRIX_CONFIG_IS_RGB && PL_CONFIG_IS_MASTER && !PL_CONFIG_HAS_CIRCLE_CLOCK
  McuShell_SendHelpStr((unsigned char*)"  pong", (unsigned char*)"pong demo\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  weather <weather>", (unsigned char*)"Show weather: sunny, cloudy, rainy, icy\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  game of life", (unsigned char*)"Conway's Game of Life\r\n", io->stdOut);
#endif
  return ERR_OK;
}

#if PL_CONFIG_USE_CLOCK
static uint8_t CheckIfClockIsOn(const McuShell_StdIOType *io) {
  if (CLOCK_GetClockIsOn()) {
    McuShell_SendStr((unsigned char*)"Clock is on, disable it first with 'clock off'.\n", io->stdErr);
    return ERR_FAILED;
  }
  return ERR_OK;
}
#endif

uint8_t DEMO_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "demo help")==0) {
    *handled = true;
    return PrintHelp(io);
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "demo status")==0)) {
    *handled = true;
    return PrintStatus(io);
#if PL_CONFIG_IS_MASTER || PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN128
  } else if (McuUtility_strcmp((char*)cmd, "demo hands random pos")==0) {
    *handled = TRUE;
  #if PL_CONFIG_USE_CLOCK
    if (CheckIfClockIsOn(io)!=ERR_OK) {
      return ERR_FAILED;
    }
  #endif
    return DemoRandomHandsPos();
  } else if (McuUtility_strcmp((char*)cmd, "demo clap")==0) {
    *handled = TRUE;
  #if PL_CONFIG_USE_CLOCK
    if (CheckIfClockIsOn(io)!=ERR_OK) {
      return ERR_FAILED;
    }
  #endif
    return DemoClap();
#endif
#if PL_CONFIG_IS_MASTER
  } else if (McuUtility_strcmp((char*)cmd, "demo square")==0) {
    *handled = TRUE;
  #if PL_CONFIG_USE_CLOCK
    if (CheckIfClockIsOn(io)!=ERR_OK) {
      return ERR_FAILED;
    }
  #endif
    return DemoSquare();
  } else if (McuUtility_strcmp((char*)cmd, "demo square rotate")==0) {
    *handled = TRUE;
  #if PL_CONFIG_USE_CLOCK
    if (CheckIfClockIsOn(io)!=ERR_OK) {
      return ERR_FAILED;
    }
  #endif
    return DemoSquareRotate();
  } else if (McuUtility_strcmp((char*)cmd, "demo square clap")==0) {
    *handled = TRUE;
  #if PL_CONFIG_USE_CLOCK
    if (CheckIfClockIsOn(io)!=ERR_OK) {
      return ERR_FAILED;
    }
  #endif
    return DemoSquareClap();
  } else if (McuUtility_strcmp((char*)cmd, "demo falling")==0) {
    *handled = TRUE;
  #if PL_CONFIG_USE_CLOCK
    if (CheckIfClockIsOn(io)!=ERR_OK) {
      return ERR_FAILED;
    }
  #endif
    return DemoFalling();
  } else if (McuUtility_strcmp((char*)cmd, "demo propeller")==0) {
    *handled = TRUE;
  #if PL_CONFIG_USE_CLOCK
    if (CheckIfClockIsOn(io)!=ERR_OK) {
      return ERR_FAILED;
    }
  #endif
    return DemoPropeller();
  } else if (McuUtility_strcmp((char*)cmd, "demo combined")==0) {
    *handled = true;
    return DEMO_DemoCombined(io);
  } else if (McuUtility_strcmp((char*)cmd, "demo 1")==0) {
    *handled = true;
    return DEMO_Demo1(io);
#if PL_CONFIG_USE_FONT
  } else if (McuUtility_strcmp((char*)cmd, "demo 2")==0) {
    *handled = true;
    return DEMO_Demo2(io);
#endif
  } else if (McuUtility_strcmp((char*)cmd, "demo middle")==0) {
    *handled = TRUE;
    return DemoMiddle();
  #if PL_CONFIG_IS_MASTER && MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5 && PL_CONFIG_USE_FONT
  } else if (McuUtility_strcmp((char*)cmd, "demo nxp")==0) {
    *handled = TRUE;
    DEMO_Nxp();
    return ERR_OK;
  #endif
#endif /* PL_CONFIG_IS_MASTER */
#if PL_MATRIX_CONFIG_IS_RGB && PL_CONFIG_IS_MASTER && !PL_CONFIG_HAS_CIRCLE_CLOCK
  } else if (McuUtility_strcmp((char*)cmd, "demo weather sunny")==0) {
    *handled = TRUE;
    DEMO_ShowWeather(weather_sunny);
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, "demo weather cloudy")==0) {
    *handled = TRUE;
    DEMO_ShowWeather(weather_cloudy);
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, "demo weather icy")==0) {
    *handled = TRUE;
    DEMO_ShowWeather(weather_icy);
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, "demo weather rainy")==0) {
    *handled = TRUE;
    DEMO_ShowWeather(weather_rainy);
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, "demo pong")==0) {
    *handled = TRUE;
    DEMO_LedPong();
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, "demo game of life")==0) {
    *handled = TRUE;
    DEMO_GameOfLife();
    return ERR_OK;
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  } else if (McuUtility_strcmp((char*)cmd, "demo led 0")==0) {
    *handled = TRUE;
    DEMO_LedDemo0();
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, "demo led 1")==0) {
    *handled = TRUE;
    DEMO_LedDemo1();
    return ERR_OK;
#endif /* PL_CONFIG_USE_NEO_PIXEL_HW */
#if PL_MATRIX_CONFIG_IS_RGB
  } else if (McuUtility_strcmp((char*)cmd, "demo ring random color")==0) {
    *handled = TRUE;
    #if PL_CONFIG_USE_CLOCK
    if (CheckIfClockIsOn(io)!=ERR_OK) {
      return ERR_FAILED;
    }
    #endif
    return DemoRandomRingColor();
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  } else if (McuUtility_strcmp((char*)cmd, "demo hands random color")==0) {
    *handled = TRUE;
    #if PL_CONFIG_USE_CLOCK
    if (CheckIfClockIsOn(io)!=ERR_OK) {
      return ERR_FAILED;
    }
    #endif
    return DemoRandomHandsColor();
#endif
  }
  return ERR_OK;
}
#endif

void DEMO_Init(void) {
}
#endif /* PL_CONFIG_USE_DEMOS */
