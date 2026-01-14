/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PD_DP_BOARD_CHIP_H__
#define __PD_DP_BOARD_CHIP_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define USB_PD_DP_BOARD_CHIP_TASK_HPD_DRIVER_EVENT (0x01u)

typedef struct _pd_dp_board_chip_config
{
    void *crossbarConfig;
    void *hpdDriverConfig;
    void *ptn36502Config;
} pd_dp_peripheral_config_t;

/*******************************************************************************
 * API
 ******************************************************************************/

pd_status_t PD_DpBoardChipsInit(void **interfaceHandle, pd_handle pdHandle, const void *param);

pd_status_t PD_DpBoardChipsDeinit(void *interfaceHandle);

pd_status_t PD_DpBoardChipsControl(void *interfaceHandle, uint32_t opCode, void *opParam);

extern void PD_DpBoardChip1msIsr(void);

extern void PD_DpBoardChipTask(void);

#endif
