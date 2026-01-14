/*
** ###################################################################
**     Processors:          MK22FN512CAP12
**                          MK22FN512VDC12
**                          MK22FN512VFX12
**                          MK22FN512VLH12
**                          MK22FN512VLL12
**                          MK22FN512VMP12
**
**     Version:             rev. 3.0, 2024-10-29
**     Build:               b250520
**
**     Abstract:
**         CMSIS Peripheral Access Layer for FMC
**
**     Copyright 1997-2016 Freescale Semiconductor, Inc.
**     Copyright 2016-2025 NXP
**     SPDX-License-Identifier: BSD-3-Clause
**
**     http:                 www.nxp.com
**     mail:                 support@nxp.com
**
**     Revisions:
**     - rev. 1.0 (2013-07-23)
**         Initial version.
**     - rev. 1.1 (2013-09-17)
**         RM rev. 0.4 update.
**     - rev. 2.0 (2013-10-29)
**         Register accessor macros added to the memory map.
**         Symbols for Processor Expert memory map compatibility added to the memory map.
**         Startup file for gcc has been updated according to CMSIS 3.2.
**         System initialization updated.
**     - rev. 2.1 (2013-10-30)
**         Definition of BITBAND macros updated to support peripherals with 32-bit acces disabled.
**     - rev. 2.2 (2013-12-20)
**         Update according to reference manual rev. 0.6,
**     - rev. 2.3 (2014-01-13)
**         Update according to reference manual rev. 0.61,
**     - rev. 2.4 (2014-02-10)
**         The declaration of clock configurations has been moved to separate header file system_MK22F51212.h
**     - rev. 2.5 (2014-05-06)
**         Update according to reference manual rev. 1.0,
**         Update of system and startup files.
**         Module access macro module_BASES replaced by module_BASE_PTRS.
**     - rev. 2.6 (2014-08-28)
**         Update of system files - default clock configuration changed.
**         Update of startup files - possibility to override DefaultISR added.
**     - rev. 2.7 (2014-10-14)
**         Interrupt INT_LPTimer renamed to INT_LPTMR0, interrupt INT_Watchdog renamed to INT_WDOG_EWM.
**     - rev. 2.8 (2015-02-19)
**         Renamed interrupt vector LLW to LLWU.
**     - rev. 2.9 (2016-03-21)
**         Added MK22FN512VFX12 part.
**         GPIO - renamed port instances: PTx -> GPIOx.
**     - rev. 3.0 (2024-10-29)
**         Change the device header file from single flat file to multiple files based on peripherals,
**         each peripheral with dedicated header file located in periphN folder.
**
** ###################################################################
*/

/*!
 * @file PERI_FMC.h
 * @version 3.0
 * @date 2024-10-29
 * @brief CMSIS Peripheral Access Layer for FMC
 *
 * CMSIS Peripheral Access Layer for FMC
 */

#if !defined(PERI_FMC_H_)
#define PERI_FMC_H_                              /**< Symbol preventing repeated inclusion */

#if (defined(CPU_MK22FN512CAP12) || defined(CPU_MK22FN512VDC12) || defined(CPU_MK22FN512VFX12) || defined(CPU_MK22FN512VLH12) || defined(CPU_MK22FN512VLL12) || defined(CPU_MK22FN512VMP12))
#include "MK22F51212_COMMON.h"
#else
  #error "No valid CPU defined!"
#endif

/* ----------------------------------------------------------------------------
   -- Device Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup Peripheral_access_layer Device Peripheral Access Layer
 * @{
 */


/*
** Start of section using anonymous unions
*/

#if defined(__ARMCC_VERSION)
  #if (__ARMCC_VERSION >= 6010050)
    #pragma clang diagnostic push
  #else
    #pragma push
    #pragma anon_unions
  #endif
#elif defined(__CWCC__)
  #pragma push
  #pragma cpp_extensions on
#elif defined(__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined(__IAR_SYSTEMS_ICC__)
  #pragma language=extended
#else
  #error Not supported compiler type
#endif

/* ----------------------------------------------------------------------------
   -- FMC Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup FMC_Peripheral_Access_Layer FMC Peripheral Access Layer
 * @{
 */

/** FMC - Size of Registers Arrays */
#define FMC_TAGVDW0S_COUNT                        8u
#define FMC_TAGVDW1S_COUNT                        8u
#define FMC_TAGVDW2S_COUNT                        8u
#define FMC_TAGVDW3S_COUNT                        8u
#define FMC_WAY_COUNT                             4u
#define FMC_WAY_SET_COUNT                         8u

/** FMC - Register Layout Typedef */
typedef struct {
  __IO uint32_t PFAPR;                             /**< Flash Access Protection Register, offset: 0x0 */
  __IO uint32_t PFB0CR;                            /**< Flash Bank 0 Control Register, offset: 0x4 */
  __IO uint32_t PFB1CR;                            /**< Flash Bank 1 Control Register, offset: 0x8 */
       uint8_t RESERVED_0[244];
  __IO uint32_t TAGVDW0S[FMC_TAGVDW0S_COUNT];      /**< Cache Tag Storage, array offset: 0x100, array step: 0x4 */
  __IO uint32_t TAGVDW1S[FMC_TAGVDW1S_COUNT];      /**< Cache Tag Storage, array offset: 0x120, array step: 0x4 */
  __IO uint32_t TAGVDW2S[FMC_TAGVDW2S_COUNT];      /**< Cache Tag Storage, array offset: 0x140, array step: 0x4 */
  __IO uint32_t TAGVDW3S[FMC_TAGVDW3S_COUNT];      /**< Cache Tag Storage, array offset: 0x160, array step: 0x4 */
       uint8_t RESERVED_1[128];
  struct {                                         /* offset: 0x200, array step: index*0x40, index2*0x8 */
    __IO uint32_t DATA_U;                            /**< Cache Data Storage (upper word), array offset: 0x200, array step: index*0x40, index2*0x8 */
    __IO uint32_t DATA_L;                            /**< Cache Data Storage (lower word), array offset: 0x204, array step: index*0x40, index2*0x8 */
  } SET[FMC_WAY_COUNT][FMC_WAY_SET_COUNT];
} FMC_Type;

/* ----------------------------------------------------------------------------
   -- FMC Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup FMC_Register_Masks FMC Register Masks
 * @{
 */

/*! @name PFAPR - Flash Access Protection Register */
/*! @{ */

#define FMC_PFAPR_M0AP_MASK                      (0x3U)
#define FMC_PFAPR_M0AP_SHIFT                     (0U)
/*! M0AP - Master 0 Access Protection
 *  0b00..No access may be performed by this master
 *  0b01..Only read accesses may be performed by this master
 *  0b10..Only write accesses may be performed by this master
 *  0b11..Both read and write accesses may be performed by this master
 */
#define FMC_PFAPR_M0AP(x)                        (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M0AP_SHIFT)) & FMC_PFAPR_M0AP_MASK)

#define FMC_PFAPR_M1AP_MASK                      (0xCU)
#define FMC_PFAPR_M1AP_SHIFT                     (2U)
/*! M1AP - Master 1 Access Protection
 *  0b00..No access may be performed by this master
 *  0b01..Only read accesses may be performed by this master
 *  0b10..Only write accesses may be performed by this master
 *  0b11..Both read and write accesses may be performed by this master
 */
#define FMC_PFAPR_M1AP(x)                        (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M1AP_SHIFT)) & FMC_PFAPR_M1AP_MASK)

#define FMC_PFAPR_M2AP_MASK                      (0x30U)
#define FMC_PFAPR_M2AP_SHIFT                     (4U)
/*! M2AP - Master 2 Access Protection
 *  0b00..No access may be performed by this master
 *  0b01..Only read accesses may be performed by this master
 *  0b10..Only write accesses may be performed by this master
 *  0b11..Both read and write accesses may be performed by this master
 */
#define FMC_PFAPR_M2AP(x)                        (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M2AP_SHIFT)) & FMC_PFAPR_M2AP_MASK)

#define FMC_PFAPR_M3AP_MASK                      (0xC0U)
#define FMC_PFAPR_M3AP_SHIFT                     (6U)
/*! M3AP - Master 3 Access Protection
 *  0b00..No access may be performed by this master
 *  0b01..Only read accesses may be performed by this master
 *  0b10..Only write accesses may be performed by this master
 *  0b11..Both read and write accesses may be performed by this master
 */
#define FMC_PFAPR_M3AP(x)                        (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M3AP_SHIFT)) & FMC_PFAPR_M3AP_MASK)

#define FMC_PFAPR_M4AP_MASK                      (0x300U)
#define FMC_PFAPR_M4AP_SHIFT                     (8U)
/*! M4AP - Master 4 Access Protection
 *  0b00..No access may be performed by this master
 *  0b01..Only read accesses may be performed by this master
 *  0b10..Only write accesses may be performed by this master
 *  0b11..Both read and write accesses may be performed by this master
 */
#define FMC_PFAPR_M4AP(x)                        (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M4AP_SHIFT)) & FMC_PFAPR_M4AP_MASK)

#define FMC_PFAPR_M5AP_MASK                      (0xC00U)
#define FMC_PFAPR_M5AP_SHIFT                     (10U)
/*! M5AP - Master 5 Access Protection
 *  0b00..No access may be performed by this master
 *  0b01..Only read accesses may be performed by this master
 *  0b10..Only write accesses may be performed by this master
 *  0b11..Both read and write accesses may be performed by this master
 */
#define FMC_PFAPR_M5AP(x)                        (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M5AP_SHIFT)) & FMC_PFAPR_M5AP_MASK)

#define FMC_PFAPR_M6AP_MASK                      (0x3000U)
#define FMC_PFAPR_M6AP_SHIFT                     (12U)
/*! M6AP - Master 6 Access Protection
 *  0b00..No access may be performed by this master
 *  0b01..Only read accesses may be performed by this master
 *  0b10..Only write accesses may be performed by this master
 *  0b11..Both read and write accesses may be performed by this master
 */
#define FMC_PFAPR_M6AP(x)                        (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M6AP_SHIFT)) & FMC_PFAPR_M6AP_MASK)

#define FMC_PFAPR_M7AP_MASK                      (0xC000U)
#define FMC_PFAPR_M7AP_SHIFT                     (14U)
/*! M7AP - Master 7 Access Protection
 *  0b00..No access may be performed by this master.
 *  0b01..Only read accesses may be performed by this master.
 *  0b10..Only write accesses may be performed by this master.
 *  0b11..Both read and write accesses may be performed by this master.
 */
#define FMC_PFAPR_M7AP(x)                        (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M7AP_SHIFT)) & FMC_PFAPR_M7AP_MASK)

#define FMC_PFAPR_M0PFD_MASK                     (0x10000U)
#define FMC_PFAPR_M0PFD_SHIFT                    (16U)
/*! M0PFD - Master 0 Prefetch Disable
 *  0b0..Prefetching for this master is enabled.
 *  0b1..Prefetching for this master is disabled.
 */
#define FMC_PFAPR_M0PFD(x)                       (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M0PFD_SHIFT)) & FMC_PFAPR_M0PFD_MASK)

#define FMC_PFAPR_M1PFD_MASK                     (0x20000U)
#define FMC_PFAPR_M1PFD_SHIFT                    (17U)
/*! M1PFD - Master 1 Prefetch Disable
 *  0b0..Prefetching for this master is enabled.
 *  0b1..Prefetching for this master is disabled.
 */
#define FMC_PFAPR_M1PFD(x)                       (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M1PFD_SHIFT)) & FMC_PFAPR_M1PFD_MASK)

#define FMC_PFAPR_M2PFD_MASK                     (0x40000U)
#define FMC_PFAPR_M2PFD_SHIFT                    (18U)
/*! M2PFD - Master 2 Prefetch Disable
 *  0b0..Prefetching for this master is enabled.
 *  0b1..Prefetching for this master is disabled.
 */
#define FMC_PFAPR_M2PFD(x)                       (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M2PFD_SHIFT)) & FMC_PFAPR_M2PFD_MASK)

#define FMC_PFAPR_M3PFD_MASK                     (0x80000U)
#define FMC_PFAPR_M3PFD_SHIFT                    (19U)
/*! M3PFD - Master 3 Prefetch Disable
 *  0b0..Prefetching for this master is enabled.
 *  0b1..Prefetching for this master is disabled.
 */
#define FMC_PFAPR_M3PFD(x)                       (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M3PFD_SHIFT)) & FMC_PFAPR_M3PFD_MASK)

#define FMC_PFAPR_M4PFD_MASK                     (0x100000U)
#define FMC_PFAPR_M4PFD_SHIFT                    (20U)
/*! M4PFD - Master 4 Prefetch Disable
 *  0b0..Prefetching for this master is enabled.
 *  0b1..Prefetching for this master is disabled.
 */
#define FMC_PFAPR_M4PFD(x)                       (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M4PFD_SHIFT)) & FMC_PFAPR_M4PFD_MASK)

#define FMC_PFAPR_M5PFD_MASK                     (0x200000U)
#define FMC_PFAPR_M5PFD_SHIFT                    (21U)
/*! M5PFD - Master 5 Prefetch Disable
 *  0b0..Prefetching for this master is enabled.
 *  0b1..Prefetching for this master is disabled.
 */
#define FMC_PFAPR_M5PFD(x)                       (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M5PFD_SHIFT)) & FMC_PFAPR_M5PFD_MASK)

#define FMC_PFAPR_M6PFD_MASK                     (0x400000U)
#define FMC_PFAPR_M6PFD_SHIFT                    (22U)
/*! M6PFD - Master 6 Prefetch Disable
 *  0b0..Prefetching for this master is enabled.
 *  0b1..Prefetching for this master is disabled.
 */
#define FMC_PFAPR_M6PFD(x)                       (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M6PFD_SHIFT)) & FMC_PFAPR_M6PFD_MASK)

#define FMC_PFAPR_M7PFD_MASK                     (0x800000U)
#define FMC_PFAPR_M7PFD_SHIFT                    (23U)
/*! M7PFD - Master 7 Prefetch Disable
 *  0b0..Prefetching for this master is enabled.
 *  0b1..Prefetching for this master is disabled.
 */
#define FMC_PFAPR_M7PFD(x)                       (((uint32_t)(((uint32_t)(x)) << FMC_PFAPR_M7PFD_SHIFT)) & FMC_PFAPR_M7PFD_MASK)
/*! @} */

/*! @name PFB0CR - Flash Bank 0 Control Register */
/*! @{ */

#define FMC_PFB0CR_B0SEBE_MASK                   (0x1U)
#define FMC_PFB0CR_B0SEBE_SHIFT                  (0U)
/*! B0SEBE - Bank 0 Single Entry Buffer Enable
 *  0b0..Single entry buffer is disabled.
 *  0b1..Single entry buffer is enabled.
 */
#define FMC_PFB0CR_B0SEBE(x)                     (((uint32_t)(((uint32_t)(x)) << FMC_PFB0CR_B0SEBE_SHIFT)) & FMC_PFB0CR_B0SEBE_MASK)

#define FMC_PFB0CR_B0IPE_MASK                    (0x2U)
#define FMC_PFB0CR_B0IPE_SHIFT                   (1U)
/*! B0IPE - Bank 0 Instruction Prefetch Enable
 *  0b0..Do not prefetch in response to instruction fetches.
 *  0b1..Enable prefetches in response to instruction fetches.
 */
#define FMC_PFB0CR_B0IPE(x)                      (((uint32_t)(((uint32_t)(x)) << FMC_PFB0CR_B0IPE_SHIFT)) & FMC_PFB0CR_B0IPE_MASK)

#define FMC_PFB0CR_B0DPE_MASK                    (0x4U)
#define FMC_PFB0CR_B0DPE_SHIFT                   (2U)
/*! B0DPE - Bank 0 Data Prefetch Enable
 *  0b0..Do not prefetch in response to data references.
 *  0b1..Enable prefetches in response to data references.
 */
#define FMC_PFB0CR_B0DPE(x)                      (((uint32_t)(((uint32_t)(x)) << FMC_PFB0CR_B0DPE_SHIFT)) & FMC_PFB0CR_B0DPE_MASK)

#define FMC_PFB0CR_B0ICE_MASK                    (0x8U)
#define FMC_PFB0CR_B0ICE_SHIFT                   (3U)
/*! B0ICE - Bank 0 Instruction Cache Enable
 *  0b0..Do not cache instruction fetches.
 *  0b1..Cache instruction fetches.
 */
#define FMC_PFB0CR_B0ICE(x)                      (((uint32_t)(((uint32_t)(x)) << FMC_PFB0CR_B0ICE_SHIFT)) & FMC_PFB0CR_B0ICE_MASK)

#define FMC_PFB0CR_B0DCE_MASK                    (0x10U)
#define FMC_PFB0CR_B0DCE_SHIFT                   (4U)
/*! B0DCE - Bank 0 Data Cache Enable
 *  0b0..Do not cache data references.
 *  0b1..Cache data references.
 */
#define FMC_PFB0CR_B0DCE(x)                      (((uint32_t)(((uint32_t)(x)) << FMC_PFB0CR_B0DCE_SHIFT)) & FMC_PFB0CR_B0DCE_MASK)

#define FMC_PFB0CR_CRC_MASK                      (0xE0U)
#define FMC_PFB0CR_CRC_SHIFT                     (5U)
/*! CRC - Cache Replacement Control
 *  0b000..LRU replacement algorithm per set across all four ways
 *  0b001..Reserved
 *  0b010..Independent LRU with ways [0-1] for ifetches, [2-3] for data
 *  0b011..Independent LRU with ways [0-2] for ifetches, [3] for data
 *  0b1xx..Reserved
 */
#define FMC_PFB0CR_CRC(x)                        (((uint32_t)(((uint32_t)(x)) << FMC_PFB0CR_CRC_SHIFT)) & FMC_PFB0CR_CRC_MASK)

#define FMC_PFB0CR_B0MW_MASK                     (0x60000U)
#define FMC_PFB0CR_B0MW_SHIFT                    (17U)
/*! B0MW - Bank 0 Memory Width
 *  0b00..32 bits
 *  0b01..64 bits
 *  0b10..Reserved
 *  0b11..Reserved
 */
#define FMC_PFB0CR_B0MW(x)                       (((uint32_t)(((uint32_t)(x)) << FMC_PFB0CR_B0MW_SHIFT)) & FMC_PFB0CR_B0MW_MASK)

#define FMC_PFB0CR_S_B_INV_MASK                  (0x80000U)
#define FMC_PFB0CR_S_B_INV_SHIFT                 (19U)
/*! S_B_INV - Invalidate Prefetch Speculation Buffer
 *  0b0..Speculation buffer and single entry buffer are not affected.
 *  0b1..Invalidate (clear) speculation buffer and single entry buffer.
 */
#define FMC_PFB0CR_S_B_INV(x)                    (((uint32_t)(((uint32_t)(x)) << FMC_PFB0CR_S_B_INV_SHIFT)) & FMC_PFB0CR_S_B_INV_MASK)

#define FMC_PFB0CR_CINV_WAY_MASK                 (0xF00000U)
#define FMC_PFB0CR_CINV_WAY_SHIFT                (20U)
/*! CINV_WAY - Cache Invalidate Way x
 *  0b0000..No cache way invalidation for the corresponding cache
 *  0b0001..Invalidate cache way for the corresponding cache: clear the tag, data, and vld bits of ways selected
 */
#define FMC_PFB0CR_CINV_WAY(x)                   (((uint32_t)(((uint32_t)(x)) << FMC_PFB0CR_CINV_WAY_SHIFT)) & FMC_PFB0CR_CINV_WAY_MASK)

#define FMC_PFB0CR_CLCK_WAY_MASK                 (0xF000000U)
#define FMC_PFB0CR_CLCK_WAY_SHIFT                (24U)
/*! CLCK_WAY - Cache Lock Way x
 *  0b0000..Cache way is unlocked and may be displaced
 *  0b0001..Cache way is locked and its contents are not displaced
 */
#define FMC_PFB0CR_CLCK_WAY(x)                   (((uint32_t)(((uint32_t)(x)) << FMC_PFB0CR_CLCK_WAY_SHIFT)) & FMC_PFB0CR_CLCK_WAY_MASK)

#define FMC_PFB0CR_B0RWSC_MASK                   (0xF0000000U)
#define FMC_PFB0CR_B0RWSC_SHIFT                  (28U)
/*! B0RWSC - Bank 0 Read Wait State Control */
#define FMC_PFB0CR_B0RWSC(x)                     (((uint32_t)(((uint32_t)(x)) << FMC_PFB0CR_B0RWSC_SHIFT)) & FMC_PFB0CR_B0RWSC_MASK)
/*! @} */

/*! @name PFB1CR - Flash Bank 1 Control Register */
/*! @{ */

#define FMC_PFB1CR_B1SEBE_MASK                   (0x1U)
#define FMC_PFB1CR_B1SEBE_SHIFT                  (0U)
/*! B1SEBE - Bank 1 Single Entry Buffer Enable
 *  0b0..Single entry buffer is disabled.
 *  0b1..Single entry buffer is enabled.
 */
#define FMC_PFB1CR_B1SEBE(x)                     (((uint32_t)(((uint32_t)(x)) << FMC_PFB1CR_B1SEBE_SHIFT)) & FMC_PFB1CR_B1SEBE_MASK)

#define FMC_PFB1CR_B1IPE_MASK                    (0x2U)
#define FMC_PFB1CR_B1IPE_SHIFT                   (1U)
/*! B1IPE - Bank 1 Instruction Prefetch Enable
 *  0b0..Do not prefetch in response to instruction fetches.
 *  0b1..Enable prefetches in response to instruction fetches.
 */
#define FMC_PFB1CR_B1IPE(x)                      (((uint32_t)(((uint32_t)(x)) << FMC_PFB1CR_B1IPE_SHIFT)) & FMC_PFB1CR_B1IPE_MASK)

#define FMC_PFB1CR_B1DPE_MASK                    (0x4U)
#define FMC_PFB1CR_B1DPE_SHIFT                   (2U)
/*! B1DPE - Bank 1 Data Prefetch Enable
 *  0b0..Do not prefetch in response to data references.
 *  0b1..Enable prefetches in response to data references.
 */
#define FMC_PFB1CR_B1DPE(x)                      (((uint32_t)(((uint32_t)(x)) << FMC_PFB1CR_B1DPE_SHIFT)) & FMC_PFB1CR_B1DPE_MASK)

#define FMC_PFB1CR_B1ICE_MASK                    (0x8U)
#define FMC_PFB1CR_B1ICE_SHIFT                   (3U)
/*! B1ICE - Bank 1 Instruction Cache Enable
 *  0b0..Do not cache instruction fetches.
 *  0b1..Cache instruction fetches.
 */
#define FMC_PFB1CR_B1ICE(x)                      (((uint32_t)(((uint32_t)(x)) << FMC_PFB1CR_B1ICE_SHIFT)) & FMC_PFB1CR_B1ICE_MASK)

#define FMC_PFB1CR_B1DCE_MASK                    (0x10U)
#define FMC_PFB1CR_B1DCE_SHIFT                   (4U)
/*! B1DCE - Bank 1 Data Cache Enable
 *  0b0..Do not cache data references.
 *  0b1..Cache data references.
 */
#define FMC_PFB1CR_B1DCE(x)                      (((uint32_t)(((uint32_t)(x)) << FMC_PFB1CR_B1DCE_SHIFT)) & FMC_PFB1CR_B1DCE_MASK)

#define FMC_PFB1CR_B1MW_MASK                     (0x60000U)
#define FMC_PFB1CR_B1MW_SHIFT                    (17U)
/*! B1MW - Bank 1 Memory Width
 *  0b00..32 bits
 *  0b01..64 bits
 *  0b10..Reserved
 *  0b11..Reserved
 */
#define FMC_PFB1CR_B1MW(x)                       (((uint32_t)(((uint32_t)(x)) << FMC_PFB1CR_B1MW_SHIFT)) & FMC_PFB1CR_B1MW_MASK)

#define FMC_PFB1CR_B1RWSC_MASK                   (0xF0000000U)
#define FMC_PFB1CR_B1RWSC_SHIFT                  (28U)
/*! B1RWSC - Bank 1 Read Wait State Control */
#define FMC_PFB1CR_B1RWSC(x)                     (((uint32_t)(((uint32_t)(x)) << FMC_PFB1CR_B1RWSC_SHIFT)) & FMC_PFB1CR_B1RWSC_MASK)
/*! @} */

/*! @name TAGVDW0S - Cache Tag Storage */
/*! @{ */

#define FMC_TAGVDW0S_valid_MASK                  (0x1U)
#define FMC_TAGVDW0S_valid_SHIFT                 (0U)
/*! valid - 1-bit valid for cache entry */
#define FMC_TAGVDW0S_valid(x)                    (((uint32_t)(((uint32_t)(x)) << FMC_TAGVDW0S_valid_SHIFT)) & FMC_TAGVDW0S_valid_MASK)

#define FMC_TAGVDW0S_tag_MASK                    (0x7FFE0U)
#define FMC_TAGVDW0S_tag_SHIFT                   (5U)
/*! tag - 14-bit tag for cache entry */
#define FMC_TAGVDW0S_tag(x)                      (((uint32_t)(((uint32_t)(x)) << FMC_TAGVDW0S_tag_SHIFT)) & FMC_TAGVDW0S_tag_MASK)
/*! @} */

/*! @name TAGVDW1S - Cache Tag Storage */
/*! @{ */

#define FMC_TAGVDW1S_valid_MASK                  (0x1U)
#define FMC_TAGVDW1S_valid_SHIFT                 (0U)
/*! valid - 1-bit valid for cache entry */
#define FMC_TAGVDW1S_valid(x)                    (((uint32_t)(((uint32_t)(x)) << FMC_TAGVDW1S_valid_SHIFT)) & FMC_TAGVDW1S_valid_MASK)

#define FMC_TAGVDW1S_tag_MASK                    (0x7FFE0U)
#define FMC_TAGVDW1S_tag_SHIFT                   (5U)
/*! tag - 14-bit tag for cache entry */
#define FMC_TAGVDW1S_tag(x)                      (((uint32_t)(((uint32_t)(x)) << FMC_TAGVDW1S_tag_SHIFT)) & FMC_TAGVDW1S_tag_MASK)
/*! @} */

/*! @name TAGVDW2S - Cache Tag Storage */
/*! @{ */

#define FMC_TAGVDW2S_valid_MASK                  (0x1U)
#define FMC_TAGVDW2S_valid_SHIFT                 (0U)
/*! valid - 1-bit valid for cache entry */
#define FMC_TAGVDW2S_valid(x)                    (((uint32_t)(((uint32_t)(x)) << FMC_TAGVDW2S_valid_SHIFT)) & FMC_TAGVDW2S_valid_MASK)

#define FMC_TAGVDW2S_tag_MASK                    (0x7FFE0U)
#define FMC_TAGVDW2S_tag_SHIFT                   (5U)
/*! tag - 14-bit tag for cache entry */
#define FMC_TAGVDW2S_tag(x)                      (((uint32_t)(((uint32_t)(x)) << FMC_TAGVDW2S_tag_SHIFT)) & FMC_TAGVDW2S_tag_MASK)
/*! @} */

/*! @name TAGVDW3S - Cache Tag Storage */
/*! @{ */

#define FMC_TAGVDW3S_valid_MASK                  (0x1U)
#define FMC_TAGVDW3S_valid_SHIFT                 (0U)
/*! valid - 1-bit valid for cache entry */
#define FMC_TAGVDW3S_valid(x)                    (((uint32_t)(((uint32_t)(x)) << FMC_TAGVDW3S_valid_SHIFT)) & FMC_TAGVDW3S_valid_MASK)

#define FMC_TAGVDW3S_tag_MASK                    (0x7FFE0U)
#define FMC_TAGVDW3S_tag_SHIFT                   (5U)
/*! tag - 14-bit tag for cache entry */
#define FMC_TAGVDW3S_tag(x)                      (((uint32_t)(((uint32_t)(x)) << FMC_TAGVDW3S_tag_SHIFT)) & FMC_TAGVDW3S_tag_MASK)
/*! @} */

/*! @name DATA_U - Cache Data Storage (upper word) */
/*! @{ */

#define FMC_DATA_U_data_MASK                     (0xFFFFFFFFU)
#define FMC_DATA_U_data_SHIFT                    (0U)
/*! data - Bits [63:32] of data entry */
#define FMC_DATA_U_data(x)                       (((uint32_t)(((uint32_t)(x)) << FMC_DATA_U_data_SHIFT)) & FMC_DATA_U_data_MASK)
/*! @} */

/* The count of FMC_DATA_U */
#define FMC_DATA_U_COUNT                         (4U)

/* The count of FMC_DATA_U */
#define FMC_DATA_U_COUNT2                        (8U)

/*! @name DATA_L - Cache Data Storage (lower word) */
/*! @{ */

#define FMC_DATA_L_data_MASK                     (0xFFFFFFFFU)
#define FMC_DATA_L_data_SHIFT                    (0U)
/*! data - Bits [31:0] of data entry */
#define FMC_DATA_L_data(x)                       (((uint32_t)(((uint32_t)(x)) << FMC_DATA_L_data_SHIFT)) & FMC_DATA_L_data_MASK)
/*! @} */

/* The count of FMC_DATA_L */
#define FMC_DATA_L_COUNT                         (4U)

/* The count of FMC_DATA_L */
#define FMC_DATA_L_COUNT2                        (8U)


/*!
 * @}
 */ /* end of group FMC_Register_Masks */


/*!
 * @}
 */ /* end of group FMC_Peripheral_Access_Layer */


/*
** End of section using anonymous unions
*/

#if defined(__ARMCC_VERSION)
  #if (__ARMCC_VERSION >= 6010050)
    #pragma clang diagnostic pop
  #else
    #pragma pop
  #endif
#elif defined(__CWCC__)
  #pragma pop
#elif defined(__GNUC__)
  /* leave anonymous unions enabled */
#elif defined(__IAR_SYSTEMS_ICC__)
  #pragma language=default
#else
  #error Not supported compiler type
#endif

/*!
 * @}
 */ /* end of group Peripheral_access_layer */


#endif  /* PERI_FMC_H_ */

