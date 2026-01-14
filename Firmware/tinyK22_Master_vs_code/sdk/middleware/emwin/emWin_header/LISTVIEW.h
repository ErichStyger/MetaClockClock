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
File        : LISTVIEW.h
Purpose     : LISTVIEW include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef LISTVIEW_H
#define LISTVIEW_H

#include "WM.h"
#include "DIALOG_Type.h"      // Req. for Create indirect data structure
#include "ICONVIEW.h"
#include "HEADER.h"

#if GUI_WINSUPPORT

#if defined(__cplusplus)
  extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines
*/
#define LISTVIEW_ALL_ITEMS  -1

/*********************************************************************
*
*       LISTVIEW color indexes
*
*  Description
*    Color indexes to be used by the LISTVIEW widget.
*/
#define LISTVIEW_CI_UNSEL       0      // Unselected element.
#define LISTVIEW_CI_SEL         1      // Selected element, without focus.
#define LISTVIEW_CI_SELFOCUS    2      // Selected element, with focus.
#define LISTVIEW_CI_DISABLED    3      // Disabled element.

/************************************************************
*
*       Create / Status flags
*/
#define LISTVIEW_CF_AUTOSCROLLBAR_H   (1 << 0)
#define LISTVIEW_CF_AUTOSCROLLBAR_V   (1 << 1)
#define LISTVIEW_CF_CELL_SELECT       (1 << 2)                     // Create Flag used to enable cell selection
#define LISTVIEW_CF_MOTION_H          (1 << 3)
#define LISTVIEW_CF_MOTION_V          (1 << 4)
#define LISTVIEW_CF_USE_COPY          (1 << 5)
#define LISTVIEW_SF_AUTOSCROLLBAR_H   LISTVIEW_CF_AUTOSCROLLBAR_H
#define LISTVIEW_SF_AUTOSCROLLBAR_V   LISTVIEW_CF_AUTOSCROLLBAR_V

/*********************************************************************
*
*       Public Types
*
**********************************************************************
*/
typedef WM_HMEM LISTVIEW_Handle;

/*********************************************************************
*
*       Create functions
*
**********************************************************************
*/
LISTVIEW_Handle LISTVIEW_Create        (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int Id, int Flags, int SpecialFlags);
LISTVIEW_Handle LISTVIEW_CreateEx      (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id);
LISTVIEW_Handle LISTVIEW_CreateUser    (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id, int NumExtraBytes);
LISTVIEW_Handle LISTVIEW_CreateAttached(WM_HWIN hParent, int Id, int SpecialFlags);
LISTVIEW_Handle LISTVIEW_CreateIndirect(const GUI_WIDGET_CREATE_INFO * pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK * cb);

/*********************************************************************
*
*       The callback ...
*
* Do not call it directly ! It is only to be used from within an
* overwritten callback.
*/
void LISTVIEW_Callback(WM_MESSAGE * pMsg);

/*********************************************************************
*
*       Member functions
*
**********************************************************************
*/
int              LISTVIEW_AddColumn            (LISTVIEW_Handle hObj, int Width, const char * s, int Align);
int              LISTVIEW_AddRow               (LISTVIEW_Handle hObj, const GUI_ConstString * ppText);
void             LISTVIEW_Clear                (LISTVIEW_Handle hObj);
int              LISTVIEW_CompareText          (const void * p0, const void * p1);
int              LISTVIEW_CompareDec           (const void * p0, const void * p1);
void             LISTVIEW_DecSel               (LISTVIEW_Handle hObj);
void             LISTVIEW_DeleteAllRows        (LISTVIEW_Handle hObj);
void             LISTVIEW_DeleteColumn         (LISTVIEW_Handle hObj, unsigned Index);
void             LISTVIEW_DeleteRow            (LISTVIEW_Handle hObj, unsigned Index);
void             LISTVIEW_DeleteRowSorted      (LISTVIEW_Handle hObj, int Row);
void             LISTVIEW_DisableRow           (LISTVIEW_Handle hObj, unsigned Row);
void             LISTVIEW_DisableSort          (LISTVIEW_Handle hObj);
void             LISTVIEW_EnableCellSelect     (LISTVIEW_Handle hObj, unsigned OnOff);  // Enables/disables cell selection
void             LISTVIEW_EnableCopy           (LISTVIEW_Handle hObj, unsigned OnOff);
void             LISTVIEW_EnableMotion         (LISTVIEW_Handle hObj, int Flags);
void             LISTVIEW_EnableRow            (LISTVIEW_Handle hObj, unsigned Row);
void             LISTVIEW_EnableSort           (LISTVIEW_Handle hObj);
GUI_COLOR        LISTVIEW_GetBkColor           (LISTVIEW_Handle hObj, unsigned Index);
int              LISTVIEW_GetCol               (LISTVIEW_Handle hObj, int xPos);
const GUI_FONT * LISTVIEW_GetFont              (LISTVIEW_Handle hObj);
GUI_COLOR        LISTVIEW_GetGridColor         (LISTVIEW_Handle hObj);
HEADER_Handle    LISTVIEW_GetHeader            (LISTVIEW_Handle hObj);
void             LISTVIEW_GetItemRect          (LISTVIEW_Handle hObj, U32 Col, U32 Row, GUI_RECT * pRect);
void             LISTVIEW_GetItemText          (LISTVIEW_Handle hObj, unsigned Column, unsigned Row, char * pBuffer, unsigned MaxSize);
unsigned         LISTVIEW_GetItemTextLen       (LISTVIEW_Handle hObj, unsigned Column, unsigned Row);
void             LISTVIEW_GetItemTextSorted    (LISTVIEW_Handle hObj, unsigned Column, unsigned Row, char * pBuffer, unsigned MaxSize);
unsigned         LISTVIEW_GetLBorder           (LISTVIEW_Handle hObj);
unsigned         LISTVIEW_GetNumColumns        (LISTVIEW_Handle hObj);
unsigned         LISTVIEW_GetNumRows           (LISTVIEW_Handle hObj);
unsigned         LISTVIEW_GetOverlap           (LISTVIEW_Handle hObj, int * pPeriod, U8 * pFlags);
unsigned         LISTVIEW_GetRBorder           (LISTVIEW_Handle hObj);
int              LISTVIEW_GetRow               (LISTVIEW_Handle hObj, int yPos);
int              LISTVIEW_GetScrollStepH       (LISTVIEW_Handle hObj);
int              LISTVIEW_GetScrollPos         (LISTVIEW_Handle hObj, int Coord);
int              LISTVIEW_GetSel               (LISTVIEW_Handle hObj);
int              LISTVIEW_GetSelCol            (LISTVIEW_Handle hObj);
int              LISTVIEW_GetSelUnsorted       (LISTVIEW_Handle hObj);
int              LISTVIEW_GetTextAlign         (LISTVIEW_Handle hObj, unsigned ColIndex);
GUI_COLOR        LISTVIEW_GetTextColor         (LISTVIEW_Handle hObj, unsigned Index);
int              LISTVIEW_GetUserData          (LISTVIEW_Handle hObj, void * pDest, int NumBytes);
U32              LISTVIEW_GetUserDataRow       (LISTVIEW_Handle hObj, unsigned Row);
int              LISTVIEW_GetVisRowIndices     (LISTVIEW_Handle hObj, int * pFirst, int * pLast);
GUI_WRAPMODE     LISTVIEW_GetWrapMode          (LISTVIEW_Handle hObj);
void             LISTVIEW_IncSel               (LISTVIEW_Handle hObj);
int              LISTVIEW_InsertRow            (LISTVIEW_Handle hObj, unsigned Index, const GUI_ConstString * ppText);
int              LISTVIEW_IsRowPartiallyVisible(LISTVIEW_Handle hObj, unsigned Index);
int              LISTVIEW_OwnerDraw            (const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo);
unsigned         LISTVIEW_RowIsDisabled        (LISTVIEW_Handle hObj, unsigned Row);
void             LISTVIEW_SetAutoScrollH       (LISTVIEW_Handle hObj, int OnOff);
void             LISTVIEW_SetAutoScrollV       (LISTVIEW_Handle hObj, int OnOff);
void             LISTVIEW_SetItemBitmap        (LISTVIEW_Handle hObj, unsigned Column, unsigned Row, int xOff, int yOff, const GUI_BITMAP * pBitmap);
void             LISTVIEW_SetBkColor           (LISTVIEW_Handle hObj, unsigned int Index, GUI_COLOR Color);
void             LISTVIEW_SetColumnWidth       (LISTVIEW_Handle hObj, unsigned int Index, int Width);
void             LISTVIEW_SetCompareFunc       (LISTVIEW_Handle hObj, unsigned Column, int (* fpCompare)(const void * p0, const void * p1));
unsigned         LISTVIEW_SetFixed             (LISTVIEW_Handle hObj, unsigned Fixed);
void             LISTVIEW_SetFont              (LISTVIEW_Handle hObj, const GUI_FONT * pFont);
void             LISTVIEW_SetGridColor         (LISTVIEW_Handle hObj, GUI_COLOR Color);
int              LISTVIEW_SetGridVis           (LISTVIEW_Handle hObj, int Show);
void             LISTVIEW_SetHeaderHeight      (LISTVIEW_Handle hObj, unsigned HeaderHeight);
void             LISTVIEW_SetItemBkColor       (LISTVIEW_Handle hObj, unsigned Column, unsigned Row, unsigned Index, GUI_COLOR Color);
void             LISTVIEW_SetItemText          (LISTVIEW_Handle hObj, unsigned Column, unsigned Row, const char * s);
void             LISTVIEW_SetItemTextColor     (LISTVIEW_Handle hObj, unsigned Column, unsigned Row, unsigned Index, GUI_COLOR Color);
void             LISTVIEW_SetItemTextSorted    (LISTVIEW_Handle hObj, unsigned Column, unsigned Row, const char * pText);
void             LISTVIEW_SetLBorder           (LISTVIEW_Handle hObj, unsigned BorderSize);
void             LISTVIEW_SetOverlap           (LISTVIEW_Handle hObj, unsigned Overlap, int Period, U8 Flags);
void             LISTVIEW_SetOwnerDraw         (LISTVIEW_Handle hObj, WIDGET_DRAW_ITEM_FUNC * pfDrawItem);
void             LISTVIEW_SetRBorder           (LISTVIEW_Handle hObj, unsigned BorderSize);
unsigned         LISTVIEW_SetRowHeight         (LISTVIEW_Handle hObj, unsigned RowHeight);
void             LISTVIEW_SetScrollPos         (LISTVIEW_Handle hObj, unsigned Col, unsigned Row);
int              LISTVIEW_SetScrollStepH       (LISTVIEW_Handle hObj, int Dist);
void             LISTVIEW_SetSel               (LISTVIEW_Handle hObj, int Sel);
void             LISTVIEW_SetSelCol            (LISTVIEW_Handle hObj, int NewCol);
void             LISTVIEW_SetSelUnsorted       (LISTVIEW_Handle hObj, int Sel);
unsigned         LISTVIEW_SetSort              (LISTVIEW_Handle hObj, unsigned Column, unsigned Reverse);
void             LISTVIEW_SetTextAlign         (LISTVIEW_Handle hObj, unsigned int Index, int Align);
void             LISTVIEW_SetTextColor         (LISTVIEW_Handle hObj, unsigned int Index, GUI_COLOR Color);
int              LISTVIEW_SetUserData          (LISTVIEW_Handle hObj, const void * pSrc, int NumBytes);
void             LISTVIEW_SetUserDataRow       (LISTVIEW_Handle hObj, unsigned Row, U32 UserData);
void             LISTVIEW_SetWrapMode          (LISTVIEW_Handle hObj, GUI_WRAPMODE WrapMode);

/*********************************************************************
*
*       Managing default values
*
**********************************************************************
*/
GUI_COLOR        LISTVIEW_GetDefaultBkColor    (unsigned  Index);
const GUI_FONT * LISTVIEW_GetDefaultFont       (void);
GUI_COLOR        LISTVIEW_GetDefaultGridColor  (void);
int              LISTVIEW_GetDefaultScrollStepH(void);
GUI_COLOR        LISTVIEW_GetDefaultTextColor  (unsigned  Index);
GUI_COLOR        LISTVIEW_SetDefaultBkColor    (unsigned  Index, GUI_COLOR Color);
const GUI_FONT * LISTVIEW_SetDefaultFont       (const GUI_FONT * pFont);
GUI_COLOR        LISTVIEW_SetDefaultGridColor  (GUI_COLOR Color);
int              LISTVIEW_SetDefaultScrollStepH(int Dist);
GUI_COLOR        LISTVIEW_SetDefaultTextColor  (unsigned  Index, GUI_COLOR Color);

#if defined(__cplusplus)
  }
#endif

#endif  // GUI_WINSUPPORT
#endif  // LISTVIEW_H

/*************************** End of file ****************************/
