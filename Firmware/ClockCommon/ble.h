/*
 * Copyright (c) 2020, Erich Styger
  *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BLE_H_
#define BLE_H_

#include "McuShell.h"

/*!
 * \brief Parses BLE shell commands.
 * \param cmd Command string to parse or send.
 * \param handled Set to true when the command has been handled.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t BLE_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/*!
 * \brief Sends one SDEP packet to the Bluefruit module over SPI.
 * \param command Command string to parse or send.
 * \param buf Data or text buffer.
 * \param count Number of payload bytes to send.
 * \param more_data Non-zero if more SDEP packets follow this one.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool BLE_sendPacket(uint16_t command, const uint8_t *buf, uint8_t count, uint8_t more_data);

/*!
 * \brief Reads pending SDEP response packets from the Bluefruit module into the receive buffer.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool BLE_getResponse(void);

/*!
 * \brief Enables or disables command echo on the BLE module.
 * \param on True to enable or turn on, false to disable or turn off.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t BLE_Echo(bool on);

/*!
 * \brief Checks whether the BLE module reports an active connection.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool BLE_IsConnected(void);

/*!
 * \brief Sends an AT command and checks the received response tail.
 * \param cmd Command string to parse or send.
 * \param rxBuf Receive buffer for command response text.
 * \param rxBufSize Size of the buffer in bytes.
 * \param expectedTailStr Expected response suffix string.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t BLE_SendATCommandExpectedResponse(const uint8_t *cmd, uint8_t *rxBuf, size_t rxBufSize, const uint8_t *expectedTailStr);

/*!
 * \brief Initializes the BLE module.
 */
void BLE_Init(void);

#endif /* BLE_H_ */
