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
File        : IMAGE.h
Purpose     : Image include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef IMAGE_H
#define IMAGE_H

#include "WM.h"
#include "DIALOG_Type.h"
#include "WIDGET.h"

#if GUI_WINSUPPORT

#if defined(__cplusplus)
  extern "C" { // Make sure we have C-declarations in C++ programs
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
/*********************************************************************
*
*       IMAGE create flags
*
*  Description
*    Create flags for the IMAGE widget. These flags can be passed to ICONVIEW_CreateEx() via the
*    \a{ExFlags} parameter.
*/
#define IMAGE_CF_MEMDEV   (1 << 0)      // Makes the IMAGE widget use an internal Memory Device for
                                        // drawing. Contrary to the Memory Device which is created
                                        // by the Window Manager's automatic use of Memory Devices
                                        // (WM_CF_MEMDEV), this device stays valid all the time. It has to be
                                        // ensured that the emWin memory pool which is defined by the
                                        // function GUI_ALLOC_AssignMemory() (in \c{GUIConf.c}), is big enough
                                        // to store the complete data. If the Memory Device can not be created,
                                        // the image is drawn directly. This might possibly mean loss
                                        // of performance.
#define IMAGE_CF_TILE     (1 << 1)      // Uses tiling to fill up the whole area of the widget.
#define IMAGE_CF_ALPHA    (1 << 2)      // Needs to be set if alpha blending is required (PNG).
#define IMAGE_CF_ATTACHED (1 << 3)      // Widget size is fixed to the parent border.
#define IMAGE_CF_AUTOSIZE (1 << 4)      // Widget size is taken from the attached image.
#define IMAGE_CF_LQ       (1 << 5)      // Fast mode (lower quality) should be used for rotating/scaling.

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef WM_HMEM IMAGE_Handle;

/*********************************************************************
*
*       Public functions
*
**********************************************************************
*/
IMAGE_Handle IMAGE_CreateEx      (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id);
IMAGE_Handle IMAGE_CreateUser    (int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id, int NumExtraBytes);
IMAGE_Handle IMAGE_CreateIndirect(const GUI_WIDGET_CREATE_INFO * pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK * cb);

void IMAGE_Callback(WM_MESSAGE * pMsg);

/*********************************************************************
*
*       Member functions
*
**********************************************************************
*/
int  IMAGE_GetImageSize(IMAGE_Handle hObj, int * pxSize, int * pySize);
int  IMAGE_GetUserData (IMAGE_Handle hObj, void * pDest, int NumBytes);
void IMAGE_SetBitmap   (IMAGE_Handle hWin, const GUI_BITMAP * pBitmap);
void IMAGE_SetBMP      (IMAGE_Handle hObj, const void * pData, U32 FileSize);
void IMAGE_SetBMPEx    (IMAGE_Handle hObj, GUI_BMP_GET_DATA_FUNC * pfGetData, void * pVoid);
void IMAGE_SetDTA      (IMAGE_Handle hObj, const void * pData, U32 FileSize);
void IMAGE_SetDTAEx    (IMAGE_Handle hObj, GUI_DTA_GET_DATA_FUNC * pfGetData, void * pVoid);
void IMAGE_SetGIF      (IMAGE_Handle hObj, const void * pData, U32 FileSize);
void IMAGE_SetGIFEx    (IMAGE_Handle hObj, GUI_GIF_GET_DATA_FUNC * pfGetData, void * pVoid);
void IMAGE_SetJPEG     (IMAGE_Handle hObj, const void * pData, U32 FileSize);
void IMAGE_SetJPEGEx   (IMAGE_Handle hObj, GUI_JPEG_GET_DATA_FUNC * pfGetData, void * pVoid);
void IMAGE_SetPNG      (IMAGE_Handle hObj, const void * pData, U32 FileSize);
void IMAGE_SetPNGEx    (IMAGE_Handle hObj, GUI_PNG_GET_DATA_FUNC * pfGetData, void * pVoid);
void IMAGE_SetTiled    (IMAGE_Handle hObj, int OnOff);
int  IMAGE_SetUserData (IMAGE_Handle hObj, const void * pSrc, int NumBytes);
void IMAGE_SetAlign    (IMAGE_Handle hObj, int Align);
void IMAGE_SetOffset   (IMAGE_Handle hObj, int xOff, int yOff);
void IMAGE_SetBkColor  (IMAGE_Handle hObj, GUI_COLOR BkColor);

#if GUI_SUPPORT_MEMDEV
U8   IMAGE_SetAlpha    (IMAGE_Handle hObj, U8 Alpha);
int  IMAGE_SetAngle    (IMAGE_Handle hObj, unsigned Angle);
void IMAGE_EnableLQ    (IMAGE_Handle hObj, int OnOff);
int  IMAGE_SetScale    (IMAGE_Handle hObj, unsigned Scale);
#endif

#if defined(__cplusplus)
  }
#endif

#endif // GUI_WINSUPPORT
#endif // IMAGE_H

/*************************** End of file ****************************/
