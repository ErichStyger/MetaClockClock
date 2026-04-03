/*
 * Copyright (c) 2022-2024, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_BUTTONS
#include "buttons.h"
#include "buttons_config.h"
#include <assert.h>
#include "McuButton.h"
#include "McuUtility.h"
#include "McuRTOS.h"
#include "McuLED.h"
#include "McuLog.h"
#include "leds.h"
#include "debounce.h"
#if McuLib_CONFIG_CPU_IS_KINETIS
  #include "fsl_port.h"
#elif McuLib_CONFIG_CPU_IS_LPC
  #include "fsl_pint.h"
  #include "fsl_syscon.h"
#endif
#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  #include "McuSystemView.h"
#endif

typedef struct BTN_Desc_t {
  McuBtn_Handle_t handle; /* handle of button pin */
} BTN_Desc_t;

static BTN_Desc_t BTN_Infos[BTN_NOF_BUTTONS];

BTN_Buttons_e BTN_RotateButton(BTN_Buttons_e button) {
#if BTN_CONFIG_ROTATION==0
  return button;
#elif BTN_CONFIG_ROTATION==180
  if (button == BTN_NAV_LEFT) {
    return BTN_NAV_RIGHT;
  } else if (button == BTN_NAV_RIGHT) {
    return BTN_NAV_LEFT;
  } else if (button == BTN_NAV_UP) {
    return BTN_NAV_DOWN;
  } else if (button == BTN_NAV_DOWN) {
    return BTN_NAV_UP;
  }
  return button;
#endif
}

bool BTN_IsPressed(BTN_Buttons_e btn) {
  assert(btn<BTN_NOF_BUTTONS);
  return McuBtn_IsOn(BTN_Infos[btn].handle);
}

uint32_t BTN_GetButtons(void) {
  uint32_t val = 0;

  if (BTN_IsPressed(BTN_NAV_UP)) {
    val |= BTN_BIT_NAV_UP;
  }
  if (BTN_IsPressed(BTN_NAV_DOWN)) {
    val |= BTN_BIT_NAV_DOWN;
  }
  if (BTN_IsPressed(BTN_NAV_LEFT)) {
    val |= BTN_BIT_NAV_LEFT;
  }
  if (BTN_IsPressed(BTN_NAV_RIGHT)) {
    val |= BTN_BIT_NAV_RIGHT;
  }
  if (BTN_IsPressed(BTN_NAV_CENTER)) {
    val |= BTN_BIT_NAV_CENTER;
  }
#if McuLib_CONFIG_CPU_IS_LPC
  if (BTN_IsPressed(BTN_USER)) {
    val |= BTN_BIT_USER;
  }
#endif
  return val;
}

#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
/* IDs for SystemView */
#define MCU_SYSTEM_VIEW_USER_ID_BUTTON_INTERRUPT  (0)

static void SysViewLogStart(void) {
  McuSystemView_RecordEnterISR();
  McuSystemView_Print((const char*)"Pressed button\r\n");
  McuSystemView_OnUserStart(MCU_SYSTEM_VIEW_USER_ID_BUTTON_INTERRUPT);
}
#endif

#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
static void SysViewLogEnd(void) {
  McuSystemView_OnUserStop(MCU_SYSTEM_VIEW_USER_ID_BUTTON_INTERRUPT);
  McuSystemView_RecordExitISR();
}
#endif

#if McuLib_CONFIG_CPU_IS_KINETIS && PL_CONFIG_USE_BUTTONS_IRQ
void PORTA_IRQHandler(void) {
  uint32_t flags;
  uint32_t buttons = 0;
  BaseType_t xHigherPriorityTaskWoken = false;

#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  SysViewLogStart();
#endif
  flags = GPIO_PortGetInterruptFlags(GPIOA);
  if (flags&(1U<<BUTTONS_PINS_NAVLEFT_PIN)) {
    buttons |= BTN_BIT_NAV_LEFT;
  }
  if (flags&(1U<<BUTTONS_PINS_NAVRIGHT_PIN)) {
    buttons |= BTN_BIT_NAV_RIGHT;
  }
  GPIO_PortClearInterruptFlags(GPIOA, flags);
  Debounce_StartDebounceFromISR(buttons, &xHigherPriorityTaskWoken);
#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  SysViewLogEnd();
#endif
  if (xHigherPriorityTaskWoken) {
    taskYIELD();
  }
  __DSB();
}
#endif

#if McuLib_CONFIG_CPU_IS_KINETIS && PL_CONFIG_USE_BUTTONS_IRQ
void PORTB_IRQHandler(void) {
  uint32_t flags;
  uint32_t buttons = 0;
  BaseType_t xHigherPriorityTaskWoken = false;

#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  SysViewLogStart();
#endif
  flags = GPIO_PortGetInterruptFlags(GPIOB);
  if (flags&(1U<<BUTTONS_PINS_NAVCENTER_PIN)) {
    buttons |= BTN_BIT_NAV_CENTER;
  }
  if (flags&(1U<<BUTTONS_PINS_NAVDOWN_PIN)) {
    buttons |= BTN_BIT_NAV_DOWN;
  }
  if (flags&(1U<<BUTTONS_PINS_NAVUP_PIN)) {
    buttons |= BTN_BIT_NAV_UP;
  }
  GPIO_PortClearInterruptFlags(GPIOB, flags);
  Debounce_StartDebounceFromISR(buttons, &xHigherPriorityTaskWoken);
#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  SysViewLogEnd();
#endif
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  __DSB(); /* need barrier due ARM Cortex bug */
}
#endif

#if McuLib_CONFIG_CPU_IS_LPC && PL_CONFIG_USE_BUTTONS_IRQ
static void pint_intr_callback(pint_pin_int_t pintr, uint32_t pmatch_status) {
  BaseType_t xHigherPriorityTaskWoken = false;

#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  SysViewLogStart();
#endif
  switch(pintr) {
    case kPINT_PinInt0: Debounce_StartDebounceFromISR(BTN_BIT_USER, &xHigherPriorityTaskWoken); break;
    case kPINT_PinInt1: Debounce_StartDebounceFromISR(BTN_BIT_NAV_UP, &xHigherPriorityTaskWoken); break;
    case kPINT_PinInt2: Debounce_StartDebounceFromISR(BTN_BIT_NAV_DOWN, &xHigherPriorityTaskWoken); break;
    case kPINT_PinInt3: Debounce_StartDebounceFromISR(BTN_BIT_NAV_LEFT, &xHigherPriorityTaskWoken); break;
    case kPINT_PinInt4: Debounce_StartDebounceFromISR(BTN_BIT_NAV_RIGHT, &xHigherPriorityTaskWoken); break;
    case kPINT_PinInt5: Debounce_StartDebounceFromISR(BTN_BIT_NAV_CENTER, &xHigherPriorityTaskWoken); break;
    default:
      break;
  }
#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  SysViewLogEnd();
#endif
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
#endif

#if McuLib_CONFIG_CPU_IS_RPxxxx && PL_CONFIG_USE_BUTTONS_IRQ
static void gpio_IsrCallback(uint gpio, uint32_t events) {
  uint32_t button = 0; /* init */
  BaseType_t xHigherPriorityTaskWoken = false;

  switch(gpio) {
    case BUTTONS_PINS_NAVCENTER_PIN:
      button = BTN_BIT_NAV_CENTER;
      break;
    case BUTTONS_PINS_NAVUP_PIN:
      button = BTN_BIT_NAV_UP;
      break;
    case BUTTONS_PINS_NAVDOWN_PIN:
      button = BTN_BIT_NAV_DOWN;
      break;
    case BUTTONS_PINS_NAVLEFT_PIN:
      button = BTN_BIT_NAV_LEFT;
      break;
    case BUTTONS_PINS_NAVRIGHT_PIN:
      button = BTN_BIT_NAV_RIGHT;
      break;
    default:
      button = 0;
      break;
  }
  if (button!=0) {
    Debounce_StartDebounceFromISR(button, &xHigherPriorityTaskWoken);
  }
}
#endif

#if PL_CONFIG_USE_SHELL
static uint8_t PrintStatus(McuShell_ConstStdIOType *io) {
  uint8_t buf[64];

  McuShell_SendStatusStr((const unsigned char*)"button", (const unsigned char*)"button status\r\n", io->stdOut);
  McuBtn_GetPinStatusString(BTN_Infos[BTN_NAV_UP].handle, buf, sizeof(buf));
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((const unsigned char*)"  up", (const unsigned char*)buf, io->stdOut);
  McuBtn_GetPinStatusString(BTN_Infos[BTN_NAV_DOWN].handle, buf, sizeof(buf));
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((const unsigned char*)"  down", (const unsigned char*)buf, io->stdOut);
  McuBtn_GetPinStatusString(BTN_Infos[BTN_NAV_LEFT].handle, buf, sizeof(buf));
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((const unsigned char*)"  left", (const unsigned char*)buf, io->stdOut);
  McuBtn_GetPinStatusString(BTN_Infos[BTN_NAV_RIGHT].handle, buf, sizeof(buf));
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((const unsigned char*)"  right", (const unsigned char*)buf, io->stdOut);
  McuBtn_GetPinStatusString(BTN_Infos[BTN_NAV_CENTER].handle, buf, sizeof(buf));
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((const unsigned char*)"  center", (const unsigned char*)buf, io->stdOut);
  return ERR_OK;
}

static uint8_t PrintHelp(McuShell_ConstStdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"button", (const unsigned char*)"Group of button commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (const unsigned char*)"Print help or status information\r\n", io->stdOut);
  return ERR_OK;
}

uint8_t BTN_ParseCommand(const uint8_t *cmd, bool *handled, McuShell_ConstStdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "button help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "button status")==0)) {
    *handled = TRUE;
    return PrintStatus(io);
  }
  return ERR_OK; /* no error */
}
#endif /* PL_CONFIG_USE_SHELL */

#if McuLib_CONFIG_CPU_IS_ESP32 && PL_CONFIG_USE_BUTTONS_IRQ
static void IRAM_ATTR gpio_interrupt_handler(void *args) {
  int gpio = (int)args;
  uint32_t button = 0; /* init */
  BaseType_t xHigherPriorityTaskWoken = false;

  switch(gpio) {
    case BUTTONS_PINS_NAVCENTER_PIN:
      button = BTN_BIT_NAV_CENTER;
      break;
    case BUTTONS_PINS_NAVUP_PIN:
      button = BTN_BIT_NAV_UP;
      break;
    case BUTTONS_PINS_NAVDOWN_PIN:
      button = BTN_BIT_NAV_DOWN;
      break;
    case BUTTONS_PINS_NAVLEFT_PIN:
      button = BTN_BIT_NAV_LEFT;
      break;
    case BUTTONS_PINS_NAVRIGHT_PIN:
      button = BTN_BIT_NAV_RIGHT;
      break;
    default:
      button = 0;
      break;
  }
  if (button!=0) {
   Debounce_StartDebounceFromISR(button, &xHigherPriorityTaskWoken);
  }
}
#endif

void BTN_Deinit(void) {
#if McuLib_CONFIG_CPU_IS_KINETIS
  DisableIRQ(PORTA_IRQn);
  DisableIRQ(PORTB_IRQn);
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

  btnConfig.hw.gpio = BUTTONS_PINS_NAVCENTER_GPIO;
  btnConfig.hw.port = BUTTONS_PINS_NAVCENTER_PORT;
  btnConfig.hw.pin = BUTTONS_PINS_NAVCENTER_PIN;
#if PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_TINYK22_APROG_HAT_V7 /* V7 does not have hardware pull-ups on the board */
  btnConfig.hw.pull = McuGPIO_PULL_UP;
#endif
  BTN_Infos[BTN_NAV_CENTER].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.gpio = BUTTONS_PINS_NAVLEFT_GPIO;
  btnConfig.hw.port = BUTTONS_PINS_NAVLEFT_PORT;
  btnConfig.hw.pin = BUTTONS_PINS_NAVLEFT_PIN;
#if PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_TINYK22_APROG_HAT_V7 /* V7 does not have hardware pull-ups on the board */
  btnConfig.hw.pull = McuGPIO_PULL_UP;
#endif
  BTN_Infos[BTN_NAV_LEFT].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.gpio = BUTTONS_PINS_NAVRIGHT_GPIO;
  btnConfig.hw.port = BUTTONS_PINS_NAVRIGHT_PORT;
  btnConfig.hw.pin = BUTTONS_PINS_NAVRIGHT_PIN;
#if PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_TINYK22_APROG_HAT_V7 /* V7 does not have hardware pull-ups on the board */
  btnConfig.hw.pull = McuGPIO_PULL_UP;
#endif
  BTN_Infos[BTN_NAV_RIGHT].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.gpio = BUTTONS_PINS_NAVUP_GPIO;
  btnConfig.hw.port = BUTTONS_PINS_NAVUP_PORT;
  btnConfig.hw.pin = BUTTONS_PINS_NAVUP_PIN;
#if PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_TINYK22_APROG_HAT_V7 /* V7 does not have hardware pull-ups on the board */
  btnConfig.hw.pull = McuGPIO_PULL_UP;
#endif
  BTN_Infos[BTN_NAV_UP].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.gpio = BUTTONS_PINS_NAVDOWN_GPIO;
  btnConfig.hw.port = BUTTONS_PINS_NAVDOWN_PORT;
  btnConfig.hw.pin = BUTTONS_PINS_NAVDOWN_PIN;
#if PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_TINYK22_APROG_HAT_V7 /* V7 does not have hardware pull-ups on the board */
  btnConfig.hw.pull = McuGPIO_PULL_UP;
#endif
  BTN_Infos[BTN_NAV_DOWN].handle = McuBtn_InitButton(&btnConfig);

  #if PL_CONFIG_USE_BUTTONS_IRQ
    PORT_SetPinInterruptConfig(BUTTONS_PINS_NAVLEFT_PORT,  BUTTONS_PINS_NAVLEFT_PIN,  kPORT_InterruptFallingEdge);
    PORT_SetPinInterruptConfig(BUTTONS_PINS_NAVRIGHT_PORT, BUTTONS_PINS_NAVRIGHT_PIN, kPORT_InterruptFallingEdge);
    NVIC_SetPriority(PORTA_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    EnableIRQ(PORTA_IRQn);

    PORT_SetPinInterruptConfig(BUTTONS_PINS_NAVDOWN_PORT, BUTTONS_PINS_NAVDOWN_PIN, kPORT_InterruptFallingEdge);
    PORT_SetPinInterruptConfig(BUTTONS_PINS_NAVUP_PORT,   BUTTONS_PINS_NAVUP_PIN,   kPORT_InterruptFallingEdge);
    PORT_SetPinInterruptConfig(BUTTONS_PINS_NAVCENTER_PORT, BUTTONS_PINS_NAVCENTER_PIN, kPORT_InterruptFallingEdge);
    NVIC_SetPriority(PORTB_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    EnableIRQ(PORTB_IRQn);
  #endif
#elif McuLib_CONFIG_CPU_IS_LPC
  /* user button on LPC845-BRK board: PIO0_4 */

  McuBtn_Config_t btnConfig;

  BUTTONS_ENABLE_CLOCK();
  McuBtn_GetDefaultConfig(&btnConfig);
  btnConfig.isLowActive = true;

  btnConfig.hw.gpio = BUTTONS_PINS_USER_GPIO;
  btnConfig.hw.port = BUTTONS_PINS_USER_PORT;
  btnConfig.hw.pin = BUTTONS_PINS_USER_PIN;
  btnConfig.hw.iocon = BUTTONS_PINS_USER_IOCON;
  BTN_Infos[BTN_USER].handle = McuBtn_InitButton(&btnConfig);
  SYSCON_AttachSignal(SYSCON, kPINT_PinInt0, BUTTONS_PINS_USER_PINTSEL);

  btnConfig.hw.gpio = BUTTONS_PINS_NAVUP_GPIO;
  btnConfig.hw.port = BUTTONS_PINS_NAVUP_PORT;
  btnConfig.hw.pin = BUTTONS_PINS_NAVUP_PIN;
  btnConfig.hw.iocon = BUTTONS_PINS_NAVUP_IOCON;
  btnConfig.hw.pull = McuGPIO_PULL_UP;
  BTN_Infos[BTN_NAV_UP].handle = McuBtn_InitButton(&btnConfig);
  SYSCON_AttachSignal(SYSCON, kPINT_PinInt1, BUTTONS_PINS_NAVUP_PINTSEL);

  btnConfig.hw.gpio = BUTTONS_PINS_NAVDOWN_GPIO;
  btnConfig.hw.port = BUTTONS_PINS_NAVDOWN_PORT;
  btnConfig.hw.pin = BUTTONS_PINS_NAVDOWN_PIN;
  btnConfig.hw.iocon = BUTTONS_PINS_NAVDOWN_IOCON;
  btnConfig.hw.pull = McuGPIO_PULL_UP;
  BTN_Infos[BTN_NAV_DOWN].handle = McuBtn_InitButton(&btnConfig);
  SYSCON_AttachSignal(SYSCON, kPINT_PinInt2, BUTTONS_PINS_NAVDOWN_PINTSEL);

  btnConfig.hw.gpio = BUTTONS_PINS_NAVLEFT_GPIO;
  btnConfig.hw.port = BUTTONS_PINS_NAVLEFT_PORT;
  btnConfig.hw.pin = BUTTONS_PINS_NAVLEFT_PIN;
  btnConfig.hw.iocon = BUTTONS_PINS_NAVLEFT_IOCON;
  btnConfig.hw.pull = McuGPIO_PULL_UP;
  BTN_Infos[BTN_NAV_LEFT].handle = McuBtn_InitButton(&btnConfig);
  SYSCON_AttachSignal(SYSCON, kPINT_PinInt3, BUTTONS_PINS_NAVLEFT_PINTSEL);

  btnConfig.hw.gpio = BUTTONS_PINS_NAVRIGHT_GPIO;
  btnConfig.hw.port = BUTTONS_PINS_NAVRIGHT_PORT;
  btnConfig.hw.pin = BUTTONS_PINS_NAVRIGHT_PIN;
  btnConfig.hw.iocon = BUTTONS_PINS_NAVRIGHT_IOCON;
  btnConfig.hw.pull = McuGPIO_PULL_UP;
  BTN_Infos[BTN_NAV_RIGHT].handle = McuBtn_InitButton(&btnConfig);
  SYSCON_AttachSignal(SYSCON, kPINT_PinInt4, BUTTONS_PINS_NAVRIGHT_PINTSEL);

  btnConfig.hw.gpio = BUTTONS_PINS_NAVCENTER_GPIO;
  btnConfig.hw.port = BUTTONS_PINS_NAVCENTER_PORT;
  btnConfig.hw.pin = BUTTONS_PINS_NAVCENTER_PIN;
  btnConfig.hw.iocon = BUTTONS_PINS_NAVCENTER_IOCON;
  btnConfig.hw.pull = McuGPIO_PULL_UP;
  BTN_Infos[BTN_NAV_CENTER].handle = McuBtn_InitButton(&btnConfig);
  SYSCON_AttachSignal(SYSCON, kPINT_PinInt5, BUTTONS_PINS_NAVCENTER_PINTSEL);

  #if PL_CONFIG_USE_BUTTONS_IRQ
    PINT_Init(PINT); /* Initialize PINT */
    PINT_PinInterruptConfig(PINT, kPINT_PinInt0, kPINT_PinIntEnableFallEdge, pint_intr_callback); /* Setup Pin Interrupt 0 for rising edge */
    PINT_EnableCallbackByIndex(PINT, kPINT_PinInt0); /* Enable callbacks for PINT0 by Index */

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
  #endif
#elif McuLib_CONFIG_CPU_IS_RPxxxx
  McuBtn_Config_t btnConfig;

  BUTTONS_ENABLE_CLOCK();
  McuBtn_GetDefaultConfig(&btnConfig);
  btnConfig.isLowActive = true;

  btnConfig.hw.pin = BUTTONS_PINS_NAVCENTER_PIN;
  btnConfig.hw.pull = McuGPIO_PULL_UP;
  BTN_Infos[BTN_NAV_CENTER].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.pin = BUTTONS_PINS_NAVLEFT_PIN;
  btnConfig.hw.pull = McuGPIO_PULL_UP;
  BTN_Infos[BTN_NAV_LEFT].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.pin = BUTTONS_PINS_NAVRIGHT_PIN;
  btnConfig.hw.pull = McuGPIO_PULL_UP;
  BTN_Infos[BTN_NAV_RIGHT].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.pin = BUTTONS_PINS_NAVUP_PIN;
  btnConfig.hw.pull = McuGPIO_PULL_UP;
  BTN_Infos[BTN_NAV_UP].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.pin = BUTTONS_PINS_NAVDOWN_PIN;
  btnConfig.hw.pull = McuGPIO_PULL_UP;
  BTN_Infos[BTN_NAV_DOWN].handle = McuBtn_InitButton(&btnConfig);
  #if PL_CONFIG_USE_BUTTONS_IRQ
    gpio_set_irq_enabled_with_callback(BUTTONS_PINS_NAVCENTER_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_IsrCallback);
    gpio_set_irq_enabled_with_callback(BUTTONS_PINS_NAVUP_PIN,     GPIO_IRQ_EDGE_FALL, true, &gpio_IsrCallback);
    gpio_set_irq_enabled_with_callback(BUTTONS_PINS_NAVDOWN_PIN,   GPIO_IRQ_EDGE_FALL, true, &gpio_IsrCallback);
    gpio_set_irq_enabled_with_callback(BUTTONS_PINS_NAVLEFT_PIN,   GPIO_IRQ_EDGE_FALL, true, &gpio_IsrCallback);
    gpio_set_irq_enabled_with_callback(BUTTONS_PINS_NAVRIGHT_PIN,  GPIO_IRQ_EDGE_FALL, true, &gpio_IsrCallback);
  #endif
#elif McuLib_CONFIG_CPU_IS_ESP32
  McuBtn_Config_t btnConfig;

  BUTTONS_ENABLE_CLOCK();
  McuBtn_GetDefaultConfig(&btnConfig);
  btnConfig.isLowActive = true;

  btnConfig.hw.pin = BUTTONS_PINS_NAVCENTER_PIN;
  btnConfig.hw.pull = McuGPIO_PULL_DISABLE;
  BTN_Infos[BTN_NAV_CENTER].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.pin = BUTTONS_PINS_NAVLEFT_PIN;
  btnConfig.hw.pull = McuGPIO_PULL_DISABLE;
  BTN_Infos[BTN_NAV_LEFT].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.pin = BUTTONS_PINS_NAVRIGHT_PIN;
  btnConfig.hw.pull = McuGPIO_PULL_DISABLE;
  BTN_Infos[BTN_NAV_RIGHT].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.pin = BUTTONS_PINS_NAVUP_PIN;
  btnConfig.hw.pull = McuGPIO_PULL_DISABLE;
  BTN_Infos[BTN_NAV_UP].handle = McuBtn_InitButton(&btnConfig);

  btnConfig.hw.pin = BUTTONS_PINS_NAVDOWN_PIN;
  btnConfig.hw.pull = McuGPIO_PULL_DISABLE;
  BTN_Infos[BTN_NAV_DOWN].handle = McuBtn_InitButton(&btnConfig);

  #if PL_CONFIG_USE_BUTTONS_IRQ
    #define ESP_INTR_FLAG_DEFAULT 0
    esp_err_t res;

    res = gpio_set_intr_type(BUTTONS_PINS_NAVCENTER_PIN, GPIO_INTR_NEGEDGE); /* set to falling edge */
    if (res!=ESP_OK) {
      McuLog_fatal("Failed setting interrupt type");
      for(;;) {}
    }
    res = gpio_set_intr_type(BUTTONS_PINS_NAVLEFT_PIN, GPIO_INTR_NEGEDGE); /* set to falling edge */
    if (res!=ESP_OK) {
      McuLog_fatal("Failed setting interrupt type");
      for(;;) {}
    }
    res = gpio_set_intr_type(BUTTONS_PINS_NAVRIGHT_PIN, GPIO_INTR_NEGEDGE); /* set to falling edge */
    if (res!=ESP_OK) {
      McuLog_fatal("Failed setting interrupt type");
      for(;;) {}
    }
    res = gpio_set_intr_type(BUTTONS_PINS_NAVUP_PIN, GPIO_INTR_NEGEDGE); /* set to falling edge */
    if (res!=ESP_OK) {
      McuLog_fatal("Failed setting interrupt type");
      for(;;) {}
    }
    res = gpio_set_intr_type(BUTTONS_PINS_NAVDOWN_PIN, GPIO_INTR_NEGEDGE); /* set to falling edge */
    if (res!=ESP_OK) {
      McuLog_fatal("Failed setting interrupt type");
      for(;;) {}
    }
    res = gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    if (res!=ESP_OK) {
      McuLog_fatal("Failed installing interrupt service");
      for(;;) {}
    }
    res = gpio_isr_handler_add(BUTTONS_PINS_NAVCENTER_PIN, gpio_interrupt_handler, (void *)BUTTONS_PINS_NAVCENTER_PIN);
    if (res!=ESP_OK) {
      McuLog_fatal("Failed adding interrupt handler");
      for(;;) {}
    }
    res = gpio_isr_handler_add(BUTTONS_PINS_NAVLEFT_PIN, gpio_interrupt_handler, (void *)BUTTONS_PINS_NAVLEFT_PIN);
    if (res!=ESP_OK) {
      McuLog_fatal("Failed adding interrupt handler");
      for(;;) {}
    }
    res = gpio_isr_handler_add(BUTTONS_PINS_NAVRIGHT_PIN, gpio_interrupt_handler, (void *)BUTTONS_PINS_NAVRIGHT_PIN);
    if (res!=ESP_OK) {
      McuLog_fatal("Failed adding interrupt handler");
      for(;;) {}
    }
    res = gpio_isr_handler_add(BUTTONS_PINS_NAVUP_PIN, gpio_interrupt_handler, (void *)BUTTONS_PINS_NAVUP_PIN);
    if (res!=ESP_OK) {
      McuLog_fatal("Failed adding interrupt handler");
      for(;;) {}
    }
    res = gpio_isr_handler_add(BUTTONS_PINS_NAVDOWN_PIN, gpio_interrupt_handler, (void *)BUTTONS_PINS_NAVDOWN_PIN);
    if (res!=ESP_OK) {
      McuLog_fatal("Failed adding interrupt handler");
      for(;;) {}
    }
  #endif /* PL_CONFIG_USE_BUTTONS_IRQ */

#endif
}

#endif /* PL_CONFIG_USE_BUTTONS */
