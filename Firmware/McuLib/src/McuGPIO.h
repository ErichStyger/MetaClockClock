/*
 * Copyright (c) 2019, Erich Styger
 * All rights reserved.
 *
 * Driver for GPIO pins
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef McuGPIO_H_
#define McuGPIO_H_

#include "fsl_gpio.h"
#include "McuLibconfig.h"
#include "McuGPIOconfig.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *McuGPIO_Handle_t;

typedef struct {
  GPIO_Type *gpio; /* pointer to GPIO */
#if McuLib_CONFIG_CPU_IS_KINETIS
  PORT_Type *port; /* pointer to port */
#elif McuLib_CONFIG_CPU_IS_LPC && McuLib_CONFIG_CORTEX_M==0
  uint32_t port; /* port number */
  uint8_t iocon; /* I/O Connection index used for muxing, e.g. IOCON_INDEX_PIO0_0 */
#elif McuLib_CONFIG_CPU_IS_LPC
  uint32_t port; /* port number */
#elif McuLib_CONFIG_CPU_IS_IMXRT
  /* no port for i.MX */
#endif
  uint32_t pin; /* pin number */
} McuGPIO_HwPin_t;

typedef struct {
  bool isInput;
  bool isHighOnInit;
  McuGPIO_HwPin_t hw;
} McuGPIO_Config_t;

void McuGPIO_GetDefaultConfig(McuGPIO_Config_t *config);

McuGPIO_Handle_t McuGPIO_InitGPIO(McuGPIO_Config_t *config);

McuGPIO_Handle_t McuGPIO_DeinitGPIO(McuGPIO_Handle_t gpio);

void McuGPIO_SetAsInput(McuGPIO_Handle_t gpio);
void McuGPIO_SetAsOutput(McuGPIO_Handle_t gpio, bool setHigh);
bool McuGPIO_IsInput(McuGPIO_Handle_t gpio);
bool McuGPIO_IsOutput(McuGPIO_Handle_t gpio);
void McuGPIO_SetLow(McuGPIO_Handle_t gpio);
void McuGPIO_SetHigh(McuGPIO_Handle_t gpio);
void McuGPIO_Toggle(McuGPIO_Handle_t gpio);
bool McuGPIO_IsLow(McuGPIO_Handle_t gpio);
bool McuGPIO_IsHigh(McuGPIO_Handle_t gpio);
void McuGPIO_SetValue(McuGPIO_Handle_t gpio, bool val);
bool McuGPIO_GetValue(McuGPIO_Handle_t gpio);

void McuGPIO_GetPinStatusString(McuGPIO_Handle_t gpio, unsigned char *buf, size_t bufSize);

typedef enum {
  McuGPIO_PULL_DISABLE,
  McuGPIO_PULL_UP,
  McuGPIO_PULL_DOWN,
} McuGPIO_PullType;

void McuGPIO_SetPullResistor(McuGPIO_Handle_t gpio, McuGPIO_PullType pull);

/* driver initialization */
void McuGPIO_Init(void);

/* driver de-initialization */
void McuGPIO_Deinit(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* McuGPIO_H_ */
