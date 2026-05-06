/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DEMOS_H_
#define DEMOS_H_

#include <stdint.h>
#include <stdbool.h>
#include "McuShell.h"

/*!
 * \brief Parses demo shell commands.
 * \param cmd Command string to parse or send.
 * \param handled Set to true when the command has been handled.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t DEMO_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*!
 * \brief Returns whether automatic demos are enabled.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool DEMO_IsOn(void);

/*!
 * \brief Initializes the demo module.
 */
void DEMO_Init(void);

#endif /* DEMOS_H_ */
