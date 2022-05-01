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
#include "matrix.h"

typedef struct STEPBOARD_Device_t {
  uint8_t addr;
  bool enabled;
#if PL_CONFIG_USE_MOTOR_ON_OFF
  bool isOn; /* if motor is powered */
  McuGPIO_Handle_t motorOnOff; /* LOW active for all motors */
#endif
  STEPPER_Handle_t stepper[PL_CONFIG_NOF_STEPPER_ON_BOARD_X][PL_CONFIG_NOF_STEPPER_ON_BOARD_Y][PL_CONFIG_NOF_STEPPER_ON_BOARD_Z];
#if PL_CONFIG_USE_NEO_PIXEL_HW
  NEOSR_Handle_t *ledRing[PL_CONFIG_NOF_STEPPER_ON_BOARD_X][PL_CONFIG_NOF_STEPPER_ON_BOARD_Y][PL_CONFIG_NOF_STEPPER_ON_BOARD_Z]; /* points to the LED ring device */
#endif
} STEPBOARD_Device_t;

/* default configuration, used for initializing the configuration */
static const STEPBOARD_Config_t defaultConfig =
{
  .addr = 0x0, .enabled = true,
  /* other fields are initialized implicitly with 0 or NULL */
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
    for(int x=0; x<PL_CONFIG_NOF_STEPPER_ON_BOARD_X; x++) {
      for(int y=0; y<PL_CONFIG_NOF_STEPPER_ON_BOARD_Y; y++) {
        for(int z=0; z<PL_CONFIG_NOF_STEPPER_ON_BOARD_Z; z++) {
          handle->stepper[x][y][z] = config->stepper[x][y][z];
        #if PL_CONFIG_USE_LED_RING
          handle->ledRing[x][y][z] = config->ledRing[x][y][z];
        #endif
        }
      }
    }
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
NEOSR_Handle_t STEPBOARD_GetStepperLedRing(STEPBOARD_Handle_t board, int x, int y, int z) {
  assert(x<PL_CONFIG_NOF_STEPPER_ON_BOARD_X && y<PL_CONFIG_NOF_STEPPER_ON_BOARD_Y && z<PL_CONFIG_NOF_STEPPER_ON_BOARD_Z);
  STEPBOARD_Device_t *handle = (STEPBOARD_Device_t*)board;
  return handle->ledRing[x][y][z];
}
#endif /* PL_CONFIG_USE_NEO_PIXEL_HW */

STEPPER_Handle_t STEPBOARD_GetStepper(STEPBOARD_Handle_t board, int x, int y, int z) {
  assert(x<PL_CONFIG_NOF_STEPPER_ON_BOARD_X && y<PL_CONFIG_NOF_STEPPER_ON_BOARD_Y && z<PL_CONFIG_NOF_STEPPER_ON_BOARD_Z);
  STEPBOARD_Device_t *handle = (STEPBOARD_Device_t*)board;
  return handle->stepper[x][y][z];
}

bool STEPBOARD_ItemsInQueue(STEPBOARD_Handle_t board) {
  STEPBOARD_Device_t *handle = (STEPBOARD_Device_t*)board;
  QueueHandle_t queue;

  for(int x=0; x<PL_CONFIG_NOF_STEPPER_ON_BOARD_X; x++) {
    for(int y=0; y<PL_CONFIG_NOF_STEPPER_ON_BOARD_Y; y++) {
      for(int z=0; z<PL_CONFIG_NOF_STEPPER_ON_BOARD_Z; z++) {
        queue = STEPPER_GetQueue(handle->stepper[x][y][z]);
        if (uxQueueMessagesWaiting(queue)!=0) { /* still things queued up? */
          return true; /* yes */
        }
      }
    }
  }
  return false; /* nothing in queues */
}

bool STEPBOARD_IsIdle(STEPBOARD_Handle_t board) {
  STEPBOARD_Device_t *handle = (STEPBOARD_Device_t*)board;

  for(int x=0; x<PL_CONFIG_NOF_STEPPER_ON_BOARD_X; x++) {
    for(int y=0; y<PL_CONFIG_NOF_STEPPER_ON_BOARD_Y; y++) {
      for(int z=0; z<PL_CONFIG_NOF_STEPPER_ON_BOARD_Z; z++) {
        if (!STEPPER_IsIdle(handle->stepper[x][y][z])) { /* still steps to do? */
          return false; /* yes, not finished */
        }
      }
    }
  }
  return true; /* finished */
}

void STEPBOARD_NormalizePosition(STEPBOARD_Handle_t board) {
  STEPBOARD_Device_t *handle = (STEPBOARD_Device_t*)board;

  for(int x=0; x<PL_CONFIG_NOF_STEPPER_ON_BOARD_X; x++) {
    for(int y=0; y<PL_CONFIG_NOF_STEPPER_ON_BOARD_Y; y++) {
      for(int z=0; z<PL_CONFIG_NOF_STEPPER_ON_BOARD_Z; z++) {
        STEPPER_NormalizePos(handle->stepper[x][y][z]);
      }
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

  dev = STEPPER_GetDevice(handle->stepper[0][0][0]); /* get the device handle: have one driver IC for two clocks */
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
