/*
 * Copyright (c) 2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef SHIFTLINMOTOR_H_
#define SHIFTLINMOTOR_H_

#include <stdint.h>

#define SHIFT_LIN_MOTOR_CONFIG_USE_FREERTOS_HEAP   (1)

typedef struct ShiftLinMotor_Device_t {
  int16_t shiftPos;
} ShiftLinMotor_Device_t;

typedef struct ShiftLinMotor_Config_t {
  uint16_t shiftPos; /* position of motor in shift register, starting with zero */
} ShiftLinMotor_Config_t;

void ShiftLinMotor_GetDefaultConfig(ShiftLinMotor_Config_t *config);

typedef void *ShiftLinMotor_Handle_t;

ShiftLinMotor_Handle_t ShiftLinMotor_InitDevice(ShiftLinMotor_Config_t *config);

/*!
 * \brief Store the motor bits to do a single step. Send them later using ShiftLinMotor_Execute()
 * \param dev ShiftLinMotor_Device_t
 * \param step Positive value > direction is set to CW. | Negative value > direction is set to CCW.
 */
void ShiftLinMotor_SingleStep(void *dev, int step);

/*!
 * \brief Send the actual stored motor bits to the steppers.
 */
void ShiftLinMotor_Execute(void);

/*!
 * \brief Set the motor stdby bit of a specific motor. Send them later using ShiftLinMotor_Execute()
 * \param dev ShiftLinMotor_Device_t
 */
void ShiftLinMotor_Stby(void *dev);

/*!
 * \brief Set the motor stdby bits of all motors. Send them later using ShiftLinMotor_Execute()
 */
void ShiftLinMotor_StbyAll(void);

#endif /* SHIFTLINMOTOR_H_ */
