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
File        : GUI_SPY_Private.h
Purpose     : Private header file for Spy
---------------------------END-OF-HEADER------------------------------
*/

#ifndef GUI_SPY_PRIVATE_H
#define GUI_SPY_PRIVATE_H

#include "GUI_Private.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#ifndef   GUI_SPY_COM_BUFFER_SIZE
  #define GUI_SPY_COM_BUFFER_SIZE 1000
#endif

#define JOB_REQUEST_STATUS  1
#define JOB_REQUEST_VERSION 2
#define JOB_REQUEST_WINDOWS 3
#define JOB_REQUEST_INPUT   4
#define JOB_REQUEST_BMP     5
#define JOB_QUIT            6

#define INPUT_PID    1
#define INPUT_KEY    2
#define INPUT_MTOUCH 3

#define TRY(x) x; if (r) return 1

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct SPY_CONTEXT SPY_CONTEXT;

struct SPY_CONTEXT{
  GUI_tSend pfSend;
  GUI_tRecv pfRecv;
  void * pConnectInfo;
  U8 acBuffer[GUI_SPY_COM_BUFFER_SIZE];
  int NumBytesInBuffer;
  int Result;
  int (* pfFlush)   (SPY_CONTEXT * pContext);
  int (* pfStoreU8) (SPY_CONTEXT * pContext,  U8 Byte);
  int (* pfStoreU16)(SPY_CONTEXT * pContext, U16 Data);
  int (* pfStoreU32)(SPY_CONTEXT * pContext, U32 Data);
  int (* pfRead)    (SPY_CONTEXT * pContext, U8 * buf, int len);
};

typedef struct {
  U16 Type;
  U32 Time;
  union {
    GUI_KEY_STATE    StateKEY;
    GUI_PID_STATE    StatePID;
    GUI_MTOUCH_STATE StateMTOUCH;
  } u;
} SPY_INPUT;



#endif // GUI_SPY_PRIVATE_H

/*************************** End of file ****************************/
