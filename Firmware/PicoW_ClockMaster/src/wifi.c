/*
 * Copyright (c) 2019-2025, Erich Styger
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
#if PL_CONFIG_USE_PICO_W
  #include "pico/cyw43_arch.h"
#endif
#if PL_CONFIG_USE_PICO_W && PL_CONFIG_USE_WIFI
  #include "lwip/ip4_addr.h"
#endif
#include "wifi.h"
#if PL_CONFIG_USE_PICO_W
  #include "PicoWiFi.h"
#endif
#if McuLib_CONFIG_CPU_IS_ESP32
  #include "esp_wifi.h"
  #include "esp_eap_client.h"
  #include "esp_event.h"
  #include "esp_system.h"
  #include "nvs_flash.h"
  #include "esp_netif.h"
  #include "esp32_mac.h"
  #include "pwd.h"
#endif
#include "McuRTOS.h"
#include "McuUtility.h"
#include "McuLog.h"
#include "McuXFormat.h"
#if PL_CONFIG_USE_PING
  #include "ping.h"
#endif
#if PL_CONFIG_USE_NTP_CLIENT
  #include "ntp_client.h"
#endif
#if PL_CONFIG_USE_MQTT_CLIENT
  #include "mqtt_client.h"
#endif
#if PL_CONFIG_USE_UDP_CLIENT
  #include "udp_client.h"
#endif
#if PL_CONFIG_USE_UDP_SERVER
  #include "udp_server.h"
#endif
#if PL_CONFIG_USE_MININI
  #include "minIni/McuMinINI.h"
  #include "MinIniKeys.h"
#endif
#if PL_CONFIG_USE_WATCHDOG
  #include "McuWatchdog.h"
#endif
#include "application.h"

#if McuLib_CONFIG_CPU_IS_ESP32
  #if PL_CONFIG_USE_BLINKY
    #include "blinky.h"
    #define LED_ON_TIME_MS_CONNECTED      1000
    #define LED_ON_TIME_MS_DISCONNECTED   20
  #endif
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
#endif /* McuLib_CONFIG_CPU_IS_ESP32 */

#define EAP_PEAP 1  /* WPA2 Enterprise with password and no certificate */
#define EAP_TTLS 2  /* TLS method */

typedef enum WiFi_PasswordMethod_e {
  WIFI_PASSWORD_METHOD_PSK,
  WIFI_PASSWORD_METHOD_WPA2,  /* not supported yet */
} WiFi_PasswordMethod_e;

#define CONFIG_USE_EEE   (0) /* EDUROAM does not yet work with the Pico W! */

#if CONFIG_USE_EEE
//  static const WiFi_PasswordMethod_e networkMode = WIFI_PASSWORD_METHOD_WPA2;
#else
//  static const WiFi_PasswordMethod_e networkMode = WIFI_PASSWORD_METHOD_PSK;
#endif

#if McuLib_CONFIG_CPU_IS_ESP32
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
#endif /* McuLib_CONFIG_CPU_IS_ESP32 */

/* default values for network */
#define WIFI_DEFAULT_HOSTNAME   "pico"
#define WIFI_DEFAULT_SSID       "ssid"
#define WIFI_DEFAULT_PASS       "password"

static struct wifi {
  bool isConnected;   /* if we are connected to the network */
#if PL_CONFIG_USE_WIFI
  bool isEnabled;     /* if true, it tries to connect to the network */
  unsigned char hostname[32];
  unsigned char ssid[32];
  unsigned char pass[64];
#endif
  TaskHandle_t taskHandle;
} wifi;

static uint8_t GetMAC(uint8_t mac[6], uint8_t *macStr, size_t macStrSize) {
#if PL_CONFIG_USE_PICO_W
  if (cyw43_wifi_get_mac(&cyw43_state, 0, mac)!=0) {
    return ERR_FAILED;
  }
#elif McuLib_CONFIG_CPU_IS_ESP32
  if (ESP32_MacRead(mac)!=ERR_OK) {
    return ERR_FAILED;
  }
#endif
  macStr[0] = '\0';
  for(int i=0; i<6; i++) {
    McuUtility_strcatNum8Hex(macStr, macStrSize, mac[i]);
    if (i<6-1) {
      McuUtility_chcat(macStr, macStrSize, ':');
    }
  }
  return ERR_OK;
}

#if McuLib_CONFIG_CPU_IS_ESP32
bool WiFi_isConnected(void) {
  bool isConnected;

  isConnected = xEventGroupGetBits(s_wifi_event_group)&WIFI_CONNECTED_BIT;
  return isConnected;
}
#endif /* McuLib_CONFIG_CPU_IS_ESP32 */

#if McuLib_CONFIG_CPU_IS_ESP32
#if 0
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
#endif
#endif /* McuLib_CONFIG_CPU_IS_ESP32 */

#if McuLib_CONFIG_CPU_IS_ESP32
static void SetPasswordMode(WiFi_PasswordMethod_e mode) {
  wifi_config_t wifi_config;

  McuLog_info("SetPasswordMode(): %d", mode);
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  memset(&wifi_config, 0, sizeof(wifi_config_t)); /* initialize all fields */
  if (mode==WIFI_PASSWORD_METHOD_WPA2) {
    strncpy((char*)wifi_config.sta.ssid, CONFIG_WIFI_EAP_SSID, sizeof(wifi_config.sta.ssid));
  } else if (mode==WIFI_PASSWORD_METHOD_PSK) {
    strncpy((char*)wifi_config.sta.ssid, (char*)wifi.ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, (char*)wifi.pass, sizeof(wifi_config.sta.password));
  } else {
    McuLog_error("Wrong connection mode: %d", mode);
  }
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
#if PL_CONFIG_USE_WIFI_EEE
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
#endif /* PL_CONFIG_USE_WIFI_EEE */
}
#endif /* McuLib_CONFIG_CPU_IS_ESP32 */

#if McuLib_CONFIG_CPU_IS_ESP32
static int initialise_esp_wifi(void) {
  WiFi_PasswordMethod_e mode = CONFIG_WIFI_START_WITH; /* starting mode */

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
    return -1;
  }

#if PL_CONFIG_USE_IDENTIFY
  const ESP32_Device_t *config;

  config = ESP32_GetDeviceConfig();
  McuLog_info("Setting hostname: %s", config->hostName);
  if (esp_netif_set_hostname(APP_WiFi_NetIf, config->hostName)!=ESP_OK) {
    McuLog_fatal("failed setting hostname");
  }
#endif
  if (esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL)!=ESP_OK) {
    McuLog_fatal("failed registering event handler");
    return -2;
  }
  if (esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL)!=ESP_OK) {
    McuLog_fatal("failed registering event handler");
    return -3;
  }

  SetPasswordMode(mode);

  McuLog_info("Starting WiFi");
  if (esp_wifi_start()!=ESP_OK) {
    McuLog_fatal("failed starting WiFi");
    return -4;
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
      McuLog_info("connected to AP SSID: %s",  wifi.ssid);
    }
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    return 0;
  } else if (bits & WIFI_EVENT_HANDLER_FAIL_BIT) {
    if (mode == WIFI_PASSWORD_METHOD_WPA2) {
      McuLog_info("Failed to connect to SSID: %s", CONFIG_WIFI_EAP_SSID);
    } else if (mode == WIFI_PASSWORD_METHOD_PSK) {
      McuLog_info("Failed to connect to SSID: %s", wifi.ssid);
    }
    return -5;
  } else {
    McuLog_error("UNEXPECTED EVENT");
    return -6;
  }
}
#endif /* #if McuLib_CONFIG_CPU_IS_ESP32 */

#if PL_CONFIG_USE_PING
static void ping_setup(const char *host) {
  static ip_addr_t ping_addr; /* has to be global! */

  if (wifi.isConnected) {
    ip4_addr_set_u32(&ping_addr, ipaddr_addr(host));
    Ping_InitAddress(&ping_addr);
  }
}
#endif

#if PL_CONFIG_USE_WIFI
static void initWiFi(void) {
#if PL_CONFIG_USE_PICO_W
  PicoWiFi_SetArchIsInitialized(true);
#endif
#if PL_CONFIG_USE_WIFI
  McuLog_info("enabling STA mode");
  #if PL_CONFIG_USE_PICO_W
  cyw43_arch_enable_sta_mode();
  #endif
#if PL_CONFIG_USE_MININI
  McuMinINI_ini_gets(NVMC_MININI_SECTION_WIFI, NVMC_MININI_KEY_WIFI_HOSTNAME, WIFI_DEFAULT_HOSTNAME, (char*)wifi.hostname, sizeof(wifi.hostname), NVMC_MININI_FILE_NAME);
  McuMinINI_ini_gets(NVMC_MININI_SECTION_WIFI, NVMC_MININI_KEY_WIFI_SSID,     WIFI_DEFAULT_SSID,     (char*)wifi.ssid, sizeof(wifi.ssid), NVMC_MININI_FILE_NAME);
  McuMinINI_ini_gets(NVMC_MININI_SECTION_WIFI, NVMC_MININI_KEY_WIFI_PASS,     WIFI_DEFAULT_PASS,     (char*)wifi.pass, sizeof(wifi.pass), NVMC_MININI_FILE_NAME);
#else
  McuUtility_strcpy(wifi.hostname, sizeof(wifi.hostname), WIFI_DEFAULT_HOSTNAME);
  McuUtility_strcpy(wifi.ssid, sizeof(wifi.ssid), WIFI_DEFAULT_SSID);
  McuUtility_strcpy(wifi.pass, sizeof(wifi.pass), WIFI_DEFAULT_PASS);
#endif
#if PL_CONFIG_USE_PICO_W
  McuLog_info("setting hostname: %s", wifi.hostname);
  netif_set_hostname(&cyw43_state.netif[0], wifi.hostname);
#endif
#if PL_CONFIG_USE_WATCHDOG
  McuWatchdog_DelayAndReport(McuWatchdog_REPORT_ID_TASK_WIFI, 10, 100);
#else
  vTaskDelay(pdMS_TO_TICKS(10*100)); /* give network tasks time to start up */
#endif
}
#endif /* PL_CONFIG_USE_WIFI */

#if PL_CONFIG_USE_PICO_W && PL_CONFIG_USE_WIFI || McuLib_CONFIG_CPU_IS_ESP32
static bool connectToWiFi(void) {
  bool isConnected = false;

  for(;;) { /* retries connection if it failed, breaks loop if success */
    if (!wifi.isEnabled) {
      break;
    }
    McuLog_info("connecting to SSID '%s'...", wifi.ssid);
  #if PL_CONFIG_USE_WATCHDOG
    TickType_t tickCount = McuWatchdog_ReportTimeStart();
    McuWatchdog_SuspendCheck(McuWatchdog_REPORT_ID_TASK_WIFI);
  #endif
  #if PL_CONFIG_USE_PICO_W
//    int res = cyw43_arch_wifi_connect_timeout_ms(wifi.ssid, wifi.pass, CYW43_AUTH_OPEN, 30000); /* can take some time to connect */
    int res = cyw43_arch_wifi_connect_timeout_ms(wifi.ssid, wifi.pass, CYW43_AUTH_WPA2_AES_PSK, 30000); /* can take some time to connect */
  #elif McuLib_CONFIG_CPU_IS_ESP32
    int res = initialise_esp_wifi();  
  #endif
  #if PL_CONFIG_USE_WATCHDOG
    McuWatchdog_ResumeCheck(McuWatchdog_REPORT_ID_TASK_WIFI);
    McuWatchdog_ReportTimeEnd(McuWatchdog_REPORT_ID_TASK_WIFI, tickCount);
  #endif
    if (res!=0) {
      McuLog_error("connection failed! code %d", res);
      #if PL_CONFIG_USE_WATCHDOG
        McuWatchdog_DelayAndReport(McuWatchdog_REPORT_ID_TASK_WIFI, 50, 100);
      #else
        vTaskDelay(pdMS_TO_TICKS(50*100)); /* limit message output */
      #endif
    } else {
      McuLog_info("success!");
    #if PL_CONFIG_USE_UDP_SERVER
      McuLog_info("resuming UDP server.");
      UdpServer_Resume();
    #endif
    #if PL_CONFIG_USE_UDP_CLIENT
      McuLog_info("resuming UDP client.");
      UdpClient_Resume();
    #endif
    #if PL_CONFIG_USE_NTP_CLIENT
      if (NtpClient_GetDefaultStart()) {
        McuLog_info("resuming NTP client.");
        NtpClient_TaskResume();
      }
    #endif
    #if PL_CONFIG_USE_MQTT_CLIENT
      if (MqttClient_Connect()!=ERR_OK) {
        McuLog_error("Failled connecting to MQTT broker");
        MqttClient_Disconnect(); /* make sure it is disconnected */
      }
      App_MqttTaskResume();
    #endif
      isConnected = true;
      break; /* break for loop */
    }
  } /* for */
  return isConnected;
}
#endif /* PL_CONFIG_USE_WIFI */

#if PL_CONFIG_USE_WIFI
static bool disconnectWiFi(void) {
  if (wifi.isConnected) {
    #if PL_CONFIG_USE_NTP_CLIENT
      NtpClient_TaskSuspend();
    #endif
    #if PL_CONFIG_USE_MQTT_CLIENT
      App_MqttTaskSuspend();
      MqttClient_Disconnect();
    #endif
    #if PL_CONFIG_USE_PICO_W
    if (cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA )!=0) {
      McuLog_fatal("leaving WiFi failed");
    }
    #endif
  }
  return false; /* not connected any more */
}
#endif

#if McuLib_CONFIG_CPU_IS_ESP32
typedef enum WiFi_State_e {
  WIFI_STATE_INIT,
  WIFI_STATE_CONNECTED,
  WIFI_STATE_DISCONNECTED,
} WiFi_State_e;

static void WiFiTask(void *pv) {
  WiFi_State_e state = WIFI_STATE_INIT;

  McuLog_info("Starting WiFi task");
#if PL_CONFIG_USE_BLINKY
  Blinky_SetOnTime(LED_ON_TIME_MS_DISCONNECTED);
#endif
  for(;;) {
    if (!wifi.isConnected && wifi.isEnabled) { /* connect to the network */
      initWiFi(); /* initialize connection and WiFi settings */
      wifi.isConnected = connectToWiFi();
    } else if (wifi.isConnected && !wifi.isEnabled) { /* request to disconnect from network */
      wifi.isConnected = disconnectWiFi();
    }
    vTaskDelay(pdMS_TO_TICKS(5000));
    wifi.isConnected = xEventGroupGetBits(s_wifi_event_group)&WIFI_CONNECTED_BIT;
    #if PL_CONFIG_USE_BLINKY
      Blinky_SetOnTime(wifi.isConnected?LED_ON_TIME_MS_CONNECTED:LED_ON_TIME_MS_DISCONNECTED);
    #endif
    if (state == WIFI_STATE_INIT && wifi.isConnected) { /* first connection */
      state = WIFI_STATE_CONNECTED;
      if (wifi.isConnected) {
    	  APP_WiFi_PrintIP();
      }
    #if PL_CONFIG_USE_UDP_SERVER
      McuLog_info("resuming UDP server.");
      UdpServer_Resume();
    #endif
    #if PL_CONFIG_USE_UDP_CLIENT
      McuLog_info("resuming UDP client.");
      UdpClient_Resume();
    #endif
    #if PL_CONFIG_USE_NTP_CLIENT
      if (NtpClient_GetDefaultStart()) {
        McuLog_info("resuming NTP client.");
        NtpClient_TaskResume();
      }
    #endif
    #if PL_CONFIG_USE_MQTT_CLIENT
      if (MqttClient_Connect()!=ERR_OK) {
        McuLog_error("Failled connecting to MQTT broker");
        MqttClient_Disconnect(); /* make sure it is disconnected */
      }
      App_MqttTaskResume();
    #endif
    }
  } /* for */
}
#endif /* #if McuLib_CONFIG_CPU_IS_ESP32 */

#if PL_CONFIG_USE_PICO_W
static void WiFiTask(void *pv) {
  int res;
#if PL_CONFIG_USE_PICO_W
  bool ledIsOn = false;
#endif

#if CONFIG_USE_EEE
  if (networkMode == WIFI_PASSWORD_METHOD_WPA2) {
    McuLog_info("using WPA2");
  }
#endif
#define WIFI_DEFAULT_ENABLE   true
#if PL_CONFIG_USE_MININI
  wifi.isEnabled = McuMinINI_ini_getbool(NVMC_MININI_SECTION_WIFI, NVMC_MININI_KEY_WIFI_ENABLE, WIFI_DEFAULT_ENABLE, NVMC_MININI_FILE_NAME);
#else
  wifi.isEnabled = WIFI_DEFAULT_ENABLE;
#endif
  McuLog_info("starting WiFi task");
#if PL_CONFIG_USE_PICO_W
  /* initialize CYW43 architecture
      - will enable BT if CYW43_ENABLE_BLUETOOTH == 1
      - will enable lwIP if CYW43_LWIP == 1
    */
  if (cyw43_arch_init_with_country(CYW43_COUNTRY_SWITZERLAND)!=0) {
    for(;;) {
      McuLog_error("failed setting country code");
      vTaskDelay(pdMS_TO_TICKS(5000));
    }
  }
#endif
#if PL_CONFIG_USE_WIFI
  for(;;) {
    if (!wifi.isConnected && wifi.isEnabled) { /* connect to the network */
      initWiFi(); /* initialize connection and WiFi settings */
      wifi.isConnected = connectToWiFi();
    } else if (wifi.isConnected && !wifi.isEnabled) { /* request to disconnect from network */
      wifi.isConnected = disconnectWiFi();
    }
    {
    #if PL_CONFIG_USE_PICO_W
      // see https://forums.raspberrypi.com/viewtopic.php?t=347706
      int linkStatus;
      static int oldLinkStatus = -1;
      const unsigned char *statusStr;

      linkStatus = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
      if (linkStatus!=oldLinkStatus) {
        oldLinkStatus = linkStatus;
        McuLog_trace("new TCP/IP link status: %s", PicoWiFi_GetTcpIpLinkStatusString(linkStatus));
      }
    #endif
    }
    #if PL_CONFIG_USE_PICO_W
    /* blink LED */
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, ledIsOn);
    ledIsOn = !ledIsOn;
    #endif
    if (wifi.isConnected) {
    #if PL_CONFIG_USE_WATCHDOG
      McuWatchdog_DelayAndReport(McuWatchdog_REPORT_ID_TASK_WIFI, 10, 100);
    #else
      vTaskDelay(pdMS_TO_TICKS(10*100));
    #endif
    } else {
    #if PL_CONFIG_USE_WATCHDOG
      McuWatchdog_DelayAndReport(McuWatchdog_REPORT_ID_TASK_WIFI, 1, 50);
    #else
      vTaskDelay(pdMS_TO_TICKS(50));
    #endif
    }
  }
#else /* not using WiFi: blink only LED */
  for(;;) {
    #if PL_CONFIG_USE_PICO_W
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, ledIsOn);
    ledIsOn = !ledIsOn;
    #endif
    vTaskDelay(pdMS_TO_TICKS(100));
  }
#endif
}
#endif /* PL_CONFIG_USE_PICO_W */

#endif /* PL_CONFIG_USE_WIFI */

#if PL_CONFIG_USE_WIFI
static uint8_t SetSSID(const unsigned char *ssid) {
  unsigned char buf[64];

  McuUtility_ScanDoubleQuotedString(&ssid, buf, sizeof(buf));
  McuUtility_strcpy(wifi.ssid, sizeof(wifi.ssid), buf);
#if PL_CONFIG_USE_MININI
  McuMinINI_ini_puts(NVMC_MININI_SECTION_WIFI, NVMC_MININI_KEY_WIFI_SSID, (char*)wifi.ssid, NVMC_MININI_FILE_NAME);
#endif
  return ERR_OK;
}
#endif

#if PL_CONFIG_USE_WIFI
static uint8_t SetPwd(const unsigned char *pwd) {
  unsigned char buf[64];

  McuUtility_ScanDoubleQuotedString(&pwd, buf, sizeof(buf));
  McuUtility_strcpy(wifi.pass, sizeof(wifi.pass), buf);
#if PL_CONFIG_USE_MININI
  McuMinINI_ini_puts(NVMC_MININI_SECTION_WIFI, NVMC_MININI_KEY_WIFI_PASS, (char*)wifi.pass, NVMC_MININI_FILE_NAME);
#endif
  return ERR_OK;
}
#endif

#if PL_CONFIG_USE_WIFI
static uint8_t SetHostname(const unsigned char *pwd) {
  unsigned char buf[64];

  McuUtility_ScanDoubleQuotedString(&pwd, buf, sizeof(buf));
  McuUtility_strcpy(wifi.hostname, sizeof(wifi.hostname), buf);
#if PL_CONFIG_USE_MININI
  McuMinINI_ini_puts(NVMC_MININI_SECTION_WIFI, NVMC_MININI_KEY_WIFI_HOSTNAME, (char*)wifi.hostname, NVMC_MININI_FILE_NAME);
#endif
  return ERR_OK;
}
#endif

#if PL_CONFIG_USE_WIFI
static uint8_t WiFi_Enable(bool enable) {
#if PL_CONFIG_USE_MININI
  if (McuMinINI_ini_putl(NVMC_MININI_SECTION_WIFI, NVMC_MININI_KEY_WIFI_ENABLE, enable, NVMC_MININI_FILE_NAME)!=1) { /* 1: success */
    return ERR_FAILED;
  }
#endif
  wifi.isEnabled = enable;
  return ERR_OK;
}
#endif

static const char *WiFi_GetHostName(void) {
#if McuLib_CONFIG_CPU_IS_ESP32
  const char *hostname;
  if (esp_netif_get_hostname(APP_WiFi_NetIf, &hostname)==ESP_OK) {
    return hostname;
  } else {
    return "esp_netif_get_hostname() failed!";
  }
#elif PL_CONFIG_USE_PICO_W
  return netif_get_hostname(&cyw43_state.netif[0]);
#endif
  return "unknown";
}

static uint8_t PrintStatus(McuShell_ConstStdIOType *io) {
  uint8_t mac[6];
  uint8_t macStr[] = "00:00:00:00:00:00\r\n";
  uint8_t buf[96];

/* load current values: they get loaded again if WiFi gets initialized. */
#if PL_CONFIG_USE_MININI
  McuMinINI_ini_gets(NVMC_MININI_SECTION_WIFI, NVMC_MININI_KEY_WIFI_HOSTNAME, WIFI_DEFAULT_HOSTNAME, (char*)wifi.hostname, sizeof(wifi.hostname), NVMC_MININI_FILE_NAME);
  McuMinINI_ini_gets(NVMC_MININI_SECTION_WIFI, NVMC_MININI_KEY_WIFI_SSID,     WIFI_DEFAULT_SSID,     (char*)wifi.ssid, sizeof(wifi.ssid), NVMC_MININI_FILE_NAME);
  McuMinINI_ini_gets(NVMC_MININI_SECTION_WIFI, NVMC_MININI_KEY_WIFI_PASS,     WIFI_DEFAULT_PASS,     (char*)wifi.pass, sizeof(wifi.pass), NVMC_MININI_FILE_NAME);
#else
  McuUtility_strcpy(wifi.hostname, sizeof(wifi.hostname), WIFI_DEFAULT_HOSTNAME);
  McuUtility_strcpy(wifi.ssid, sizeof(wifi.ssid), WIFI_DEFAULT_SSID);
  McuUtility_strcpy(wifi.pass, sizeof(wifi.pass), WIFI_DEFAULT_PASS);
#endif
  McuShell_SendStatusStr((unsigned char*)"wifi", (const unsigned char*)"Status of WiFi\r\n", io->stdOut);
#if McuLib_CONFIG_CPU_IS_ESP32
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)IDF_VER);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  IDF", buf, io->stdOut);
#endif
#if PL_CONFIG_USE_WIFI
  McuShell_SendStatusStr((uint8_t*)"  enabled", wifi.isEnabled?(unsigned char*)"yes\r\n":(unsigned char*)"no\r\n", io->stdOut);
  McuShell_SendStatusStr((uint8_t*)"  connected", wifi.isConnected?(unsigned char*)"yes\r\n":(unsigned char*)"no\r\n", io->stdOut);
  McuUtility_strcpy(buf, sizeof(buf), wifi.ssid);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((uint8_t*)"  SSID", buf, io->stdOut);

  McuUtility_strcpy(buf, sizeof(buf), wifi.pass);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((uint8_t*)"  pass", buf, io->stdOut);

#if PL_CONFIG_USE_PICO_W
  int val = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
  if (val<0) {
    McuUtility_strcpy(buf, sizeof(buf), "ERROR\r\n");
  } else {
    switch(val) {
      case CYW43_LINK_JOIN: McuUtility_strcpy(buf, sizeof(buf), "joined\r\n"); break;
      case CYW43_LINK_FAIL: McuUtility_strcpy(buf, sizeof(buf), "failed\r\n"); break;
      case CYW43_LINK_NONET: McuUtility_strcpy(buf, sizeof(buf), "no network\r\n"); break;
      case CYW43_LINK_BADAUTH: McuUtility_strcpy(buf, sizeof(buf), "bad authentication\r\n"); break;
      default: McuUtility_strcpy(buf, sizeof(buf), "ERR\r\n"); break;
    }
  }
  McuShell_SendStatusStr((uint8_t*)"  wifi link", buf, io->stdOut);
#endif /* PL_CONFIG_USE_PICO_W */

#if PL_CONFIG_USE_PICO_W
  McuUtility_strcpy(buf, sizeof(buf), PicoWiFi_GetTcpIpLinkStatusString(cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA)));
  McuUtility_strcat(buf, sizeof(buf), "\r\n");
  McuShell_SendStatusStr((uint8_t*)"  tcp link", buf, io->stdOut);
#endif

#endif /* PL_CONFIG_USE_WIFI */

  if (GetMAC(mac, macStr, sizeof(macStr))==ERR_OK) {
    McuUtility_strcat(macStr, sizeof(macStr), (unsigned char*)"\r\n");
  } else {
    McuUtility_strcpy(macStr, sizeof(macStr), (unsigned char*)"ERROR\r\n");
  }
  McuShell_SendStatusStr((uint8_t*)"  MAC", macStr, io->stdOut);
#if PL_CONFIG_USE_WIFI
#if PL_CONFIG_USE_PICO_W
  McuUtility_strcpy(buf, sizeof(buf), ip4addr_ntoa(netif_ip4_addr(netif_list)));
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((uint8_t*)"  IP", buf, io->stdOut);
#elif McuLib_CONFIG_CPU_IS_ESP32
  {
    esp_netif_ip_info_t ip_info;

    if (APP_WiFi_GetIpInfo(&ip_info)==0) {
      GetIpInfoString(buf, sizeof(buf), &ip_info);
      McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
    } else {
      McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"failed getting IP info\r\n");
    }
    McuShell_SendStatusStr((uint8_t*)"  IP", buf, io->stdOut);
  }
#endif

  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)WiFi_GetHostName());
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((uint8_t*)"  hostname", buf, io->stdOut);
#endif
  return ERR_OK;
}

uint8_t WiFi_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  const unsigned char *p;

  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "wifi help")==0) {
    McuShell_SendHelpStr((unsigned char*)"wifi", (const unsigned char*)"Group of WiFi application commands\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char*)"  help|status", (const unsigned char*)"Print help or status information\r\n", io->stdOut);
  #if PL_CONFIG_USE_WIFI
    McuShell_SendHelpStr((unsigned char*)"  enable|disable", (const unsigned char*)"Enable or disable WiFi connection\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char*)"  set ssid \"<ssid>\"", (const unsigned char*)"Set the SSID\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char*)"  set pwd \"<password>\"", (const unsigned char*)"Set the password\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char*)"  set hostname \"<name>\"", (const unsigned char*)"Set the hostname\r\n", io->stdOut);
  #endif
  #if PL_CONFIG_USE_PING
    McuShell_SendHelpStr((unsigned char*)"  ping <host>", (const unsigned char*)"Ping host\r\n", io->stdOut);
  #endif
    *handled = TRUE;
    return ERR_OK;
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "wifi status")==0)) {
    *handled = TRUE;
    return PrintStatus(io);
  #if PL_CONFIG_USE_WIFI
  } else if (McuUtility_strncmp((char*)cmd, "wifi set ssid ", sizeof("wifi set ssid ")-1)==0) {
    *handled = TRUE;
    p = cmd + sizeof("wifi set ssid ")-1;
    return SetSSID(p);
  } else if (McuUtility_strncmp((char*)cmd, "wifi set pwd ", sizeof("wifi set pwd ")-1)==0) {
    *handled = TRUE;
    p = cmd + sizeof("wifi set pwd ")-1;
    return SetPwd(p);
  } else if (McuUtility_strncmp((char*)cmd, "wifi set hostname ", sizeof("wifi set hostname ")-1)==0) {
    *handled = TRUE;
    p = cmd + sizeof("wifi set hostname ")-1;
    return SetHostname(p);
  #endif
  #if PL_CONFIG_USE_PING
  } else if (McuUtility_strncmp((char*)cmd, "wifi ping ", sizeof("wifi ping ")-1)==0) {
    *handled = TRUE;
    p = cmd + sizeof("wifi ping ")-1;
    ping_setup(p);
    return ERR_OK;
  #endif
  #if PL_CONFIG_USE_WIFI
  } else if (McuUtility_strcmp((char*)cmd, "wifi enable")==0) {
    *handled = TRUE;
    return WiFi_Enable(true);
  } else if (McuUtility_strcmp((char*)cmd, "wifi disable")==0) {
    *handled = TRUE;
    return WiFi_Enable(false);
  #endif
  }
  return ERR_OK;
}

void WiFi_Deinit(void) {
  wifi.isConnected = false;
}

void WiFi_Init(void) {
  wifi.isConnected = false;
#if PL_CONFIG_USE_WIFI
  wifi.isEnabled = true;
  if (xTaskCreate(
      WiFiTask,  /* pointer to the task */
      "WiFi", /* task name for kernel awareness debugging */
#if PL_CONFIG_USE_PICO_W
      4096/sizeof(StackType_t), /* task stack size */
#elif McuLib_CONFIG_CPU_IS_ESP32
      20*1024/sizeof(StackType_t), /* task stack size */
#endif
      (void*)NULL, /* optional task startup argument */
      tskIDLE_PRIORITY+2,  /* initial priority */
      &wifi.taskHandle
    ) != pdPASS)
  {
    McuLog_fatal("failed creating task");
    for(;;){} /* error! probably out of memory */
  }
#endif /* PL_CONFIG_USE_WIFI */
}

