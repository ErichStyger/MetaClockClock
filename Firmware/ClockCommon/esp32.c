/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_ESP32
#include "esp32.h"
#include "McuGPIO.h"

static McuGPIO_Handle_t ESP_BL, ESP_RES;

/*
 * UART1:
 * Tx to ESP: PTE0
 * Rx from ESP: PTE1
 * BL: PTC9
 * RES: PTC8
 */

void ESP_Init(void) {
  McuGPIO_Config_t config;

  McuGPIO_GetDefaultConfig(&config);
  config.hw.gpio = GPIOC;
  config.hw.port = PORTC;
  config.hw.pin = 9U;
  config.isInput = false;
  config.isHighOnInit = true;
  ESP_BL = McuGPIO_InitGPIO(&config);

  config.hw.gpio = GPIOC;
  config.hw.port = PORTC;
  config.hw.pin = 8U;
  config.isInput = false;
  config.isHighOnInit = true;
  ESP_RES = McuGPIO_InitGPIO(&config);
}

#endif /* PL_CONFIG_USE_ESP32 */
