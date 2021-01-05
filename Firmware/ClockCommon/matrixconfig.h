/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MATRIXCONFIG_H_
#define MATRIXCONFIG_H_

#include "platform.h"
#include <stdbool.h>
#include <stdint.h>

#if PL_CONFIG_IS_MASTER
/* configuration for master: */
#if PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_8x3
  #define MATRIX_NOF_BOARDS           (6)   /* total number of boards in matrix */
  #define MATRIX_NOF_STEPPERS_X       (8)   /* number of clocks in x (horizontal) direction */
  #define MATRIX_NOF_STEPPERS_Y       (3)   /* number of clocks in y (vertical) direction */
  #define MATRIX_NOF_STEPPERS_Z       (2)   /* number of clocks in z direction */
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_12x5_MOD
  #define MATRIX_NOF_BOARDS           (38)  /* total number of boards in matrix */
  #define MATRIX_NOF_STEPPERS_X       (12)  /* number of clocks in x (horizontal) direction */
  #define MATRIX_NOF_STEPPERS_Y       (5)   /* number of clocks in y (vertical) direction */
  #define MATRIX_NOF_STEPPERS_Z       (2)   /* number of clocks in z direction */
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_12x5_60B
  #define MATRIX_NOF_BOARDS           (15)  /* total number of boards in matrix */
  #define MATRIX_NOF_STEPPERS_X       (12)  /* number of clocks in x (horizontal) direction */
  #define MATRIX_NOF_STEPPERS_Y       (5)   /* number of clocks in y (vertical) direction */
  #define MATRIX_NOF_STEPPERS_Z       (2)   /* number of clocks in z direction */
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_16x9_ALEXIS
  #define MATRIX_NOF_BOARDS           (4*9)  /* total number of boards in matrix */
  #define MATRIX_NOF_STEPPERS_X       (16)  /* number of clocks in x (horizontal) direction */
  #define MATRIX_NOF_STEPPERS_Y       (9)   /* number of clocks in y (vertical) direction */
  #define MATRIX_NOF_STEPPERS_Z       (2)   /* number of clocks in z direction */
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_SMARTWALL_8x5
  #define MATRIX_NOF_BOARDS           (1)   /* total number of boards in matrix */
  #define MATRIX_NOF_STEPPERS_X       (8)   /* number of clocks in x (horizontal) direction */
  #define MATRIX_NOF_STEPPERS_Y       (1)   /* number of clocks in y (vertical) direction */
  #define MATRIX_NOF_STEPPERS_Z       (1)   /* number of clocks in z direction */
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_8x3_V4
  #define MATRIX_NOF_BOARDS           (12)   /* total number of boards in matrix */
  #define MATRIX_NOF_STEPPERS_X       (8)   /* number of clocks in x (horizontal) direction */
  #define MATRIX_NOF_STEPPERS_Y       (3)   /* number of clocks in y (vertical) direction */
  #define MATRIX_NOF_STEPPERS_Z       (2)   /* number of clocks in z direction */
#else
  #error "unknown configuration"
#endif

/* no steppers on the master itself */
#define MATRIX_NOF_STEPPERS       (0)  /* number of stepper on board: master does not have stepper motors */

typedef enum MATRIX_BOARD_ADDR_e {
#if PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_8x3
  BOARD_ADDR_00=0x25, /* first column, top board */
  BOARD_ADDR_01=0x24,
  BOARD_ADDR_02=0x23,

  BOARD_ADDR_05=0x22, /* second column, top board */
  BOARD_ADDR_06=0x21,
  BOARD_ADDR_07=0x20,
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_12x5_MOD
  BOARD_ADDR_00=0x45, /* column 1+2, top board */
  BOARD_ADDR_01=0x46,
  BOARD_ADDR_02=0x47,
  BOARD_ADDR_03=0x48,
  BOARD_ADDR_04=0x49,

  BOARD_ADDR_05=0x4A, /* column 3+4, top board */
  BOARD_ADDR_06=0x4B,
  BOARD_ADDR_07=0x4C,
  BOARD_ADDR_08=0x4D,
  BOARD_ADDR_09=0x4E,

  BOARD_ADDR_10=0x53, /* column 5+6, top row */
  BOARD_ADDR_11=0x52,
  BOARD_ADDR_12=0x51,
  BOARD_ADDR_13=0x50,
  BOARD_ADDR_14=0x4F,

  BOARD_ADDR_15=0x40, /* column 7+8, top row */
  BOARD_ADDR_16=0x41,
  BOARD_ADDR_17=0x42,
  BOARD_ADDR_18=0x43,
  BOARD_ADDR_19=0x44,

  BOARD_ADDR_20=0x59, /* column 9, top row (single clocks) */
  BOARD_ADDR_21=0x5A,
  BOARD_ADDR_22=0x5B,
  BOARD_ADDR_23=0x5C,
  BOARD_ADDR_24=0x5D,

  BOARD_ADDR_25=0x54, /* column 19, top row (single clocks) */
  BOARD_ADDR_26=0x55,
  BOARD_ADDR_27=0x56,
  BOARD_ADDR_28=0x57,
  BOARD_ADDR_29=0x58,

  BOARD_ADDR_30=0x66, BOARD_ADDR_37=0x67, /* column 11+12, (single, dual clocks) */
  BOARD_ADDR_31=0x65,
  BOARD_ADDR_32=0x64,
  BOARD_ADDR_33=0x63, BOARD_ADDR_34=0x62,
  BOARD_ADDR_35=0x61, BOARD_ADDR_36=0x60,

#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_12x5_60B
  BOARD_ADDR_00=0x20, /* first column, top board */
  BOARD_ADDR_01=0x21,
  BOARD_ADDR_02=0x22,
  BOARD_ADDR_03=0x23,
  BOARD_ADDR_04=0x24,

  BOARD_ADDR_05=0x25, /* second column, top board */
  BOARD_ADDR_06=0x26,
  BOARD_ADDR_07=0x27,
  BOARD_ADDR_08=0x28,
  BOARD_ADDR_09=0x15,

  BOARD_ADDR_10=0x10, /* third column, top row */
  BOARD_ADDR_11=0x11,
  BOARD_ADDR_12=0x12,
  BOARD_ADDR_13=0x13,
  BOARD_ADDR_14=0x14,
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_16x9_ALEXIS
  BOARD_ADDR_00=0x18, /* first column, top board */
  BOARD_ADDR_01=0x17,
  BOARD_ADDR_02=0x16,
  BOARD_ADDR_03=0x15,
  BOARD_ADDR_04=0x14,
  BOARD_ADDR_05=0x13,
  BOARD_ADDR_06=0x12,
  BOARD_ADDR_07=0x11,
  BOARD_ADDR_08=0x10,

  BOARD_ADDR_09=0x27, /* second column, top board */
  BOARD_ADDR_10=0x26,
  BOARD_ADDR_11=0x25,
  BOARD_ADDR_12=0x24,
  BOARD_ADDR_13=0x23,
  BOARD_ADDR_14=0x22,
  BOARD_ADDR_15=0x21,
  BOARD_ADDR_16=0x20,
  BOARD_ADDR_17=0x19,

  BOARD_ADDR_18=0x36, /* third column, top row */
  BOARD_ADDR_19=0x35,
  BOARD_ADDR_20=0x34,
  BOARD_ADDR_21=0x33,
  BOARD_ADDR_22=0x32,
  BOARD_ADDR_23=0x31,
  BOARD_ADDR_24=0x30,
  BOARD_ADDR_25=0x29,
  BOARD_ADDR_26=0x28,

  BOARD_ADDR_27=0x45, /* fourth column, top row */
  BOARD_ADDR_28=0x44,
  BOARD_ADDR_29=0x43,
  BOARD_ADDR_30=0x42,
  BOARD_ADDR_31=0x41,
  BOARD_ADDR_32=0x40,
  BOARD_ADDR_33=0x39,
  BOARD_ADDR_34=0x38,
  BOARD_ADDR_35=0x37,
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_SMARTWALL_8x5
  BOARD_ADDR_00=0x10,
  /*
  BOARD_ADDR_01=0x11,
  BOARD_ADDR_02=0x12,
  BOARD_ADDR_03=0x13,
  BOARD_ADDR_04=0x14, */
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_8x3_V4
  BOARD_ADDR_00=0x70,
  BOARD_ADDR_01=0x71,
  BOARD_ADDR_02=0x72,
  BOARD_ADDR_03=0x73,
  BOARD_ADDR_04=0x74,
  BOARD_ADDR_05=0x75,
  BOARD_ADDR_06=0x76,
  BOARD_ADDR_07=0x77,
  BOARD_ADDR_08=0x78,
  BOARD_ADDR_09=0x79,
  BOARD_ADDR_10=0x7A,
  BOARD_ADDR_11=0x7B,
#else
  #error "unknown matrix"
#endif
} MATRIX_BOARD_ADDR_e;

typedef struct MatrixClock_t {
  uint8_t addr; /* RS-485 address */
  struct {
    uint8_t x, y, z; /* coordinates on the board */
  } board;
} MatrixClock_t;

extern const MatrixClock_t clockMatrix[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z];

typedef struct MATRIX_BoardList_t {
  uint8_t addr;  /* RS-485 address of the board */
  bool cmdSent;  /* if command has been sent or not to the board */
} MATRIX_BoardList_t;

extern MATRIX_BoardList_t MATRIX_BoardList[MATRIX_NOF_BOARDS];
#endif /* PL_CONFIG_IS_MASTER */

#if PL_CONFIG_IS_CLIENT
  /* configurations of (slave) stepper motor boards */
  #if PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_LPC845_2X2
    #define MATRIX_NOF_STEPPERS_X       (PL_CONFIG_NOF_STEPPER_ON_BOARD_X)  /* number of clocks in x (horizontal) direction */
    #define MATRIX_NOF_STEPPERS_Y       (PL_CONFIG_NOF_STEPPER_ON_BOARD_Y)  /* number of clocks in y (vertical) direction */
    #define MATRIX_NOF_STEPPERS_Z       (PL_CONFIG_NOF_STEPPER_ON_BOARD_Z)  /* number of clocks in z direction */
    #define MATRIX_NOF_BOARDS         (1)  /* number of boards in matrix */
  #elif PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_LPC845_1X4
    #define MATRIX_NOF_STEPPERS_X       (PL_CONFIG_NOF_STEPPER_ON_BOARD_X)  /* number of clocks in x (horizontal) direction */
    #define MATRIX_NOF_STEPPERS_Y       (PL_CONFIG_NOF_STEPPER_ON_BOARD_Y)  /* number of clocks in y (vertical) direction */
    #define MATRIX_NOF_STEPPERS_Z       (PL_CONFIG_NOF_STEPPER_ON_BOARD_Z)  /* number of clocks in z direction */
    #define MATRIX_NOF_BOARDS         (1)  /* number of boards in matrix */
  #elif PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN64 || PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN128
    #define MATRIX_NOF_STEPPERS_X       (PL_CONFIG_NOF_STEPPER_ON_BOARD_X)  /* number of clocks in x (horizontal) direction */
    #define MATRIX_NOF_STEPPERS_Y       (PL_CONFIG_NOF_STEPPER_ON_BOARD_Y)  /* number of clocks in y (vertical) direction */
    #define MATRIX_NOF_STEPPERS_Z       (PL_CONFIG_NOF_STEPPER_ON_BOARD_Z)  /* number of clocks in z direction */
    #define MATRIX_NOF_BOARDS         (1)  /* number of boards in matrix */
  #elif PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_PIXELUNIT_K02FN64 || PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_PIXELUNIT_K02FN128
    #define MATRIX_NOF_STEPPERS_X       (PL_CONFIG_NOF_STEPPER_ON_BOARD_X)  /* number of clocks in x (horizontal) direction */
    #define MATRIX_NOF_STEPPERS_Y       (PL_CONFIG_NOF_STEPPER_ON_BOARD_Y)  /* number of clocks in y (vertical) direction */
    #define MATRIX_NOF_STEPPERS_Z       (PL_CONFIG_NOF_STEPPER_ON_BOARD_Z)  /* number of clocks in z direction */
    #define MATRIX_NOF_BOARDS           (1)  /* number of boards in board matrix */
  #endif

  #define MATRIX_NOF_STEPPERS       (MATRIX_NOF_STEPPERS_X*MATRIX_NOF_STEPPERS_Y*MATRIX_NOF_STEPPERS_Z)
#endif /* PL_CONFIG_IS_CLIENT */

#endif /* MATRIXCONFIG_H_ */
