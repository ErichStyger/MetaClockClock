/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 * 
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "GUI.h"
#include "WM.h"
#include "GUIDRV_FlexColor.h" // TODO - choose appropriate LCD driver here - see emWin documentation
#include "emwin_support.h"

#include "fsl_debug_console.h"
#include "fsl_gpio.h"

#ifndef GUI_MEMORY_ADDR
static uint32_t s_gui_memory[(GUI_NUMBYTES + 3) / 4]; /* needs to be word aligned */
#define GUI_MEMORY_ADDR ((uint32_t)s_gui_memory)
#endif

/*******************************************************************************
 * Implementation of communication with the LCD controller
 ******************************************************************************/

	// TODO - implement event handling
	
static void BOARD_LCD_InterfaceInit(void)
{
	// TODO - LCD interface initialization
}

void BOARD_LCD_InterfaceDeinit(void)
{
	// TODO - LCD interface deinitialization
}

/*******************************************************************************
 * Implementation of communication with the touch controller
 ******************************************************************************/

	// TODO - implement event handling

static void BOARD_Touch_InterfaceInit(void)
{
	// TODO - touch interface initialization
}

void BOARD_Touch_InterfaceDeinit(void)
{
	// TODO - touch interface deinitialization
}

int BOARD_Touch_Poll(void)
{
	// TODO - touch handling
	return 0;
}

/*******************************************************************************
 * Application implemented functions required by emWin library
 ******************************************************************************/

void LCD_X_Config(void)
{
    // Initialize the gui device with specified display driver and color conversion
	GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER, COLOR_CONVERSION, 0, 0);
    
    LCD_SetSizeEx(0, LCD_WIDTH, LCD_HEIGHT);
    LCD_SetVSizeEx(0, LCD_WIDTH, LCD_HEIGHT);

    // Initialize touch interface
    BOARD_Touch_InterfaceInit();
}

int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void *pData)
{
    int result = 0;

    switch (Cmd)
    {
        case LCD_X_INITCONTROLLER:
        	BOARD_LCD_InterfaceInit();
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
    volatile int per;
    volatile int i;
    for (per = Period; per > 0; per--)
    {
        for (i = 15000; i > 0; i--)
            ;
    }
}
