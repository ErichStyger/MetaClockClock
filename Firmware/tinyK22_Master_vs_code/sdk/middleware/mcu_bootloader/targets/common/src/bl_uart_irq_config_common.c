/*
 * Copyright (c) 2014 Freescale Semiconductor, Inc.
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "bl_context.h"
#include "bootloader_common.h"
#include "bootloader_config.h"
#include "bl_irq_common.h"
#include "autobaud.h"
#include "serial_packet.h"
#include "fsl_device_registers.h"
#include "fsl_uart.h"
#include "fsl_assert.h"

#if (BL_CONFIG_SCUART || BL_CONFIG_UART)
static const IRQn_Type uart_irqs[] = UART_RX_TX_IRQS;

void UART_SetSystemIRQ(uint32_t instance, PeripheralSystemIRQSetting set)
{
    if (set == kPeripheralEnableIRQ)
    {
        NVIC_EnableIRQ(uart_irqs[instance]);
    }
    else
    {
        NVIC_DisableIRQ(uart_irqs[instance]);
    }
}
#endif //BL_CONFIG_SCUART || BL_CONFIG_UART
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
