/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEOSTEPPERRING_H_
#define NEOSTEPPERRING_H_

#include "platform.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "NeoStepperRingConfig.h"

typedef void *NEOSR_Handle_t;

typedef struct NEOSR_Config_t {
  bool ledCw;      /* if LEDs are organized cw (clockwise) or ccw (counter-clockwise) */
  int ledLane;     /* LED lane */
  int ledStartPos; /* LED starting position in lane */
  struct {
    bool enabled; /* if LED is on or off */
    uint8_t red, green, blue; /* colors for hand LED */
  } hand;
  struct {
    bool enabled; /* if LED is on or off */
    uint8_t red, green, blue; /* colors for ring LED */
  } ring;
} NEOSR_Config_t;

void NEOSR_GetDefaultConfig(NEOSR_Config_t *config);

NEOSR_Handle_t NEOSR_InitDevice(NEOSR_Config_t *config);

/* set the LEDs for the hand, including dual-hands */
void NEOSR_Illuminate(NEOSR_Handle_t device, int32_t stepperPos);

void NEOSR_SetHandLedEnabled(NEOSR_Handle_t device, bool on);
bool NEOSR_GetHandLedEnabled(NEOSR_Handle_t device);
void NEOSR_SetHandColor(NEOSR_Handle_t device, uint8_t red, uint8_t green, uint8_t blue);

#if PL_CONFIG_USE_LED_DIMMING
  void NEOSR_SetHandBrightness(NEOSR_Handle_t device, uint8_t brightness);
  void NEOSR_StartHandDimming(NEOSR_Handle_t device, uint8_t targetBrightness);
  bool NEOSR_HandDimmingNotFinished(NEOSR_Handle_t device);
#endif

#if PL_CONFIG_USE_EXTENDED_HANDS
  void NEOSR_Set2ndHandLedEnabled(NEOSR_Handle_t device, bool on);
  bool NEOSR_Get2ndHandLedEnabled(NEOSR_Handle_t device);
  void NEOSR_Set2ndHandColor(NEOSR_Handle_t device, uint8_t red, uint8_t green, uint8_t blue);
#endif

void NEOSR_SetRingLedEnabled(NEOSR_Handle_t device, bool on);
bool NEOSR_GetRingLedEnabled(NEOSR_Handle_t device);
void NEOSR_SetRingPixelColor(NEOSR_Handle_t device, uint8_t pos, uint8_t red, uint8_t green, uint8_t blue);
void NEOSR_SetRingColor(NEOSR_Handle_t device, uint8_t red, uint8_t green, uint8_t blue);

void NEOSR_IlluminatePos(int stepperPos, int ledStartPos, int ledLane, bool cw, int ledRed, int ledGreen, int ledBlue);

void NEOSR_StrCatRingStatus(NEOSR_Handle_t device, unsigned char *buf, size_t bufSize);
void NEOSR_StrCatHandStatus(NEOSR_Handle_t device, unsigned char *buf, size_t bufSize);

void NEOSR_Deinit(void);
void NEOSR_Init(void);

#endif /* NEOSTEPPERRING_H_ */
