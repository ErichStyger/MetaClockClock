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
#define DEMO_PDB_BASE               PDB0
#define DEMO_PDB_IRQ_ID             PDB0_IRQn
#define DEMO_PDB_IRQ_HANDLER        PDB0_IRQHandler
#define DEMO_PDB_DELAY_VALUE        1000U
#define DEMO_PDB_MODULUS_VALUE      1000U
#define DEMO_PDB_DAC_INTERVAL_VALUE 800U

#define DEMO_DAC_BASE             DAC0
#define DEMO_DAC_CHANNEL          kPDB_DACTriggerChannel0
#define DEMO_DAC_IRQ_ID           DAC0_IRQn
#define DEMO_DAC_IRQ_HANDLER      DAC0_IRQHandler
#define DEMO_DAC_USED_BUFFER_SIZE DAC_DATL_COUNT
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
void BOARD_InitHardware(void);
/*${prototype:end}*/

#endif /* _APP_H_ */
