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
File        : MOVIE.h
Purpose     : MOVIE public header file (API)
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef MOVIE_H
#define MOVIE_H

#include "WM.h"
#include "DIALOG_Type.h"  // Req. for Create indirect data structure
#include "WIDGET.h"

#if GUI_WINSUPPORT

#if defined(__cplusplus)
  extern "C" {  // Make sure we have C-declarations in C++ programs
#endif

/*********************************************************************
*
*       MOVIE bitmap indexes
*
*  Description
*    Bitmap indexes for MOVIE widget.
*/
#define MOVIE_BI_CENTER_PLAY  0  // Bitmap to be shown as play button in the center of the widget.
#define MOVIE_BI_PANEL_PLAY   1  // Bitmap for panel play button.
#define MOVIE_BI_PANEL_PAUSE  2  // Bitmap for panel pause button.
#define MOVIE_BI_PANEL_BEGIN  3  // Bitmap for panel begin button.

/*********************************************************************
*
*       MOVIE color indexes
*
*  Description
*    Color indexes for MOVIE widget.
*/
#define MOVIE_CI_CENTER 0  // Color for center bitmap.
#define MOVIE_CI_PANEL  1  // Color for panel bitmaps.
#define MOVIE_CI_BK     2  // Color for background if MOVIE does not fill the complete widget area.
#define MOVIE_CI_LEFT   3  // Color for progress bar (left).
#define MOVIE_CI_RIGHT  4  // Color for progress bar (right).

/*********************************************************************
*
*       MOVIE period indexes
*
*  Description
*    Period indexes for MOVIE widgets.
*/
#define MOVIE_PI_SHIFT_IN  0  // The length in ms for shifting in the panel.
#define MOVIE_PI_SHIFT_OUT 1  // The length in ms for shifting out the panel.
#define MOVIE_PI_INACTIVE  2  // Period it takes from releasing the widget until the shifting operation is started.

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef WM_HMEM MOVIE_Handle;

/*********************************************************************
*
*       Create function(s)

  Note: the parameters to a create function may vary.
         Some widgets may have multiple create functions
*/
MOVIE_Handle MOVIE_CreateUser(int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id, int NumExtraBytes);

/*********************************************************************
*
*       The callback ...
*
* Do not call it directly ! It is only to be used from within an
* overwritten callback.
*/
void MOVIE_Callback(WM_MESSAGE *pMsg);

/*********************************************************************
*
*       Private functions
*
**********************************************************************
*/
void MOVIE__CalcPanel (void * pVoid);
void MOVIE__CalcCenter(void * pVoid);

/*********************************************************************
*
*       Member functions
*
**********************************************************************
*/
int              MOVIE_GetBarSize     (MOVIE_Handle hObj);
GUI_COLOR        MOVIE_GetColor       (MOVIE_Handle hObj, unsigned ColorIndex);
GUI_MOVIE_HANDLE MOVIE_GethMovie      (MOVIE_Handle hObj);
int              MOVIE_GetInfo        (MOVIE_Handle hObj, GUI_MOVIE_INFO * pInfo);
int              MOVIE_GetPanelSize   (MOVIE_Handle hObj);
int              MOVIE_GetPeriod      (MOVIE_Handle hObj, unsigned PeriodIndex);
int              MOVIE_GetSpace       (MOVIE_Handle hObj);
int              MOVIE_GetUserData    (MOVIE_Handle hObj, void * pDest, int NumBytes);
void             MOVIE_GotoFrame      (MOVIE_Handle hObj, U32 Frame);
int              MOVIE_Pause          (MOVIE_Handle hObj);
int              MOVIE_Play           (MOVIE_Handle hObj);
void             MOVIE_SetBarSize     (MOVIE_Handle hObj, int ySizeBar);
void             MOVIE_SetBitmapObj   (MOVIE_Handle hObj, GUI_DRAW_HANDLE hDrawObj, unsigned Index);
void             MOVIE_SetCenterBitmap(MOVIE_Handle hObj, const GUI_BITMAP * pBmPlay);
void             MOVIE_SetColor       (MOVIE_Handle hObj, unsigned ColorIndex, GUI_COLOR Color);
void             MOVIE_SetColors      (MOVIE_Handle hObj, GUI_COLOR ColorCenter, GUI_COLOR ColorPanel, GUI_COLOR ColorLeft, GUI_COLOR ColorRight, GUI_COLOR ColorBk);
void             MOVIE_SetData        (MOVIE_Handle hObj, const U8 * pData, U32 Size, int DoLoop, GUI_MOVIE_FUNC * pfNotify);
void             MOVIE_SetDataEx      (MOVIE_Handle hObj, GUI_MOVIE_GET_DATA_FUNC * pfGetData, void * pVoidImage, void * pVoidTable, int DoLoop, GUI_MOVIE_FUNC * pfNotify);
void             MOVIE_SetPanelBitmaps(MOVIE_Handle hObj, const GUI_BITMAP * pBmPlay, const GUI_BITMAP * pBmPause, const GUI_BITMAP * pBmStart);
void             MOVIE_SetPeriod      (MOVIE_Handle hObj, unsigned PeriodIndex, int Period);
void             MOVIE_SetPeriods     (MOVIE_Handle hObj, int PeriodShiftIn, int PeriodShiftOut, int PeriodInactive);
void             MOVIE_SetPanelSize   (MOVIE_Handle hObj, int ySizePanel);
void             MOVIE_SetSizes       (MOVIE_Handle hObj, int ySizePanel, int ySizeBar, int Space);
void             MOVIE_SetSpace       (MOVIE_Handle hObj, int Space);
int              MOVIE_SetUserData    (MOVIE_Handle hObj, const void * pSrc, int NumBytes);

/*********************************************************************
*
*       Managing default values
*
**********************************************************************
*/
GUI_COLOR MOVIE_GetDefaultColor    (unsigned ColorIndex);
int       MOVIE_GetDefaultPeriod   (unsigned PeriodIndex);
int       MOVIE_GetDefaultSizeBar  (void);
int       MOVIE_GetDefaultSizePanel(void);
int       MOVIE_GetDefaultSpace    (void);
void      MOVIE_SetDefaultColor    (unsigned ColorIndex, GUI_COLOR Color);
void      MOVIE_SetDefaultPeriod   (unsigned PeriodIndex, int Period);
void      MOVIE_SetDefaultSizeBar  (int SizeBar);
void      MOVIE_SetDefaultSizePanel(int SizePanel);
void      MOVIE_SetDefaultSpace    (int Space);

#if defined(__cplusplus)
  }
#endif

#endif   // GUI_WINSUPPORT
#endif   // MOVIE_H

/*************************** End of file ****************************/
