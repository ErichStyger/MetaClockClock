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
#define DEMO_PDB_BASE        PDB0
#define DEMO_PDB_IRQ_ID      PDB0_IRQn
#define DEMO_PDB_IRQ_HANDLER PDB0_IRQHandler

#define DEMO_PDB_ADC_TRIGGER_CHANNEL    kPDB_ADCTriggerChannel0 /* For ADC0. */
#define DEMO_PDB_ADC_PRETRIGGER_CHANNEL kPDB_ADCPreTrigger0     /* For ADC0_SC1[0]. */

#define DEMO_ADC_BASE          ADC0
#define DEMO_ADC_CHANNEL_GROUP 0U
#define DEMO_ADC_USER_CHANNEL  0U
#define DEMO_ADC_IRQ_ID        ADC0_IRQn
#define DEMO_ADC_IRQ_HANDLER   ADC0_IRQHandler
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
/*${prototype:end}*/

#endif /* _APP_H_ */
