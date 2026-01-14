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
File        : TEXT.h
Purpose     : TEXT include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef TEXT_H
#define TEXT_H

#include "WM.h"
#include "DIALOG_Type.h"      /* Req. for Create indirect data structure */
#include "WIDGET.h"

#if GUI_WINSUPPORT

#if defined(__cplusplus)
  extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/************************************************************
*
*       #defines
*
*************************************************************
*/

/************************************************************
*
*       Create / Status flags
*/
#define TEXT_CF_LEFT    GUI_TA_LEFT
#define TEXT_CF_RIGHT   GUI_TA_RIGHT
#define TEXT_CF_HCENTER GUI_TA_HCENTER

#define TEXT_CF_VCENTER GUI_TA_VCENTER
#define TEXT_CF_TOP     GUI_TA_TOP
#define TEXT_CF_BOTTOM  GUI_TA_BOTTOM


/*********************************************************************
*
*       Public Types
*
**********************************************************************

*/
typedef WM_HMEM TEXT_Handle;

/*********************************************************************
*
*       Create functions
*
**********************************************************************
*/
TEXT_Handle TEXT_Create        (int x0, int y0, int xSize, int ySize, int Id, int Flags, const char * s, int Align);
TEXT_Handle TEXT_CreateAsChild (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int Id, int Flags, const char * s, int Align);
TEXT_Handle TEXT_CreateEx      (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id, const char * pText);
TEXT_Handle TEXT_CreateUser    (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id, const char * pText, int NumExtraBytes);
TEXT_Handle TEXT_CreateIndirect(const GUI_WIDGET_CREATE_INFO * pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK * cb);

/*********************************************************************
*
*       The callback ...
*
* Do not call it directly ! It is only to be used from within an
* overwritten callback.
*/
void TEXT_Callback(WM_MESSAGE * pMsg);

/*********************************************************************
*
*       Member functions
*
**********************************************************************
*/

/* Methods changing properties */

GUI_COLOR            TEXT_GetBkColor   (TEXT_Handle hObj); 
const GUI_FONT *     TEXT_GetFont      (TEXT_Handle hObj);
int                  TEXT_GetNumLines  (TEXT_Handle hObj);
#if GUI_SUPPORT_ROTATION
const GUI_ROTATION * TEXT_GetRotation  (TEXT_Handle hObj);
#endif
int                  TEXT_GetText      (TEXT_Handle hObj, char * pDest, U32 BufferSize);
int                  TEXT_GetTextAlign (TEXT_Handle hObj);
GUI_COLOR            TEXT_GetTextColor (TEXT_Handle hObj);
GUI_COLOR            TEXT_GetFrameColor(TEXT_Handle hObj);
void                 TEXT_GetTextOffset(TEXT_Handle hObj, int * pxPos, int * pyPos);
int                  TEXT_GetUserData  (TEXT_Handle hObj, void * pDest, int NumBytes);
GUI_WRAPMODE         TEXT_GetWrapMode  (TEXT_Handle hObj);
void                 TEXT_SetBkColor   (TEXT_Handle hObj, GUI_COLOR Color);
void                 TEXT_SetFont      (TEXT_Handle hObj, const GUI_FONT * pFont);
int                  TEXT_SetDec       (TEXT_Handle hObj, I32 v, U8 Len, U8 Shift, U8 Signed, U8 Space);
int                  TEXT_SetHex       (TEXT_Handle hObj, I32 v, U8 Len);
#if GUI_SUPPORT_ROTATION
void                 TEXT_SetRotation  (TEXT_Handle hObj, const GUI_ROTATION * pLCD_Api);
#endif
int                  TEXT_SetText      (TEXT_Handle hObj, const char * s);
void                 TEXT_SetTextAlign (TEXT_Handle hObj, int Align);
void                 TEXT_SetTextColor (TEXT_Handle hObj, GUI_COLOR Color);
void                 TEXT_SetFrameColor(TEXT_Handle hObj, GUI_COLOR Color);
void                 TEXT_SetTextOffset(TEXT_Handle hObj, int xPos, int yPos);
int                  TEXT_SetUserData  (TEXT_Handle hObj, const void * pSrc, int NumBytes);
void                 TEXT_SetWrapMode  (TEXT_Handle hObj, GUI_WRAPMODE WrapMode);

int                  TEXT__SetDec      (TEXT_Handle hObj, I32 v, U8 Len, U8 Shift, U8 Signed, U8 Space, I32 Min, I32 Max);

/*********************************************************************
*
*       Managing default values
*
**********************************************************************
*/

const GUI_FONT *     TEXT_GetDefaultFont      (void);
GUI_COLOR            TEXT_GetDefaultFrameColor(void);
#if GUI_SUPPORT_ROTATION
const GUI_ROTATION * TEXT_GetDefaultRotation  (void);
#endif
GUI_COLOR            TEXT_GetDefaultTextColor (void);
GUI_WRAPMODE         TEXT_GetDefaultWrapMode  (void);
void                 TEXT_SetDefaultFrameColor(GUI_COLOR Color);
void                 TEXT_SetDefaultFont      (const GUI_FONT * pFont);
#if GUI_SUPPORT_ROTATION
const GUI_ROTATION * TEXT_SetDefaultRotation  (const GUI_ROTATION * pLCD_Api);
#endif
void                 TEXT_SetDefaultTextColor (GUI_COLOR Color);
GUI_WRAPMODE         TEXT_SetDefaultWrapMode  (GUI_WRAPMODE WrapMode);

#if defined(__cplusplus)
  }
#endif

#endif  // GUI_WINSUPPORT
#endif  // TEXT_H

/*************************** End of file ****************************/
