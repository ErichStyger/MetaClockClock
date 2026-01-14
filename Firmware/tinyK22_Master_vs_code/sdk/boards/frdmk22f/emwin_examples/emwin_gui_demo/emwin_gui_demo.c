/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "app.h"
#include "emwin_support.h"

#include "GUI.h"
#include "BUTTON.h"
#include "CHECKBOX.h"
#include "SLIDER.h"
#include "DROPDOWN.h"
#include "RADIO.h"
#include "MULTIPAGE.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#ifndef GUI_NORMAL_FONT
#define GUI_NORMAL_FONT (&GUI_Font16_ASCII)
#endif

#ifndef GUI_LARGE_FONT
#define GUI_LARGE_FONT (&GUI_Font16B_ASCII)
#endif

#ifndef GUI_SCALE_FACTOR
#define GUI_SCALE_FACTOR 1
#endif

#ifndef GUI_SCALE_FACTOR_X
#define GUI_SCALE_FACTOR_X GUI_SCALE_FACTOR
#endif

#ifndef GUI_SCALE_FACTOR_Y
#define GUI_SCALE_FACTOR_Y GUI_SCALE_FACTOR
#endif

#define GUI_SCALE(a)                   ((int)((a) * (GUI_SCALE_FACTOR)))
#define GUI_SCALE_X(x)                 ((int)((x) * (GUI_SCALE_FACTOR_X)))
#define GUI_SCALE_Y(y)                 ((int)((y) * (GUI_SCALE_FACTOR_Y)))
#define GUI_SCALE_COORDS(x, y)         GUI_SCALE_X(x), GUI_SCALE_Y(y)
#define GUI_SCALE_RECT(x0, y0, xs, ys) GUI_SCALE_X(x0), GUI_SCALE_Y(y0), GUI_SCALE_X(xs), GUI_SCALE_Y(ys)

#define GUI_ID_DRAWAREA (GUI_ID_USER + 0)
#define GUI_ID_PAGEWIN1 (GUI_ID_USER + 1)
#define GUI_ID_PAGEWIN2 (GUI_ID_USER + 2)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

static DROPDOWN_Handle hDropdown0;
static RADIO_Handle hRadio0;
static CHECKBOX_Handle hCheck0;
static WM_HWIN hDrawArea;

static SLIDER_Handle hSlider0;
static SLIDER_Handle hSlider1;
static SPINBOX_Handle hSpinbox0;
static PROGBAR_Handle hProgbar0;

static const GUI_COLOR color_list[]      = {GUI_BLACK,   GUI_YELLOW, GUI_ORANGE, GUI_RED,
                                       GUI_MAGENTA, GUI_BLUE,   GUI_CYAN,   GUI_GREEN};
static const GUI_POINT triangle_points[] = {
    {GUI_SCALE(0), GUI_SCALE(0)}, {GUI_SCALE(-50), GUI_SCALE(100)}, {GUI_SCALE(50), GUI_SCALE(100)}};

static void cbDrawArea(WM_MESSAGE *pMsg)
{
    switch (pMsg->MsgId)
    {
        case WM_PAINT:
            GUI_SetColor(color_list[DROPDOWN_GetSel(hDropdown0)]);
            switch (RADIO_GetValue(hRadio0))
            {
                case 0:
                    if (CHECKBOX_GetState(hCheck0))
                    {
                        GUI_FillRect(GUI_SCALE_X(70) - GUI_SCALE(50), GUI_SCALE_Y(70) - GUI_SCALE(50),
                                     GUI_SCALE_X(70) + GUI_SCALE(50), GUI_SCALE_Y(70) + GUI_SCALE(50));
                    }
                    else
                    {
                        GUI_DrawRect(GUI_SCALE_X(70) - GUI_SCALE(50), GUI_SCALE_Y(70) - GUI_SCALE(50),
                                     GUI_SCALE_X(70) + GUI_SCALE(50), GUI_SCALE_Y(70) + GUI_SCALE(50));
                    }
                    break;
                case 1:
                    if (CHECKBOX_GetState(hCheck0))
                    {
                        GUI_FillPolygon(triangle_points, 3, GUI_SCALE_COORDS(70, 20));
                    }
                    else
                    {
                        GUI_DrawPolygon(triangle_points, 3, GUI_SCALE_COORDS(70, 20));
                    }
                    break;
                case 2:
                    if (CHECKBOX_GetState(hCheck0))
                    {
                        GUI_FillEllipse(GUI_SCALE_COORDS(70, 70), GUI_SCALE(50), GUI_SCALE(50));
                    }
                    else
                    {
                        GUI_DrawEllipse(GUI_SCALE_COORDS(70, 70), GUI_SCALE(50), GUI_SCALE(50));
                    }
                    break;
            }
            break;
        default:
            WM_DefaultProc(pMsg);
            break;
    }
}

static void cbPageWin1(WM_MESSAGE *pMsg)
{
    int NCode;
    int Id;

    switch (pMsg->MsgId)
    {
        case WM_NOTIFY_PARENT:
            Id    = WM_GetId(pMsg->hWinSrc);
            NCode = pMsg->Data.v;

            switch (Id)
            {
                case GUI_ID_RADIO0:
                    switch (NCode)
                    {
                        case WM_NOTIFICATION_VALUE_CHANGED:
                            WM_InvalidateWindow(hDrawArea);
                            break;
                    }
                    break;

                case GUI_ID_DROPDOWN0:
                    switch (NCode)
                    {
                        case WM_NOTIFICATION_SEL_CHANGED:
                            WM_InvalidateWindow(hDrawArea);
                            break;
                    }
                    break;

                case GUI_ID_CHECK0:
                    switch (NCode)
                    {
                        case WM_NOTIFICATION_VALUE_CHANGED:
                            WM_InvalidateWindow(hDrawArea);
                            break;
                    }
                    break;
            }
            break;

        default:
            WM_DefaultProc(pMsg);
            break;
    }
}

static void cbPageWin2(WM_MESSAGE *pMsg)
{
    int NCode;
    int Id;

    switch (pMsg->MsgId)
    {
        case WM_NOTIFY_PARENT:
            Id    = WM_GetId(pMsg->hWinSrc);
            NCode = pMsg->Data.v;

            switch (Id)
            {
                case GUI_ID_SLIDER0:
                    switch (NCode)
                    {
                        case WM_NOTIFICATION_VALUE_CHANGED:
                            PROGBAR_SetValue(hProgbar0, SLIDER_GetValue(hSlider0));
                            break;
                    }
                    break;

                case GUI_ID_SLIDER1:
                    switch (NCode)
                    {
                        case WM_NOTIFICATION_VALUE_CHANGED:
                            SPINBOX_SetValue(hSpinbox0, SLIDER_GetValue(hSlider1));
                            break;
                    }
                    break;

                case GUI_ID_SPINBOX0:
                    switch (NCode)
                    {
                        case WM_NOTIFICATION_VALUE_CHANGED:
                            SLIDER_SetValue(hSlider1, SPINBOX_GetValue(hSpinbox0));
                            break;
                    }
                    break;
            }
            break;

        default:
            WM_DefaultProc(pMsg);
            break;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_InitHardware();

    PRINTF("GUI demo start.\r\n");

    GUI_Init();

    CHECKBOX_SetDefaultFont(GUI_NORMAL_FONT);
    DROPDOWN_SetDefaultFont(GUI_NORMAL_FONT);
    MULTIPAGE_SetDefaultFont(GUI_NORMAL_FONT);
    RADIO_SetDefaultFont(GUI_NORMAL_FONT);

    /* Create multipage widget */
    MULTIPAGE_Handle hMultipage0;
    hMultipage0 = MULTIPAGE_CreateEx(GUI_SCALE_RECT(10, 10, 300, 220), 0, WM_CF_SHOW, 0, GUI_ID_MULTIPAGE0);

    WM_HWIN hPageWin;

    /* Create window for page 1 and add it */
    hPageWin = WINDOW_CreateEx(GUI_SCALE_RECT(0, 0, 300, 200), WM_HBKWIN, 0, 0, GUI_ID_PAGEWIN1, cbPageWin1);
    MULTIPAGE_AddPage(hMultipage0, hPageWin, "Shapes & Colors");

    /* Create widgets on page 1 */
    hDropdown0 = DROPDOWN_CreateEx(GUI_SCALE_RECT(10, 10, 100, 80), hPageWin, WM_CF_SHOW, DROPDOWN_CF_AUTOSCROLLBAR,
                                   GUI_ID_DROPDOWN0);
    DROPDOWN_SetTextHeight(hDropdown0, GUI_SCALE_Y(20));
    DROPDOWN_SetScrollbarWidth(hDropdown0, GUI_SCALE_X(20));
    DROPDOWN_AddString(hDropdown0, "Black");
    DROPDOWN_AddString(hDropdown0, "Yellow");
    DROPDOWN_AddString(hDropdown0, "Orange");
    DROPDOWN_AddString(hDropdown0, "Red");
    DROPDOWN_AddString(hDropdown0, "Purple");
    DROPDOWN_AddString(hDropdown0, "Blue");
    DROPDOWN_AddString(hDropdown0, "Cyan");
    DROPDOWN_AddString(hDropdown0, "Green");

    hRadio0 =
        RADIO_CreateEx(GUI_SCALE_RECT(10, 50, 100, 90), hPageWin, WM_CF_SHOW, 0, GUI_ID_RADIO0, 3, GUI_SCALE_Y(30));
    RADIO_SetText(hRadio0, "Square", 0);
    RADIO_SetText(hRadio0, "Triangle", 1);
    RADIO_SetText(hRadio0, "Circle", 2);

    hCheck0 = CHECKBOX_CreateEx(GUI_SCALE_RECT(10, 150, 100, 16), hPageWin, WM_CF_SHOW, 0, GUI_ID_CHECK0);
    CHECKBOX_SetText(hCheck0, "Fill");

    hDrawArea =
        WINDOW_CreateEx(GUI_SCALE_RECT(130, 30, 140, 140), hPageWin, WM_CF_SHOW, 0, GUI_ID_DRAWAREA, cbDrawArea);

    /* Create window for page 2 and add it */
    hPageWin = WINDOW_CreateEx(GUI_SCALE_RECT(0, 0, 300, 200), WM_HBKWIN, 0, 0, GUI_ID_PAGEWIN2, cbPageWin2);
    MULTIPAGE_AddPage(hMultipage0, hPageWin, "Sliders & spinbox");

    /* Create widgets on page 2 */
    hSlider0 = SLIDER_CreateEx(GUI_SCALE_RECT(60, 160, 200, 30), hPageWin, WM_CF_SHOW, 0, GUI_ID_SLIDER0);
    SLIDER_SetWidth(hSlider0, GUI_SCALE(10));
    SLIDER_SetValue(hSlider0, 50);

    hSlider1 =
        SLIDER_CreateEx(GUI_SCALE_RECT(10, 10, 30, 180), hPageWin, WM_CF_SHOW, SLIDER_CF_VERTICAL, GUI_ID_SLIDER1);
    SLIDER_SetWidth(hSlider1, GUI_SCALE(10));
    SLIDER_SetValue(hSlider1, 50);

    hSpinbox0 = SPINBOX_CreateEx(GUI_SCALE_RECT(100, 80, 100, 50), hPageWin, WM_CF_SHOW, GUI_ID_SPINBOX0, 0, 100);
    SPINBOX_SetFont(hSpinbox0, GUI_LARGE_FONT);
    SPINBOX_SetValue(hSpinbox0, 50);

    hProgbar0 =
        PROGBAR_CreateEx(GUI_SCALE_RECT(60, 10, 200, 40), hPageWin, WM_CF_SHOW, PROGBAR_CF_HORIZONTAL, GUI_ID_PROGBAR0);
    PROGBAR_SetFont(hProgbar0, GUI_LARGE_FONT);
    PROGBAR_SetValue(hProgbar0, 50);
    PROGBAR_SKINFLEX_PROPS pProps = {0};
    PROGBAR_GetSkinFlexProps(&pProps, 0);
    pProps.ColorText = GUI_BLACK;
    PROGBAR_SetSkinFlexProps(&pProps, 0);

    WM_SetDesktopColor(GUI_WHITE);

    GUI_MULTIBUF_Begin();
    GUI_Exec();
    GUI_MULTIBUF_End();

    while (1)
    {
        /* Poll touch controller for update */
        if (BOARD_Touch_Poll())
        {
            GUI_MULTIBUF_Begin();
            GUI_Exec();
            GUI_MULTIBUF_End();
        }
    }
}
