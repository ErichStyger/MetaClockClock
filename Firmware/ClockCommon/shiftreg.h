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
 * \brief Reads sensor bits through the shift register chain.
 * \param data Data buffer.
 * \param dataSize Number of bytes in the data buffer.
 */
void ShiftReg_ReadSensorBits(uint8_t *data, size_t dataSize);
#endif

/*!
 * \brief Writes motor bits through the shift register chain.
 * \param data Data buffer.
 * \param nofBytes Number of bytes in the data buffer.
 */
void ShiftReg_WriteMotorBits(const uint8_t *data, size_t nofBytes);

/*!
 * \brief Stores the motor phase bits for one motor.
 * \param index Motor index.
 * \param w Motor phase bit array or width value.
 */
void ShiftReg_StoreMotorBits(uint32_t index, const bool w[SHIFTREG_NOF_MOTOR_BITS]);

/*!
 * \brief Stores the standby bit for one motor.
 * \param motorIdx Motor index.
 */
void ShiftReg_StoreMotorStbyBit(uint32_t motorIdx);

/*!
 * \brief Stores standby bits for all motors.
 */
void ShiftReg_StoreMotorStbyBitsAll(void);

/*!
 * \brief Sends the stored motor bits to the shift registers.
 */
void ShiftReg_SendStoredMotorBits(void);

/*!
 * \brief Sends stored motor bits and automatically clocks them out.
 */
void ShiftReg_SendStoredMotorBitsAutoClk(void);

/*!
 * \brief Sends stored motor bits only if they changed.
 */
void ShiftReg_SendStoredMotorBitsIfChanged(void);

/*!
 * \brief Sends changed stored motor bits and automatically clocks them out.
 */
void ShiftReg_SendStoredMotorBitsIfChangedAutoClk(void);

/*!
 * \brief Parses shift-register shell commands.
 * \param cmd Command string to parse or send.
 * \param handled Set to true when the command has been handled.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t ShiftReg_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*!
 * \brief Deinitializes shift-register support.
 */
void ShiftReg_Deinit(void);

/*!
 * \brief Initializes shift-register support.
 */
void ShiftReg_Init(void);

#endif /* PL_CONFIG_USE_SHIFT_REGS */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* SHIFTREG_H_ */
