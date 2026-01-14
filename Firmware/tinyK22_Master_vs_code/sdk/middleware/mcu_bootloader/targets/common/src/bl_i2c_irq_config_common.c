/*
 * Copyright (c) 2015 Freescale Semiconductor, Inc.
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
#include "fsl_i2c.h"
#include "fsl_assert.h"

#if defined(BL_CONFIG_I2C) && BL_CONFIG_I2C
static const IRQn_Type i2c_irqs[] = I2C_IRQS;

void I2C_SetSystemIRQ(uint32_t instance, PeripheralSystemIRQSetting set)
{
    if (set == kPeripheralEnableIRQ)
    {
        NVIC_EnableIRQ(i2c_irqs[instance]);
    }
    else
    {
        NVIC_DisableIRQ(i2c_irqs[instance]);
    }
}
#endif //BL_CONFIG_I2C
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
