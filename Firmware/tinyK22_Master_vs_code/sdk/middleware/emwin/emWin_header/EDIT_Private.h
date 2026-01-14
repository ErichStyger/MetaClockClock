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
File        : EDIT_Private.h
Purpose     : Internal header file
---------------------------END-OF-HEADER------------------------------
*/

#ifndef EDIT_PRIVATE_H
#define EDIT_PRIVATE_H

#include "EDIT.h"

#if GUI_WINSUPPORT

#include "WM_Intern.h"
#include "WIDGET.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define EDIT_REALLOC_SIZE  16

#ifndef EDIT_XOFF
  #define EDIT_XOFF        1
#endif

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct EDIT_Obj_struct EDIT_Obj;

typedef struct {
  int              Align;
  int              Border;
  const GUI_FONT * pFont;
  GUI_COLOR        aTextColor[3];
  GUI_COLOR        aBkColor[3];
} EDIT_PROPS;

struct EDIT_Obj_struct {
  WIDGET               Widget;
  WM_HMEM              hpText;
  I16                  MaxLen;
  U16                  BufferSize;
  I32                  Min, Max;        // Min max values as normalized floats (integers)
  U8                   NumDecs;         // Number of decimals
  I32                  CurrentValue;    // Current value
  int                  CursorPos;       // Cursor position. 0 means left most
  unsigned             SelSize;         // Number of selected characters
  U8                   EditMode;        // Insert or overwrite mode
  U8                   XSizeCursor;     // Size of cursor when working in insert mode
  U8                   Flags;
  U8                   Radius;          // Currently only used by AppWizard
  tEDIT_AddKeyEx     * pfAddKeyEx;      // Handle key input
  tEDIT_UpdateBuffer * pfUpdateBuffer;  // Update textbuffer
  EDIT_PROPS           Props;
  WM_HTIMER            hTimer;
  U8                   MinMaxMode;
  int                  TimerPeriod;
  int                  ScrollPos;       // Horizontal scrolling position
  U16                  PrevStrLen;      // Previous string length, used for scrolling position calculation.
  char               * pDispText;       // Pointer to buffer with the password char.
};

/*********************************************************************
*
*       Macros for internal use
*
**********************************************************************
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  #define EDIT_INIT_ID(p) (p->Widget.DebugId = WIDGET_TYPE_EDIT)
#else
  #define EDIT_INIT_ID(p)
#endif

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  EDIT_Obj * EDIT_LockH(EDIT_Handle h);
  #define EDIT_LOCK_H(h)   EDIT_LockH(h)
#else
  #define EDIT_LOCK_H(h)   (EDIT_Obj *)WM_LOCK_H(h)
#endif

/*********************************************************************
*
*       Public data (internal defaults)
*
**********************************************************************
*/
extern EDIT_PROPS EDIT__DefaultProps;

/*********************************************************************
*
*       Public functions (internal)
*
**********************************************************************
*/
U16  EDIT__GetCurrentChar  (EDIT_Obj * pObj);
void EDIT__SetCursorPos    (EDIT_Handle hObj, int CursorPos);
void EDIT__SetCursorPosEx  (EDIT_Handle hObj, int CursorPos, U8 Delete);
void EDIT__SetValueUnsigned(EDIT_Handle hObj, I32 Value);

#endif // GUI_WINSUPPORT

#endif // EDIT_PRIVATE_H

/*************************** End of file ****************************/
