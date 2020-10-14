/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STEPPERBOARD_H_
#define STEPPERBOARD_H_

#include "platform.h"
#if PL_CONFIG_USE_STEPPER

#include <stdint.h>
#include <stdbool.h>
#include "stepper.h"
#include "McuGPIO.h"
#if PL_CONFIG_USE_NEO_PIXEL
  #include "NeoStepperRing.h"
#endif

typedef void *STEPBOARD_Handle_t;

typedef struct STEPBOARD_Config_t {
  uint8_t addr;      /* RS-485 address */
  bool enabled;      /* if board is enabled or not */
  STEPPER_Handle_t stepper[STEPPER_NOF_CLOCKS][STEPPER_NOF_CLOCK_MOTORS];
#if PL_CONFIG_USE_NEO_PIXEL
  NEOSR_Handle_t *ledRing[STEPPER_NOF_CLOCKS][STEPPER_NOF_CLOCK_MOTORS]; /* points to the LED ring device */
#endif
} STEPBOARD_Config_t;

void STEPBOARD_GetDefaultConfig(STEPBOARD_Config_t *config);

bool STEPBOARD_IsIdle(STEPBOARD_Handle_t board);

bool STEPBOARD_ItemsInQueue(STEPBOARD_Handle_t board);
uint8_t STEPBOARD_GetAddress(STEPBOARD_Handle_t board);
bool STEPBOARD_IsEnabled(STEPBOARD_Handle_t board);
bool STEPBOARD_IsIdle(STEPBOARD_Handle_t board);
void STEPBOARD_MoveAndWait(STEPBOARD_Handle_t board, uint32_t waitMs);
void STEPBOARD_NormalizePosition(STEPBOARD_Handle_t board);

STEPPER_Handle_t STEPBOARD_GetStepper(STEPBOARD_Handle_t board, int clock, int motor);

#if PL_CONFIG_USE_NEO_PIXEL
NEOSR_Handle_t STEPBOARD_GetStepperLedRing(STEPBOARD_Handle_t board, int clock, int motor);
#endif /* PL_CONFIG_USE_NEO_PIXEL */

STEPBOARD_Handle_t STEPBOARD_GetBoard(void);
void STEPBOARD_SetBoard(STEPBOARD_Handle_t);

STEPBOARD_Handle_t STEPBOARD_DeinitDevice(STEPBOARD_Handle_t board);
STEPBOARD_Handle_t STEPBOARD_InitDevice(STEPBOARD_Config_t *config);

#if PL_CONFIG_USE_MOTOR_ON_OFF
  void STEPBOARD_MotorSwitchOnOff(STEPBOARD_Handle_t board, bool on);
  void STEPBOARD_SetMotorSwitch(STEPBOARD_Handle_t board, McuGPIO_Handle_t gpioHandle);
  bool STEPBOARD_IsMotorSwitchOn(STEPBOARD_Handle_t board);
#endif

void STEPBOARD_Deinit(void);
void STEPBOARD_Init(void);
#endif /* PL_CONFIG_USE_STEPPER */

#endif /* STEPPERBOARD_H_ */
