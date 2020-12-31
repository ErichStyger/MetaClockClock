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
    [0][0][0]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][0][1]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][1][0]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][1][1]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][2][0]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][2][1]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [1][0][0]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][0][1]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][1][0]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][1][1]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][2][0]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][2][1]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=1, .enabled=true},

    [2][0][0]={.addr=BOARD_ADDR_00, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [2][0][1]={.addr=BOARD_ADDR_00, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [2][1][0]={.addr=BOARD_ADDR_01, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [2][1][1]={.addr=BOARD_ADDR_01, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [2][2][0]={.addr=BOARD_ADDR_02, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [2][2][1]={.addr=BOARD_ADDR_02, .board.x=2, .board.y=0, .board.z=1, .enabled=true},

    [3][0][0]={.addr=BOARD_ADDR_00, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [3][0][1]={.addr=BOARD_ADDR_00, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [3][1][0]={.addr=BOARD_ADDR_01, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [3][1][1]={.addr=BOARD_ADDR_01, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [3][2][0]={.addr=BOARD_ADDR_02, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [3][2][1]={.addr=BOARD_ADDR_02, .board.x=3, .board.y=0, .board.z=1, .enabled=true},

    [4][0][0]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][0][1]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][1][0]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][1][1]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][2][0]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][2][1]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [5][0][0]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][0][1]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][1][0]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][1][1]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][2][0]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][2][1]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=1, .enabled=true},

    [6][0][0]={.addr=BOARD_ADDR_05, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [6][0][1]={.addr=BOARD_ADDR_05, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [6][1][0]={.addr=BOARD_ADDR_06, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [6][1][1]={.addr=BOARD_ADDR_06, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [6][2][0]={.addr=BOARD_ADDR_07, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [6][2][1]={.addr=BOARD_ADDR_07, .board.x=2, .board.y=0, .board.z=1, .enabled=true},

    [7][0][0]={.addr=BOARD_ADDR_05, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [7][0][1]={.addr=BOARD_ADDR_05, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [7][1][0]={.addr=BOARD_ADDR_06, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [7][1][1]={.addr=BOARD_ADDR_06, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [7][2][0]={.addr=BOARD_ADDR_07, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [7][2][1]={.addr=BOARD_ADDR_07, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
  };

  MATRIX_BoardList_t MATRIX_BoardList[MATRIX_NOF_BOARDS] = {
      {.enabled = true, .addr=BOARD_ADDR_00, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_01, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_02, .cmdSent=false},

      {.enabled = true, .addr=BOARD_ADDR_05, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_06, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_07, .cmdSent=false},
  };
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_12x5_MOD
  const MatrixClock_t clockMatrix[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z] = /* information about how the clocks are organized */
  {
    /* 1st column */
    [0][0][0]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [0][0][1]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][1][0]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [0][1][1]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][2][0]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [0][2][1]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][3][0]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [0][3][1]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][4][0]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [0][4][1]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [1][0][0]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [1][0][1]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][1][0]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [1][1][1]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][2][0]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [1][2][1]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][3][0]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [1][3][1]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][4][0]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [1][4][1]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=1, .enabled=true},

    [2][0][0]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [2][0][1]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [2][1][0]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [2][1][1]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [2][2][0]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [2][2][1]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [2][3][0]={.addr=BOARD_ADDR_08, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [2][3][1]={.addr=BOARD_ADDR_08, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [2][4][0]={.addr=BOARD_ADDR_09, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [2][4][1]={.addr=BOARD_ADDR_09, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [3][0][0]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [3][0][1]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [3][1][0]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [3][1][1]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [3][2][0]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [3][2][1]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [3][3][0]={.addr=BOARD_ADDR_08, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [3][3][1]={.addr=BOARD_ADDR_08, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [3][4][0]={.addr=BOARD_ADDR_09, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [3][4][1]={.addr=BOARD_ADDR_09, .board.x=1, .board.y=0, .board.z=1, .enabled=true},

    [4][0][0]={.addr=BOARD_ADDR_10, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [4][0][1]={.addr=BOARD_ADDR_10, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][1][0]={.addr=BOARD_ADDR_11, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [4][1][1]={.addr=BOARD_ADDR_11, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][2][0]={.addr=BOARD_ADDR_12, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [4][2][1]={.addr=BOARD_ADDR_12, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][3][0]={.addr=BOARD_ADDR_13, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [4][3][1]={.addr=BOARD_ADDR_13, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][4][0]={.addr=BOARD_ADDR_14, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [4][4][1]={.addr=BOARD_ADDR_14, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [5][0][0]={.addr=BOARD_ADDR_10, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [5][0][1]={.addr=BOARD_ADDR_10, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][1][0]={.addr=BOARD_ADDR_11, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [5][1][1]={.addr=BOARD_ADDR_11, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][2][0]={.addr=BOARD_ADDR_12, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [5][2][1]={.addr=BOARD_ADDR_12, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][3][0]={.addr=BOARD_ADDR_13, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [5][3][1]={.addr=BOARD_ADDR_13, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][4][0]={.addr=BOARD_ADDR_14, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [5][4][1]={.addr=BOARD_ADDR_14, .board.x=1, .board.y=0, .board.z=1, .enabled=true},

    [6][0][0]={.addr=BOARD_ADDR_15, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [6][0][1]={.addr=BOARD_ADDR_15, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [6][1][0]={.addr=BOARD_ADDR_16, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [6][1][1]={.addr=BOARD_ADDR_16, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [6][2][0]={.addr=BOARD_ADDR_17, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [6][2][1]={.addr=BOARD_ADDR_17, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [6][3][0]={.addr=BOARD_ADDR_18, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [6][3][1]={.addr=BOARD_ADDR_18, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [6][4][0]={.addr=BOARD_ADDR_19, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [6][4][1]={.addr=BOARD_ADDR_19, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [7][0][0]={.addr=BOARD_ADDR_15, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [7][0][1]={.addr=BOARD_ADDR_15, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [7][1][0]={.addr=BOARD_ADDR_16, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [7][1][1]={.addr=BOARD_ADDR_16, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [7][2][0]={.addr=BOARD_ADDR_17, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [7][2][1]={.addr=BOARD_ADDR_17, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [7][3][0]={.addr=BOARD_ADDR_18, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [7][3][1]={.addr=BOARD_ADDR_18, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [7][4][0]={.addr=BOARD_ADDR_19, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [7][4][1]={.addr=BOARD_ADDR_19, .board.x=1, .board.y=0, .board.z=1, .enabled=true},

    [8][0][0]={.addr=BOARD_ADDR_20, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [8][0][1]={.addr=BOARD_ADDR_20, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [8][1][0]={.addr=BOARD_ADDR_21, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [8][1][1]={.addr=BOARD_ADDR_21, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [8][2][0]={.addr=BOARD_ADDR_22, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [8][2][1]={.addr=BOARD_ADDR_22, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [8][3][0]={.addr=BOARD_ADDR_23, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [8][3][1]={.addr=BOARD_ADDR_23, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [8][4][0]={.addr=BOARD_ADDR_24, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [8][4][1]={.addr=BOARD_ADDR_24, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [9][0][0]={.addr=BOARD_ADDR_25, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [9][0][1]={.addr=BOARD_ADDR_25, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [9][1][0]={.addr=BOARD_ADDR_26, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [9][1][1]={.addr=BOARD_ADDR_26, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [9][2][0]={.addr=BOARD_ADDR_27, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [9][2][1]={.addr=BOARD_ADDR_27, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [9][3][0]={.addr=BOARD_ADDR_28, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [9][3][1]={.addr=BOARD_ADDR_28, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [9][4][0]={.addr=BOARD_ADDR_29, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [9][4][1]={.addr=BOARD_ADDR_29, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [10][0][0]={.addr=BOARD_ADDR_30, .board.x=0, .board.y=0, .board.z=0, .enabled=true},[10][0][1]={.addr=BOARD_ADDR_30, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [10][1][0]={.addr=BOARD_ADDR_31, .board.x=0, .board.y=0, .board.z=0, .enabled=true},[10][1][1]={.addr=BOARD_ADDR_31, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [10][2][0]={.addr=BOARD_ADDR_32, .board.x=0, .board.y=0, .board.z=0, .enabled=true},[10][2][1]={.addr=BOARD_ADDR_32, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [10][3][0]={.addr=BOARD_ADDR_33, .board.x=0, .board.y=0, .board.z=0, .enabled=true},[10][3][1]={.addr=BOARD_ADDR_33, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [10][4][0]={.addr=BOARD_ADDR_35, .board.x=0, .board.y=0, .board.z=0, .enabled=true},[10][4][1]={.addr=BOARD_ADDR_35, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [11][0][0]={.addr=BOARD_ADDR_37, .board.x=0, .board.y=0, .board.z=0, .enabled=true},[11][0][1]={.addr=BOARD_ADDR_37, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [11][1][0]={.addr=BOARD_ADDR_31, .board.x=1, .board.y=0, .board.z=0, .enabled=true},[11][1][1]={.addr=BOARD_ADDR_31, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [11][2][0]={.addr=BOARD_ADDR_32, .board.x=1, .board.y=0, .board.z=0, .enabled=true},[11][2][1]={.addr=BOARD_ADDR_32, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [11][3][0]={.addr=BOARD_ADDR_34, .board.x=0, .board.y=0, .board.z=0, .enabled=true},[11][3][1]={.addr=BOARD_ADDR_34, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [11][4][0]={.addr=BOARD_ADDR_36, .board.x=0, .board.y=0, .board.z=0, .enabled=true},[11][4][1]={.addr=BOARD_ADDR_36, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
};

  MATRIX_BoardList_t MATRIX_BoardList[MATRIX_NOF_BOARDS] = {
      {.enabled = true, .addr=BOARD_ADDR_00, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_01, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_02, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_03, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_04, .cmdSent=false},

      {.enabled = true, .addr=BOARD_ADDR_05, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_06, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_07, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_08, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_09, .cmdSent=false},

      {.enabled = true, .addr=BOARD_ADDR_10, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_11, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_12, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_13, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_14, .cmdSent=false},

      {.enabled = true, .addr=BOARD_ADDR_15, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_16, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_17, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_18, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_19, .cmdSent=false},

      {.enabled = true, .addr=BOARD_ADDR_20, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_21, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_22, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_23, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_24, .cmdSent=false},

      {.enabled = true, .addr=BOARD_ADDR_25, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_26, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_27, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_28, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_29, .cmdSent=false},

      {.enabled = true, .addr=BOARD_ADDR_30, .cmdSent=false},      {.enabled = true, .addr=BOARD_ADDR_37, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_31, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_32, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_33, .cmdSent=false},      {.enabled = true, .addr=BOARD_ADDR_34, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_35, .cmdSent=false},      {.enabled = true, .addr=BOARD_ADDR_36, .cmdSent=false},
};
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_12x5_60B
  const MatrixClock_t clockMatrix[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z] = /* information about how the clocks are organized */
  {
    [0][0][0]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][0][1]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][1][0]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][1][1]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][2][0]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][2][1]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][3][0]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][3][1]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][4][0]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][4][1]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [1][0][0]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][0][1]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][1][0]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][1][1]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][2][0]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][2][1]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][3][0]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][3][1]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][4][0]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][4][1]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=1, .enabled=true},

    [2][0][0]={.addr=BOARD_ADDR_00, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [2][0][1]={.addr=BOARD_ADDR_00, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [2][1][0]={.addr=BOARD_ADDR_01, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [2][1][1]={.addr=BOARD_ADDR_01, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [2][2][0]={.addr=BOARD_ADDR_02, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [2][2][1]={.addr=BOARD_ADDR_02, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [2][3][0]={.addr=BOARD_ADDR_03, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [2][3][1]={.addr=BOARD_ADDR_03, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [2][4][0]={.addr=BOARD_ADDR_04, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [2][4][1]={.addr=BOARD_ADDR_04, .board.x=2, .board.y=0, .board.z=1, .enabled=true},

    [3][0][0]={.addr=BOARD_ADDR_00, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [3][0][1]={.addr=BOARD_ADDR_00, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [3][1][0]={.addr=BOARD_ADDR_01, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [3][1][1]={.addr=BOARD_ADDR_01, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [3][2][0]={.addr=BOARD_ADDR_02, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [3][2][1]={.addr=BOARD_ADDR_02, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [3][3][0]={.addr=BOARD_ADDR_03, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [3][3][1]={.addr=BOARD_ADDR_03, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [3][4][0]={.addr=BOARD_ADDR_04, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [3][4][1]={.addr=BOARD_ADDR_04, .board.x=3, .board.y=0, .board.z=1, .enabled=true},

    [4][0][0]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][0][1]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][1][0]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][1][1]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][2][0]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][2][1]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][3][0]={.addr=BOARD_ADDR_08, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][3][1]={.addr=BOARD_ADDR_08, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][4][0]={.addr=BOARD_ADDR_09, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][4][1]={.addr=BOARD_ADDR_09, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [5][0][0]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][0][1]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][1][0]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][1][1]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][2][0]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][2][1]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][3][0]={.addr=BOARD_ADDR_08, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][3][1]={.addr=BOARD_ADDR_08, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][4][0]={.addr=BOARD_ADDR_09, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][4][1]={.addr=BOARD_ADDR_09, .board.x=1, .board.y=0, .board.z=1, .enabled=true},

    [6][0][0]={.addr=BOARD_ADDR_05, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [6][0][1]={.addr=BOARD_ADDR_05, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [6][1][0]={.addr=BOARD_ADDR_06, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [6][1][1]={.addr=BOARD_ADDR_06, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [6][2][0]={.addr=BOARD_ADDR_07, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [6][2][1]={.addr=BOARD_ADDR_07, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [6][3][0]={.addr=BOARD_ADDR_08, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [6][3][1]={.addr=BOARD_ADDR_08, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [6][4][0]={.addr=BOARD_ADDR_09, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [6][4][1]={.addr=BOARD_ADDR_09, .board.x=2, .board.y=0, .board.z=1, .enabled=true},

    [7][0][0]={.addr=BOARD_ADDR_05, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [7][0][1]={.addr=BOARD_ADDR_05, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [7][1][0]={.addr=BOARD_ADDR_06, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [7][1][1]={.addr=BOARD_ADDR_06, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [7][2][0]={.addr=BOARD_ADDR_07, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [7][2][1]={.addr=BOARD_ADDR_07, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [7][3][0]={.addr=BOARD_ADDR_08, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [7][3][1]={.addr=BOARD_ADDR_08, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [7][4][0]={.addr=BOARD_ADDR_09, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [7][4][1]={.addr=BOARD_ADDR_09, .board.x=3, .board.y=0, .board.z=1, .enabled=true},

    [8][0][0]={.addr=BOARD_ADDR_10, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [8][0][1]={.addr=BOARD_ADDR_10, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [8][1][0]={.addr=BOARD_ADDR_11, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [8][1][1]={.addr=BOARD_ADDR_11, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [8][2][0]={.addr=BOARD_ADDR_12, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [8][2][1]={.addr=BOARD_ADDR_12, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [8][3][0]={.addr=BOARD_ADDR_13, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [8][3][1]={.addr=BOARD_ADDR_13, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [8][4][0]={.addr=BOARD_ADDR_14, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [8][4][1]={.addr=BOARD_ADDR_14, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [9][0][0]={.addr=BOARD_ADDR_10, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [9][0][1]={.addr=BOARD_ADDR_10, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [9][1][0]={.addr=BOARD_ADDR_11, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [9][1][1]={.addr=BOARD_ADDR_11, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [9][2][0]={.addr=BOARD_ADDR_12, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [9][2][1]={.addr=BOARD_ADDR_12, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [9][3][0]={.addr=BOARD_ADDR_13, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [9][3][1]={.addr=BOARD_ADDR_13, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [9][4][0]={.addr=BOARD_ADDR_14, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [9][4][1]={.addr=BOARD_ADDR_14, .board.x=1, .board.y=0, .board.z=1, .enabled=true},

    [10][0][0]={.addr=BOARD_ADDR_10, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [10][0][1]={.addr=BOARD_ADDR_10, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [10][1][0]={.addr=BOARD_ADDR_11, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [10][1][1]={.addr=BOARD_ADDR_11, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [10][2][0]={.addr=BOARD_ADDR_12, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [10][2][1]={.addr=BOARD_ADDR_12, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [10][3][0]={.addr=BOARD_ADDR_13, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [10][3][1]={.addr=BOARD_ADDR_13, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [10][4][0]={.addr=BOARD_ADDR_14, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [10][4][1]={.addr=BOARD_ADDR_14, .board.x=2, .board.y=0, .board.z=1, .enabled=true},

    [11][0][0]={.addr=BOARD_ADDR_10, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [11][0][1]={.addr=BOARD_ADDR_10, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [11][1][0]={.addr=BOARD_ADDR_11, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [11][1][1]={.addr=BOARD_ADDR_11, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [11][2][0]={.addr=BOARD_ADDR_12, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [11][2][1]={.addr=BOARD_ADDR_12, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [11][3][0]={.addr=BOARD_ADDR_13, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [11][3][1]={.addr=BOARD_ADDR_13, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [11][4][0]={.addr=BOARD_ADDR_14, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [11][4][1]={.addr=BOARD_ADDR_14, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
  };

  MATRIX_BoardList_t MATRIX_BoardList[MATRIX_NOF_BOARDS] = {
      {.enabled = true, .addr=BOARD_ADDR_00, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_01, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_02, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_03, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_04, .cmdSent=false},

      {.enabled = true, .addr=BOARD_ADDR_05, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_06, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_07, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_08, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_09, .cmdSent=false},

      {.enabled = true, .addr=BOARD_ADDR_10, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_11, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_12, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_13, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_14, .cmdSent=false},
  };
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_16x9_ALEXIS
  const MatrixClock_t clockMatrix[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z] = /* information about how the clocks are organized */
  {
    [0][0][0]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][0][1]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][1][0]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][1][1]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][2][0]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][2][1]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][3][0]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][3][1]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][4][0]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][4][1]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][5][0]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][5][1]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][6][0]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][6][1]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][7][0]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][7][1]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][8][0]={.addr=BOARD_ADDR_08, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][8][1]={.addr=BOARD_ADDR_08, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [1][0][0]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][0][1]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][1][0]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][1][1]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][2][0]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][2][1]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][3][0]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][3][1]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][4][0]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][4][1]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][5][0]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][5][1]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][6][0]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][6][1]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][7][0]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][7][1]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][8][0]={.addr=BOARD_ADDR_08, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][8][1]={.addr=BOARD_ADDR_08, .board.x=1, .board.y=0, .board.z=1, .enabled=true},

    [2][0][0]={.addr=BOARD_ADDR_00, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [2][0][1]={.addr=BOARD_ADDR_00, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [2][1][0]={.addr=BOARD_ADDR_01, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [2][1][1]={.addr=BOARD_ADDR_01, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [2][2][0]={.addr=BOARD_ADDR_02, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [2][2][1]={.addr=BOARD_ADDR_02, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [2][3][0]={.addr=BOARD_ADDR_03, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [2][3][1]={.addr=BOARD_ADDR_03, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [2][4][0]={.addr=BOARD_ADDR_04, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [2][4][1]={.addr=BOARD_ADDR_04, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [2][5][0]={.addr=BOARD_ADDR_05, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [2][5][1]={.addr=BOARD_ADDR_05, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [2][6][0]={.addr=BOARD_ADDR_06, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [2][6][1]={.addr=BOARD_ADDR_06, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [2][7][0]={.addr=BOARD_ADDR_07, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [2][7][1]={.addr=BOARD_ADDR_07, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [2][8][0]={.addr=BOARD_ADDR_08, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [2][8][1]={.addr=BOARD_ADDR_08, .board.x=2, .board.y=0, .board.z=1, .enabled=true},

    [3][0][0]={.addr=BOARD_ADDR_00, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [3][0][1]={.addr=BOARD_ADDR_00, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [3][1][0]={.addr=BOARD_ADDR_01, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [3][1][1]={.addr=BOARD_ADDR_01, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [3][2][0]={.addr=BOARD_ADDR_02, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [3][2][1]={.addr=BOARD_ADDR_02, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [3][3][0]={.addr=BOARD_ADDR_03, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [3][3][1]={.addr=BOARD_ADDR_03, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [3][4][0]={.addr=BOARD_ADDR_04, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [3][4][1]={.addr=BOARD_ADDR_04, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [3][5][0]={.addr=BOARD_ADDR_05, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [3][5][1]={.addr=BOARD_ADDR_05, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [3][6][0]={.addr=BOARD_ADDR_06, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [3][6][1]={.addr=BOARD_ADDR_06, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [3][7][0]={.addr=BOARD_ADDR_07, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [3][7][1]={.addr=BOARD_ADDR_07, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [3][8][0]={.addr=BOARD_ADDR_08, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [3][8][1]={.addr=BOARD_ADDR_08, .board.x=3, .board.y=0, .board.z=1, .enabled=true},

    [4][0][0]={.addr=BOARD_ADDR_09, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][0][1]={.addr=BOARD_ADDR_09, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][1][0]={.addr=BOARD_ADDR_10, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][1][1]={.addr=BOARD_ADDR_10, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][2][0]={.addr=BOARD_ADDR_11, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][2][1]={.addr=BOARD_ADDR_11, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][3][0]={.addr=BOARD_ADDR_12, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][3][1]={.addr=BOARD_ADDR_12, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][4][0]={.addr=BOARD_ADDR_13, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][4][1]={.addr=BOARD_ADDR_13, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][5][0]={.addr=BOARD_ADDR_14, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][5][1]={.addr=BOARD_ADDR_14, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][6][0]={.addr=BOARD_ADDR_15, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][6][1]={.addr=BOARD_ADDR_15, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][7][0]={.addr=BOARD_ADDR_16, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][7][1]={.addr=BOARD_ADDR_16, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][8][0]={.addr=BOARD_ADDR_17, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][8][1]={.addr=BOARD_ADDR_17, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [5][0][0]={.addr=BOARD_ADDR_09, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][0][1]={.addr=BOARD_ADDR_09, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][1][0]={.addr=BOARD_ADDR_10, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][1][1]={.addr=BOARD_ADDR_10, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][2][0]={.addr=BOARD_ADDR_11, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][2][1]={.addr=BOARD_ADDR_11, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][3][0]={.addr=BOARD_ADDR_12, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][3][1]={.addr=BOARD_ADDR_12, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][4][0]={.addr=BOARD_ADDR_13, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][4][1]={.addr=BOARD_ADDR_13, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][5][0]={.addr=BOARD_ADDR_14, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][5][1]={.addr=BOARD_ADDR_14, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][6][0]={.addr=BOARD_ADDR_15, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][6][1]={.addr=BOARD_ADDR_15, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][7][0]={.addr=BOARD_ADDR_16, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][7][1]={.addr=BOARD_ADDR_16, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][8][0]={.addr=BOARD_ADDR_17, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][8][1]={.addr=BOARD_ADDR_17, .board.x=1, .board.y=0, .board.z=1, .enabled=true},

    [6][0][0]={.addr=BOARD_ADDR_09, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [6][0][1]={.addr=BOARD_ADDR_09, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [6][1][0]={.addr=BOARD_ADDR_10, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [6][1][1]={.addr=BOARD_ADDR_10, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [6][2][0]={.addr=BOARD_ADDR_11, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [6][2][1]={.addr=BOARD_ADDR_11, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [6][3][0]={.addr=BOARD_ADDR_12, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [6][3][1]={.addr=BOARD_ADDR_12, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [6][4][0]={.addr=BOARD_ADDR_13, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [6][4][1]={.addr=BOARD_ADDR_13, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [6][5][0]={.addr=BOARD_ADDR_14, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [6][5][1]={.addr=BOARD_ADDR_14, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [6][6][0]={.addr=BOARD_ADDR_15, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [6][6][1]={.addr=BOARD_ADDR_15, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [6][7][0]={.addr=BOARD_ADDR_16, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [6][7][1]={.addr=BOARD_ADDR_16, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [6][8][0]={.addr=BOARD_ADDR_17, .board.x=2, .board.y=0, .board.z=0, .enabled=true},  [6][8][1]={.addr=BOARD_ADDR_17, .board.x=2, .board.y=0, .board.z=1, .enabled=true},

    [7][0][0]={.addr=BOARD_ADDR_09, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [7][0][1]={.addr=BOARD_ADDR_09, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [7][1][0]={.addr=BOARD_ADDR_10, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [7][1][1]={.addr=BOARD_ADDR_10, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [7][2][0]={.addr=BOARD_ADDR_11, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [7][2][1]={.addr=BOARD_ADDR_11, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [7][3][0]={.addr=BOARD_ADDR_12, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [7][3][1]={.addr=BOARD_ADDR_12, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [7][4][0]={.addr=BOARD_ADDR_13, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [7][4][1]={.addr=BOARD_ADDR_13, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [7][5][0]={.addr=BOARD_ADDR_14, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [7][5][1]={.addr=BOARD_ADDR_14, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [7][6][0]={.addr=BOARD_ADDR_15, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [7][6][1]={.addr=BOARD_ADDR_15, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [7][7][0]={.addr=BOARD_ADDR_16, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [7][7][1]={.addr=BOARD_ADDR_16, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [7][8][0]={.addr=BOARD_ADDR_17, .board.x=3, .board.y=0, .board.z=0, .enabled=true},  [7][8][1]={.addr=BOARD_ADDR_17, .board.x=3, .board.y=0, .board.z=1, .enabled=true},

    [8][0][0]={.addr=BOARD_ADDR_18, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [8][0][1]={.addr=BOARD_ADDR_18, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [8][1][0]={.addr=BOARD_ADDR_19, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [8][1][1]={.addr=BOARD_ADDR_19, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [8][2][0]={.addr=BOARD_ADDR_20, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [8][2][1]={.addr=BOARD_ADDR_20, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [8][3][0]={.addr=BOARD_ADDR_21, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [8][3][1]={.addr=BOARD_ADDR_21, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [8][4][0]={.addr=BOARD_ADDR_22, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [8][4][1]={.addr=BOARD_ADDR_22, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [8][5][0]={.addr=BOARD_ADDR_23, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [8][5][1]={.addr=BOARD_ADDR_23, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [8][6][0]={.addr=BOARD_ADDR_24, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [8][6][1]={.addr=BOARD_ADDR_24, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [8][7][0]={.addr=BOARD_ADDR_25, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [8][7][1]={.addr=BOARD_ADDR_25, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [8][8][0]={.addr=BOARD_ADDR_26, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [8][8][1]={.addr=BOARD_ADDR_26, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [9][0][0]={.addr=BOARD_ADDR_18, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [9][0][1]={.addr=BOARD_ADDR_18, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [9][1][0]={.addr=BOARD_ADDR_19, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [9][1][1]={.addr=BOARD_ADDR_19, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [9][2][0]={.addr=BOARD_ADDR_20, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [9][2][1]={.addr=BOARD_ADDR_20, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [9][3][0]={.addr=BOARD_ADDR_21, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [9][3][1]={.addr=BOARD_ADDR_21, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [9][4][0]={.addr=BOARD_ADDR_22, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [9][4][1]={.addr=BOARD_ADDR_22, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [9][5][0]={.addr=BOARD_ADDR_23, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [9][5][1]={.addr=BOARD_ADDR_23, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [9][6][0]={.addr=BOARD_ADDR_24, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [9][6][1]={.addr=BOARD_ADDR_24, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [9][7][0]={.addr=BOARD_ADDR_25, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [9][7][1]={.addr=BOARD_ADDR_25, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [9][8][0]={.addr=BOARD_ADDR_26, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [9][8][1]={.addr=BOARD_ADDR_26, .board.x=1, .board.y=0, .board.z=1, .enabled=true},

    [10][0][0]={.addr=BOARD_ADDR_18, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [10][0][1]={.addr=BOARD_ADDR_18, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [10][1][0]={.addr=BOARD_ADDR_19, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [10][1][1]={.addr=BOARD_ADDR_19, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [10][2][0]={.addr=BOARD_ADDR_20, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [10][2][1]={.addr=BOARD_ADDR_20, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [10][3][0]={.addr=BOARD_ADDR_21, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [10][3][1]={.addr=BOARD_ADDR_21, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [10][4][0]={.addr=BOARD_ADDR_22, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [10][4][1]={.addr=BOARD_ADDR_22, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [10][5][0]={.addr=BOARD_ADDR_23, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [10][5][1]={.addr=BOARD_ADDR_23, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [10][6][0]={.addr=BOARD_ADDR_24, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [10][6][1]={.addr=BOARD_ADDR_24, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [10][7][0]={.addr=BOARD_ADDR_25, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [10][7][1]={.addr=BOARD_ADDR_25, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [10][8][0]={.addr=BOARD_ADDR_26, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [10][8][1]={.addr=BOARD_ADDR_26, .board.x=2, .board.y=0, .board.z=1, .enabled=true},

    [11][0][0]={.addr=BOARD_ADDR_18, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [11][0][1]={.addr=BOARD_ADDR_18, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [11][1][0]={.addr=BOARD_ADDR_19, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [11][1][1]={.addr=BOARD_ADDR_19, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [11][2][0]={.addr=BOARD_ADDR_20, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [11][2][1]={.addr=BOARD_ADDR_20, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [11][3][0]={.addr=BOARD_ADDR_21, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [11][3][1]={.addr=BOARD_ADDR_21, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [11][4][0]={.addr=BOARD_ADDR_22, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [11][4][1]={.addr=BOARD_ADDR_22, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [11][5][0]={.addr=BOARD_ADDR_23, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [11][5][1]={.addr=BOARD_ADDR_23, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [11][6][0]={.addr=BOARD_ADDR_24, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [11][6][1]={.addr=BOARD_ADDR_24, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [11][7][0]={.addr=BOARD_ADDR_25, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [11][7][1]={.addr=BOARD_ADDR_25, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [11][8][0]={.addr=BOARD_ADDR_26, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [11][8][1]={.addr=BOARD_ADDR_26, .board.x=3, .board.y=0, .board.z=1, .enabled=true},

    [12][0][0]={.addr=BOARD_ADDR_27, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [12][0][1]={.addr=BOARD_ADDR_27, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [12][1][0]={.addr=BOARD_ADDR_28, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [12][1][1]={.addr=BOARD_ADDR_28, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [12][2][0]={.addr=BOARD_ADDR_29, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [12][2][1]={.addr=BOARD_ADDR_29, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [12][3][0]={.addr=BOARD_ADDR_30, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [12][3][1]={.addr=BOARD_ADDR_30, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [12][4][0]={.addr=BOARD_ADDR_31, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [12][4][1]={.addr=BOARD_ADDR_31, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [12][5][0]={.addr=BOARD_ADDR_32, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [12][5][1]={.addr=BOARD_ADDR_32, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [12][6][0]={.addr=BOARD_ADDR_33, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [12][6][1]={.addr=BOARD_ADDR_33, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [12][7][0]={.addr=BOARD_ADDR_34, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [12][7][1]={.addr=BOARD_ADDR_34, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [12][8][0]={.addr=BOARD_ADDR_35, .board.x=0, .board.y=0, .board.z=0, .enabled=true}, [12][8][1]={.addr=BOARD_ADDR_35, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [13][0][0]={.addr=BOARD_ADDR_27, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [13][0][1]={.addr=BOARD_ADDR_27, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [13][1][0]={.addr=BOARD_ADDR_28, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [13][1][1]={.addr=BOARD_ADDR_28, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [13][2][0]={.addr=BOARD_ADDR_29, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [13][2][1]={.addr=BOARD_ADDR_29, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [13][3][0]={.addr=BOARD_ADDR_30, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [13][3][1]={.addr=BOARD_ADDR_30, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [13][4][0]={.addr=BOARD_ADDR_31, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [13][4][1]={.addr=BOARD_ADDR_31, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [13][5][0]={.addr=BOARD_ADDR_32, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [13][5][1]={.addr=BOARD_ADDR_32, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [13][6][0]={.addr=BOARD_ADDR_33, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [13][6][1]={.addr=BOARD_ADDR_33, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [13][7][0]={.addr=BOARD_ADDR_34, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [13][7][1]={.addr=BOARD_ADDR_34, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [13][8][0]={.addr=BOARD_ADDR_35, .board.x=1, .board.y=0, .board.z=0, .enabled=true}, [13][8][1]={.addr=BOARD_ADDR_35, .board.x=1, .board.y=0, .board.z=1, .enabled=true},

    [14][0][0]={.addr=BOARD_ADDR_27, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [14][0][1]={.addr=BOARD_ADDR_27, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [14][1][0]={.addr=BOARD_ADDR_28, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [14][1][1]={.addr=BOARD_ADDR_28, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [14][2][0]={.addr=BOARD_ADDR_29, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [14][2][1]={.addr=BOARD_ADDR_29, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [14][3][0]={.addr=BOARD_ADDR_30, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [14][3][1]={.addr=BOARD_ADDR_30, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [14][4][0]={.addr=BOARD_ADDR_31, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [14][4][1]={.addr=BOARD_ADDR_31, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [14][5][0]={.addr=BOARD_ADDR_32, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [14][5][1]={.addr=BOARD_ADDR_32, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [14][6][0]={.addr=BOARD_ADDR_33, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [14][6][1]={.addr=BOARD_ADDR_33, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [14][7][0]={.addr=BOARD_ADDR_34, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [14][7][1]={.addr=BOARD_ADDR_34, .board.x=2, .board.y=0, .board.z=1, .enabled=true},
    [14][8][0]={.addr=BOARD_ADDR_35, .board.x=2, .board.y=0, .board.z=0, .enabled=true}, [14][8][1]={.addr=BOARD_ADDR_35, .board.x=2, .board.y=0, .board.z=1, .enabled=true},

    [15][0][0]={.addr=BOARD_ADDR_27, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [15][0][1]={.addr=BOARD_ADDR_27, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [15][1][0]={.addr=BOARD_ADDR_28, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [15][1][1]={.addr=BOARD_ADDR_28, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [15][2][0]={.addr=BOARD_ADDR_29, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [15][2][1]={.addr=BOARD_ADDR_29, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [15][3][0]={.addr=BOARD_ADDR_30, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [15][3][1]={.addr=BOARD_ADDR_30, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [15][4][0]={.addr=BOARD_ADDR_31, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [15][4][1]={.addr=BOARD_ADDR_31, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [15][5][0]={.addr=BOARD_ADDR_32, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [15][5][1]={.addr=BOARD_ADDR_32, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [15][6][0]={.addr=BOARD_ADDR_33, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [15][6][1]={.addr=BOARD_ADDR_33, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [15][7][0]={.addr=BOARD_ADDR_34, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [15][7][1]={.addr=BOARD_ADDR_34, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
    [15][8][0]={.addr=BOARD_ADDR_35, .board.x=3, .board.y=0, .board.z=0, .enabled=true}, [15][8][1]={.addr=BOARD_ADDR_35, .board.x=3, .board.y=0, .board.z=1, .enabled=true},
  };

  MATRIX_BoardList_t MATRIX_BoardList[MATRIX_NOF_BOARDS] = {
      {.enabled = true, .addr=BOARD_ADDR_00, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_01, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_02, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_03, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_04, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_05, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_06, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_07, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_08, .cmdSent=false},

      {.enabled = true, .addr=BOARD_ADDR_09, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_10, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_11, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_12, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_13, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_14, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_15, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_16, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_17, .cmdSent=false},

      {.enabled = true, .addr=BOARD_ADDR_18, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_19, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_20, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_21, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_22, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_23, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_24, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_25, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_26, .cmdSent=false},

      {.enabled = true, .addr=BOARD_ADDR_27, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_28, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_29, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_30, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_31, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_32, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_33, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_34, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_35, .cmdSent=false},
  };
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_SMARTWALL_8x5
const MatrixClock_t clockMatrix[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z] = /* information about how the clocks are organized */
{
			/* 1st row */
			[0][0][0]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=0, .enabled=true},
			[1][0][0]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=0, .enabled=true},
			[2][0][0]={.addr=BOARD_ADDR_00, .board.x=2, .board.y=0, .board.z=0, .enabled=true},
			[3][0][0]={.addr=BOARD_ADDR_00, .board.x=3, .board.y=0, .board.z=0, .enabled=true},
			[4][0][0]={.addr=BOARD_ADDR_00, .board.x=4, .board.y=0, .board.z=0, .enabled=true},
			[5][0][0]={.addr=BOARD_ADDR_00, .board.x=5, .board.y=0, .board.z=0, .enabled=true},
			[6][0][0]={.addr=BOARD_ADDR_00, .board.x=6, .board.y=0, .board.z=0, .enabled=true},
			[7][0][0]={.addr=BOARD_ADDR_00, .board.x=7, .board.y=0, .board.z=0, .enabled=true},

			/* 2nd row */
			[0][1][0]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=0, .enabled=true},
			[1][1][0]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=0, .enabled=true},
			[2][1][0]={.addr=BOARD_ADDR_01, .board.x=2, .board.y=0, .board.z=0, .enabled=true},
			[3][1][0]={.addr=BOARD_ADDR_01, .board.x=3, .board.y=0, .board.z=0, .enabled=true},
			[4][1][0]={.addr=BOARD_ADDR_01, .board.x=4, .board.y=0, .board.z=0, .enabled=true},
			[5][1][0]={.addr=BOARD_ADDR_01, .board.x=5, .board.y=0, .board.z=0, .enabled=true},
			[6][1][0]={.addr=BOARD_ADDR_01, .board.x=6, .board.y=0, .board.z=0, .enabled=true},
			[7][1][0]={.addr=BOARD_ADDR_01, .board.x=7, .board.y=0, .board.z=0, .enabled=true},

			/* 3rd row */
			[0][2][0]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=0, .enabled=true},
			[1][2][0]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=0, .enabled=true},
			[2][2][0]={.addr=BOARD_ADDR_02, .board.x=2, .board.y=0, .board.z=0, .enabled=true},
			[3][2][0]={.addr=BOARD_ADDR_02, .board.x=3, .board.y=0, .board.z=0, .enabled=true},
			[4][2][0]={.addr=BOARD_ADDR_02, .board.x=4, .board.y=0, .board.z=0, .enabled=true},
			[5][2][0]={.addr=BOARD_ADDR_02, .board.x=5, .board.y=0, .board.z=0, .enabled=true},
			[6][2][0]={.addr=BOARD_ADDR_02, .board.x=6, .board.y=0, .board.z=0, .enabled=true},
			[7][2][0]={.addr=BOARD_ADDR_02, .board.x=7, .board.y=0, .board.z=0, .enabled=true},

			/* 4th row */
			[0][3][0]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=0, .enabled=true},
			[1][3][0]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=0, .enabled=true},
			[2][3][0]={.addr=BOARD_ADDR_03, .board.x=2, .board.y=0, .board.z=0, .enabled=true},
			[3][3][0]={.addr=BOARD_ADDR_03, .board.x=3, .board.y=0, .board.z=0, .enabled=true},
			[4][3][0]={.addr=BOARD_ADDR_03, .board.x=4, .board.y=0, .board.z=0, .enabled=true},
			[5][3][0]={.addr=BOARD_ADDR_03, .board.x=5, .board.y=0, .board.z=0, .enabled=true},
			[6][3][0]={.addr=BOARD_ADDR_03, .board.x=6, .board.y=0, .board.z=0, .enabled=true},
			[7][3][0]={.addr=BOARD_ADDR_03, .board.x=7, .board.y=0, .board.z=0, .enabled=true},

			/* 5th row */
			[0][4][0]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=0, .enabled=true},
			[1][4][0]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=0, .enabled=true},
			[2][4][0]={.addr=BOARD_ADDR_04, .board.x=2, .board.y=0, .board.z=0, .enabled=true},
			[3][4][0]={.addr=BOARD_ADDR_04, .board.x=3, .board.y=0, .board.z=0, .enabled=true},
			[4][4][0]={.addr=BOARD_ADDR_04, .board.x=4, .board.y=0, .board.z=0, .enabled=true},
			[5][4][0]={.addr=BOARD_ADDR_04, .board.x=5, .board.y=0, .board.z=0, .enabled=true},
			[6][4][0]={.addr=BOARD_ADDR_04, .board.x=6, .board.y=0, .board.z=0, .enabled=true},
			[7][4][0]={.addr=BOARD_ADDR_04, .board.x=7, .board.y=0, .board.z=0, .enabled=true},
	};

	MATRIX_BoardList_t MATRIX_BoardList[MATRIX_NOF_BOARDS] = {
		{	.enabled = true, .addr=BOARD_ADDR_00, .cmdSent=false},
		{	.enabled = true, .addr=BOARD_ADDR_01, .cmdSent=false},
		{	.enabled = true, .addr=BOARD_ADDR_02, .cmdSent=false},
		{	.enabled = true, .addr=BOARD_ADDR_03, .cmdSent=false},
		{	.enabled = true, .addr=BOARD_ADDR_04, .cmdSent=false},
	};
#elif PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_CLOCK_8x3_V4
  const MatrixClock_t clockMatrix[MATRIX_NOF_STEPPERS_X][MATRIX_NOF_STEPPERS_Y][MATRIX_NOF_STEPPERS_Z] = /* information about how the clocks are organized */
  {
    [0][0][0]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][0][1]={.addr=BOARD_ADDR_00, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][1][0]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][1][1]={.addr=BOARD_ADDR_01, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [0][2][0]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [0][2][1]={.addr=BOARD_ADDR_02, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [1][0][0]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][0][1]={.addr=BOARD_ADDR_00, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][1][0]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][1][1]={.addr=BOARD_ADDR_01, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [1][2][0]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [1][2][1]={.addr=BOARD_ADDR_02, .board.x=1, .board.y=0, .board.z=1, .enabled=true},

    [2][0][0]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [2][0][1]={.addr=BOARD_ADDR_03, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [2][1][0]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [2][1][1]={.addr=BOARD_ADDR_04, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [2][2][0]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [2][2][1]={.addr=BOARD_ADDR_05, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [3][0][0]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [3][0][1]={.addr=BOARD_ADDR_03, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [3][1][0]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [3][1][1]={.addr=BOARD_ADDR_04, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [3][2][0]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [3][2][1]={.addr=BOARD_ADDR_05, .board.x=1, .board.y=0, .board.z=1, .enabled=true},

    [4][0][0]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][0][1]={.addr=BOARD_ADDR_06, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][1][0]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][1][1]={.addr=BOARD_ADDR_07, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [4][2][0]={.addr=BOARD_ADDR_08, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [4][2][1]={.addr=BOARD_ADDR_08, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [5][0][0]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][0][1]={.addr=BOARD_ADDR_06, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][1][0]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][1][1]={.addr=BOARD_ADDR_07, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [5][2][0]={.addr=BOARD_ADDR_08, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [5][2][1]={.addr=BOARD_ADDR_08, .board.x=1, .board.y=0, .board.z=1, .enabled=true},

    [6][0][0]={.addr=BOARD_ADDR_09, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [6][0][1]={.addr=BOARD_ADDR_09, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [6][1][0]={.addr=BOARD_ADDR_10, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [6][1][1]={.addr=BOARD_ADDR_10, .board.x=0, .board.y=0, .board.z=1, .enabled=true},
    [6][2][0]={.addr=BOARD_ADDR_11, .board.x=0, .board.y=0, .board.z=0, .enabled=true},  [6][2][1]={.addr=BOARD_ADDR_11, .board.x=0, .board.y=0, .board.z=1, .enabled=true},

    [7][0][0]={.addr=BOARD_ADDR_09, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [7][0][1]={.addr=BOARD_ADDR_09, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [7][1][0]={.addr=BOARD_ADDR_10, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [7][1][1]={.addr=BOARD_ADDR_10, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
    [7][2][0]={.addr=BOARD_ADDR_11, .board.x=1, .board.y=0, .board.z=0, .enabled=true},  [7][2][1]={.addr=BOARD_ADDR_11, .board.x=1, .board.y=0, .board.z=1, .enabled=true},
  };

	MATRIX_BoardList_t MATRIX_BoardList[MATRIX_NOF_BOARDS] = {
      {.enabled = true, .addr=BOARD_ADDR_00, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_01, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_02, .cmdSent=false},

      {.enabled = true, .addr=BOARD_ADDR_03, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_04, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_05, .cmdSent=false},

      {.enabled = true, .addr=BOARD_ADDR_06, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_07, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_08, .cmdSent=false},

      {.enabled = true, .addr=BOARD_ADDR_09, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_10, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_11, .cmdSent=false},
  };
#else
  #error "unknown matrix"
#endif

#endif /* PL_CONFIG_IS_MASTER */
