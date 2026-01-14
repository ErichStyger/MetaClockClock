/*
 * Copyright (c) 2013-2015 Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * @file microseconds.c
 * @brief Microseconds timer driver source file
 *
 * Notes: The driver configure PIT as lifetime timer
 */
#include <stdarg.h>

#include "bootloader_common.h"
#include "microseconds.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
enum
{
    kFrequency_1MHz = 1000000UL
};

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
uint32_t s_tickPerMicrosecondMul8; //!< This value equal to 8 times ticks per microseconds
uint64_t s_timeoutTicks;           //!< Tick value of timeout.
////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

//! @brief Initialize timer facilities.
//!
//! It is initialize the timer to lifetime timer by chained channel 0
//! and channel 1 together, and set b0th channels to maximum counting period
void microseconds_init(void)
{
    uint32_t pitClock;

    // PIT clock gate control ON
#if defined(SIM)
//    SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
#elif defined(CCM)
    CLOCK_EnableClock(kCLOCK_Lpit);
#if defined(LPIT_PERIPH_CLOCKS)
    CLOCK_EnableClock(kCLOCK_LpitPeriph);
#endif // #if defined(LPIT_PERIPH_CLOCKS)
#endif
    /* Enable LPIT */
    LPIT->MCR |= LPIT_MCR_M_CEN_MASK;

    // Reset LPIT
    LPIT->MCR |= LPIT_MCR_SW_RST_MASK;
    LPIT->MCR &= ~LPIT_MCR_SW_RST_MASK;

    // Clear all interrupt flags
    LPIT->MSR = (LPIT_MSR_TIF0_MASK | LPIT_MSR_TIF1_MASK | LPIT_MSR_TIF2_MASK | LPIT_MSR_TIF3_MASK);

    // Disable timer interrupts
    LPIT->MIER = 0;

    // Set up timer 1 to max value
    LPIT->CHANNEL[1].TVAL = 0xFFFFFFFF;              // setup timer 1 for maximum counting period
    LPIT->CHANNEL[1].TCTRL = 0;                      // Set 32-bit periodic counter mode
    LPIT->CHANNEL[1].TCTRL |= LPIT_TCTRL_CHAIN_MASK; // chain timer 1 to timer 0
    LPIT->CHANNEL[1].TCTRL |= LPIT_TCTRL_T_EN_MASK;  // start timer 1

    // Set up timer 0 to max value
    LPIT->CHANNEL[0].TVAL = 0xFFFFFFFF;            // setup timer 0 for maximum counting period
    LPIT->CHANNEL[0].TCTRL = LPIT_TCTRL_T_EN_MASK; // start timer 0

    /* Calculate this value early
     * The reason why use this solution is that lowest clock frequency supported by L0PB and L4KS
     * is 0.25MHz, this solution will make sure ticks per microscond is greater than 0.
     */

    pitClock = microseconds_get_clock();
    s_tickPerMicrosecondMul8 = (pitClock * 8) / kFrequency_1MHz;

    // Make sure this value is greater than 0
    if (!s_tickPerMicrosecondMul8)
    {
        s_tickPerMicrosecondMul8 = 1;
    }
}

//! @brief Shutdown the microsecond timer
void microseconds_shutdown(void)
{
    // Disable the clock to LPIT
    LPIT->MCR &= ~LPIT_MCR_M_CEN_MASK;
}

//! @brief Read back running tick count
uint64_t microseconds_get_ticks(void)
{
    uint64_t valueH;
    volatile uint32_t valueL;

    // Make sure that there are no rollover of valueL.
    // Because the valueL always decreases, so, if the formal valueL is greater than
    // current value, that means the valueH is updated during read valueL.
    // In this case, we need to re-update valueH and valueL.
    do
    {
        valueL = LPIT->CHANNEL[0].CVAL;
        valueH = LPIT->CHANNEL[1].CVAL;
    } while (valueL < LPIT->CHANNEL[0].CVAL);

    // Invert to turn into an up counter
    return ~((valueH << 32) | valueL);
}

//! @brief Returns the conversion of ticks to actual microseconds
//!        This is used to seperate any calculations from getting a timer
//         value for speed critical scenarios
uint32_t microseconds_convert_to_microseconds(uint32_t ticks)
{
    // return the total ticks divided by the number of Mhz the system clock is at to give microseconds
    return (8 * ticks / s_tickPerMicrosecondMul8); //!< Assumes system clock will never be < 0.125 Mhz
}

//! @brief Returns the conversion of microseconds to ticks
uint64_t microseconds_convert_to_ticks(uint32_t microseconds)
{
    return ((uint64_t)microseconds * s_tickPerMicrosecondMul8 / 8);
}

//! @brief Delay specified time
//!
//! @param us Delay time in microseconds unit
void microseconds_delay(uint32_t us)
{
    uint64_t currentTicks = microseconds_get_ticks();

    //! The clock value in Mhz = ticks/microsecond
    uint64_t ticksNeeded = ((uint64_t)us * s_tickPerMicrosecondMul8 / 8) + currentTicks;
    while (microseconds_get_ticks() < ticksNeeded)
    {
        ;
    }
}

//! @brief Set delay time
//!
//! @param us Delay time in microseconds unit
void microseconds_set_delay(uint32_t us)
{
    uint64_t ticks;
    ticks = microseconds_convert_to_ticks(us);
    s_timeoutTicks = microseconds_get_ticks() + ticks;
}

//! @brief Get timeout flag
//!
//! @retval true Timeout is reached.
//! @retval false Timeout isn't reached.
bool microseconds_timeout(void)
{
    uint64_t currentTicks = microseconds_get_ticks();

    return (currentTicks < s_timeoutTicks) ? false : true;
}

#if !defined(CCM)
//! @brief Gets the clock value used for microseconds driver
uint32_t microseconds_get_clock(void)
{
    return get_bus_clock();
}
#endif

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
