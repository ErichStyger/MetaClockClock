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
File        : GUI_SVG_Global.h
Purpose     : Global public definitions used by SVG drivers
---------------------------END-OF-HEADER------------------------------
*/

#ifndef GUI_SVG_GLOBAL_H
#define GUI_SVG_GLOBAL_H

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Standard library fixed size types
*
**********************************************************************
*/
#ifdef WIN32
  #if (_MSC_VER <= 1900)                      // Older compiler version do not know about the header
    typedef char                 int8_t;
    typedef unsigned char        uint8_t;
    typedef short                int16_t;
    typedef unsigned short       uint16_t;
    typedef int                  int32_t;
    typedef unsigned int         uint32_t;
    #if (_MSC_VER > 1200)                     // VC6 does not support typedef (unsigned) long long 
      typedef long long          int64_t;
      typedef unsigned long long uint64_t;
    #endif
  #else
    #include <stdint.h>
  #endif
#else
  #include <stdint.h>
#endif

/*********************************************************************
*
*       Macros
*
**********************************************************************
*/
//
// Redirect 3rd party typename to the same type defined as GUI_...
//
#define REDIRECT_TYPE(TYPE_NAME)    typedef GUI_##TYPE_NAME TYPE_NAME
//
// Used to define a given 3rd party type with private members
// with the same size of given type if source is not available.
//
#define DEFINE_TYPE_PRIVATE(TYPE_NAME, NUM_BYTES)   \
  typedef struct {                                  \
    U32 a[NUM_BYTES / 4];                           \
  } GUI_##TYPE_NAME;                                \
  REDIRECT_TYPE(TYPE_NAME)
//
// Used to define a given 3rd party type with public members,
// required when the named members are used in the source file.
//
#define DEFINE_TYPE_PUBLIC(TYPE_NAME, MEMBERS)   \
  typedef struct {                               \
    MEMBERS                                      \
  } GUI_##TYPE_NAME;                             \
  REDIRECT_TYPE(TYPE_NAME)

//
// Stringify defines
//
#define SVG_STRINGIFY(x)   #x
#define SVG_MAKE_STR(x)    SVG_STRINGIFY(x)

#if defined(__cplusplus)
}
#endif

#endif /* GUI_SVG_GLOBAL_H */

/*************************** End of file ****************************/
