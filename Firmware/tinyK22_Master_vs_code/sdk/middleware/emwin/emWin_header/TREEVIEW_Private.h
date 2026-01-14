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
File        : TREEVIEW_Private.h
Purpose     : TREEVIEW private header file
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef TREEVIEW_PRIVATE_H
#define TREEVIEW_PRIVATE_H

#include "WM_Intern.h"
#include "TREEVIEW.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define TREEVIEW_ITEM_STATE_EXPANDED (1 << 1)
#define TREEVIEW_ITEM_STATE_ISLAST   (1 << 2)

#define TREEVIEW_STATE_HASLINES      (1 << 0)

#define TREEVIEW_COLORS_BK   0
#define TREEVIEW_COLORS_TEXT 1
#define TREEVIEW_COLORS_LINE 2

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  const GUI_BITMAP * apBm[3]; /* Closed, Open, Leaf */
} TREEVIEW_ITEM_DATA;

typedef struct {
  WM_HMEM              hParent; /* Handle to treeview object */
  TREEVIEW_ITEM_Handle hNext;
  TREEVIEW_ITEM_Handle hPrev;
  U32                  UserData;
  WM_HMEM              hData;  /* Handle of TREEVIEW_ITEM_DATA structure */
  U16                  xSize;
  U16                  ySize;
  U16                  Flags;
  U16                  Connectors;
  U8                   Level;  /* 0...15 */
  char                 acText[1];
} TREEVIEW_ITEM_OBJ;

typedef struct {
  const GUI_FONT * pFont;
  GUI_COLOR aBkColor[3];
  GUI_COLOR aTextColor[3];
  GUI_COLOR aLineColor[3];
  GUI_COLOR FocusColor;
  const GUI_BITMAP * apBm[5]; /* Closed, Open, Leaf, Plus, Minus */
  int Indent;
  int TextIndent;
  int MinItemHeight;
} TREEVIEW_PROPS;

typedef struct {
  WIDGET                  Widget;
  WIDGET_DRAW_ITEM_FUNC * pfDrawItem;
  WM_SCROLL_STATE         ScrollStateV;
  WM_SCROLL_STATE         ScrollStateH;
  TREEVIEW_PROPS          Props;
  U16                     Flags;
  TREEVIEW_ITEM_Handle    hFirst;
  TREEVIEW_ITEM_Handle    hLast;
  TREEVIEW_ITEM_Handle    hSel;
  GUI_TIMER_HANDLE        hTimer;
  /* Cache variables */
  int NumItems;
  int NumVisItems;
  int xSizeItems;     /* xSize in pixel used for all visible items */
  int ySizeItems;     /* ySize in pixel used for all visible items */
  I16 xOffPM, yOffPM; /* x/y offset of PM bitmap */
  U16 xOverlapHLine;
} TREEVIEW_OBJ;

/*********************************************************************
*
*       Macros for internal use
*
**********************************************************************
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  #define TREEVIEW_INIT_ID(p) p->Widget.DebugId = WIDGET_TYPE_TREEVIEW
#else
  #define TREEVIEW_INIT_ID(p)
#endif

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  TREEVIEW_OBJ * TREEVIEW_LockH(TREEVIEW_Handle h);
  #define TREEVIEW_LOCK_H(h)   TREEVIEW_LockH(h)
#else
  #define TREEVIEW_LOCK_H(h)   (TREEVIEW_OBJ *)WM_LOCK_H(h)
#endif

/*********************************************************************
*
*       Module internal data
*
**********************************************************************
*/
extern TREEVIEW_PROPS TREEVIEW__DefaultProps;
extern GUI_CONST_STORAGE GUI_BITMAP TREEVIEW__bmOpen;
extern GUI_CONST_STORAGE GUI_BITMAP TREEVIEW__bmClosed;
extern GUI_CONST_STORAGE GUI_BITMAP TREEVIEW__bmLeaf;
extern GUI_CONST_STORAGE GUI_BITMAP TREEVIEW__bmPlus;
extern GUI_CONST_STORAGE GUI_BITMAP TREEVIEW__bmMinus;

#endif   /* GUI_WINSUPPORT */
#endif   /* TREEVIEW_H */

/*************************** End of file ****************************/
