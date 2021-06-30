/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "matrixconfig.h"
#include "matrix.h"

#if PL_CONFIG_IS_MASTER

#if PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_8x3
  const MatrixClock_t clockMatrix[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z] = /* information about how the clocks are organized */
  {
    [0][0][0]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=0},  [0][0][1]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=1},
    [0][1][0]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=0},  [0][1][1]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=1},
    [0][2][0]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=0},  [0][2][1]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=1},

    [1][0][0]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=0},  [1][0][1]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=1},
    [1][1][0]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=0},  [1][1][1]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=1},
    [1][2][0]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=0},  [1][2][1]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=1},

    [2][0][0]={.addr=BOARD_ADDR_00, .board.x=2, .board.y=0, .board.z=0},  [2][0][1]={.addr=BOARD_ADDR_00, .board.x=2, .board.y=0, .board.z=1},
    [2][1][0]={.addr=BOARD_ADDR_01, .board.x=2, .board.y=0, .board.z=0},  [2][1][1]={.addr=BOARD_ADDR_01, .board.x=2, .board.y=0, .board.z=1},
    [2][2][0]={.addr=BOARD_ADDR_02, .board.x=2, .board.y=0, .board.z=0},  [2][2][1]={.addr=BOARD_ADDR_02, .board.x=2, .board.y=0, .board.z=1},

    [3][0][0]={.addr=BOARD_ADDR_00, .board.x=3, .board.y=0, .board.z=0},  [3][0][1]={.addr=BOARD_ADDR_00, .board.x=3, .board.y=0, .board.z=1},
    [3][1][0]={.addr=BOARD_ADDR_01, .board.x=3, .board.y=0, .board.z=0},  [3][1][1]={.addr=BOARD_ADDR_01, .board.x=3, .board.y=0, .board.z=1},
    [3][2][0]={.addr=BOARD_ADDR_02, .board.x=3, .board.y=0, .board.z=0},  [3][2][1]={.addr=BOARD_ADDR_02, .board.x=3, .board.y=0, .board.z=1},

    [4][0][0]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=0},  [4][0][1]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=1},
    [4][1][0]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=0},  [4][1][1]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=1},
    [4][2][0]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=0},  [4][2][1]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=1},

    [5][0][0]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=0},  [5][0][1]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=1},
    [5][1][0]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=0},  [5][1][1]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=1},
    [5][2][0]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=0},  [5][2][1]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=1},

    [6][0][0]={.addr=BOARD_ADDR_05, .board.x=2, .board.y=0, .board.z=0},  [6][0][1]={.addr=BOARD_ADDR_05, .board.x=2, .board.y=0, .board.z=1},
    [6][1][0]={.addr=BOARD_ADDR_06, .board.x=2, .board.y=0, .board.z=0},  [6][1][1]={.addr=BOARD_ADDR_06, .board.x=2, .board.y=0, .board.z=1},
    [6][2][0]={.addr=BOARD_ADDR_07, .board.x=2, .board.y=0, .board.z=0},  [6][2][1]={.addr=BOARD_ADDR_07, .board.x=2, .board.y=0, .board.z=1},

    [7][0][0]={.addr=BOARD_ADDR_05, .board.x=3, .board.y=0, .board.z=0},  [7][0][1]={.addr=BOARD_ADDR_05, .board.x=3, .board.y=0, .board.z=1},
    [7][1][0]={.addr=BOARD_ADDR_06, .board.x=3, .board.y=0, .board.z=0},  [7][1][1]={.addr=BOARD_ADDR_06, .board.x=3, .board.y=0, .board.z=1},
    [7][2][0]={.addr=BOARD_ADDR_07, .board.x=3, .board.y=0, .board.z=0},  [7][2][1]={.addr=BOARD_ADDR_07, .board.x=3, .board.y=0, .board.z=1},
  };

  MATRIX_BoardList_t MATRIX_BoardList[MATRIX_NOF_BOARDS] = {
      {.addr=BOARD_ADDR_00, .cmdSent=false},
      {.addr=BOARD_ADDR_01, .cmdSent=false},
      {.addr=BOARD_ADDR_02, .cmdSent=false},

      {.addr=BOARD_ADDR_05, .cmdSent=false},
      {.addr=BOARD_ADDR_06, .cmdSent=false},
      {.addr=BOARD_ADDR_07, .cmdSent=false},
  };
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_12x5_MOD
  const MatrixClock_t clockMatrix[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z] = /* information about how the clocks are organized */
  {
    /* 1st column */
    [0][0][0]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=0}, [0][0][1]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=1},
    [0][1][0]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=0}, [0][1][1]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=1},
    [0][2][0]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=0}, [0][2][1]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=1},
    [0][3][0]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=0}, [0][3][1]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=1},
    [0][4][0]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=0}, [0][4][1]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=1},

    [1][0][0]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=0}, [1][0][1]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=1},
    [1][1][0]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=0}, [1][1][1]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=1},
    [1][2][0]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=0}, [1][2][1]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=1},
    [1][3][0]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=0}, [1][3][1]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=1},
    [1][4][0]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=0}, [1][4][1]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=1},

    [2][0][0]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=0}, [2][0][1]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=1},
    [2][1][0]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=0}, [2][1][1]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=1},
    [2][2][0]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=0}, [2][2][1]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=1},
    [2][3][0]={.addr=BOARD_ADDR_08, .board.x=0, .board.y=0, .board.z=0}, [2][3][1]={.addr=BOARD_ADDR_08, .board.x=0, .board.y=0, .board.z=1},
    [2][4][0]={.addr=BOARD_ADDR_09, .board.x=0, .board.y=0, .board.z=0}, [2][4][1]={.addr=BOARD_ADDR_09, .board.x=0, .board.y=0, .board.z=1},

    [3][0][0]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=0}, [3][0][1]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=1},
    [3][1][0]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=0}, [3][1][1]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=1},
    [3][2][0]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=0}, [3][2][1]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=1},
    [3][3][0]={.addr=BOARD_ADDR_08, .board.x=1, .board.y=0, .board.z=0}, [3][3][1]={.addr=BOARD_ADDR_08, .board.x=1, .board.y=0, .board.z=1},
    [3][4][0]={.addr=BOARD_ADDR_09, .board.x=1, .board.y=0, .board.z=0}, [3][4][1]={.addr=BOARD_ADDR_09, .board.x=1, .board.y=0, .board.z=1},

    [4][0][0]={.addr=BOARD_ADDR_10, .board.x=0, .board.y=0, .board.z=0}, [4][0][1]={.addr=BOARD_ADDR_10, .board.x=0, .board.y=0, .board.z=1},
    [4][1][0]={.addr=BOARD_ADDR_11, .board.x=0, .board.y=0, .board.z=0}, [4][1][1]={.addr=BOARD_ADDR_11, .board.x=0, .board.y=0, .board.z=1},
    [4][2][0]={.addr=BOARD_ADDR_12, .board.x=0, .board.y=0, .board.z=0}, [4][2][1]={.addr=BOARD_ADDR_12, .board.x=0, .board.y=0, .board.z=1},
    [4][3][0]={.addr=BOARD_ADDR_13, .board.x=0, .board.y=0, .board.z=0}, [4][3][1]={.addr=BOARD_ADDR_13, .board.x=0, .board.y=0, .board.z=1},
    [4][4][0]={.addr=BOARD_ADDR_14, .board.x=0, .board.y=0, .board.z=0}, [4][4][1]={.addr=BOARD_ADDR_14, .board.x=0, .board.y=0, .board.z=1},

    [5][0][0]={.addr=BOARD_ADDR_10, .board.x=1, .board.y=0, .board.z=0}, [5][0][1]={.addr=BOARD_ADDR_10, .board.x=1, .board.y=0, .board.z=1},
    [5][1][0]={.addr=BOARD_ADDR_11, .board.x=1, .board.y=0, .board.z=0}, [5][1][1]={.addr=BOARD_ADDR_11, .board.x=1, .board.y=0, .board.z=1},
    [5][2][0]={.addr=BOARD_ADDR_12, .board.x=1, .board.y=0, .board.z=0}, [5][2][1]={.addr=BOARD_ADDR_12, .board.x=1, .board.y=0, .board.z=1},
    [5][3][0]={.addr=BOARD_ADDR_13, .board.x=1, .board.y=0, .board.z=0}, [5][3][1]={.addr=BOARD_ADDR_13, .board.x=1, .board.y=0, .board.z=1},
    [5][4][0]={.addr=BOARD_ADDR_14, .board.x=1, .board.y=0, .board.z=0}, [5][4][1]={.addr=BOARD_ADDR_14, .board.x=1, .board.y=0, .board.z=1},

    [6][0][0]={.addr=BOARD_ADDR_15, .board.x=0, .board.y=0, .board.z=0}, [6][0][1]={.addr=BOARD_ADDR_15, .board.x=0, .board.y=0, .board.z=1},
    [6][1][0]={.addr=BOARD_ADDR_16, .board.x=0, .board.y=0, .board.z=0}, [6][1][1]={.addr=BOARD_ADDR_16, .board.x=0, .board.y=0, .board.z=1},
    [6][2][0]={.addr=BOARD_ADDR_17, .board.x=0, .board.y=0, .board.z=0}, [6][2][1]={.addr=BOARD_ADDR_17, .board.x=0, .board.y=0, .board.z=1},
    [6][3][0]={.addr=BOARD_ADDR_18, .board.x=0, .board.y=0, .board.z=0}, [6][3][1]={.addr=BOARD_ADDR_18, .board.x=0, .board.y=0, .board.z=1},
    [6][4][0]={.addr=BOARD_ADDR_19, .board.x=0, .board.y=0, .board.z=0}, [6][4][1]={.addr=BOARD_ADDR_19, .board.x=0, .board.y=0, .board.z=1},

    [7][0][0]={.addr=BOARD_ADDR_15, .board.x=1, .board.y=0, .board.z=0}, [7][0][1]={.addr=BOARD_ADDR_15, .board.x=1, .board.y=0, .board.z=1},
    [7][1][0]={.addr=BOARD_ADDR_16, .board.x=1, .board.y=0, .board.z=0}, [7][1][1]={.addr=BOARD_ADDR_16, .board.x=1, .board.y=0, .board.z=1},
    [7][2][0]={.addr=BOARD_ADDR_17, .board.x=1, .board.y=0, .board.z=0}, [7][2][1]={.addr=BOARD_ADDR_17, .board.x=1, .board.y=0, .board.z=1},
    [7][3][0]={.addr=BOARD_ADDR_18, .board.x=1, .board.y=0, .board.z=0}, [7][3][1]={.addr=BOARD_ADDR_18, .board.x=1, .board.y=0, .board.z=1},
    [7][4][0]={.addr=BOARD_ADDR_19, .board.x=1, .board.y=0, .board.z=0}, [7][4][1]={.addr=BOARD_ADDR_19, .board.x=1, .board.y=0, .board.z=1},

    [8][0][0]={.addr=BOARD_ADDR_20, .board.x=0, .board.y=0, .board.z=0}, [8][0][1]={.addr=BOARD_ADDR_20, .board.x=0, .board.y=0, .board.z=1},
    [8][1][0]={.addr=BOARD_ADDR_21, .board.x=0, .board.y=0, .board.z=0}, [8][1][1]={.addr=BOARD_ADDR_21, .board.x=0, .board.y=0, .board.z=1},
    [8][2][0]={.addr=BOARD_ADDR_22, .board.x=0, .board.y=0, .board.z=0}, [8][2][1]={.addr=BOARD_ADDR_22, .board.x=0, .board.y=0, .board.z=1},
    [8][3][0]={.addr=BOARD_ADDR_23, .board.x=0, .board.y=0, .board.z=0}, [8][3][1]={.addr=BOARD_ADDR_23, .board.x=0, .board.y=0, .board.z=1},
    [8][4][0]={.addr=BOARD_ADDR_24, .board.x=0, .board.y=0, .board.z=0}, [8][4][1]={.addr=BOARD_ADDR_24, .board.x=0, .board.y=0, .board.z=1},

    [9][0][0]={.addr=BOARD_ADDR_25, .board.x=0, .board.y=0, .board.z=0}, [9][0][1]={.addr=BOARD_ADDR_25, .board.x=0, .board.y=0, .board.z=1},
    [9][1][0]={.addr=BOARD_ADDR_26, .board.x=0, .board.y=0, .board.z=0}, [9][1][1]={.addr=BOARD_ADDR_26, .board.x=0, .board.y=0, .board.z=1},
    [9][2][0]={.addr=BOARD_ADDR_27, .board.x=0, .board.y=0, .board.z=0}, [9][2][1]={.addr=BOARD_ADDR_27, .board.x=0, .board.y=0, .board.z=1},
    [9][3][0]={.addr=BOARD_ADDR_28, .board.x=0, .board.y=0, .board.z=0}, [9][3][1]={.addr=BOARD_ADDR_28, .board.x=0, .board.y=0, .board.z=1},
    [9][4][0]={.addr=BOARD_ADDR_29, .board.x=0, .board.y=0, .board.z=0}, [9][4][1]={.addr=BOARD_ADDR_29, .board.x=0, .board.y=0, .board.z=1},

    [10][0][0]={.addr=BOARD_ADDR_30, .board.x=0, .board.y=0, .board.z=0},[10][0][1]={.addr=BOARD_ADDR_30, .board.x=0, .board.y=0, .board.z=1},
    [10][1][0]={.addr=BOARD_ADDR_31, .board.x=0, .board.y=0, .board.z=0},[10][1][1]={.addr=BOARD_ADDR_31, .board.x=0, .board.y=0, .board.z=1},
    [10][2][0]={.addr=BOARD_ADDR_32, .board.x=0, .board.y=0, .board.z=0},[10][2][1]={.addr=BOARD_ADDR_32, .board.x=0, .board.y=0, .board.z=1},
    [10][3][0]={.addr=BOARD_ADDR_33, .board.x=0, .board.y=0, .board.z=0},[10][3][1]={.addr=BOARD_ADDR_33, .board.x=0, .board.y=0, .board.z=1},
    [10][4][0]={.addr=BOARD_ADDR_35, .board.x=0, .board.y=0, .board.z=0},[10][4][1]={.addr=BOARD_ADDR_35, .board.x=0, .board.y=0, .board.z=1},

    [11][0][0]={.addr=BOARD_ADDR_37, .board.x=0, .board.y=0, .board.z=0},[11][0][1]={.addr=BOARD_ADDR_37, .board.x=0, .board.y=0, .board.z=1},
    [11][1][0]={.addr=BOARD_ADDR_31, .board.x=1, .board.y=0, .board.z=0},[11][1][1]={.addr=BOARD_ADDR_31, .board.x=1, .board.y=0, .board.z=1},
    [11][2][0]={.addr=BOARD_ADDR_32, .board.x=1, .board.y=0, .board.z=0},[11][2][1]={.addr=BOARD_ADDR_32, .board.x=1, .board.y=0, .board.z=1},
    [11][3][0]={.addr=BOARD_ADDR_34, .board.x=0, .board.y=0, .board.z=0},[11][3][1]={.addr=BOARD_ADDR_34, .board.x=0, .board.y=0, .board.z=1},
    [11][4][0]={.addr=BOARD_ADDR_36, .board.x=0, .board.y=0, .board.z=0},[11][4][1]={.addr=BOARD_ADDR_36, .board.x=0, .board.y=0, .board.z=1},
};

  MATRIX_BoardList_t MATRIX_BoardList[MATRIX_NOF_BOARDS] = {
      {.addr=BOARD_ADDR_00, .cmdSent=false},
      {.addr=BOARD_ADDR_01, .cmdSent=false},
      {.addr=BOARD_ADDR_02, .cmdSent=false},
      {.addr=BOARD_ADDR_03, .cmdSent=false},
      {.addr=BOARD_ADDR_04, .cmdSent=false},

      {.addr=BOARD_ADDR_05, .cmdSent=false},
      {.addr=BOARD_ADDR_06, .cmdSent=false},
      {.addr=BOARD_ADDR_07, .cmdSent=false},
      {.addr=BOARD_ADDR_08, .cmdSent=false},
      {.addr=BOARD_ADDR_09, .cmdSent=false},

      {.addr=BOARD_ADDR_10, .cmdSent=false},
      {.addr=BOARD_ADDR_11, .cmdSent=false},
      {.addr=BOARD_ADDR_12, .cmdSent=false},
      {.addr=BOARD_ADDR_13, .cmdSent=false},
      {.addr=BOARD_ADDR_14, .cmdSent=false},

      {.addr=BOARD_ADDR_15, .cmdSent=false},
      {.addr=BOARD_ADDR_16, .cmdSent=false},
      {.addr=BOARD_ADDR_17, .cmdSent=false},
      {.addr=BOARD_ADDR_18, .cmdSent=false},
      {.addr=BOARD_ADDR_19, .cmdSent=false},

      {.addr=BOARD_ADDR_20, .cmdSent=false},
      {.addr=BOARD_ADDR_21, .cmdSent=false},
      {.addr=BOARD_ADDR_22, .cmdSent=false},
      {.addr=BOARD_ADDR_23, .cmdSent=false},
      {.addr=BOARD_ADDR_24, .cmdSent=false},

      {.addr=BOARD_ADDR_25, .cmdSent=false},
      {.addr=BOARD_ADDR_26, .cmdSent=false},
      {.addr=BOARD_ADDR_27, .cmdSent=false},
      {.addr=BOARD_ADDR_28, .cmdSent=false},
      {.addr=BOARD_ADDR_29, .cmdSent=false},

      {.addr=BOARD_ADDR_30, .cmdSent=false},      {.addr=BOARD_ADDR_37, .cmdSent=false},
      {.addr=BOARD_ADDR_31, .cmdSent=false},
      {.addr=BOARD_ADDR_32, .cmdSent=false},
      {.addr=BOARD_ADDR_33, .cmdSent=false},      {.addr=BOARD_ADDR_34, .cmdSent=false},
      {.addr=BOARD_ADDR_35, .cmdSent=false},      {.addr=BOARD_ADDR_36, .cmdSent=false},
};
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_12x5_60B
  const MatrixClock_t clockMatrix[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z] = /* information about how the clocks are organized */
  {
    [0][0][0]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=0},  [0][0][1]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=1},
    [0][1][0]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=0},  [0][1][1]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=1},
    [0][2][0]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=0},  [0][2][1]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=1},
    [0][3][0]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=0},  [0][3][1]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=1},
    [0][4][0]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=0},  [0][4][1]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=1},

    [1][0][0]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=0},  [1][0][1]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=1},
    [1][1][0]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=0},  [1][1][1]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=1},
    [1][2][0]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=0},  [1][2][1]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=1},
    [1][3][0]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=0},  [1][3][1]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=1},
    [1][4][0]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=0},  [1][4][1]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=1},

    [2][0][0]={.addr=BOARD_ADDR_00, .board.x=2, .board.y=0, .board.z=0},  [2][0][1]={.addr=BOARD_ADDR_00, .board.x=2, .board.y=0, .board.z=1},
    [2][1][0]={.addr=BOARD_ADDR_01, .board.x=2, .board.y=0, .board.z=0},  [2][1][1]={.addr=BOARD_ADDR_01, .board.x=2, .board.y=0, .board.z=1},
    [2][2][0]={.addr=BOARD_ADDR_02, .board.x=2, .board.y=0, .board.z=0},  [2][2][1]={.addr=BOARD_ADDR_02, .board.x=2, .board.y=0, .board.z=1},
    [2][3][0]={.addr=BOARD_ADDR_03, .board.x=2, .board.y=0, .board.z=0},  [2][3][1]={.addr=BOARD_ADDR_03, .board.x=2, .board.y=0, .board.z=1},
    [2][4][0]={.addr=BOARD_ADDR_04, .board.x=2, .board.y=0, .board.z=0},  [2][4][1]={.addr=BOARD_ADDR_04, .board.x=2, .board.y=0, .board.z=1},

    [3][0][0]={.addr=BOARD_ADDR_00, .board.x=3, .board.y=0, .board.z=0},  [3][0][1]={.addr=BOARD_ADDR_00, .board.x=3, .board.y=0, .board.z=1},
    [3][1][0]={.addr=BOARD_ADDR_01, .board.x=3, .board.y=0, .board.z=0},  [3][1][1]={.addr=BOARD_ADDR_01, .board.x=3, .board.y=0, .board.z=1},
    [3][2][0]={.addr=BOARD_ADDR_02, .board.x=3, .board.y=0, .board.z=0},  [3][2][1]={.addr=BOARD_ADDR_02, .board.x=3, .board.y=0, .board.z=1},
    [3][3][0]={.addr=BOARD_ADDR_03, .board.x=3, .board.y=0, .board.z=0},  [3][3][1]={.addr=BOARD_ADDR_03, .board.x=3, .board.y=0, .board.z=1},
    [3][4][0]={.addr=BOARD_ADDR_04, .board.x=3, .board.y=0, .board.z=0},  [3][4][1]={.addr=BOARD_ADDR_04, .board.x=3, .board.y=0, .board.z=1},

    [4][0][0]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=0},  [4][0][1]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=1},
    [4][1][0]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=0},  [4][1][1]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=1},
    [4][2][0]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=0},  [4][2][1]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=1},
    [4][3][0]={.addr=BOARD_ADDR_08, .board.x=0, .board.y=0, .board.z=0},  [4][3][1]={.addr=BOARD_ADDR_08, .board.x=0, .board.y=0, .board.z=1},
    [4][4][0]={.addr=BOARD_ADDR_09, .board.x=0, .board.y=0, .board.z=0},  [4][4][1]={.addr=BOARD_ADDR_09, .board.x=0, .board.y=0, .board.z=1},

    [5][0][0]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=0},  [5][0][1]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=1},
    [5][1][0]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=0},  [5][1][1]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=1},
    [5][2][0]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=0},  [5][2][1]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=1},
    [5][3][0]={.addr=BOARD_ADDR_08, .board.x=1, .board.y=0, .board.z=0},  [5][3][1]={.addr=BOARD_ADDR_08, .board.x=1, .board.y=0, .board.z=1},
    [5][4][0]={.addr=BOARD_ADDR_09, .board.x=1, .board.y=0, .board.z=0},  [5][4][1]={.addr=BOARD_ADDR_09, .board.x=1, .board.y=0, .board.z=1},

    [6][0][0]={.addr=BOARD_ADDR_05, .board.x=2, .board.y=0, .board.z=0},  [6][0][1]={.addr=BOARD_ADDR_05, .board.x=2, .board.y=0, .board.z=1},
    [6][1][0]={.addr=BOARD_ADDR_06, .board.x=2, .board.y=0, .board.z=0},  [6][1][1]={.addr=BOARD_ADDR_06, .board.x=2, .board.y=0, .board.z=1},
    [6][2][0]={.addr=BOARD_ADDR_07, .board.x=2, .board.y=0, .board.z=0},  [6][2][1]={.addr=BOARD_ADDR_07, .board.x=2, .board.y=0, .board.z=1},
    [6][3][0]={.addr=BOARD_ADDR_08, .board.x=2, .board.y=0, .board.z=0},  [6][3][1]={.addr=BOARD_ADDR_08, .board.x=2, .board.y=0, .board.z=1},
    [6][4][0]={.addr=BOARD_ADDR_09, .board.x=2, .board.y=0, .board.z=0},  [6][4][1]={.addr=BOARD_ADDR_09, .board.x=2, .board.y=0, .board.z=1},

    [7][0][0]={.addr=BOARD_ADDR_05, .board.x=3, .board.y=0, .board.z=0},  [7][0][1]={.addr=BOARD_ADDR_05, .board.x=3, .board.y=0, .board.z=1},
    [7][1][0]={.addr=BOARD_ADDR_06, .board.x=3, .board.y=0, .board.z=0},  [7][1][1]={.addr=BOARD_ADDR_06, .board.x=3, .board.y=0, .board.z=1},
    [7][2][0]={.addr=BOARD_ADDR_07, .board.x=3, .board.y=0, .board.z=0},  [7][2][1]={.addr=BOARD_ADDR_07, .board.x=3, .board.y=0, .board.z=1},
    [7][3][0]={.addr=BOARD_ADDR_08, .board.x=3, .board.y=0, .board.z=0},  [7][3][1]={.addr=BOARD_ADDR_08, .board.x=3, .board.y=0, .board.z=1},
    [7][4][0]={.addr=BOARD_ADDR_09, .board.x=3, .board.y=0, .board.z=0},  [7][4][1]={.addr=BOARD_ADDR_09, .board.x=3, .board.y=0, .board.z=1},

    [8][0][0]={.addr=BOARD_ADDR_10, .board.x=0, .board.y=0, .board.z=0},  [8][0][1]={.addr=BOARD_ADDR_10, .board.x=0, .board.y=0, .board.z=1},
    [8][1][0]={.addr=BOARD_ADDR_11, .board.x=0, .board.y=0, .board.z=0},  [8][1][1]={.addr=BOARD_ADDR_11, .board.x=0, .board.y=0, .board.z=1},
    [8][2][0]={.addr=BOARD_ADDR_12, .board.x=0, .board.y=0, .board.z=0},  [8][2][1]={.addr=BOARD_ADDR_12, .board.x=0, .board.y=0, .board.z=1},
    [8][3][0]={.addr=BOARD_ADDR_13, .board.x=0, .board.y=0, .board.z=0},  [8][3][1]={.addr=BOARD_ADDR_13, .board.x=0, .board.y=0, .board.z=1},
    [8][4][0]={.addr=BOARD_ADDR_14, .board.x=0, .board.y=0, .board.z=0},  [8][4][1]={.addr=BOARD_ADDR_14, .board.x=0, .board.y=0, .board.z=1},

    [9][0][0]={.addr=BOARD_ADDR_10, .board.x=1, .board.y=0, .board.z=0},  [9][0][1]={.addr=BOARD_ADDR_10, .board.x=1, .board.y=0, .board.z=1},
    [9][1][0]={.addr=BOARD_ADDR_11, .board.x=1, .board.y=0, .board.z=0},  [9][1][1]={.addr=BOARD_ADDR_11, .board.x=1, .board.y=0, .board.z=1},
    [9][2][0]={.addr=BOARD_ADDR_12, .board.x=1, .board.y=0, .board.z=0},  [9][2][1]={.addr=BOARD_ADDR_12, .board.x=1, .board.y=0, .board.z=1},
    [9][3][0]={.addr=BOARD_ADDR_13, .board.x=1, .board.y=0, .board.z=0},  [9][3][1]={.addr=BOARD_ADDR_13, .board.x=1, .board.y=0, .board.z=1},
    [9][4][0]={.addr=BOARD_ADDR_14, .board.x=1, .board.y=0, .board.z=0},  [9][4][1]={.addr=BOARD_ADDR_14, .board.x=1, .board.y=0, .board.z=1},

    [10][0][0]={.addr=BOARD_ADDR_10, .board.x=2, .board.y=0, .board.z=0}, [10][0][1]={.addr=BOARD_ADDR_10, .board.x=2, .board.y=0, .board.z=1},
    [10][1][0]={.addr=BOARD_ADDR_11, .board.x=2, .board.y=0, .board.z=0}, [10][1][1]={.addr=BOARD_ADDR_11, .board.x=2, .board.y=0, .board.z=1},
    [10][2][0]={.addr=BOARD_ADDR_12, .board.x=2, .board.y=0, .board.z=0}, [10][2][1]={.addr=BOARD_ADDR_12, .board.x=2, .board.y=0, .board.z=1},
    [10][3][0]={.addr=BOARD_ADDR_13, .board.x=2, .board.y=0, .board.z=0}, [10][3][1]={.addr=BOARD_ADDR_13, .board.x=2, .board.y=0, .board.z=1},
    [10][4][0]={.addr=BOARD_ADDR_14, .board.x=2, .board.y=0, .board.z=0}, [10][4][1]={.addr=BOARD_ADDR_14, .board.x=2, .board.y=0, .board.z=1},

    [11][0][0]={.addr=BOARD_ADDR_10, .board.x=3, .board.y=0, .board.z=0}, [11][0][1]={.addr=BOARD_ADDR_10, .board.x=3, .board.y=0, .board.z=1},
    [11][1][0]={.addr=BOARD_ADDR_11, .board.x=3, .board.y=0, .board.z=0}, [11][1][1]={.addr=BOARD_ADDR_11, .board.x=3, .board.y=0, .board.z=1},
    [11][2][0]={.addr=BOARD_ADDR_12, .board.x=3, .board.y=0, .board.z=0}, [11][2][1]={.addr=BOARD_ADDR_12, .board.x=3, .board.y=0, .board.z=1},
    [11][3][0]={.addr=BOARD_ADDR_13, .board.x=3, .board.y=0, .board.z=0}, [11][3][1]={.addr=BOARD_ADDR_13, .board.x=3, .board.y=0, .board.z=1},
    [11][4][0]={.addr=BOARD_ADDR_14, .board.x=3, .board.y=0, .board.z=0}, [11][4][1]={.addr=BOARD_ADDR_14, .board.x=3, .board.y=0, .board.z=1},
  };

  MATRIX_BoardList_t MATRIX_BoardList[MATRIX_NOF_BOARDS] = {
      {.addr=BOARD_ADDR_00, .cmdSent=false},
      {.addr=BOARD_ADDR_01, .cmdSent=false},
      {.addr=BOARD_ADDR_02, .cmdSent=false},
      {.addr=BOARD_ADDR_03, .cmdSent=false},
      {.addr=BOARD_ADDR_04, .cmdSent=false},

      {.addr=BOARD_ADDR_05, .cmdSent=false},
      {.addr=BOARD_ADDR_06, .cmdSent=false},
      {.addr=BOARD_ADDR_07, .cmdSent=false},
      {.addr=BOARD_ADDR_08, .cmdSent=false},
      {.addr=BOARD_ADDR_09, .cmdSent=false},

      {.addr=BOARD_ADDR_10, .cmdSent=false},
      {.addr=BOARD_ADDR_11, .cmdSent=false},
      {.addr=BOARD_ADDR_12, .cmdSent=false},
      {.addr=BOARD_ADDR_13, .cmdSent=false},
      {.addr=BOARD_ADDR_14, .cmdSent=false},
  };
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_16x9_ALEXIS
  const MatrixClock_t clockMatrix[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z] = /* information about how the clocks are organized */
  {
    [0][0][0]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=0},  [0][0][1]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=1},
    [0][1][0]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=0},  [0][1][1]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=1},
    [0][2][0]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=0},  [0][2][1]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=1},
    [0][3][0]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=0},  [0][3][1]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=1},
    [0][4][0]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=0},  [0][4][1]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=1},
    [0][5][0]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=0},  [0][5][1]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=1},
    [0][6][0]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=0},  [0][6][1]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=1},
    [0][7][0]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=0},  [0][7][1]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=1},
    [0][8][0]={.addr=BOARD_ADDR_08, .board.x=0, .board.y=0, .board.z=0},  [0][8][1]={.addr=BOARD_ADDR_08, .board.x=0, .board.y=0, .board.z=1},

    [1][0][0]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=0},  [1][0][1]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=1},
    [1][1][0]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=0},  [1][1][1]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=1},
    [1][2][0]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=0},  [1][2][1]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=1},
    [1][3][0]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=0},  [1][3][1]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=1},
    [1][4][0]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=0},  [1][4][1]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=1},
    [1][5][0]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=0},  [1][5][1]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=1},
    [1][6][0]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=0},  [1][6][1]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=1},
    [1][7][0]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=0},  [1][7][1]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=1},
    [1][8][0]={.addr=BOARD_ADDR_08, .board.x=1, .board.y=0, .board.z=0},  [1][8][1]={.addr=BOARD_ADDR_08, .board.x=1, .board.y=0, .board.z=1},

    [2][0][0]={.addr=BOARD_ADDR_00, .board.x=2, .board.y=0, .board.z=0},  [2][0][1]={.addr=BOARD_ADDR_00, .board.x=2, .board.y=0, .board.z=1},
    [2][1][0]={.addr=BOARD_ADDR_01, .board.x=2, .board.y=0, .board.z=0},  [2][1][1]={.addr=BOARD_ADDR_01, .board.x=2, .board.y=0, .board.z=1},
    [2][2][0]={.addr=BOARD_ADDR_02, .board.x=2, .board.y=0, .board.z=0},  [2][2][1]={.addr=BOARD_ADDR_02, .board.x=2, .board.y=0, .board.z=1},
    [2][3][0]={.addr=BOARD_ADDR_03, .board.x=2, .board.y=0, .board.z=0},  [2][3][1]={.addr=BOARD_ADDR_03, .board.x=2, .board.y=0, .board.z=1},
    [2][4][0]={.addr=BOARD_ADDR_04, .board.x=2, .board.y=0, .board.z=0},  [2][4][1]={.addr=BOARD_ADDR_04, .board.x=2, .board.y=0, .board.z=1},
    [2][5][0]={.addr=BOARD_ADDR_05, .board.x=2, .board.y=0, .board.z=0},  [2][5][1]={.addr=BOARD_ADDR_05, .board.x=2, .board.y=0, .board.z=1},
    [2][6][0]={.addr=BOARD_ADDR_06, .board.x=2, .board.y=0, .board.z=0},  [2][6][1]={.addr=BOARD_ADDR_06, .board.x=2, .board.y=0, .board.z=1},
    [2][7][0]={.addr=BOARD_ADDR_07, .board.x=2, .board.y=0, .board.z=0},  [2][7][1]={.addr=BOARD_ADDR_07, .board.x=2, .board.y=0, .board.z=1},
    [2][8][0]={.addr=BOARD_ADDR_08, .board.x=2, .board.y=0, .board.z=0},  [2][8][1]={.addr=BOARD_ADDR_08, .board.x=2, .board.y=0, .board.z=1},

    [3][0][0]={.addr=BOARD_ADDR_00, .board.x=3, .board.y=0, .board.z=0},  [3][0][1]={.addr=BOARD_ADDR_00, .board.x=3, .board.y=0, .board.z=1},
    [3][1][0]={.addr=BOARD_ADDR_01, .board.x=3, .board.y=0, .board.z=0},  [3][1][1]={.addr=BOARD_ADDR_01, .board.x=3, .board.y=0, .board.z=1},
    [3][2][0]={.addr=BOARD_ADDR_02, .board.x=3, .board.y=0, .board.z=0},  [3][2][1]={.addr=BOARD_ADDR_02, .board.x=3, .board.y=0, .board.z=1},
    [3][3][0]={.addr=BOARD_ADDR_03, .board.x=3, .board.y=0, .board.z=0},  [3][3][1]={.addr=BOARD_ADDR_03, .board.x=3, .board.y=0, .board.z=1},
    [3][4][0]={.addr=BOARD_ADDR_04, .board.x=3, .board.y=0, .board.z=0},  [3][4][1]={.addr=BOARD_ADDR_04, .board.x=3, .board.y=0, .board.z=1},
    [3][5][0]={.addr=BOARD_ADDR_05, .board.x=3, .board.y=0, .board.z=0},  [3][5][1]={.addr=BOARD_ADDR_05, .board.x=3, .board.y=0, .board.z=1},
    [3][6][0]={.addr=BOARD_ADDR_06, .board.x=3, .board.y=0, .board.z=0},  [3][6][1]={.addr=BOARD_ADDR_06, .board.x=3, .board.y=0, .board.z=1},
    [3][7][0]={.addr=BOARD_ADDR_07, .board.x=3, .board.y=0, .board.z=0},  [3][7][1]={.addr=BOARD_ADDR_07, .board.x=3, .board.y=0, .board.z=1},
    [3][8][0]={.addr=BOARD_ADDR_08, .board.x=3, .board.y=0, .board.z=0},  [3][8][1]={.addr=BOARD_ADDR_08, .board.x=3, .board.y=0, .board.z=1},

    [4][0][0]={.addr=BOARD_ADDR_09, .board.x=0, .board.y=0, .board.z=0},  [4][0][1]={.addr=BOARD_ADDR_09, .board.x=0, .board.y=0, .board.z=1},
    [4][1][0]={.addr=BOARD_ADDR_10, .board.x=0, .board.y=0, .board.z=0},  [4][1][1]={.addr=BOARD_ADDR_10, .board.x=0, .board.y=0, .board.z=1},
    [4][2][0]={.addr=BOARD_ADDR_11, .board.x=0, .board.y=0, .board.z=0},  [4][2][1]={.addr=BOARD_ADDR_11, .board.x=0, .board.y=0, .board.z=1},
    [4][3][0]={.addr=BOARD_ADDR_12, .board.x=0, .board.y=0, .board.z=0},  [4][3][1]={.addr=BOARD_ADDR_12, .board.x=0, .board.y=0, .board.z=1},
    [4][4][0]={.addr=BOARD_ADDR_13, .board.x=0, .board.y=0, .board.z=0},  [4][4][1]={.addr=BOARD_ADDR_13, .board.x=0, .board.y=0, .board.z=1},
    [4][5][0]={.addr=BOARD_ADDR_14, .board.x=0, .board.y=0, .board.z=0},  [4][5][1]={.addr=BOARD_ADDR_14, .board.x=0, .board.y=0, .board.z=1},
    [4][6][0]={.addr=BOARD_ADDR_15, .board.x=0, .board.y=0, .board.z=0},  [4][6][1]={.addr=BOARD_ADDR_15, .board.x=0, .board.y=0, .board.z=1},
    [4][7][0]={.addr=BOARD_ADDR_16, .board.x=0, .board.y=0, .board.z=0},  [4][7][1]={.addr=BOARD_ADDR_16, .board.x=0, .board.y=0, .board.z=1},
    [4][8][0]={.addr=BOARD_ADDR_17, .board.x=0, .board.y=0, .board.z=0},  [4][8][1]={.addr=BOARD_ADDR_17, .board.x=0, .board.y=0, .board.z=1},

    [5][0][0]={.addr=BOARD_ADDR_09, .board.x=1, .board.y=0, .board.z=0},  [5][0][1]={.addr=BOARD_ADDR_09, .board.x=1, .board.y=0, .board.z=1},
    [5][1][0]={.addr=BOARD_ADDR_10, .board.x=1, .board.y=0, .board.z=0},  [5][1][1]={.addr=BOARD_ADDR_10, .board.x=1, .board.y=0, .board.z=1},
    [5][2][0]={.addr=BOARD_ADDR_11, .board.x=1, .board.y=0, .board.z=0},  [5][2][1]={.addr=BOARD_ADDR_11, .board.x=1, .board.y=0, .board.z=1},
    [5][3][0]={.addr=BOARD_ADDR_12, .board.x=1, .board.y=0, .board.z=0},  [5][3][1]={.addr=BOARD_ADDR_12, .board.x=1, .board.y=0, .board.z=1},
    [5][4][0]={.addr=BOARD_ADDR_13, .board.x=1, .board.y=0, .board.z=0},  [5][4][1]={.addr=BOARD_ADDR_13, .board.x=1, .board.y=0, .board.z=1},
    [5][5][0]={.addr=BOARD_ADDR_14, .board.x=1, .board.y=0, .board.z=0},  [5][5][1]={.addr=BOARD_ADDR_14, .board.x=1, .board.y=0, .board.z=1},
    [5][6][0]={.addr=BOARD_ADDR_15, .board.x=1, .board.y=0, .board.z=0},  [5][6][1]={.addr=BOARD_ADDR_15, .board.x=1, .board.y=0, .board.z=1},
    [5][7][0]={.addr=BOARD_ADDR_16, .board.x=1, .board.y=0, .board.z=0},  [5][7][1]={.addr=BOARD_ADDR_16, .board.x=1, .board.y=0, .board.z=1},
    [5][8][0]={.addr=BOARD_ADDR_17, .board.x=1, .board.y=0, .board.z=0},  [5][8][1]={.addr=BOARD_ADDR_17, .board.x=1, .board.y=0, .board.z=1},

    [6][0][0]={.addr=BOARD_ADDR_09, .board.x=2, .board.y=0, .board.z=0},  [6][0][1]={.addr=BOARD_ADDR_09, .board.x=2, .board.y=0, .board.z=1},
    [6][1][0]={.addr=BOARD_ADDR_10, .board.x=2, .board.y=0, .board.z=0},  [6][1][1]={.addr=BOARD_ADDR_10, .board.x=2, .board.y=0, .board.z=1},
    [6][2][0]={.addr=BOARD_ADDR_11, .board.x=2, .board.y=0, .board.z=0},  [6][2][1]={.addr=BOARD_ADDR_11, .board.x=2, .board.y=0, .board.z=1},
    [6][3][0]={.addr=BOARD_ADDR_12, .board.x=2, .board.y=0, .board.z=0},  [6][3][1]={.addr=BOARD_ADDR_12, .board.x=2, .board.y=0, .board.z=1},
    [6][4][0]={.addr=BOARD_ADDR_13, .board.x=2, .board.y=0, .board.z=0},  [6][4][1]={.addr=BOARD_ADDR_13, .board.x=2, .board.y=0, .board.z=1},
    [6][5][0]={.addr=BOARD_ADDR_14, .board.x=2, .board.y=0, .board.z=0},  [6][5][1]={.addr=BOARD_ADDR_14, .board.x=2, .board.y=0, .board.z=1},
    [6][6][0]={.addr=BOARD_ADDR_15, .board.x=2, .board.y=0, .board.z=0},  [6][6][1]={.addr=BOARD_ADDR_15, .board.x=2, .board.y=0, .board.z=1},
    [6][7][0]={.addr=BOARD_ADDR_16, .board.x=2, .board.y=0, .board.z=0},  [6][7][1]={.addr=BOARD_ADDR_16, .board.x=2, .board.y=0, .board.z=1},
    [6][8][0]={.addr=BOARD_ADDR_17, .board.x=2, .board.y=0, .board.z=0},  [6][8][1]={.addr=BOARD_ADDR_17, .board.x=2, .board.y=0, .board.z=1},

    [7][0][0]={.addr=BOARD_ADDR_09, .board.x=3, .board.y=0, .board.z=0},  [7][0][1]={.addr=BOARD_ADDR_09, .board.x=3, .board.y=0, .board.z=1},
    [7][1][0]={.addr=BOARD_ADDR_10, .board.x=3, .board.y=0, .board.z=0},  [7][1][1]={.addr=BOARD_ADDR_10, .board.x=3, .board.y=0, .board.z=1},
    [7][2][0]={.addr=BOARD_ADDR_11, .board.x=3, .board.y=0, .board.z=0},  [7][2][1]={.addr=BOARD_ADDR_11, .board.x=3, .board.y=0, .board.z=1},
    [7][3][0]={.addr=BOARD_ADDR_12, .board.x=3, .board.y=0, .board.z=0},  [7][3][1]={.addr=BOARD_ADDR_12, .board.x=3, .board.y=0, .board.z=1},
    [7][4][0]={.addr=BOARD_ADDR_13, .board.x=3, .board.y=0, .board.z=0},  [7][4][1]={.addr=BOARD_ADDR_13, .board.x=3, .board.y=0, .board.z=1},
    [7][5][0]={.addr=BOARD_ADDR_14, .board.x=3, .board.y=0, .board.z=0},  [7][5][1]={.addr=BOARD_ADDR_14, .board.x=3, .board.y=0, .board.z=1},
    [7][6][0]={.addr=BOARD_ADDR_15, .board.x=3, .board.y=0, .board.z=0},  [7][6][1]={.addr=BOARD_ADDR_15, .board.x=3, .board.y=0, .board.z=1},
    [7][7][0]={.addr=BOARD_ADDR_16, .board.x=3, .board.y=0, .board.z=0},  [7][7][1]={.addr=BOARD_ADDR_16, .board.x=3, .board.y=0, .board.z=1},
    [7][8][0]={.addr=BOARD_ADDR_17, .board.x=3, .board.y=0, .board.z=0},  [7][8][1]={.addr=BOARD_ADDR_17, .board.x=3, .board.y=0, .board.z=1},

    [8][0][0]={.addr=BOARD_ADDR_18, .board.x=0, .board.y=0, .board.z=0},  [8][0][1]={.addr=BOARD_ADDR_18, .board.x=0, .board.y=0, .board.z=1},
    [8][1][0]={.addr=BOARD_ADDR_19, .board.x=0, .board.y=0, .board.z=0},  [8][1][1]={.addr=BOARD_ADDR_19, .board.x=0, .board.y=0, .board.z=1},
    [8][2][0]={.addr=BOARD_ADDR_20, .board.x=0, .board.y=0, .board.z=0},  [8][2][1]={.addr=BOARD_ADDR_20, .board.x=0, .board.y=0, .board.z=1},
    [8][3][0]={.addr=BOARD_ADDR_21, .board.x=0, .board.y=0, .board.z=0},  [8][3][1]={.addr=BOARD_ADDR_21, .board.x=0, .board.y=0, .board.z=1},
    [8][4][0]={.addr=BOARD_ADDR_22, .board.x=0, .board.y=0, .board.z=0},  [8][4][1]={.addr=BOARD_ADDR_22, .board.x=0, .board.y=0, .board.z=1},
    [8][5][0]={.addr=BOARD_ADDR_23, .board.x=0, .board.y=0, .board.z=0},  [8][5][1]={.addr=BOARD_ADDR_23, .board.x=0, .board.y=0, .board.z=1},
    [8][6][0]={.addr=BOARD_ADDR_24, .board.x=0, .board.y=0, .board.z=0},  [8][6][1]={.addr=BOARD_ADDR_24, .board.x=0, .board.y=0, .board.z=1},
    [8][7][0]={.addr=BOARD_ADDR_25, .board.x=0, .board.y=0, .board.z=0},  [8][7][1]={.addr=BOARD_ADDR_25, .board.x=0, .board.y=0, .board.z=1},
    [8][8][0]={.addr=BOARD_ADDR_26, .board.x=0, .board.y=0, .board.z=0},  [8][8][1]={.addr=BOARD_ADDR_26, .board.x=0, .board.y=0, .board.z=1},

    [9][0][0]={.addr=BOARD_ADDR_18, .board.x=1, .board.y=0, .board.z=0},  [9][0][1]={.addr=BOARD_ADDR_18, .board.x=1, .board.y=0, .board.z=1},
    [9][1][0]={.addr=BOARD_ADDR_19, .board.x=1, .board.y=0, .board.z=0},  [9][1][1]={.addr=BOARD_ADDR_19, .board.x=1, .board.y=0, .board.z=1},
    [9][2][0]={.addr=BOARD_ADDR_20, .board.x=1, .board.y=0, .board.z=0},  [9][2][1]={.addr=BOARD_ADDR_20, .board.x=1, .board.y=0, .board.z=1},
    [9][3][0]={.addr=BOARD_ADDR_21, .board.x=1, .board.y=0, .board.z=0},  [9][3][1]={.addr=BOARD_ADDR_21, .board.x=1, .board.y=0, .board.z=1},
    [9][4][0]={.addr=BOARD_ADDR_22, .board.x=1, .board.y=0, .board.z=0},  [9][4][1]={.addr=BOARD_ADDR_22, .board.x=1, .board.y=0, .board.z=1},
    [9][5][0]={.addr=BOARD_ADDR_23, .board.x=1, .board.y=0, .board.z=0},  [9][5][1]={.addr=BOARD_ADDR_23, .board.x=1, .board.y=0, .board.z=1},
    [9][6][0]={.addr=BOARD_ADDR_24, .board.x=1, .board.y=0, .board.z=0},  [9][6][1]={.addr=BOARD_ADDR_24, .board.x=1, .board.y=0, .board.z=1},
    [9][7][0]={.addr=BOARD_ADDR_25, .board.x=1, .board.y=0, .board.z=0},  [9][7][1]={.addr=BOARD_ADDR_25, .board.x=1, .board.y=0, .board.z=1},
    [9][8][0]={.addr=BOARD_ADDR_26, .board.x=1, .board.y=0, .board.z=0},  [9][8][1]={.addr=BOARD_ADDR_26, .board.x=1, .board.y=0, .board.z=1},

    [10][0][0]={.addr=BOARD_ADDR_18, .board.x=2, .board.y=0, .board.z=0}, [10][0][1]={.addr=BOARD_ADDR_18, .board.x=2, .board.y=0, .board.z=1},
    [10][1][0]={.addr=BOARD_ADDR_19, .board.x=2, .board.y=0, .board.z=0}, [10][1][1]={.addr=BOARD_ADDR_19, .board.x=2, .board.y=0, .board.z=1},
    [10][2][0]={.addr=BOARD_ADDR_20, .board.x=2, .board.y=0, .board.z=0}, [10][2][1]={.addr=BOARD_ADDR_20, .board.x=2, .board.y=0, .board.z=1},
    [10][3][0]={.addr=BOARD_ADDR_21, .board.x=2, .board.y=0, .board.z=0}, [10][3][1]={.addr=BOARD_ADDR_21, .board.x=2, .board.y=0, .board.z=1},
    [10][4][0]={.addr=BOARD_ADDR_22, .board.x=2, .board.y=0, .board.z=0}, [10][4][1]={.addr=BOARD_ADDR_22, .board.x=2, .board.y=0, .board.z=1},
    [10][5][0]={.addr=BOARD_ADDR_23, .board.x=2, .board.y=0, .board.z=0}, [10][5][1]={.addr=BOARD_ADDR_23, .board.x=2, .board.y=0, .board.z=1},
    [10][6][0]={.addr=BOARD_ADDR_24, .board.x=2, .board.y=0, .board.z=0}, [10][6][1]={.addr=BOARD_ADDR_24, .board.x=2, .board.y=0, .board.z=1},
    [10][7][0]={.addr=BOARD_ADDR_25, .board.x=2, .board.y=0, .board.z=0}, [10][7][1]={.addr=BOARD_ADDR_25, .board.x=2, .board.y=0, .board.z=1},
    [10][8][0]={.addr=BOARD_ADDR_26, .board.x=2, .board.y=0, .board.z=0}, [10][8][1]={.addr=BOARD_ADDR_26, .board.x=2, .board.y=0, .board.z=1},

    [11][0][0]={.addr=BOARD_ADDR_18, .board.x=3, .board.y=0, .board.z=0}, [11][0][1]={.addr=BOARD_ADDR_18, .board.x=3, .board.y=0, .board.z=1},
    [11][1][0]={.addr=BOARD_ADDR_19, .board.x=3, .board.y=0, .board.z=0}, [11][1][1]={.addr=BOARD_ADDR_19, .board.x=3, .board.y=0, .board.z=1},
    [11][2][0]={.addr=BOARD_ADDR_20, .board.x=3, .board.y=0, .board.z=0}, [11][2][1]={.addr=BOARD_ADDR_20, .board.x=3, .board.y=0, .board.z=1},
    [11][3][0]={.addr=BOARD_ADDR_21, .board.x=3, .board.y=0, .board.z=0}, [11][3][1]={.addr=BOARD_ADDR_21, .board.x=3, .board.y=0, .board.z=1},
    [11][4][0]={.addr=BOARD_ADDR_22, .board.x=3, .board.y=0, .board.z=0}, [11][4][1]={.addr=BOARD_ADDR_22, .board.x=3, .board.y=0, .board.z=1},
    [11][5][0]={.addr=BOARD_ADDR_23, .board.x=3, .board.y=0, .board.z=0}, [11][5][1]={.addr=BOARD_ADDR_23, .board.x=3, .board.y=0, .board.z=1},
    [11][6][0]={.addr=BOARD_ADDR_24, .board.x=3, .board.y=0, .board.z=0}, [11][6][1]={.addr=BOARD_ADDR_24, .board.x=3, .board.y=0, .board.z=1},
    [11][7][0]={.addr=BOARD_ADDR_25, .board.x=3, .board.y=0, .board.z=0}, [11][7][1]={.addr=BOARD_ADDR_25, .board.x=3, .board.y=0, .board.z=1},
    [11][8][0]={.addr=BOARD_ADDR_26, .board.x=3, .board.y=0, .board.z=0}, [11][8][1]={.addr=BOARD_ADDR_26, .board.x=3, .board.y=0, .board.z=1},

    [12][0][0]={.addr=BOARD_ADDR_27, .board.x=0, .board.y=0, .board.z=0}, [12][0][1]={.addr=BOARD_ADDR_27, .board.x=0, .board.y=0, .board.z=1},
    [12][1][0]={.addr=BOARD_ADDR_28, .board.x=0, .board.y=0, .board.z=0}, [12][1][1]={.addr=BOARD_ADDR_28, .board.x=0, .board.y=0, .board.z=1},
    [12][2][0]={.addr=BOARD_ADDR_29, .board.x=0, .board.y=0, .board.z=0}, [12][2][1]={.addr=BOARD_ADDR_29, .board.x=0, .board.y=0, .board.z=1},
    [12][3][0]={.addr=BOARD_ADDR_30, .board.x=0, .board.y=0, .board.z=0}, [12][3][1]={.addr=BOARD_ADDR_30, .board.x=0, .board.y=0, .board.z=1},
    [12][4][0]={.addr=BOARD_ADDR_31, .board.x=0, .board.y=0, .board.z=0}, [12][4][1]={.addr=BOARD_ADDR_31, .board.x=0, .board.y=0, .board.z=1},
    [12][5][0]={.addr=BOARD_ADDR_32, .board.x=0, .board.y=0, .board.z=0}, [12][5][1]={.addr=BOARD_ADDR_32, .board.x=0, .board.y=0, .board.z=1},
    [12][6][0]={.addr=BOARD_ADDR_33, .board.x=0, .board.y=0, .board.z=0}, [12][6][1]={.addr=BOARD_ADDR_33, .board.x=0, .board.y=0, .board.z=1},
    [12][7][0]={.addr=BOARD_ADDR_34, .board.x=0, .board.y=0, .board.z=0}, [12][7][1]={.addr=BOARD_ADDR_34, .board.x=0, .board.y=0, .board.z=1},
    [12][8][0]={.addr=BOARD_ADDR_35, .board.x=0, .board.y=0, .board.z=0}, [12][8][1]={.addr=BOARD_ADDR_35, .board.x=0, .board.y=0, .board.z=1},

    [13][0][0]={.addr=BOARD_ADDR_27, .board.x=1, .board.y=0, .board.z=0}, [13][0][1]={.addr=BOARD_ADDR_27, .board.x=1, .board.y=0, .board.z=1},
    [13][1][0]={.addr=BOARD_ADDR_28, .board.x=1, .board.y=0, .board.z=0}, [13][1][1]={.addr=BOARD_ADDR_28, .board.x=1, .board.y=0, .board.z=1},
    [13][2][0]={.addr=BOARD_ADDR_29, .board.x=1, .board.y=0, .board.z=0}, [13][2][1]={.addr=BOARD_ADDR_29, .board.x=1, .board.y=0, .board.z=1},
    [13][3][0]={.addr=BOARD_ADDR_30, .board.x=1, .board.y=0, .board.z=0}, [13][3][1]={.addr=BOARD_ADDR_30, .board.x=1, .board.y=0, .board.z=1},
    [13][4][0]={.addr=BOARD_ADDR_31, .board.x=1, .board.y=0, .board.z=0}, [13][4][1]={.addr=BOARD_ADDR_31, .board.x=1, .board.y=0, .board.z=1},
    [13][5][0]={.addr=BOARD_ADDR_32, .board.x=1, .board.y=0, .board.z=0}, [13][5][1]={.addr=BOARD_ADDR_32, .board.x=1, .board.y=0, .board.z=1},
    [13][6][0]={.addr=BOARD_ADDR_33, .board.x=1, .board.y=0, .board.z=0}, [13][6][1]={.addr=BOARD_ADDR_33, .board.x=1, .board.y=0, .board.z=1},
    [13][7][0]={.addr=BOARD_ADDR_34, .board.x=1, .board.y=0, .board.z=0}, [13][7][1]={.addr=BOARD_ADDR_34, .board.x=1, .board.y=0, .board.z=1},
    [13][8][0]={.addr=BOARD_ADDR_35, .board.x=1, .board.y=0, .board.z=0}, [13][8][1]={.addr=BOARD_ADDR_35, .board.x=1, .board.y=0, .board.z=1},

    [14][0][0]={.addr=BOARD_ADDR_27, .board.x=2, .board.y=0, .board.z=0}, [14][0][1]={.addr=BOARD_ADDR_27, .board.x=2, .board.y=0, .board.z=1},
    [14][1][0]={.addr=BOARD_ADDR_28, .board.x=2, .board.y=0, .board.z=0}, [14][1][1]={.addr=BOARD_ADDR_28, .board.x=2, .board.y=0, .board.z=1},
    [14][2][0]={.addr=BOARD_ADDR_29, .board.x=2, .board.y=0, .board.z=0}, [14][2][1]={.addr=BOARD_ADDR_29, .board.x=2, .board.y=0, .board.z=1},
    [14][3][0]={.addr=BOARD_ADDR_30, .board.x=2, .board.y=0, .board.z=0}, [14][3][1]={.addr=BOARD_ADDR_30, .board.x=2, .board.y=0, .board.z=1},
    [14][4][0]={.addr=BOARD_ADDR_31, .board.x=2, .board.y=0, .board.z=0}, [14][4][1]={.addr=BOARD_ADDR_31, .board.x=2, .board.y=0, .board.z=1},
    [14][5][0]={.addr=BOARD_ADDR_32, .board.x=2, .board.y=0, .board.z=0}, [14][5][1]={.addr=BOARD_ADDR_32, .board.x=2, .board.y=0, .board.z=1},
    [14][6][0]={.addr=BOARD_ADDR_33, .board.x=2, .board.y=0, .board.z=0}, [14][6][1]={.addr=BOARD_ADDR_33, .board.x=2, .board.y=0, .board.z=1},
    [14][7][0]={.addr=BOARD_ADDR_34, .board.x=2, .board.y=0, .board.z=0}, [14][7][1]={.addr=BOARD_ADDR_34, .board.x=2, .board.y=0, .board.z=1},
    [14][8][0]={.addr=BOARD_ADDR_35, .board.x=2, .board.y=0, .board.z=0}, [14][8][1]={.addr=BOARD_ADDR_35, .board.x=2, .board.y=0, .board.z=1},

    [15][0][0]={.addr=BOARD_ADDR_27, .board.x=3, .board.y=0, .board.z=0}, [15][0][1]={.addr=BOARD_ADDR_27, .board.x=3, .board.y=0, .board.z=1},
    [15][1][0]={.addr=BOARD_ADDR_28, .board.x=3, .board.y=0, .board.z=0}, [15][1][1]={.addr=BOARD_ADDR_28, .board.x=3, .board.y=0, .board.z=1},
    [15][2][0]={.addr=BOARD_ADDR_29, .board.x=3, .board.y=0, .board.z=0}, [15][2][1]={.addr=BOARD_ADDR_29, .board.x=3, .board.y=0, .board.z=1},
    [15][3][0]={.addr=BOARD_ADDR_30, .board.x=3, .board.y=0, .board.z=0}, [15][3][1]={.addr=BOARD_ADDR_30, .board.x=3, .board.y=0, .board.z=1},
    [15][4][0]={.addr=BOARD_ADDR_31, .board.x=3, .board.y=0, .board.z=0}, [15][4][1]={.addr=BOARD_ADDR_31, .board.x=3, .board.y=0, .board.z=1},
    [15][5][0]={.addr=BOARD_ADDR_32, .board.x=3, .board.y=0, .board.z=0}, [15][5][1]={.addr=BOARD_ADDR_32, .board.x=3, .board.y=0, .board.z=1},
    [15][6][0]={.addr=BOARD_ADDR_33, .board.x=3, .board.y=0, .board.z=0}, [15][6][1]={.addr=BOARD_ADDR_33, .board.x=3, .board.y=0, .board.z=1},
    [15][7][0]={.addr=BOARD_ADDR_34, .board.x=3, .board.y=0, .board.z=0}, [15][7][1]={.addr=BOARD_ADDR_34, .board.x=3, .board.y=0, .board.z=1},
    [15][8][0]={.addr=BOARD_ADDR_35, .board.x=3, .board.y=0, .board.z=0}, [15][8][1]={.addr=BOARD_ADDR_35, .board.x=3, .board.y=0, .board.z=1},
  };

  MATRIX_BoardList_t MATRIX_BoardList[MATRIX_NOF_BOARDS] = {
      {.addr=BOARD_ADDR_00, .cmdSent=false},
      {.addr=BOARD_ADDR_01, .cmdSent=false},
      {.addr=BOARD_ADDR_02, .cmdSent=false},
      {.addr=BOARD_ADDR_03, .cmdSent=false},
      {.addr=BOARD_ADDR_04, .cmdSent=false},
      {.addr=BOARD_ADDR_05, .cmdSent=false},
      {.addr=BOARD_ADDR_06, .cmdSent=false},
      {.addr=BOARD_ADDR_07, .cmdSent=false},
      {.addr=BOARD_ADDR_08, .cmdSent=false},

      {.addr=BOARD_ADDR_09, .cmdSent=false},
      {.addr=BOARD_ADDR_10, .cmdSent=false},
      {.addr=BOARD_ADDR_11, .cmdSent=false},
      {.addr=BOARD_ADDR_12, .cmdSent=false},
      {.addr=BOARD_ADDR_13, .cmdSent=false},
      {.addr=BOARD_ADDR_14, .cmdSent=false},
      {.addr=BOARD_ADDR_15, .cmdSent=false},
      {.addr=BOARD_ADDR_16, .cmdSent=false},
      {.addr=BOARD_ADDR_17, .cmdSent=false},

      {.addr=BOARD_ADDR_18, .cmdSent=false},
      {.addr=BOARD_ADDR_19, .cmdSent=false},
      {.addr=BOARD_ADDR_20, .cmdSent=false},
      {.addr=BOARD_ADDR_21, .cmdSent=false},
      {.addr=BOARD_ADDR_22, .cmdSent=false},
      {.addr=BOARD_ADDR_23, .cmdSent=false},
      {.addr=BOARD_ADDR_24, .cmdSent=false},
      {.addr=BOARD_ADDR_25, .cmdSent=false},
      {.addr=BOARD_ADDR_26, .cmdSent=false},

      {.addr=BOARD_ADDR_27, .cmdSent=false},
      {.addr=BOARD_ADDR_28, .cmdSent=false},
      {.addr=BOARD_ADDR_29, .cmdSent=false},
      {.addr=BOARD_ADDR_30, .cmdSent=false},
      {.addr=BOARD_ADDR_31, .cmdSent=false},
      {.addr=BOARD_ADDR_32, .cmdSent=false},
      {.addr=BOARD_ADDR_33, .cmdSent=false},
      {.addr=BOARD_ADDR_34, .cmdSent=false},
      {.addr=BOARD_ADDR_35, .cmdSent=false},
  };
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_SMARTWALL_8x5
const MatrixClock_t clockMatrix[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z] = /* information about how the clocks are organized */
{
			/* 1st row */
			[0][0][0]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=0},
			[1][0][0]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=0},
			[2][0][0]={.addr=BOARD_ADDR_00, .board.x=2, .board.y=0, .board.z=0},
			[3][0][0]={.addr=BOARD_ADDR_00, .board.x=3, .board.y=0, .board.z=0},
			[4][0][0]={.addr=BOARD_ADDR_00, .board.x=4, .board.y=0, .board.z=0},
			[5][0][0]={.addr=BOARD_ADDR_00, .board.x=5, .board.y=0, .board.z=0},
			[6][0][0]={.addr=BOARD_ADDR_00, .board.x=6, .board.y=0, .board.z=0},
			[7][0][0]={.addr=BOARD_ADDR_00, .board.x=7, .board.y=0, .board.z=0},
#if 0 /* not enabled yet */
			/* 2nd row */
			[0][1][0]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=0},
			[1][1][0]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=0},
			[2][1][0]={.addr=BOARD_ADDR_01, .board.x=2, .board.y=0, .board.z=0},
			[3][1][0]={.addr=BOARD_ADDR_01, .board.x=3, .board.y=0, .board.z=0},
			[4][1][0]={.addr=BOARD_ADDR_01, .board.x=4, .board.y=0, .board.z=0},
			[5][1][0]={.addr=BOARD_ADDR_01, .board.x=5, .board.y=0, .board.z=0},
			[6][1][0]={.addr=BOARD_ADDR_01, .board.x=6, .board.y=0, .board.z=0},
			[7][1][0]={.addr=BOARD_ADDR_01, .board.x=7, .board.y=0, .board.z=0},

			/* 3rd row */
			[0][2][0]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=0},
			[1][2][0]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=0},
			[2][2][0]={.addr=BOARD_ADDR_02, .board.x=2, .board.y=0, .board.z=0},
			[3][2][0]={.addr=BOARD_ADDR_02, .board.x=3, .board.y=0, .board.z=0},
			[4][2][0]={.addr=BOARD_ADDR_02, .board.x=4, .board.y=0, .board.z=0},
			[5][2][0]={.addr=BOARD_ADDR_02, .board.x=5, .board.y=0, .board.z=0},
			[6][2][0]={.addr=BOARD_ADDR_02, .board.x=6, .board.y=0, .board.z=0},
			[7][2][0]={.addr=BOARD_ADDR_02, .board.x=7, .board.y=0, .board.z=0},

			/* 4th row */
			[0][3][0]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=0},
			[1][3][0]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=0},
			[2][3][0]={.addr=BOARD_ADDR_03, .board.x=2, .board.y=0, .board.z=0},
			[3][3][0]={.addr=BOARD_ADDR_03, .board.x=3, .board.y=0, .board.z=0},
			[4][3][0]={.addr=BOARD_ADDR_03, .board.x=4, .board.y=0, .board.z=0},
			[5][3][0]={.addr=BOARD_ADDR_03, .board.x=5, .board.y=0, .board.z=0},
			[6][3][0]={.addr=BOARD_ADDR_03, .board.x=6, .board.y=0, .board.z=0},
			[7][3][0]={.addr=BOARD_ADDR_03, .board.x=7, .board.y=0, .board.z=0},

			/* 5th row */
			[0][4][0]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=0},
			[1][4][0]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=0},
			[2][4][0]={.addr=BOARD_ADDR_04, .board.x=2, .board.y=0, .board.z=0},
			[3][4][0]={.addr=BOARD_ADDR_04, .board.x=3, .board.y=0, .board.z=0},
			[4][4][0]={.addr=BOARD_ADDR_04, .board.x=4, .board.y=0, .board.z=0},
			[5][4][0]={.addr=BOARD_ADDR_04, .board.x=5, .board.y=0, .board.z=0},
			[6][4][0]={.addr=BOARD_ADDR_04, .board.x=6, .board.y=0, .board.z=0},
			[7][4][0]={.addr=BOARD_ADDR_04, .board.x=7, .board.y=0, .board.z=0},
#endif
	};

	MATRIX_BoardList_t MATRIX_BoardList[MATRIX_NOF_BOARDS] = {
		{.addr=BOARD_ADDR_00, .cmdSent=false},
#if 0
		{.addr=BOARD_ADDR_01, .cmdSent=false},
		{.addr=BOARD_ADDR_02, .cmdSent=false},
		{.addr=BOARD_ADDR_03, .cmdSent=false},
		{.addr=BOARD_ADDR_04, .cmdSent=false},
#endif
	};
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_8x3_V4
  const MatrixClock_t clockMatrix[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z] = /* information about how the clocks are organized */
  {
    [0][0][0]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=0},  [0][0][1]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=1},
    [0][1][0]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=0},  [0][1][1]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=1},
    [0][2][0]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=0},  [0][2][1]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=1},

    [1][0][0]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=0},  [1][0][1]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=1},
    [1][1][0]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=0},  [1][1][1]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=1},
    [1][2][0]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=0},  [1][2][1]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=1},

    [2][0][0]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=0},  [2][0][1]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=1},
    [2][1][0]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=0},  [2][1][1]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=1},
    [2][2][0]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=0},  [2][2][1]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=1},

    [3][0][0]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=0},  [3][0][1]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=1},
    [3][1][0]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=0},  [3][1][1]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=1},
    [3][2][0]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=0},  [3][2][1]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=1},

    [4][0][0]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=0},  [4][0][1]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=1},
    [4][1][0]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=0},  [4][1][1]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=1},
    [4][2][0]={.addr=BOARD_ADDR_08, .board.x=0, .board.y=0, .board.z=0},  [4][2][1]={.addr=BOARD_ADDR_08, .board.x=0, .board.y=0, .board.z=1},

    [5][0][0]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=0},  [5][0][1]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=1},
    [5][1][0]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=0},  [5][1][1]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=1},
    [5][2][0]={.addr=BOARD_ADDR_08, .board.x=1, .board.y=0, .board.z=0},  [5][2][1]={.addr=BOARD_ADDR_08, .board.x=1, .board.y=0, .board.z=1},

    [6][0][0]={.addr=BOARD_ADDR_09, .board.x=0, .board.y=0, .board.z=0},  [6][0][1]={.addr=BOARD_ADDR_09, .board.x=0, .board.y=0, .board.z=1},
    [6][1][0]={.addr=BOARD_ADDR_10, .board.x=0, .board.y=0, .board.z=0},  [6][1][1]={.addr=BOARD_ADDR_10, .board.x=0, .board.y=0, .board.z=1},
    [6][2][0]={.addr=BOARD_ADDR_11, .board.x=0, .board.y=0, .board.z=0},  [6][2][1]={.addr=BOARD_ADDR_11, .board.x=0, .board.y=0, .board.z=1},

    [7][0][0]={.addr=BOARD_ADDR_09, .board.x=1, .board.y=0, .board.z=0},  [7][0][1]={.addr=BOARD_ADDR_09, .board.x=1, .board.y=0, .board.z=1},
    [7][1][0]={.addr=BOARD_ADDR_10, .board.x=1, .board.y=0, .board.z=0},  [7][1][1]={.addr=BOARD_ADDR_10, .board.x=1, .board.y=0, .board.z=1},
    [7][2][0]={.addr=BOARD_ADDR_11, .board.x=1, .board.y=0, .board.z=0},  [7][2][1]={.addr=BOARD_ADDR_11, .board.x=1, .board.y=0, .board.z=1},
  };

	MATRIX_BoardList_t MATRIX_BoardList[MATRIX_NOF_BOARDS] = {
      {.addr=BOARD_ADDR_00, .cmdSent=false},
      {.addr=BOARD_ADDR_01, .cmdSent=false},
      {.addr=BOARD_ADDR_02, .cmdSent=false},

      {.addr=BOARD_ADDR_03, .cmdSent=false},
      {.addr=BOARD_ADDR_04, .cmdSent=false},
      {.addr=BOARD_ADDR_05, .cmdSent=false},

      {.addr=BOARD_ADDR_06, .cmdSent=false},
      {.addr=BOARD_ADDR_07, .cmdSent=false},
      {.addr=BOARD_ADDR_08, .cmdSent=false},

      {.addr=BOARD_ADDR_09, .cmdSent=false},
      {.addr=BOARD_ADDR_10, .cmdSent=false},
      {.addr=BOARD_ADDR_11, .cmdSent=false},
  };
#elif PL_MATRIX_ID_CIRCULAR_CLOCK_1x12
  const MatrixClock_t clockMatrix[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z] = /* information about how the clocks are organized */
  {
    /* center clock */
    [0][0][0]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=0},  [0][0][1]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=1},
    /* hour clocks */
    [1][0][0]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=0},  [1][0][1]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=1},
    [2][0][0]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=0},  [2][0][1]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=1},
    [3][0][0]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=0},  [3][0][1]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=1},
    [4][0][0]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=0},  [4][0][1]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=1},
    [5][0][0]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=0},  [5][0][1]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=1},
    [6][0][0]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=0},  [6][0][1]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=1},
    [7][0][0]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=0},  [7][0][1]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=1},
    [8][0][0]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=0},  [8][0][1]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=1},
    [9][0][0]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=0},  [9][0][1]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=1},
    [10][0][0]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=0},  [10][0][1]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=1},
    [11][0][0]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=0},  [11][0][1]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=1},
    [12][0][0]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=0},  [12][0][1]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=1},
  };

  MATRIX_BoardList_t MATRIX_BoardList[MATRIX_NOF_BOARDS] = {
      {.addr=BOARD_ADDR_00, .cmdSent=false},
      {.addr=BOARD_ADDR_01, .cmdSent=false},
      {.addr=BOARD_ADDR_02, .cmdSent=false},

      {.addr=BOARD_ADDR_03, .cmdSent=false},
      {.addr=BOARD_ADDR_04, .cmdSent=false},
      {.addr=BOARD_ADDR_05, .cmdSent=false},
  };
#else
  #error "unknown matrix"
#endif

#endif /* PL_CONFIG_IS_MASTER */
