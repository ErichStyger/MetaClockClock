/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APPWIFI_H_
#define APPWIFI_H_

#include "platform.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#if PL_CONFIG_USE_SHELL
  #include "McuShell.h"

  /*!
   * \brief Command line and shell handler
   * \param cmd The command to be parsed
   * \param handled If command has been recognized and handled
   * \param io I/O handler to be used
   * \return error code, otherwise ERR_OK
   */
  uint8_t WiFi_ParseCommand(const unsigned char* cmd, bool *handled, const McuShell_StdIOType *io);
#endif

/*!
 * \brief Decides if connected to a network or not
 * \return true if connected, false if not
 */
bool WiFi_isConnected(void);

/*!
 * \brief Module Initialization
 */
void WiFi_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* APPWIFI_H_ */
