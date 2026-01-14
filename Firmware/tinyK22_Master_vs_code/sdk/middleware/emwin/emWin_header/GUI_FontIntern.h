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
File        : GUI_FontIntern.h
Purpose     : Internal declarations used in font files
---------------------------END-OF-HEADER------------------------------
*/

#ifndef  GUI_FONTINTERN_H    /* Guard against multiple inclusion */
#define  GUI_FONTINTERN_H

#include "GUI.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

//
// Proportional fonts
//
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font8_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font10S_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font10_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font13_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font13B_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font13H_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font13HB_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font16_1_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font16_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_CHARINFO_EXT  GUI_Font16_HK_CharInfo[169];
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font16B_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font20_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font20B_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font24_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font24B_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font32_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font32B_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_FontComic18B_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_FontComic24B_ASCII_Prop1;

//
// Monospaced fonts
//
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font6x8_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font8x8_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font8x13_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font8x15B_ASCII_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font8x16_ASCII_Prop1;

//
// Proportional fonts, antialiased
//
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font12_ASCII_AA4_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font16_ASCII_AA4_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font24_ASCII_AA4_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font32_ASCII_AA4_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font48_ASCII_AA4_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font64_ASCII_AA4_Prop1;

//
// Monospaced fonts, antialiased
//
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font5x12_ASCII_AA4_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font8x16_ASCII_AA4_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font12x24_ASCII_AA4_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font16x32_ASCII_AA4_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font24x48_ASCII_AA4_Prop1;
extern GUI_CONST_STORAGE GUI_FONT_PROP_EXT GUI_Font32x64_ASCII_AA4_Prop1;

#if defined(__cplusplus)
  }
#endif


#endif   /* Guard against multiple inclusion */

/*************************** End of file ****************************/
