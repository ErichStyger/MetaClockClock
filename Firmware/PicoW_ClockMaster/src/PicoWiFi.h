/*
 * Copyright (c) 2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PICO_WIFI_H_
#define PICO_WIFI_H_

#include <stdbool.h>

/*!
 * \brief Sets the initialized status of the CYW43 architecture
 * \param isInitialized If initialized or not
 */
void PicoWiFi_SetArchIsInitialized(bool isInitialized);

/*!
 * \brief Decides if the CY43 architecture and subsystem is initialized or not.
 * \return true if subsystem is initialized, false otherwise.
 */
bool PicoWiFi_GetArchIsInitialized(void);

/*!
 * \brief Returns the WiFi status as string
 * \param linkStatus Linkstatus ID
 * \return String for the link status
 */
const unsigned char *PicoWiFi_GetTcpIpLinkStatusString(int linkStatus);

/*!
 * \brief Module de-initialization
 */
void PicoWiFi_Deinit(void);

/*!
 * \brief Module initialization
 */
void PicoWiFi_Init(void);

#endif /* PICO_WIFI_H_ */
