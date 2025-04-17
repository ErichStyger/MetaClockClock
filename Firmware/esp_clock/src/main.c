/*
 * Copyright (c) 2023, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "McuLib.h"

#if McuLib_CONFIG_CPU_IS_RPxxxx
#include "application.h"

int main(void) {
  PL_Init();
  vTaskStartScheduler();
  for(;;) {
    /* shall not get here */
  }
  return 0;
}
#elif McuLib_CONFIG_CPU_IS_ESP32
#include <stdio.h>
#include "McuRTOS.h"
#include "sdkconfig.h"
#include "esp_chip_info.h"
#include "esp_flash.h"

void app_main(void) {

  /* Print chip information */
  esp_chip_info_t chip_info;
  uint32_t flash_size;
  esp_chip_info(&chip_info);
  printf("This is %s chip with %d CPU core(s), WiFi%s%s%s, ",
     CONFIG_IDF_TARGET,
     chip_info.cores,
     (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
     (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
     (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

  unsigned major_rev = chip_info.revision / 100;
  unsigned minor_rev = chip_info.revision % 100;
  printf("silicon revision v%d.%d, ", major_rev, minor_rev);
  if (esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
    printf("Get flash size failed");
    return;
  }
  printf("%lu MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
         (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
  printf("Minimum free heap size: %lu bytes\n", esp_get_minimum_free_heap_size());

#if 0
  for (int i = 10; i >= 0; i--) {
    printf("Restarting in %d seconds...\n", i);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  printf("Restarting now.\n");
  fflush(stdout);
  esp_restart();
#elif 1
  PL_Init();
  for(;;) { /* staying here, otherwise this task gets deleted */
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
#endif
}
#endif
