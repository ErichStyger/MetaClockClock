/*
 * Copyright (c) 2023, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_SENSOR
#include "sensor.h"
#include "McuUtility.h"
#include "McuLog.h"
#include "McuRTOS.h"
#include "McuGDisplaySSD1306.h"
#include "McuFontDisplay.h"
#include "McuFontHelv18Bold.h"
#if PL_CONFIG_USE_SHT31
  #include "McuSHT31.h"
#elif PL_CONFIG_USE_SHT40
  #include "McuSHT40.h"
#else
  #error "unknown sensor?"
#endif
#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  #include "McuSystemView.h"
#endif

static float Sensor_temperature, Sensor_humidity;

float Sensor_GetTemperature(void) {
  return Sensor_temperature; /* technically no mutex required, as 32bit access in one instruction. But leave at least a comment. Mutex required if both sensor values need to be from the same time */
}

float Sensor_GetHumidity(void) {
  return Sensor_humidity; /* technically no mutex required, as 32bit access in one instruction */
}

static void sensorTask(void *pv) {
  uint8_t res;
  float temperature, humidity;

  Sensor_temperature = Sensor_humidity = 0.0f; /* init */
#if PL_CONFIG_USE_SHT31
  McuSHT31_Init();
#elif PL_CONFIG_USE_SHT40
  McuSHT40_Init();
#endif
  for(;;) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  #if PL_CONFIG_USE_SHT31
    res = McuSHT31_ReadTempHum(&temperature, &humidity);
  #elif PL_CONFIG_USE_SHT40
    res = McuSHT40_ReadTempHum(&temperature, &humidity);
  #endif
    if (res==ERR_OK) {
      Sensor_temperature = temperature;
      Sensor_humidity = humidity;
    } else {
      McuLog_fatal("error reading SHT sensor.");
      vTaskDelay(pdMS_TO_TICKS(5000));
    }
  }
}

void Sensor_Deinit(void) {  //  GCOVR_EXCL_LINE
  /* todo */                //  GCOVR_EXCL_LINE
}                           //  GCOVR_EXCL_LINE

void Sensor_Init(void) {
  BaseType_t res;

  res = xTaskCreate(sensorTask, "sensor", (4*1024)/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+1, NULL);
  if (res!=pdPASS) {
    /* error! */
    for(;;) {}  // GCOVR_EXCL_LINE
  }
}
#endif /* PL_CONFIG_USE_SENSOR */
