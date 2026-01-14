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
**         CMSIS Peripheral Access Layer for MCG
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
 * @file PERI_MCG.h
 * @version 3.0
 * @date 2024-10-29
 * @brief CMSIS Peripheral Access Layer for MCG
 *
 * CMSIS Peripheral Access Layer for MCG
 */

#if !defined(PERI_MCG_H_)
#define PERI_MCG_H_                              /**< Symbol preventing repeated inclusion */

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
   -- MCG Peripheral Access Layer
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup MCG_Peripheral_Access_Layer MCG Peripheral Access Layer
 * @{
 */

/** MCG - Register Layout Typedef */
typedef struct {
  __IO uint8_t C1;                                 /**< MCG Control 1 Register, offset: 0x0 */
  __IO uint8_t C2;                                 /**< MCG Control 2 Register, offset: 0x1 */
  __IO uint8_t C3;                                 /**< MCG Control 3 Register, offset: 0x2 */
  __IO uint8_t C4;                                 /**< MCG Control 4 Register, offset: 0x3 */
  __IO uint8_t C5;                                 /**< MCG Control 5 Register, offset: 0x4 */
  __IO uint8_t C6;                                 /**< MCG Control 6 Register, offset: 0x5 */
  __IO uint8_t S;                                  /**< MCG Status Register, offset: 0x6 */
       uint8_t RESERVED_0[1];
  __IO uint8_t SC;                                 /**< MCG Status and Control Register, offset: 0x8 */
       uint8_t RESERVED_1[1];
  __IO uint8_t ATCVH;                              /**< MCG Auto Trim Compare Value High Register, offset: 0xA */
  __IO uint8_t ATCVL;                              /**< MCG Auto Trim Compare Value Low Register, offset: 0xB */
  __IO uint8_t C7;                                 /**< MCG Control 7 Register, offset: 0xC */
  __IO uint8_t C8;                                 /**< MCG Control 8 Register, offset: 0xD */
} MCG_Type;

/* ----------------------------------------------------------------------------
   -- MCG Register Masks
   ---------------------------------------------------------------------------- */

/*!
 * @addtogroup MCG_Register_Masks MCG Register Masks
 * @{
 */

/*! @name C1 - MCG Control 1 Register */
/*! @{ */

#define MCG_C1_IREFSTEN_MASK                     (0x1U)
#define MCG_C1_IREFSTEN_SHIFT                    (0U)
/*! IREFSTEN - Internal Reference Stop Enable
 *  0b0..Internal reference clock is disabled in Stop mode.
 *  0b1..Internal reference clock is enabled in Stop mode if IRCLKEN is set or if MCG is in FEI, FBI, or BLPI modes before entering Stop mode.
 */
#define MCG_C1_IREFSTEN(x)                       (((uint8_t)(((uint8_t)(x)) << MCG_C1_IREFSTEN_SHIFT)) & MCG_C1_IREFSTEN_MASK)

#define MCG_C1_IRCLKEN_MASK                      (0x2U)
#define MCG_C1_IRCLKEN_SHIFT                     (1U)
/*! IRCLKEN - Internal Reference Clock Enable
 *  0b0..MCGIRCLK inactive.
 *  0b1..MCGIRCLK active.
 */
#define MCG_C1_IRCLKEN(x)                        (((uint8_t)(((uint8_t)(x)) << MCG_C1_IRCLKEN_SHIFT)) & MCG_C1_IRCLKEN_MASK)

#define MCG_C1_IREFS_MASK                        (0x4U)
#define MCG_C1_IREFS_SHIFT                       (2U)
/*! IREFS - Internal Reference Select
 *  0b0..External reference clock is selected.
 *  0b1..The slow internal reference clock is selected.
 */
#define MCG_C1_IREFS(x)                          (((uint8_t)(((uint8_t)(x)) << MCG_C1_IREFS_SHIFT)) & MCG_C1_IREFS_MASK)

#define MCG_C1_FRDIV_MASK                        (0x38U)
#define MCG_C1_FRDIV_SHIFT                       (3U)
/*! FRDIV - FLL External Reference Divider
 *  0b000..If RANGE = 0 or OSCSEL=1 , Divide Factor is 1; for all other RANGE values, Divide Factor is 32.
 *  0b001..If RANGE = 0 or OSCSEL=1 , Divide Factor is 2; for all other RANGE values, Divide Factor is 64.
 *  0b010..If RANGE = 0 or OSCSEL=1 , Divide Factor is 4; for all other RANGE values, Divide Factor is 128.
 *  0b011..If RANGE = 0 or OSCSEL=1 , Divide Factor is 8; for all other RANGE values, Divide Factor is 256.
 *  0b100..If RANGE = 0 or OSCSEL=1 , Divide Factor is 16; for all other RANGE values, Divide Factor is 512.
 *  0b101..If RANGE = 0 or OSCSEL=1 , Divide Factor is 32; for all other RANGE values, Divide Factor is 1024.
 *  0b110..If RANGE = 0 or OSCSEL=1 , Divide Factor is 64; for all other RANGE values, Divide Factor is 1280 .
 *  0b111..If RANGE = 0 or OSCSEL=1 , Divide Factor is 128; for all other RANGE values, Divide Factor is 1536 .
 */
#define MCG_C1_FRDIV(x)                          (((uint8_t)(((uint8_t)(x)) << MCG_C1_FRDIV_SHIFT)) & MCG_C1_FRDIV_MASK)

#define MCG_C1_CLKS_MASK                         (0xC0U)
#define MCG_C1_CLKS_SHIFT                        (6U)
/*! CLKS - Clock Source Select
 *  0b00..Encoding 0 - Output of FLL or PLL is selected (depends on PLLS control bit).
 *  0b01..Encoding 1 - Internal reference clock is selected.
 *  0b10..Encoding 2 - External reference clock is selected.
 *  0b11..Encoding 3 - Reserved.
 */
#define MCG_C1_CLKS(x)                           (((uint8_t)(((uint8_t)(x)) << MCG_C1_CLKS_SHIFT)) & MCG_C1_CLKS_MASK)
/*! @} */

/*! @name C2 - MCG Control 2 Register */
/*! @{ */

#define MCG_C2_IRCS_MASK                         (0x1U)
#define MCG_C2_IRCS_SHIFT                        (0U)
/*! IRCS - Internal Reference Clock Select
 *  0b0..Slow internal reference clock selected.
 *  0b1..Fast internal reference clock selected.
 */
#define MCG_C2_IRCS(x)                           (((uint8_t)(((uint8_t)(x)) << MCG_C2_IRCS_SHIFT)) & MCG_C2_IRCS_MASK)

#define MCG_C2_LP_MASK                           (0x2U)
#define MCG_C2_LP_SHIFT                          (1U)
/*! LP - Low Power Select
 *  0b0..FLL or PLL is not disabled in bypass modes.
 *  0b1..FLL or PLL is disabled in bypass modes (lower power)
 */
#define MCG_C2_LP(x)                             (((uint8_t)(((uint8_t)(x)) << MCG_C2_LP_SHIFT)) & MCG_C2_LP_MASK)

#define MCG_C2_EREFS_MASK                        (0x4U)
#define MCG_C2_EREFS_SHIFT                       (2U)
/*! EREFS - External Reference Select
 *  0b0..External reference clock requested.
 *  0b1..Oscillator requested.
 */
#define MCG_C2_EREFS(x)                          (((uint8_t)(((uint8_t)(x)) << MCG_C2_EREFS_SHIFT)) & MCG_C2_EREFS_MASK)

#define MCG_C2_HGO_MASK                          (0x8U)
#define MCG_C2_HGO_SHIFT                         (3U)
/*! HGO - High Gain Oscillator Select
 *  0b0..Configure crystal oscillator for low-power operation.
 *  0b1..Configure crystal oscillator for high-gain operation.
 */
#define MCG_C2_HGO(x)                            (((uint8_t)(((uint8_t)(x)) << MCG_C2_HGO_SHIFT)) & MCG_C2_HGO_MASK)

#define MCG_C2_RANGE_MASK                        (0x30U)
#define MCG_C2_RANGE_SHIFT                       (4U)
/*! RANGE - Frequency Range Select
 *  0b00..Encoding 0 - Low frequency range selected for the crystal oscillator .
 *  0b01..Encoding 1 - High frequency range selected for the crystal oscillator .
 *  0b1x..Encoding 2 - Very high frequency range selected for the crystal oscillator .
 */
#define MCG_C2_RANGE(x)                          (((uint8_t)(((uint8_t)(x)) << MCG_C2_RANGE_SHIFT)) & MCG_C2_RANGE_MASK)

#define MCG_C2_FCFTRIM_MASK                      (0x40U)
#define MCG_C2_FCFTRIM_SHIFT                     (6U)
/*! FCFTRIM - Fast Internal Reference Clock Fine Trim */
#define MCG_C2_FCFTRIM(x)                        (((uint8_t)(((uint8_t)(x)) << MCG_C2_FCFTRIM_SHIFT)) & MCG_C2_FCFTRIM_MASK)

#define MCG_C2_LOCRE0_MASK                       (0x80U)
#define MCG_C2_LOCRE0_SHIFT                      (7U)
/*! LOCRE0 - Loss of Clock Reset Enable
 *  0b0..Interrupt request is generated on a loss of OSC0 external reference clock.
 *  0b1..Generate a reset request on a loss of OSC0 external reference clock.
 */
#define MCG_C2_LOCRE0(x)                         (((uint8_t)(((uint8_t)(x)) << MCG_C2_LOCRE0_SHIFT)) & MCG_C2_LOCRE0_MASK)
/*! @} */

/*! @name C3 - MCG Control 3 Register */
/*! @{ */

#define MCG_C3_SCTRIM_MASK                       (0xFFU)
#define MCG_C3_SCTRIM_SHIFT                      (0U)
/*! SCTRIM - Slow Internal Reference Clock Trim Setting */
#define MCG_C3_SCTRIM(x)                         (((uint8_t)(((uint8_t)(x)) << MCG_C3_SCTRIM_SHIFT)) & MCG_C3_SCTRIM_MASK)
/*! @} */

/*! @name C4 - MCG Control 4 Register */
/*! @{ */

#define MCG_C4_SCFTRIM_MASK                      (0x1U)
#define MCG_C4_SCFTRIM_SHIFT                     (0U)
/*! SCFTRIM - Slow Internal Reference Clock Fine Trim */
#define MCG_C4_SCFTRIM(x)                        (((uint8_t)(((uint8_t)(x)) << MCG_C4_SCFTRIM_SHIFT)) & MCG_C4_SCFTRIM_MASK)

#define MCG_C4_FCTRIM_MASK                       (0x1EU)
#define MCG_C4_FCTRIM_SHIFT                      (1U)
/*! FCTRIM - Fast Internal Reference Clock Trim Setting */
#define MCG_C4_FCTRIM(x)                         (((uint8_t)(((uint8_t)(x)) << MCG_C4_FCTRIM_SHIFT)) & MCG_C4_FCTRIM_MASK)

#define MCG_C4_DRST_DRS_MASK                     (0x60U)
#define MCG_C4_DRST_DRS_SHIFT                    (5U)
/*! DRST_DRS - DCO Range Select
 *  0b00..Encoding 0 - Low range (reset default).
 *  0b01..Encoding 1 - Mid range.
 *  0b10..Encoding 2 - Mid-high range.
 *  0b11..Encoding 3 - High range.
 */
#define MCG_C4_DRST_DRS(x)                       (((uint8_t)(((uint8_t)(x)) << MCG_C4_DRST_DRS_SHIFT)) & MCG_C4_DRST_DRS_MASK)

#define MCG_C4_DMX32_MASK                        (0x80U)
#define MCG_C4_DMX32_SHIFT                       (7U)
/*! DMX32 - DCO Maximum Frequency with 32.768 kHz Reference
 *  0b0..DCO has a default range of 25%.
 *  0b1..DCO is fine-tuned for maximum frequency with 32.768 kHz reference.
 */
#define MCG_C4_DMX32(x)                          (((uint8_t)(((uint8_t)(x)) << MCG_C4_DMX32_SHIFT)) & MCG_C4_DMX32_MASK)
/*! @} */

/*! @name C5 - MCG Control 5 Register */
/*! @{ */

#define MCG_C5_PRDIV0_MASK                       (0x1FU)
#define MCG_C5_PRDIV0_SHIFT                      (0U)
/*! PRDIV0 - PLL External Reference Divider
 *  0b00000..Divide Factor is 1
 *  0b00001..Divide Factor is 2
 *  0b00010..Divide Factor is 3
 *  0b00011..Divide Factor is 4
 *  0b00100..Divide Factor is 5
 *  0b00101..Divide Factor is 6
 *  0b00110..Divide Factor is 7
 *  0b00111..Divide Factor is 8
 *  0b01000..Divide Factor is 9
 *  0b01001..Divide Factor is 10
 *  0b01010..Divide Factor is 11
 *  0b01011..Divide Factor is 12
 *  0b01100..Divide Factor is 13
 *  0b01101..Divide Factor is 14
 *  0b01110..Divide Factor is 15
 *  0b01111..Divide Factor is 16
 *  0b10000..Divide Factor is 17
 *  0b10001..Divide Factor is 18
 *  0b10010..Divide Factor is 19
 *  0b10011..Divide Factor is 20
 *  0b10100..Divide Factor is 21
 *  0b10101..Divide Factor is 22
 *  0b10110..Divide Factor is 23
 *  0b10111..Divide Factor is 24
 *  0b11000..Divide Factor is 25
 *  0b11001..Divide Factor is 26
 *  0b11010..Divide Factor is 27
 *  0b11011..Divide Factor is 28
 *  0b11100..Divide Factor is 29
 *  0b11101..Divide Factor is 30
 *  0b11110..Divide Factor is 31
 *  0b11111..Divide Factor is 32
 */
#define MCG_C5_PRDIV0(x)                         (((uint8_t)(((uint8_t)(x)) << MCG_C5_PRDIV0_SHIFT)) & MCG_C5_PRDIV0_MASK)

#define MCG_C5_PLLSTEN0_MASK                     (0x20U)
#define MCG_C5_PLLSTEN0_SHIFT                    (5U)
/*! PLLSTEN0 - PLL Stop Enable
 *  0b0..MCGPLLCLK is disabled in any of the Stop modes.
 *  0b1..MCGPLLCLK is enabled if system is in Normal Stop mode.
 */
#define MCG_C5_PLLSTEN0(x)                       (((uint8_t)(((uint8_t)(x)) << MCG_C5_PLLSTEN0_SHIFT)) & MCG_C5_PLLSTEN0_MASK)

#define MCG_C5_PLLCLKEN0_MASK                    (0x40U)
#define MCG_C5_PLLCLKEN0_SHIFT                   (6U)
/*! PLLCLKEN0 - PLL Clock Enable
 *  0b0..MCGPLLCLK is inactive.
 *  0b1..MCGPLLCLK is active.
 */
#define MCG_C5_PLLCLKEN0(x)                      (((uint8_t)(((uint8_t)(x)) << MCG_C5_PLLCLKEN0_SHIFT)) & MCG_C5_PLLCLKEN0_MASK)
/*! @} */

/*! @name C6 - MCG Control 6 Register */
/*! @{ */

#define MCG_C6_VDIV0_MASK                        (0x1FU)
#define MCG_C6_VDIV0_SHIFT                       (0U)
/*! VDIV0 - VCO 0 Divider
 *  0b00000..Multiply Factor is 24
 *  0b00001..Multiply Factor is 25
 *  0b00010..Multiply Factor is 26
 *  0b00011..Multiply Factor is 27
 *  0b00100..Multiply Factor is 28
 *  0b00101..Multiply Factor is 29
 *  0b00110..Multiply Factor is 30
 *  0b00111..Multiply Factor is 31
 *  0b01000..Multiply Factor is 32
 *  0b01001..Multiply Factor is 33
 *  0b01010..Multiply Factor is 34
 *  0b01011..Multiply Factor is 35
 *  0b01100..Multiply Factor is 36
 *  0b01101..Multiply Factor is 37
 *  0b01110..Multiply Factor is 38
 *  0b01111..Multiply Factor is 39
 *  0b10000..Multiply Factor is 40
 *  0b10001..Multiply Factor is 41
 *  0b10010..Multiply Factor is 42
 *  0b10011..Multiply Factor is 43
 *  0b10100..Multiply Factor is 44
 *  0b10101..Multiply Factor is 45
 *  0b10110..Multiply Factor is 46
 *  0b10111..Multiply Factor is 47
 *  0b11000..Multiply Factor is 48
 *  0b11001..Multiply Factor is 49
 *  0b11010..Multiply Factor is 50
 *  0b11011..Multiply Factor is 51
 *  0b11100..Multiply Factor is 52
 *  0b11101..Multiply Factor is 53
 *  0b11110..Multiply Factor is 54
 *  0b11111..Multiply Factor is 55
 */
#define MCG_C6_VDIV0(x)                          (((uint8_t)(((uint8_t)(x)) << MCG_C6_VDIV0_SHIFT)) & MCG_C6_VDIV0_MASK)

#define MCG_C6_CME0_MASK                         (0x20U)
#define MCG_C6_CME0_SHIFT                        (5U)
/*! CME0 - Clock Monitor Enable
 *  0b0..External clock monitor is disabled for OSC0.
 *  0b1..External clock monitor is enabled for OSC0.
 */
#define MCG_C6_CME0(x)                           (((uint8_t)(((uint8_t)(x)) << MCG_C6_CME0_SHIFT)) & MCG_C6_CME0_MASK)

#define MCG_C6_PLLS_MASK                         (0x40U)
#define MCG_C6_PLLS_SHIFT                        (6U)
/*! PLLS - PLL Select
 *  0b0..FLL is selected.
 *  0b1..PLL is selected (PRDIV 0 need to be programmed to the correct divider to generate a PLL reference clock
 *       in the range of 2-4 MHz prior to setting the PLLS bit).
 */
#define MCG_C6_PLLS(x)                           (((uint8_t)(((uint8_t)(x)) << MCG_C6_PLLS_SHIFT)) & MCG_C6_PLLS_MASK)

#define MCG_C6_LOLIE0_MASK                       (0x80U)
#define MCG_C6_LOLIE0_SHIFT                      (7U)
/*! LOLIE0 - Loss of Lock Interrrupt Enable
 *  0b0..No interrupt request is generated on loss of lock.
 *  0b1..Generate an interrupt request on loss of lock.
 */
#define MCG_C6_LOLIE0(x)                         (((uint8_t)(((uint8_t)(x)) << MCG_C6_LOLIE0_SHIFT)) & MCG_C6_LOLIE0_MASK)
/*! @} */

/*! @name S - MCG Status Register */
/*! @{ */

#define MCG_S_IRCST_MASK                         (0x1U)
#define MCG_S_IRCST_SHIFT                        (0U)
/*! IRCST - Internal Reference Clock Status
 *  0b0..Source of internal reference clock is the slow clock (32 kHz IRC).
 *  0b1..Source of internal reference clock is the fast clock (4 MHz IRC).
 */
#define MCG_S_IRCST(x)                           (((uint8_t)(((uint8_t)(x)) << MCG_S_IRCST_SHIFT)) & MCG_S_IRCST_MASK)

#define MCG_S_OSCINIT0_MASK                      (0x2U)
#define MCG_S_OSCINIT0_SHIFT                     (1U)
/*! OSCINIT0 - OSC Initialization */
#define MCG_S_OSCINIT0(x)                        (((uint8_t)(((uint8_t)(x)) << MCG_S_OSCINIT0_SHIFT)) & MCG_S_OSCINIT0_MASK)

#define MCG_S_CLKST_MASK                         (0xCU)
#define MCG_S_CLKST_SHIFT                        (2U)
/*! CLKST - Clock Mode Status
 *  0b00..Encoding 0 - Output of the FLL is selected (reset default).
 *  0b01..Encoding 1 - Internal reference clock is selected.
 *  0b10..Encoding 2 - External reference clock is selected.
 *  0b11..Encoding 3 - Output of the PLL is selected.
 */
#define MCG_S_CLKST(x)                           (((uint8_t)(((uint8_t)(x)) << MCG_S_CLKST_SHIFT)) & MCG_S_CLKST_MASK)

#define MCG_S_IREFST_MASK                        (0x10U)
#define MCG_S_IREFST_SHIFT                       (4U)
/*! IREFST - Internal Reference Status
 *  0b0..Source of FLL reference clock is the external reference clock.
 *  0b1..Source of FLL reference clock is the internal reference clock.
 */
#define MCG_S_IREFST(x)                          (((uint8_t)(((uint8_t)(x)) << MCG_S_IREFST_SHIFT)) & MCG_S_IREFST_MASK)

#define MCG_S_PLLST_MASK                         (0x20U)
#define MCG_S_PLLST_SHIFT                        (5U)
/*! PLLST - PLL Select Status
 *  0b0..Source of PLLS clock is FLL clock.
 *  0b1..Source of PLLS clock is PLL output clock.
 */
#define MCG_S_PLLST(x)                           (((uint8_t)(((uint8_t)(x)) << MCG_S_PLLST_SHIFT)) & MCG_S_PLLST_MASK)

#define MCG_S_LOCK0_MASK                         (0x40U)
#define MCG_S_LOCK0_SHIFT                        (6U)
/*! LOCK0 - Lock Status
 *  0b0..PLL is currently unlocked.
 *  0b1..PLL is currently locked.
 */
#define MCG_S_LOCK0(x)                           (((uint8_t)(((uint8_t)(x)) << MCG_S_LOCK0_SHIFT)) & MCG_S_LOCK0_MASK)

#define MCG_S_LOLS0_MASK                         (0x80U)
#define MCG_S_LOLS0_SHIFT                        (7U)
/*! LOLS0 - Loss of Lock Status
 *  0b0..PLL has not lost lock since LOLS 0 was last cleared.
 *  0b1..PLL has lost lock since LOLS 0 was last cleared.
 */
#define MCG_S_LOLS0(x)                           (((uint8_t)(((uint8_t)(x)) << MCG_S_LOLS0_SHIFT)) & MCG_S_LOLS0_MASK)
/*! @} */

/*! @name SC - MCG Status and Control Register */
/*! @{ */

#define MCG_SC_LOCS0_MASK                        (0x1U)
#define MCG_SC_LOCS0_SHIFT                       (0U)
/*! LOCS0 - OSC0 Loss of Clock Status
 *  0b0..Loss of OSC0 has not occurred.
 *  0b1..Loss of OSC0 has occurred.
 */
#define MCG_SC_LOCS0(x)                          (((uint8_t)(((uint8_t)(x)) << MCG_SC_LOCS0_SHIFT)) & MCG_SC_LOCS0_MASK)

#define MCG_SC_FCRDIV_MASK                       (0xEU)
#define MCG_SC_FCRDIV_SHIFT                      (1U)
/*! FCRDIV - Fast Clock Internal Reference Divider
 *  0b000..Divide Factor is 1
 *  0b001..Divide Factor is 2.
 *  0b010..Divide Factor is 4.
 *  0b011..Divide Factor is 8.
 *  0b100..Divide Factor is 16
 *  0b101..Divide Factor is 32
 *  0b110..Divide Factor is 64
 *  0b111..Divide Factor is 128.
 */
#define MCG_SC_FCRDIV(x)                         (((uint8_t)(((uint8_t)(x)) << MCG_SC_FCRDIV_SHIFT)) & MCG_SC_FCRDIV_MASK)

#define MCG_SC_FLTPRSRV_MASK                     (0x10U)
#define MCG_SC_FLTPRSRV_SHIFT                    (4U)
/*! FLTPRSRV - FLL Filter Preserve Enable
 *  0b0..FLL filter and FLL frequency will reset on changes to currect clock mode.
 *  0b1..Fll filter and FLL frequency retain their previous values during new clock mode change.
 */
#define MCG_SC_FLTPRSRV(x)                       (((uint8_t)(((uint8_t)(x)) << MCG_SC_FLTPRSRV_SHIFT)) & MCG_SC_FLTPRSRV_MASK)

#define MCG_SC_ATMF_MASK                         (0x20U)
#define MCG_SC_ATMF_SHIFT                        (5U)
/*! ATMF - Automatic Trim Machine Fail Flag
 *  0b0..Automatic Trim Machine completed normally.
 *  0b1..Automatic Trim Machine failed.
 */
#define MCG_SC_ATMF(x)                           (((uint8_t)(((uint8_t)(x)) << MCG_SC_ATMF_SHIFT)) & MCG_SC_ATMF_MASK)

#define MCG_SC_ATMS_MASK                         (0x40U)
#define MCG_SC_ATMS_SHIFT                        (6U)
/*! ATMS - Automatic Trim Machine Select
 *  0b0..32 kHz Internal Reference Clock selected.
 *  0b1..4 MHz Internal Reference Clock selected.
 */
#define MCG_SC_ATMS(x)                           (((uint8_t)(((uint8_t)(x)) << MCG_SC_ATMS_SHIFT)) & MCG_SC_ATMS_MASK)

#define MCG_SC_ATME_MASK                         (0x80U)
#define MCG_SC_ATME_SHIFT                        (7U)
/*! ATME - Automatic Trim Machine Enable
 *  0b0..Auto Trim Machine disabled.
 *  0b1..Auto Trim Machine enabled.
 */
#define MCG_SC_ATME(x)                           (((uint8_t)(((uint8_t)(x)) << MCG_SC_ATME_SHIFT)) & MCG_SC_ATME_MASK)
/*! @} */

/*! @name ATCVH - MCG Auto Trim Compare Value High Register */
/*! @{ */

#define MCG_ATCVH_ATCVH_MASK                     (0xFFU)
#define MCG_ATCVH_ATCVH_SHIFT                    (0U)
/*! ATCVH - ATM Compare Value High */
#define MCG_ATCVH_ATCVH(x)                       (((uint8_t)(((uint8_t)(x)) << MCG_ATCVH_ATCVH_SHIFT)) & MCG_ATCVH_ATCVH_MASK)
/*! @} */

/*! @name ATCVL - MCG Auto Trim Compare Value Low Register */
/*! @{ */

#define MCG_ATCVL_ATCVL_MASK                     (0xFFU)
#define MCG_ATCVL_ATCVL_SHIFT                    (0U)
/*! ATCVL - ATM Compare Value Low */
#define MCG_ATCVL_ATCVL(x)                       (((uint8_t)(((uint8_t)(x)) << MCG_ATCVL_ATCVL_SHIFT)) & MCG_ATCVL_ATCVL_MASK)
/*! @} */

/*! @name C7 - MCG Control 7 Register */
/*! @{ */

#define MCG_C7_OSCSEL_MASK                       (0x3U)
#define MCG_C7_OSCSEL_SHIFT                      (0U)
/*! OSCSEL - MCG OSC Clock Select
 *  0b00..Selects Oscillator (OSCCLK0).
 *  0b01..Selects 32 kHz RTC Oscillator.
 *  0b10..Selects Oscillator (OSCCLK1).
 *  0b11..RESERVED
 */
#define MCG_C7_OSCSEL(x)                         (((uint8_t)(((uint8_t)(x)) << MCG_C7_OSCSEL_SHIFT)) & MCG_C7_OSCSEL_MASK)
/*! @} */

/*! @name C8 - MCG Control 8 Register */
/*! @{ */

#define MCG_C8_LOCS1_MASK                        (0x1U)
#define MCG_C8_LOCS1_SHIFT                       (0U)
/*! LOCS1 - RTC Loss of Clock Status
 *  0b0..Loss of RTC has not occur.
 *  0b1..Loss of RTC has occur
 */
#define MCG_C8_LOCS1(x)                          (((uint8_t)(((uint8_t)(x)) << MCG_C8_LOCS1_SHIFT)) & MCG_C8_LOCS1_MASK)

#define MCG_C8_CME1_MASK                         (0x20U)
#define MCG_C8_CME1_SHIFT                        (5U)
/*! CME1 - Clock Monitor Enable1
 *  0b0..External clock monitor is disabled for RTC clock.
 *  0b1..External clock monitor is enabled for RTC clock.
 */
#define MCG_C8_CME1(x)                           (((uint8_t)(((uint8_t)(x)) << MCG_C8_CME1_SHIFT)) & MCG_C8_CME1_MASK)

#define MCG_C8_LOLRE_MASK                        (0x40U)
#define MCG_C8_LOLRE_SHIFT                       (6U)
/*! LOLRE - PLL Loss of Lock Reset Enable
 *  0b0..Interrupt request is generated on a PLL loss of lock indication. The PLL loss of lock interrupt enable
 *       bit must also be set to generate the interrupt request.
 *  0b1..Generate a reset request on a PLL loss of lock indication.
 */
#define MCG_C8_LOLRE(x)                          (((uint8_t)(((uint8_t)(x)) << MCG_C8_LOLRE_SHIFT)) & MCG_C8_LOLRE_MASK)

#define MCG_C8_LOCRE1_MASK                       (0x80U)
#define MCG_C8_LOCRE1_SHIFT                      (7U)
/*! LOCRE1 - Loss of Clock Reset Enable
 *  0b0..Interrupt request is generated on a loss of RTC external reference clock.
 *  0b1..Generate a reset request on a loss of RTC external reference clock
 */
#define MCG_C8_LOCRE1(x)                         (((uint8_t)(((uint8_t)(x)) << MCG_C8_LOCRE1_SHIFT)) & MCG_C8_LOCRE1_MASK)
/*! @} */


/*!
 * @}
 */ /* end of group MCG_Register_Masks */


/*!
 * @}
 */ /* end of group MCG_Peripheral_Access_Layer */


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


#endif  /* PERI_MCG_H_ */

