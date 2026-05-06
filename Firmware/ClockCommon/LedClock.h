/*
 * Copyright (c) 2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LEDCLOCK_H_
#define LEDCLOCK_H_

#include "McuTimeDate.h"
#include "McuShell.h"

/*!
 * \brief Parses LED clock shell commands.
 * \param cmd Command string to parse or send.
 * \param handled Set to true when the command has been handled.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t LedClock_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*!
 * \brief Shows the supplied time and date on the LED clock matrix.
 * \param time Time record to read or display.
 * \param date Date record to read or display.
 */
void LedClock_ShowTimeDate(TIMEREC *time, DATEREC *date);

/*!
 * \brief Checks whether a matrix pixel is currently used by the LED clock.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool LedClock_IsPixelUsed(int32_t x, int32_t y, int32_t z);

/*!
 * \brief Releases all pixels reserved by the LED clock.
 */
void LedClock_ReleasePixelAll(void);

/*!
 * \brief Deinitializes the LED clock module.
 */
void LedClock_Deinit(void);

/*!
 * \brief Initializes the LED clock module.
 */
void LedClock_Init(void);

#endif /* LEDCLOCK_H_ */
