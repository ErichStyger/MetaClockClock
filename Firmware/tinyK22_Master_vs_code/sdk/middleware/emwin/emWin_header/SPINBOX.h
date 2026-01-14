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
File        : SPINBOX.h
Purpose     : SPINBOX header file
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef SPINBOX_H
#define SPINBOX_H

#include "WM.h"
#include "DIALOG_Type.h" // Required for Create indirect data structure
#include "WIDGET.h"
#include "EDIT.h"

#if GUI_WINSUPPORT

#if defined(__cplusplus)
  extern "C" {             // Make sure we have C-declarations in C++ programs
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
/*********************************************************************
*
*       States
*/
#define SPINBOX_STATE_PRESSED(x)     (U8)(1 << (U8)x)    // These flags are stored in (SPINBOX_OBJ->State)        | x must be 0 or 1
#define SPINBOX_STATE_FOCUS          WIDGET_STATE_FOCUS  // This is read from         (SPINBOX_OBJ->Widget.State)

#define SPINBOX_EDGE_RIGHT           0
#define SPINBOX_EDGE_LEFT            1
#define SPINBOX_EDGE_CENTER          2

#define SPINBOX_EM_STEP              0
#define SPINBOX_EM_EDIT              1

#ifndef SPINBOX_EM_DEFAULT
  #define SPINBOX_EM_DEFAULT         SPINBOX_EM_STEP
#endif

/*********************************************************************
*
*       Color indices
*/
#define SPINBOX_CI_DISABLED          EDIT_CI_DISABLED
#define SPINBOX_CI_ENABLED           EDIT_CI_ENABLED
#define SPINBOX_CI_PRESSED           2

/*********************************************************************
*
*       SPINBOX timer indexes
*
*  Description
*    Timer indexes used by the routine SPINBOX_SetTimerPeriod().
*/
#define SPINBOX_TI_TIMERSTART       0      // Time it takes to start auto increase/decrease of its value.
#define SPINBOX_TI_TIMERINC         1      // Time between two increments/decrements.

/*********************************************************************
*
*       Skinning property indices
*/
#define SPINBOX_SKIN_FLEX            SPINBOX_DrawSkinFlex

#define SPINBOX_SKINFLEX_PI_PRESSED  0
#define SPINBOX_SKINFLEX_PI_FOCUSED  1
#define SPINBOX_SKINFLEX_PI_ENABLED  2
#define SPINBOX_SKINFLEX_PI_DISABLED 3
#define SPINBOX_SKIN_FLEX_RADIUS     2

/*********************************************************************
*
*       Public Types
*
**********************************************************************
*/
typedef WM_HMEM SPINBOX_Handle;

typedef struct {
  GUI_COLOR aColorFrame[2];   // [0] Outer color of surrounding frame.         [1] Inner color of surrounding frame.
  GUI_COLOR aColorUpper[2];   // [0] Upper color of gradient for upper button. [1] Lower color of gradient for upper button.
  GUI_COLOR aColorLower[2];   // [0] Upper color of gradient for lower button. [1] Lower color of gradient for lower button.
  GUI_COLOR ColorArrow;       // Color of the button arrow.
  GUI_COLOR ColorBk;          // Color of the background.                      // See WIDGET_ITEM_CREATE in SPINBOX_DrawSkinFlex()
  GUI_COLOR ColorText;        // Color of the text.                            // See WIDGET_ITEM_CREATE in SPINBOX_DrawSkinFlex()
  GUI_COLOR ColorButtonFrame; // Color of the button frame.
} SPINBOX_SKINFLEX_PROPS;

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/
/*********************************************************************
*
*       Creation
*/
SPINBOX_Handle SPINBOX_CreateEx      (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int Id, int Min, int Max);
SPINBOX_Handle SPINBOX_CreateUser    (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int Id, int Min, int Max, int NumExtraBytes);
SPINBOX_Handle SPINBOX_CreateIndirect(const GUI_WIDGET_CREATE_INFO * pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK * cb);

/*********************************************************************
*
*       Callback, should be called only from within a custom callback.
*/
void           SPINBOX_Callback(WM_MESSAGE * pMsg);

/*********************************************************************
*
*       Get / Set properties
*/
void             SPINBOX_EnableBlink     (SPINBOX_Handle hObj, int Period, int OnOff);
GUI_COLOR        SPINBOX_GetBkColor      (SPINBOX_Handle hObj, unsigned int Index);
GUI_COLOR        SPINBOX_GetButtonBkColor(SPINBOX_Handle hObj, unsigned int Index);
EDIT_Handle      SPINBOX_GetEditHandle   (SPINBOX_Handle hObj);
const GUI_FONT * SPINBOX_GetFont         (SPINBOX_Handle hObj);
void             SPINBOX_GetRange        (SPINBOX_Handle hObj, I32 * pMin, I32 * pMax);
GUI_COLOR        SPINBOX_GetTextColor    (SPINBOX_Handle hObj, unsigned int Index);
U32              SPINBOX_GetTimerPeriod  (SPINBOX_Handle hObj, U32 Index);
int              SPINBOX_GetUserData     (SPINBOX_Handle hObj, void * pDest, int NumBytes);
I32              SPINBOX_GetValue        (SPINBOX_Handle hObj);
void             SPINBOX_SetBkColor      (SPINBOX_Handle hObj, unsigned int Index, GUI_COLOR Color);
void             SPINBOX_SetButtonBkColor(SPINBOX_Handle hObj, unsigned int Index, GUI_COLOR Color);
void             SPINBOX_SetButtonSize   (SPINBOX_Handle hObj, unsigned ButtonSize);
void             SPINBOX_SetEdge         (SPINBOX_Handle hObj, U8 Edge);
void             SPINBOX_SetEditMode     (SPINBOX_Handle hObj, U8 EditMode);
void             SPINBOX_SetFont         (SPINBOX_Handle hObj, const GUI_FONT * pFont);
void             SPINBOX_SetRange        (SPINBOX_Handle hObj, I32 Min, I32 Max);
U16              SPINBOX_SetStep         (SPINBOX_Handle hObj, U16 Step);
void             SPINBOX_SetTextColor    (SPINBOX_Handle hObj, unsigned int Index, GUI_COLOR Color);
void             SPINBOX_SetTimerPeriod  (SPINBOX_Handle hObj, U32 Index, U32 Period);
int              SPINBOX_SetUserData     (SPINBOX_Handle hObj, const void * pSrc, int NumBytes);
void             SPINBOX_SetValue        (SPINBOX_Handle hObj, I32 Value);

/*********************************************************************
*
*       Managing default values
*/
U16  SPINBOX_GetDefaultButtonSize(void);
void SPINBOX_SetDefaultButtonSize(U16 ButtonSize);

/*********************************************************************
*
*       Skinning
*/
void                    SPINBOX_GetSkinFlexProps     (SPINBOX_SKINFLEX_PROPS * pProps, int Index);
void                    SPINBOX_SetSkinClassic       (SPINBOX_Handle hObj);
void                    SPINBOX_SetSkin              (SPINBOX_Handle hObj, WIDGET_DRAW_ITEM_FUNC * pfDrawSkin);
int                     SPINBOX_DrawSkinFlex         (const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo);
void                    SPINBOX_SetSkinFlexProps     (const SPINBOX_SKINFLEX_PROPS * pProps, int Index);
void                    SPINBOX_SetDefaultSkinClassic(void);
WIDGET_DRAW_ITEM_FUNC * SPINBOX_SetDefaultSkin(WIDGET_DRAW_ITEM_FUNC * pfDrawSkin);

#if defined(__cplusplus)
  }
#endif

#endif  // GUI_WINSUPPORT
#endif  // SPINBOX_H

/*************************** End of file ****************************/
