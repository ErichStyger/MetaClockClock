/*
 * Copyright 2017-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __FSL_ACOMP_H__
#define __FSL_ACOMP_H__

#include "fsl_common.h"
/*! @addtogroup lpc_acomp */
/*! @{*/

/*! @file */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @name Driver version */
/*@{*/
/*! @brief ACOMP driver version 2.1.0. */
#define FSL_ACOMP_DRIVER_VERSION (MAKE_VERSION(2, 1, 0))
/*@}*/

/*!
 * @brief The ACOMP ladder reference voltage.
 */
typedef enum _acomp_ladder_reference_voltage
{
    kACOMP_LadderRefVoltagePinVDD    = 0U, /*!< Supply from pin VDD. */
    kACOMP_LadderRefVoltagePinVDDCMP = 1U, /*!< Supply from pin VDDCMP. */
} acomp_ladder_reference_voltage_t;

/*!
 * @brief The ACOMP interrupts enable.
 */
typedef enum _acomp_interrupt_enable
{
    kACOMP_InterruptsFallingEdgeEnable = 0U, /*!< Enable the falling edge interrupts. */
    kACOMP_InterruptsRisingEdgeEnable  = 1U, /*!< Enable the rising edge interrupts. */
    kACOMP_InterruptsBothEdgesEnable   = 2U, /*!< Enable the both edges interrupts. */
#if defined(FSL_FEATURE_ACOMP_HAS_CTRL_INTENA) && FSL_FEATURE_ACOMP_HAS_CTRL_INTENA
    kACOMP_InterruptsDisable = 3U, /*!< Disable the interrupts. */
#endif                             /*FSL_FEATURE_ACOMP_HAS_CTRL_INTENA*/
} acomp_interrupt_enable_t;

/*!
 * @brief The ACOMP hysteresis selection.
 */
typedef enum _acomp_hysteresis_selection
{
    kACOMP_HysteresisNoneSelection = 0U, /*!< None (the output will switch as the voltages cross). */
    kACOMP_Hysteresis5MVSelection  = 1U, /*!< 5mV. */
    kACOMP_Hysteresis10MVSelection = 2U, /*!< 10mV. */
    kACOMP_Hysteresis20MVSelection = 3U, /*!< 20mV. */
} acomp_hysteresis_selection_t;

/*!
 * @brief The structure for ACOMP basic configuration.
 */
typedef struct _acomp_config
{
    bool enableSyncToBusClk; /*!< If true, Comparator output is synchronized to the bus clock for
                                  output to other modules. If false, Comparator output is used directly. */
    acomp_hysteresis_selection_t hysteresisSelection; /*!< Controls the hysteresis of the comparator. */
} acomp_config_t;

/*!
 * @brief The structure for ACOMP voltage ladder.
 */
typedef struct _acomp_ladder_config
{
    uint8_t ladderValue; /*!< Voltage ladder value. 00000 = Vss, 00001 = 1*Vref/31, ..., 11111 = Vref. */
    acomp_ladder_reference_voltage_t
        referenceVoltage; /*!< Selects the reference voltage(Vref) for the voltage ladder. */
} acomp_ladder_config_t;
/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/*!
 * @name Initialization
 * @{
 */

/*!
 * @brief Initialize the ACOMP module.
 *
 * @param base ACOMP peripheral base address.
 * @param config Pointer to "acomp_config_t" structure.
 */
void ACOMP_Init(ACOMP_Type *base, const acomp_config_t *config);

/*!
 * @brief De-initialize the ACOMP module.
 *
 * @param base ACOMP peripheral base address.
 */
void ACOMP_Deinit(ACOMP_Type *base);

/*!
 * @brief Gets an available pre-defined settings for the ACOMP's configuration.
 *
 * This function initializes the converter configuration structure with available settings. The default values are:
 * @code
 *   config->enableSyncToBusClk = false;
 *   config->hysteresisSelection = kACOMP_hysteresisNoneSelection;
 * @endcode
 * In default configuration, the ACOMP's output would be used directly and switch as the voltages cross.
 *
 * @param config Pointer to the configuration structure.
 */
void ACOMP_GetDefaultConfig(acomp_config_t *config);

/*!
 * @brief Enable ACOMP interrupts.
 *
 * @param base ACOMP peripheral base address.
 * @param enable Enable/Disable interrupt feature.
 */
void ACOMP_EnableInterrupts(ACOMP_Type *base, acomp_interrupt_enable_t enable);

/*!
 * @brief Get interrupts status flags.
 *
 * @param base ACOMP peripheral base address.
 * @return Reflect the state ACOMP edge-detect status, true or false.
 */
static inline bool ACOMP_GetInterruptsStatusFlags(ACOMP_Type *base)
{
    return ((ACOMP_CTRL_COMPEDGE_MASK & base->CTRL) != 0UL) ? true : false;
}

/*!
 * @brief Clear the ACOMP interrupts status flags.
 *
 * @param base ACOMP peripheral base address.
 */
static inline void ACOMP_ClearInterruptsStatusFlags(ACOMP_Type *base)
{
    base->CTRL |= ACOMP_CTRL_EDGECLR_MASK;
    base->CTRL &= ~ACOMP_CTRL_EDGECLR_MASK;
}

/*!
 * @brief Get ACOMP output status flags.
 *
 * @param base ACOMP peripheral base address.
 * @return Reflect the state of the comparator output, true or false.
 */
static inline bool ACOMP_GetOutputStatusFlags(ACOMP_Type *base)
{
    return ((ACOMP_CTRL_COMPSTAT_MASK & base->CTRL) != 0UL) ? true : false;
}

/*!
 * @brief Set the ACOMP postive and negative input channel.
 *
 * @param base ACOMP peripheral base address.
 * @param postiveInputChannel The index of postive input channel.
 * @param negativeInputChannel The index of negative input channel.
 */
static inline void ACOMP_SetInputChannel(ACOMP_Type *base, uint32_t postiveInputChannel, uint32_t negativeInputChannel)
{
    base->CTRL = (base->CTRL & ~(ACOMP_CTRL_COMP_VP_SEL_MASK | ACOMP_CTRL_COMP_VM_SEL_MASK)) |
                 (ACOMP_CTRL_COMP_VP_SEL(postiveInputChannel) | ACOMP_CTRL_COMP_VM_SEL(negativeInputChannel));
}

/*!
 * @brief Set the voltage ladder configuration.
 *
 * @param base ACOMP peripheral base address.
 * @param config The structure for voltage ladder. If the config is NULL, voltage ladder would be diasbled,
 *               otherwise the voltage ladder would be configured and enabled.
 */
void ACOMP_SetLadderConfig(ACOMP_Type *base, const acomp_ladder_config_t *config);

/*@}*/

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

/*@}*/

#endif /* _FSL_ACOMP_H_ */
