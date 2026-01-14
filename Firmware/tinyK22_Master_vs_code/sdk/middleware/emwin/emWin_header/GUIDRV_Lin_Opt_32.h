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
File        : GUIDRV_Lin_Opt_32.h
Purpose     : Optimized routines, included by GUIDRV_Lin_..._32.c
---------------------------END-OF-HEADER------------------------------
*/

/*********************************************************************
*
*       Static functions
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawHLineOpt32
*/
static void _DrawHLineOpt32(GUI_DEVICE * pDevice, int y, int x0, int x1) {
  DRIVER_CONTEXT * pContext;
  register LCD_PIXELINDEX ColorIndex;
  U32 * pDest;
  U32 Off, RemPixels;
  LCD_PIXELINDEX IndexMask;

  pContext  = (DRIVER_CONTEXT *)pDevice->u.pContext;
  RemPixels = x1 - x0 + 1;
  Off       = XY2OFF32(pContext->vxSizePhys, x0, y);
  pDest     = OFF2PTR32(pContext->VRAMAddr, Off);
  if (GUI_pContext->DrawMode & LCD_DRAWMODE_XOR) {
    IndexMask = pDevice->pColorConvAPI->pfGetIndexMask() & 0x00FFFFFF;
    do {
      ColorIndex  = READ_MEM32P(pDest);
      ColorIndex ^= IndexMask;
      WRITE_MEM32P(pDest, ColorIndex);
      pDest++;
    } while (--RemPixels);
  } else {
    ColorIndex = LCD__GetColorIndex();
    if (RemPixels >= 16) {
      do {
        WRITE_MEM32P(pDest     , ColorIndex);
        WRITE_MEM32P(pDest +  1, ColorIndex);
        WRITE_MEM32P(pDest +  2, ColorIndex);
        WRITE_MEM32P(pDest +  3, ColorIndex);
        WRITE_MEM32P(pDest +  4, ColorIndex);
        WRITE_MEM32P(pDest +  5, ColorIndex);
        WRITE_MEM32P(pDest +  6, ColorIndex);
        WRITE_MEM32P(pDest +  7, ColorIndex);
        WRITE_MEM32P(pDest +  8, ColorIndex);
        WRITE_MEM32P(pDest +  9, ColorIndex);
        WRITE_MEM32P(pDest + 10, ColorIndex);
        WRITE_MEM32P(pDest + 11, ColorIndex);
        WRITE_MEM32P(pDest + 12, ColorIndex);
        WRITE_MEM32P(pDest + 13, ColorIndex);
        WRITE_MEM32P(pDest + 14, ColorIndex);
        WRITE_MEM32P(pDest + 15, ColorIndex);
        pDest     += 16;
        RemPixels -= 16;
      } while (RemPixels >= 16);
      RemPixels -= (RemPixels >> 4) << 4;
    }
    while (RemPixels--) {
      WRITE_MEM32P(pDest, ColorIndex);
      pDest++;
    }
  }
}

/*********************************************************************
*
*       _DrawVLineOpt32
*/
static void _DrawVLineOpt32(GUI_DEVICE * pDevice, int x, int y0, int y1) {
  DRIVER_CONTEXT * pContext;
  register LCD_PIXELINDEX ColorIndex;
  U32 * pDest;
  U32 Off, NumLines, OffLine;
  LCD_PIXELINDEX IndexMask;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  NumLines = y1 - y0 + 1;
  OffLine  = pContext->vxSizePhys;
  Off      = XY2OFF32(pContext->vxSizePhys, x, y0);
  pDest    = OFF2PTR32(pContext->VRAMAddr, Off);
  if (GUI_pContext->DrawMode & LCD_DRAWMODE_XOR) {
    IndexMask = pDevice->pColorConvAPI->pfGetIndexMask() & 0x00FFFFFF;
    do {
      ColorIndex  = READ_MEM32P(pDest);
      ColorIndex ^= IndexMask;
      WRITE_MEM32P(pDest, ColorIndex);
      pDest += OffLine;
    } while (--NumLines);
  } else {
    ColorIndex = LCD__GetColorIndex();
    do {
      WRITE_MEM32P(pDest, ColorIndex);
      pDest += OffLine;
    } while (--NumLines);
  }
}

/*********************************************************************
*
*       _FillRectOpt32
*
* Purpose:
*   Optimized filling routine for 32 bpp
*/
static void _FillRectOpt32(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1) {
  DRIVER_CONTEXT * pContext;
  register LCD_PIXELINDEX ColorIndex;
  U32 * pDest;
  U32 * pDest0;
  U32 Off, Off0, RemPixels, NumLines, RemLines, OffLine, RemItems;
  LCD_PIXELINDEX IndexMask;

  pContext  = (DRIVER_CONTEXT *)pDevice->u.pContext;
  RemPixels = x1 - x0 + 1;
  NumLines  = y1 - y0 + 1;
  OffLine   = pContext->vxSizePhys;
  pDest     = NULL;
  if (GUI_pContext->DrawMode & LCD_DRAWMODE_XOR) {
    IndexMask = pDevice->pColorConvAPI->pfGetIndexMask() & 0x00FFFFFF;
    Off0      = XY2OFF32(pContext->vxSizePhys, x0, y0);
    if (RemPixels) {
      for (RemLines = NumLines; RemLines; RemLines--) {
        RemItems = RemPixels;
        Off = Off0 + OffLine * (RemLines - 1);
        do {
          ColorIndex  = READ_MEM32(pContext->VRAMAddr, Off);
          ColorIndex ^= IndexMask;
          WRITE_MEM32(pContext->VRAMAddr, Off, ColorIndex);
          Off++;
        } while (--RemItems);
      }
    }
  } else {
    Off        = XY2OFF32(pContext->vxSizePhys, x0, y0);
    pDest0     = OFF2PTR32(pContext->VRAMAddr, Off);
    ColorIndex = LCD__GetColorIndex();
    if (RemPixels >= 16) {
      for (RemLines = NumLines; RemLines; RemLines--) {
        RemItems = RemPixels;
        pDest    = pDest0 + OffLine * (RemLines - 1);
        do {
          WRITE_MEM32P(pDest     , ColorIndex);
          WRITE_MEM32P(pDest +  1, ColorIndex);
          WRITE_MEM32P(pDest +  2, ColorIndex);
          WRITE_MEM32P(pDest +  3, ColorIndex);
          WRITE_MEM32P(pDest +  4, ColorIndex);
          WRITE_MEM32P(pDest +  5, ColorIndex);
          WRITE_MEM32P(pDest +  6, ColorIndex);
          WRITE_MEM32P(pDest +  7, ColorIndex);
          WRITE_MEM32P(pDest +  8, ColorIndex);
          WRITE_MEM32P(pDest +  9, ColorIndex);
          WRITE_MEM32P(pDest + 10, ColorIndex);
          WRITE_MEM32P(pDest + 11, ColorIndex);
          WRITE_MEM32P(pDest + 12, ColorIndex);
          WRITE_MEM32P(pDest + 13, ColorIndex);
          WRITE_MEM32P(pDest + 14, ColorIndex);
          WRITE_MEM32P(pDest + 15, ColorIndex);
          pDest     += 16;
          RemItems -= 16;
        } while (RemItems >= 16);
      }
      pDest0 = pDest;
      RemPixels -= (RemPixels >> 4) << 4;
    }
    if (RemPixels) {
      for (RemLines = NumLines; RemLines; RemLines--) {
        RemItems = RemPixels;
        pDest    = pDest0 + OffLine * (RemLines - 1);
        do {
          WRITE_MEM32P(pDest, ColorIndex);
          pDest++;
        } while (--RemItems);
      }
    }
  }
}

/*************************** End of file ****************************/
