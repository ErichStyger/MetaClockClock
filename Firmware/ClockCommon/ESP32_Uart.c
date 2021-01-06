/*
 * ESP32_Uart.c
 *
 * Copyright (c) 2020: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_ESP32_UART
#include "esp32.h"
#include "McuShell.h"
#include "McuRTOS.h"

/* UART1 on K22FN512. Mux the pins using the pins muxing tool */
#include "fsl_uart.h"
#define Esp32ShellUart_CONFIG_UART_DEVICE                   UART1
#define Esp32ShellUart_CONFIG_UART_SET_UART_CLOCK()         /* nothing needed */
#define Esp32ShellUart_CONFIG_UART_WRITE_BLOCKING           UART_WriteBlocking
#define Esp32ShellUart_CONFIG_UART_GET_FLAGS                UART_GetStatusFlags
#define Esp32ShellUart_CONFIG_UART_HW_RX_READY_FLAGS        (kUART_RxDataRegFullFlag|kUART_RxOverrunFlag)
#define Esp32ShellUart_CONFIG_UART_READ_BYTE                UART_ReadByte
#define Esp32ShellUart_CONFIG_UART_CONFIG_STRUCT            uart_config_t
#define Esp32ShellUart_CONFIG_UART_GET_DEFAULT_CONFIG       UART_GetDefaultConfig
#define Esp32ShellUart_CONFIG_UART_ENABLE_INTERRUPTS        UART_EnableInterrupts
#define Esp32ShellUart_CONFIG_UART_ENABLE_INTERRUPT_FLAGS   (kUART_RxDataRegFullInterruptEnable | kUART_RxOverrunInterruptEnable)
#define Esp32ShellUart_CONFIG_UART_IRQ_NUMBER               UART1_RX_TX_IRQn
#define Esp32ShellUart_CONFIG_UART_INIT                     UART_Init
#define Esp32ShellUart_CONFIG_UART_GET_CLOCK_FREQ_SELECT    kCLOCK_CoreSysClk
#define Esp32ShellUart_CONFIG_UART_IRQ_HANDLER              UART1_RX_TX_IRQHandler
#define Esp32ShellUart_CONFIG_CLEAR_STATUS_FLAGS            UART_ClearStatusFlags

#ifndef Esp32ShellUart_CONFIG_UART_RX_QUEUE_LENGTH
  #define Esp32ShellUart_CONFIG_UART_RX_QUEUE_LENGTH    (McuShell_CONFIG_DEFAULT_SHELL_BUFFER_SIZE)
#endif

#ifndef Esp32ShellUart_CONFIG_UART_BAUDRATE
  #define Esp32ShellUart_CONFIG_UART_BAUDRATE           115200
#endif


uint8_t Esp32ShellUart_DefaultShellBuffer[McuShell_DEFAULT_SHELL_BUFFER_SIZE]; /* default buffer which can be used by the application */
McuShell_ConstStdIOType Esp32ShellUart_stdio;

static QueueHandle_t uartRxQueue;

/*********************************************************************************************************/
/* Shell interface */
/*********************************************************************************************************/
static void Esp32ShellUart_SendChar(unsigned char ch) {
  Esp32ShellUart_CONFIG_UART_WRITE_BLOCKING(Esp32ShellUart_CONFIG_UART_DEVICE, &ch, 1);
}

static void Esp32ShellUart_ReadChar(uint8_t *c) {
  uint8_t ch;

  if (xQueueReceive(uartRxQueue, &ch, 0)==pdPASS ) {
    *c = ch; /* return received character */
  } else {
    *c = '\0'; /* nothing received */
  }
}

static bool Esp32ShellUart_CharPresent(void) {
  return uxQueueMessagesWaiting(uartRxQueue)!=0;
}

McuShell_ConstStdIOType Esp32ShellUart_stdio = {
    (McuShell_StdIO_In_FctType)Esp32ShellUart_ReadChar, /* stdin */
    (McuShell_StdIO_OutErr_FctType)Esp32ShellUart_SendChar,  /* stdout */
    (McuShell_StdIO_OutErr_FctType)Esp32ShellUart_SendChar,  /* stderr */
    Esp32ShellUart_CharPresent /* if input is not empty */
  };

uint8_t Esp32ShellUart_DefaultShellBuffer[McuShell_DEFAULT_SHELL_BUFFER_SIZE]; /* default buffer which can be used by the application */
/*********************************************************************************************************/
void Esp32ShellUart_CONFIG_UART_IRQ_HANDLER(void) {
  uint8_t data;
  uint32_t flags;
  BaseType_t xHigherPriorityTaskWoken;

  flags = Esp32ShellUart_CONFIG_UART_GET_FLAGS(Esp32ShellUart_CONFIG_UART_DEVICE);
  /* If new data arrived. */
  if (flags&Esp32ShellUart_CONFIG_UART_HW_RX_READY_FLAGS) {
    data = Esp32ShellUart_CONFIG_UART_READ_BYTE(Esp32ShellUart_CONFIG_UART_DEVICE);
    (void)xQueueSendFromISR(uartRxQueue, &data, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken != pdFALSE) {
      vPortYieldFromISR();
    }
  }
  Esp32ShellUart_CONFIG_CLEAR_STATUS_FLAGS(Esp32ShellUart_CONFIG_UART_DEVICE, flags);
  __DSB();
}

static void InitUart(void) {
  /* NOTE: Muxing of the UART pins and clocking of the UART needs to be done in the Pins/clocks tool! */
  Esp32ShellUart_CONFIG_UART_CONFIG_STRUCT config;

  Esp32ShellUart_CONFIG_UART_SET_UART_CLOCK();
  Esp32ShellUart_CONFIG_UART_GET_DEFAULT_CONFIG(&config);
  config.baudRate_Bps = Esp32ShellUart_CONFIG_UART_BAUDRATE;
  config.enableRx     = true;
  config.enableTx     = true;

  /* Initialize the USART with configuration. */
  Esp32ShellUart_CONFIG_UART_INIT(Esp32ShellUart_CONFIG_UART_DEVICE, &config, CLOCK_GetFreq(Esp32ShellUart_CONFIG_UART_GET_CLOCK_FREQ_SELECT));
  Esp32ShellUart_CONFIG_UART_ENABLE_INTERRUPTS(Esp32ShellUart_CONFIG_UART_DEVICE, Esp32ShellUart_CONFIG_UART_ENABLE_INTERRUPT_FLAGS);
  EnableIRQ(Esp32ShellUart_CONFIG_UART_IRQ_NUMBER);
  NVIC_SetPriority(Esp32ShellUart_CONFIG_UART_IRQ_NUMBER, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
}

void Esp32ShellUart_Deinit(void) {
  vQueueDelete(uartRxQueue);
  uartRxQueue = NULL;
}

void Esp32ShellUart_Init(void) {
  InitUart();
  uartRxQueue = xQueueCreate(Esp32ShellUart_CONFIG_UART_RX_QUEUE_LENGTH, sizeof(uint8_t));
  if (uartRxQueue==NULL) {
    for(;;){} /* out of memory? */
  }
  vQueueAddToRegistry(uartRxQueue, "Esp32UartRxQueue");
}

#endif /* PL_CONFIG_USE_ESP32_UART */
