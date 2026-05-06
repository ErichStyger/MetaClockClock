/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERMEZZO_H_
#define INTERMEZZO_H_

#include <stdbool.h>
#include "McuShell.h"
#include "McuRTOS.h"

/*!
 * \brief Initializes intermezzo runtime settings.
 */
void Intermezzo_InitSettings(void);

/*!
 * \brief Parses intermezzo shell commands.
 * \param cmd Command string to parse or send.
 * \param handled Set to true when the command has been handled.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t INTERMEZZO_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*!
 * \brief Runs an intermezzo when the configured timing allows it.
 * \param lastClockUpdateTickCount Tick count of the last clock display update.
 * \param intermezzoShown Set to true if an intermezzo was shown.
 */
void INTERMEZZO_Play(TickType_t lastClockUpdateTickCount, bool *intermezzoShown);

/*!
 * \brief Runs a specific intermezzo by number.
 * \param nr Intermezzo number to run.
 */
void INTERMEZZO_PlaySpecific(uint8_t nr);

/*!
 * \brief Returns whether intermezzos are enabled.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool INTERMEZZO_IsOn(void);

/*!
 * \brief Initializes the intermezzo module.
 */
void INTERMEZZO_Init(void);

#endif /* INTERMEZZO_H_ */
