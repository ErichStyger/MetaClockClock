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
File        : GUIDRV_SH_MEM.h
Purpose     : Interface definition for GUIDRV_SH_MEM driver
---------------------------END-OF-HEADER------------------------------
*/

#ifndef GUIDRV_SH_MEM_H
#define GUIDRV_SH_MEM_H

/*********************************************************************
*
*       Display drivers
*/
//
// Addresses
//
extern const GUI_DEVICE_API GUIDRV_Win_API;

extern const GUI_DEVICE_API GUIDRV_SH_MEM_API;
extern const GUI_DEVICE_API GUIDRV_SH_MEM_API_OXY;
extern const GUI_DEVICE_API GUIDRV_SH_MEM_API_OSX;
extern const GUI_DEVICE_API GUIDRV_SH_MEM_API_OSY;

extern const GUI_DEVICE_API GUIDRV_SH_MEM_3_API;
extern const GUI_DEVICE_API GUIDRV_SH_MEM_3_API_OXY;
extern const GUI_DEVICE_API GUIDRV_SH_MEM_3_API_OSX;
extern const GUI_DEVICE_API GUIDRV_SH_MEM_3_API_OSY;

//
// Macros to be used in configuration files
//
#if defined(WIN32) && !defined(LCD_SIMCONTROLLER)

  #define GUIDRV_SH_MEM            &GUIDRV_Win_API
  #define GUIDRV_SH_MEM_OXY        &GUIDRV_Win_API
  #define GUIDRV_SH_MEM_OSX        &GUIDRV_Win_API
  #define GUIDRV_SH_MEM_OSY        &GUIDRV_Win_API

  #define GUIDRV_SH_MEM_3          &GUIDRV_Win_API
  #define GUIDRV_SH_MEM_3_OXY      &GUIDRV_Win_API
  #define GUIDRV_SH_MEM_3_OSX      &GUIDRV_Win_API
  #define GUIDRV_SH_MEM_3_OSY      &GUIDRV_Win_API

#else

  #define GUIDRV_SH_MEM            &GUIDRV_SH_MEM_API
  #define GUIDRV_SH_MEM_OXY        &GUIDRV_SH_MEM_API_OXY
  #define GUIDRV_SH_MEM_OSX        &GUIDRV_SH_MEM_API_OSX
  #define GUIDRV_SH_MEM_OSY        &GUIDRV_SH_MEM_API_OSY

  #define GUIDRV_SH_MEM_3          &GUIDRV_SH_MEM_3_API
  #define GUIDRV_SH_MEM_3_OXY      &GUIDRV_SH_MEM_3_API_OXY
  #define GUIDRV_SH_MEM_3_OSX      &GUIDRV_SH_MEM_3_API_OSX
  #define GUIDRV_SH_MEM_3_OSY      &GUIDRV_SH_MEM_3_API_OSY

#endif

#define GUIDRV_SH_MEM_8BITMODE  0
#define GUIDRV_SH_MEM_9BITMODE  1
#define GUIDRV_SH_MEM_10BITMODE 2

typedef struct {
  unsigned Period;           // Period used for toggling VCOM
  unsigned ExtMode;          // Setting of EXTMODE configuration pin
  unsigned BitMode;          // 8- or 10-bit line addressing
  unsigned SendEnd;          // Send end command 0 - no, 1 - yes
  void (* pfToggleVCOM)(void);
} CONFIG_SH_MEM;

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/
void GUIDRV_SH_MEM_SetBus8  (GUI_DEVICE * pDevice, GUI_PORT_API * pHW_API);
void GUIDRV_SH_MEM_Config   (GUI_DEVICE * pDevice, CONFIG_SH_MEM * pConfig);
void GUIDRV_SH_MEM_3_SetBus8(GUI_DEVICE * pDevice, GUI_PORT_API * pHW_API);
void GUIDRV_SH_MEM_3_Config (GUI_DEVICE * pDevice, CONFIG_SH_MEM * pConfig);

#endif

/*************************** End of file ****************************/
