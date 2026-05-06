/*
 * pixel.h
 *
 * Copyright (c) 2020: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#ifndef PIXEL_H_
#define PIXEL_H_

#include <stdint.h>
#include <stdbool.h>
#include "McuShell.h"

/*!
 * \brief Sets one pixel stepper position as zero.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 */
void PIXEL_Zero(uint8_t x, uint8_t y, uint8_t z);

/*!
 * \brief Sets all pixel stepper positions as zero.
 */
void PIXEL_ZeroAll(void);

/*!
 * \brief Parses pixel shell commands.
 * \param cmd Command string to parse or send.
 * \param handled Set to true when the command has been handled.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t PIXEL_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*!
 * \brief Initializes pixel support.
 */
void PIXEL_Init(void);

#endif /* PIXEL_H_ */
