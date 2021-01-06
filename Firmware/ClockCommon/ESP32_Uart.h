/*
 * ESP32_Uart.h
 *
 * Copyright (c) 2020: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#ifndef ESP32_UART_H_
#define ESP32_UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "McuShell.h"

extern uint8_t Esp32ShellUart_DefaultShellBuffer[McuShell_DEFAULT_SHELL_BUFFER_SIZE]; /* default buffer which can be used by the application */
extern McuShell_ConstStdIOType Esp32ShellUart_stdio;

void Esp32ShellUart_Deinit(void);
void Esp32ShellUart_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* ESP32_UART_H_ */
