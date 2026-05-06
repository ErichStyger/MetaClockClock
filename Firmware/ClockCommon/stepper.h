/*
 * Copyright (c) 2021-2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STEPPER_H_
#define STEPPER_H_

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  STEPPER_MOVE_MODE_SHORT,  /* shortest move to position */
  STEPPER_MOVE_MODE_CW,     /* clock-wise move */
  STEPPER_MOVE_MODE_CCW,    /* counter-clock-wise move */
} STEPPER_MoveMode_e;

#if PL_CONFIG_USE_STEPPER
#include <stdint.h>
#include <stdbool.h>
#include "McuShell.h"
#include "McuRTOS.h"
#include "stepperConfig.h"

typedef void *STEPPER_Handle_t;

typedef struct STEPPER_Config_t {
  void *device; /* point to the motor device */
  void (*stepFn)(void *device, int step); /* function pointer to perform a single step forward (1) or backward (-1) */
} STEPPER_Config_t;

/*!
 * \brief Fills a stepper configuration with default values.
 * \param config Configuration structure to fill or use.
 */
void STEPPER_GetDefaultConfig(STEPPER_Config_t *config);

/*!
 * \brief Initializes a stepper device instance.
 * \param config Configuration structure to fill or use.
 * \return Requested handle.
 */
STEPPER_Handle_t STEPPER_InitDevice(STEPPER_Config_t *config);

/*!
 * \brief Stops the shared stepper timer.
 */
void STEPPER_StopTimer(void);
/*!
 * \brief Starts the shared stepper timer.
 */
void STEPPER_StartTimer(void);
/*!
 * \brief Advances one stepper from the timer callback.
 * \param stepper Stepper device handle.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool STEPPER_TimerStepperCallback(STEPPER_Handle_t stepper);

/*!
 * \brief Parses stepper shell commands.
 * \param cmd Command string to parse or send.
 * \param handled Set to true when the command has been handled.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t STEPPER_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

typedef enum {
  STEPPER_HAND_MM = 0, /* inner, mm */
  STEPPER_HAND_HH = 1  /* outer, hh */
} STEPPER_Hand_e;

typedef enum {
  STEPPER_CLOCK_0=0,
  STEPPER_CLOCK_1=1,
  STEPPER_CLOCK_2=2,
  STEPPER_CLOCK_3=3,
} STEPPER_Clock_e;

/*!
 * \brief Checks whether a stepper has no pending movement.
 * \param stepper Stepper device handle.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool STEPPER_IsIdle(STEPPER_Handle_t stepper);

#if PL_CONFIG_IS_ANALOG_CLOCK
/*!
 * \brief Queues an absolute clock-hand move in degrees.
 * \param stepper Stepper device handle.
 * \param degree Move angle in degrees.
 * \param mode Stepper or clock move mode.
 * \param delay Move delay value.
 * \param speedUp True to use acceleration at the beginning of the move.
 * \param slowDown True to use deceleration at the end of the move.
 */
void STEPPER_MoveClockDegreeAbs(STEPPER_Handle_t stepper, int32_t degree, STEPPER_MoveMode_e mode, uint8_t delay, bool speedUp, bool slowDown);
/*!
 * \brief Queues a relative clock-hand move in degrees.
 * \param stepper Stepper device handle.
 * \param degree Move angle in degrees.
 * \param mode Stepper or clock move mode.
 * \param delay Move delay value.
 * \param speedUp True to use acceleration at the beginning of the move.
 * \param slowDown True to use deceleration at the end of the move.
 */
void STEPPER_MoveClockDegreeRel(STEPPER_Handle_t stepper, int32_t degree, STEPPER_MoveMode_e mode, uint8_t delay, bool speedUp, bool slowDown);
/*!
 * \brief Queues a relative motor move in degrees.
 * \param stepper Stepper device handle.
 * \param degree Move angle in degrees.
 * \param delay Move delay value.
 */
void STEPPER_MoveMotorDegreeRel(STEPPER_Handle_t stepper, int32_t degree, uint16_t delay);
#endif
/*!
 * \brief Queues a relative motor move in steps.
 * \param stepper Stepper device handle.
 * \param steps Number of motor steps.
 * \param delay Move delay value.
 */
void STEPPER_MoveMotorStepsRel(STEPPER_Handle_t stepper, int32_t steps, uint16_t delay);
/*!
 * \brief Queues an absolute motor move in steps.
 * \param stepper Stepper device handle.
 * \param steps Number of motor steps.
 * \param delay Move delay value.
 */
void STEPPER_MoveMotorStepsAbs(STEPPER_Handle_t stepper, int32_t steps, uint16_t delay);

/*!
 * \brief Sets the current logical stepper position.
 * \param stepper Stepper device handle.
 * \param pos Pixel or step position.
 */
void STEPPER_SetPos(STEPPER_Handle_t stepper, int32_t pos);
/*!
 * \brief Gets the current logical stepper position.
 * \param stepper Stepper device handle.
 * \return Requested numeric value.
 */
int32_t STEPPER_GetPos(STEPPER_Handle_t stepper);
/*!
 * \brief Normalizes the stepper position into the configured range.
 * \param stepper Stepper device handle.
 * \return Requested numeric value.
 */
int32_t STEPPER_NormalizePos(STEPPER_Handle_t stepper);

/*!
 * \brief Appends stepper status text to a buffer.
 * \param stepper Stepper device handle.
 * \param buf Data or text buffer.
 * \param bufSize Size of the buffer in bytes.
 */
void STEPPER_StrCatStatus(STEPPER_Handle_t stepper, unsigned char *buf, size_t bufSize);
/*!
 * \brief Gets the movement queue for a stepper.
 * \param stepper Stepper device handle.
 * \return Requested handle.
 */
QueueHandle_t STEPPER_GetQueue(STEPPER_Handle_t stepper);

/*!
 * \brief Gets the hardware device pointer associated with a stepper.
 * \param stepper Stepper device handle.
 * \return Requested handle.
 */
void *STEPPER_GetDevice(STEPPER_Handle_t stepper);

/*!
 * \brief Deinitializes stepper support.
 */
void STEPPER_Deinit(void);

/*!
 * \brief Initializes stepper support.
 */
void STEPPER_Init(void);
#endif /* PL_CONFIG_USE_STEPPER */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* STEPPER_H_ */
