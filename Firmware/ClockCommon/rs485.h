/*
 * Copyright (c) 2019-2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RS485_H_
#define RS485_H_

#include <stdbool.h>
#include <stdint.h>
#include "McuShell.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! special pre-defined node addresses */
#define RS485_BROADCAST_ADDRESS (0x00)
  /*!< special broadcast address */
#define RS485_ILLEGAL_ADDRESS   (0xff)
  /*!< illegal/initialization value */

/*!
 * \brief Sends a command to an RS-485 destination and waits for the expected response.
 * \param dstAddr Destination RS-485 address.
 * \param cmd Command string to parse or send.
 * \param timeoutMs Timeout in milliseconds.
 * \param nofRetry Number of send retries.
 * \param shellIO Shell I/O streams used for command output.
 * \param rsIO Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t RS485_SendCommand(uint8_t dstAddr, const unsigned char *cmd, int32_t timeoutMs, uint32_t nofRetry, McuShell_ConstStdIOType *shellIO, McuShell_ConstStdIOType *rsIO);

/*!
 * \brief Gets the local RS-485 address.
 * \return Requested 8-bit value.
 */
uint8_t RS485_GetAddress(void);

/*!
 * \brief Parses RS-485 shell commands.
 * \param cmd Command string to parse or send.
 * \param handled Set to true when the command has been handled.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t RS485_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*!
 * \brief Deinitializes RS-485 communication.
 */
void RS485_Deinit(void);

/*!
 * \brief Initializes RS-485 communication.
 */
void RS485_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* RS485_H_ */
