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
#include "McuWait.h"
#include "McuUtility.h"
#include "Shell.h"

typedef struct {
  McuGPIO_Handle_t en; /* EN/FAULT, input/output, EN: power stage is turned off with LOW. IC pulls it low in case of fault. */
  McuGPIO_Handle_t stby; /* STBY/RESET, if LOW device is forced into low consumption mode */
  McuGPIO_Handle_t dir; /* direction */
  McuGPIO_Handle_t step; /* step */
} STS220_Device;

typedef void *STS220_DeviceHandle;


bool STS220_HasFault(STS220_DeviceHandle device) {
  STS220_Device *dev = (STS220_Device*)device;
  bool hasFault;
  bool prevEnValue;

  prevEnValue = McuGPIO_GetValue(dev->en); /* get current value */
  McuGPIO_SetAsInput(dev->en);
  hasFault = !McuGPIO_GetValue(dev->en); /* LOW: fault condition */
  McuGPIO_SetAsOutput(dev->en, prevEnValue);
  return hasFault;
}

void STS220_SetEnable(STS220_DeviceHandle device, bool enable) {
  STS220_Device *dev = (STS220_Device*)device;
  McuGPIO_SetValue(dev->en, enable); /* HIGH: enable device, LOW: Disable device */
}

void STS220_SetStandby(STS220_DeviceHandle device, bool standby) {
  STS220_Device *dev = (STS220_Device*)device;
  McuGPIO_SetValue(dev->stby, !standby); /* LOW: put device into standby mode, HIGH leave low-power mode */
}

void STS220_SetDir(STS220_DeviceHandle device, bool fw) {
  STS220_Device *dev = (STS220_Device*)device;
  McuGPIO_SetValue(dev->dir, fw);
}

void STS220_DoSteps(STS220_DeviceHandle device, int nof) {
  STS220_Device *dev = (STS220_Device*)device;

  while(nof>0) {
    McuGPIO_SetHigh(dev->step);
    McuWait_Waitms(10);
    McuGPIO_SetLow(dev->step);
    McuWait_Waitms(10);
    nof--;
  }
}

static STS220_Device sts220;

static void STS220_Init(void) {
  McuGPIO_Config_t config;

  CLOCK_EnableClock(kCLOCK_PortA);
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

  /* PTC3: DIR */
  config.hw.gpio = GPIOC;
  config.hw.port = PORTC;
  config.hw.pin = 3;
  config.isInput = false;
  config.isHighOnInit = false;
  sts220.dir = McuGPIO_InitGPIO(&config);

  /* PTA1: STEP */
  config.hw.gpio = GPIOA;
  config.hw.port = PORTA;
  config.hw.pin = 1;
  config.isInput = false;
  config.isHighOnInit = false;
  sts220.step = McuGPIO_InitGPIO(&config);
}

static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  McuShell_SendStatusStr((unsigned char*)"app", (unsigned char*)"Application status information\r\n", io->stdOut);
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"app", (unsigned char*)"Group of application commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
  return ERR_OK;
}

uint8_t APP_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "app help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "app status")==0)) {
    *handled = TRUE;
    return PrintStatus(io);
  }
  return ERR_OK;
}

void APP_Run(void) {
  int nofSteps = 10;

  McuLib_Init();
  McuWait_Init();
  McuGPIO_Init();
  McuLog_Init();
  STS220_Init();
  SHELL_Init();

  vTaskStartScheduler();

  STS220_SetStandby(&sts220, true);
  STS220_SetStandby(&sts220, false);

  STS220_SetEnable(&sts220, false);
  STS220_SetEnable(&sts220, true);

  STS220_SetDir(&sts220, true);
  STS220_DoSteps(&sts220, nofSteps);

  STS220_SetDir(&sts220, false);
  STS220_DoSteps(&sts220, nofSteps);

  STS220_SetStandby(&sts220, true);
  STS220_SetEnable(&sts220, false);

  bool hasFault = STS220_HasFault(&sts220);
  for(;;) {
  }
}
