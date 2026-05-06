/*
 * Copyright (c) 2019, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MAGNETS_H_
#define MAGNETS_H_

#include "platform.h"
#include <stdbool.h>
#include "McuShell.h"

/*!
 * \brief Parses magnetic sensor shell commands.
 * \param cmd Command string to parse or send.
 * \param handled Set to true when the command has been handled.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t MAG_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

typedef enum {
#if PL_CONFIG_IS_LPC845
  MAG_MAG0,
  MAG_MAG1,
  MAG_MAG2,
  MAG_MAG3,
  MAG_MAG4,
  MAG_MAG5,
  MAG_MAG6,
  MAG_MAG7,
#elif PL_CONFIG_IS_K02
  MAG_MAG0,
  MAG_MAG1,
  MAG_MAG2,
  MAG_MAG3,
#endif
  MAG_NOF_MAG
} MAG_MagSensor_e;

/*!
 * \brief Checks whether the selected magnetic sensor is triggered.
 * \param sensor Magnetic sensor to query.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool MAG_IsTriggered(MAG_MagSensor_e sensor);

/*!
 * \brief Deinitializes the magnetic sensor module.
 */
void MAG_Deinit(void);
/*!
 * \brief Initializes the magnetic sensor module.
 */
void MAG_Init(void);

#endif /* MAGNETS_H_ */
