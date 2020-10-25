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
#if PL_MATRIX_CONFIG_IS_8x3
  #define MATRIX_NOF_BOARDS         (6)   /* total number of boards in matrix */
  #define MATRIX_NOF_CLOCKS_X       (8)   /* number of clocks in x (horizontal) direction */
  #define MATRIX_NOF_CLOCKS_Y       (3)   /* number of clocks in y (vertical) direction */
  #define MATRIX_NOF_CLOCKS_Z       (2)   /* number of clocks in z direction */
#elif PL_MATRIX_CONFIG_IS_12x5 && PL_CONFIG_IS_NEW_MODULAR
  #define MATRIX_NOF_BOARDS         (37)  /* total number of boards in matrix */
  #define MATRIX_NOF_CLOCKS_X       (12)  /* number of clocks in x (horizontal) direction */
  #define MATRIX_NOF_CLOCKS_Y       (5)   /* number of clocks in y (vertical) direction */
  #define MATRIX_NOF_CLOCKS_Z       (2)   /* number of clocks in z direction */
#elif PL_MATRIX_CONFIG_IS_12x5
  #define MATRIX_NOF_BOARDS         (15)  /* total number of boards in matrix */
  #define MATRIX_NOF_CLOCKS_X       (12)  /* number of clocks in x (horizontal) direction */
  #define MATRIX_NOF_CLOCKS_Y       (5)   /* number of clocks in y (vertical) direction */
  #define MATRIX_NOF_CLOCKS_Z       (2)   /* number of clocks in z direction */
#endif

/* no steppers on the master itself */
#define MATRIX_NOF_STEPPERS       (0)  /* number of stepper on board */

typedef enum {
#if PL_MATRIX_CONFIG_IS_8x3
  BOARD_ADDR_00=0x25, /* first column, top board */
  BOARD_ADDR_01=0x24,
  BOARD_ADDR_02=0x23,

  BOARD_ADDR_05=0x22, /* second column, top board */
  BOARD_ADDR_06=0x21,
  BOARD_ADDR_07=0x20,
#elif PL_MATRIX_CONFIG_IS_12x5 && PL_CONFIG_IS_NEW_MODULAR
  BOARD_ADDR_00=0x45, /* first column, top board */
  BOARD_ADDR_01=0x46,
  BOARD_ADDR_02=0x47,
  BOARD_ADDR_03=0x48,
  BOARD_ADDR_04=0x49,

  BOARD_ADDR_05=0x4A, /* second column, top board */
  BOARD_ADDR_06=0x4B,
  BOARD_ADDR_07=0x4C,
  BOARD_ADDR_08=0x4D,
  BOARD_ADDR_09=0x4E,

  BOARD_ADDR_10=0x53, /* third column, top row */
  BOARD_ADDR_11=0x52,
  BOARD_ADDR_12=0x51,
  BOARD_ADDR_13=0x50,
  BOARD_ADDR_14=0x4F,

  BOARD_ADDR_15=0x40, /* fourth column, top row */
  BOARD_ADDR_16=0x41,
  BOARD_ADDR_17=0x42,
  BOARD_ADDR_18=0x43,
  BOARD_ADDR_19=0x44,

  BOARD_ADDR_20=0x59, /* firth column, top row (single clocks) */
  BOARD_ADDR_21=0x5A,
  BOARD_ADDR_22=0x5B,
  BOARD_ADDR_23=0x5C,
  BOARD_ADDR_24=0x5D,

  BOARD_ADDR_25=0x54, /* firth column, top row (single clocks) */
  BOARD_ADDR_26=0x55,
  BOARD_ADDR_27=0x56,
  BOARD_ADDR_28=0x57,
  BOARD_ADDR_29=0x58,

  BOARD_ADDR_30=0x66, BOARD_ADDR_37=0x67, /* sixth column, top row (3 dual, 4 single clocks) */
  BOARD_ADDR_31=0x65,
  BOARD_ADDR_32=0x64,
  BOARD_ADDR_33=0x63, BOARD_ADDR_34=0x62,
  BOARD_ADDR_35=0x61, BOARD_ADDR_36=0x60,

#elif PL_MATRIX_CONFIG_IS_12x5
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
#else
  #error "unknown matrix"
#endif
} MATRIX_BOARD_ADDR_e;

typedef struct {
  uint8_t addr; /* RS-485 address */
  uint8_t nr;   /* clock number, 0..3 */
  struct {
    uint8_t x, y; /* coordinates on the board */
  } board;
  bool enabled; /* if enabled or not */
} MatrixClock_t;

extern const MatrixClock_t clockMatrix[MATRIX_NOF_CLOCKS_X][MATRIX_NOF_CLOCKS_Y];

typedef struct {
  bool enabled;  /* if board is enabled or not */
  uint8_t addr;  /* RS-485 address of the board */
  bool cmdSent;  /* if command has been sent or not to the board */
} MATRIX_BoardList_t;

extern MATRIX_BoardList_t MATRIX_BoardList[MATRIX_NOF_BOARDS];

#elif PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_LPC845_2X2
  #define MATRIX_NOF_CLOCKS_X       (PL_CONFIG_NOF_CLOCK_ON_BOARD_X)  /* number of clocks in x (horizontal) direction */
  #define MATRIX_NOF_CLOCKS_Y       (PL_CONFIG_NOF_CLOCK_ON_BOARD_Y)  /* number of clocks in y (vertical) direction */
  #define MATRIX_NOF_CLOCKS_Z       (PL_CONFIG_NOF_CLOCK_ON_BOARD_Z)  /* number of clocks in z direction */
  #define MATRIX_NOF_BOARDS         (1)  /* number of boards in matrix */
  #define MATRIX_NOF_STEPPERS       (MATRIX_NOF_CLOCKS_X*MATRIX_NOF_CLOCKS_Y*MATRIX_NOF_CLOCKS_Z)  /* number of stepper on board */
  #define MATRIX_STEPPER_MAPPING_X  {0, 1} /* map stepper on x position for boards. This reflects the X (horizontal) order of stepper */
#elif PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_LPC845_1X4
  #define MATRIX_NOF_CLOCKS_X       (PL_CONFIG_NOF_CLOCK_ON_BOARD_X)  /* number of clocks in x (horizontal) direction */
  #define MATRIX_NOF_CLOCKS_Y       (PL_CONFIG_NOF_CLOCK_ON_BOARD_Y)  /* number of clocks in y (vertical) direction */
  #define MATRIX_NOF_CLOCKS_Z       (PL_CONFIG_NOF_CLOCK_ON_BOARD_Z)  /* number of clocks in z direction */
  #define MATRIX_NOF_BOARDS         (1)  /* number of boards in matrix */
  #define MATRIX_NOF_STEPPERS       (MATRIX_NOF_CLOCKS_X*MATRIX_NOF_CLOCKS_Y*MATRIX_NOF_CLOCKS_Z)  /* number of stepper on board */
  #define MATRIX_STEPPER_MAPPING_X  {3, 2, 1, 0} /* map stepper on x position for boards. This reflects the X (horizontal) order of stepper */
#elif PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN64 || PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN128
  #define MATRIX_NOF_CLOCKS_X       (PL_CONFIG_NOF_CLOCK_ON_BOARD_X)  /* number of clocks in x (horizontal) direction */
  #define MATRIX_NOF_CLOCKS_Y       (PL_CONFIG_NOF_CLOCK_ON_BOARD_Y)  /* number of clocks in y (vertical) direction */
  #define MATRIX_NOF_CLOCKS_Z       (PL_CONFIG_NOF_CLOCK_ON_BOARD_Z)  /* number of clocks in z direction */
  #define MATRIX_NOF_BOARDS         (1)  /* number of boards in matrix */
  #define MATRIX_NOF_STEPPERS       (MATRIX_NOF_CLOCKS_X*MATRIX_NOF_CLOCKS_Y*MATRIX_NOF_CLOCKS_Z)  /* number of stepper on board */
  #if MATRIX_NOF_CLOCKS_X==1
    #define MATRIX_STEPPER_MAPPING_X  {0}  /* map stepper on x position for boards. This reflects the X (horizontal) order of stepper */
  #elif MATRIX_NOF_CLOCKS_X==2
    #define MATRIX_STEPPER_MAPPING_X  {0, 1}  /* map stepper on x position for boards. This reflects the X (horizontal) order of stepper */
  #else
    #error
  #endif
#endif /* PL_CONFIG_IS_MASTER */

#endif /* MATRIXCONFIG_H_ */
