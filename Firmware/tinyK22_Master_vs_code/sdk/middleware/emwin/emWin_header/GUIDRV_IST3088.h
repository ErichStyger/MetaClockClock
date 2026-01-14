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
File        : GUIDRV_IST3088.h
Purpose     : Interface definition for GUIDRV_IST3088 driver
---------------------------END-OF-HEADER------------------------------
*/

#ifndef GUIDRV_IST3088_H
#define GUIDRV_IST3088_H

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Display drivers
*/
//
// Addresses
//
extern const GUI_DEVICE_API GUIDRV_IST3088_4_API;
extern const GUI_DEVICE_API GUIDRV_IST3088_OY_4_API;
extern const GUI_DEVICE_API GUIDRV_IST3088_OX_4_API;
extern const GUI_DEVICE_API GUIDRV_IST3088_OXY_4_API;
extern const GUI_DEVICE_API GUIDRV_IST3088_OS_4_API;
extern const GUI_DEVICE_API GUIDRV_IST3088_OSY_4_API;
extern const GUI_DEVICE_API GUIDRV_IST3088_OSX_4_API;
extern const GUI_DEVICE_API GUIDRV_IST3088_OSXY_4_API;

//
// Macros to be used in configuration files
//
#if defined(WIN32) && !defined(LCD_SIMCONTROLLER)

  #define GUIDRV_IST3088_4       &GUIDRV_Win_API
  #define GUIDRV_IST3088_OY_4    &GUIDRV_Win_API
  #define GUIDRV_IST3088_OX_4    &GUIDRV_Win_API
  #define GUIDRV_IST3088_OXY_4   &GUIDRV_Win_API
  #define GUIDRV_IST3088_OS_4    &GUIDRV_Win_API
  #define GUIDRV_IST3088_OSY_4   &GUIDRV_Win_API
  #define GUIDRV_IST3088_OSX_4   &GUIDRV_Win_API
  #define GUIDRV_IST3088_OSXY_4  &GUIDRV_Win_API

#else

  #define GUIDRV_IST3088_4       &GUIDRV_IST3088_4_API
  #define GUIDRV_IST3088_OY_4    &GUIDRV_IST3088_OY_4_API
  #define GUIDRV_IST3088_OX_4    &GUIDRV_IST3088_OX_4_API
  #define GUIDRV_IST3088_OXY_4   &GUIDRV_IST3088_OXY_4_API
  #define GUIDRV_IST3088_OS_4    &GUIDRV_IST3088_OS_4_API
  #define GUIDRV_IST3088_OSY_4   &GUIDRV_IST3088_OSY_4_API
  #define GUIDRV_IST3088_OSX_4   &GUIDRV_IST3088_OSX_4_API
  #define GUIDRV_IST3088_OSXY_4  &GUIDRV_IST3088_OSXY_4_API

#endif

/*********************************************************************
*
*       Public routines
*/
void GUIDRV_IST3088_SetBus16(GUI_DEVICE * pDevice, GUI_PORT_API * pHW_API);

#if defined(__cplusplus)
}
#endif

#endif

/*************************** End of file ****************************/
