/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_HAS_BUTTONS
#include "buttons.h"
#include "buttons_config.h"
#include <assert.h>
#include "McuRTOS.h"
#include "McuButton.h"
#if McuLib_CONFIG_CPU_IS_KINETIS
  #include "fsl_port.h"
#elif McuLib_CONFIG_CPU_IS_LPC
  #include "fsl_pint.h"
  #include "fsl_syscon.h"
#endif
#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  #include "McuSystemView.h"
#endif
#include "McuDebounce.h"
#if PL_CONFIG_USE_CLOCK
  #include "clock.h"
#endif

typedef struct BTN_Desc_t {
  McuBtn_Handle_t handle; /* handle of pin */
} BTN_Desc_t;

static BTN_Desc_t BTN_Infos[BTN_NOF_BUTTONS];

bool BTN_IsPressed(BTN_Buttons_e btn) {
  assert(btn<BTN_NOF_BUTTONS);
  return McuBtn_IsOn(BTN_Infos[btn].handle);
}

static uint32_t GetButtons(void) {
  uint32_t val = 0;

#if PL_CONFIG_HAS_SWITCH_USER
  if (BTN_IsPressed(BTN_USER)) {
    val |= BTN_BIT_USER;
  }
#endif
#if PL_CONFIG_HAS_SWITCH_7WAY
  if (BTN_IsPressed(BTN_UP)) {
    val |= BTN_BIT_UP;
  }
  if (BTN_IsPressed(BTN_DOWN)) {
    val |= BTN_BIT_DOWN;
  }
  if (BTN_IsPressed(BTN_LEFT)) {
    val |= BTN_BIT_LEFT;
  }
  if (BTN_IsPressed(BTN_RIGHT)) {
    val |= BTN_BIT_RIGHT;
  }
  if (BTN_IsPressed(BTN_MID)) {
    val |= BTN_BIT_MID;
  }
  if (BTN_IsPressed(BTN_SET)) {
    val |= BTN_BIT_SET;
  }
  if (BTN_IsPressed(BTN_RST)) {
    val |= BTN_BIT_RST;
  }
#elif PL_CONFIG_HAS_SWITCH_2WAY
  if (BTN_IsPressed(BTN_SET)) {
    val |= BTN_BIT_SET;
  }
  if (BTN_IsPressed(BTN_RST)) {
    val |= BTN_BIT_RST;
  }
#endif
  return val;
}

static void OnDebounceEvent(McuDbnc_EventKinds event, uint32_t buttons);

#define TIMER_PERIOD_MS  20 /* frequency of debouncing timer */
static McuDbnc_Desc_t data =
{
  .state = MCUDBMC_STATE_IDLE,  /* state of state machine */
  .timerPeriodMs = TIMER_PERIOD_MS, /* timer period for debouncing */
  .timer = NULL, /* FreeRTOS timer handle */
  .debounceTimeMs = 100, /* debouncing time */
  .repeatTimeMs   = 300, /* time for repeated button events */
  .longKeyTimeMs  = 1000, /* time for a long key press */
  .getButtons = GetButtons, /* callback to get bitset of buttons */
  .onDebounceEvent = OnDebounceEvent, /* debounce event handler */
};

static void OnDebounceEvent(McuDbnc_EventKinds event, uint32_t buttons) {
  switch(event) {
    case MCUDBNC_EVENT_PRESSED:
      break;

    case MCUDBNC_EVENT_PRESSED_REPEAT:
      break;

    case MCUDBNC_EVENT_LONG_PRESSED:
      break;

    case MCUDBNC_EVENT_LONG_PRESSED_REPEAT:
      break;

    case MCUDBNC_EVENT_RELEASED:
#if PL_CONFIG_USE_CLOCK
      CLOCK_ButtonHandler(event, buttons);
#endif
      break;

    case MCUDBNC_EVENT_LONG_RELEASED:
#if PL_CONFIG_USE_CLOCK
      CLOCK_ButtonHandler(event, buttons);
#endif
      break;

    default:
    case MCUDBNC_EVENT_END:
      (void)xTimerStop(data.timer, pdMS_TO_TICKS(100)); /* stop timer */
      break;
  }
}

static void vTimerCallbackDebounce(TimerHandle_t pxTimer) {
  /* called with TIMER_PERIOD_MS during debouncing */
  McuDbnc_Process(&data);
}

static void StartDebounce(uint32_t buttons, bool fromISR) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if (data.state==MCUDBMC_STATE_IDLE) {
    data.scanValue = buttons;
    data.state = MCUDBMC_STATE_START;
    McuDbnc_Process(&data);
    if (fromISR) {
      (void)xTimerStartFromISR(data.timer, &xHigherPriorityTaskWoken);
    } else {
      (void)xTimerStart(data.timer, pdMS_TO_TICKS(100));
    }
    if (fromISR) {
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  }
}

#if 0 && McuLib_CONFIG_CPU_IS_KINETIS
void PORTE_IRQHandler(void) {
  uint32_t flags;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

#if PL_CONFIG_HAS_SWITCH_2WAY
  flags = GPIO_PortGetInterruptFlags(GPIOE);
  if (flags&(1U<<BUTTONS_DOWN_PIN)) {
    GPIO_PortClearInterruptFlags(BUTTONS_DOWN_GPIO, 1U<<BUTTONS_DOWN_PIN);
    StartDebounce(BTN_BIT_DOWN, true);
  }
  if (flags&(1U<<BUTTONS_UP_PIN)) {
    GPIO_PortClearInterruptFlags(BUTTONS_UP_GPIO, 1U<<BUTTONS_UP_PIN);
    StartDebounce(BTN_BIT_UP, true);
  }
#endif
  if (xHigherPriorityTaskWoken) {
    taskYIELD();
  }
  __DSB();
}
#endif

#if McuLib_CONFIG_CPU_IS_LPC
static void pint_intr_callback(pint_pin_int_t pintr, uint32_t pmatch_status) {
#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  #define MCU_SYSTEM_VIEW_USER_ID_BUTTON_INTERRUPT (0)
  McuSystemView_RecordEnterISR();
  McuSystemView_Print((const char*)"Pressed button\r\n");
  McuSystemView_OnUserStart(MCU_SYSTEM_VIEW_USER_ID_BUTTON_INTERRUPT);
#endif
  switch(pintr) {
  #if PL_CONFIG_HAS_SWITCH_USER
    case kPINT_PinInt0: StartDebounce(BTN_BIT_USER, true); break;
  #endif
  #if PL_CONFIG_HAS_SWITCH_7WAY
    case kPINT_PinInt1: StartDebounce(BTN_BIT_UP, true); break;
    case kPINT_PinInt2: StartDebounce(BTN_BIT_DOWN, true); break;
    case kPINT_PinInt3: StartDebounce(BTN_BIT_LEFT, true); break;
    case kPINT_PinInt4: StartDebounce(BTN_BIT_RIGHT, true); break;
    case kPINT_PinInt5: StartDebounce(BTN_BIT_MID, true); break;
    case kPINT_PinInt6: StartDebounce(BTN_BIT_SET, true); break;
    case kPINT_PinInt7: StartDebounce(BTN_BIT_RST, true); break;
  #endif
    default:
      break;
  }
#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  McuSystemView_OnUserStop(MCU_SYSTEM_VIEW_USER_ID_BUTTON_INTERRUPT);
  McuSystemView_RecordExitISR();
#endif
}
#endif

void BTN_Deinit(void) {
#if McuLib_CONFIG_CPU_IS_KINETIS
  DisableIRQ(PORTE_IRQn);
#endif
  for(int i=0; i<BTN_NOF_BUTTONS; i++) {
    BTN_Infos[i].handle = McuBtn_DeinitButton(BTN_Infos[i].handle);
  }
}

void BTN_Init(void) {
#if McuLib_CONFIG_CPU_IS_KINETIS
  McuBtn_Config_t btnConfig;

  BUTTONS_ENABLE_CLOCK();
  McuBtn_GetDefaultConfig(&btnConfig);
  btnConfig.isLowActive = true;

#if PL_CONFIG_HAS_SWITCH_2WAY
  btnConfig.hw.gpio = BUTTONS_UP_GPIO;
  btnConfig.hw.port = BUTTONS_UP_PORT;
  btnConfig.hw.pin = BUTTONS_UP_PIN;
  BTN_Infos[BTN_SET].handle = McuBtn_InitButton(&btnConfig);
  McuBtn_EnablePullResistor(BTN_Infos[BTN_SET].handle);

  btnConfig.hw.gpio = BUTTONS_DOWN_GPIO;
  btnConfig.hw.port = BUTTONS_DOWN_PORT;
  btnConfig.hw.pin = BUTTONS_DOWN_PIN;
  BTN_Infos[BTN_RST].handle = McuBtn_InitButton(&btnConfig);
  McuBtn_EnablePullResistor(BTN_Infos[BTN_RST].handle);
#endif

#elif McuLib_CONFIG_CPU_IS_LPC
  /* user button on LPC845-BRK board: PIO0_4 */

  McuBtn_Config_t btnConfig;

  BUTTONS_ENABLE_CLOCK();
  McuBtn_GetDefaultConfig(&btnConfig);
  btnConfig.isLowActive = true;

#if PL_CONFIG_HAS_SWITCH_USER
  btnConfig.hw.gpio = BUTTONS_USER_GPIO;
  btnConfig.hw.port = BUTTONS_USER_PORT;
  btnConfig.hw.pin = BUTTONS_USER_PIN;
  btnConfig.hw.iocon = BUTTONS_USER_IOCON;
  BTN_Infos[BTN_USER].handle = McuBtn_InitButton(&btnConfig);
  SYSCON_AttachSignal(SYSCON, kPINT_PinInt0, BUTTONS_USER_PINTSEL);
#endif

#if PL_CONFIG_HAS_SWITCH_7WAY
  btnConfig.hw.gpio = BUTTONS_UP_GPIO;
  btnConfig.hw.port = BUTTONS_UP_PORT;
  btnConfig.hw.pin = BUTTONS_UP_PIN;
  btnConfig.hw.iocon = BUTTONS_UP_IOCON;
  BTN_Infos[BTN_UP].handle = McuBtn_InitButton(&btnConfig);
  McuBtn_EnablePullResistor(BTN_Infos[BTN_UP].handle);
  SYSCON_AttachSignal(SYSCON, kPINT_PinInt1, BUTTONS_UP_PINTSEL);

  btnConfig.hw.gpio = BUTTONS_DOWN_GPIO;
  btnConfig.hw.port = BUTTONS_DOWN_PORT;
  btnConfig.hw.pin = BUTTONS_DOWN_PIN;
  btnConfig.hw.iocon = BUTTONS_DOWN_IOCON;
  BTN_Infos[BTN_DOWN].handle = McuBtn_InitButton(&btnConfig);
  McuBtn_EnablePullResistor(BTN_Infos[BTN_DOWN].handle);
  SYSCON_AttachSignal(SYSCON, kPINT_PinInt2, BUTTONS_DOWN_PINTSEL);

  btnConfig.hw.gpio = BUTTONS_LEFT_GPIO;
  btnConfig.hw.port = BUTTONS_LEFT_PORT;
  btnConfig.hw.pin = BUTTONS_LEFT_PIN;
  btnConfig.hw.iocon = BUTTONS_LEFT_IOCON;
  BTN_Infos[BTN_LEFT].handle = McuBtn_InitButton(&btnConfig);
  McuBtn_EnablePullResistor(BTN_Infos[BTN_LEFT].handle);
  SYSCON_AttachSignal(SYSCON, kPINT_PinInt3, BUTTONS_LEFT_PINTSEL);

  btnConfig.hw.gpio = BUTTONS_RIGHT_GPIO;
  btnConfig.hw.port = BUTTONS_RIGHT_PORT;
  btnConfig.hw.pin = BUTTONS_RIGHT_PIN;
  btnConfig.hw.iocon = BUTTONS_RIGHT_IOCON;
  BTN_Infos[BTN_RIGHT].handle = McuBtn_InitButton(&btnConfig);
  McuBtn_EnablePullResistor(BTN_Infos[BTN_RIGHT].handle);
  SYSCON_AttachSignal(SYSCON, kPINT_PinInt4, BUTTONS_RIGHT_PINTSEL);

  btnConfig.hw.gpio = BUTTONS_MID_GPIO;
  btnConfig.hw.port = BUTTONS_MID_PORT;
  btnConfig.hw.pin = BUTTONS_MID_PIN;
  btnConfig.hw.iocon = BUTTONS_MID_IOCON;
  BTN_Infos[BTN_MID].handle = McuBtn_InitButton(&btnConfig);
  McuBtn_EnablePullResistor(BTN_Infos[BTN_MID].handle);
  SYSCON_AttachSignal(SYSCON, kPINT_PinInt5, BUTTONS_MID_PINTSEL);

  btnConfig.hw.gpio = BUTTONS_SET_GPIO;
  btnConfig.hw.port = BUTTONS_SET_PORT;
  btnConfig.hw.pin = BUTTONS_SET_PIN;
  btnConfig.hw.iocon = BUTTONS_SET_IOCON;
  BTN_Infos[BTN_SET].handle = McuBtn_InitButton(&btnConfig);
  McuBtn_EnablePullResistor(BTN_Infos[BTN_SET].handle);
  SYSCON_AttachSignal(SYSCON, kPINT_PinInt6, BUTTONS_SET_PINTSEL);

  btnConfig.hw.gpio = BUTTONS_RST_GPIO;
  btnConfig.hw.port = BUTTONS_RST_PORT;
  btnConfig.hw.pin = BUTTONS_RST_PIN;
  btnConfig.hw.iocon = BUTTONS_RST_IOCON;
  BTN_Infos[BTN_RST].handle = McuBtn_InitButton(&btnConfig);
  McuBtn_EnablePullResistor(BTN_Infos[BTN_RST].handle);
  SYSCON_AttachSignal(SYSCON, kPINT_PinInt7, BUTTONS_RST_PINTSEL);
#endif

  PINT_Init(PINT); /* Initialize PINT */
  /* Setup Pin Interrupt 0 for rising edge */
  PINT_PinInterruptConfig(PINT, kPINT_PinInt0, kPINT_PinIntEnableFallEdge, pint_intr_callback);
  /* Enable callbacks for PINT0 by Index */
  PINT_EnableCallbackByIndex(PINT, kPINT_PinInt0);

#if PL_CONFIG_HAS_SWITCH_7WAY
  PINT_PinInterruptConfig(PINT, kPINT_PinInt1, kPINT_PinIntEnableFallEdge, pint_intr_callback);
  PINT_EnableCallbackByIndex(PINT, kPINT_PinInt1);
  PINT_PinInterruptConfig(PINT, kPINT_PinInt2, kPINT_PinIntEnableFallEdge, pint_intr_callback);
  PINT_EnableCallbackByIndex(PINT, kPINT_PinInt2);
  PINT_PinInterruptConfig(PINT, kPINT_PinInt3, kPINT_PinIntEnableFallEdge, pint_intr_callback);
  PINT_EnableCallbackByIndex(PINT, kPINT_PinInt3);
  PINT_PinInterruptConfig(PINT, kPINT_PinInt4, kPINT_PinIntEnableFallEdge, pint_intr_callback);
  PINT_EnableCallbackByIndex(PINT, kPINT_PinInt4);
  PINT_PinInterruptConfig(PINT, kPINT_PinInt5, kPINT_PinIntEnableFallEdge, pint_intr_callback);
  PINT_EnableCallbackByIndex(PINT, kPINT_PinInt5);
  PINT_PinInterruptConfig(PINT, kPINT_PinInt6, kPINT_PinIntEnableFallEdge, pint_intr_callback);
  PINT_EnableCallbackByIndex(PINT, kPINT_PinInt6);
  PINT_PinInterruptConfig(PINT, kPINT_PinInt7, kPINT_PinIntEnableFallEdge, pint_intr_callback);
  PINT_EnableCallbackByIndex(PINT, kPINT_PinInt7);
#endif
#endif

  data.timer = xTimerCreate(
        "tmrDbnc", /* name */
        pdMS_TO_TICKS(TIMER_PERIOD_MS), /* period/time */
        pdTRUE, /* auto reload */
        (void*)0, /* timer ID */
        vTimerCallbackDebounce); /* callback */
  if (data.timer==NULL) {
    for(;;); /* failure! */
  }
}
#endif /* PL_CONFIG_HAS_BUTTONS */
