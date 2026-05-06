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

/*!
 * \brief Fills a linear shift motor configuration with default values.
 * \param config Configuration structure to fill or use.
 */
void ShiftLinMotor_GetDefaultConfig(ShiftLinMotor_Config_t *config);

typedef void *ShiftLinMotor_Handle_t;

/*!
 * \brief Initializes a linear shift motor device instance.
 * \param config Configuration structure to fill or use.
 * \return Requested handle.
 */
ShiftLinMotor_Handle_t ShiftLinMotor_InitDevice(ShiftLinMotor_Config_t *config);

/*!
 * \brief Stores one step pattern for a linear shift motor.
 * \param dev Underlying device pointer.
 * \param step Step pattern index to store.
 */
void ShiftLinMotor_SingleStep(void *dev, int step);

/*!
 * \brief Transfers the stored linear shift motor bits to the hardware.
 */
void ShiftLinMotor_Execute(void);

/*!
 * \brief Puts one linear shift motor device into standby.
 * \param dev Underlying device pointer.
 */
void ShiftLinMotor_Stby(void *dev);

/*!
 * \brief Puts all linear shift motor devices into standby.
 */
void ShiftLinMotor_StbyAll(void);

#endif /* SHIFTLINMOTOR_H_ */
