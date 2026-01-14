/*
 * Copyright (c) 2015 Freescale Semiconductor, Inc.
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "bootloader_config.h"
#include "bl_irq_common.h"
#include "fsl_device_registers.h"
#include "fsl_dspi.h"
#include "bootloader_common.h"
#include <assert.h>

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
#if defined(BL_CONFIG_DSPI) && BL_CONFIG_DSPI
static const IRQn_Type dspi_irqs[] = SPI_IRQS;

void DSPI_SetSystemIRQ(uint32_t instance, PeripheralSystemIRQSetting set)
{
    if (set == kPeripheralEnableIRQ)
    {
        NVIC_EnableIRQ(dspi_irqs[instance]);
    }
    else
    {
        NVIC_DisableIRQ(dspi_irqs[instance]);
    }
}
#endif //BL_CONFIG_DSPI
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
