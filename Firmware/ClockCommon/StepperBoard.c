/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_STEPPER
#include "StepperBoardConfig.h"
#include "StepperBoard.h"
#include "stepper.h"
#include "McuGPIO.h"
#include <stddef.h>
#include <string.h>
#include <assert.h>
#if STEPBOARD_CONFIG_USE_FREERTOS_HEAP
  #include "McuRTOS.h"
#endif
#if PL_CONFIG_USE_WDT
  #include "watchdog.h"
#endif
#include "Matrix.h"

typedef struct STEPBOARD_Device_t {
  uint8_t addr;
  bool enabled;
#if PL_CONFIG_USE_MOTOR_ON_OFF
  bool isOn; /* if motor is powered */
  McuGPIO_Handle_t motorOnOff; /* LOW active for all motors */
#endif
  STEPPER_Handle_t stepper[STEPPER_NOF_CLOCKS][STEPPER_NOF_CLOCK_MOTORS];
#if PL_CONFIG_USE_NEO_PIXEL_HW
  NEOSR_Handle_t *ledRing[STEPPER_NOF_CLOCKS][STEPPER_NOF_CLOCK_MOTORS]; /* points to the LED ring device */
#endif
} STEPBOARD_Device_t;

/* default configuration, used for initializing the configuration */
static const STEPBOARD_Config_t defaultConfig =
{
  .addr = 0x0, .enabled = true,
  .stepper[0][0] = NULL, .stepper[0][1] = NULL,
#if PL_CONFIG_USE_NEO_PIXEL_HW
  .ledRing[0][0] = NULL, .ledRing[0][1] = NULL,
#endif
#if STEPPER_NOF_CLOCKS >= 2
  .stepper[1][0] = NULL, .stepper[1][1] = NULL,
#if PL_CONFIG_USE_NEO_PIXEL_HW
  .ledRing[1][0] = NULL, .ledRing[1][1] = NULL,
#endif
#endif
#if STEPPER_NOF_CLOCKS >= 3
  .stepper[2][0] = NULL, .stepper[2][1] = NULL,
#if PL_CONFIG_USE_NEO_PIXEL_HW
  .ledRing[2][0] = NULL, .ledRing[2][1] = NULL,
#endif
#endif
#if STEPPER_NOF_CLOCKS >= 4
  .stepper[3][0] = NULL, .stepper[3][1] = NULL,
#if PL_CONFIG_USE_NEO_PIXEL_HW
  .ledRing[3][0] = NULL, .ledRing[3][1] = NULL,
#endif
#endif
};

void STEPBOARD_GetDefaultConfig(STEPBOARD_Config_t *config) {
  memcpy(config, &defaultConfig, sizeof(*config));
}

STEPBOARD_Handle_t STEPBOARD_InitDevice(STEPBOARD_Config_t *config) {
  STEPBOARD_Device_t *handle;

#if STEPBOARD_CONFIG_USE_FREERTOS_HEAP
  handle = (STEPBOARD_Device_t*)pvPortMalloc(sizeof(STEPBOARD_Device_t)); /* get a new device descriptor */
#else
  handle = (STEPBOARD_Device_t*)malloc(sizeof(STEPBOARD_Device_t)); /* get a new device descriptor */
#endif
  assert(handle!=NULL);
  if (handle!=NULL) { /* if malloc failed, will return NULL pointer */
    memset(handle, 0, sizeof(STEPBOARD_Device_t)); /* init all fields */
    handle->addr = config->addr;
    handle->enabled = config->enabled;
    handle->stepper[0][0] = config->stepper[0][0];
    handle->stepper[0][1] = config->stepper[0][1];
#if PL_CONFIG_USE_NEO_PIXEL_HW
    handle->ledRing[0][0] = config->ledRing[0][0];
    handle->ledRing[0][1] = config->ledRing[0][1];
#endif
#if STEPPER_NOF_CLOCKS >= 2
    handle->stepper[1][0] = config->stepper[1][0];
    handle->stepper[1][1] = config->stepper[1][1];
#if PL_CONFIG_USE_NEO_PIXEL_HW
    handle->ledRing[1][0] = config->ledRing[1][0];
    handle->ledRing[1][1] = config->ledRing[1][1];
#endif
#endif
#if STEPPER_NOF_CLOCKS >= 3
    handle->stepper[2][0] = config->stepper[2][0];
    handle->stepper[2][1] = config->stepper[2][1];
#if PL_CONFIG_USE_NEO_PIXEL_HW
    handle->ledRing[2][0] = config->ledRing[2][0];
    handle->ledRing[2][1] = config->ledRing[2][1];
#endif
#endif
#if STEPPER_NOF_CLOCKS >= 4
    handle->stepper[3][0] = config->stepper[3][0];
    handle->stepper[3][1] = config->stepper[3][1];
#if PL_CONFIG_USE_NEO_PIXEL_HW
    handle->ledRing[3][0] = config->ledRing[3][0];
    handle->ledRing[3][1] = config->ledRing[3][1];
#endif
#endif
  #if PL_CONFIG_USE_MOTOR_ON_OFF
    handle->motorOnOff = NULL; /* use STEPBOARD_SetMotorSwitch() to assign handle */
  #endif
  } else {
    for(;;) {
      __asm("nop");
    }
  }
  return handle;
}

uint8_t STEPBOARD_GetAddress(STEPBOARD_Handle_t board) {
  STEPBOARD_Device_t *handle = (STEPBOARD_Device_t*)board;
  return handle->addr;
}

bool STEPBOARD_IsEnabled(STEPBOARD_Handle_t board) {
  STEPBOARD_Device_t *handle = (STEPBOARD_Device_t*)board;
  return handle->enabled;
}

#if PL_CONFIG_USE_NEO_PIXEL_HW
NEOSR_Handle_t STEPBOARD_GetStepperLedRing(STEPBOARD_Handle_t board, int clock, int motor) {
  STEPBOARD_Device_t *handle = (STEPBOARD_Device_t*)board;
  return handle->ledRing[clock][motor];
}
#endif /* PL_CONFIG_USE_NEO_PIXEL_HW */

STEPPER_Handle_t STEPBOARD_GetStepper(STEPBOARD_Handle_t board, int clock, int motor) {
  STEPBOARD_Device_t *handle = (STEPBOARD_Device_t*)board;
  return handle->stepper[clock][motor];
}

bool STEPBOARD_ItemsInQueue(STEPBOARD_Handle_t board) {
  STEPBOARD_Device_t *handle = (STEPBOARD_Device_t*)board;
  QueueHandle_t queue;

  for(int i=0; i<STEPPER_NOF_CLOCKS; i++) {
    for(int j=0; j<STEPPER_NOF_CLOCK_MOTORS; j++) {
      queue = STEPPER_GetQueue(handle->stepper[i][j]);
      if (uxQueueMessagesWaiting(queue)!=0) { /* still things queued up? */
        return true; /* yes */
      }
    }
  }
  return false; /* nothing in queues */
}

bool STEPBOARD_IsIdle(STEPBOARD_Handle_t board) {
  STEPBOARD_Device_t *handle = (STEPBOARD_Device_t*)board;

  for(int i=0; i<STEPPER_NOF_CLOCKS; i++) {
    for(int j=0; j<STEPPER_NOF_CLOCK_MOTORS; j++) {
      if (!STEPPER_IsIdle(handle->stepper[i][j])) { /* still steps to do? */
        return false; /* yes, not finished */
      }
    }
  }
  return true; /* finished */
}

void STEPBOARD_NormalizePosition(STEPBOARD_Handle_t board) {
  STEPBOARD_Device_t *handle = (STEPBOARD_Device_t*)board;

  for(int i=0; i<STEPPER_NOF_CLOCKS; i++) {
    for(int j=0; j<STEPPER_NOF_CLOCK_MOTORS; j++) {
      STEPPER_NormalizePos(handle->stepper[i][j]);
    }
  }
}

void STEPBOARD_MoveAndWait(STEPBOARD_Handle_t board, uint32_t waitMs) {
  STEPPER_StartTimer();
  do {
    vTaskDelay(pdMS_TO_TICKS(waitMs));
#if PL_CONFIG_USE_WDT
    WDT_Report(WDT_REPORT_ID_CURR_TASK, waitMs);
#endif
  } while (!STEPBOARD_IsIdle(board));
}

STEPBOARD_Handle_t STEPBOARD_DeinitDevice(STEPBOARD_Handle_t device) {
#if STEPBOARD_CONFIG_USE_FREERTOS_HEAP
  vPortFree(device);
#else
  free(device);
#endif
  return NULL;
}

static STEPBOARD_Handle_t currBoard;
STEPBOARD_Handle_t STEPBOARD_GetBoard(void) {
  return currBoard;
}

void STEPBOARD_SetBoard(STEPBOARD_Handle_t board) {
  currBoard = board;
}

#if PL_CONFIG_USE_MOTOR_ON_OFF
bool STEPBOARD_IsMotorSwitchOn(STEPBOARD_Handle_t board) {
  STEPBOARD_Device_t *handle = (STEPBOARD_Device_t*)board;
  return handle->motorOnOff!=NULL && McuGPIO_IsLow(handle->motorOnOff); /* LOW Active */
}
#endif

#if PL_CONFIG_USE_MOTOR_ON_OFF
void STEPBOARD_SetMotorSwitch(STEPBOARD_Handle_t board, McuGPIO_Handle_t gpioHandle) {
  STEPBOARD_Device_t *handle = (STEPBOARD_Device_t*)board;
  handle->motorOnOff = gpioHandle;
}
#endif

#if PL_CONFIG_USE_MOTOR_ON_OFF
void STEPBOARD_MotorSwitchOnOff(STEPBOARD_Handle_t board, bool on) {
  STEPBOARD_Device_t *handle = (STEPBOARD_Device_t*)board;
  void *dev;

  dev = STEPPER_GetDevice(handle->stepper[0][0]); /* get the device handle: have one driver IC for two clocks */
  if (on) {
    MATRIX_EnableStepper(dev);
    McuGPIO_SetValue(handle->motorOnOff, false); /* the high-side switch is LOW-active */
  } else {
    MATRIX_DisableStepper(dev);
    McuGPIO_SetValue(handle->motorOnOff, true); /* the high-side switch is LOW-active */
  }
}
#endif

void STEPBOARD_Deinit(void) {
}

void STEPBOARD_Init(void) {
}

#endif /* PL_CONFIG_USE_STEPPER */
