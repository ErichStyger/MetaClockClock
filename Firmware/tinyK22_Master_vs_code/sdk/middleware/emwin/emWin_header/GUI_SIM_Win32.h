/*********************************************************************
*                SEGGER Microcontroller GmbH                         *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2023  SEGGER Microcontroller GmbH                *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V6.50 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The software has been licensed to  NXP Semiconductors USA, Inc.  whose
registered  office  is  situated  at 411 E. Plumeria Drive, San  Jose,
CA 95134, USA  solely for  the  purposes  of  creating  libraries  for
NXPs M0, M3/M4 and  ARM7/9 processor-based  devices,  sublicensed  and
distributed under the terms and conditions of the NXP End User License
Agreement.
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
Licensing information
Licensor:                 SEGGER Microcontroller Systems LLC
Licensed to:              NXP Semiconductors, 1109 McKay Dr, M/S 76, San Jose, CA 95131, USA
Licensed SEGGER software: emWin
License number:           GUI-00186
License model:            emWin License Agreement, dated August 20th 2011 and Amendment No. 1, dated October 17th 2017 and Amendment No. 2, dated December 18th 2018
Licensed platform:        NXP's ARM 7/9, Cortex-M0, M3, M4, M7, A7, M33
----------------------------------------------------------------------
Support and Update Agreement (SUA)
SUA period:               2011-08-19 - 2025-09-02
Contact to extend SUA:    sales@segger.com
----------------------------------------------------------------------
File        : GUI_SIM_Win32.h
Purpose     : Declares public functions of Simulation
----------------------------------------------------------------------
*/

#ifndef SIM_GUI_H
#define SIM_GUI_H

#include <windows.h>

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/********************************************************************
*
*       Defines
*
*********************************************************************
*/
#ifdef _WIN64
  #ifndef GWL_USERDATA
    #define GWL_USERDATA GWLP_USERDATA
  #endif
#endif

/********************************************************************
*
*       Data
*
*********************************************************************
*/
extern HINSTANCE SIM_GUI_hInst;
extern HWND      SIM_GUI_hWndMain;

/********************************************************************
*
*       Types
*
*********************************************************************
*/
/*********************************************************************
*
*       SIM_GUI_INFO
*
*   Description
*     Contains the window handles of the simulation. This structure
*     is required for SIM_GUI_SetCallback().
*/
typedef struct {
  HWND hWndMain;         // Handle to the main window.
  HWND ahWndLCD[16];     // Array of handles to the display layers.
  HWND ahWndColor[16];   // Array of handles to the palette layers.
} SIM_GUI_INFO;

typedef int  SIM_GUI_tfHook           (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, int * pResult);
typedef void SIM_GUI_tfDelayHandler   (int ms);
typedef void SIM_GUI_tfExecIdleHandler(void);

/********************************************************************
*
*       Interface
*
*********************************************************************
*/
void SIM_GUI_ShowDevice            (int OnOff);
void SIM_GUI_SetCallback           (int (* pfCallback)(SIM_GUI_INFO * pInfo));
void SIM_GUI_HandleKeyEvents       (unsigned Msg, WPARAM wParam);
HWND SIM_GUI_CreateCompositeWindow (HWND hParent, int x, int y, int xSize, int ySize, int DisplayIndex);
HWND SIM_GUI_CreateLCDWindow       (HWND hParent, int x, int y, int xSize, int ySize, int LayerIndex);
HWND SIM_GUI_CreateLOGWindow       (HWND hParent, int x, int y, int xSize, int ySize);
HWND SIM_GUI_CreateLCDInfoWindow   (HWND hParent, int x, int y, int xSize, int ySize, int LayerIndex);
void SIM_GUI_Enable                (void);
int  SIM_GUI_Init                  (HINSTANCE hInst, HWND hWndMain, char * pCmdLine, const char * sAppName);
void SIM_GUI_CopyToClipboard       (int LayerIndex);
void SIM_GUI_SetCompositeWindowHook(SIM_GUI_tfHook * pfHook);
void SIM_GUI_SetLCDWindowHook      (SIM_GUI_tfHook         * pfHook);
void SIM_GUI_SetDelayHandler       (SIM_GUI_tfDelayHandler * pfHandler);
void SIM_GUI_SetExecIdleHandler    (SIM_GUI_tfExecIdleHandler * pfHandler);
void SIM_GUI_GetCompositeSize      (int * pxSize, int * pySize);
int  SIM_GUI_GetTransColor         (void);
void SIM_GUI_GetLCDPos             (int * px, int * py);
void SIM_GUI_Exit                  (void);
void SIM_GUI_SetMessageBoxOnError  (int OnOff);
int  SIM_GUI_App                   (HINSTANCE hInstance,  LPSTR lpCmdLine);
void SIM_GUI_SetDeviceWindowHook   (SIM_GUI_tfHook * pfHook);
void SIM_GUI_SetPixel              (int x, int y, unsigned Color);

void         SIM_GUI_LOG_Time  (void);
void __cdecl SIM_GUI_LOG_Add   (const char *format ,... );
void         SIM_GUI_LOG_AddRed(void);
void         SIM_GUI_LOG_Clear (void);

void LCDSIM_CalcCompositePixels(int * pxSize, int * pySize);
const unsigned char * LCDSIM_GetPixelComposite(void);
int  LCDSIM_GetBackBufferIndex(int LayerIndex);

void LCDSIM_Paint         (HWND hWnd);
void LCDSIM_PaintComposite(HWND hWnd);
void LCDSIM_SetTransMode  (int LayerIndex, int TransMode);
void LCDSIM_SetChroma     (int LayerIndex, unsigned long ChromaMin, unsigned long ChromaMax);

#if defined(__cplusplus)
}
#endif 

#endif /* SIM_GUI_H */
