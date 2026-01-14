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
File        : SWIPELIST.h
Purpose     : SWIPELIST include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef SWIPELIST_PRIVATE_H
#define SWIPELIST_PRIVATE_H

#include "WM_Intern.h"
#include "SWIPELIST.h"
#include "GUI_ARRAY.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Object definition
*
**********************************************************************
*/
typedef struct {
  WM_HWIN hWin;
  int     xPos;
  int     yPos;
} SWIPELIST_WIN;

typedef struct {
  GUI_ARRAY      WinArray;
  GUI_ARRAY      TextArray;
  int            TextSize;
  WM_HMEM        hDrawObj;
  GUI_COLOR      SepColor;
  int            SepSize;
  int            Pos;
  int            Size;
  int            BitmapAlign;
  int            TextAlign;
  U8             Flags;
  U32            UserData;
} SWIPELIST_ITEM;

typedef struct {
  const GUI_FONT * pSepFont;
  const GUI_FONT * pHeaderFont;
  const GUI_FONT * pTextFont;
  GUI_COLOR        aTextColor[5];
  GUI_COLOR        aBkColor[4];
  int              BitmapSpace;
  int              aBorderSize[4];
  int              Period;
  U8               Flags;
  int              Threshold;
  unsigned         Overlap;
} SWIPELIST_PROPS;

typedef struct {
  WIDGET                  Widget;
  SWIPELIST_PROPS         Props;
  WIDGET_DRAW_ITEM_FUNC * pfDrawItem;
  GUI_ARRAY               ItemArray;
  int                     Pos;
  int                     Size;
  int                     FirstVisible;
  int                     LastVisible;
  int                     Sel;
  int                     ReleasedItem;
  WM_HMEM                 hContext;       // Motion context.
} SWIPELIST_OBJ;

/*********************************************************************
*
*       Macros for internal use
*
**********************************************************************
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  #define SWIPELIST_INIT_ID(p) p->Widget.DebugId = WIDGET_TYPE_SWIPELIST
#else
  #define SWIPELIST_INIT_ID(p)
#endif

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  SWIPELIST_OBJ * SWIPELIST_LockH(SWIPELIST_Handle h);
  #define SWIPELIST_LOCK_H(h)   SWIPELIST_LockH(h)
#else
  #define SWIPELIST_LOCK_H(h)   (SWIPELIST_OBJ *)WM_LOCK_H(h)
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
//
// WM_MOTION_OVERLAP... flags are stored in the upper two bits of Props.Flags
//
#define OVERLAP_FLAG_SHIFT     2
#define OVERLAP_FLAG_MASK      ((WM_MOTION_OVERLAP_TOP | WM_MOTION_OVERLAP_BOTTOM) << OVERLAP_FLAG_SHIFT)

/*********************************************************************
*
*       Module internal data
*
**********************************************************************
*/
extern SWIPELIST_PROPS SWIPELIST__DefaultProps;
extern GUI_COLOR       SWIPELIST__DefaultSepColor;
extern int             SWIPELIST__DefaultSepSize;
extern int             SWIPELIST__DefaultTextAlign;

/*********************************************************************
*
*       Private functions
*
**********************************************************************
*/
void SWIPELIST__SetDrawObj   (SWIPELIST_Handle hObj, int Index, int Align, GUI_DRAW_HANDLE hDrawObj);
void SWIPELIST__CalcItemPos  (GUI_ARRAY ItemArray, int StartIndex, int EndIndex, int AddSize);
int  SWIPELIST__CalcItemSize (SWIPELIST_Handle hObj, int ItemIndex);

#endif   /* if GUI_WINSUPPORT */
#endif   /* SWIPELIST_PRIVATE_H */
