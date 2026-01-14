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
-------------------------- END-OF-HEADER -----------------------------

File    : GUI_Resource.h
Purpose : Resource ID definitions for GUI Simulation
*/

#ifndef GUI_RESOURCE_H        // Avoid multiple inclusion.
#define GUI_RESOURCE_H

/*********************************************************************
*
*      Menu IDs
*/
#define GUI_IDM_ABOUT                       104
#define GUI_IDM_EXIT                        105
#define GUI_IDR_MAINFRAME                   128

#define GUI_ID_EDIT_COPY                    500
#define GUI_ID_VIEW_LCD_0                   510
#define GUI_ID_VIEW_LCD_1                   511
#define GUI_ID_VIEW_LCD_2                   512
#define GUI_ID_VIEW_LCD_3                   513
#define GUI_ID_VIEW_LCD_4                   514
#define GUI_ID_VIEW_LCD_5                   515
#define GUI_ID_VIEW_LCDINFO_0               520
#define GUI_ID_VIEW_LCDINFO_1               521
#define GUI_ID_VIEW_LCDINFO_2               522
#define GUI_ID_VIEW_LCDINFO_3               523
#define GUI_ID_VIEW_LCDINFO_4               524
#define GUI_ID_VIEW_LCDINFO_5               525
#define GUI_ID_FILE_STOPAPPLICATION         540
#define GUI_ID_FILE_CONTINUEAPPLICATION     550
#define GUI_ID_VIEW_SYSINFO                 560
#define GUI_ID_VIEW_LOG                     570
#define GUI_ID_FILE_STAYONTOP               580

/*********************************************************************
*
*      Menu / accelerator resource IDs
*/
#define GUI_IDC_SIMULATION                  109
#define GUI_IDC_SIMULATION_POPUP            110

/*********************************************************************
*
*      Control IDs
*/
#define GUI_IDC_COPYRIGHT                   1000
#define GUI_IDC_VERSION                     1001
#define GUI_IDC_APPNAME                     1002
#define GUI_IDC_STATIC                        -1

/*********************************************************************
*
*      Dialog resource IDs
*/
#define GUI_IDD_ABOUTBOX                    103

/*********************************************************************
*
*      Bitmap resource IDs
*/
#define GUI_IDB_LOGO                        133
#define GUI_IDB_DEVICE                      145
#define GUI_IDB_DEVICE1                     146

#define GUI_IDB_FRAME_N                     150
#define GUI_IDB_FRAME_S                     151
#define GUI_IDB_FRAME_O                     152
#define GUI_IDB_FRAME_W                     153
#define GUI_IDB_FRAME_NW                    154
#define GUI_IDB_FRAME_SW                    155
#define GUI_IDB_FRAME_SO                    156
#define GUI_IDB_FRAME_NO                    157

#define GUI_IDB_FRAME_OFF                   160
#define GUI_IDB_FRAME_OFF1                  161

#endif                        // Avoid multiple inclusion.

/*************************** End of file ****************************/
