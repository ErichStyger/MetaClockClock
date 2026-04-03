/*
 * Copyright (c) 2022-2024, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_PICO_W
#include "pico/cyw43_arch.h"
#include "PicoWiFi.h"

static bool cyw43IsInitialized = false;

void PicoWiFi_SetArchIsInitialized(bool isInitialized) {
  cyw43IsInitialized = isInitialized;
}

bool PicoWiFi_GetArchIsInitialized(void) {
  return cyw43IsInitialized;
}

const unsigned char *PicoWiFi_GetTcpIpLinkStatusString(int linkStatus) {
  /* return a string for the value returned by cyw43_tcpip_link_status() */
  const unsigned char *statusStr;

  switch(linkStatus) {
    case CYW43_LINK_DOWN:     statusStr = "LINK_DOWN, WiFi down"; break;
    case CYW43_LINK_JOIN:     statusStr = "LINK_JOIN, connected to WiFi"; break;
    case CYW43_LINK_NOIP:     statusStr = "LINK_NOIP, connected to WiFi, but no IP address "; break;
    case CYW43_LINK_UP:       statusStr = "LINK_UP, connect to WiFi with an IP address "; break;
    case CYW43_LINK_FAIL:     statusStr = "LINK_FAIL, connection failed "; break;
    case CYW43_LINK_NONET:    statusStr = "LINK_NONET, no matching SSID found "; break;
    case CYW43_LINK_BADAUTH:  statusStr = "LINK_BADAUTH, authentication failure"; break;
    default:                  statusStr = "<unknown>"; break;
  } /* switch */
  return statusStr;
}

void PicoWiFi_Deinit(void) {
  cyw43_arch_deinit();
  PicoWiFi_SetArchIsInitialized(false);
}

void PicoWiFi_Init(void) {
  PicoWiFi_SetArchIsInitialized(false);
}

#endif /* PL_CONFIG_USE_PICO_W */
