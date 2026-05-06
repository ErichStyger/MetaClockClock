/*
 * Copyright (c) 2021, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "McuShell.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"
#if PL_CONFIG_USE_NEO_PIXEL_HW
/*!
 * \brief Requests a NeoPixel LED update from an interrupt service routine.
 * \return True if the condition or operation succeeds, false otherwise.
 */
  bool APP_RequestUpdateLEDsFromISR(void);
/*!
 * \brief Requests a NeoPixel LED update from task context.
 */
  void APP_RequestUpdateLEDs(void);
#endif

/*!
 * \brief Parses application shell commands.
 * \param cmd Command string to parse or send.
 * \param handled Set to true when the command has been handled.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t APP_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*!
 * \brief Runs the application and starts the scheduler.
 */
void APP_Run(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* APPLICATION_H_ */
