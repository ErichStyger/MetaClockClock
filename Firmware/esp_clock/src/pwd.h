/*
 * Copyright (c) 2021, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _WIFI_PWD_H_
#define _WIFI_PWD_H_

#include "platform.h"

#include "pwd.txt" /* not on git! */

/* ***************************************************************** */
/* HSLU network with EAP2 authentication */
/* ***************************************************************** */
#ifndef CONFIG_WIFI_EAP_METHOD
  #define CONFIG_WIFI_EAP_METHOD     EAP_PEAP
#endif

/* ***************************************************************** */
/* home network with SSID and password authentication */
/* ***************************************************************** */

/* the following defines which network is checked first: */
#if 1 /* home */
  #define CONFIG_WIFI_START_WITH       WIFI_PASSWORD_METHOD_PSK
#else /* HSLU */
  #define CONFIG_WIFI_START_WITH       WIFI_PASSWORD_METHOD_WPA2
#endif

#define CONFIG_WIFI_HOSTNAME          "myHostname" /*!< hostname for WiFi connection */

#define CONFIG_WIFI_EAP_ID            "myID"

#ifndef CONFIG_WIFI_EAP_SSID
  #define CONFIG_WIFI_EAP_SSID          "myEEE"
#endif
#ifndef CONFIG_WIFI_EAP_PASSWORD
  #define CONFIG_WIFI_EAP_PASSWORD      "passwordEEE"
#endif

#define CONFIG_WIFI_PSK_SSID          "SSIDhome"
#define CONFIG_WIFI_PSK_PASSWORD      "passwordHome"

#endif /* _WIFI_PWD_H_ */
