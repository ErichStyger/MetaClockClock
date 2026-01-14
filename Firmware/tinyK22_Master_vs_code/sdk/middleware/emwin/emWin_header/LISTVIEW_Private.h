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
File        : LISTVIEW_Private.h
Purpose     : Private LISTVIEW include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef LISTVIEW_PRIVATE_H
#define LISTVIEW_PRIVATE_H

#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

#include <stddef.h>

#include "LISTVIEW.h"
#include "GUI_ARRAY.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define SORT_TYPE                 U16

#define LISTVIEW_CELL_INFO_COLORS (1 << 0)
#define LISTVIEW_CELL_INFO_BITMAP (1 << 1)

/*********************************************************************
*
*       Object definition
*
**********************************************************************
*/

typedef struct {
  GUI_COLOR                    aBkColor[4];
  GUI_COLOR                    aTextColor[4];
  GUI_COLOR                    GridColor;
  const GUI_FONT             * pFont;
  U16                          ScrollStepH;
  GUI_WRAPMODE                 WrapMode;
  int                          DefaultAlign;
} LISTVIEW_PROPS;

typedef struct {
  void                      (* pfDraw)(LISTVIEW_Handle hObj, unsigned Column, unsigned Row, GUI_RECT * pRect);
  void                       * pData;
  GUI_COLOR                    aBkColor[4];
  GUI_COLOR                    aTextColor[4];
  I16                          xOff, yOff;
  U8                           Flags;
} LISTVIEW_CELL_INFO;

typedef struct {
  WM_HMEM                      hCellInfo;                                         // Optional cell info. If 0, there is no cell info. (Defaults used)
  char                         acText[1];
} LISTVIEW_CELL;

typedef struct {
  GUI_ARRAY                    hCellArray;
  char                         Disabled;
  U32                          UserData;
} LISTVIEW_ROW;

typedef struct {
  U8                           Align;
  int                       (* fpCompare)    (const void * p0, const void * p1);  // User function to be called to compare the contents of 2 cells
} LISTVIEW_COLUMN;

typedef struct LISTVIEW_Obj LISTVIEW_Obj;

typedef struct {
  WM_HMEM                      hSortArray;
  SORT_TYPE                    SortArrayNumItems;
  int                       (* fpSort)(LISTVIEW_Handle hObj);                     // Function to be called to get a sorted array
  void                      (* fpFree)(WM_HMEM hObj);                             // Function to be called to free the sort object
  U8                           Reverse;
} LISTVIEW_SORT;

struct LISTVIEW_Obj {
  WIDGET                       Widget;
  HEADER_Handle                hHeader;
  WIDGET_DRAW_ITEM_FUNC *      pfDrawItem;
  GUI_ARRAY                    hRowArray;                                         // Each entry is a handle of LISTVIEW_ROW structure.
  GUI_ARRAY                    hColumnArray;                                      // Each entry is a handle of LISTVIEW_COLUMN structure.
  LISTVIEW_PROPS               Props;
  WIDGET_COPY                  Copy;
  int                          Sel;
  int                          SelCol;
  int                          ShowGrid;
  int                          SortIndex;                                         // Column for sorting
  int                          MotionPosY;
  unsigned                     RowDistY;
  unsigned                     LBorder;
  unsigned                     RBorder;
  unsigned                     Fixed;
  WM_SCROLL_STATE              ScrollStateV;
  WM_SCROLL_STATE              ScrollStateH;
  WM_HMEM                      hSort;
  WM_HMEM                      hContext;                                          // Motion context.
  unsigned                     OverlapDistance;
  int                          OverlapPeriod;
  U8                           OverlapFlags;
  U8                           IsSorted;
  U8                           IsPresorted;
  U8                           ReverseSort;                                       // Set to 1 if reverse sorting is required
  U8                           Flags;
  U8                           MotionStarted;                                     // Internal flag to check if motion swiping has been started
  U8                           xOff, yOff;                                        // Internal offsets required for AppWizard
};

/*********************************************************************
*
*       Macros for internal use
*
**********************************************************************
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  #define LISTVIEW_INIT_ID(p) (p->Widget.DebugId = WIDGET_TYPE_LISTVIEW)
#else
  #define LISTVIEW_INIT_ID(p)
#endif

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  LISTVIEW_Obj * LISTVIEW_LockH(LISTVIEW_Handle h);
  #define LISTVIEW_LOCK_H(h)   LISTVIEW_LockH(h)
#else
  #define LISTVIEW_LOCK_H(h)   (LISTVIEW_Obj *)WM_LOCK_H(h)
#endif

/*********************************************************************
*
*       Private (module internal) data
*
**********************************************************************
*/
extern LISTVIEW_PROPS LISTVIEW_DefaultProps;

/*********************************************************************
*
*       Private (module internal) functions
*
**********************************************************************
*/
LISTVIEW_CELL_INFO * LISTVIEW__CreateCellInfoLocked (LISTVIEW_Handle   hObj, unsigned Column, unsigned Row);
unsigned             LISTVIEW__GetNumColumns        (LISTVIEW_Obj    * pObj);
unsigned             LISTVIEW__GetNumRows           (LISTVIEW_Obj    * pObj);
unsigned             LISTVIEW__GetNumVisibleRows    (LISTVIEW_Handle hObj, LISTVIEW_Obj * pObj);
LISTVIEW_CELL_INFO * LISTVIEW__GetpCellInfo         (LISTVIEW_Handle   hObj, unsigned Column, unsigned Row);
LISTVIEW_ROW       * LISTVIEW__GetpRow              (LISTVIEW_Handle   hObj, int Row);
unsigned             LISTVIEW__GetRowDistY          (LISTVIEW_Obj    * pObj);
unsigned             LISTVIEW__GetRowSorted         (LISTVIEW_Handle   hObj, int Row);
void                 LISTVIEW__InvalidateInsideArea (LISTVIEW_Handle   hObj);
void                 LISTVIEW__InvalidateRow        (LISTVIEW_Handle   hObj, int Sel);
void                 LISTVIEW__InvalidateRowAndBelow(LISTVIEW_Handle   hObj, int Sel);
void                 LISTVIEW__SetOffset            (LISTVIEW_Handle   hObj, U8 xOff, U8 yOff);
void                 LISTVIEW__SetSel               (LISTVIEW_Handle   hObj, int NewSel, int CheckPos);
void                 LISTVIEW__SetSelCol            (LISTVIEW_Handle   hObj, int NewSelCol);
int                  LISTVIEW__UpdateScrollParas    (LISTVIEW_Handle   hObj);

#endif // GUI_WINSUPPORT
#endif // LISTVIEW_PRIVATE_H

/*************************** End of file ****************************/
