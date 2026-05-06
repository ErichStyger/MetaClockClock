/*
 * Copyright (c) 2019-2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NVMC_H_
#define NVMC_H_

#include "platform.h"
#include <stdint.h>
#include <stdbool.h>
#include "McuShell.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NVMC_FLAGS_MAGNET_ENABLED   (1<<0)  /* if magnets are present on hands or not */

/*!
 * \brief Gets whether a matrix coordinate is enabled in non-volatile configuration.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool NVMC_GetIsEnabled(uint8_t x, uint8_t y, uint8_t z);

/*!
 * \brief Stores whether a matrix coordinate is enabled in non-volatile configuration.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \param isEnabled Stored enabled state.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NVMC_SetIsEnabled(uint8_t x, uint8_t y, uint8_t z, bool isEnabled);

/*!
 * \brief Gets the stored zero offset for a stepper.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \return Requested numeric value.
 */
int16_t NVMC_GetStepperZeroOffset(uint8_t x, uint8_t y, uint8_t z);

/*!
 * \brief Stores the zero offset for a stepper.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \param offset Stepper zero offset value.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NVMC_SetStepperZeroOffset(uint8_t x, uint8_t y, uint8_t z, int16_t offset);

/*!
 * \brief Gets the stored RS-485 address.
 * \param addr RS-485 address value or destination buffer.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NVMC_GetRS485Addr(uint8_t *addr);

/*!
 * \brief Stores the RS-485 address.
 * \param addr RS-485 address value or destination buffer.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NVMC_SetRS485Addr(uint8_t addr);

/*!
 * \brief Gets the stored non-volatile configuration flags.
 * \param flags Configuration flags value or destination buffer.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NVMC_GetFlags(uint32_t *flags);

/*!
 * \brief Stores the non-volatile configuration flags.
 * \param flags Configuration flags value or destination buffer.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NVMC_SetFlags(uint32_t flags);

/*!
 * \brief Parses non-volatile memory configuration shell commands.
 * \param cmd Command string to parse or send.
 * \param handled Set to true when the command has been handled.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NVMC_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*!
 * \brief Deinitializes non-volatile memory configuration support.
 */
void NVMC_Deinit(void);

/*!
 * \brief Initializes non-volatile memory configuration support.
 */
void NVMC_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* NVMC_H_ */
