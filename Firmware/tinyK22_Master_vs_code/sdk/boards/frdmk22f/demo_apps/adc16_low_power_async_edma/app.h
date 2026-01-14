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
#define DEMO_ADC16_BASEADDR      ADC0
#define DEMO_ADC16_CHANNEL_GROUP 0U
#define DEMO_ADC16_CHANNEL       26U                     /* Temperature sensor. */
#define ADC16_RESULT_REG_ADDR    (uint32_t)(&ADC0->R[0]) /* Get adc16 result register address */

#define DEMO_DMA_BASEADDR   DMA0
#define DEMO_DMA_IRQ_ID     DMA0_IRQn
#define DEMO_DMA_CHANNEL    0U
#define DEMO_DMA_ADC_SOURCE kDmaRequestMux0ADC0

#define DEMO_DMAMUX_BASEADDR DMAMUX0

#define DEMO_LPTMR_BASE LPTMR0

#define LED_INIT()   LED_RED_INIT(LOGIC_LED_OFF)
#define LED_TOGGLE() LED_RED_TOGGLE()
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
void BOARD_ConfigTriggerSource(void);
/*${prototype:end}*/

#endif /* _APP_H_ */
