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
File        : MULTIEDIT.h
Purpose     : MULTIEDIT include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef MULTIEDIT_H
#define MULTIEDIT_H

#include "WM.h"
#include "WIDGET.h"
#include "DIALOG_Type.h"      /* Req. for Create indirect data structure */

#if GUI_WINSUPPORT

#if defined(__cplusplus)
  extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       MULTIEDIT create flags
*
*  Description
*    Create flags that define the behavior of the FRAMEWIN widget. These flags are OR-combinable
*    and can be specified upon creation of the widget via the \a{ExFlags} parameter of MULTIEDIT_CreateEx().
*/
#define MULTIEDIT_CF_READONLY           (1 << 0)     // Enables read only mode.
#define MULTIEDIT_CF_INSERT             (1 << 2)     // Enables insert mode.
#define MULTIEDIT_CF_AUTOSCROLLBAR_V    (1 << 3)     // Automatic use of a vertical scroll bar.
#define MULTIEDIT_CF_AUTOSCROLLBAR_H    (1 << 4)     // Automatic use of a horizontal scroll bar.
#define MULTIEDIT_CF_PASSWORD           (1 << 5)     // Enables password mode.
#define MULTIEDIT_CF_SHOWCURSOR         (1 << 6)     // Shows the cursor.
#define MULTIEDIT_CF_MOTION_H           (1 << 7)     // Enables motion support on X-axis.
#define MULTIEDIT_CF_MOTION_V           (1 << 8)     // Enables motion support on Y-axis.
#define MULTIEDIT_CF_USE_COPY           (1 << 9)     // Enables copyrect optimization
/* status flags */
#define MULTIEDIT_SF_READONLY           MULTIEDIT_CF_READONLY
#define MULTIEDIT_SF_INSERT             MULTIEDIT_CF_INSERT
#define MULTIEDIT_SF_AUTOSCROLLBAR_V    MULTIEDIT_CF_AUTOSCROLLBAR_V
#define MULTIEDIT_SF_AUTOSCROLLBAR_H    MULTIEDIT_CF_AUTOSCROLLBAR_H
#define MULTIEDIT_SF_PASSWORD           MULTIEDIT_CF_PASSWORD
#define MULTIEDIT_SF_MOTION_H           MULTIEDIT_CF_MOTION_H
#define MULTIEDIT_SF_MOTION_V           MULTIEDIT_CF_MOTION_V

/*********************************************************************
*
*       MULTIEDIT color indexes
*
*  Description
*    Color indexes used by the MULTIEDIT widget.
*/
#define MULTIEDIT_CI_EDIT               0                   // Color in edit mode.
#define MULTIEDIT_CI_READONLY           1                   // Color in read-only mode.

/*********************************************************************
*
*       MULTIEDIT cursor color indexes
*
*  Description
*    Color indexes used for the cursor of the MULTIEDIT widget.
*/
#define MULTIEDIT_CI_CURSOR_BK          0                   // Background color for cursor
#define MULTIEDIT_CI_CURSOR_FG          1                   // Foreground color for cursor

#define GUI_MULTIEDIT_SHOWCURSOR        (1 << 10)
#define GUI_MULTIEDIT_CURSORBLINK       (1 << 11)

/*********************************************************************
*
*                         Public Types
*
**********************************************************************
*/

typedef WM_HMEM MULTIEDIT_HANDLE;

/*********************************************************************
*
*                 Create functions
*
**********************************************************************
*/
MULTIEDIT_HANDLE MULTIEDIT_Create        (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int Id, int Flags, int ExFlags, const char * pText, int MaxLen);
MULTIEDIT_HANDLE MULTIEDIT_CreateEx      (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id, int BufferSize, const char * pText);
MULTIEDIT_HANDLE MULTIEDIT_CreateIndirect(const GUI_WIDGET_CREATE_INFO* pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK * cb);
MULTIEDIT_HANDLE MULTIEDIT_CreateUser    (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id, int BufferSize, const char * pText, int NumExtraBytes);

/*********************************************************************
*
*       The callback ...
*
* Do not call it directly ! It is only to be used from within an
* overwritten callback.
*/
void MULTIEDIT_Callback(WM_MESSAGE * pMsg);

/*********************************************************************
*
*                 Member functions
*
**********************************************************************
*/

int              MULTIEDIT_AddKey               (MULTIEDIT_HANDLE hObj, U16 Key);
int              MULTIEDIT_AddText              (MULTIEDIT_HANDLE hObj, const char * s);
void             MULTIEDIT_EnableBlink          (MULTIEDIT_HANDLE hObj, int Period, int OnOff);
void             MULTIEDIT_EnableCopy           (MULTIEDIT_HANDLE hObj, unsigned OnOff);
void             MULTIEDIT_EnableMotion         (MULTIEDIT_HANDLE hObj, int Flags);
GUI_COLOR        MULTIEDIT_GetBkColor           (MULTIEDIT_HANDLE hObj, unsigned Index);
int              MULTIEDIT_GetCursorCharPos     (MULTIEDIT_HANDLE hObj);
void             MULTIEDIT_GetCursorPixelPos    (MULTIEDIT_HANDLE hObj, int * pxPos, int * pyPos);
const GUI_FONT * MULTIEDIT_GetFont              (MULTIEDIT_HANDLE hObj);
int              MULTIEDIT_GetNumChars          (MULTIEDIT_HANDLE hObj);
void             MULTIEDIT_GetPrompt            (MULTIEDIT_HANDLE hObj, char* sDest, int MaxNumChars);
void             MULTIEDIT_GetText              (MULTIEDIT_HANDLE hObj, char* sDest, int MaxNumChars);
GUI_COLOR        MULTIEDIT_GetTextColor         (MULTIEDIT_HANDLE hObj, unsigned Index);
int              MULTIEDIT_GetTextFromLine      (MULTIEDIT_HANDLE hObj, char * sDest, int MaxLen, unsigned CharPos, unsigned Line);
int              MULTIEDIT_GetTextFromPos       (MULTIEDIT_HANDLE hObj, char * sDest, int MaxLen, int CharStart, int LineStart, int CharEnd, int LineEnd);
int              MULTIEDIT_GetTextSize          (MULTIEDIT_HANDLE hObj);
int              MULTIEDIT_GetUserData          (MULTIEDIT_HANDLE hObj, void * pDest, int NumBytes);
int              MULTIEDIT_ReadBytesFromPos     (MULTIEDIT_HANDLE hObj, char * sDest, U32 SizeOfBuffer, U32 Off);
int              MULTIEDIT_ReadCharsFromPos     (MULTIEDIT_HANDLE hObj, char * sDest, U32 SizeOfBuffer, U32 Offset);
void             MULTIEDIT_SetTextAlign         (MULTIEDIT_HANDLE hObj, int Align);
void             MULTIEDIT_SetAutoScrollH       (MULTIEDIT_HANDLE hObj, int OnOff);
void             MULTIEDIT_SetAutoScrollV       (MULTIEDIT_HANDLE hObj, int OnOff);
void             MULTIEDIT_SetBkColor           (MULTIEDIT_HANDLE hObj, unsigned Index, GUI_COLOR color);
void             MULTIEDIT_SetCursorColor       (MULTIEDIT_HANDLE hObj, unsigned Index, GUI_COLOR color);
void             MULTIEDIT_SetCursorCharPos     (MULTIEDIT_HANDLE hObj, int x, int y);       /* Not yet implemented */
void             MULTIEDIT_SetCursorPixelPos    (MULTIEDIT_HANDLE hObj, int x, int y);       /* Not yet implemented */
void             MULTIEDIT_SetCursorOffset      (MULTIEDIT_HANDLE hObj, int Offset);
void             MULTIEDIT_SetHBorder           (MULTIEDIT_HANDLE hObj, unsigned HBorder);
void             MULTIEDIT_SetFocusable         (MULTIEDIT_HANDLE hObj, int State);
void             MULTIEDIT_SetFont              (MULTIEDIT_HANDLE hObj, const GUI_FONT * pFont);
void             MULTIEDIT_SetInsertMode        (MULTIEDIT_HANDLE hObj, int OnOff);
void             MULTIEDIT_SetInvertCursor      (MULTIEDIT_HANDLE hObj, int OnOff);
void             MULTIEDIT_SetBufferSize        (MULTIEDIT_HANDLE hObj, int BufferSize);
void             MULTIEDIT_SetMaxNumChars       (MULTIEDIT_HANDLE hObj, unsigned MaxNumChars);
void             MULTIEDIT_SetPrompt            (MULTIEDIT_HANDLE hObj, const char* sPrompt);
void             MULTIEDIT_SetReadOnly          (MULTIEDIT_HANDLE hObj, int OnOff);
void             MULTIEDIT_SetPasswordMode      (MULTIEDIT_HANDLE hObj, int OnOff);
void             MULTIEDIT_SetText              (MULTIEDIT_HANDLE hObj, const char* s);
void             MULTIEDIT_SetTextColor         (MULTIEDIT_HANDLE hObj, unsigned Index, GUI_COLOR color);
int              MULTIEDIT_SetUserData          (MULTIEDIT_HANDLE hObj, const void * pSrc, int NumBytes);
void             MULTIEDIT_SetWrapNone          (MULTIEDIT_HANDLE hObj);
void             MULTIEDIT_SetWrapChar          (MULTIEDIT_HANDLE hObj);
void             MULTIEDIT_SetWrapWord          (MULTIEDIT_HANDLE hObj);
int              MULTIEDIT_ShowCursor           (MULTIEDIT_HANDLE hObj, unsigned OnOff);
//
// Default getters/setters
//
int              MULTIEDIT_GetDefaultAlign      (void);
GUI_COLOR        MULTIEDIT_GetDefaultBkColor    (unsigned Index);
GUI_COLOR        MULTIEDIT_GetDefaultCursorColor(unsigned Index);
const GUI_FONT * MULTIEDIT_GetDefaultFont       (void);
unsigned         MULTIEDIT_GetDefaultHBorder    (void);
GUI_COLOR        MULTIEDIT_GetDefaultTextColor  (unsigned Index);
void             MULTIEDIT_SetDefaultAlign      (int Align);
void             MULTIEDIT_SetDefaultBkColor    (GUI_COLOR Color, unsigned Index);
void             MULTIEDIT_SetDefaultCursorColor(GUI_COLOR Color, unsigned Index);
void             MULTIEDIT_SetDefaultFont       (const GUI_FONT * pFont);
void             MULTIEDIT_SetDefaultHBorder    (unsigned HBorder);
void             MULTIEDIT_SetDefaultTextColor  (GUI_COLOR Color, unsigned Index);

/*********************************************************************
*
*       Macros for compatibility with older versions
*
**********************************************************************
*/

#define MULTIEDIT_SetMaxLen(hObj, MaxLen) MULTIEDIT_SetBufferSize(hObj, MaxLen)
#define MULTIEDIT_GetStringSize           MULTIEDIT_GetTextSize
#define MULTIEDIT_SetFocussable           MULTIEDIT_SetFocusable

#if defined(__cplusplus)
  }
#endif

#endif  // GUI_WINSUPPORT
#endif  // MULTIEDIT_H

/*************************** End of file ****************************/
