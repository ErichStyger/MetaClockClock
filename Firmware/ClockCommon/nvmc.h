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

#define NVMC_VERSION_1_0  (10) /* initial version */
#define NVMC_VERSION_1_1  (11) /* added nofActiveMotors */

#define NVMC_CURRENT_VERSION    NVMC_VERSION_1_1 /* active and current version */

#define NVMC_FLAGS_MAGNET_ENABLED   (1<<0)  /* if magnets are present on hands or not */
typedef struct {
  uint32_t version; /* NVMC_CURRENT_VERSION, must be 32bit type to have struct 4 byte aligned! */
  uint8_t addrRS485; /* device address on the RS-485 bus */
  uint8_t nofActiveMotors; /* used for the modular clock boards to define the number of active clocks */
  uint32_t flags;          /* various flags */
  int16_t zeroOffsets[PL_CONFIG_NOF_STEPPER_ON_BOARD_X][PL_CONFIG_NOF_STEPPER_ON_BOARD_Y][PL_CONFIG_NOF_STEPPER_ON_BOARD_Z]; /* two offsets for each motor, offset from the magnet sensor to the zero position */
  /* fill up to 64 bytes, needed for flash programming! */
  uint8_t filler[64-4-1-1-4-4-(PL_CONFIG_NOF_STEPPER_ON_BOARD_X*PL_CONFIG_NOF_STEPPER_ON_BOARD_Y*PL_CONFIG_NOF_STEPPER_ON_BOARD_Z*2)];
} NVMC_Data_t;

/*!
 * \brief Returns a pointer to the data or NULL if the data is not present (erased)
 */
const NVMC_Data_t *NVMC_GetDataPtr(void);

uint8_t NVMC_WriteConfig(NVMC_Data_t *data);

int16_t NVMC_GetStepperZeroOffset(uint8_t x, uint8_t y, uint8_t z);

uint8_t NVMC_GetRS485Addr(void);

uint8_t NVMC_GetNofActiveMotors(void);

uint8_t NVMC_GetNofActiveMotors(void);

uint32_t NVMC_GetFlags(void);

bool NVMC_IsErased(void);

uint8_t NVMC_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

bool NVMC_HasValidConfig(void);

void NVMC_Deinit(void);
void NVMC_Init(void);

#endif /* NVMC_H_ */
