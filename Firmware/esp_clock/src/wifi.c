/*
 * Copyright (c) 2019, 2020, 2021, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * ******************************************************************
 * Dear programmer:
 * When I wrote this code, only god and I knew how it worked.
 * Now, only god knows it!
 *
 * Therefore, if you are trying to optimize or change this code
 * and it fails (most surely), please increase the counter below
 * as a warning for the next person:
 *
 * total_hours_wasted_here = 257
 * *******************************************************************
 */

#include "platform.h"
#if PL_CONFIG_USE_WIFI
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_eap_client.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#if PL_CONFIG_USE_SNTP_TIME
  #include "sntp_time.h"
#endif
#if PL_CONFIG_USE_UDP_CLIENT
  #include "udp_client.h"
#endif
#if PL_CONFIG_USE_UDP_SERVER
  #include "udp_server.h"
#endif
#if PL_CONFIG_USE_BLINKY
  #include "blinky.h"
  #define LED_ON_TIME_MS_CONNECTED      1000
  #define LED_ON_TIME_MS_DISCONNECTED   20
#endif
#include "McuUtility.h"
#include "McuXFormat.h"
#include "esp32_mac.h"
#include "McuLog.h"

#include "driver/gpio.h"

#define EAP_PEAP 1  /* WPA2 Enterprise with password and no certificate */
#define EAP_TTLS 2  /* TLS method */

#include "pwd.h"  /* local file with login information */

#ifndef CONFIG_ESP_MAXIMUM_RETRY
  #define CONFIG_ESP_MAXIMUM_RETRY (2) /*  number of retries to connect to the network */
#endif

static int s_retry_num = 0;

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t s_wifi_event_group;
/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_EVENT_HANDLER_CONNECTED_BIT (1<<0)
#define WIFI_EVENT_HANDLER_FAIL_BIT      (1<<1)
#define WIFI_CONNECTED_BIT               (1<<2)

static esp_netif_t *APP_WiFi_NetIf;
static bool APP_WiFi_isOn = true;

static int APP_WiFi_GetIpInfo(esp_netif_ip_info_t *pIp_info) {
  esp_netif_t *netif;

  netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  if (netif==NULL) {
    return -1; /* failed */
  }
  /* Copy the netif IP info into our variable. */
  esp_netif_get_ip_info(netif, pIp_info);
  return 0; /* OK */
}

static void GetIpInfoString(unsigned char *buf, size_t bufSize, esp_netif_ip_info_t *pIp_info) {
  McuXFormat_xsnprintf((char*)buf, bufSize, "IP:"IPSTR " MASK:"IPSTR " GW:"IPSTR, IP2STR(&pIp_info->ip), IP2STR(&pIp_info->netmask), IP2STR(&pIp_info->gw));
}

void APP_WiFi_PrintIP(void) {
  esp_netif_ip_info_t ip_info;

  if (APP_WiFi_GetIpInfo(&ip_info)!=0) {
    McuLog_error("failed getting netif()");
  } else {
    unsigned char buf[64];

    GetIpInfoString(buf, sizeof(buf), &ip_info);
    McuLog_info("%s", buf);
  }
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    McuLog_info("WIFI_EVENT_STA_START: start event");
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    McuLog_info("WIFI_EVENT_STA_DISCONNECTED: disconnected, retry %d", s_retry_num);
    if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
      esp_wifi_connect();
      s_retry_num++;
      McuLog_info("retry to connect to the AP");
    } else {
      xEventGroupSetBits(s_wifi_event_group, WIFI_EVENT_HANDLER_FAIL_BIT);
    }
    McuLog_info("connect to the AP fail");
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    McuLog_info("IP_EVENT_STA_GOT_IP: got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_EVENT_HANDLER_CONNECTED_BIT);
  }
}

typedef enum WiFi_PasswordMethod_e {
  WIFI_PASSWORD_METHOD_PSK,
  WIFI_PASSWORD_METHOD_WPA2,
} WiFi_PasswordMethod_e;

static void GetNetworkSSID(unsigned char *ssidBuf, size_t ssidBufSize) {
  wifi_ap_record_t ap;
  esp_err_t err;

  err = esp_wifi_sta_get_ap_info(&ap);
  if (err==ESP_OK) {
    McuUtility_strcpy(ssidBuf, ssidBufSize, ap.ssid);
  } else if (err==ESP_ERR_WIFI_CONN) {
    McuUtility_strcpy(ssidBuf, ssidBufSize, (unsigned char*)"not init");
  } else if (err==ESP_ERR_WIFI_NOT_CONNECT) {
    McuUtility_strcpy(ssidBuf, ssidBufSize, (unsigned char*)"not connected");
  } else {
    McuUtility_strcpy(ssidBuf, ssidBufSize, (unsigned char*)"error?");
  }
}

static void SetPasswordMode(WiFi_PasswordMethod_e mode) {
  wifi_config_t wifi_config;

  McuLog_info("SetPasswordMode(): %d", mode);
  ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
  memset(&wifi_config, 0, sizeof(wifi_config_t)); /* initialize all fields */
  if (mode==WIFI_PASSWORD_METHOD_WPA2) {
    strncpy((char*)wifi_config.sta.ssid, CONFIG_WIFI_EAP_SSID, sizeof(wifi_config.sta.ssid));
  } else if (mode==WIFI_PASSWORD_METHOD_PSK) {
    strncpy((char*)wifi_config.sta.ssid, CONFIG_WIFI_PSK_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, CONFIG_WIFI_PSK_PASSWORD, sizeof(wifi_config.sta.password));
  } else {
    McuLog_error("Wrong connection mode: %d", mode);
  }
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
  if (mode==WIFI_PASSWORD_METHOD_WPA2) {
    const ESP32_Device_t *device;

    device = ESP32_GetDeviceConfig();
    McuLog_info("EAP_ID: %s", device->eee_id);
    ESP_ERROR_CHECK( esp_eap_client_set_identity((uint8_t *)device->eee_id, strlen(device->eee_id)) );
    if (CONFIG_WIFI_EAP_METHOD == EAP_PEAP || CONFIG_WIFI_EAP_METHOD == EAP_TTLS) {
      McuLog_info("EAP_USERNAME: %s", device->eee_id);
      ESP_ERROR_CHECK( esp_eap_client_set_username((uint8_t *)device->eee_id, strlen(device->eee_id)) );
      ESP_ERROR_CHECK( esp_eap_client_set_password((uint8_t *)device->eee_pwd, strlen(device->eee_pwd)) );
    }
    ESP_ERROR_CHECK( esp_wifi_sta_enterprise_enable() );
  }
}

static void initialise_wifi(void) {
  WiFi_PasswordMethod_e mode = CONFIG_WIFI_START_WITH; /* starting mode */
  const ESP32_Device_t *config;

  s_wifi_event_group = xEventGroupCreate();
  if (esp_netif_init()!=ESP_OK) {
    McuLog_fatal("failed initializing network interface");
  }
  if (esp_event_loop_create_default()!=ESP_OK) {
    McuLog_fatal("failed creating default event loop");
  }
  APP_WiFi_NetIf = esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  if (esp_wifi_init(&cfg)!=ESP_OK) {
    McuLog_fatal("failed initializing WiFi");
  }

  config = ESP32_GetDeviceConfig();
  McuLog_info("Setting hostname: %s", config->hostName);
  if (esp_netif_set_hostname(APP_WiFi_NetIf, config->hostName)!=ESP_OK) {
    McuLog_fatal("failed setting hostname");
  }

  if (esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL)!=ESP_OK) {
    McuLog_fatal("failed registering event handler");
  }
  if (esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL)!=ESP_OK) {
    McuLog_fatal("failed registering event handler");
  }

  SetPasswordMode(mode);

  McuLog_info("Starting WiFi");
  if (esp_wifi_start()!=ESP_OK) {
    McuLog_fatal("failed starting WiFi");
  }

  /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
   * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
  EventBits_t bits;

  do {
    if (APP_WiFi_isOn) {
      bits = xEventGroupWaitBits(s_wifi_event_group,
              WIFI_EVENT_HANDLER_CONNECTED_BIT | WIFI_EVENT_HANDLER_FAIL_BIT, /* bits to wait for */
              pdTRUE, /* bits to clear on exit: we do not clear the WIFI_CONNECTED_BIT because used by WiFi task */
              pdFALSE, /* wait for all bits */
              pdMS_TO_TICKS(20000)); /* wait time */
      if (bits&WIFI_EVENT_HANDLER_CONNECTED_BIT) {
        break; /* leave loop */
      }
      if (bits&WIFI_EVENT_HANDLER_FAIL_BIT) {
        McuLog_info("FAILED connecting, restarting WiFi with different mode");
        ESP_ERROR_CHECK(esp_wifi_stop());
        /* toggle mode */
        if (mode==WIFI_PASSWORD_METHOD_PSK) {
          mode = WIFI_PASSWORD_METHOD_WPA2;
        } else if (mode==WIFI_PASSWORD_METHOD_WPA2) {
          ESP_ERROR_CHECK(esp_wifi_sta_enterprise_disable());
          mode = WIFI_PASSWORD_METHOD_PSK;
        }
        SetPasswordMode(mode);
        if (esp_wifi_start()!=ESP_OK) {
          McuLog_fatal("failed starting WiFi");
        }
      }
    } else {
      vTaskDelay(pdMS_TO_TICKS(500));
    }
  } while(true); /* breaks if we are connected */

  /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually happened. */
  if (bits & WIFI_EVENT_HANDLER_CONNECTED_BIT) {
    if (mode == WIFI_PASSWORD_METHOD_WPA2) {
      McuLog_info("connected to AP SSID: %s ",  CONFIG_WIFI_EAP_SSID);
    } else if (mode == WIFI_PASSWORD_METHOD_PSK) {
      McuLog_info("connected to AP SSID: %s",  CONFIG_WIFI_PSK_SSID);
    }
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  } else if (bits & WIFI_EVENT_HANDLER_FAIL_BIT) {
    if (mode == WIFI_PASSWORD_METHOD_WPA2) {
      McuLog_info("Failed to connect to SSID: %s", CONFIG_WIFI_EAP_SSID);
    } else if (mode == WIFI_PASSWORD_METHOD_PSK) {
      McuLog_info("Failed to connect to SSID: %s", CONFIG_WIFI_PSK_SSID);
    }
  } else {
    McuLog_error("UNEXPECTED EVENT");
  }
}

typedef enum WiFi_State_e {
  WIFI_STATE_INIT,
  WIFI_STATE_CONNECTED,
  WIFI_STATE_DISCONNECTED,
} WiFi_State_e;

volatile bool stopIt = true;

static void WiFiTask(void *pv) {
  bool isConnected = false;
  WiFi_State_e state = WIFI_STATE_INIT;

  McuLog_info("Initialize WiFi");
  initialise_wifi();
#if PL_CONFIG_USE_BLINKY
  Blinky_SetOnTime(LED_ON_TIME_MS_DISCONNECTED);
#endif
  for(;;) {
    vTaskDelay(pdMS_TO_TICKS(5000));
    isConnected = xEventGroupGetBits(s_wifi_event_group)&WIFI_CONNECTED_BIT;
    #if PL_CONFIG_USE_BLINKY
      Blinky_SetOnTime(isConnected?LED_ON_TIME_MS_CONNECTED:LED_ON_TIME_MS_DISCONNECTED);
    #endif
    if (state == WIFI_STATE_INIT && isConnected) { /* first connection */
      state = WIFI_STATE_CONNECTED;
      if (isConnected) {
    	  APP_WiFi_PrintIP();
      }
    #if PL_CONFIG_USE_UDP_SERVER
      McuLog_info("starting UDP server.");
      UDP_Server_Start();
    #endif
    #if PL_CONFIG_USE_UDP_CLIENT
      McuLog_info("starting UDP client.");
      UDP_Client_Start();
    #endif
    #if PL_CONFIG_USE_SNTP_TIME
      if (SNTP_ObtainTime()!=0) {
        McuLog_error("failed getting SNTP time.");
      }
    #endif
#if 0
    } else if (isConnected) {
      ESP_LOGI(TAG, "still connected.");
      APP_WiFi_PrintIP();

      const char *hostname;
      ESP_ERROR_CHECK(esp_netif_get_hostname(APP_WiFi_NetIf, &hostname));
      ESP_LOGI(TAG,"hostname: %s", hostname);
    } else {
      ESP_LOGI(TAG, "not connected yet.");
#endif
    }
  } /* for */
}

bool WiFi_isConnected(void) {
  bool isConnected;

  isConnected = xEventGroupGetBits(s_wifi_event_group)&WIFI_CONNECTED_BIT;
  return isConnected;
}

#if PL_CONFIG_USE_SHELL
static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  uint8_t buf[32];
  uint8_t mac[6];

  McuShell_SendStatusStr((unsigned char*)"wifi", (unsigned char*)"ESP32 WiFi status\r\n", io->stdOut);
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)IDF_VER);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  IDF", buf, io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  connected", WiFi_isConnected()?(unsigned char*)"yes\r\n":(unsigned char*)"no\r\n", io->stdOut);

  if (ESP32_MacRead(mac)==ERR_OK) {
    ESP32_MacToString(mac, buf, sizeof(buf));
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  } else {
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"FAILED\r\n");
  }
  McuShell_SendStatusStr((unsigned char*)"  MAC", buf, io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  WiFi", APP_WiFi_isOn?(unsigned char*)"on\r\n":(unsigned char*)"off\r\n", io->stdOut);

  if (WiFi_isConnected()) {
    unsigned char buf[64];
    const char *hostname;

    if (esp_netif_get_hostname(APP_WiFi_NetIf, &hostname)==ERR_OK) {
      McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)hostname);
      McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
    } else {
      McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"failed\r\n");
    }
    McuShell_SendStatusStr((unsigned char*)"  hostname", buf, io->stdOut);
    {
      esp_netif_ip_info_t ip_info;

      if (APP_WiFi_GetIpInfo(&ip_info)!=0) {
        McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"failed\r\n");
      } else {
        GetIpInfoString(buf, sizeof(buf), &ip_info);
        McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
        McuShell_SendStatusStr((unsigned char*)"  IP", buf, io->stdOut);
      }
    }

    GetNetworkSSID(buf, sizeof(buf));
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
    McuShell_SendStatusStr((unsigned char*)"  SSID", buf, io->stdOut);
  }
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"wifi", (unsigned char*)"Group of ESP32 WiFi commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows WiFi help or status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  on|off", (unsigned char*)"Turn WiFi on or off\r\n", io->stdOut);
  return ERR_OK;
}

uint8_t WiFi_ParseCommand(const unsigned char* cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"wifi help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"wifi status")==0) {
    *handled = TRUE;
    return PrintStatus(io);
  } else if (McuUtility_strcmp((char*)cmd, (char*)"wifi on")==0) {
    *handled = TRUE;
    APP_WiFi_isOn = true;
    return ERR_OK;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"wifi off")==0) {
    *handled = TRUE;
    APP_WiFi_isOn = false;
    return ERR_OK;
  }
  return ERR_OK;
}
#endif /* PL_CONFIG_USE_SHELL */

void WiFi_Init(void) {
  BaseType_t res;

  res = xTaskCreate(WiFiTask, "WiFi", 20*1024/sizeof(StackType_t), NULL, tskIDLE_PRIORITY, NULL);
  if (res==pdPASS) {
    McuLog_info("created WiFi task");
  } else {
    McuLog_error("failed creating WiFi task!");
  }
}
#endif /* PL_CONFIG_USE_WIFI */
