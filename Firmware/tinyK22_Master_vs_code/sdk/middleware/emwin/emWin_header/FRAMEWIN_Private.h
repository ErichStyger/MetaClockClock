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
File        : FRAMEWIN_Private.h
Purpose     : FRAMEWIN private header file
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef FRAMEWIN_PRIVATE_H
#define FRAMEWIN_PRIVATE_H

#include "WM_Intern.h"
#include "FRAMEWIN.h"
#include "WIDGET.h"

#if GUI_WINSUPPORT

/******************************************************************
*
*       Config defaults
*
*******************************************************************
*/
//
//Support for 3D effects
//
#ifndef FRAMEWIN_CLIENTCOLOR_DEFAULT
  #if WIDGET_USE_FLEX_SKIN
    #define FRAMEWIN_CLIENTCOLOR_DEFAULT GUI_WHITE
  #else
    #define FRAMEWIN_CLIENTCOLOR_DEFAULT GUI_GRAY_C0
  #endif
#endif

//
// Default for top frame size
//
#ifndef FRAMEWIN_TITLEHEIGHT_DEFAULT
  #define FRAMEWIN_TITLEHEIGHT_DEFAULT 0
#endif

//
// Default for left/right/top/bottom frame size
//
#ifndef FRAMEWIN_BORDER_DEFAULT
  #define FRAMEWIN_BORDER_DEFAULT 3
#endif

//
// Default for inner frame size
//
#ifndef FRAMEWIN_IBORDER_DEFAULT
  #define FRAMEWIN_IBORDER_DEFAULT 1
#endif

//
// Default font
//
#ifndef FRAMEWIN_DEFAULT_FONT
  #if WIDGET_USE_FLEX_SKIN
    #if   WIDGET_USE_SCHEME_SMALL
      #define FRAMEWIN_DEFAULT_FONT &GUI_Font13_1
    #elif WIDGET_USE_SCHEME_MEDIUM
      #define FRAMEWIN_DEFAULT_FONT &GUI_Font16_1
    #elif WIDGET_USE_SCHEME_LARGE
      #define FRAMEWIN_DEFAULT_FONT &GUI_Font24_1
    #endif
  #else
    #if   WIDGET_USE_SCHEME_SMALL
      #define FRAMEWIN_DEFAULT_FONT &GUI_Font8_1
    #elif WIDGET_USE_SCHEME_MEDIUM
      #define FRAMEWIN_DEFAULT_FONT &GUI_Font13_1
    #elif WIDGET_USE_SCHEME_LARGE
      #define FRAMEWIN_DEFAULT_FONT &GUI_Font16_1
    #endif
  #endif
#endif

//
// Default bar color when framewin is active
//
#ifndef FRAMEWIN_BARCOLOR_ACTIVE_DEFAULT
  #define FRAMEWIN_BARCOLOR_ACTIVE_DEFAULT GUI_BLUE
#endif

//
// Default bar color when framewin is inactive
//
#ifndef FRAMEWIN_BARCOLOR_INACTIVE_DEFAULT
  #define FRAMEWIN_BARCOLOR_INACTIVE_DEFAULT GUI_DARKGRAY
#endif

//
// Default frame color
//
#ifndef FRAMEWIN_FRAMECOLOR_DEFAULT
  #define FRAMEWIN_FRAMECOLOR_DEFAULT GUI_GRAY_AA
#endif

//
// Default text color when framewin is active
//
#ifndef FRAMEWIN_TEXTCOLOR_INACTIVE_DEFAULT
  #if WIDGET_USE_FLEX_SKIN
    #define FRAMEWIN_TEXTCOLOR_INACTIVE_DEFAULT GUI_BLACK
  #else
    #define FRAMEWIN_TEXTCOLOR_INACTIVE_DEFAULT GUI_WHITE
  #endif
#endif

//
// Default text color when framewin is inactive
//
#ifndef FRAMEWIN_TEXTCOLOR_ACTIVE_DEFAULT
  #if WIDGET_USE_FLEX_SKIN
    #define FRAMEWIN_TEXTCOLOR_ACTIVE_DEFAULT GUI_BLACK
  #else
    #define FRAMEWIN_TEXTCOLOR_ACTIVE_DEFAULT GUI_WHITE
  #endif
#endif

//
// Default text alignment
//
#ifndef FRAMEWIN_TEXTALIGN_DEFAULT
  #define FRAMEWIN_TEXTALIGN_DEFAULT GUI_TA_LEFT
#endif

/*********************************************************************
*
*       Object definition
*
**********************************************************************
*/
typedef int GUI_HOOK_FUNC(WM_MESSAGE* pMsg);

typedef struct GUI_HOOK {
  struct GUI_HOOK* pNext;
  GUI_HOOK_FUNC*   pHookFunc;
} GUI_HOOK;

typedef struct {
  unsigned              (* pfGetBordersize)(FRAMEWIN_Handle hObj, unsigned Index);
  WIDGET_DRAW_ITEM_FUNC  * pfDrawSkin;
} FRAMEWIN_SKIN_PRIVATE;

typedef struct {
  const GUI_FONT      * pFont;
  GUI_COLOR             aBarColor[2];
  GUI_COLOR             aTextColor[2];
  GUI_COLOR             ClientColor;
  FRAMEWIN_SKIN_PRIVATE SkinPrivate;
  I16                   TitleHeight;
  I16                   BorderSize;
  I16                   IBorderSize;
  I16                   TextAlign;
} FRAMEWIN_PROPS;

typedef struct {
  WIDGET                  Widget;
  FRAMEWIN_PROPS          Props;
  WIDGET_SKIN const     * pWidgetSkin;
  WIDGET_DRAW_ITEM_FUNC * pfDrawItem;      // Only for drawing the title bar...
  WM_CALLBACK           * cb;
  WM_HWIN                 hClient;
  WM_HWIN                 hMenu;
  WM_HWIN                 hText;
  GUI_RECT                rRestore;
  U16                     Flags;
  WM_HWIN                 hFocusedChild;   // Handle to focused child .. default none (0)
  WM_DIALOG_STATUS      * pDialogStatus;
  GUI_HOOK              * pFirstHook;
} FRAMEWIN_Obj;

typedef struct {
  I16      TitleHeight;
  I16      MenuHeight;
  GUI_RECT rClient;
  GUI_RECT rTitleText;
} FRAMEWIN_POSITIONS;

/*********************************************************************
*
*       Macros for internal use
*
**********************************************************************
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  #define FRAMEWIN_INIT_ID(p) (p->Widget.DebugId = WIDGET_TYPE_FRAMEWIN)
#else
  #define FRAMEWIN_INIT_ID(p)
#endif

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  FRAMEWIN_Obj * FRAMEWIN_LockH(FRAMEWIN_Handle h);
  #define FRAMEWIN_LOCK_H(h)   FRAMEWIN_LockH(h)
#else
  #define FRAMEWIN_LOCK_H(h)   (FRAMEWIN_Obj *)WM_LOCK_H(h)
#endif

/*********************************************************************
*
*       Public data (internal defaults)
*
**********************************************************************
*/
extern FRAMEWIN_PROPS      FRAMEWIN__DefaultProps;

extern const WIDGET_SKIN   FRAMEWIN__SkinClassic;
extern       WIDGET_SKIN   FRAMEWIN__Skin;

extern const WIDGET_SKIN * FRAMEWIN__pSkinDefault;

/*********************************************************************
*
*       Private functions
*
**********************************************************************
*/
void     FRAMEWIN__CalcPositions  (FRAMEWIN_Handle hObj, FRAMEWIN_POSITIONS * pPos);
int      FRAMEWIN__CalcTitleHeight(FRAMEWIN_Obj * pObj);
void     FRAMEWIN__UpdatePositions(FRAMEWIN_Handle hObj);
void     FRAMEWIN__UpdateButtons  (FRAMEWIN_Handle hObj, int OldHeight, int OldBorderSizeL, int OldBorderSizeR, int OldBorderSizeT);
void     FRAMEWIN__GetTitleLimits (FRAMEWIN_Handle hObj, int * pxMin, int * pxMax);
unsigned FRAMEWIN__GetBorderSize  (FRAMEWIN_Handle hObj, unsigned Index);

#endif   // GUI_WINSUPPORT
#endif   // FRAMEWIN_PRIVATE_H

/*************************** End of file ****************************/
