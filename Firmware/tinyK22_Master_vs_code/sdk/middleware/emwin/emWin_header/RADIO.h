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
File        : RADIO.h
Purpose     : RADIO include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef RADIO_H
#define RADIO_H

#include "WM.h"
#include "DIALOG_Type.h"      /* Req. for Create indirect data structure */
#include "WIDGET.h"

#if GUI_WINSUPPORT

#if defined(__cplusplus)
  extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Bitmap indices
*/
#define RADIO_BI_INACTIV  0
#define RADIO_BI_ACTIV    1
#define RADIO_BI_INACTIVE RADIO_BI_INACTIV
#define RADIO_BI_ACTIVE   RADIO_BI_ACTIV
#define RADIO_BI_CHECK    2

/*********************************************************************
*
*       Skinning property indices
*/
#define RADIO_SKINFLEX_PI_PRESSED     0
#define RADIO_SKINFLEX_PI_UNPRESSED   1

/*********************************************************************
*
*       Defaults for public configuration switches
*
**********************************************************************

The following are defaults for config switches which affect the
interface specified in this module
*/

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define RADIO_TEXTPOS_RIGHT       0
#define RADIO_TEXTPOS_LEFT        WIDGET_STATE_USER0  /* Not implemented, TBD */

/*********************************************************************
*
*       Public Types
*
**********************************************************************
*/
typedef WM_HMEM RADIO_Handle;

typedef struct {
  GUI_COLOR aColorButton[4];
  int       ButtonSize;
} RADIO_SKINFLEX_PROPS;

/*********************************************************************
*
*       Create functions
*
**********************************************************************
*/

RADIO_Handle RADIO_Create        (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int Id, int Flags, unsigned Para);
RADIO_Handle RADIO_CreateEx      (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id, int NumItems, int Spacing);
RADIO_Handle RADIO_CreateUser    (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id, int NumItems, int Spacing, int NumExtraBytes);
RADIO_Handle RADIO_CreateIndirect(const GUI_WIDGET_CREATE_INFO * pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK * cb);

/*********************************************************************
*
*       The callback ...
*
* Do not call it directly ! It is only to be used from within an
* overwritten callback.
*/
void RADIO_Callback(WM_MESSAGE * pMsg);

/*********************************************************************
*
*       Managing default values
*
**********************************************************************
*/

void             RADIO_SetDefaultFont      (const GUI_FONT * pFont);
GUI_COLOR        RADIO_SetDefaultFocusColor(GUI_COLOR Color);
void             RADIO_SetDefaultImage     (const GUI_BITMAP * pBitmap, unsigned int Index);
void             RADIO_SetDefaultTextColor (GUI_COLOR TextColor);
const GUI_FONT * RADIO_GetDefaultFont      (void);
GUI_COLOR        RADIO_GetDefaultTextColor (void);

/*********************************************************************
*
*       Member functions
*
**********************************************************************
*/

void               RADIO_AddValue       (RADIO_Handle hObj, int Add);
void               RADIO_Dec            (RADIO_Handle hObj);
GUI_COLOR          RADIO_GetBkColor     (RADIO_Handle hObj);
GUI_COLOR          RADIO_GetFocusColor  (RADIO_Handle hObj);
const GUI_FONT *   RADIO_GetFont        (RADIO_Handle hObj);
const GUI_BITMAP * RADIO_GetImage       (RADIO_Handle hObj, unsigned int Index);
int                RADIO_GetNumItems    (RADIO_Handle hObj);
U16                RADIO_GetSpacing     (RADIO_Handle hObj);
int                RADIO_GetText        (RADIO_Handle hObj, unsigned Index, char * pBuffer, int MaxLen);
GUI_COLOR          RADIO_GetTextColor   (RADIO_Handle hObj);
int                RADIO_GetUserData    (RADIO_Handle hObj, void * pDest, int NumBytes);
void               RADIO_Inc            (RADIO_Handle hObj);
void               RADIO_SetBkColor     (RADIO_Handle hObj, GUI_COLOR Color);
GUI_COLOR          RADIO_SetFocusColor  (RADIO_Handle hObj, GUI_COLOR Color);
void               RADIO_SetFont        (RADIO_Handle hObj, const GUI_FONT * pFont);
void               RADIO_SetGroupId     (RADIO_Handle hObj, U8 GroupId);
void               RADIO_SetImage       (RADIO_Handle hObj, const GUI_BITMAP * pBitmap, unsigned int Index);
void               RADIO_SetReactOnLevel(void);  // Not to be documented
void               RADIO_SetReactOnTouch(void);  // Not to be documented
void               RADIO_SetSpacing     (RADIO_Handle hObj, U16 Spacing);
void               RADIO_SetText        (RADIO_Handle hObj, const char* pText, unsigned Index);
void               RADIO_SetTextColor   (RADIO_Handle hObj, GUI_COLOR Color);
void               RADIO_SetValue       (RADIO_Handle hObj, int v);
int                RADIO_SetUserData    (RADIO_Handle hObj, const void * pSrc, int NumBytes);

/*********************************************************************
*
*       Member functions: Skinning
*
**********************************************************************
*/
void RADIO_GetSkinFlexProps     (RADIO_SKINFLEX_PROPS * pProps, int Index);
void RADIO_SetSkinClassic       (RADIO_Handle hObj);
void RADIO_SetSkin              (RADIO_Handle hObj, WIDGET_DRAW_ITEM_FUNC * pfDrawSkin);
int  RADIO_DrawSkinFlex         (const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo);
void RADIO_SetSkinFlexProps     (const RADIO_SKINFLEX_PROPS * pProps, int Index);
void RADIO_SetDefaultSkinClassic(void);
WIDGET_DRAW_ITEM_FUNC * RADIO_SetDefaultSkin(WIDGET_DRAW_ITEM_FUNC * pfDrawSkin);

#define RADIO_SKIN_FLEX    RADIO_DrawSkinFlex

/*********************************************************************
*
*       Query state
*
**********************************************************************
*/
int RADIO_GetValue(RADIO_Handle hObj);

#if defined(__cplusplus)
  }
#endif

#endif  // GUI_WINSUPPORT
#endif  // RADIO_H

/*************************** End of file ****************************/
