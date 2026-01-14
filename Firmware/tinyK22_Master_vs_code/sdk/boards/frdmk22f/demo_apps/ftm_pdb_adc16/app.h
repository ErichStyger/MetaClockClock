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
#define DEMO_ADC_BASE0          ADC0
#define DEMO_ADC_BASE1          ADC1
#define DEMO_ADC_USER_CHANNEL1  1U
#define DEMO_ADC_USER_CHANNEL26 26U /* Temperature sensor. */
#define DEMO_ADC_USER_CHANNEL7  7U
#define DEMO_ADC_CHANNEL_GROUP0 0U
#define DEMO_ADC_CHANNEL_GROUP1 1U
#define DEMO_ADC_CLOCK_SOURCE   kADC16_ClockSourceAlt2
#define DEMO_ADC_CLOCK_DIVIDER  kADC16_ClockDivider4

#define DEMO_ADC_IRQ_ID0      ADC0_IRQn
#define DEMO_ADC_IRQ_ID1      ADC1_IRQn
#define DEMO_ADC_IRQ_HANDLER0 ADC0_IRQHandler
#define DEMO_ADC_IRQ_HANDLER1 ADC1_IRQHandler

#define DEMO_PDB_BASE                     PDB0
#define DEMO_PDB_TRIGGER_CHANNEL0         kPDB_ADCTriggerChannel0
#define DEMO_PDB_TRIGGER_CHANNEL1         kPDB_ADCTriggerChannel1
#define DEMO_PDB_PRETRIGGER_CHANNEL0_MASK 1U
#define DEMO_PDB_PRETRIGGER_CHANNEL1_MASK 2U

#define DEMO_FTM_BASE FTM0

#define FTM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_CoreSysClk)
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
/*${prototype:end}*/

#endif /* _APP_H_ */
