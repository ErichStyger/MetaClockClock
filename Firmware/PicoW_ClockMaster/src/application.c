/*
 * Copyright (c) 2023-2024, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_PICO_W
  #include "pico/cyw43_arch.h"
  #include "McuPicoWiFi.h"
#endif
#include "application.h"
#include "McuRTOS.h"
#include "McuLog.h"
#include "McuUtility.h"
#include "McuLED.h"
#include "leds.h"
#if PL_CONFIG_USE_BUTTONS && !PL_CONFIG_USE_BUTTONS_IRQ
  #include "buttons.h"
  #include "debounce.h"
#endif
#if PL_CONFIG_USE_OLED
  #include "oled.h"
#endif
#if PL_CONFIG_USE_GCOV
  #include "McuCoverage.h"
#endif
#if PL_CONFIG_USE_GPROF
  #include "profile_test.h"
#endif
#include <stdio.h>
#if PL_CONFIG_USE_MQTT_CLIENT
  #include "mqtt_client.h"
#endif
#if PL_CONFIG_USE_SENSOR
  #include "mqtt_sensor.h"
  #include "sensor.h"
#endif
#if PL_CONFIG_USE_WATCHDOG
  #include "McuWatchdog.h"
#endif
#if PL_CONFIG_USE_BALBOA
  #include "balboa.h"
  #include "mqtt_balboa.h"
#endif
#if PL_CONFIG_USE_TIME_DATE
  #include "McuTimeDate.h"
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
  uint32_t ms = 0;
  volatile bool dumpCoverage = true;

#if PL_CONFIG_USE_PICO_W && PL_CONFIG_USE_LEDS
  Leds_InitFromTask(); /* needed for the on-board Pico-W LED */
#endif
#if PL_CONFIG_USE_TIME_DATE
  McuTimeDate_Init(); /* if using RTOS with external RTC, need to do this in a task! */
#endif
  for(;;) {
  #if PL_CONFIG_USE_BUTTONS && !PL_CONFIG_USE_BUTTONS_IRQ
    /*! \TODO if enabled WiFi, it triggers GPIO button interrupts? Doing polling instead */
    uint32_t buttons;

    buttons = BTN_GetButtons();
    if (buttons!=0) { /* poll buttons */
      Debounce_StartDebounce(buttons);
    }
    vTaskDelay(pdMS_TO_TICKS(20));
    ms += 20;
  #else
    vTaskDelay(pdMS_TO_TICKS(1000));
    ms += 1000;
  #endif
  #if PL_CONFIG_USE_GCOV
    if (dumpCoverage && ms>5*1000) {
      vTaskEndScheduler(); /* exit scheduler to write coverage information */
    }
  #endif
  #if PL_CONFIG_USE_GPROF
    gprof_test();
    if (ms>5*1000) {
        vTaskEndScheduler();
    }
  #endif
  }
}

#if PL_CONFIG_USE_MQTT_CLIENT

static TaskHandle_t mqttTaskHandle = NULL;

void App_MqttTaskResume(void) {
  if (mqttTaskHandle!=NULL) {
    vTaskResume(mqttTaskHandle);
  }
}

void App_MqttTaskSuspend(void) {
  if (mqttTaskHandle!=NULL) {
    vTaskSuspend(mqttTaskHandle);
  }
}

#if PL_CONFIG_USE_BALBOA
static void ForceUpdateValues(balboa_publish_items_t *items) {
  /* force prev value to something different than the actual values to force an update. \todo use a 'forceUpdate' flag instead */
  items->current_temperature.prev_value = 0.0f;
  items->set_temperature.prev_value = 0.0f;
  items->lights.prev_value = !Balboa_GetLightIsOn();
  items->pump1.prev_value = !Balboa_GetPump1IsOn();
  items->pump2.prev_value  = !Balboa_GetPump2IsOn();
  items->pumpCirculation.prev_value = !Balboa_GetPumpCirculationIsOn();
  items->lowTempRange.prev_value = !Balboa_GetLowTempRangeIsOn();
  items->heatingReady.prev_value = !Balboa_GetHeatingReadyIsOn();
  items->filterCycle1.prev_filter.isEnabled = false;
  items->filterCycle1.prev_filter.startHour = 0;
  items->filterCycle1.prev_filter.startMinute = 0;
  items->filterCycle1.prev_filter.endHour = 0;
  items->filterCycle1.prev_filter.endMinute = 0;
  items->filterCycle2.prev_filter.isEnabled = false;
  items->filterCycle2.prev_filter.startHour = 0;
  items->filterCycle2.prev_filter.startMinute = 0;
  items->filterCycle2.prev_filter.endHour = 0;
  items->filterCycle2.prev_filter.endMinute = 0;
  items->time.prev_hours = 0;
  items->time.prev_minutes = 0;
  items->reboot.prev_value = true;
}
#endif /* #if PL_CONFIG_USE_BALBOA */

static void MqttTask(void *pv) {
  #if PL_CONFIG_USE_BALBOA
    balboa_publish_items_t items;
    int nofPublished;
    /* wait some time for network connection and RS-485 communication */
  #if PL_CONFIG_USE_WATCHDOG
    McuWatchdog_DelayAndReport(McuWatchdog_REPORT_ID_TASK_APP, 50, 100);
  #else
    vTaskDelay(pdMS_TO_TICKS(50*100));
  #endif
    ForceUpdateValues(&items); /* initial values */
  #endif
    items.reboot.isOn = false;
  for(;;) {
#if PL_CONFIG_USE_SENSOR
    if (MqttClient_CanPublish()) {
      float t, h;
      h = Sensor_GetHumidity();
      t = Sensor_GetTemperature();
      if (MqttClient_Publish_SensorValues(t, h)!=ERR_OK) {
        McuLog_error("failed publishing sensor values");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(5000));
#elif PL_CONFIG_USE_BALBOA
    if (MqttClient_CanPublish()) {
      items.current_temperature.temperature = Balboa_GetCurrentTemperature();
      items.set_temperature.temperature = Balboa_GetSetTemperature();
      items.lowTempRange.isOn = Balboa_GetLowTempRangeIsOn();
      items.lights.isOn = Balboa_GetLightIsOn();
      items.pump1.isOn = Balboa_GetPump1IsOn();
      items.pump2.isOn = Balboa_GetPump2IsOn();
      items.pumpCirculation.isOn = Balboa_GetPumpCirculationIsOn();
      items.heatingReady.isOn = Balboa_GetHeatingReadyIsOn();
      Balboa_GetFilterCycle(0, 
        &items.filterCycle1.filter.isEnabled, 
        &items.filterCycle1.filter.startHour, &items.filterCycle1.filter.startMinute, 
        &items.filterCycle1.filter.endHour, &items.filterCycle1.filter.endMinute);
      Balboa_GetFilterCycle(1, 
        &items.filterCycle2.filter.isEnabled, 
        &items.filterCycle2.filter.startHour, &items.filterCycle2.filter.startMinute, 
        &items.filterCycle2.filter.endHour, &items.filterCycle2.filter.endMinute);
      Balboa_GetTime(&items.time.hours, &items.time.minutes);
      if (MqttClient_Publish_BalboaValues(&items, &nofPublished)!=ERR_OK) {
        McuLog_error("failed publishing sensor values");
      }
      if (nofPublished>0) {
        Leds_On(LEDS_GREEN);
      #if PL_CONFIG_USE_WATCHDOG
        McuWatchdog_DelayAndReport(McuWatchdog_REPORT_ID_TASK_APP, 1, 50);
      #else
        vTaskDelay(pdMS_TO_TICKS(50));
      #endif
        Leds_Off(LEDS_GREEN);
      }
    }
  #if PL_CONFIG_USE_WATCHDOG
    McuWatchdog_DelayAndReport(McuWatchdog_REPORT_ID_TASK_APP, 5, 100);
  #else
    vTaskDelay(pdMS_TO_TICKS(5*100));
  #endif
#endif
  } /* for */
}

#endif

void App_Init(void) {
#if PL_CONFIG_USE_APP_TASK
  if (xTaskCreate(
      AppTask,  /* pointer to the task */
      "App", /* task name for kernel awareness debugging */
      1500/sizeof(StackType_t), /* task stack size */
      (void*)NULL, /* optional task startup argument */
      tskIDLE_PRIORITY+2,  /* initial priority */
      (TaskHandle_t*)NULL /* optional task handle to create */
    ) != pdPASS)
  {
    McuLog_fatal("Failed creating task");         // GCOVR_EXCL_LINE
    for(;;){} /* error! probably out of memory */ // GCOVR_EXCL_LINE
  }
#endif /* PL_CONFIG_USE_APP_TASK */
#if PL_CONFIG_USE_MQTT_CLIENT
  if (xTaskCreate(
      MqttTask,  /* pointer to the task */
      "mqtt", /* task name for kernel awareness debugging */
      4*1024/sizeof(StackType_t), /* task stack size */
      (void*)NULL, /* optional task startup argument */
      tskIDLE_PRIORITY+2,  /* initial priority */
      &mqttTaskHandle /* optional task handle to create */
    ) != pdPASS)
  {
    McuLog_fatal("Failed creating MQTT task");
    for(;;){} /* error! probably out of memory */
  }
#endif /* PL_CONFIG_USE_MQTT_CLIENT */
}
