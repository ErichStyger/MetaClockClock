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
File        : GUIDRV_SLin.h
Purpose     : Interface definition for GUIDRV_SLin driver
---------------------------END-OF-HEADER------------------------------
*/

#include "GUIDRV_SLin.h"
#include "GUIDRV_NoOpt_1_8.h"

#ifndef GUIDRV_SLIN_PRIVATE_H
#define GUIDRV_SLIN_PRIVATE_H

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
//
// Use unique context identified
//
#define DRIVER_CONTEXT DRIVER_CONTEXT_SLIN

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct DRIVER_CONTEXT DRIVER_CONTEXT;

/*********************************************************************
*
*       MANAGE_VRAM_CONTEXT
*/
typedef struct {
  //
  // Should be the same for all controllers
  //
  U8   (* pfReadVMEM )  (DRIVER_CONTEXT * pContext, U32 Off);
  void (* pfWriteVMEM)  (DRIVER_CONTEXT * pContext, U32 Off, U8 Data);
  void (* pfWritePixels)(DRIVER_CONTEXT * pContext, U32 Off, U8 Data);
  //
  // Depends on controller
  //
  void (* pfSetADDR)    (DRIVER_CONTEXT * pContext, U32 Off);
  void (* pfSetWriteNC) (DRIVER_CONTEXT * pContext);
  void (* pfSetWriteC)  (DRIVER_CONTEXT * pContext);
  void (* pfSendCache)  (DRIVER_CONTEXT * pContext, int x0, int y0, int x1, int y1);
  U32  (* pfXY2Off)     (DRIVER_CONTEXT * pContext, int x, int y);
  //
  // Only used by SSD1848
  //
  U8   (* pfData)       (U8 Data);
  void (* pfPutData)    (DRIVER_CONTEXT * pContext, U32 Off, U8 Data);
  U8   (* pfGetData)    (DRIVER_CONTEXT * pContext, U32 Off);
  void (* pfWriteData)  (DRIVER_CONTEXT * pContext, U8 Data);
  //
  // Only used by T6963
  //
  void (* pfCheckBusy)  (DRIVER_CONTEXT * pContext, U8 Mask);
  void (* pfSetOrg)     (DRIVER_CONTEXT * pContext);
} MANAGE_VMEM_API;

/*********************************************************************
*
*       DRIVER_CONTEXT
*/
struct DRIVER_CONTEXT {
  //
  // Common data
  //
  int xSize, ySize;
  int vxSize, vySize;
  int UseCache;
  int MemSize;
  U32 BufferOffset;
  //
  // Cache specific data
  //
  U8 * pVMEM;
  int CacheLocked;
  int IsDirty;
  GUI_RECT DirtyRect;
  //
  // Driver specific data
  //
  int FirstSEG;
  int FirstCOM;
  int UseMirror;   // Used by SSD1848
  int CheckBusy;   // Used by T6963
  U32 Off;         // Used by T6963
  U8  UseDualScan; // Used by T6963
  //
  // Accelerators for calculation
  //
  int BytesPerLine;
  int BitsPerPixel;
  //
  // Mirroring (used by UC1617)
  //
  const U8 * pMirror;
  //
  // Pointer to driver internal initialization routine
  //
  void (* pfInit)(GUI_DEVICE * pDevice);
  //
  // Set if cache is used
  //
  int  (* pfControlCache)(GUI_DEVICE *  pDevice, int Cmd);
  void (* pfRefresh     )(GUI_DEVICE *  pDevice);
  //
  // API-Tables
  //
  MANAGE_VMEM_API ManageVMEM_API; // Memory management
  GUI_PORT_API    HW_API;         // Hardware routines
};

/*********************************************************************
*
*       LOG2PHYS_xxx
*/
#define LOG2PHYS_X      (                  x    )
#define LOG2PHYS_X_OX   (pContext->xSize - x - 1)
#define LOG2PHYS_X_OY   (                  x    )
#define LOG2PHYS_X_OXY  (pContext->xSize - x - 1)
#define LOG2PHYS_X_OS   (                  y    )
#define LOG2PHYS_X_OSX  (pContext->ySize - y - 1)
#define LOG2PHYS_X_OSY  (                  y    )
#define LOG2PHYS_X_OSXY (pContext->ySize - y - 1)

#define LOG2PHYS_Y      (                  y    )
#define LOG2PHYS_Y_OX   (                  y    )
#define LOG2PHYS_Y_OY   (pContext->ySize - y - 1)
#define LOG2PHYS_Y_OXY  (pContext->ySize - y - 1)
#define LOG2PHYS_Y_OS   (                  x    )
#define LOG2PHYS_Y_OSX  (                  x    )
#define LOG2PHYS_Y_OSY  (pContext->xSize - x - 1)
#define LOG2PHYS_Y_OSXY (pContext->xSize - x - 1)

/*********************************************************************
*
*       _SetPixelIndex_##EXT
*/
#define DEFINE_SETPIXELINDEX(EXT, X_PHYS, Y_PHYS)                                                 \
static void _SetPixelIndex_##EXT(GUI_DEVICE * pDevice, int x, int y, LCD_PIXELINDEX PixelIndex) { \
  DRIVER_CONTEXT * pContext;                                                                      \
                                                                                                  \
  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;                                               \
  _SetPixelIndex(pContext, X_PHYS, Y_PHYS, PixelIndex);                                           \
}

/*********************************************************************
*
*       _GetPixelIndex_##EXT
*/
#define DEFINE_GETPIXELINDEX(EXT, X_PHYS, Y_PHYS)                                \
static LCD_PIXELINDEX _GetPixelIndex_##EXT(GUI_DEVICE * pDevice, int x, int y) { \
  DRIVER_CONTEXT * pContext;                                                     \
  LCD_PIXELINDEX PixelIndex;                                                     \
                                                                                 \
  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;                              \
  PixelIndex = _GetPixelIndex(pContext, X_PHYS, Y_PHYS);                         \
  return PixelIndex;                                                             \
}

/*********************************************************************
*
*       _GetDevProp_##EXT
*/
#define DEFINE_GETDEVPROP(EXT, MX, MY, SWAP)                    \
static I32 _GetDevProp_##EXT(GUI_DEVICE * pDevice, int Index) { \
  switch (Index) {                                              \
  case LCD_DEVCAP_MIRROR_X: return MX;                          \
  case LCD_DEVCAP_MIRROR_Y: return MY;                          \
  case LCD_DEVCAP_SWAP_XY:  return SWAP;                        \
  default:                  break;                              \
  }                                                             \
  return _GetDevProp(pDevice, Index);                           \
}

/*********************************************************************
*
*       DEFINE_FUNCTIONS
*/
#define DEFINE_FUNCTIONS(EXT, X_PHYS, Y_PHYS, MX, MY, SWAP) \
  DEFINE_SETPIXELINDEX(EXT, X_PHYS, Y_PHYS)                 \
  DEFINE_GETPIXELINDEX(EXT, X_PHYS, Y_PHYS)                 \
  DEFINE_GETDEVPROP(EXT, MX, MY, SWAP)                      \
  DEFINE_GUI_DEVICE_API(EXT)

/*********************************************************************
*
*       Private functions
*
**********************************************************************
*/
void (*GUIDRV__SLin_GetDevFunc(GUI_DEVICE ** ppDevice, int Index))(void);
void   GUIDRV__SLin_SetOrg    (GUI_DEVICE *  pDevice,  int x, int y);
I32    GUIDRV__SLin_GetDevProp(GUI_DEVICE *  pDevice,  int Index);
void   GUIDRV__SLin_GetRect   (GUI_DEVICE *  pDevice,  LCD_RECT * pRect);

void   GUIDRV__SLin_SendCache (DRIVER_CONTEXT * pContext, int x0, int y0, int x1, int y1);
void   GUIDRV__SLin_WriteVMEM (DRIVER_CONTEXT * pContext, U32 Off, U8 Data, int x, int y);

#if defined(__cplusplus)
}
#endif

#endif

/*************************** End of file ****************************/
