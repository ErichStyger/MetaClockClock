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
File        : WHEEL.h
Purpose     : WHEEL include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef WHEEL_PRIVATE_H
#define WHEEL_PRIVATE_H

#include "WM_Intern.h"
#include "WHEEL.h"
#include "GUI_ARRAY.h"

#if (GUI_SUPPORT_MEMDEV && GUI_WINSUPPORT)

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define INDEX_CELL   0
#define INDEX_CENTER 1

/*********************************************************************
*
*       Object definition
*
**********************************************************************
*/
typedef struct {
  I16       Align;
  I16       xOff;
  I16       yOff;
  GUI_COLOR Color;
} WHEEL_ITEM_ATTR;

typedef struct {
  WHEEL_ITEM_ATTR  aAttr[2];        // Separate attributes for cell (0) & center (1)
  GUI_ARRAY        TextArray;       // Single array of text to be used
  const GUI_FONT * apFont[2];       // Font
  U8               HasCenter;       // >0 if center properties are available
} WHEEL_TEXT;

typedef struct {
  WHEEL_ITEM_ATTR  aAttr[2];        // Separate attributes for cell (0) & center (1)
  GUI_HMEM         aDraw[2];        // Separate memory blocks of GUI_DRAW_HANDLEs for cell (0) & center (1)
  unsigned         aNumItems[2];    // Number of items of each block
  U8               HasCenter;       // >0 if center properties are available
} WHEEL_DRAW;

typedef struct {
  U8               FrameRadius;     // Radius of frame
  U8               FrameSize;       // Size of frame
  GUI_COLOR        FrameColor;      // Color of frame
  GUI_COLOR        Color;           // Inner color
} WHEEL_BOX;

typedef struct {
  U32              Period;          // Time in ms it takes to stop the WHEEL in ms
} WHEEL_PROPS;

typedef struct {
  WIDGET           Widget;          // (obvious)
  WHEEL_PROPS      Props;           // (obvious)
  GUI_ARRAY        TextArray;       // Array of WHEEL_TEXT contains the text to be shown
  GUI_ARRAY        DrawArray;       // Array of WHEEL_DRAW contains the images to be shown
  WHEEL_BOX        aBox[2];         // Separate WHEEL_BOX objects for cell & center
  GUI_COLOR        BkColor;         // Background color
  unsigned         NumItems;        // Number of items
  U16              CellSize;        // Size of cell
  U16              CenterSize;      // Size of center
  U16              Cutaway;         // Size of cutaway
  U16              HBorder;         // Additional horizontal border for boxes in morph mode and for text clipping in plain mode
  U8               AlignCutaway;    // Alignment of cutaway
  U8               Align;           // Alignment of items
  U8               HasCenter;       // Has separate center attributes
  U8               Index;           // Index of draw object currently in operation
  int              Pos;             // Motion pos
  int              ReleasedItem;    // Index of last released item
  GUI_ANIM_HANDLE  hAnim;           // Handle of animation used to move to a position
  int              AnimDist;        // Distance to be moved
  int              AnimStart;       // Start value of animation
  GUI_HMEM         ahOverlay[3];    // Separate memory blocks of GUI_DRAW_HANDLEs for overlay bitmaps
  WHEEL_ITEM_ATTR  aAttrOverlay[3]; // Alignment & color of overlay bitmaps
  WM_HMEM          hContext;        // Motion context
  WIDGET_DRAW_ITEM_FUNC * pfDrawItem;
} WHEEL_OBJ;

/*********************************************************************
*
*       Private functions
*
**********************************************************************
*/
void         WHEEL__SetMotionFlags        (WHEEL_OBJ * pObj);
void         WHEEL__AddBitmapObjects      (WHEEL_Handle hObj, const GUI_HMEM                    * phMem, unsigned NumItems, U8 Align, I16 xOff, I16 yOff, GUI_COLOR Color);
void         WHEEL__SetCenterBitmapObjects(WHEEL_Handle hObj, unsigned Index, const GUI_HMEM    * phMem, unsigned NumItems, U8 Align, I16 xOff, I16 yOff, GUI_COLOR Color);
char       * WHEEL__GetItemTextLocked     (WHEEL_Handle hObj, unsigned Index, unsigned Row);
void         WHEEL__ClipPos               (WHEEL_OBJ * pObj);

/*********************************************************************
*
*       Macros for internal use
*
**********************************************************************
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  #define WHEEL_INIT_ID(p) p->Widget.DebugId = WIDGET_TYPE_WHEEL
#else
  #define WHEEL_INIT_ID(p)
#endif

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  WHEEL_OBJ * WHEEL_LockH(WHEEL_Handle h);
  #define WHEEL_LOCK_H(h)   WHEEL_LockH(h)
#else
  #define WHEEL_LOCK_H(h)   (WHEEL_OBJ *)WM_LOCK_H(h)
#endif

/*********************************************************************
*
*       Module internal data
*
**********************************************************************
*/
extern WHEEL_PROPS WHEEL__DefaultProps;

#endif   // (GUI_SUPPORT_MEMDEV && GUI_WINSUPPORT)
#endif   // WHEEL_PRIVATE_H

/************************* end of file ******************************/
