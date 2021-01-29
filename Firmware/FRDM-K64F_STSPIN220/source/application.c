/*
 * application.c
 *
 * Copyright (c) 2021: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#include "application.h"
#include "McuLib.h"
#include "McuGPIO.h"
#include "McuLog.h"

typedef struct {
  McuGPIO_Handle_t en; /* EN/FAULT, input/output, EN: power stage is turned off with LOW */
  McuGPIO_Handle_t stby; /* STBY/RESET, if LOW device is forced into low consumption mode */
} STS220_Device;

typedef void *STS220_DeviceHandle;

void STS220_SetEnable(STS220_DeviceHandle device, bool enable) {
  STS220_Device *dev = (STS220_Device*)device;
  McuGPIO_SetValue(dev->en, enable); /* HIGH: enable device */
}

void STS220_SetStandby(STS220_DeviceHandle device, bool standby) {
  STS220_Device *dev = (STS220_Device*)device;
  McuGPIO_SetValue(dev->stby, !standby); /* LOW: put device into standby mode */
}

static STS220_Device sts220;

static void STS220_Init(void) {
  McuGPIO_Config_t config;

  CLOCK_EnableClock(kCLOCK_PortB);
  CLOCK_EnableClock(kCLOCK_PortC);

  McuGPIO_GetDefaultConfig(&config);

  /* PTB9: EN */
  config.hw.gpio = GPIOB;
  config.hw.port = PORTB;
  config.hw.pin = 9;
  config.isInput = false;
  config.isHighOnInit = false; /* default LOW: power stage disabled */
  sts220.en = McuGPIO_InitGPIO(&config);

  /* PTC12: STBY */
  config.hw.gpio = GPIOC;
  config.hw.port = PORTC;
  config.hw.pin = 12;
  config.isInput = false;
  config.isHighOnInit = false; /* default LOW: device in standby mode */
  sts220.stby = McuGPIO_InitGPIO(&config);

}

void APP_Run(void) {
  McuLib_Init();
  McuGPIO_Init();
  McuLog_Init();
  STS220_Init();

  STS220_SetEnable(&sts220, false);
  STS220_SetEnable(&sts220, true);

  STS220_SetStandby(&sts220, true);
  STS220_SetStandby(&sts220, false);

  for(;;) {

  }
}
