/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*${header:start}*/
#include "pin_mux.h"
#include "fsl_port.h"
#include "fsl_smc.h"
#include "fsl_rcm.h"
#include "fsl_uart.h"
#include "fsl_pmc.h"
#include "fsl_notifier.h"
#include "power_manager.h"
#include "fsl_debug_console.h"
#include "app.h"
#include "peripherals.h"
#include "board.h"
/*${header:end}*/

extern void APP_SetClockRunFromVlpr(void);

/*${function:start}*/

void APP_SetClockVlpr(void)
{
    const sim_clock_config_t simConfig = {
        .pllFllSel = 3U,          /* PLLFLLSEL select IRC48MCLK. */
        .er32kSrc  = 2U,          /* ERCLK32K selection, use RTC. */
        .clkdiv1   = 0x00040000U, /* SIM_CLKDIV1. */
    };

    CLOCK_SetSimSafeDivs();
    CLOCK_SetInternalRefClkConfig(kMCG_IrclkEnable, kMCG_IrcFast, 0U);

    /* MCG works in PEE mode now, will switch to BLPI mode. */

    CLOCK_ExternalModeToFbeModeQuick();                     /* Enter FBE. */
    CLOCK_SetFbiMode(kMCG_Dmx32Default, kMCG_DrsLow, NULL); /* Enter FBI. */
    CLOCK_SetLowPowerEnable(true);                          /* Enter BLPI. */

    CLOCK_SetSimConfig(&simConfig);
}

void APP_SetClockRunFromVlpr(void)
{
    const sim_clock_config_t simConfig = {
        .pllFllSel = 1U,          /* PLLFLLSEL select PLL. */
        .er32kSrc  = 2U,          /* ERCLK32K selection, use RTC. */
        .clkdiv1   = 0x01230000U, /* SIM_CLKDIV1. */
    };

    const mcg_pll_config_t pll0Config = {
        .enableMode = 0U,
        .prdiv      = 0x3U,
        .vdiv       = 0x10U,
    };

    CLOCK_SetSimSafeDivs();

    /* Currently in BLPI mode, will switch to PEE mode. */
    /* Enter FBI. */
    CLOCK_SetLowPowerEnable(false);
    /* Enter FBE. */
    CLOCK_SetFbeMode(3U, kMCG_Dmx32Default, kMCG_DrsLow, NULL);
    /* Enter PBE. */
    CLOCK_SetPbeMode(kMCG_PllClkSelPll0, &pll0Config);
    /* Enter PEE. */
    CLOCK_SetPeeMode();

    CLOCK_SetSimConfig(&simConfig);
}

void APP_SetClockHsrun(void)
{
    const sim_clock_config_t simConfig = {
        .pllFllSel = 1U,          /* PLLFLLSEL select PLL. */
        .er32kSrc  = 2U,          /* ERCLK32K selection, use RTC. */
        .clkdiv1   = 0x01340000U, /* SIM_CLKDIV1. */
    };

    const mcg_pll_config_t pll0Config = {
        .enableMode = 0U,
        .prdiv      = 0x1U,
        .vdiv       = 0x6U,
    };

    CLOCK_SetPbeMode(kMCG_PllClkSelPll0, &pll0Config);
    CLOCK_SetPeeMode();

    CLOCK_SetSimConfig(&simConfig);
}

void APP_SetClockRunFromHsrun(void)
{
    const sim_clock_config_t simConfig = {
        .pllFllSel = 1U,          /* PLLFLLSEL select PLL. */
        .er32kSrc  = 2U,          /* ERCLK32K selection, use RTC. */
        .clkdiv1   = 0x01230000U, /* SIM_CLKDIV1. */
    };

    const mcg_pll_config_t pll0Config = {
        .enableMode = 0U,
        .prdiv      = 0x3U,
        .vdiv       = 0x10U,
    };

    CLOCK_SetPbeMode(kMCG_PllClkSelPll0, &pll0Config);
    CLOCK_SetPeeMode();

    CLOCK_SetSimConfig(&simConfig);
}

static void APP_InitDebugConsole(void)
{
    uint32_t uartClkSrcFreq;
    uartClkSrcFreq = CLOCK_GetFreq(APP_DEBUG_UART_DEFAULT_CLKSRC_NAME);
    DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, APP_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
}

void BOARD_InitHardware(void)
{
    /* Must configure pins before PMC_ClearPeriphIOIsolationFlag */
    BOARD_InitBootPins();

    /* Power related. */
    SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeAll);
    if (kRCM_SourceWakeup & RCM_GetPreviousResetSources(RCM)) /* Wakeup from VLLS. */
    {
        PMC_ClearPeriphIOIsolationFlag(PMC);
        NVIC_ClearPendingIRQ(LLWU_IRQn);
    }

    BOARD_InitBootClocks();
    APP_InitDebugConsole();
    BOARD_InitBootPeripherals();
}

status_t callback0(notifier_notification_block_t *notify, void *dataPtr)
{
    user_callback_data_t *userData     = (user_callback_data_t *)dataPtr;
    status_t ret                       = kStatus_Fail;
    app_power_mode_t targetMode        = ((power_user_config_t *)notify->targetConfig)->mode;
    smc_power_state_t originPowerState = userData->originPowerState;
    smc_power_state_t powerState;

    switch (notify->notifyType)
    {
        case kNOTIFIER_NotifyBefore:
            userData->beforeNotificationCounter++;
            /* Wait for debug console output finished. */
            while (!(kUART_TransmissionCompleteFlag & UART_GetStatusFlags((UART_Type *)BOARD_DEBUG_UART_BASEADDR)))
            {
            }
            DbgConsole_Deinit();

            if ((kAPP_PowerModeRun != targetMode) && (kAPP_PowerModeHsrun != targetMode) &&
                (kAPP_PowerModeVlpr != targetMode))
            {
                /*
                 * Set pin for current leakage.
                 * Debug console RX pin: Set to pinmux to disable.
                 * Debug console TX pin: Don't need to change.
                 */
                PORT_SetPinMux(DEBUG_CONSOLE_RX_PORT, DEBUG_CONSOLE_RX_PIN, kPORT_PinDisabledOrAnalog);
            }

            ret = kStatus_Success;
            break;
        case kNOTIFIER_NotifyRecover:
            break;
        case kNOTIFIER_CallbackAfter:
            userData->afterNotificationCounter++;
            powerState = SMC_GetPowerModeState(SMC);

            /*
             * For some other platforms, if enter LLS mode from VLPR mode, when wakeup, the
             * power mode is VLPR. But for some platforms, if enter LLS mode from VLPR mode,
             * when wakeup, the power mode is RUN. In this case, the clock setting is still
             * VLPR mode setting, so change to RUN mode setting here.
             */
            if ((kSMC_PowerStateVlpr == originPowerState) && (kSMC_PowerStateRun == powerState))
            {
                APP_SetClockRunFromVlpr();
            }

            if ((kAPP_PowerModeRun != targetMode) && (kAPP_PowerModeHsrun != targetMode) &&
                (kAPP_PowerModeVlpr != targetMode))
            {
                /*
                 * Debug console RX pin is set to disable for current leakage, nee to re-configure pinmux.
                 * Debug console TX pin: Don't need to change.
                 */
                PORT_SetPinMux(DEBUG_CONSOLE_RX_PORT, DEBUG_CONSOLE_RX_PIN, DEBUG_CONSOLE_RX_PINMUX);
            }

            /*
             * If enter stop modes when MCG in PEE mode, then after wakeup, the MCG is in PBE mode,
             * need to enter PEE mode manually.
             */
            if ((kAPP_PowerModeRun != targetMode) && (kAPP_PowerModeWait != targetMode) &&
                (kAPP_PowerModeVlpw != targetMode) && (kAPP_PowerModeHsrun != targetMode) &&
                (kAPP_PowerModeVlpr != targetMode))
            {
                if (kSMC_PowerStateRun == originPowerState)
                {
                    /* Wait for PLL lock. */
                    while (!(kMCG_Pll0LockFlag & CLOCK_GetStatusFlags()))
                    {
                    }
                    CLOCK_SetPeeMode();
                }
            }

            /* Set debug console clock source. */
            APP_InitDebugConsole();

            ret = kStatus_Success;
            break;
        default:
            break;
    }
    return ret;
}
/*${function:end}*/
