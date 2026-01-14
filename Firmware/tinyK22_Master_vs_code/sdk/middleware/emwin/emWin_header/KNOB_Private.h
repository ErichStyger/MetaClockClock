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
File        : KNOB.h
Purpose     : KNOB include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef KNOB_PRIVATE_H
#define KNOB_PRIVATE_H

#include "WM_Intern.h"
#include "KNOB.h"

#if (GUI_SUPPORT_MEMDEV && GUI_WINSUPPORT)

/*********************************************************************
*
*       Object definition
*
**********************************************************************
*/
typedef struct {
  I32 Snap;          // Position where the knob snaps
  I32 Period;        // Time it takes to stop the knob in ms
  GUI_COLOR BkColor; // The Bk color
  I32 Offset;        // the offset
  I32 MinRange;
  I32 MaxRange;
  I32 MinVRange;
  I32 MaxVRange;
  I32 TickSize;      // Minimum movement range in 1/10 of degree
  I32 KeyValue;      // Range of movement for one key push
  U8  Invert;
} KNOB_PROPS;

typedef struct {
  WIDGET Widget;
  WIDGET_DRAW_ITEM_FUNC * pfOwnerDraw;
  void (* pfRotate)(GUI_MEMDEV_Handle hSrc, GUI_MEMDEV_Handle hDst, int dx, int dy, int a, int Mag);
  KNOB_PROPS Props;
  WM_HMEM hContext;
  I32 Angle;
  I32 Value;
  int xSize;
  int ySize;
  I32 AngleRotate;
  GUI_MEMDEV_Handle hMemSrc;
  GUI_MEMDEV_Handle hMemDst;
  GUI_MEMDEV_Handle hMemBk;
  I32               VRangeABS;
} KNOB_OBJ;

/*********************************************************************
*
*       Macros for internal use
*
**********************************************************************
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  #define KNOB_INIT_ID(p) p->Widget.DebugId = WIDGET_TYPE_KNOB
#else
  #define KNOB_INIT_ID(p)
#endif

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  KNOB_OBJ * KNOB_LockH(KNOB_Handle h);
  #define KNOB_LOCK_H(h)   KNOB_LockH(h)
#else
  #define KNOB_LOCK_H(h)   (KNOB_OBJ *)WM_LOCK_H(h)
#endif

/*********************************************************************
*
*       Module internal data
*
**********************************************************************
*/
extern KNOB_PROPS KNOB__DefaultProps;

#endif   // (GUI_SUPPORT_MEMDEV && GUI_WINSUPPORT)
#endif   // KNOB_PRIVATE_H
