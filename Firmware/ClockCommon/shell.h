/*
 * Copyright (c) 2021-2023, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SHELL_H_
#define SHELL_H_

#include "McuShell.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Sends a string to all configured shell outputs.
 * \param str Null-terminated string.
 */
void SHELL_SendString(const unsigned char *str);

/*!
 * \brief Sends a string to one shell I/O target.
 * \param str Null-terminated string.
 * \param io Shell I/O streams used for command output.
 */
void SHELL_SendStringToIO(const unsigned char *str, McuShell_ConstStdIOType *io);

/*!
 * \brief Sends one character to all configured shell outputs.
 * \param ch Character to send or print.
 */
void SHELL_SendChar(unsigned char ch);

/*!
 * \brief Parses and dispatches one shell command using the supplied I/O target.
 * \param command Command string to parse or send.
 * \param io Shell I/O streams used for command output.
 * \param silent True to suppress command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t SHELL_ParseCommandIO(const unsigned char *command, McuShell_ConstStdIOType *io, bool silent);

/*!
 * \brief Deinitializes shell support.
 */
void SHELL_Deinit(void);

/*!
 * \brief Initializes shell support.
 */
void SHELL_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* SHELL_H_ */
