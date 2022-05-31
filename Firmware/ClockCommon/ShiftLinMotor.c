/*
 * Copyright (c) 2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "ShiftLinMotor.h"
#include "McuRTOS.h"
#include "shiftreg.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

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

void ShiftLinMotor_SingleStep(void *dev, int step) {
	ShiftLinMotor_Device_t *device = (ShiftLinMotor_Device_t *)dev;
	bool w[3];

	w[0] = true; /* clk */
	w[1] = step<0; /* dir */
	w[2] = true; /* stby, low active */
	if (step<0) {
		step = -step;
	}
	ShiftReg_StoreMotorBits(device->shiftPos, w);
}

void ShiftLinMotor_Execute(void) {
	ShiftReg_SendStoredMotorBitsIfChangedAutoClk();
}

void ShiftLinMotor_Stby(void *dev) {
	ShiftLinMotor_Device_t *device = (ShiftLinMotor_Device_t *)dev;
	ShiftReg_StoreMotorStbyBit(device->shiftPos);
}

void ShiftLinMotor_StbyAll(void) {
	ShiftReg_StoreMotorStbyBitsAll();
	/*ShiftReg_SendStoredMotorBitsIfChanged();*/
}

