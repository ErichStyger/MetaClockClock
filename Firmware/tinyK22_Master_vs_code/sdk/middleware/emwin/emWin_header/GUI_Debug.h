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
File        : GUI_Debug.h
Purpose     : Debug macros
----------------------------------------------------------------------
   Debug macros for logging

 In the GUI Simulation, all output is transferred into the log window.
*/

#ifndef GUI_DEBUG_H
#define GUI_DEBUG_H

#include <stddef.h>

#include "GUI.h"

#define GUI_DEBUG_LEVEL_NOCHECK       0  /* No run time checks are performed */
#define GUI_DEBUG_LEVEL_CHECK_PARA    1  /* Parameter checks are performed to avoid crashes */
#define GUI_DEBUG_LEVEL_CHECK_ALL     2  /* Parameter checks and consistency checks are performed */
#define GUI_DEBUG_LEVEL_LOG_ERRORS    3  /* Errors are recorded */
#define GUI_DEBUG_LEVEL_LOG_WARNINGS  4  /* Errors & Warnings are recorded */
#define GUI_DEBUG_LEVEL_LOG_ALL       5  /* Errors, Warnings and Messages are recorded. */

#ifndef GUI_DEBUG_LEVEL
  #ifdef WIN32
    #define GUI_DEBUG_LEVEL GUI_DEBUG_LEVEL_LOG_WARNINGS  /* Simulation should log all warnings */
  #else
    #define GUI_DEBUG_LEVEL GUI_DEBUG_LEVEL_NOCHECK       /* For most targets, min. size is important */
  #endif
#endif

#define GUI_LOCK_H(hMem)   GUI_ALLOC_LockH(hMem)
#define GUI_UNLOCK_H(pMem) GUI_ALLOC_UnlockH((void **)&pMem)
#define GUI_UNLOCK_C(pMem) GUI_ALLOC_UnlockC((const void **)&pMem)

/*******************************************************************
*
*               Commandline
*
********************************************************************
*/

#ifdef WIN32
  #define GUI_DEBUG_GETCMDLINE() SIM_GetCmdLine()
#else
  #define GUI_DEBUG_GETCMDLINE() 0
#endif

/*******************************************************************
*
*               Error macros
*
********************************************************************
*/

/* Make sure the macros are actually defined */

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_LOG_ERRORS
  #define GUI_DEBUG_ERROROUT(s)              GUI_ErrorOut(s)
  #define GUI_DEBUG_ERROROUT1(s,p0)          GUI_ErrorOut1(s,p0)
  #define GUI_DEBUG_ERROROUT2(s,p0,p1)       GUI_ErrorOut2(s,p0,p1)
  #define GUI_DEBUG_ERROROUT3(s,p0,p1,p2)    GUI_ErrorOut3(s,p0,p1,p2)
  #define GUI_DEBUG_ERROROUT4(s,p0,p1,p2,p3) GUI_ErrorOut4(s,p0,p1,p2,p3)
  #define GUI_DEBUG_ERROROUT_IF(exp,s)              { if (exp) GUI_DEBUG_ERROROUT(s); }
  #define GUI_DEBUG_ERROROUT1_IF(exp,s,p0)          { if (exp) GUI_DEBUG_ERROROUT1(s,p0); }
  #define GUI_DEBUG_ERROROUT2_IF(exp,s,p0,p1)       { if (exp) GUI_DEBUG_ERROROUT2(s,p0,p1); }
  #define GUI_DEBUG_ERROROUT3_IF(exp,s,p0,p1,p2)    { if (exp) GUI_DEBUG_ERROROUT3(s,p0,p1,p2); }
  #define GUI_DEBUG_ERROROUT4_IF(exp,s,p0,p1,p2,p3) { if (exp) GUI_DEBUG_ERROROUT4(s,p0,p1,p2,p3); }
#else
  #define GUI_DEBUG_ERROROUT(s)
  #define GUI_DEBUG_ERROROUT1(s,p0)
  #define GUI_DEBUG_ERROROUT2(s,p0,p1)
  #define GUI_DEBUG_ERROROUT3(s,p0,p1,p2)
  #define GUI_DEBUG_ERROROUT4(s,p0,p1,p2,p3)
  #define GUI_DEBUG_ERROROUT_IF(exp,s)
  #define GUI_DEBUG_ERROROUT1_IF(exp,s,p0)
  #define GUI_DEBUG_ERROROUT2_IF(exp,s,p0,p1)
  #define GUI_DEBUG_ERROROUT3_IF(exp,s,p0,p1,p2)
  #define GUI_DEBUG_ERROROUT4_IF(exp,s,p0,p1,p2,p3)
#endif

/*******************************************************************
*
*               Warning macros
*
********************************************************************
*/

/* Make sure the macros are actually defined */

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_LOG_WARNINGS
  #define GUI_DEBUG_WARN(s)              GUI_Warn(s)
  #define GUI_DEBUG_WARN1(s,p0)          GUI_Warn1(s,p0)
  #define GUI_DEBUG_WARN2(s,p0,p1)       GUI_Warn2(s,p0,p1)
  #define GUI_DEBUG_WARN3(s,p0,p1,p2)    GUI_Warn3(s,p0,p1,p2)
  #define GUI_DEBUG_WARN4(s,p0,p1,p2,p3) GUI_Warn4(s,p0,p1,p2,p3)
  #define GUI_DEBUG_WARN_IF(exp,s)                  { if (exp) GUI_DEBUG_WARN(s); }
  #define GUI_DEBUG_WARN1_IF(exp,s,p0)              { if (exp) GUI_DEBUG_WARN1(s,p0); }
  #define GUI_DEBUG_WARN2_IF(exp,s,p0,p1)           { if (exp) GUI_DEBUG_WARN2(s,p0,p1); }
  #define GUI_DEBUG_WARN3_IF(exp,s,p0,p1,p2)        { if (exp) GUI_DEBUG_WARN3(s,p0,p1,p2); }
  #define GUI_DEBUG_WARN4_IF(exp,s,p0,p1,p2,p3)     { if (exp) GUI_DEBUG_WARN4(s,p0,p1,p2,p3); }
#else
  #define GUI_DEBUG_WARN(s)
  #define GUI_DEBUG_WARN1(s,p0)
  #define GUI_DEBUG_WARN2(s,p0,p1)
  #define GUI_DEBUG_WARN3(s,p0,p1,p2)
  #define GUI_DEBUG_WARN4(s,p0,p1,p2,p3)
  #define GUI_DEBUG_WARN_IF(exp,s)
  #define GUI_DEBUG_WARN1_IF(exp,s,p0)
  #define GUI_DEBUG_WARN2_IF(exp,s,p0,p1)
  #define GUI_DEBUG_WARN3_IF(exp,s,p0,p1,p2)
  #define GUI_DEBUG_WARN4_IF(exp,s,p0,p1,p2,p3)
#endif

/*******************************************************************
*
*               Logging macros
*
********************************************************************
*/
/* Make sure the macros are actually defined */

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_LOG_ALL
  #define GUI_DEBUG_LOG(s)              GUI_Log(s)
  #define GUI_DEBUG_LOG1(s,p0)          GUI_Log1(s,p0)
  #define GUI_DEBUG_LOG2(s,p0,p1)       GUI_Log2(s,p0,p1)
  #define GUI_DEBUG_LOG3(s,p0,p1,p2)    GUI_Log3(s,p0,p1,p2)
  #define GUI_DEBUG_LOG4(s,p0,p1,p2,p3) GUI_Log4(s,p0,p1,p2,p3)
  #define GUI_DEBUG_LOG_IF(exp,s)                   { if (exp) GUI_DEBUG_LOG(s); }
  #define GUI_DEBUG_LOG1_IF(exp,s,p0)               { if (exp) GUI_DEBUG_LOG1(s,p0); }
  #define GUI_DEBUG_LOG2_IF(exp,s,p0,p1)            { if (exp) GUI_DEBUG_LOG2(s,p0,p1); }
  #define GUI_DEBUG_LOG3_IF(exp,s,p0,p1,p2)         { if (exp) GUI_DEBUG_LOG3(s,p0,p1,p2); }
  #define GUI_DEBUG_LOG4_IF(exp,s,p0,p1,p2,p3)      { if (exp) GUI_DEBUG_LOG4(s,p0,p1,p2,p3); }
#else
  #define GUI_DEBUG_LOG(s)
  #define GUI_DEBUG_LOG1(s,p0)
  #define GUI_DEBUG_LOG2(s,p0,p1)
  #define GUI_DEBUG_LOG3(s,p0,p1,p2)
  #define GUI_DEBUG_LOG4(s,p0,p1,p2,p3)
  #define GUI_DEBUG_LOG_IF(exp,s)
  #define GUI_DEBUG_LOG1_IF(exp,s,p0)
  #define GUI_DEBUG_LOG2_IF(exp,s,p0,p1)
  #define GUI_DEBUG_LOG3_IF(exp,s,p0,p1,p2)
  #define GUI_DEBUG_LOG4_IF(exp,s,p0,p1,p2,p3)
#endif

/*******************************************************************
*
*               Asserts
*
********************************************************************
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_LOG_ERRORS
  #define GUI_DEBUG_ASSERT(exp)                     { if (!exp) GUI_DEBUG_ERROROUT(#exp); }
#else
  #define GUI_DEBUG_ASSERT(exp)
#endif

#endif /* LCD_H */




/*************************** End of file ****************************/
