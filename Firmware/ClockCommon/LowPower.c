/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * NXP Application notes for LowPower: AN4470 & AN4503
 * see as well:
 * https://mcuoneclipse.com/2014/03/16/starting-point-for-kinetis-low-power-lls-mode/
 *
 * Application is using LLS (Low Leakage Stop) mode
 */

#include "platform.h"
#include "LowPower.h"
#include "McuRTOS.h"
#include "fsl_llwu.h"
#include "cmsis_gcc.h"
#include "fsl_smc.h"
#include "McuLog.h"

void LP_EnterWaitMode(void) {
  __asm volatile("dsb");
  __asm volatile("wfi");
  __asm volatile("isb");
}

#if LP_MODE_SELECTED==LP_MODE_STOP
void App_EnterStopMode(void) {
  status_t status;

  SMC_PreEnterStopModes();
  status = SMC_SetPowerModeStop(SMC, kSMC_PartialStop);
  SMC_PostExitStopModes();
  if (status!=kStatus_Success) {
    McuLog_fatal("failed exit stop mode");
    for(;;) {}
  }
}
#endif

void LP_Init(void) {
}

#if 0 && PL_CONFIG_USE_LOW_POWER
#define DEMO_LLWU_PERIPHERAL  LLWU

#define APP_LLWU              DEMO_LLWU_PERIPHERAL
#define APP_LLWU_IRQHANDLER   DEMO_LLWU_IRQHANDLER

#define APP_LPTMR             DEMO_LPTMR_PERIPHERAL
#define APP_LPTMR_IRQHANDLER  DEMO_LPTMR_IRQHANDLER

#define LLWU_LPTMR_IDX        0U      /* LLWU_M0IF */

/* interrupt called in case of LLWU wake-up */
void LLWU_ISR(void) {
#if 0
  uint32_t wakeUpFlags;

  //LED_B_On(); /* short blink to indicate wakeup activity */
  wakeUpFlags = Cpu_GetLLSWakeUpFlags();
  if (wakeUpFlags&LLWU_INT_MODULE0) { /* LPTMR */
    LPTMR_PDD_ClearInterruptFlag(LPTMR0_BASE_PTR); /* Clear interrupt flag */
    LLWU_F3 |= LLWU_F3_MWUF0_MASK; /* clear WakeUpInt flag for Module0 (LPTMR) */
    LED_R_On();
    for(;;) {}  /*! \todo somehow do not get here? */
  }
  if (wakeUpFlags&LLWU_INT_MODULE5) { /* RTC Alarm */
    LLWU_F3 |= LLWU_F3_MWUF5_MASK; /* clear WakeUpInt flag for Module5 (RTC) */
    RTC_ALARM_ISR();
  }
  if (wakeUpFlags&LLWU_EXT_PIN5) {   /* PTB0 = LightSensor */
    LLWU_F1 |= LLWU_F1_WUF5_MASK; //Clear WakeUpInt Flag
    LightSensor_Done_ISR();
  }
#endif
  /* If wakeup by LPTMR. */
  if (LLWU_GetInternalWakeupModuleFlag(APP_LLWU, LLWU_LPTMR_IDX))
  {
      /* Disable lptmr as a wakeup source, so that lptmr's IRQ Handler will be executed when reset from VLLSx mode. */
      LLWU_EnableInternalModuleInterruptWakup(APP_LLWU, LLWU_LPTMR_IDX, false);
  }
  /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
  exception return operation might vector to incorrect interrupt */
  __DSB();
}
#endif /* PL_CONFIG_USE_LOW_POWER */

#if 0
/*
 * LowPower.c
 *
 *  Created on: Feb 22, 2019
 *      Author: dave
 *
 * NXP Application notes to LowPower: AN4470 & AN4503
 *
 * For the K22DX256 controller:
 * https://mcuoneclipse.com/2014/03/16/starting-point-for-kinetis-low-power-lls-mode/
 */

#include "platform.h"
#include "LowPower.h"
#include "Cpu.h"
#include "CLS1.h"
#include "Events.h"
#include "UI.h"
#include "LPTMR_PDD.h"
#include "LightSensor.h"
#include "RTC.h"

#define LP_USE_PLL    (1)  /* if using PLL clock configuration */

static volatile bool stopModeAllowed = FALSE;
#if LP_USE_PLL
static unsigned char ucMCG_C1; /* backup to restore register value */
#endif

BaseType_t xEnterTicklessIdle(void) {
#if PL_CONFIG_HAS_LOW_POWER
  return pdTRUE; /* enter tickless idle mode */
#else
  return pdFALSE; /* do not enter tickless idle mode */
#endif
}

void LowPower_EnterLowpowerMode(void) {
#if PL_CONFIG_HAS_LOW_POWER
  if(stopModeAllowed) {
#if LP_USE_PLL
    ucMCG_C1 = MCG_C1; /* remember current MCG value for the wakeup */
#endif
    Cpu_SetOperationMode(DOM_STOP, NULL, NULL);
  } else {
    __asm volatile("dsb");
    __asm volatile("wfi");
    __asm volatile("isb");
  }
#else
  #warning "low power mode disabled!"
  __asm volatile("dsb");
  __asm volatile("wfi");
  __asm volatile("isb");
#endif
}

void LowPower_EnableStopMode(void) {
  stopModeAllowed = TRUE;
}

void LowPower_DisableStopMode(void) {
  stopModeAllowed = FALSE;
}

bool LowPower_StopModeIsEnabled(void) {
  return stopModeAllowed;
}

void LowPower_Init(void) {
  /* configure LLWU (Low Leakage Wake-Up) pins */
  LLWU_PE2 |= LLWU_PE2_WUPE5(0x1); //Enable PTB0 (LightSensor) as WakeUpSource with rising edge
#if PL_BOARD_REV==20 || PL_BOARD_REV==21
  LLWU_PE2 |= LLWU_PE2_WUPE6(0x1); //Enable PTC1 (UserButton) as WakeUpSouce with rising edge
#else
  LLWU_PE4 |= LLWU_PE4_WUPE12(0x1); //Enable PTD0 (UserButton) as WakeUpSouce with rising edge
#endif
}

/* interrupt called in case of LLWU wake-up */
void LLWU_ISR(void) {
#if LP_USE_PLL
  #define CLOCK_DIV 2
  #define CLOCK_MUL 24
  #define MCG_C6_VDIV0_LOWEST 24
#endif
  uint32_t wakeUpFlags;

#if LP_USE_PLL
  // Switch to PLL and wait for it to fully start up
  // https://community.nxp.com/thread/458972
  MCG_C5 = ((CLOCK_DIV - 1) | MCG_C5_PLLSTEN0_MASK); // move from state FEE to state PBE (or FBE) PLL remains enabled in normal stop modes
  MCG_C6 = ((CLOCK_MUL - MCG_C6_VDIV0_LOWEST) | MCG_C6_PLLS_MASK);
  while ((MCG_S & MCG_S_PLLST_MASK) == 0) {}   // loop until the PLLS clock source becomes valid
  while ((MCG_S & MCG_S_LOCK0_MASK) == 0) {}    // loop until PLL locks
  MCG_C1 = ucMCG_C1;                      // finally move from PBE to PEE mode - switch to PLL clock (the original settings are returned)
  while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST(11)) {} // loop until the PLL clock is selected
#endif
  //LED_B_On(); /* short blink to indicate wakeup activity */

  wakeUpFlags = Cpu_GetLLSWakeUpFlags();
  if (wakeUpFlags&LLWU_INT_MODULE0) { /* LPTMR */
    LPTMR_PDD_ClearInterruptFlag(LPTMR0_BASE_PTR); /* Clear interrupt flag */
    LLWU_F3 |= LLWU_F3_MWUF0_MASK; /* clear WakeUpInt flag fo-r Module0 (LPTMR) */
    LED_R_On();
    for(;;) {}  /*! \todo somehow do not get here? */
  }
  if (wakeUpFlags&LLWU_INT_MODULE5) { /* RTC Alarm */
    LLWU_F3 |= LLWU_F3_MWUF5_MASK; /* clear WakeUpInt flag for Module5 (RTC) */
    RTC_ALARM_ISR();
  }
  if (wakeUpFlags&LLWU_EXT_PIN5) {   /* PTB0 = LightSensor */
    LLWU_F1 |= LLWU_F1_WUF5_MASK; //Clear WakeUpInt Flag
    LightSensor_Done_ISR();
  }
#if PL_BOARD_REV==20 || PL_BOARD_REV==21
  if (wakeUpFlags&LLWU_EXT_PIN6) { /* PTC1 = UserButton */
    LLWU_F1 |= LLWU_F1_WUF6_MASK; //Clear WakeUpInt Flag
    UI_ButtonPressed_ISR();
  }
#else
  if (wakeUpFlags&LLWU_EXT_PIN12) { /* PTD0 = UserButton */
    LLWU_F2 |= LLWU_F2_WUF12_MASK; //Clear WakeUpInt Flag
    //LED_R_On();
    //WAIT1_Waitus(1000);
    //LED_R_Off();
    UI_ButtonPressed_ISR();
  }
#endif
  WAIT1_Waitus(20);
  //LED_B_Off();
  /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
       exception return operation might vector to incorrect interrupt */
  __DSB();
}
#endif
