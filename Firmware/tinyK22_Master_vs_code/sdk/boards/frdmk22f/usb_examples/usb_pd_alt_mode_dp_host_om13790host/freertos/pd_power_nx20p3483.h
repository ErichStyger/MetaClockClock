/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PD_POWER_NX20P3483_T__
#define __PD_POWER_NX20P3483_T__

typedef struct _pd_nx20p_config
{
    uint8_t nx20pI2CMaster;
    uint8_t nx20pSlaveAddress;
    uint32_t nx20pI2cSrcClock;
    void *nx20pI2cReleaseBus;
} pd_nx20p_config_t;

void PD_NX20PSetSinkOVP(uint8_t port, uint16_t voltage);
void PD_NX20PSet5VSourceOCP(uint8_t port, uint16_t current);
void PD_NX20PInit(uint8_t port, uint32_t source5VOcpCurrent, uint32_t sinkOvpVoltage);
void PD_NX20PDeinit(uint8_t port);
void PD_NX20PClearInt(uint8_t port);
void PD_NX20PExitDeadBatteryMode(uint8_t port);

#endif
