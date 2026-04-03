/*
 * Copyright (c) 2020-2025, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APPWIFI_H_
#define APPWIFI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"
#include <stdbool.h>

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
 * \brief Module de-initialization
 */
void WiFi_Deinit(void);

/*!
 * \brief Module initialization
 */
void WiFi_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* APPWIFI_H_ */
