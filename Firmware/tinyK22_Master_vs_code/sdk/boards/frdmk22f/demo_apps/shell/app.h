/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
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
/*(${macro:start}*/
#define LED_NUMBERS  3U
#define LED_1_INIT() LED_RED_INIT(LOGIC_LED_OFF)
#define LED_2_INIT() LED_GREEN_INIT(LOGIC_LED_OFF)
#define LED_3_INIT() LED_BLUE_INIT(LOGIC_LED_OFF)
#define LED_1_ON()   LED_RED_ON()
#define LED_1_OFF()  LED_RED_OFF()
#define LED_2_ON()   LED_GREEN_ON()
#define LED_2_OFF()  LED_GREEN_OFF()
#define LED_3_ON()   LED_BLUE_ON()
#define LED_3_OFF()  LED_BLUE_OFF()
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
void Led_Init(void);
/*${prototype:end}*/
#endif
