/*
 * Copyright (c) 2014 Freescale Semiconductor, Inc.
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if !defined(__TARGET_CONFIG_H__)
#define __TARGET_CONFIG_H__

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

//! @brief Constants for clock frequencies.
enum _target_clocks
{
    kDefaultClock = 20971520u,
    kHIRC = 48000000u,
    kMinCoreClockWithUsbSupport = 20000000u,
    kMaxCoreClock = 120000000u,
    kMaxBusClock = 60000000u,
    kMaxFlexBusClock = 30000000u,
    kMaxFlashClock = 25000000u,

    kDivider_Min = 1,
    kDivider_Max = 16,
};

//! @brief Memory Map index constants
enum
{
    kIndexFlashArray = 0,
    kIndexSRAM = 1,
    kSRAMSeparatrix = (uint32_t)0x20000000 //!< This value is the start address of SRAM_U
};

//! @brief Constants for sram partition
enum _sram_partition
{
    kSram_LowerPart = 1,
    kSram_UpperPart = 1,
};

//!@brief Unique ID constants
enum _uid_constrants
{
    kUniqueId_SizeInBytes = 16,
};

//! @brief Version constants for the target.
enum _target_version_constants
{
    kTarget_Version_Name = 'T',
    kTarget_Version_Major = 1,
    kTarget_Version_Minor = 0,
    kTarget_Version_Bugfix = 0
};

#endif // __TARGET_CONFIG_H__
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
