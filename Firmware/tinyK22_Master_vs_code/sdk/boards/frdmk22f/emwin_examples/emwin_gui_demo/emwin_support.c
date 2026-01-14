/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "GUI.h"
#include "WM.h"
#include "GUIDRV_FlexColor.h"
#include "emwin_support.h"

#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "fsl_dspi.h"
#include "fsl_dspi_cmsis.h"
#include "fsl_ili9341.h"
#include "fsl_i2c_cmsis.h"
#include "fsl_ft6x06.h"

#ifndef GUI_MEMORY_ADDR
static uint32_t s_gui_memory[(GUI_NUMBYTES + 3) / 4]; /* needs to be word aligned */
#define GUI_MEMORY_ADDR ((uint32_t)s_gui_memory)
#endif

/*******************************************************************************
 * Implementation of PortAPI for emWin LCD driver
 ******************************************************************************/
static volatile uint32_t spi_event;
static volatile bool spi_event_received;

static void SPI_MasterSignalEvent(uint32_t event)
{
    spi_event          = event;
    spi_event_received = true;
}

static uint32_t SPI_WaitEvent(void)
{
    uint32_t event;

    while (!spi_event_received)
        ;

    event              = spi_event;
    spi_event_received = false;

    return event;
}

static void APP_pfWrite8_A0(U8 Data)
{
    GPIO_PortClear(BOARD_LCD_DC_GPIO, 1u << BOARD_LCD_DC_GPIO_PIN);
    BOARD_LCD_SPI.Send(&Data, 1);
    SPI_WaitEvent();
}

static void APP_pfWrite8_A1(U8 Data)
{
    GPIO_PortSet(BOARD_LCD_DC_GPIO, 1u << BOARD_LCD_DC_GPIO_PIN);
    BOARD_LCD_SPI.Send(&Data, 1);
    SPI_WaitEvent();
}

static void APP_pfWriteM8_A1(U8 *pData, int NumItems)
{
    GPIO_PortSet(BOARD_LCD_DC_GPIO, 1u << BOARD_LCD_DC_GPIO_PIN);
    BOARD_LCD_SPI.Send(pData, NumItems);
    SPI_WaitEvent();
}

static U8 APP_pfRead8_A1(void)
{
    uint8_t Data;
#if defined(BOARD_LCD_READABLE) && (BOARD_LCD_READABLE == 0)
    PRINTF("Warning: LCD does not support read operation, the image may get distorted.\r\n");
    assert(0);
#endif
    GPIO_PortSet(BOARD_LCD_DC_GPIO, 1u << BOARD_LCD_DC_GPIO_PIN);
    BOARD_LCD_SPI.Receive(&Data, 1);
    SPI_WaitEvent();
    return Data;
}

static void APP_pfReadM8_A1(U8 *pData, int NumItems)
{
#if defined(BOARD_LCD_READABLE) && (BOARD_LCD_READABLE == 0)
    PRINTF("Warning: LCD does not support read operation, the image may get distorted.\r\n");
    assert(0);
#endif
    GPIO_PortSet(BOARD_LCD_DC_GPIO, 1u << BOARD_LCD_DC_GPIO_PIN);
    BOARD_LCD_SPI.Receive(pData, NumItems);
    SPI_WaitEvent();
}

static void BOARD_LCD_InterfaceInit(void)
{
    /* Define the init structure for the data/command output pin */
    gpio_pin_config_t dc_config = {
        kGPIO_DigitalOutput,
        1,
    };

    /* Init data/command GPIO output . */
    GPIO_PinInit(BOARD_LCD_DC_GPIO, BOARD_LCD_DC_GPIO_PIN, &dc_config);

/* DMA Mux init and EDMA init */
#ifdef BOARD_LCD_DSPI_DMA_BASEADDR
    edma_config_t edmaConfig = {0};
    EDMA_GetDefaultConfig(&edmaConfig);
    EDMA_Init(BOARD_LCD_DSPI_DMA_BASEADDR, &edmaConfig);
#endif
#ifdef BOARD_LCD_DSPI_DMA_MUX_BASEADDR
    DMAMUX_Init(BOARD_LCD_DSPI_DMA_MUX_BASEADDR);
#endif

    /* SPI master init */
    BOARD_LCD_SPI.Initialize(SPI_MasterSignalEvent);
    BOARD_LCD_SPI.PowerControl(ARM_POWER_FULL);
    BOARD_LCD_SPI.Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_DATA_BITS(8), BOARD_LCD_SPI_BAUDRATE);

#ifdef BOARD_LCD_SPI_IRQ
    NVIC_SetPriority(BOARD_LCD_SPI_IRQ, 1U);
#endif
}

void BOARD_LCD_InterfaceDeinit(void)
{
    BOARD_LCD_SPI.PowerControl(ARM_POWER_OFF);
    BOARD_LCD_SPI.Uninitialize();
}

/*******************************************************************************
 * Implementation of communication with the touch controller
 ******************************************************************************/
static ft6x06_handle_t touch_handle;

static void I2C_MasterSignalEvent(uint32_t event)
{
    /* Notify touch driver about status of the IO operation */
    FT6X06_EventHandler(&touch_handle, event);
}

static void BOARD_Touch_InterfaceInit(void)
{
    NVIC_SetPriority(BOARD_TOUCH_I2C_IRQ, 0);

    /*Init I2C1 */
    BOARD_TOUCH_I2C.Initialize(I2C_MasterSignalEvent);
    BOARD_TOUCH_I2C.PowerControl(ARM_POWER_FULL);
    BOARD_TOUCH_I2C.Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
}

void BOARD_Touch_InterfaceDeinit(void)
{
    BOARD_TOUCH_I2C.PowerControl(ARM_POWER_OFF);
    BOARD_TOUCH_I2C.Uninitialize();
}

int BOARD_Touch_Poll(void)
{
    touch_event_t touch_event;
    int touch_x;
    int touch_y;
    GUI_PID_STATE pid_state;

    if (kStatus_Success != FT6X06_GetSingleTouch(&touch_handle, &touch_event, &touch_x, &touch_y))
    {
        return 0;
    }
    else if (touch_event != kTouch_Reserved)
    {
        pid_state.x       = LCD_WIDTH - touch_y;
        pid_state.y       = touch_x;
        pid_state.Pressed = ((touch_event == kTouch_Down) || (touch_event == kTouch_Contact));
        pid_state.Layer   = 0;
        GUI_TOUCH_StoreStateEx(&pid_state);
        return 1;
    }
    return 0;
}

/*******************************************************************************
 * Application implemented functions required by emWin library
 ******************************************************************************/
void LCD_X_Config(void)
{
    GUI_DEVICE *pDevice;
    GUI_PORT_API PortAPI;
    CONFIG_FLEXCOLOR Config = {0, 0, GUI_SWAP_XY, 0, 1};

    pDevice = GUI_DEVICE_CreateAndLink(GUIDRV_FLEXCOLOR, GUICC_565, 0, 0);
    GUIDRV_FlexColor_Config(pDevice, &Config);

    LCD_SetSizeEx(0, LCD_WIDTH, LCD_HEIGHT);
    LCD_SetVSizeEx(0, LCD_WIDTH, LCD_HEIGHT);

    PortAPI.pfWrite8_A0  = APP_pfWrite8_A0;
    PortAPI.pfWrite8_A1  = APP_pfWrite8_A1;
    PortAPI.pfWriteM8_A1 = APP_pfWriteM8_A1;
    PortAPI.pfRead8_A1   = APP_pfRead8_A1;
    PortAPI.pfReadM8_A1  = APP_pfReadM8_A1;
    GUIDRV_FlexColor_SetFunc(pDevice, &PortAPI, GUIDRV_FLEXCOLOR_F66709, GUIDRV_FLEXCOLOR_M16C0B8);

    BOARD_Touch_InterfaceInit();
    FT6X06_Init(&touch_handle, &BOARD_TOUCH_I2C);
}

int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void *pData)
{
    int result = 0;

    switch (Cmd)
    {
        case LCD_X_INITCONTROLLER:
            BOARD_LCD_InterfaceInit();
            GUI_X_Delay(50); /* settle down delay after reset */
            FT9341_Init(APP_pfWrite8_A1, APP_pfWrite8_A0);
            break;
        default:
            result = -1;
            break;
    }

    return result;
}

void GUI_X_Config(void)
{
    /* Assign work memory area to emWin */
    GUI_ALLOC_AssignMemory((void *)GUI_MEMORY_ADDR, GUI_NUMBYTES);

    /* Select default font */
    GUI_SetDefaultFont(GUI_FONT_6X8);
}

void GUI_X_Init(void)
{
}

/* Dummy RTOS stub required by emWin */
void GUI_X_InitOS(void)
{
}

/* Dummy RTOS stub required by emWin */
void GUI_X_Lock(void)
{
}

/* Dummy RTOS stub required by emWin */
void GUI_X_Unlock(void)
{
}

/* Dummy RTOS stub required by emWin */
U32 GUI_X_GetTaskId(void)
{
    return 0;
}

void GUI_X_ExecIdle(void)
{
}

GUI_TIMER_TIME GUI_X_GetTime(void)
{
    return 0;
}

void GUI_X_Delay(int Period)
{
    volatile int i;
    for (; Period > 0; Period--)
    {
        for (i = 15000; i > 0; i--)
            ;
    }
}
