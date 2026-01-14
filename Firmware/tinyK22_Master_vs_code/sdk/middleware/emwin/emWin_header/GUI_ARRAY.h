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
File        : GUI_ARRAY.h
Purpose     : Array handling routines
---------------------------END-OF-HEADER------------------------------
*/

#ifndef GUI_ARRAY_H
#define GUI_ARRAY_H

#include "GUI_Private.h"

/*********************************************************************
*
*       Public types
*
**********************************************************************
*/
typedef GUI_HMEM GUI_ARRAY;

typedef struct {
  void   ** ppItem;
  unsigned  i;
  GUI_ARRAY hArray;
} GUI_ARRAY_ITERATOR;

typedef void (GUI_ARRAY_ITERATE_CALLBACK)   (GUI_ARRAY hArray, unsigned i, void * pItem);
typedef int  (GUI_ARRAY_ITERATE_EX_CALLBACK)(GUI_ARRAY hArray, unsigned i, void * pItem, void * pUser);

/*********************************************************************
*
*       Public functions
*
**********************************************************************
*/
GUI_ARRAY GUI_ARRAY_Create          (void);
int       GUI_ARRAY_AddItem         (GUI_ARRAY hArray, const void * pNew, int Len);
void      GUI_ARRAY_Delete          (GUI_ARRAY hArray);
void      GUI_ARRAY_DeletePtr       (GUI_ARRAY * phArray);
GUI_HMEM  GUI_ARRAY_GethItem        (GUI_ARRAY hArray, unsigned int Index);
GUI_HMEM  GUI_ARRAY_GetLasthItem    (GUI_ARRAY hArray);
unsigned  GUI_ARRAY_GetNumItems     (GUI_ARRAY hArray);
void    * GUI_ARRAY_GetpItemLocked  (GUI_ARRAY hArray, unsigned int Index);
int       GUI_ARRAY_SethItem        (GUI_ARRAY hArray, unsigned int Index, GUI_HMEM hItem);
GUI_HMEM  GUI_ARRAY_SetItem         (GUI_ARRAY hArray, unsigned int Index, const void * pData, int Len);
void      GUI_ARRAY_DeleteItem      (GUI_ARRAY hArray, unsigned int Index);
char      GUI_ARRAY_InsertBlankItem (GUI_ARRAY hArray, unsigned int Index);
GUI_HMEM  GUI_ARRAY_InsertItem      (GUI_ARRAY hArray, unsigned int Index, int Len);
void    * GUI_ARRAY_ResizeItemLocked(GUI_ARRAY hArray, unsigned int Index, int Len);
void      GUI_ARRAY_BeginIterator   (GUI_ARRAY hArray, void ** ppItem, GUI_ARRAY_ITERATOR * pIterator);
void    * GUI_ARRAY_GetNextItem     (GUI_ARRAY_ITERATOR * pIterator);
void      GUI_ARRAY_EndIterator     (GUI_ARRAY_ITERATOR * pIterator);
void      GUI_ARRAY_Iterate         (GUI_ARRAY hArray, GUI_ARRAY_ITERATE_CALLBACK * cbIterate);
void      GUI_ARRAY_IterateEx       (GUI_ARRAY hArray, GUI_ARRAY_ITERATE_EX_CALLBACK * cbIterate, void * pUser);

#endif /* GUI_ARRAY_H */

/*************************** End of file ****************************/
