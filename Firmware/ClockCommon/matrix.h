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

#if PL_CONFIG_USE_NEO_PIXEL_HW
  #include "NeoPixel.h"

  void MATRIX_SetHandLedEnabledAll(bool on);
  void MATRIX_Set2ndHandLedEnabledAll(bool on);
  void MATRIX_SetHandLedEnabled(int32_t x, int32_t y, uint8_t z, bool on);
  void MATRIX_SetHandColorAll(uint8_t red, uint8_t green, uint8_t blue);
  void MATRIX_SetHandColor(int32_t x, int32_t y, int32_t z, uint8_t red, uint8_t green, uint8_t blue);

  void MATRIX_SetRingLedEnabledAll(bool on);
  void MATRIX_SetRingLedEnabled(int32_t x, int32_t y, uint8_t z, bool on);
  void MATRIX_SetRingColorAll(uint8_t red, uint8_t green, uint8_t blue);
  void MATRIX_SetRingColor(int32_t x, int32_t y, int32_t z, uint8_t red, uint8_t green, uint8_t blue);
  void MATRIX_SetRingPixelColor(int32_t x, int32_t y, uint8_t pos, uint8_t red, uint8_t green, uint8_t blue);

  void MATRIX_IlluminateHands(void);

  void MATRIX_GetHandColorBrightness(uint32_t *pColor, uint8_t *pBrightness);
  NEO_PixelColor MATRIX_GetHandColorAdjusted(void);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
  void MATRIX_DrawHandColor(uint8_t x, uint8_t y, uint8_t z, uint32_t color);
  void MATRIX_DrawAllHandColor(uint32_t color);
  void MATRIX_DrawRingColor(uint8_t x, uint8_t y, uint8_t z, uint32_t color);
  void MATRIX_DrawAllRingColor(uint32_t color);
#endif

#if PL_CONFIG_USE_LED_DIMMING
  void MATRIX_SetHandBrightness(int32_t x, int32_t y, int32_t z, uint8_t brightness);
  void MATRIX_StartHandDimming(int32_t x, int32_t y, int32_t z, uint8_t targetBrightness);

  void MATRIX_SetHandBrightnessAll(uint8_t brightness);
  void MATRIX_SetHandBrightness(int32_t x, int32_t y, int32_t z, uint8_t brightness);
#endif

void MATRIX_DrawHLine(int x, int y, int w);
void MATRIX_DrawVLine(int x, int y, int h);
void MATRIX_DrawRectangle(int x, int y, int w, int h);

uint8_t MATRIX_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

void MATRIX_Delay(int32_t ms);

uint8_t MATRIX_GetAddress(int32_t x, int32_t y, int32_t z);

uint8_t MATRIX_MoveAllto12(int32_t timeoutMs, const McuShell_StdIOType *io);
uint8_t MATRIX_DrawMoveMode(uint8_t x, uint8_t y, STEPPER_MoveMode_e mode0, STEPPER_MoveMode_e mode1);
uint8_t MATRIX_DrawAllMoveMode(STEPPER_MoveMode_e mode0, STEPPER_MoveMode_e mode1);

uint8_t MATRIX_DrawClockDelays(uint8_t x, uint8_t y, uint8_t delay0, uint8_t delay1);
uint8_t MATRIX_DrawAllClockDelays(uint8_t delay0, uint8_t delay1);

uint8_t MATRIX_DrawIsRelative(uint8_t x, uint8_t y, bool isRel0, bool isRel1);
uint8_t MATRIX_DrawAllIsRelative(bool isRel0, bool isRel1);

void MATRIX_DrawClockLEDs(uint8_t x, uint8_t y, bool on0, bool on1);
void MATRIX_EnableDisableHandLED(uint8_t x, uint8_t y, uint8_t z, bool enable);
void MATRIX_Set2ndHandLedEnabled(int32_t x, int32_t y, uint8_t z, bool on);

uint8_t MATRIX_DrawAllClockHands(int16_t angle0, int16_t angle1);
uint8_t MATRIX_DrawClockHands(uint8_t x, uint8_t y, int16_t angle0, int16_t angle1);
void MATRIX_DrawClockHand(uint8_t x, uint8_t y, uint8_t z, int16_t angle);

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

void MATRIX_Init(void);

#endif /* MATRIX_H_ */
