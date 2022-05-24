/*
 * Copyright (c) 2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "ShiftLinMotor.h"
#include "McuRTOS.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct ShiftLinMotor_Device_t {
  int16_t shiftPos;
} ShiftLinMotor_Device_t;

void ShiftLinMotor_GetDefaultConfig(ShiftLinMotor_Config_t *config) {
  config->shiftPos = 0;
}

ShiftLinMotor_Handle_t ShiftLinMotor_InitDevice(ShiftLinMotor_Config_t *config) {
  ShiftLinMotor_Device_t *handle;

#if SHIFT_LIN_MOTOR_CONFIG_USE_FREERTOS_HEAP
  handle = (ShiftLinMotor_Device_t*)pvPortMalloc(sizeof(ShiftLinMotor_Device_t)); /* get a new device descriptor */
#else
  handle = (ShiftLinMotor_Device_t*)malloc(sizeof(ShiftLinMotor_Device_t)); /* get a new device descriptor */
#endif
  assert(handle!=NULL);
  if (handle!=NULL) { /* if malloc failed, will return NULL pointer */
    memset(handle, 0, sizeof(ShiftLinMotor_Device_t)); /* init all fields */
    handle->shiftPos = config->shiftPos;
  }
  return handle;
}
