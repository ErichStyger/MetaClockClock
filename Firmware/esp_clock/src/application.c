/*
 * Copyright (c) 2023, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_PICO_W
  #include "pico/cyw43_arch.h"
  #include "PicoWiFi.h"
#endif
#include "application.h"
#include "McuRTOS.h"
#include "McuLog.h"
#include "McuUtility.h"
#include "McuLED.h"
#include "leds.h"
#if !PL_CONFIG_USE_BUTTONS_IRQ
  #include "buttons.h"
  #include "debounce.h"
#endif
#if PL_CONFIG_USE_OLED
  #include "oled.h"
#endif

#if PL_CONFIG_USE_BUTTONS
void App_OnButtonEvent(uint32_t buttonBits, McuDbnc_EventKinds kind) {
  unsigned char buf[64];

  buf[0] = '\0';
  if (buttonBits&BTN_BIT_NAV_UP) {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"up,");
  }
  if (buttonBits&BTN_BIT_NAV_DOWN) {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"down,");
  }
  if (buttonBits&BTN_BIT_NAV_LEFT) {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"left,");
  }
  if (buttonBits&BTN_BIT_NAV_RIGHT) {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"right,");
  }
  if (buttonBits&BTN_BIT_NAV_CENTER) {
    McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)"center,");
  }
  switch(kind) {
    case MCUDBNC_EVENT_PRESSED:             McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)" pressed"); break;
    case MCUDBNC_EVENT_PRESSED_REPEAT:      McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)" pressed repeat"); break;
    case MCUDBNC_EVENT_LONG_PRESSED:        McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)" long pressed"); break;
    case MCUDBNC_EVENT_LONG_PRESSED_REPEAT: McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)" long pressed repeat"); break;
    case MCUDBNC_EVENT_RELEASED:            McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)" released"); break;
    case MCUDBNC_EVENT_LONG_RELEASED:       McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)" long released"); break;
    case MCUDBNC_EVENT_END:                 McuUtility_strcat(buf, sizeof(buf), (const unsigned char*)" end"); break;
    default: break;
  } /* switch */
  if (buf[0]!='\0') {
    McuLog_info((char*)buf);
  #if PL_CONFIG_USE_OLED
    //OLED_SendText(buf);
  #endif
  }
}
#endif

static void AppTask(void *pv) {
#if PL_CONFIG_USE_PICO_W && PL_CONFIG_USE_LEDS
  Leds_InitFromTask(); /* needed for the on-board Pico-W LED */
#endif
  for(;;) {
#if PL_CONFIG_USE_BUTTONS && !PL_CONFIG_USE_BUTTONS_IRQ
    /*! \TODO if enabled WiFi, it triggers GPIO button interrupts? Doing polling instead */
    uint32_t buttons;

    buttons = BTN_GetButtons();
    if (buttons!=0) { /* poll buttons */
      Debounce_StartDebounce(buttons);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
#else
    vTaskDelay(pdMS_TO_TICKS(1000));
#endif
  }
}

#if 0 /* only for forced dump if system is overloaded */
#include "esp_sysview_trace.h"

static void SysviewTask(void *pv) {
  for(;;) {
    esp_sysview_flush(ESP_APPTRACE_TMO_INFINITE); /* must be frequent enough! */
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}
#endif

void App_Init(void) {
#if 0 /* only for forced dump if system is overloaded */
  if (xTaskCreatePinnedToCore(
      SysviewTask,  /* pointer to the task */
      "Sysview", /* task name for kernel awareness debugging */
      1500/sizeof(StackType_t), /* task stack size */
      (void*)NULL, /* optional task startup argument */
      tskIDLE_PRIORITY+5,  /* initial priority */
      (TaskHandle_t*)NULL, /* optional task handle to create */
      0 /* core */
    ) != pdPASS)
  {
    McuLog_fatal("Failed creating task");
    for(;;){} /* error! probably out of memory */
  }
#endif
  if (xTaskCreatePinnedToCore(
      AppTask,  /* pointer to the task */
      "App", /* task name for kernel awareness debugging */
      1500/sizeof(StackType_t), /* task stack size */
      (void*)NULL, /* optional task startup argument */
      tskIDLE_PRIORITY+2,  /* initial priority */
      (TaskHandle_t*)NULL, /* optional task handle to create */
      0 /* core */
    ) != pdPASS)
  {
    McuLog_fatal("Failed creating task");
    for(;;){} /* error! probably out of memory */
  }
}
