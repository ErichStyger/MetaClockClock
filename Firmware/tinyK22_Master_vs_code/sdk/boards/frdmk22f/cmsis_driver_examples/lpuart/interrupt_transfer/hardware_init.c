/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*${header:start}*/
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
/*${header:end}*/

/*${function:start}*/
void BOARD_InitHardware(void)
{
    BOARD_BootClockRUN();
    CLOCK_SetLpuartClock(0x1U);
}

uint32_t LPUART0_GetFreq(void)
{
    return CLOCK_GetPllFllSelClkFreq();
}

/*${function:end}*/
