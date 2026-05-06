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
 * \brief Writes data through the SPI register chain.
 * \param data Data buffer.
 * \param size Number of bytes in the data buffer.
 */
void SpiReg_WriteData(const uint8_t *data, size_t size);

/*!
 * \brief Reads data through the SPI register chain.
 * \param data Data buffer.
 * \param size Number of bytes in the data buffer.
 */
void SpiReg_ReadData(uint8_t *data, size_t size);

/*!
 * \brief Deinitializes SPI register support.
 */
void SpiReg_Deinit(void);

/*!
 * \brief Initializes SPI register support.
 */
void SpiReg_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* SPIREG_H_ */
