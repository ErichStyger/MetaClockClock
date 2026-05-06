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
#include "NeoPixel.h"

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
#if PL_CONFIG_USE_EXTENDED_HANDS
  bool enabled2ndHandMap[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z]; /* if hand is enabled */
  int32_t color2ndHandMap[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z]; /* color for each 2nd hand */
#endif
  int32_t colorHandMap[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z]; /* color for each hand */
  int32_t colorRingMap[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z]; /* color for each ring */
#endif /* PL_MATRIX_CONFIG_IS_RGB */
} MATRIX_Matrix_t;

extern MATRIX_Matrix_t matrix; /* map of current matrix */

#if PL_CONFIG_IS_ANALOG_CLOCK && (PL_CONFIG_USE_NEO_PIXEL_HW || PL_MATRIX_CONFIG_IS_RGB)
/*!
 * \brief Updates the LED hand illumination for the analog clock.
 */
  void MATRIX_IlluminateHands(void);

/*!
 * \brief Gets the configured hand color and brightness.
 * \param pColor Pointer receiving the packed RGB color value.
 * \param pBrightness Pointer receiving the brightness value.
 */
  void MATRIX_GetHandColorBrightness(uint32_t *pColor, uint8_t *pBrightness);
/*!
 * \brief Gets the hand color with brightness applied.
 * \return Computed packed color value.
 */
  NEO_PixelColor MATRIX_GetHandColorAdjusted(void);
#endif
#if PL_MATRIX_CONFIG_IS_RGB
/*!
 * \brief Sets the LED ring color for one matrix position.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \param color Packed RGB color value.
 */
  void MATRIX_DrawRingColor(uint8_t x, uint8_t y, uint8_t z, uint32_t color);
/*!
 * \brief Sets the LED ring color for all matrix positions.
 * \param color Packed RGB color value.
 */
  void MATRIX_DrawAllRingColor(uint32_t color);

/*!
 * \brief Gets the hand color with brightness applied.
 * \return Computed packed color value.
 */
  NEO_PixelColor MATRIX_GetHandColorAdjusted(void);
#endif

#if PL_CONFIG_USE_LED_DIMMING
/*!
 * \brief Sets the hand LED brightness at a matrix coordinate.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \param brightness Brightness value.
 */
  void MATRIX_SetHandBrightness(int32_t x, int32_t y, int32_t z, uint8_t brightness);
/*!
 * \brief Starts hand LED dimming toward the target brightness.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \param targetBrightness Target brightness for dimming.
 */
  void MATRIX_StartHandDimming(int32_t x, int32_t y, int32_t z, uint8_t targetBrightness);

/*!
 * \brief Sets the hand LED brightness for all matrix coordinates.
 * \param brightness Brightness value.
 */
  void MATRIX_SetHandBrightnessAll(uint8_t brightness);
/*!
 * \brief Sets the hand LED brightness at a matrix coordinate.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \param brightness Brightness value.
 */
  void MATRIX_SetHandBrightness(int32_t x, int32_t y, int32_t z, uint8_t brightness);
#endif

#if PL_CONFIG_USE_LED_RING
#include "NeoStepperRing.h"
/*!
 * \brief Gets the LED ring device for a matrix coordinate.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \return LED ring device handle.
 */
NEOSR_Handle_t MATRIX_GetLedRingDevice(int32_t x, int32_t y, uint8_t z);
#endif

/*!
 * \brief Draws a horizontal line into the matrix target map.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param w Motor phase bit array or width value.
 */
void MATRIX_DrawHLine(int x, int y, int w);
/*!
 * \brief Draws a vertical line into the matrix target map.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param h Line or rectangle height.
 */
void MATRIX_DrawVLine(int x, int y, int h);
/*!
 * \brief Draws a rectangle into the matrix target map.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param w Motor phase bit array or width value.
 * \param h Line or rectangle height.
 */
void MATRIX_DrawRectangle(int x, int y, int w, int h);

/*!
 * \brief Parses matrix shell commands.
 * \param cmd Command string to parse or send.
 * \param handled Set to true when the command has been handled.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t MATRIX_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*!
 * \brief Delays matrix processing while feeding the watchdog when enabled.
 * \param ms Reported elapsed time in milliseconds.
 */
void MATRIX_Delay(int32_t ms);

/*!
 * \brief Gets the RS-485 board address for a matrix coordinate.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \return Requested 8-bit value.
 */
uint8_t MATRIX_GetAddress(int32_t x, int32_t y, int32_t z);

/*!
 * \brief Checks whether the board with the given RS-485 address is disabled.
 * \param addr RS-485 address value or destination buffer.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool MATRIX_BoardWithAddressIsDisabled(uint8_t addr);

/*!
 * \brief Moves all clock hands to the 12 o-clock position.
 * \param timeoutMs Timeout in milliseconds.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t MATRIX_MoveAllto12(int32_t timeoutMs, const McuShell_StdIOType *io);

/*!
 * \brief Moves all clock hands to their configured start position.
 * \param timeoutMs Timeout in milliseconds.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t MATRIX_MoveAllToStartPosition(int32_t timeoutMs, const McuShell_StdIOType *io);

/*!
 * \brief Enables or disables all clock hands.
 * \param enable True to enable or turn on, false to disable or turn off.
 */
void MATRIX_EnableDisableHandsAll(bool enable);

#if MATRIX_NOF_STEPPERS_Z==2 /* the special functions below are only available for dual shaft motors */
/*!
 * \brief Sets checked move delays for both hands at one matrix coordinate.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param delay0 Move delay value.
 * \param delay1 Move delay value.
 */
void MATRIX_SetMoveDelayZ0Z1Checked(uint8_t x, uint8_t y, uint8_t delay0, uint8_t delay1);
/*!
 * \brief Sets move delays for both hands on all clocks.
 * \param delay0 Move delay value.
 * \param delay1 Move delay value.
 */
void MATRIX_SetMoveDelayZ0Z1All(uint8_t delay0, uint8_t delay1);
#endif
/*!
 * \brief Sets the same move delay for every hand.
 * \param delay Move delay value.
 */
void MATRIX_SetMoveDelayAll(uint8_t delay);

/*!
 * \brief Gets the default move delay used for matrix movements.
 * \return Requested 8-bit value.
 */
uint8_t MATRIX_GetDefaultDelay(void);
/*!
 * \brief Sets the default move delay used for matrix movements.
 * \param delay Move delay value.
 */
void MATRIX_SetDefaultDelay(uint8_t delay);

/*!
 * \brief Sends queued matrix changes to the remote boards.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t MATRIX_SendToRemoteQueue(void);
/*!
 * \brief Broadcasts queue execution and optionally waits until boards are idle.
 * \param wait True to wait for completion.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t MATRIX_ExecuteRemoteQueueAndWait(bool wait);
/*!
 * \brief Sends queued matrix changes, executes them, and optionally waits for completion.
 * \param wait True to wait for completion.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t MATRIX_SendToRemoteQueueExecuteAndWait(bool wait);
/*!
 * \brief Waits until all addressed boards report idle or the timeout expires.
 * \param timeoutMs Timeout in milliseconds.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t MATRIX_WaitForIdle(int32_t timeoutMs);

#if PL_CONFIG_USE_STEPPER
/*!
 * \brief Gets the stepper handle for a matrix coordinate.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \return Requested handle.
 */
STEPPER_Handle_t MATRIX_GetStepper(int32_t x, int32_t y, int32_t z);
#endif

/*!
 * \brief Services matrix stepper timing from the timer callback.
 */
void MATRIX_TimerCallback(void);

/* optional callback to ignore a given coordinate/motor */
typedef bool(*MATRIX_IgnoreCallbackFct)(int32_t, int32_t, int32_t);

/*!
 * \brief Installs the callback used to skip selected matrix coordinates.
 * \param fct Callback function pointer.
 */
void MATRIX_SetIgnoreCallback(MATRIX_IgnoreCallbackFct fct);

/*!
 * \brief Enables the stepper driver associated with a device pointer.
 * \param dev Underlying device pointer.
 */
void MATRIX_EnableStepper(void *dev);
/*!
 * \brief Disables the stepper driver associated with a device pointer.
 * \param dev Underlying device pointer.
 */
void MATRIX_DisableStepper(void *dev);

/*!
 * \brief Requests an RGB LED update for the matrix.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t MATRIX_RequestRgbUpdate(void);

/*!
 * \brief Initializes the matrix module and its task resources.
 */
void MATRIX_Init(void);

#endif /* MATRIX_H_ */
