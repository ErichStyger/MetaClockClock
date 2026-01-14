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
File        : GAUGE.h
Purpose     : GAUGE include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef GAUGE_PRIVATE_H
#define GAUGE_PRIVATE_H

#include "WM_Intern.h"
#include "GAUGE.h"

#if (GUI_WINSUPPORT)

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define PRE_VOID                   \
  {                                \
    GAUGE_OBJ * pObj;              \
    if (hObj) {                    \
      GUI_LOCK();                  \
      pObj = GAUGE_LOCK_H(hObj); {

#define POST_VOID_INVALIDATE       \
        WM_InvalidateWindow(hObj); \
      } GUI_UNLOCK_H(pObj);        \
      GUI_UNLOCK();                \
    }                              \
  }

#define POST_VOID                  \
      } GUI_UNLOCK_H(pObj);        \
      GUI_UNLOCK();                \
    }                              \
  }

/*********************************************************************
*
*       Object definition
*
**********************************************************************
*/
typedef struct {
  GUI_COLOR aColor[2];
  GUI_COLOR BkColor;
  I16       Align;
  I16       xOff, yOff;
} GAUGE_PROPS;

typedef struct {
  WIDGET      Widget;             // (obvious)
  GAUGE_PROPS Props;              // (obvious)
  int         Radius;             // 
  int         aWidth[2];          // 
  I32         Value;              // 
  I32         Ang0;               // 
  I32         Ang1;               // 
  I32         Angle;              // Current angle within the given range (Ang0 & Ang1)
  I32         MinVRange;          // Minimum of value range
  I32         MaxVRange;          // Maximum of value range
  U8          Flags;
} GAUGE_OBJ;

/*********************************************************************
*
*       Macros for internal use
*
**********************************************************************
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  #define GAUGE_INIT_ID(p) (p->Widget.DebugId = WIDGET_TYPE_GAUGE)
#else
  #define GAUGE_INIT_ID(p)
#endif

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  GAUGE_OBJ * GAUGE_LockH(GAUGE_Handle h);
  #define GAUGE_LOCK_H(h) GAUGE_LockH(h)
#else
  #define GAUGE_LOCK_H(h) (GAUGE_OBJ *)WM_LOCK_H(h)
#endif

/*********************************************************************
*
*       Public data (internal defaults)
*
**********************************************************************
*/
extern GAUGE_PROPS GAUGE__DefaultProps;

#endif  // (GUI_SUPPORT_MEMDEV && GUI_WINSUPPORT)
#endif  // GAUGE_PRIVATE_H
