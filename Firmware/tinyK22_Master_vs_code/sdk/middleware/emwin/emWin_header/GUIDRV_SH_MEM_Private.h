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
File        : GUIDRV_SH_MEM_Private.h
Purpose     : Private header file  for GUIDRV_SH_MEM driver
---------------------------END-OF-HEADER------------------------------
*/

#ifndef GUIDRV_SH_MEM_PRIVATE_H
#define GUIDRV_SH_MEM_PRIVATE_H

#include "GUIDRV_SH_MEM.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define DRIVER_CONTEXT DRIVER_CONTEXT_SH_MEM

#define COMMAND_LEN    (0x02)

#define XY2OFF(pContext, x, y) (2 + (x >> 3) + (y * (pContext->BytesPerLine + 2)))

#define XY2OFF3(pContext, x, y) (((x * 3) / 24) * 3 + (pContext->BytesPerLine * y))

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct DRIVER_CONTEXT DRIVER_CONTEXT;

struct DRIVER_CONTEXT {
  GUI_PORT_API     HW_API;
  int              xSize, ySize;
  int              ySizePhys;
  int              BytesPerLine;
  GUI_TIMER_HANDLE hTimer;
  unsigned         Period;                                                 // Period used for toggling VCOM
  void          (* pfToggleVCOM)(void);                                    // Routine to be called for toggling VCOM
  void          (* pfSendLine)(DRIVER_CONTEXT * pContext, int LineIndex);  // Routine to be used to send one line of data
  U8             * pDirty;                                                 // Pointer to dirty markers
  U8             * pVRAM;                                                  // Pointer to cache
  U8               CacheLocked;                                            // Cache is locked if 1
  U8               IsDirty;                                                // Set to 1 if any bit is dirty
  U8               VCom;                                                   // Current state of VCOM signal
  U8               ExtMode;                                                // Setting of EXTMODE configuration pin
  U8               BitMode;                                                // 8-, 9- or 10-bit line addressing
  U8               SendEnd;                                                // Send end command after sending cache
};

void         GUIDRV_SH_MEM__SendCacheOnDemand(DRIVER_CONTEXT * pContext, int y0, int y1);
const void * GUIDRV_SH_MEM__GetDevData       (GUI_DEVICE * pDevice, int Index);
void         GUIDRV_SH_MEM__GetRect          (GUI_DEVICE * pDevice, LCD_RECT * pRect);
void         GUIDRV_SH_MEM__SetOrg           (GUI_DEVICE * pDevice, int x, int y);
void      (* GUIDRV_SH_MEM__GetDevFunc       (GUI_DEVICE ** ppDevice, int Index))(void);
void      (* GUIDRV_SH_MEM_3__GetDevFunc     (GUI_DEVICE ** ppDevice, int Index))(void);
const void * GUIDRV_SH_MEM_3__GetDevData     (GUI_DEVICE * pDevice, int Index);

#endif // GUIDRV_SH_MEM_PRIVATE_H

/*************************** End of file ****************************/
