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
File        : WIDGET_ID.h
Purpose     : Unique widget IDs
---------------------------END-OF-HEADER------------------------------
*/

#ifndef WIDGET_ID_H        /* Avoid multiple inclusion  */
#define WIDGET_ID_H

/*********************************************************************
*
*       Widget type IDs
* 
*  Description
*    Unique type IDs for all widget types.
*/
#define WIDGET_TYPE_BUTTON            /* BUTT */   0x42555454UL   // BUTTON           widget.
#define WIDGET_TYPE_CHECKBOX          /* CHEC */   0x43484543UL   // CHECKBOX         widget.
#define WIDGET_TYPE_DROPDOWN          /* DROP */   0x44524F50UL   // DROPDOWN         widget.
#define WIDGET_TYPE_EDIT              /* EDIT */   0x45444954UL   // EDIT             widget.
#define WIDGET_TYPE_FRAMEWIN          /* FRAM */   0x4652414DUL   // FRAMEWIN         widget.
#define WIDGET_TYPE_FRAMEWIN_CLIENT   /* FRAC */   0x46524143UL   // FRAMEWIN client  widget.
#define WIDGET_TYPE_GRAPH             /* GRAP */   0x47524150UL   // GRAPH            widget.
#define WIDGET_TYPE_HEADER            /* HEAD */   0x48454144UL   // HEADER           widget.
#define WIDGET_TYPE_KEYBOARD          /* KEYB */   0x4B455942UL   // KEYBOARD         widget.
#define WIDGET_TYPE_LISTBOX           /* LISB */   0x4C495342UL   // LISTBOX          widget.
#define WIDGET_TYPE_LISTVIEW          /* LISV */   0x4C495356UL   // LISTVIEW         widget.
#define WIDGET_TYPE_LISTWHEEL         /* LISW */   0x4C495357UL   // LISTWHEEL        widget.
#define WIDGET_TYPE_MENU              /* MENU */   0x4D454E55UL   // MENU             widget.
#define WIDGET_TYPE_MOVIE             /* MOVI */   0x4D4F5649UL   // MOVIE            widget.
#define WIDGET_TYPE_MULTIEDIT         /* MULE */   0x4D554C45UL   // MULTIEDIT        widget.
#define WIDGET_TYPE_MULTIPAGE         /* MULP */   0x4D554C50UL   // MULTIPAGE        widget.
#define WIDGET_TYPE_MULTIPAGE_CLIENT  /* MPCL */   0x4D50434CUL   // MULTIPAGE client widget.
#define WIDGET_TYPE_PROGBAR           /* PROG */   0x50524F47UL   // PROGBAR          widget.
#define WIDGET_TYPE_RADIO             /* RADI */   0x52414449UL   // RADIO            widget.
#define WIDGET_TYPE_SCROLLBAR         /* SCRO */   0x5343524FUL   // SCROLLBAR        widget.
#define WIDGET_TYPE_SLIDER            /* SLID */   0x534C4944UL   // SLIDER           widget.
#define WIDGET_TYPE_SWIPELIST         /* SWIP */   0x53574950UL   // SWIPELIST        widget.
#define WIDGET_TYPE_TEXT              /* TEXT */   0x54455854UL   // TEXT             widget.
#define WIDGET_TYPE_TREEVIEW          /* TREE */   0x54524545UL   // TREEVIEW         widget.
#define WIDGET_TYPE_ICONVIEW          /* ICON */   0x49434F4EUL   // ICONVIEW         widget.
#define WIDGET_TYPE_IMAGE             /* IMAG */   0x494D4147UL   // IMAGE            widget.
#define WIDGET_TYPE_SPINBOX           /* SPIN */   0x5350494EUL   // SPINBOX          widget.
#define WIDGET_TYPE_KNOB              /* KNOB */   0x4B4E4F42UL   // KNOB             widget.
#define WIDGET_TYPE_WINDOW            /* WIND */   0x57494E44UL   // WINDOW           widget.
#define WIDGET_TYPE_ROTARY            /* ROTA */   0x524F5441UL   // ROTARY           widget.
#define WIDGET_TYPE_SWITCH            /* SWIT */   0x53574954UL   // SWITCH           widget.
#define WIDGET_TYPE_TICKER            /* TICK */   0x5449434BUL   // TICKER           widget.
#define WIDGET_TYPE_GAUGE             /* GAUG */   0x47415547UL   // GAUGE            widget.
#define WIDGET_TYPE_QRCODE            /* QRCO */   0x5152434FUL   // QRCODE           widget.
#define WIDGET_TYPE_SCROLLER          /* SCRL */   0x5343523CUL   // SCROLLER         widget.
#define WIDGET_TYPE_WHEEL             /* WHEL */   0x5748454CUL   // WHEEL            widget.

#endif   /* WIDGET_ID_H */

/*************************** End of file ****************************/
