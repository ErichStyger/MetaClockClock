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

#define NVMC_FLAGS_MAGNET_ENABLED   (1<<0)  /* if magnets are enabled or not */

/*!
 * \brief Provide the id/label of the stepper motor
 * \param motorIdx Motor index
 * \param id Where the id gets stored
 * \return Error code, ERR_OK for everything ok
 */
uint8_t NVMC_GetStepperID(uint8_t motorIdx, int16_t *id);
/*!
 * \brief Set the id/label of the stepper motor
 * \param motorIdx Motor index
 * \return Error code, ERR_OK for everything ok
 */
uint8_t NVMC_SetStepperID(uint8_t motorIdx, uint16_t id);

/*!
 * \brief Provide the offset from the magnet position for a stepper motor
 * \param motorIdx Motor index
 * \return Stored offset
 */
int16_t NVMC_GetStepperZeroOffset(uint8_t motorIdx);

/*!
 * \brief Store the offset from the magnet position for a stepper motor
 * \param motorIdx Motor index
 * \return Error code, ERR_OK for everything ok
 */
uint8_t NVMC_SetStepperZeroOffset(uint8_t motorIdx, int16_t offset);

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
 * \brief Return for a given motor the character it is responsible for.
 * \param motorIdx Motor index number
 * \param xp Where to store the x coordinate, returns -1 for not attached motor
 * \param yp Where to store the y coordinate, returns -1 for not attached motor
 */
uint8_t NVMC_GetStepperPosition(uint8_t motorIdx, int16_t *xp, int16_t *yp);

/*!
 * \brief Set position for a given motor the character it is responsible for.
 * \param motorIdx Motor index number
 * \param xp Where to store the x coordinate, returns -1 for not attached motor
 * \param yp Where to store the y coordinate, returns -1 for not attached motor
 */
uint8_t NVMC_SetStepperPosition(uint8_t motorIdx, int16_t xPos, int16_t yPos);

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

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* NVMC_H_ */
