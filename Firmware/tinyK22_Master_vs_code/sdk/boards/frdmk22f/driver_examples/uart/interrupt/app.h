/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_H_
#define _APP_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define DEMO_UART            UART1
#define DEMO_UART_CLKSRC     SYS_CLK
#define DEMO_UART_CLK_FREQ   CLOCK_GetFreq(SYS_CLK)
#define DEMO_UART_IRQn       UART1_RX_TX_IRQn
#define DEMO_UART_IRQHandler UART1_RX_TX_IRQHandler
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
/*${prototype:end}*/

#endif /* _APP_H_ */
