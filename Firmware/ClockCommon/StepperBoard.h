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
#if PL_CONFIG_USE_NEO_PIXEL_HW
  #include "NeoStepperRing.h"
#endif

typedef void *STEPBOARD_Handle_t;

typedef struct STEPBOARD_Config_t {
  uint8_t addr;      /* RS-485 address */
  bool enabled;      /* if board is enabled or not */
  STEPPER_Handle_t stepper[PL_CONFIG_NOF_STEPPER_ON_BOARD_X][PL_CONFIG_NOF_STEPPER_ON_BOARD_Y][PL_CONFIG_NOF_STEPPER_ON_BOARD_Z];
#if PL_CONFIG_USE_LED_RING
  NEOSR_Handle_t *ledRing[PL_CONFIG_NOF_STEPPER_ON_BOARD_X][PL_CONFIG_NOF_STEPPER_ON_BOARD_Y][PL_CONFIG_NOF_STEPPER_ON_BOARD_Z]; /* points to the LED ring device */
#endif
} STEPBOARD_Config_t;

/*!
 * \brief Fills a stepper board configuration with default values.
 * \param config Configuration structure to fill or use.
 */
void STEPBOARD_GetDefaultConfig(STEPBOARD_Config_t *config);

/*!
 * \brief Checks whether all steppers on a board are idle.
 * \param board Stepper board handle.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool STEPBOARD_IsIdle(STEPBOARD_Handle_t board);

/*!
 * \brief Checks whether a board has queued stepper commands.
 * \param board Stepper board handle.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool STEPBOARD_ItemsInQueue(STEPBOARD_Handle_t board);
/*!
 * \brief Gets the RS-485 address of a stepper board.
 * \param board Stepper board handle.
 * \return Requested 8-bit value.
 */
uint8_t STEPBOARD_GetAddress(STEPBOARD_Handle_t board);
/*!
 * \brief Checks whether a stepper board is enabled.
 * \param board Stepper board handle.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool STEPBOARD_IsEnabled(STEPBOARD_Handle_t board);
/*!
 * \brief Checks whether all steppers on a board are idle.
 * \param board Stepper board handle.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool STEPBOARD_IsIdle(STEPBOARD_Handle_t board);
/*!
 * \brief Starts queued board movement and waits for the requested time.
 * \param board Stepper board handle.
 * \param waitMs Time to wait in milliseconds.
 */
void STEPBOARD_MoveAndWait(STEPBOARD_Handle_t board, uint32_t waitMs);
/*!
 * \brief Normalizes all stepper positions on a board.
 * \param board Stepper board handle.
 */
void STEPBOARD_NormalizePosition(STEPBOARD_Handle_t board);

/*!
 * \brief Gets a stepper handle from a board coordinate.
 * \param board Stepper board handle.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \return Requested handle.
 */
STEPPER_Handle_t STEPBOARD_GetStepper(STEPBOARD_Handle_t board, int x, int y, int z);

#if PL_CONFIG_USE_LED_RING
/*!
 * \brief Gets the LED ring device for a board coordinate.
 * \param board Stepper board handle.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \return Requested handle.
 */
NEOSR_Handle_t STEPBOARD_GetStepperLedRing(STEPBOARD_Handle_t board, int x, int y, int z);
#endif /* PL_CONFIG_USE_NEO_PIXEL_HW */

/*!
 * \brief Gets the global stepper board handle.
 * \return Requested handle.
 */
STEPBOARD_Handle_t STEPBOARD_GetBoard(void);
/*!
 * \brief Sets the global stepper board handle.
 * \param board Stepper board handle to store globally.
 */
void STEPBOARD_SetBoard(STEPBOARD_Handle_t board);

/*!
 * \brief Deinitializes a stepper board device instance.
 * \param board Stepper board handle.
 * \return Requested handle.
 */
STEPBOARD_Handle_t STEPBOARD_DeinitDevice(STEPBOARD_Handle_t board);
/*!
 * \brief Initializes a stepper board device instance.
 * \param config Configuration structure to fill or use.
 * \return Requested handle.
 */
STEPBOARD_Handle_t STEPBOARD_InitDevice(STEPBOARD_Config_t *config);

#if PL_CONFIG_USE_MOTOR_ON_OFF
/*!
 * \brief Turns a board motor power switch on or off.
 * \param board Stepper board handle.
 * \param on True to enable or turn on, false to disable or turn off.
 */
  void STEPBOARD_MotorSwitchOnOff(STEPBOARD_Handle_t board, bool on);
/*!
 * \brief Assigns the GPIO used as the board motor power switch.
 * \param board Stepper board handle.
 * \param gpioHandle GPIO handle controlling motor power.
 */
  void STEPBOARD_SetMotorSwitch(STEPBOARD_Handle_t board, McuGPIO_Handle_t gpioHandle);
/*!
 * \brief Checks whether the board motor power switch is on.
 * \param board Stepper board handle.
 * \return True if the condition or operation succeeds, false otherwise.
 */
  bool STEPBOARD_IsMotorSwitchOn(STEPBOARD_Handle_t board);
#endif

/*!
 * \brief Deinitializes the stepper board module.
 */
void STEPBOARD_Deinit(void);
/*!
 * \brief Initializes the stepper board module.
 */
void STEPBOARD_Init(void);
#endif /* PL_CONFIG_USE_STEPPER */

#endif /* STEPPERBOARD_H_ */
