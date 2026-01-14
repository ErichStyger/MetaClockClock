/*
 * Copyright (c) 2014-2015 Freescale Semiconductor, Inc.
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
#include "bootloader_common.h"
#include "bl_context.h"
#include "fsl_device_registers.h"
#include "fsl_port.h"
#include "peripherals_pinmux.h"
#include "pin_mux.h"

#if (BL_CONFIG_LPUART || BL_CONFIG_SCUART || BL_CONFIG_UART)

#if BL_ENABLE_PINMUX_UART4
#define BL_ENABLED_MAX_UART_INSTANCE (4)
#elif BL_ENABLE_PINMUX_UART3
#define BL_ENABLED_MAX_UART_INSTANCE (3)
#elif BL_ENABLE_PINMUX_UART2
#define BL_ENABLED_MAX_UART_INSTANCE (2)
#elif BL_ENABLE_PINMUX_UART1
#define BL_ENABLED_MAX_UART_INSTANCE (1)
#elif BL_ENABLE_PINMUX_UART0
#define BL_ENABLED_MAX_UART_INSTANCE (0)
#endif

//! UART autobaud port irq configurations
#define PORT_IRQC_INTERRUPT_ENABLED_PRIORITY 1
#define PORT_IRQC_INTERRUPT_RESTORED_PRIORITY 0

#define PORT_IRQC_INTERRUPT_FALLING_EDGE 0xA
#define PORT_IRQC_INTERRUPT_DISABLE 0

//! this is to store the function pointer for calling back to the function that wants
//! the UART RX instance pin that triggered the interrupt. This only supports 1 pin
//! for UART0 because UART1 is on PORTC which does not support interrupts :(

static pin_irq_callback_t s_pin_irq_func[BL_ENABLED_MAX_UART_INSTANCE + 1] = { 0 };

#endif // BL_CONFIG_LPUART || BL_CONFIG_SCUART || BL_CONFIG_UART

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
uint32_t PORT_GetPinIsf(PORT_Type *base, uint32_t pin)
{
    return (base->PCR[pin] & PORT_PCR_ISF_MASK) >> PORT_PCR_ISF_SHIFT;
}

/*!
 * @brief Configure pinmux for uart module.
 *
 * This function only support switching default or gpio or fixed-ALTx mode on fixed pins
 * (Although there are many ALTx-pinmux configuration choices on various pins for the same
 * peripheral module)
 */
void uart_pinmux_config(uint32_t instance, pinmux_type_t pinmux)
{
    switch (instance)
    {
#if BL_ENABLE_PINMUX_UART0
        case 0:
            switch (pinmux)
            {
                case kPinmuxType_Default:
                    UART0_RestoreDefault();
                    break;
                case kPinmuxType_PollForActivity:
                    UART0_PollForActivity();
                    break;
                case kPinmuxType_Peripheral:
                    UART0_InitPins();
                    break;
                default:
                    break;
            }
            break;
#endif // #if BL_ENABLE_PINMUX_UART0

#if BL_ENABLE_PINMUX_UART1
        case 1:
            switch (pinmux)
            {
                case kPinmuxType_Default:
                    UART1_RestoreDefault();
                    break;
                case kPinmuxType_PollForActivity:
                    UART1_PollForActivity();
                    break;
                case kPinmuxType_Peripheral:
                    UART1_InitPins();
                    break;
                default:
                    break;
            }
            break;
#endif // #if BL_ENABLE_PINMUX_UART1

#if BL_ENABLE_PINMUX_UART2
        case 2:
            switch (pinmux)
            {
                case kPinmuxType_Default:
                    UART2_RestoreDefault();
                    break;
                case kPinmuxType_PollForActivity:
                    UART2_PollForActivity();
                    break;
                case kPinmuxType_Peripheral:
                    UART2_InitPins();
                    break;
                default:
                    break;
            }
            break;
#endif // #if BL_ENABLE_PINMUX_UART2

#if BL_ENABLE_PINMUX_UART3
        case 3:
            switch (pinmux)
            {
                case kPinmuxType_Default:
                    UART3_RestoreDefault();
                    break;
                case kPinmuxType_PollForActivity:
                    UART3_PollForActivity();
                    break;
                case kPinmuxType_Peripheral:
                    UART3_InitPins();
                    break;
                default:
                    break;
            }
            break;
#endif // #if BL_ENABLE_PINMUX_UART3

#if BL_ENABLE_PINMUX_UART4
        case 4:
            switch (pinmux)
            {
                case kPinmuxType_Default:
                    UART4_RestoreDefault();
                    break;
                case kPinmuxType_PollForActivity:
                    UART4_PollForActivity();
                    break;
                case kPinmuxType_Peripheral:
                    UART4_InitPins();
                    break;
                default:
                    break;
            }
            break;
#endif // #if BL_ENABLE_PINMUX_UART4

        default:
            break;
    }
}

/*!
 * @brief Configure pinmux for i2c module.
 *
 * This function only support switching default or gpio or fixed-ALTx mode on fixed pins
 * (Although there are many ALTx-pinmux configuration choices on various pins for the same
 * peripheral module)
 */
void i2c_pinmux_config(uint32_t instance, pinmux_type_t pinmux)
{
    switch (instance)
    {
#if BL_ENABLE_PINMUX_I2C0
        case 0:
            switch (pinmux)
            {
                case kPinmuxType_Default:
                    I2C0_RestoreDefault();
                    break;
                case kPinmuxType_Peripheral:
                    I2C0_InitPins();
                    break;
                default:
                    break;
            }
            break;
#endif // #if BL_ENABLE_PINMUX_I2C0

#if BL_ENABLE_PINMUX_I2C1
        case 1:
            switch (pinmux)
            {
                case kPinmuxType_Default:
                    I2C1_RestoreDefault();
                    break;
                case kPinmuxType_Peripheral:
                    I2C1_InitPins();
                    break;
                default:
                    break;
            }
            break;
#endif // #if BL_ENABLE_PINMUX_I2C1

#if BL_ENABLE_PINMUX_I2C2
        case 2:
            switch (pinmux)
            {
                case kPinmuxType_Default:
                    I2C2_RestoreDefault();
                    break;
                case kPinmuxType_Peripheral:
                    I2C2_InitPins();
                    break;
                default:
                    break;
            }
#endif // #if BL_ENABLE_PINMUX_I2C2

        default:
            break;
    }
}

/*!
 * @brief Configure pinmux for SPI module.
 *
 * This function only support switching default or gpio or fixed-ALTx mode on fixed pins
 * (Although there are many ALTx-pinmux configuration choices on various pins for the same
 * peripheral module)
 */
void spi_pinmux_config(uint32_t instance, pinmux_type_t pinmux)
{
    switch (instance)
    {
#if BL_ENABLE_PINMUX_SPI0
        case 0:
            switch (pinmux)
            {
                case kPinmuxType_Default:
                    SPI0_RestoreDefault();
                    break;
                case kPinmuxType_Peripheral:
                    SPI0_InitPins();
                    break;
                default:
                    break;
            }
            break;
#endif // #if BL_ENABLE_PINMUX_SPI0

#if BL_ENABLE_PINMUX_SPI1
        case 1:
            switch (pinmux)
            {
                case kPinmuxType_Default:
                    SPI1_RestoreDefault();
                    break;
                case kPinmuxType_Peripheral:
                    SPI1_InitPins();
                    break;
                default:
                    break;
            }
            break;
#endif // #if BL_ENABLE_PINMUX_SPI1

#if BL_ENABLE_PINMUX_SPI2
        case 2:
            switch (pinmux)
            {
                case kPinmuxType_Default:
                    SPI2_RestoreDefault();
                    break;
                case kPinmuxType_Peripheral:
                    SPI2_InitPins();
                    break;
                default:
                    break;
            }
            break;
#endif // #if BL_ENABLE_PINMUX_SPI2

        default:
            break;
    }
}

/*!
 * @brief Configure pinmux for CAN module.
 *
 * This function only support switching default or gpio or fixed-ALTx mode on fixed pins
 * (Although there are many ALTx-pinmux configuration choices on various pins for the same
 * peripheral module)
 */
void can_pinmux_config(uint32_t instance, pinmux_type_t pinmux)
{
    switch (instance)
    {
#if BL_ENABLE_PINMUX_CAN0
        case 0:
            switch (pinmux)
            {
                case kPinmuxType_Default:
                    CAN0_RestoreDefault();
                    break;
                case kPinmuxType_Peripheral:
                    CAN0_InitPins();
                    break;
                default:
                    break;
            }
            break;
#endif // #if BL_ENABLE_PINMUX_CAN0

        default:
            break;
    }
}

//! @brief this is going to be used for autobaud IRQ handling for UART0
#if BL_ENABLE_PINMUX_UART0
void UART0_RX_GPIO_IRQHandler(void)
{
    // Check if the pin for UART0 is what triggered the RX PORT interrupt
    if (PORT_GetPinIsf(UART0_RX_PORT_BASE, UART0_RX_GPIO_PIN_NUM) && s_pin_irq_func[0])
    {
        s_pin_irq_func[0](0);
        PORT_ClearPinsInterruptFlags(UART0_RX_PORT_BASE, ~0U);
    }
}
#endif // #if BL_ENABLE_PINMUX_UART0

//! @brief this is going to be used for autobaud IRQ handling for UART1
#if BL_ENABLE_PINMUX_UART1
void UART1_RX_GPIO_IRQHandler(void)
{
    // Check if the pin for UART1 is what triggered the RX PORT interrupt
    if (PORT_GetPinIsf(UART1_RX_PORT_BASE, UART1_RX_GPIO_PIN_NUM) && s_pin_irq_func[1])
    {
        s_pin_irq_func[1](1);
        PORT_ClearPinsInterruptFlags(UART1_RX_PORT_BASE, ~0U);
    }
}
#endif // #if BL_ENABLE_PINMUX_UART1

//! @brief this is going to be used for autobaud IRQ handling for UART2
#if BL_ENABLE_PINMUX_UART2
void UART2_RX_GPIO_IRQHandler(void)
{
    // Check if the pin for UART2 is what triggered the RX PORT interrupt
    if (PORT_GetPinIsf(UART2_RX_PORT_BASE, UART2_RX_GPIO_PIN_NUM) && s_pin_irq_func[2])
    {
        s_pin_irq_func[2](2);
        PORT_ClearPinsInterruptFlags(UART2_RX_PORT_BASE, ~0U);
    }
}
#endif // #if BL_ENABLE_PINMUX_UART2

//! @brief this is going to be used for autobaud IRQ handling for UART3
#if BL_ENABLE_PINMUX_UART3
void UART3_RX_GPIO_IRQHandler(void)
{
    // Check if the pin for UART3 is what triggered the RX PORT interrupt
    if (PORT_GetPinIsf(UART3_RX_PORT_BASE, UART3_RX_GPIO_PIN_NUM) && s_pin_irq_func[3])
    {
        s_pin_irq_func[3](3);
        PORT_ClearPinsInterruptFlags(UART3_RX_PORT_BASE, ~0U);
    }
}
#endif // #if BL_ENABLE_PINMUX_UART3

//! @brief this is going to be used for autobaud IRQ handling for UART4
#if BL_ENABLE_PINMUX_UART4
void UART4_RX_GPIO_IRQHandler(void)
{
    // Check if the pin for UART4 is what triggered the RX PORT interrupt
    if (PORT_GetPinIsf(UART4_RX_PORT_BASE, UART4_RX_GPIO_PIN_NUM) && s_pin_irq_func[4])
    {
        s_pin_irq_func[4](4);
        PORT_ClearPinsInterruptFlags(UART4_RX_PORT_BASE, ~0U);
    }
}
#endif // #if BL_ENABLE_PINMUX_UART4

void enable_autobaud_pin_irq(uint32_t instance, pin_irq_callback_t func)
{
    switch (instance)
    {
#if BL_ENABLE_PINMUX_UART0
        case 0:
            // Only look for a falling edge for our interrupts
            PORT_SetPinInterruptConfig(UART0_RX_PORT_BASE, UART0_RX_GPIO_PIN_NUM, kPORT_InterruptFallingEdge);
            NVIC_SetPriority(UART0_RX_GPIO_IRQn, PORT_IRQC_INTERRUPT_ENABLED_PRIORITY);
            NVIC_EnableIRQ(UART0_RX_GPIO_IRQn);
            s_pin_irq_func[0] = func;
            break;
#endif // #if BL_ENABLE_PINMUX_UART0

#if BL_ENABLE_PINMUX_UART1
        case 1:
            // Only look for a falling edge for our interrupts
            PORT_SetPinInterruptConfig(UART1_RX_PORT_BASE, UART1_RX_GPIO_PIN_NUM, kPORT_InterruptFallingEdge);
            NVIC_SetPriority(UART1_RX_GPIO_IRQn, PORT_IRQC_INTERRUPT_ENABLED_PRIORITY);
            NVIC_EnableIRQ(UART1_RX_GPIO_IRQn);            
            s_pin_irq_func[1] = func;
            break;
#endif // #if BL_ENABLE_PINMUX_UART1

#if BL_ENABLE_PINMUX_UART2
        case 2:
            // Only look for a falling edge for our interrupts
            PORT_SetPinInterruptConfig(UART2_RX_PORT_BASE, UART2_RX_GPIO_PIN_NUM, kPORT_InterruptFallingEdge);
            NVIC_SetPriority(UART2_RX_GPIO_IRQn, PORT_IRQC_INTERRUPT_ENABLED_PRIORITY);
            NVIC_EnableIRQ(UART2_RX_GPIO_IRQn);            
            s_pin_irq_func[2] = func;
            break;
#endif // #if BL_ENABLE_PINMUX_UART2

#if BL_ENABLE_PINMUX_UART3
        case 3:
            // Only look for a falling edge for our interrupts
            PORT_SetPinInterruptConfig(UART3_RX_PORT_BASE, UART3_RX_GPIO_PIN_NUM, kPORT_InterruptFallingEdge);
            NVIC_SetPriority(UART3_RX_GPIO_IRQn, PORT_IRQC_INTERRUPT_ENABLED_PRIORITY);
            NVIC_EnableIRQ(UART3_RX_GPIO_IRQn);
            s_pin_irq_func[3] = func;
            break;
#endif // #if BL_ENABLE_PINMUX_UART3

#if BL_ENABLE_PINMUX_UART4
        case 4:
            // Only look for a falling edge for our interrupts
            PORT_SetPinInterruptConfig(UART4_RX_PORT_BASE, UART4_RX_GPIO_PIN_NUM, kPORT_InterruptFallingEdge);
            NVIC_SetPriority(UART4_RX_GPIO_IRQn, PORT_IRQC_INTERRUPT_ENABLED_PRIORITY);
            NVIC_EnableIRQ(UART4_RX_GPIO_IRQn);            
            s_pin_irq_func[4] = func;
            break;
#endif // #if BL_ENABLE_PINMUX_UART4

        default:
            break;
    }
}

void disable_autobaud_pin_irq(uint32_t instance)
{
    switch (instance)
    {
#if BL_ENABLE_PINMUX_UART0
        case 0:
            NVIC_DisableIRQ(UART0_RX_GPIO_IRQn);
            NVIC_SetPriority(UART0_RX_GPIO_IRQn, PORT_IRQC_INTERRUPT_RESTORED_PRIORITY);
            PORT_SetPinInterruptConfig(UART0_RX_PORT_BASE, UART0_RX_GPIO_PIN_NUM, kPORT_InterruptOrDMADisabled);
            s_pin_irq_func[0] = 0;
            break;
#endif // #if BL_ENABLE_PINMUX_UART0

#if BL_ENABLE_PINMUX_UART1
        case 1:
            NVIC_DisableIRQ(UART1_RX_GPIO_IRQn);
            NVIC_SetPriority(UART1_RX_GPIO_IRQn, PORT_IRQC_INTERRUPT_RESTORED_PRIORITY);
            PORT_SetPinInterruptConfig(UART1_RX_PORT_BASE, UART1_RX_GPIO_PIN_NUM, kPORT_InterruptOrDMADisabled);
            s_pin_irq_func[1] = 0;
            break;
#endif // #if BL_ENABLE_PINMUX_UART1

#if BL_ENABLE_PINMUX_UART2
        case 2:
            NVIC_DisableIRQ(UART2_RX_GPIO_IRQn);
            NVIC_SetPriority(UART2_RX_GPIO_IRQn, PORT_IRQC_INTERRUPT_RESTORED_PRIORITY);
            PORT_SetPinInterruptConfig(UART2_RX_PORT_BASE, UART2_RX_GPIO_PIN_NUM, kPORT_InterruptOrDMADisabled);
            s_pin_irq_func[2] = 0;
            break;
#endif // #if BL_ENABLE_PINMUX_UART2

#if BL_ENABLE_PINMUX_UART3
        case 3:
            NVIC_DisableIRQ(UART3_RX_GPIO_IRQn);
            NVIC_SetPriority(UART3_RX_GPIO_IRQn, PORT_IRQC_INTERRUPT_RESTORED_PRIORITY);
            PORT_SetPinInterruptConfig(UART3_RX_PORT_BASE, UART3_RX_GPIO_PIN_NUM, kPORT_InterruptOrDMADisabled);
            s_pin_irq_func[3] = 0;
            break;
#endif // #if BL_ENABLE_PINMUX_UART3

#if BL_ENABLE_PINMUX_UART4
        case 4:
            NVIC_DisableIRQ(UART4_RX_GPIO_IRQn);
            NVIC_SetPriority(UART4_RX_GPIO_IRQn, PORT_IRQC_INTERRUPT_RESTORED_PRIORITY);
            PORT_SetPinInterruptConfig(UART4_RX_PORT_BASE, UART4_RX_GPIO_PIN_NUM, kPORT_InterruptOrDMADisabled);
            s_pin_irq_func[4] = 0;
            break;
#endif // #if BL_ENABLE_PINMUX_UART4

        default:
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
