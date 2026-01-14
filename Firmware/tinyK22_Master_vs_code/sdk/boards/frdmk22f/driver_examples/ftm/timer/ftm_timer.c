/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "board.h"
#include "app.h"
#include "fsl_ftm.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef DEMO_TIMER_PERIOD_US
/* Set counter period to 1ms */
#define DEMO_TIMER_PERIOD_US (1000U)
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool ftmIsrFlag           = false;
volatile uint32_t milisecondCounts = 0U;

/*******************************************************************************
 * Code
 ******************************************************************************/
int main(void)
{
    uint32_t cnt;
    uint32_t loop       = 2U;
    uint32_t secondLoop = 1000U;
    const char *signals = "-|";
    ftm_config_t ftmInfo;

    /* Board pin, clock, debug console init */
    BOARD_InitHardware();

    /* Print a note to terminal */
    PRINTF("\r\nFTM example to simulate a timer\r\n");
    PRINTF("\r\nYou will see a \"-\" or \"|\" in terminal every 1 second:\r\n");

    /* Fill in the FTM config struct with the default settings */
    FTM_GetDefaultConfig(&ftmInfo);
    /* Calculate the clock division based on the timer period frequency to be obtained */
    ftmInfo.prescale =
        FTM_CalculateCounterClkDiv(BOARD_FTM_BASEADDR, 1000000U / DEMO_TIMER_PERIOD_US, FTM_SOURCE_CLOCK);
    ;

    /* Initialize FTM module */
    FTM_Init(BOARD_FTM_BASEADDR, &ftmInfo);

    /* Set timer period */
    FTM_SetTimerPeriod(BOARD_FTM_BASEADDR,
                       USEC_TO_COUNT(DEMO_TIMER_PERIOD_US, FTM_SOURCE_CLOCK / (1U << ftmInfo.prescale)));

    FTM_EnableInterrupts(BOARD_FTM_BASEADDR, kFTM_TimeOverflowInterruptEnable);

    EnableIRQ(BOARD_FTM_IRQ_NUM);

    FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);

    cnt = 0;
    while (true)
    {
        if (ftmIsrFlag)
        {
            milisecondCounts++;
            ftmIsrFlag = false;
            if (milisecondCounts >= secondLoop)
            {
                PRINTF("%c", signals[cnt & 1]);
                cnt++;
                if (cnt >= loop)
                {
                    cnt = 0;
                }
                milisecondCounts = 0U;
            }
        }
        __WFI();
    }
}

void BOARD_FTM_HANDLER(void)
{
    /* Clear interrupt flag.*/
    FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, kFTM_TimeOverflowFlag);
    ftmIsrFlag = true;
    SDK_ISR_EXIT_BARRIER;
}
