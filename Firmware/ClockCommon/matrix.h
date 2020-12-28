/*
 * Copyright (c) 2019, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MATRIX_H_
#define MATRIX_H_

#include "platform.h"
#include "matrixconfig.h"
#include <stdint.h>
#include <stdbool.h>
#include "McuShell.h"
#include "stepper.h"

typedef struct MATRIX_Matrix_t {
  int16_t angleMap[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z]; /* absolute angle move */
#if PL_CONFIG_USE_RELATIVE_MOVES
  int16_t relAngleMap[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z]; /* map of relative move (0 for no move) */
#endif
  int8_t delayMap[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z]; /* map of motors with speed delay */
  STEPPER_MoveMode_e moveMap[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z];
#if PL_MATRIX_CONFIG_IS_RGB
  bool enabledRingMap[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z]; /* if ring is enabled */
  bool enabledHandMap[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z]; /* if hand is enabled */
#if PL_CONFIG_USE_DUAL_HANDS
  bool enabled2ndHandMap[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z]; /* if hand is enabled */
  int32_t color2ndHandMap[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z]; /* color for each 2nd hand */
#endif
  int32_t colorHandMap[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z]; /* color for each hand */
  int32_t colorRingMap[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z]; /* color for each ring */
#endif /* PL_MATRIX_CONFIG_IS_RGB */
} MATRIX_Matrix_t;

extern MATRIX_Matrix_t matrix; /* map of current matrix */

#if PL_CONFIG_USE_NEO_PIXEL_HW
  #include "NeoPixel.h"

  void MATRIX_IlluminateHands(void);

  void MATRIX_GetHandColorBrightness(uint32_t *pColor, uint8_t *pBrightness);
  NEO_PixelColor MATRIX_GetHandColorAdjusted(void);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  void MATRIX_DrawRingColor(uint8_t x, uint8_t y, uint8_t z, uint32_t color);
  void MATRIX_DrawAllRingColor(uint32_t color);
#endif

#if PL_CONFIG_USE_LED_DIMMING
  void MATRIX_SetHandBrightness(int32_t x, int32_t y, int32_t z, uint8_t brightness);
  void MATRIX_StartHandDimming(int32_t x, int32_t y, int32_t z, uint8_t targetBrightness);

  void MATRIX_SetHandBrightnessAll(uint8_t brightness);
  void MATRIX_SetHandBrightness(int32_t x, int32_t y, int32_t z, uint8_t brightness);
#endif

#if PL_CONFIG_USE_LED_RING
#include "NeoStepperRing.h"
NEOSR_Handle_t MATRIX_GetLedRingDevice(int32_t x, int32_t y, uint8_t z);
#endif

void MATRIX_DrawHLine(int x, int y, int w);
void MATRIX_DrawVLine(int x, int y, int h);
void MATRIX_DrawRectangle(int x, int y, int w, int h);

uint8_t MATRIX_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

void MATRIX_Delay(int32_t ms);

uint8_t MATRIX_GetAddress(int32_t x, int32_t y, int32_t z);

uint8_t MATRIX_MoveAllto12(int32_t timeoutMs, const McuShell_StdIOType *io);

uint8_t MATRIX_DrawClockDelays(uint8_t x, uint8_t y, uint8_t delay0, uint8_t delay1);
uint8_t MATRIX_DrawAllClockDelays(uint8_t delay0, uint8_t delay1);

uint8_t MATRIX_DrawIsRelative(uint8_t x, uint8_t y, bool isRel0, bool isRel1);
uint8_t MATRIX_DrawAllIsRelative(bool isRel0, bool isRel1);

uint8_t MATRIX_SendToRemoteQueue(void);
uint8_t MATRIX_ExecuteRemoteQueueAndWait(bool wait);
uint8_t MATRIX_SendToRemoteQueueExecuteAndWait(bool wait);

uint8_t MATRIX_ShowTime(uint8_t hour, uint8_t minute, bool hasBorder, bool wait);
uint8_t MATRIX_ShowTimeLarge(uint8_t hour, uint8_t minute, bool wait);

uint8_t MATRIX_ShowTemperature(uint8_t temperature, bool wait);
uint8_t MATRIX_ShowTemperatureLarge(uint8_t temperature, bool wait);

uint8_t MATRIX_ShowHumidity(uint8_t humidity, bool wait);
uint8_t MATRIX_ShowHumidityLarge(uint8_t humidity, bool wait);

uint8_t MATRIX_ShowLux(uint16_t humidity, bool wait);
uint8_t MATRIX_ShowLuxLarge(uint16_t humidity, bool wait);

#if PL_CONFIG_USE_STEPPER
STEPPER_Handle_t MATRIX_GetStepper(int32_t x, int32_t y, int32_t z);
#endif

void MATRIX_TimerCallback(void);

void MATRIX_EnableStepper(void *dev);
void MATRIX_DisableStepper(void *dev);

void MATRIX_RequestRgbUpdate(void);

void MATRIX_Init(void);

#endif /* MATRIX_H_ */
