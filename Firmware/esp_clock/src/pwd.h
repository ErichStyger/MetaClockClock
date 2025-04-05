/*
 * Copyright (c) 2021, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _WIFI_PWD_H_
#define _WIFI_PWD_H_

#include "platform.h"

#include "pwd.txt" /* not on git!s */

/* ***************************************************************** */
/* HSLU network with EAP2 authentication */
/* ***************************************************************** */
#ifndef CONFIG_WIFI_EAP_METHOD
  #define CONFIG_WIFI_EAP_METHOD     EAP_PEAP
#endif
#ifndef CONFIG_WIFI_EAP_SSID
  #define CONFIG_WIFI_EAP_SSID       "mySSID"
#endif 

/* ***************************************************************** */
/* home network with SSID and password authentication */
/* ***************************************************************** */

/* the following defines which network is checked first: */
#if 0 /* home */
  #define CONFIG_WIFI_START_WITH       WIFI_PASSWORD_METHOD_PSK
#else /* HSLU */
  #define CONFIG_WIFI_START_WITH       WIFI_PASSWORD_METHOD_WPA2
#endif

#define CONFIG_WIFI_PSK_SSID          "SSIDhome"
#define CONFIG_WIFI_PSK_PASSWORD      "passwordHome"

#endif /* _WIFI_PWD_H_ */
