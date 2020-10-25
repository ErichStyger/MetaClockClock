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
#include "McuUtility.h"
#include "McuTimeDate.h"
#if PL_CONFIG_USE_LED_RING
  #include "application.h"
#endif
#if PL_CONFIG_USE_CLOCK
  #include "clock.h"
#endif
#include "mfont.h"
#include "StepperBoard.h"
#include "NeoPixel.h"

#if PL_CONFIG_IS_MASTER
static uint8_t DEMO_FailedDemo(uint8_t res) {
  return res; /* used to set a breakpoint in case of failure */
}
#endif

#if PL_CONFIG_USE_LED_RING
static void DEMO_LedDemo0(void) {
  MATRIX_SetRingLedEnabledAll(true);
  MATRIX_SetRingColorAll(2, 0, 0);
  APP_RequestUpdateLEDs();
  vTaskDelay(pdMS_TO_TICKS(2000));

  MATRIX_SetRingColorAll(0, 2, 0);
  APP_RequestUpdateLEDs();
  vTaskDelay(pdMS_TO_TICKS(2000));

  MATRIX_SetRingColorAll(0, 0, 1);
  APP_RequestUpdateLEDs();
  vTaskDelay(pdMS_TO_TICKS(2000));

  MATRIX_SetRingColorAll(2, 2, 2);
  APP_RequestUpdateLEDs();
  vTaskDelay(pdMS_TO_TICKS(2000));

  MATRIX_SetRingColorAll(0, 0, 0);
  APP_RequestUpdateLEDs();
  vTaskDelay(pdMS_TO_TICKS(2000));
  MATRIX_SetRingLedEnabledAll(false);
}
#endif /* PL_CONFIG_USE_LED_RING */

#if PL_CONFIG_USE_LED_RING && PL_CONFIG_IS_MASTER
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

  xPos = MATRIX_NOF_CLOCKS_X-3;
  yPos = 0;

#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
  (void)MATRIX_DrawAllClockDelays(4, 4);
  (void)MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  for(int x=0; x<3; x++) {
    for(int y=0; y<3; y++) {
      for(int z=0; z<2; z++) {
        MATRIX_SetHandLedEnabled(xPos+x, yPos+y, z, weather[x][y].hand[z].enabled);
        MATRIX_SetHandColor(xPos+x, yPos+y, z, weather[x][y].hand[z].r, weather[x][y].hand[z].g, weather[x][y].hand[z].b);
        MATRIX_SetRingLedEnabled(xPos+x, yPos+y, z, weather[x][y].ring[z].enabled);
        MATRIX_SetRingColor(xPos+x, yPos+y, z, weather[x][y].ring[z].r, weather[x][y].ring[z].g, weather[x][y].ring[z].b); /* yellow */
        (void)MATRIX_DrawClockHand(xPos+x, yPos+y, z, weather[x][y].hand[z].angle);
      }
    }
  }
  APP_RequestUpdateLEDs();
  (void)MATRIX_SendToRemoteQueueExecuteAndWait(true);
}
#endif /* PL_CONFIG_USE_LED_RING */

#if PL_CONFIG_USE_LED_RING
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

static void DEMO_LedDemo1(void) {
  uint8_t rowStartStep[MATRIX_NOF_CLOCKS_Y]; /* rainbow color starting values */
  NEO_PixelColor color;
  uint8_t brightness = 2;
  int val = 0;

  /* fill in default row start values */
  for(int i=0; i<MATRIX_NOF_CLOCKS_Y;i++) {
    rowStartStep[i] = val;
    val+=20;
    if(val>=0xff) {
      val = 0;
    }
  }

  /* clear all */
  MATRIX_SetRingColorAll(0, 0, 0);
  MATRIX_SetRingLedEnabledAll(true);
  APP_RequestUpdateLEDs();
  vTaskDelay(pdMS_TO_TICKS(100));

  for(int i=0; i<200; i++) {
    for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
      color = Rainbow(256, rowStartStep[y]);
      color = NEO_BrightnessPercentColor(color, brightness);
      rowStartStep[y]++;
      for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
        for(int z=0; z<MATRIX_NOF_CLOCKS_Z; z++) {
          MATRIX_SetRingColor(x, y, z, NEO_SPLIT_RGB(color));
        }
      }
    }
    APP_RequestUpdateLEDs();
    vTaskDelay(pdMS_TO_TICKS(20));
  }
  MATRIX_SetRingLedEnabledAll(false);
  APP_RequestUpdateLEDs();
}
#endif

#if PL_CONFIG_USE_LED_RING && PL_CONFIG_IS_MASTER
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
  MATRIX_SetRingColor(b->posX, b->posY, 0, b->r, b->g, b->b);
}

static void ClearBall(PongBall_t *b) {
  MATRIX_SetRingColor(b->posX, b->posY, 0, 0, 0, 0);
}

static void DrawPlayer(PongPlayer_t *player) {
  MATRIX_SetRingColor(player->posX, player->posY, 0, player->r, player->g, player->b);
}

static void ClearPlayer(PongPlayer_t *player) {
  MATRIX_SetRingColor(player->posX, player->posY, 0, 0, 0, 0);
}

static void MovePlayer(PongPlayer_t *player, int deltaY) {
  ClearPlayer(player);
  player->posY += deltaY;
  if (player->posY<0) {
    player->posY = 0;
  } else if (player->posY>MATRIX_NOF_CLOCKS_Y-1) {
    player->posY =MATRIX_NOF_CLOCKS_Y-1;
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
  APP_RequestUpdateLEDs();
  vTaskDelay(pdMS_TO_TICKS(100));
  for(int i=0; i<10; i++) {
    ClearBall(ball);
    APP_RequestUpdateLEDs();
    vTaskDelay(pdMS_TO_TICKS(100));
    DrawBall(ball);
    APP_RequestUpdateLEDs();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
  MATRIX_SetRingColorAll(0, 0, 0);
  MATRIX_SetRingLedEnabledAll(false);
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
  APP_RequestUpdateLEDs();
  vTaskDelay(pdMS_TO_TICKS(100));
  player->r >>= 5;
  player->g >>= 5;
  player->b >>= 5;
  DrawPlayer(player);
  APP_RequestUpdateLEDs();
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
  } else if (b->posY >= MATRIX_NOF_CLOCKS_Y) {
    b->posY = MATRIX_NOF_CLOCKS_Y-2;
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

  MATRIX_SetHandColorAll(0x12, 0x12, 0x12);
  MATRIX_SetRingColorAll(0, 0, 0);
  MFONT_PrintString((unsigned char*)"    ", 0, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MFONT_PrintString((unsigned char*)"PONG", 0, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MFONT_PrintString((unsigned char*)"    ", 0, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);

#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_SetHandLedEnabledAll(false);
  MATRIX_Set2ndHandLedEnabledAll(false);
  MATRIX_SetRingLedEnabledAll(true);
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
  playerR.posX = MATRIX_NOF_CLOCKS_X-1;
  playerR.posY = MATRIX_NOF_CLOCKS_Y/2;

  DrawPlayer(&playerL);
  DrawPlayer(&playerR);
  DrawBall(&ball);
  APP_RequestUpdateLEDs();
  vTaskDelay(pdMS_TO_TICKS(1000));

  for(int i=0; i<60; i++) {
    gameOver = MoveBall(&ball, &playerL, &playerR);
    DrawPlayer(&playerL);
    DrawPlayer(&playerR);
    APP_RequestUpdateLEDs();
    if (gameOver) {
      break;
    }
#if MATRIX_NOF_CLOCKS_X>=12 && MATRIX_NOF_CLOCKS_X>=5
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
#elif MATRIX_NOF_CLOCKS_X>=8 && MATRIX_NOF_CLOCKS_Y>=3
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
#endif /* PL_CONFIG_USE_LED_RING */

#if PL_CONFIG_IS_MASTER
static uint8_t DemoSquare(void) {
  uint8_t res;

#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
  (void)MATRIX_DrawAllClockDelays(4, 4);
  (void)MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  /* build initial squares */
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y+=2) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x+=2) {
      (void)MATRIX_DrawClockHands(x,     y, 180,  90);
      (void)MATRIX_DrawClockHands(x,   y+1,  90,   0);
      (void)MATRIX_DrawClockHands(x+1,   y, 270, 180);
      (void)MATRIX_DrawClockHands(x+1, y+1,   0, 270);
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
  (void)MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CW);
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y+=2) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x+=2) {
      (void)MATRIX_DrawClockHands(x,     y, 180+2*360,  90+2*360);
      (void)MATRIX_DrawClockHands(x,   y+1,  90+2*360,   0+2*360);
      (void)MATRIX_DrawClockHands(x+1,   y, 270+2*360, 180+2*360);
      (void)MATRIX_DrawClockHands(x+1, y+1,   0+2*360, 270+2*360);
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
  (void)MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CCW, STEPPER_MOVE_MODE_CW);
  //(void)MATRIX_DrawAllIsRelative(true, true);
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y+=2) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x+=2) {
      (void)MATRIX_DrawClockHands(x,     y, 180-2*360,  90+2*360);
      (void)MATRIX_DrawClockHands(x,   y+1,  90-2*360,   0+2*360);
      (void)MATRIX_DrawClockHands(x+1,   y, 270-2*360, 180+2*360);
      (void)MATRIX_DrawClockHands(x+1, y+1,   0-2*360, 270+2*360);
    }
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  //(void)MATRIX_DrawAllIsRelative(false, false);
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  return res;
}
#endif

#if PL_CONFIG_IS_MASTER
static uint8_t DemoPropeller(void) {
  uint8_t res;

#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
  (void)MATRIX_DrawAllClockDelays(4, 4);
  (void)MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      (void)MATRIX_DrawClockHands(x, y, 0,  180);
    }
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  (void)MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CW);
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      (void)MATRIX_DrawClockHands(x, y, 0+2*360, 180+2*360);
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

#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
  (void)MATRIX_DrawAllClockDelays(4, 4);
  (void)MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      (void)MATRIX_DrawClockHands(x, y, 0,  0);
    }
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y;y++) {
    (void)MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CCW, STEPPER_MOVE_MODE_CW);
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      (void)MATRIX_DrawClockHands(x, y, 180, 180);
    }
    res = MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
    if (res!=ERR_OK) {
      return DEMO_FailedDemo(res);
    }
  }
  return ERR_OK;
}
#endif


#if PL_CONFIG_IS_MASTER || PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN128
static uint8_t DemoRandomHandsPos(void) {
#if PL_CONFIG_IS_MASTER
  uint8_t res;

  (void)MATRIX_DrawAllClockDelays(2, 2);
  (void)MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      (void)MATRIX_DrawClockHands(x, y, McuUtility_random(0, 359),  McuUtility_random(0, 359));
    }
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  return ERR_OK;
#else
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      for(int z=0; z<MATRIX_NOF_CLOCKS_Z; z++) {
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
static uint8_t DEMO_Demo0(const McuShell_StdIOType *io) {
  uint8_t res;

#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
  (void)MATRIX_DrawAllClockDelays(2, 2);
  /* set move mode: */
  (void)MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      MATRIX_DrawClockDelays(x, y, 2+y, 2+y);
    }
  }
  /* set movement: */
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      (void)MATRIX_DrawClockHands(x, y, 90, 270);
    }
  }
  res = MATRIX_SendToRemoteQueue(); /* queue commands */
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }

  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      (void)MATRIX_DrawClockHands(x, y, 0, 180);
    }
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  /* move to park position */
  res = MATRIX_MoveAllto12(20000, io);
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  return res;
}

static uint8_t DEMO_Demo1(const McuShell_StdIOType *io) {
  int angle0, angle1;
  uint8_t res;

#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
  (void)MATRIX_DrawAllClockDelays(2, 2);
  (void)MATRIX_DrawAllClockHands(0, 180);
  res = MATRIX_SendToRemoteQueue(); /* queue command */
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  /* configure delays */
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      (void)MATRIX_DrawClockDelays(x, y, 5+x, 5+x);
    }
  }

  /* rotate them to the next quadrant */
  angle0 = angle1 = 0;
  for(int i=0; i<3; i++) {
    angle0 = (angle0+90)%360;
    angle1 = (angle1+90)%360;
    (void)MATRIX_DrawAllClockHands(angle0, angle1);
    res = MATRIX_SendToRemoteQueue(); /* queue command */
    if (res!=ERR_OK) {
      return DEMO_FailedDemo(res);
    }
  }
  /* execute */
  res = MATRIX_ExecuteRemoteQueueAndWait(true);
  return res;
}

static uint8_t DEMO_Demo2(const McuShell_StdIOType *io) {
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
  MATRIX_DrawAllClockDelays(1, 1);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  for(int i=2;i<12;i++) {
    MATRIX_ShowTime(21, i, true, true);
  }
  return MATRIX_MoveAllto12(10000, io);
}

static uint8_t DEMO_Demo3(const McuShell_StdIOType *io) {
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
  MATRIX_DrawAllClockDelays(2, 2);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  MATRIX_DrawAllClockHands(270, 180);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  return MATRIX_MoveAllto12(10000, io);
}

#if MATRIX_NOF_CLOCKS_X>=12 && MATRIX_NOF_CLOCKS_Y>=5
static void DEMO_Nxp(void) {
  MATRIX_DrawAllClockDelays(3, 3);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);

#if PL_CONFIG_USE_NEO_PIXEL
  MATRIX_SetHandLedEnabledAll(true);
#endif
  DemoRandomHandsPos();
  MATRIX_Delay(2000);
  MFONT_PrintString((unsigned char*)"RUNS", 0, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MATRIX_Delay(1000);
#if PL_CONFIG_USE_NEO_PIXEL
  MATRIX_SetHandLedEnabledAll(true);
#endif
  DemoRandomHandsPos();
  MFONT_PrintString((unsigned char*)"WITH", 0, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MATRIX_Delay(1000);
#if PL_CONFIG_USE_NEO_PIXEL
  MATRIX_SetHandLedEnabledAll(true);
#endif
  DemoRandomHandsPos();
  MFONT_PrintString((unsigned char*)"    ", 0, 0, MFONT_SIZE_3x5);
  MFONT_PrintString((unsigned char*)"N", 1, 0, MFONT_SIZE_3x5);
  MFONT_PrintString((unsigned char*)"X", 4, 0, MFONT_SIZE_3x5);
  MFONT_PrintString((unsigned char*)"P", 7, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MATRIX_Delay(1000);
#if PL_CONFIG_USE_NEO_PIXEL
  MATRIX_SetHandLedEnabledAll(true);
#endif
  DemoRandomHandsPos();
  MFONT_PrintString((unsigned char*)"    ", 0, 0, MFONT_SIZE_3x5);
  MFONT_PrintString((unsigned char*)"L", 1, 0, MFONT_SIZE_3x5);
  MFONT_PrintString((unsigned char*)"P", 4, 0, MFONT_SIZE_3x5);
  MFONT_PrintString((unsigned char*)"C", 7, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MATRIX_Delay(1000);
#if PL_CONFIG_USE_NEO_PIXEL
  MATRIX_SetHandLedEnabledAll(true);
#endif
  DemoRandomHandsPos();
  MFONT_PrintString((unsigned char*)"    ", 0, 0, MFONT_SIZE_3x5);
  MFONT_PrintString((unsigned char*)"8", 1, 0, MFONT_SIZE_3x5);
  MFONT_PrintString((unsigned char*)"4", 4, 0, MFONT_SIZE_3x5);
  MFONT_PrintString((unsigned char*)"5", 7, 0, MFONT_SIZE_3x5);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  MATRIX_Delay(1000);
#if PL_CONFIG_USE_NEO_PIXEL
  MATRIX_SetHandLedEnabledAll(true);
#endif
  DemoRandomHandsPos();
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
}
#endif /* PL_MATRIX_CONFIG_IS_12x5 */

static uint8_t DEMO_Demo4(const McuShell_StdIOType *io) {
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
  MATRIX_DrawAllClockDelays(0, 0);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CW);
  MATRIX_DrawAllClockHands(180, 0);
  MATRIX_SendToRemoteQueue();
  MATRIX_DrawAllClockHands(0, 0);
  MATRIX_SendToRemoteQueueExecuteAndWait(true);
  return MATRIX_MoveAllto12(10000, io);
}

static uint8_t DEMO_DemoCombined(const McuShell_StdIOType *io) {
  uint8_t res = ERR_OK;
  TIMEREC time;

  MATRIX_DrawAllClockDelays(2, 2);
  McuTimeDate_GetTime(&time);
#if MATRIX_NOF_CLOCKS_X>=12 && MATRIX_NOF_CLOCKS_Y>=5
  (void)MATRIX_ShowTimeLarge(time.Hour, time.Min, true);
#else
  (void)MATRIX_ShowTime(time.Hour, time.Min, false, true);
#endif

  (void)DemoRandomHandsPos();
  MATRIX_Delay(2000);
  (void)DemoRandomHandsPos();
  MATRIX_Delay(2000);
  (void)DemoRandomHandsPos();
  MATRIX_Delay(2000);

  MATRIX_DrawAllClockDelays(2, 2);
  res = MATRIX_ShowTime(20, 34, false, true);
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
  MATRIX_Delay(3000);
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif

  MATRIX_DrawAllClockDelays(2, 2);
  McuTimeDate_GetTime(&time);
  (void)MATRIX_ShowTime(time.Hour, time.Min, false, true);
  MATRIX_Delay(3000);

  MATRIX_DrawAllClockDelays(2, 2);
  res = MATRIX_ShowTemperature(22, true);
  MATRIX_Delay(3000);
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }
#if PL_CONFIG_USE_LED_RING
  MATRIX_SetHandLedEnabledAll(true);
#endif

  res = DEMO_Demo1(io);
  if (res!=ERR_OK) {
    return DEMO_FailedDemo(res);
  }

  MATRIX_DrawAllClockDelays(2, 2);
  McuTimeDate_GetTime(&time);
  (void)MATRIX_ShowTime(time.Hour, time.Min, true, true);
  MATRIX_Delay(3000);

#if PL_CONFIG_USE_LED_RING
  MATRIX_SetHandLedEnabledAll(true);
#endif
  MATRIX_Delay(3000);
  return MATRIX_MoveAllto12(10000, io);
}
#endif /* PL_CONFIG_IS_MASTER */

#if PL_CONFIG_USE_LED_RING || PL_MATRIX_CONFIG_IS_RGB
static uint8_t DemoRandomHandsColor(void) {
#if PL_CONFIG_USE_LED_RING
  uint32_t color;

  MATRIX_SetHandLedEnabledAll(true);
  for (int i=0; i<10; i++) {
    for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
      for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
        for(int z=0; z<MATRIX_NOF_CLOCKS_Z; z++) {
          (void)MATRIX_SetHandColor(x, y, z, McuUtility_random(1, 255),  McuUtility_random(1, 255), McuUtility_random(1, 255));
        }
      }
    }
    APP_RequestUpdateLEDs();
    vTaskDelay(pdMS_TO_TICKS(500));
  }
  color = MATRIX_GetHandColorAdjusted();
  MATRIX_SetHandColorAll(NEO_SPLIT_RGB(color));
  return ERR_OK;
#elif PL_MATRIX_CONFIG_IS_RGB
  uint8_t res;

  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      for(int z=0; z<MATRIX_NOF_CLOCKS_Z; z++) {
        uint8_t r, g, b;

        r = McuUtility_random(1, 255);
        g = McuUtility_random(1, 255);
        b = McuUtility_random(1, 255);
        MATRIX_DrawHandColor(x, y, z, NEO_COMBINE_RGB(r,g,b));
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
#endif /* PL_CONFIG_USE_LED_RING */
}
#endif

#if PL_CONFIG_USE_LED_RING || PL_MATRIX_CONFIG_IS_RGB
static uint8_t DemoRandomRingColor(void) {
#if PL_CONFIG_USE_LED_RING
  MATRIX_SetHandLedEnabledAll(false);
  MATRIX_SetRingLedEnabledAll(true);
  for(int i=0; i<10; i++) {
    for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
      for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
        for(int z=0; z<MATRIX_NOF_CLOCKS_Z; z++) {
          (void)MATRIX_SetRingColor(x, y, z, McuUtility_random(0, 0x5),  McuUtility_random(0, 0x5), McuUtility_random(0, 0x5)); /* limit brightness */
        }
      }
    }
    APP_RequestUpdateLEDs();
    vTaskDelay(pdMS_TO_TICKS(500));
  } /* for */
  MATRIX_SetRingLedEnabledAll(false);
  MATRIX_SetHandLedEnabledAll(true);
  return ERR_OK;
#elif PL_MATRIX_CONFIG_IS_RGB
  uint8_t res;

  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      for(int z=0; z<MATRIX_NOF_CLOCKS_Z; z++) {
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
#endif /* PL_CONFIG_USE_LED_RING */
}
#endif

#if PL_CONFIG_IS_MASTER || PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN128
static uint8_t DemoClap(void) {
#if PL_CONFIG_USE_DUAL_HANDS
  MATRIX_Set2ndHandLedEnabledAll(false);
#endif
#if PL_CONFIG_IS_MASTER
  (void)MATRIX_DrawAllClockDelays(4, 4);
  (void)MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      (void)MATRIX_DrawClockHands(x, y, 0,  180);
    }
  }
  (void)MATRIX_SendToRemoteQueueExecuteAndWait(true); /* queue commands */
  for (int i=0; i<2; i++) {
    (void)MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CCW);
    for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
      for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
        (void)MATRIX_DrawClockHands(x, y, 90, 90);
      }
    }
    (void)MATRIX_SendToRemoteQueue();
    (void)MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CCW, STEPPER_MOVE_MODE_CW);
    for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
      for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
        (void)MATRIX_DrawClockHands(x, y, 0, 180);
      }
    }
    (void)MATRIX_SendToRemoteQueue();
  }
  return MATRIX_ExecuteRemoteQueueAndWait(true);
#else
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      STEPPER_MoveClockDegreeAbs(MATRIX_GetStepper(x, y, 0), 0, STEPPER_MOVE_MODE_SHORT, 4, true, true);
      STEPPER_MoveClockDegreeAbs(MATRIX_GetStepper(x, y, 1), 180, STEPPER_MOVE_MODE_SHORT, 4, true, true);
    }
  }
  STEPPER_StartTimer();
  while(!STEPBOARD_IsIdle(STEPBOARD_GetBoard())) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      STEPPER_MoveClockDegreeAbs(MATRIX_GetStepper(x, y, 0), 90, STEPPER_MOVE_MODE_CW, 4, true, true);
      STEPPER_MoveClockDegreeAbs(MATRIX_GetStepper(x, y, 1), 90, STEPPER_MOVE_MODE_CCW, 4, true, true);
    }
  }
  STEPPER_StartTimer();
  while(!STEPBOARD_IsIdle(STEPBOARD_GetBoard())) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
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

#if PL_CONFIG_USE_SHELL
static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"demo", (unsigned char*)"Group of demo commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
  #if PL_CONFIG_IS_MASTER || PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN128
  McuShell_SendHelpStr((unsigned char*)"  hands random pos", (unsigned char*)"Randomize hand position\r\n", io->stdOut);
#if PL_CONFIG_USE_LED_RING || PL_MATRIX_CONFIG_IS_RGB
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
  McuShell_SendHelpStr((unsigned char*)"  0", (unsigned char*)"Demo with propeller\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  1", (unsigned char*)"Demo with changing angles\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  2", (unsigned char*)"Demo fast clock\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  3", (unsigned char*)"Demo moving hand around\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  4", (unsigned char*)"Demo with LED and hand test\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  delay <delay>", (unsigned char*)"Set default delay\r\n", io->stdOut);
  #if MATRIX_NOF_CLOCKS_X >= MFONT_SIZE_X_3x5 && MATRIX_NOF_CLOCKS_Y >= MFONT_SIZE_Y_3x5
  McuShell_SendHelpStr((unsigned char*)"  time <time>", (unsigned char*)"Show time\r\n", io->stdOut);
#endif
  #if MATRIX_NOF_CLOCKS_X >= MFONT_SIZE_X_3x5 && MATRIX_NOF_CLOCKS_Y >= MFONT_SIZE_Y_3x5
  McuShell_SendHelpStr((unsigned char*)"  time large <time>", (unsigned char*)"Show large time\r\n", io->stdOut);
  #endif
  McuShell_SendHelpStr((unsigned char*)"  temperature <tt>", (unsigned char*)"Show temperature\r\n", io->stdOut);
  #if MATRIX_NOF_CLOCKS_X >= MFONT_SIZE_X_3x5 && MATRIX_NOF_CLOCKS_Y >= MFONT_SIZE_Y_3x5
  McuShell_SendHelpStr((unsigned char*)"  temperature large <tt>", (unsigned char*)"Show large temperature\r\n", io->stdOut);
  #endif
  McuShell_SendHelpStr((unsigned char*)"  humidity <hh>", (unsigned char*)"Show humidity\r\n", io->stdOut);
  #if MATRIX_NOF_CLOCKS_X >= MFONT_SIZE_X_3x5 && MATRIX_NOF_CLOCKS_Y >= MFONT_SIZE_Y_3x5
  McuShell_SendHelpStr((unsigned char*)"  humidity large <hh>", (unsigned char*)"Show large humidity\r\n", io->stdOut);
  #endif
  McuShell_SendHelpStr((unsigned char*)"  text <xy> <text>", (unsigned char*)"Write text at position\r\n", io->stdOut);
  #if MATRIX_NOF_CLOCKS_X >= MFONT_SIZE_X_3x5 && MATRIX_NOF_CLOCKS_Y >= MFONT_SIZE_Y_3x5
  McuShell_SendHelpStr((unsigned char*)"  text large <xy> <text>", (unsigned char*)"Write large text at position\r\n", io->stdOut);
  #endif
  #if MATRIX_NOF_CLOCKS_X >= MFONT_SIZE_X_3x5 && MATRIX_NOF_CLOCKS_Y >= MFONT_SIZE_Y_3x5
  McuShell_SendHelpStr((unsigned char*)"  nxp", (unsigned char*)"NXP demo\r\n", io->stdOut);
  #endif
#endif
#if PL_CONFIG_USE_LED_RING
  McuShell_SendHelpStr((unsigned char*)"  led 0", (unsigned char*)"LED color demo\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  led 1", (unsigned char*)"LED rainbow demo\r\n", io->stdOut);
#endif
#if PL_CONFIG_USE_LED_RING && PL_CONFIG_IS_MASTER
  McuShell_SendHelpStr((unsigned char*)"  pong", (unsigned char*)"pong demo\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  weather <weather>", (unsigned char*)"Show weather: sunny, cloudy, rainy, icy\r\n", io->stdOut);
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
#if PL_CONFIG_IS_MASTER
  const unsigned char *p;
  uint8_t xPos, yPos;
#endif

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
  } else if (McuUtility_strcmp((char*)cmd, "demo 0")==0) {
    *handled = true;
    return DEMO_Demo0(io);
  } else if (McuUtility_strcmp((char*)cmd, "demo 1")==0) {
    *handled = true;
    return DEMO_Demo1(io);
  } else if (McuUtility_strcmp((char*)cmd, "demo 2")==0) {
    *handled = true;
    return DEMO_Demo2(io);
  } else if (McuUtility_strcmp((char*)cmd, "demo 3")==0) {
    *handled = true;
    return DEMO_Demo3(io);
  } else if (McuUtility_strcmp((char*)cmd, "demo 4")==0) {
    *handled = true;
    return DEMO_Demo4(io);
#if MATRIX_NOF_CLOCKS_Y >= 5
  } else if (McuUtility_strncmp((char*)cmd, "demo time large ", sizeof("demo time large ")-1)==0) {
    uint8_t hour, minute, second, hsec;

    *handled = TRUE;
    p = cmd + sizeof("demo time large ")-1;
    if (McuUtility_ScanTime(&p, &hour, &minute, &second, &hsec)==ERR_OK) {
      return MATRIX_ShowTimeLarge(hour, minute, true);
    } else {
      return ERR_FAILED;
    }
  #endif
  } else if (McuUtility_strncmp((char*)cmd, "demo time ", sizeof("demo time ")-1)==0) {
    uint8_t hour, minute, second, hsec;

    *handled = TRUE;
    p = cmd + sizeof("demo time ")-1;
    if (McuUtility_ScanTime(&p, &hour, &minute, &second, &hsec)==ERR_OK) {
      return MATRIX_ShowTime(hour, minute, true, true);
    } else {
      return ERR_FAILED;
    }
  #if MATRIX_NOF_CLOCKS_Y >= 5
  } else if (McuUtility_strncmp((char*)cmd, "demo temperature large ", sizeof("demo temperature large ")-1)==0) {
    uint8_t temperature;

    *handled = TRUE;
    p = cmd + sizeof("demo temperature large ")-1;
    if (McuUtility_ScanDecimal8uNumber(&p, &temperature)==ERR_OK) {
      return MATRIX_ShowTemperatureLarge(temperature, true);
    } else {
      return ERR_FAILED;
    }
  #endif
  } else if (McuUtility_strncmp((char*)cmd, "demo temperature ", sizeof("demo temperature ")-1)==0) {
    uint8_t temperature;

    *handled = TRUE;
    p = cmd + sizeof("demo temperature ")-1;
    if (McuUtility_ScanDecimal8uNumber(&p, &temperature)==ERR_OK) {
      return MATRIX_ShowTemperature(temperature, true);
    } else {
      return ERR_FAILED;
    }
  #if MATRIX_NOF_CLOCKS_Y >= 5
  } else if (McuUtility_strncmp((char*)cmd, "demo humidity large ", sizeof("demo humidity large ")-1)==0) {
    uint8_t humidity;

    *handled = TRUE;
    p = cmd + sizeof("demo humidity large ")-1;
    if (McuUtility_ScanDecimal8uNumber(&p, &humidity)==ERR_OK) {
      return MATRIX_ShowHumidityLarge(humidity, true);
    } else {
      return ERR_FAILED;
    }
  #endif
  } else if (McuUtility_strncmp((char*)cmd, "demo humidity ", sizeof("demo humidity ")-1)==0) {
    uint8_t humidity;

    *handled = TRUE;
    p = cmd + sizeof("demo humidity ")-1;
    if (McuUtility_ScanDecimal8uNumber(&p, &humidity)==ERR_OK) {
      return MATRIX_ShowHumidity(humidity, true);
    } else {
      return ERR_FAILED;
    }
  #if MATRIX_NOF_CLOCKS_Y >= 5
  } else if (McuUtility_strncmp((char*)cmd, "demo lux large ", sizeof("demo lux large ")-1)==0) {
    uint16_t lux;

    *handled = TRUE;
    p = cmd + sizeof("demo lux large ")-1;
    if (McuUtility_ScanDecimal16uNumber(&p, &lux)==ERR_OK) {
      return MATRIX_ShowLuxLarge(lux, true);
    } else {
      return ERR_FAILED;
    }
  #endif
  } else if (McuUtility_strncmp((char*)cmd, "demo lux ", sizeof("demo lux ")-1)==0) {
    uint16_t lux;

    *handled = TRUE;
    p = cmd + sizeof("demo lux ")-1;
    if (McuUtility_ScanDecimal16uNumber(&p, &lux)==ERR_OK) {
      return MATRIX_ShowLux(lux, true);
    } else {
      return ERR_FAILED;
    }
  #if MATRIX_NOF_CLOCKS_X >= MFONT_SIZE_X_3x5 && MATRIX_NOF_CLOCKS_Y >= MFONT_SIZE_Y_3x5
  } else if (McuUtility_strncmp((char*)cmd, "demo text large ", sizeof("demo text large ")-1)==0) {
    *handled = TRUE;
    p = cmd + sizeof("demo text large ")-1;
    if (   McuUtility_ScanDecimal8uNumber(&p, &xPos)==ERR_OK && xPos<MATRIX_NOF_CLOCKS_X
        && McuUtility_ScanDecimal8uNumber(&p, &yPos)==ERR_OK && yPos<MATRIX_NOF_CLOCKS_Y
        )
    {
      uint8_t buf[8];

      McuUtility_SkipSpaces(&p);
      if (McuUtility_ReadEscapedName(p, buf, sizeof(buf), NULL, NULL, NULL)!=ERR_OK) {
        return ERR_FAILED;
      }
      MFONT_PrintString(buf, xPos, yPos, MFONT_SIZE_3x5);
      return MATRIX_SendToRemoteQueueExecuteAndWait(true);
    } else {
      return ERR_FAILED;
    }
  } else if (McuUtility_strcmp((char*)cmd, "demo nxp")==0) {
    *handled = TRUE;
    DEMO_Nxp();
    return ERR_OK;
  #endif
  } else if (McuUtility_strncmp((char*)cmd, "demo text ", sizeof("demo text ")-1)==0) {
  *handled = TRUE;
  p = cmd + sizeof("demo text ")-1;
  if (   McuUtility_ScanDecimal8uNumber(&p, &xPos)==ERR_OK && xPos<MATRIX_NOF_CLOCKS_X
      && McuUtility_ScanDecimal8uNumber(&p, &yPos)==ERR_OK && yPos<MATRIX_NOF_CLOCKS_Y
      )
  {
    uint8_t buf[8];

    McuUtility_SkipSpaces(&p);
    if (McuUtility_ReadEscapedName(p, buf, sizeof(buf), NULL, NULL, NULL)!=ERR_OK) {
      return ERR_FAILED;
    }
    MFONT_PrintString(buf, xPos, yPos, MFONT_SIZE_2x3);
    return MATRIX_SendToRemoteQueueExecuteAndWait(true);
  } else {
    return ERR_FAILED;
  }
#endif /* PL_CONFIG_IS_MASTER */
#if PL_CONFIG_USE_LED_RING && PL_CONFIG_IS_MASTER
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
#endif
#if PL_CONFIG_USE_LED_RING
  } else if (McuUtility_strcmp((char*)cmd, "demo led 0")==0) {
    *handled = TRUE;
    DEMO_LedDemo0();
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, "demo led 1")==0) {
    *handled = TRUE;
    DEMO_LedDemo1();
    return ERR_OK;
#endif /* PL_CONFIG_USE_LED_RING */
#if PL_CONFIG_USE_LED_RING || PL_MATRIX_CONFIG_IS_RGB
  } else if (McuUtility_strcmp((char*)cmd, "demo ring random color")==0) {
    *handled = TRUE;
    #if PL_CONFIG_USE_CLOCK
    if (CheckIfClockIsOn(io)!=ERR_OK) {
      return ERR_FAILED;
    }
    #endif
    return DemoRandomRingColor();
#endif /* PL_CONFIG_USE_LED_RING */
#if PL_CONFIG_USE_LED_RING || PL_MATRIX_CONFIG_IS_RGB
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
