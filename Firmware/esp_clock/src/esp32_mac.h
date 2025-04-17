/*
 * Copyright (c) 2021, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ESP32_MAC_H_
#define ESP32_MAC_H_

#include "platform.h"
#include "McuLib.h"
#if PL_CONFIG_USE_WIFI && McuLib_CONFIG_CPU_IS_ESP32

#include <stdint.h>
#include <stddef.h> /* for size_t */

#ifdef __cplusplus
extern "C" {
#endif

#define MAC_ADDR_SIZE 6

/*!
 * \brief transforms a binary MAC into a string, e.g. "d8:a0:1d:42:ed:50"
 * \param mac binary MAC
 * \param buf buffer where to store the string
 * \param bufSize size of buffer
 */
void ESP32_MacToString(uint8_t mac[MAC_ADDR_SIZE], uint8_t *buf, size_t bufSize);

/*!
 * \brief Read the MAC address into buffer
 * \param mac buffer where to store the MAC address
 */
uint8_t ESP32_MacRead(uint8_t mac[MAC_ADDR_SIZE]);

/*!
 * \brief Module initialization
 */
void ESP32_MacInit(void);

#endif /* PL_CONFIG_USE_IDENTIFY */

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* ESP32_MAC_H_ */
