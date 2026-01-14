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
File        : JPEGConf.h
Purpose     : Configuration header file to draw JPEG with hardware
----------------------------------------------------------------------
*/

#ifndef JPEGCONF_H
#define JPEGCONF_H

#define JPEG_ARGB8888 0
#define JPEG_RGB565   1

typedef struct {
  U32                 WorkBufferSize;
  void              * (* pfMalloc)(size_t size);
  void                (* pfFree)  (void * ptr);
  int                 ColorFormat;
  U8                * pWorkBuffer;
  GUI_HMEM            hWorkBuffer;
  U8                * pOutBuffer;
  GUI_HMEM            hOutBuffer;
  U32                 OutBufferSize;
  U8                * pInBuffer;
  U32                 NumBytesInBuffer;
  int                 BitsPerPixel;
  int                 BytesPerLine;
  GUI_GET_DATA_FUNC * pfGetData;
  void              * pVoid;
  U32                 Off;
  U32                 xPos;
  U32                 yPos;
  U32                 xSize;
  U32                 ySize;
  U32                 BlockSize;
  U32                 TotalMCUs;
  U32                 IndexMCU;
  U32                 (* pfConvert)(U32 BlockIndex);
  //U8                  HFactor;
  U8                  VFactor;
  U32                 xSizeExtended;
  U8                  Error;
  U8                  IRQFlag;
#if GUI_SUPPORT_MEMDEV
  GUI_MEMDEV_Handle   hMem;
#endif
} JPEG_X_CONTEXT;

int  JPEG_X_Draw        (GUI_GET_DATA_FUNC * pfGetData, void * p, int x0, int y0);
void JPEG_X_DeInit      (void);
void JPEG_X_Init(JPEG_X_CONTEXT * pContext);
void JPEG_X_SetStayAlive(int OnOff);
void JPEG_X_IncStayAlive(void);
void JPEG_X_DecStayAlive(void);

void MOVIE_X_cbNotify    (GUI_MOVIE_HANDLE hMovie, int Notification, U32 CurrentFrame);

#endif /* LCDCONF_H */

/*************************** End of file ****************************/
