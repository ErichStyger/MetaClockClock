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
#define LED_INIT() LED_GREEN_INIT(LOGIC_LED_OFF)
#define LED_ON()   LED_GREEN_ON()
#define LED_OFF()  LED_GREEN_OFF()

#define DEMO_CMP_BASE         CMP0
#define DEMO_CMP_USER_CHANNEL 0U /* PTC6, CMP0_IN0, J1-16 on FRDM-K22F. */
#define DEMO_CMP_DAC_CHANNEL  7U
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
/*${prototype:end}*/

#endif /* _APP_H_ */
