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
  bool APP_RequestUpdateLEDsFromISR(void);
  void APP_RequestUpdateLEDs(void);
#endif

/*!
 * \brief Command line shell parser
 * \param cmd Pointer to the command line string
 * \param handled Used to indicate that the command has been handled
 * \param io Input/Output handler
 * \return Error Code, ERR_OK if everyhing was ok.
 */
uint8_t APP_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*! run the application, does not return */
void APP_Run(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* APPLICATION_H_ */
