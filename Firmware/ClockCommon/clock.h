/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLOCK_H_
#define CLOCK_H_

#include <stdbool.h>
#include "McuShell.h"
#include "McuDebounce.h"

typedef enum CLOCK_Mode_e {
  CLOCK_MODE_ON,
  CLOCK_MODE_OFF,
  CLOCK_MODE_TOGGLE,
} CLOCK_Mode_e;

typedef enum CLOCK_Notify_e { /* direct task notification bits */
  CLOCK_NOTIFY_BUTTON_PRESSED_USR       = (1<<0),
  CLOCK_NOTIFY_BUTTON_PRESSED_USR_LONG  = (1<<1),
  CLOCK_NOTIFY_BUTTON_PRESSED_RST       = (1<<2),
  CLOCK_NOTIFY_BUTTON_PRESSED_UP        = (1<<3),
  CLOCK_NOTIFY_BUTTON_PRESSED_DOWN      = (1<<4),
  CLOCK_NOTIFY_BUTTON_PRESSED_LEFT      = (1<<5),
  CLOCK_NOTIFY_BUTTON_PRESSED_RIGHT     = (1<<6),
  CLOCK_NOTIFY_BUTTON_PRESSED_MID       = (1<<7),
  CLOCK_NOTIFY_BUTTON_PRESSED_SET       = (1<<8),
  CLOCK_NOTIFY_UPDATE_CLOCK             = (1<<9), /* update clock because settings have changed */
} CLOCK_Notify_e;

/*!
 * \brief Notifies the clock task about a clock event.
 * \param msg Notification message.
 */
void CLOCK_Notify(CLOCK_Notify_e msg);
/*!
 * \brief Handles debounced button events for clock control.
 * \param event Debounced button event kind.
 * \param buttons Button bit mask associated with the event.
 */
void CLOCK_ButtonHandler(McuDbnc_EventKinds event, uint32_t buttons);

/*!
 * \brief Moves the clock hands into the selected park mode.
 * \param mode Stepper or clock move mode.
 */
void CLOCK_Park(CLOCK_Mode_e mode);
/*!
 * \brief Turns the clock display on using the selected mode.
 * \param mode Stepper or clock move mode.
 */
void CLOCK_On(CLOCK_Mode_e mode);

/*!
 * \brief Parses clock shell commands.
 * \param cmd Command string to parse or send.
 * \param handled Set to true when the command has been handled.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t CLOCK_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*!
 * \brief Returns whether the clock is currently enabled.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool CLOCK_GetClockIsOn(void);

/*!
 * \brief Initializes the clock module and task.
 */
void CLOCK_Init(void);

#endif /* CLOCK_H_ */
