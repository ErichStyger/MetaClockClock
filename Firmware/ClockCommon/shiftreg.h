/*
 * Copyright (c) 2022-2023, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SHIFTREG_H_
#define SHIFTREG_H_

#include "platform.h"
#if PL_CONFIG_USE_SHIFT_REGS
#include "shiftreg_config.h"
#include "McuGPIO.h"
#include "McuULN2003.h"
#include "McuShell.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#if 0 /* not used */
#define SHIFTREG_NOF_SENSORBIT_BYTES   ((((SHIFTREG_CONFIG_NOF_DRIVER_BOARDS*SHIFTREG_CONFIG_NOF_MOTORS_PER_DRIVER_BOARD)-1)/8)+1)
  /*!< number of bytes for the sensor bits */
#endif

#define SHIFTREG_NOF_MOTORS (SHIFTREG_CONFIG_NOF_DRIVER_BOARDS*SHIFTREG_CONFIG_NOF_MOTORS_PER_DRIVER_BOARD)
/*!< Number of motors connected to the shift registers */

#define SHIFTREG_NOF_MOTOR_BITS  (3)
  /*!< Number of bits to be shifted for each motor: for each motor STBY (bit 0), DIR (bit 1), STCK (bit 2) */

#define SHIFTREG_NOF_MOTORBIT_BYTES    ((((SHIFTREG_CONFIG_NOF_DRIVER_BOARDS*SHIFTREG_CONFIG_NOF_MOTORS_PER_DRIVER_BOARD*SHIFTREG_NOF_MOTOR_BITS)-1)/8)+1)
  /*!< number of bytes for the sensor bits */

#if 0 /* not used */
/*!
 * \brief Read sensor (magnet) bits from the shift register(s)
 * \param data Pointer where to store the data, one bit for each sensor
 * \param dataSize Number of data bytes
 */
void ShiftReg_ReadSensorBits(uint8_t *data, size_t dataSize);
#endif

/*!
 * \brief Send motor (coil) bits to the shift register(s)
 * \param data Array of data, one bit for each motor coil
 * \param nofBytes Number of data bytes
 */
void ShiftReg_WriteMotorBits(const uint8_t *data, size_t nofBytes);

/*!
 * \brief Store the motor bits to be sent in global variable array. Send them later using ShiftReg_WriteMotorBits()
 * \param index Motor index number, starting with 0
 * \param w Array of truth table for the motor pins
 */
void ShiftReg_StoreMotorBits(uint32_t index, const bool w[SHIFTREG_NOF_MOTOR_BITS]);

/*!
 * \brief Set the motor specific stdby-bit  in global variable array and clear the clk-bit. Send them later using ShiftReg_WriteMotorBits()
 * \param index Motor index number, starting with 0
 */
void ShiftReg_StoreMotorStbyBit(uint32_t motorIdx);

/*!
 * \brief Set ALL stby-bits in global variable array and clear the clk-bits. Send them later using ShiftReg_WriteMotorBits()
 */
void ShiftReg_StoreMotorStbyBitsAll(void);

/*!
 * \brief Send the stored bits which has been sent by ShiftReg_StoreMotorBits(). Note that this function uses a mutex to access the SPI bus.
 */
void ShiftReg_SendStoredMotorBits(void);

/*!
 * \brief Send the stored bits which has been sent by ShiftReg_StoreMotorBits(), does auto clock toggling and sends the data again . Note that this function uses a mutex to access the SPI bus.
 */
void ShiftReg_SendStoredMotorBitsAutoClk(void);

/*!
 * \brief Send the motor bits, but only if they have changed from the last time they were sent.
 */
void ShiftReg_SendStoredMotorBitsIfChanged(void);

/*!
 * \brief Send the motor bits, does auto clock toggling and sends the data again, but only if they have changed from the last time they were sent and does Auto Clk.
 */
void ShiftReg_SendStoredMotorBitsIfChangedAutoClk(void);

/*!
 * \brief Command line parser for the ShiftReg module
 * \param cmd Command string to be parsed
 * \param handled If the command has been recognized
 * \param io Standard I/O to be used
 */
uint8_t ShiftReg_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*!
 * \brief Module de-initialization
 */
void ShiftReg_Deinit(void);

/*!
 * \brief Module initialization
 */
void ShiftReg_Init(void);

#endif /* PL_CONFIG_USE_SHIFT_REGS */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* SHIFTREG_H_ */
