/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_HAS_PARK_BUTTON
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
#include "clock.h"

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

  if (BTN_IsPressed(BTN_USER)) {
    val |= BTN_BIT_USER;
  }
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
      CLOCK_On(CLOCK_MODE_TOGGLE);
      break;

    case MCUDBNC_EVENT_LONG_RELEASED:
      CLOCK_Park(CLOCK_MODE_TOGGLE);
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

#if McuLib_CONFIG_CPU_IS_KINETIS
#if TINYK22_HAT_VERSION>3
  /* all buttons are on PTB */
void PORTA_IRQHandler(void) {
  uint32_t flags;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  flags = GPIO_PortGetInterruptFlags(GPIOA);
  if (flags&(1U<<BUTTONS_PINS_HATNAVLEFT_PIN)) {
    GPIO_PortClearInterruptFlags(BUTTONS_PINS_HATNAVLEFT_GPIO, 1U<<BUTTONS_PINS_HATNAVLEFT_PIN);
    StartDebounce(BTN_BIT_NAV_LEFT, true);
  }
  if (flags&(1U<<BUTTONS_PINS_HATNAVRIGHT_PIN)) {
    GPIO_PortClearInterruptFlags(BUTTONS_PINS_HATNAVRIGHT_GPIO, 1U<<BUTTONS_PINS_HATNAVRIGHT_PIN);
    StartDebounce(BTN_BIT_NAV_RIGHT, true);
  }
  if (xHigherPriorityTaskWoken) {
    taskYIELD();
  }
  __DSB();
}
#endif
#endif

#if McuLib_CONFIG_CPU_IS_KINETIS
void PORTB_IRQHandler(void) {
  uint32_t flags;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  flags = GPIO_PortGetInterruptFlags(GPIOB);
  if (flags&(1U<<BUTTONS_PINS_HATNAVCENTER_PIN)) {
    GPIO_PortClearInterruptFlags(BUTTONS_PINS_HATNAVCENTER_GPIO, 1U<<BUTTONS_PINS_HATNAVCENTER_PIN);
    StartDebounce(BTN_BIT_NAV_CENTER, true);
  }
  if (flags&(1U<<BUTTONS_PINS_HATNAVDOWN_PIN)) {
    GPIO_PortClearInterruptFlags(BUTTONS_PINS_HATNAVDOWN_GPIO, 1U<<BUTTONS_PINS_HATNAVDOWN_PIN);
    StartDebounce(BTN_BIT_NAV_DOWN, true);
  }
  if (flags&(1U<<BUTTONS_PINS_HATNAVUP_PIN)) {
    GPIO_PortClearInterruptFlags(BUTTONS_PINS_HATNAVUP_GPIO, 1U<<BUTTONS_PINS_HATNAVUP_PIN);
    StartDebounce(BTN_BIT_NAV_UP, true);
  }
#if TINYK22_HAT_VERSION==3 /* all buttons are on PTB */
  if (flags&(1U<<BUTTONS_PINS_HATNAVLEFT_PIN)) {
    GPIO_PortClearInterruptFlags(BUTTONS_PINS_HATNAVLEFT_GPIO, 1U<<BUTTONS_PINS_HATNAVLEFT_PIN);
    StartDebounce(BTN_BIT_NAV_LEFT, true);
  }
  if (flags&(1U<<BUTTONS_PINS_HATNAVRIGHT_PIN)) {
    GPIO_PortClearInterruptFlags(BUTTONS_PINS_HATNAVRIGHT_GPIO, 1U<<BUTTONS_PINS_HATNAVRIGHT_PIN);
    StartDebounce(BTN_BIT_NAV_RIGHT, true);
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
  StartDebounce(BTN_BIT_USER, true);
#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  McuSystemView_OnUserStop(MCU_SYSTEM_VIEW_USER_ID_BUTTON_INTERRUPT);
  McuSystemView_RecordExitISR();
#endif
}
#endif

void BTN_Deinit(void) {
#if McuLib_CONFIG_CPU_IS_KINETIS
#if TINYK22_HAT_VERSION==3
  /* all buttons are on PTB */
  DisableIRQ(PORTB_IRQn);
#else
  DisableIRQ(PORTA_IRQn);
  DisableIRQ(PORTB_IRQn);
#endif
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

  btnConfig.hw.gpio = BUTTONS_PINS_HATNAVCENTER_GPIO;
  btnConfig.hw.port = BUTTONS_PINS_HATNAVCENTER_PORT;
  btnConfig.hw.pin = BUTTONS_PINS_HATNAVCENTER_PIN;
  BTN_Infos[BTN_NAV_CENTER].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.gpio = BUTTONS_PINS_HATNAVLEFT_GPIO;
  btnConfig.hw.port = BUTTONS_PINS_HATNAVLEFT_PORT;
  btnConfig.hw.pin = BUTTONS_PINS_HATNAVLEFT_PIN;
  BTN_Infos[BTN_NAV_LEFT].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.gpio = BUTTONS_PINS_HATNAVRIGHT_GPIO;
  btnConfig.hw.port = BUTTONS_PINS_HATNAVRIGHT_PORT;
  btnConfig.hw.pin = BUTTONS_PINS_HATNAVRIGHT_PIN;
  BTN_Infos[BTN_NAV_RIGHT].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.gpio = BUTTONS_PINS_HATNAVUP_GPIO;
  btnConfig.hw.port = BUTTONS_PINS_HATNAVUP_PORT;
  btnConfig.hw.pin = BUTTONS_PINS_HATNAVUP_PIN;
  BTN_Infos[BTN_NAV_UP].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.gpio = BUTTONS_PINS_HATNAVDOWN_GPIO;
  btnConfig.hw.port = BUTTONS_PINS_HATNAVDOWN_PORT;
  btnConfig.hw.pin = BUTTONS_PINS_HATNAVDOWN_PIN;
  BTN_Infos[BTN_NAV_DOWN].handle = McuBtn_InitButton(&btnConfig);

#if TINYK22_HAT_VERSION==3
  /* all buttons are on PTB */
  PORT_SetPinInterruptConfig(BUTTONS_PINS_HATNAVLEFT_PORT,  BUTTONS_PINS_HATNAVLEFT_PIN,  kPORT_InterruptFallingEdge);
  PORT_SetPinInterruptConfig(BUTTONS_PINS_HATNAVRIGHT_PORT, BUTTONS_PINS_HATNAVRIGHT_PIN, kPORT_InterruptFallingEdge);
  PORT_SetPinInterruptConfig(BUTTONS_PINS_HATNAVDOWN_PORT, BUTTONS_PINS_HATNAVDOWN_PIN, kPORT_InterruptFallingEdge);
  PORT_SetPinInterruptConfig(BUTTONS_PINS_HATNAVUP_PORT,   BUTTONS_PINS_HATNAVUP_PIN,   kPORT_InterruptFallingEdge);
  PORT_SetPinInterruptConfig(BUTTONS_PINS_HATNAVCENTER_PORT, BUTTONS_PINS_HATNAVCENTER_PIN, kPORT_InterruptFallingEdge);
  NVIC_SetPriority(PORTB_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
  EnableIRQ(PORTB_IRQn);
#else
  PORT_SetPinInterruptConfig(BUTTONS_PINS_HATNAVLEFT_PORT,  BUTTONS_PINS_HATNAVLEFT_PIN,  kPORT_InterruptFallingEdge);
  PORT_SetPinInterruptConfig(BUTTONS_PINS_HATNAVRIGHT_PORT, BUTTONS_PINS_HATNAVRIGHT_PIN, kPORT_InterruptFallingEdge);
  NVIC_SetPriority(PORTA_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
  EnableIRQ(PORTA_IRQn);

  PORT_SetPinInterruptConfig(BUTTONS_PINS_HATNAVDOWN_PORT, BUTTONS_PINS_HATNAVDOWN_PIN, kPORT_InterruptFallingEdge);
  PORT_SetPinInterruptConfig(BUTTONS_PINS_HATNAVUP_PORT,   BUTTONS_PINS_HATNAVUP_PIN,   kPORT_InterruptFallingEdge);
  PORT_SetPinInterruptConfig(BUTTONS_PINS_HATNAVCENTER_PORT, BUTTONS_PINS_HATNAVCENTER_PIN, kPORT_InterruptFallingEdge);
  NVIC_SetPriority(PORTB_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
  EnableIRQ(PORTB_IRQn);
#endif
#elif McuLib_CONFIG_CPU_IS_LPC
  /* user button on LPC845-BRK board: PIO0_4 */

  McuBtn_Config_t btnConfig;

  BUTTONS_ENABLE_CLOCK();
  McuBtn_GetDefaultConfig(&btnConfig);
  btnConfig.isLowActive = true;

  btnConfig.hw.gpio = BUTTONS_USER_GPIO;
  btnConfig.hw.port = BUTTONS_USER_PORT;
  btnConfig.hw.pin = BUTTONS_USER_PIN;
  btnConfig.hw.iocon = BUTTONS_USER_IOCON;
  BTN_Infos[BTN_USER].handle = McuBtn_InitButton(&btnConfig);

  SYSCON_AttachSignal(SYSCON, kPINT_PinInt0, kSYSCON_GpioPort0Pin4ToPintsel);

  PINT_Init(PINT); /* Initialize PINT */
  /* Setup Pin Interrupt 0 for rising edge */
  PINT_PinInterruptConfig(PINT, kPINT_PinInt0, kPINT_PinIntEnableFallEdge, pint_intr_callback);
  /* Enable callbacks for PINT0 by Index */
  PINT_EnableCallbackByIndex(PINT, kPINT_PinInt0);
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
#endif /* PL_CONFIG_HAS_PARK_BUTTON */
