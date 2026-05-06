/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCES_PIXELDMA_H_
#define SOURCES_PIXELDMA_H_

#include <stdint.h>

/*!
 * \brief Transfers pixel data using DMA.
 * \param dataAddress Start address of the data to transfer.
 * \param nofBytes Number of bytes in the data buffer.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t PIXDMA_Transfer(uint32_t dataAddress, size_t nofBytes);

/*!
 * \brief Initializes PixelDMA hardware support.
 */
void PIXDMA_Init(void);

#endif /* SOURCES_PIXELDMA_H_ */
