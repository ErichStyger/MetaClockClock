/*
 * Copyright (c) 2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Note: Mutex to access the bus is implemented on the higher level.
 */

#ifndef SPIREG_H_
#define SPIREG_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Sends a block of data to the bus
 * \param data Pointer to the data to be sent
 * \param size Data size in bytes
 */
void SpiReg_WriteData(const uint8_t *data, size_t size);

/*!
 * \brief Reads a block of data from the bus
 * \param data Pointer to where the data gets stored
 * \param size Data size in bytes
 */
void SpiReg_ReadData(uint8_t *data, size_t size);

/*!
 * \brief Module de-initialization
 */
void SpiReg_Deinit(void);

/*!
 * \brief Module initialization
 */
void SpiReg_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* SPIREG_H_ */
