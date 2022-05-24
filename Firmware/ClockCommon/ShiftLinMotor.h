/*
 * Copyright (c) 2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef SHIFTLINMOTOR_H_
#define SHIFTLINMOTOR_H_

#include <stdint.h>

#define SHIFT_LIN_MOTOR_CONFIG_USE_FREERTOS_HEAP   (1)

typedef struct ShiftLinMotor_Config_t {
  uint16_t shiftPos; /* position of motor in shift register, starting with zero */
} ShiftLinMotor_Config_t;

void ShiftLinMotor_GetDefaultConfig(ShiftLinMotor_Config_t *config);

typedef void *ShiftLinMotor_Handle_t;

ShiftLinMotor_Handle_t ShiftLinMotor_InitDevice(ShiftLinMotor_Config_t *config);

#endif /* SHIFTLINMOTOR_H_ */
