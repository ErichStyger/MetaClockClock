/*
 * Copyright (c) 2023-2024, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MY_LEDS_H_
#define MY_LEDS_H_

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#if PL_CONFIG_USE_LEDS

#include "McuLib.h"
#include "leds_config.h"

#if PL_CONFIG_USE_SHELL
  #include <stdint.h>
  #include <stdbool.h>
  #include "McuShell.h"

/*!
 * \brief Parses LED shell commands.
 * \param cmd Command string to parse or send.
 * \param handled Set to true when the command has been handled.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
  uint8_t Leds_ParseCommand(const uint8_t *cmd, bool *handled, McuShell_ConstStdIOType *io);
#endif

/*!
 * handles for the different LEDs
 */
typedef enum LEDS_Leds_e {
#if LEDS_CONFIG_HAS_RED_LED
  LEDS_RED,
#endif
#if LEDS_CONFIG_HAS_GREEN_LED
  LEDS_GREEN,
#endif
#if LEDS_CONFIG_HAS_BLUE_LED
  LEDS_BLUE,
#endif
#if LEDS_CONFIG_HAS_ONBOARD_LED
  LEDS_ONBOARD,
#endif
  LEDS_NOF_LEDS, /*!< Sentinel, must be last! */
} LEDS_Leds_e;

/*!
 * \brief Turns the selected LED on.
 * \param led LED identifier.
 */
void Leds_On(LEDS_Leds_e led);

/*!
 * \brief Turns the selected LED off.
 * \param led LED identifier.
 */
void Leds_Off(LEDS_Leds_e led);

/*!
 * \brief Toggles the selected LED.
 * \param led LED identifier.
 */
void Leds_Neg(LEDS_Leds_e led);

/*!
 * \brief Gets the current state of the selected LED.
 * \param led LED identifier.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool Leds_Get(LEDS_Leds_e led);

/*!
 * \brief Initializes the LED module.
 */
void Leds_Init(void);

/*!
 * \brief Performs LED initialization that has to run from task context.
 */
void Leds_InitFromTask(void);

/*!
 * \brief Deinitializes the LED module.
 */
void Leds_Deinit(void);

#endif /* PL_CONFIG_USE_LEDS */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* MY_LEDS_H_ */
