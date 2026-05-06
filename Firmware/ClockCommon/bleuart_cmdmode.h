/*
 * Copyright (c) 2020, Erich Styger
  *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCES_BLEUART_CMDMODE_H_
#define SOURCES_BLEUART_CMDMODE_H_

#include "McuShell.h"

extern McuShell_ConstStdIOType BLEUART_stdio;
extern uint8_t BLEUART_DefaultShellBuffer[McuShell_DEFAULT_SHELL_BUFFER_SIZE]; /* default buffer which can be used by the application */

/*!
 * \brief Parses BLE UART command-mode shell commands.
 * \param cmd Command string to parse or send.
 * \param handled Set to true when the command has been handled.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t BLEUART_CMDMODE_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*!
 * \brief Initializes BLE UART command-mode support and its background task.
 */
void BLEUART_CMDMODE_Init(void);

#endif /* SOURCES_BLEUART_CMDMODE_H_ */
