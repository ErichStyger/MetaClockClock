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
File        : IMAGE_Private.h
Purpose     : Private IMAGE include
--------------------END-OF-HEADER-------------------------------------
*/

#ifndef IMAGE_PRIVATE_H
#define IMAGE_PRIVATE_H

#include "GUI_Private.h"
#include "WM_Intern.h"
#include "IMAGE.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Private config defaults
*
**********************************************************************
*/
#ifndef IMAGE_SUPPORT_TRANSPARENCY
  #define IMAGE_SUPPORT_TRANSPARENCY WM_SUPPORT_TRANSPARENCY
#endif

/*********************************************************************
*
*       Object definition
*
**********************************************************************
*/
typedef struct {
  U32 Flags;
  GUI_COLOR BkColor;
} IMAGE_PROPS;

typedef struct {
  WIDGET              Widget;
  IMAGE_PROPS         Props;
  const void        * pData;                                                          // Data pointer of the object to be drawn (Bitmap, BMP, GIF, JPEG, PNG)
  void             (* pfDrawImageAt) (IMAGE_Handle hObj, int xPos, int yPos);         // Object specific (Bitmap, BMP, GIF, JPEG, PNG) code
  void             (* pfPaint)       (IMAGE_Handle hObj);                             // Drawing method specific (Default, Tiled, Magnified) code
  void             (* pfGetImageSize)(IMAGE_Handle hObj, int * pxSize, int * pySize); // Returns the image size of the attached item
  void             (* pfOnTimer)     (IMAGE_Handle hObj);                             // Timer function for animated images (currently only animated GIFs are supported)
  U32                 FileSize;
  //
  // Data items used by IAMGE_GIF.c
  //
  int                 NumImages;    // Number of (sub)images
  int                 CurrentImage; // Image index used for animated images
  GUI_TIMER_HANDLE    hTimer;       // Timer used for animated images
  //
  // Data items used by IAMGE_DTA.c
  //
  GUI_BITMAP          Bitmap;       // Bitmap structure
  GUI_LOGPALETTE      Palette;      // Palette structure
  //
  // Data items used by Image_...Ex() - functions
  //
  void              * pVoid;        // Void pointer passed to GetData() function
  GUI_GET_DATA_FUNC * pfGetData;    // Pointer to GetData() function
  //
  // Alignment (Important: When tiling is active, alignment does not have any effect)
  //
  I16                 xOff, yOff;   // Additional offsets
  U8                  Align;        // Alignment
  //
  // Data items used if memory devices are available and IMAGE_CF_MEMDEV has been set
  //
  #if GUI_SUPPORT_MEMDEV
    GUI_MEMDEV_Handle hMem;
    unsigned          Scale;
    unsigned          Angle;
    U8                Alpha;
    U8                IsDirty;
    //
    // Pointer to scaling function
    //
    void (* pFunc)(GUI_MEMDEV_Handle hSrc, GUI_MEMDEV_Handle hDst, int dx, int dy, int a, int Mag);
  #endif
} IMAGE_OBJ;

/*********************************************************************
*
*       Macros for internal use
*
**********************************************************************
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  #define IMAGE_INIT_ID(p) (p->Widget.DebugId = WIDGET_TYPE_IMAGE)
#else
  #define IMAGE_INIT_ID(p)
#endif

#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
  IMAGE_OBJ * IMAGE__LockH(IMAGE_Handle h);
  #define IMAGE_LOCK_H(h)   IMAGE__LockH(h)
#else
  #define IMAGE_LOCK_H(h)   (IMAGE_OBJ *)WM_LOCK_H(h)
#endif

/*********************************************************************
*
*       Module internal data
*
**********************************************************************
*/
extern IMAGE_PROPS IMAGE__DefaultProps;

/*********************************************************************
*
*       Private functions
*
**********************************************************************
*/
void IMAGE__SetWindowSize(IMAGE_Handle hObj);
void IMAGE__FreeAttached (IMAGE_Handle hObj, int LeaveTimer);
void IMAGE__SetVoid      (IMAGE_Handle hObj, const void * pData);

#endif // GUI_WINSUPPORT
#endif // IMAGE_PRIVATE_H

/*************************** End of file ****************************/
