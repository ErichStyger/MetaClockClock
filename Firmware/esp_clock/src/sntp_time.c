/*
 * Copyright (c) 2021, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "platform.h"
#if PL_CONFIG_USE_SNTP_TIME
#include "sntp_time.h"
#include "esp_sntp.h"
#include "esp_log.h"
#include "wifi.h"
#include "shell.h"
#include "McuTimeDate.h"
#include "McuUtility.h"
#include "McuLog.h"

static void SNTP_SetTime(void) {
  time_t now = 0;
  struct tm timeinfo = { 0 };

  time(&now);
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1); /* see https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv */
  tzset();
  localtime_r(&now, &timeinfo);

  char strftime_buf[64];

  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  McuLog_info("The current date/time is: %s", strftime_buf);

#if PL_CONFIG_USE_TIME_DATE
  uint32_t seconds;
  TIMEREC timeRec;
  DATEREC dateRec;

  /* set own time */
  seconds = (uint32_t)time(NULL); /* get number of seconds since 1970 */
  McuTimeDate_UnixSecondsToTimeDateCustom(seconds, -1, &timeRec, &dateRec, 1970);
  McuTimeDate_SetTimeDate(&timeRec, &dateRec);

#if PL_CONFIG_USE_ROBO_REMOTE
  unsigned char cmd[McuShell_CONFIG_DEFAULT_SHELL_BUFFER_SIZE];
  unsigned char response[64]; /* only dummy */
  static bool isInitialized = false;

  if (!isInitialized) { /* only once, not to interfer with challenge messages */
    isInitialized = true;
    /* set time and time of robot */
    McuUtility_strcpy(cmd, sizeof(cmd), (unsigned char*)"\"McuTimeDate date ");
    McuTimeDate_AddDateString(cmd, sizeof(cmd), &dateRec, (unsigned char*)McuTimeDate_CONFIG_DEFAULT_DATE_FORMAT_STR);
    McuUtility_strcat(cmd, sizeof(cmd), (unsigned char*)";McuTimeDate time ");
    McuTimeDate_AddTimeString(cmd, sizeof(cmd), &timeRec, (unsigned char*)McuTimeDate_CONFIG_DEFAULT_TIME_FORMAT_STR);
    McuUtility_strcat(cmd, sizeof(cmd), (unsigned char*)"\"");
    SHELL_SendToRobotAndGetResponse(cmd, response, sizeof(response));
  }
#endif
#endif
}

void time_sync_notification_cb(struct timeval *tv) {
  McuLog_info("Notification of a time synchronization event");
  SNTP_SetTime();
}

static void initialize_sntp(void) {
  McuLog_info("Initializing SNTP");
  esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, "pool.ntp.org");
  sntp_set_time_sync_notification_cb(time_sync_notification_cb);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
  sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
  esp_sntp_init();
}

int SNTP_ObtainTime(void) {
  const int retry_count = 10;
  int retry = 0;

  if (!WiFi_isConnected()) {
    McuLog_error("Cannot get sntp time, WiFi not connected");
    return -1; /* failure */
  }
  /**
   * NTP server address could be acquired via DHCP,
   * see LWIP_DHCP_GET_NTP_SRV menuconfig option
   */
#if LWIP_DHCP_GET_NTP_SRV
  esp_sntp_servermode_dhcp(true);
#endif

  initialize_sntp();

  /* wait for time to be set */
  while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
    McuLog_info("Waiting for system time to be set... (%d/%d)", retry, retry_count);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
  /* here we have received a valid time/date from the SNTP server */
  return 0; /* ok */
}

void SNTP_Init(void) {
  /* nothing needed */
}

#endif /* PL_CONFIG_USE_SNTP_TIME */
