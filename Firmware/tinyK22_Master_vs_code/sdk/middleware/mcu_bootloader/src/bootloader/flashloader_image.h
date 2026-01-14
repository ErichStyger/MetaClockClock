/*
 * Copyright (c) 2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#if !defined(__FLASHLOADER_IMAGE_H__)
#define __FLASHLOADER_IMAGE_H__

//! @addtogroup flashloader
//! @{

//! @name Flashloader globals
//@{

//! @brief Image of flashloader to load to RAM, in raw binary format.
extern const uint8_t g_flashloaderImage[];

//! @brief Size of flashloader image.
extern const uint32_t g_flashloaderSize;

//! @brief Base address in RAM of flashloader image.
extern const uint32_t g_flashloaderBase;

//! @brief Entry point of flashloader image.
extern const uint32_t g_flashloaderEntry;

//! @brief Stack limit of flashloader image.
extern const uint32_t g_flashloaderStack;

//@}

//! @}

#endif // __FLASHLOADER_IMAGE_H__
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
