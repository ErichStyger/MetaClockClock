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
File        : MOVIE_Private.h
Purpose     : MOVIE private header file
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef MOVIE_PRIVATE_H
#define MOVIE_PRIVATE_H

#include "WM_Intern.h"
#include "MOVIE.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define MOVIE_FLAG_DOLOOP         (1 << 0)
#define MOVIE_FLAG_TIMER          (1 << 1)
#define MOVIE_FLAG_PROGBARPRESSED (1 << 2)

/*********************************************************************
*
*       Object definition
*
**********************************************************************
*/
typedef struct {
  GUI_ANIM_HANDLE hAnim;
  int             Start;
  int             End;
  I16             Pos;
} MOVIE_ANIM_DATA;

typedef struct {
  GUI_COLOR aColor[5];
  int       aPeriod[3];
  int       ySizePanel;
  int       ySizeBar;
  int       Space;
} MOVIE_PROPS;

typedef struct {
  WIDGET           Widget;
  MOVIE_PROPS      Props;
  GUI_MOVIE_INFO   Info;
  GUI_MOVIE_HANDLE hMovie;
  WM_HMEM          ahDrawObj[4];
  int              yOffPanel;
  WM_HTIMER        hTimerPanel;
  MOVIE_ANIM_DATA  AnimShift;
  GUI_MOVIE_FUNC * pfNotify;
  U8               Flags;
  U8               BitmapIndex;
  U8               PanelVisible;
} MOVIE_OBJ;

/*********************************************************************
*
*       Macros for internal use
*
**********************************************************************
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  #define MOVIE_INIT_ID(p) (p->Widget.DebugId = WIDGET_TYPE_MOVIE)
#else
  #define MOVIE_INIT_ID(p)
#endif

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  MOVIE_OBJ * MOVIE_LockH(MOVIE_Handle h);
  #define MOVIE_LOCK_H(h)   MOVIE_LockH(h)
#else
  #define MOVIE_LOCK_H(h)   (MOVIE_OBJ *)WM_LOCK_H(h)
#endif

#endif   /* GUI_WINSUPPORT */
#endif   /* MOVIE_PRIVATE_H */

/*************************** End of file ****************************/
