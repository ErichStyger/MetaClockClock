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
File        : HEADER_Private.h
Purpose     : Private HEADER include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef HEADER_PRIVATE_H
#define HEADER_PRIVATE_H

#include "WM_Intern.h"
#include "HEADER.h"
#include "WIDGET.h"
#include "GUI_ARRAY.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Object definition
*
**********************************************************************
*/
typedef struct {
  int     Width;
  I16     Align;
  WM_HMEM hDrawObj;
  char    acText[1];
} HEADER_COLUMN;

typedef struct {
  WIDGET_DRAW_ITEM_FUNC * pfDrawSkin;
} HEADER_SKIN_PRIVATE;

typedef struct {
  const GUI_FONT    * pFont;
  GUI_COLOR           BkColor;
  GUI_COLOR           TextColor;
  GUI_COLOR           ArrowColor;
  HEADER_SKIN_PRIVATE SkinPrivate;
  U8                  BorderH;
  U8                  BorderV;
} HEADER_PROPS;

typedef struct {
  WIDGET              Widget;
  HEADER_PROPS        Props;
  WIDGET_SKIN const * pWidgetSkin;
  GUI_ARRAY           Columns;
  int                 CapturePosX;
  int                 CaptureItem;
  int                 ScrollPos;
  int                 Sel;
  int                 DirIndicatorColumn;
  int                 DirIndicatorReverse;
  unsigned            Fixed;
  U8                  DragLimit;
  U8                  ResizeableColumns;
  U8                  Height;
  U8                  xOff;
} HEADER_Obj;

/*********************************************************************
*
*       Private (module internal) data
*
**********************************************************************
*/

extern HEADER_PROPS        HEADER__DefaultProps;
extern const GUI_CURSOR  * HEADER__pDefaultCursor;

extern const WIDGET_SKIN   HEADER__SkinClassic;
extern       WIDGET_SKIN   HEADER__Skin;

extern WIDGET_SKIN const * HEADER__pSkinDefault;

/*********************************************************************
*
*       Macros for internal use
*
**********************************************************************
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  #define HEADER_INIT_ID(p)  (p->Widget.DebugId = WIDGET_TYPE_HEADER)
#else
  #define HEADER_INIT_ID(p)
#endif

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  HEADER_Obj * HEADER_LockH(HEADER_Handle h);
  #define HEADER_LOCK_H(h)   HEADER_LockH(h)
#else
  #define HEADER_LOCK_H(h)   (HEADER_Obj *)WM_LOCK_H(h)
#endif

/*********************************************************************
*
*       Private (module internal) functions
*
**********************************************************************
*/
void         HEADER__SetDrawObj       (HEADER_Handle hObj, unsigned Index, GUI_DRAW_HANDLE hDrawObj);
void         HEADER__SetOffset        (HEADER_Handle hObj, U8 xOff);
const char * HEADER__GetItemTextLocked(HEADER_Handle hObj, unsigned Index, void ** ppUnlock);

#endif // GUI_WINSUPPORT
#endif // Avoid multiple inclusion

/*************************** End of file ****************************/
