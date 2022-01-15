/*
 * Copyright (c) 2019, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NVMC_H_
#define NVMC_H_

#include "platform.h"
#include <stdint.h>
#include <stdbool.h>
#include "McuShell.h"

#define NVMC_FLAGS_MAGNET_ENABLED   (1<<0)  /* if magnets are present on hands or not */

/*!
 * \brief Getter for the 'enabled' setting of a motor
 * \param x Motor x position
 * \param y Motor y position
 * \param z Motor z position
 * \return If motor is enabled
 */
bool NVMC_GetIsEnabled(uint8_t x, uint8_t y, uint8_t z);

/*!
 * \brief Setter for the 'enabled' setting of a motor
 * \param x Motor x position
 * \param y Motor y position
 * \param z Motor z position
 * \return Error code, ERR_OK for everything ok
 */
uint8_t NVMC_SetIsEnabled(uint8_t x, uint8_t y, uint8_t z, bool isEnabled);

/*!
 * \brief Provide the offset from the magnet position for a stepper motor
 * \param x Motor x position
 * \param y Motor y position
 * \param z Motor z position
 * \return Stored offset
 */
int16_t NVMC_GetStepperZeroOffset(uint8_t x, uint8_t y, uint8_t z);

/*!
 * \brief Store the offset from the magnet position for a stepper motor
 * \param x Motor x position
 * \param y Motor y position
 * \param z Motor z position
 * \param offset Offset to store
 * \return Error code, ERR_OK for everything ok
 */
uint8_t NVMC_SetStepperZeroOffset(uint8_t x, uint8_t y, uint8_t z, int16_t offset);

/*!
 * \brief Getter to get the RS-485 address
 * \param addr Where to store the address
 * \return Error code, ERR_OK for everything ok
 */
uint8_t NVMC_GetRS485Addr(uint8_t *addr);

/*!
 * \brief Setter to set the RS-485 address.
 * \param addr Address to store
 * \return Error code, ERR_OK for everything ok
 */
uint8_t NVMC_SetRS485Addr(uint8_t addr);

/*!
 * \brief Getter to get the flags stored
 * \flags Where to store the flags
 * \return Error code, ERR_OK for everything ok
 */
uint8_t NVMC_GetFlags(uint32_t *flags);

/*!
 * \brief Setter to set the flags in the configuration
 * \param flags The flags to be set
 * \return Error code, ERR_OK for everything ok
 */
uint8_t NVMC_SetFlags(uint32_t flags);

/*! \brief Command line shell parser interface */
uint8_t NVMC_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*! \brief Module de-initialization */
void NVMC_Deinit(void);

/*! \brief Module initialization */
void NVMC_Init(void);

#endif /* NVMC_H_ */
