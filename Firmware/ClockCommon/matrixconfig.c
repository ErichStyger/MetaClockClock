/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "matrixconfig.h"
#include "matrix.h"

#if PL_CONFIG_IS_MASTER

#if PL_MATRIX_CONFIG_IS_8x3
  const MatrixClock_t clockMatrix[MATRIX_NOF_CLOCKS_X][MATRIX_NOF_CLOCKS_Y] = /* information about how the clocks are organized */
  {
    [0][0]={.addr=BOARD_ADDR_00, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [1][0]={.addr=BOARD_ADDR_00, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [2][0]={.addr=BOARD_ADDR_00, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [3][0]={.addr=BOARD_ADDR_00, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [0][1]={.addr=BOARD_ADDR_01, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [1][1]={.addr=BOARD_ADDR_01, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [2][1]={.addr=BOARD_ADDR_01, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [3][1]={.addr=BOARD_ADDR_01, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [0][2]={.addr=BOARD_ADDR_02, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [1][2]={.addr=BOARD_ADDR_02, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [2][2]={.addr=BOARD_ADDR_02, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [3][2]={.addr=BOARD_ADDR_02, .nr=0, .board.x=3, .board.y=0, .enabled=true},

    [4][0]={.addr=BOARD_ADDR_05, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [5][0]={.addr=BOARD_ADDR_05, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [6][0]={.addr=BOARD_ADDR_05, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [7][0]={.addr=BOARD_ADDR_05, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [4][1]={.addr=BOARD_ADDR_06, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [5][1]={.addr=BOARD_ADDR_06, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [6][1]={.addr=BOARD_ADDR_06, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [7][1]={.addr=BOARD_ADDR_06, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [4][2]={.addr=BOARD_ADDR_07, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [5][2]={.addr=BOARD_ADDR_07, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [6][2]={.addr=BOARD_ADDR_07, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [7][2]={.addr=BOARD_ADDR_07, .nr=0, .board.x=3, .board.y=0, .enabled=true},
  };

  MATRIX_BoardList_t MATRIX_BoardList[MATRIX_NOF_BOARDS] = {
      {.enabled = true, .addr=BOARD_ADDR_00, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_01, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_02, .cmdSent=false},

      {.enabled = true, .addr=BOARD_ADDR_05, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_06, .cmdSent=false},
      {.enabled = true, .addr=BOARD_ADDR_07, .cmdSent=false},
  };
#elif PL_MATRIX_CONFIG_IS_12x5 && PL_CONFIG_IS_NEW_MODULAR
  const MatrixClock_t clockMatrix[MATRIX_NOF_CLOCKS_X][MATRIX_NOF_CLOCKS_Y] = /* information about how the clocks are organized */
  {
    [0][0]={.addr=BOARD_ADDR_00, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [1][0]={.addr=BOARD_ADDR_00, .nr=1, .board.x=1, .board.y=0, .enabled=true},
    [0][1]={.addr=BOARD_ADDR_01, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [1][1]={.addr=BOARD_ADDR_01, .nr=1, .board.x=1, .board.y=0, .enabled=true},
    [0][2]={.addr=BOARD_ADDR_02, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [1][2]={.addr=BOARD_ADDR_02, .nr=1, .board.x=1, .board.y=0, .enabled=true},
    [0][3]={.addr=BOARD_ADDR_03, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [1][3]={.addr=BOARD_ADDR_03, .nr=1, .board.x=1, .board.y=0, .enabled=true},
    [0][4]={.addr=BOARD_ADDR_04, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [1][4]={.addr=BOARD_ADDR_04, .nr=1, .board.x=1, .board.y=0, .enabled=true},

    [2][0]={.addr=BOARD_ADDR_05, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [3][0]={.addr=BOARD_ADDR_05, .nr=1, .board.x=1, .board.y=0, .enabled=true},
    [2][1]={.addr=BOARD_ADDR_06, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [3][1]={.addr=BOARD_ADDR_06, .nr=1, .board.x=1, .board.y=0, .enabled=true},
    [2][2]={.addr=BOARD_ADDR_07, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [3][2]={.addr=BOARD_ADDR_07, .nr=1, .board.x=1, .board.y=0, .enabled=true},
    [2][3]={.addr=BOARD_ADDR_08, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [3][3]={.addr=BOARD_ADDR_08, .nr=1, .board.x=1, .board.y=0, .enabled=true},
    [2][4]={.addr=BOARD_ADDR_09, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [3][4]={.addr=BOARD_ADDR_09, .nr=1, .board.x=1, .board.y=0, .enabled=true},

    [4][0]={.addr=BOARD_ADDR_10, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [5][0]={.addr=BOARD_ADDR_10, .nr=1, .board.x=1, .board.y=0, .enabled=true},
    [4][1]={.addr=BOARD_ADDR_11, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [5][1]={.addr=BOARD_ADDR_11, .nr=1, .board.x=1, .board.y=0, .enabled=true},
    [4][2]={.addr=BOARD_ADDR_12, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [5][2]={.addr=BOARD_ADDR_12, .nr=1, .board.x=1, .board.y=0, .enabled=true},
    [4][3]={.addr=BOARD_ADDR_13, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [5][3]={.addr=BOARD_ADDR_13, .nr=1, .board.x=1, .board.y=0, .enabled=true},
    [4][4]={.addr=BOARD_ADDR_14, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [5][4]={.addr=BOARD_ADDR_14, .nr=1, .board.x=1, .board.y=0, .enabled=true},

    [6][0]={.addr=BOARD_ADDR_15, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [7][0]={.addr=BOARD_ADDR_15, .nr=1, .board.x=1, .board.y=0, .enabled=true},
    [6][1]={.addr=BOARD_ADDR_16, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [7][1]={.addr=BOARD_ADDR_16, .nr=1, .board.x=1, .board.y=0, .enabled=true},
    [6][2]={.addr=BOARD_ADDR_17, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [7][2]={.addr=BOARD_ADDR_17, .nr=1, .board.x=1, .board.y=0, .enabled=true},
    [6][3]={.addr=BOARD_ADDR_18, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [7][3]={.addr=BOARD_ADDR_18, .nr=1, .board.x=1, .board.y=0, .enabled=true},
    [6][4]={.addr=BOARD_ADDR_19, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [7][4]={.addr=BOARD_ADDR_19, .nr=1, .board.x=1, .board.y=0, .enabled=true},

    [8][0]={.addr=BOARD_ADDR_20, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [9][0]={.addr=BOARD_ADDR_25, .nr=0, .board.x=0, .board.y=0, .enabled=true},
    [8][1]={.addr=BOARD_ADDR_21, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [9][1]={.addr=BOARD_ADDR_26, .nr=0, .board.x=0, .board.y=0, .enabled=true},
    [8][2]={.addr=BOARD_ADDR_22, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [9][2]={.addr=BOARD_ADDR_27, .nr=0, .board.x=0, .board.y=0, .enabled=true},
    [8][3]={.addr=BOARD_ADDR_23, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [9][3]={.addr=BOARD_ADDR_28, .nr=0, .board.x=0, .board.y=0, .enabled=true},
    [8][4]={.addr=BOARD_ADDR_24, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [9][4]={.addr=BOARD_ADDR_29, .nr=0, .board.x=0, .board.y=0, .enabled=true},

    [10][0]={.addr=BOARD_ADDR_30, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [11][0]={.addr=BOARD_ADDR_37, .nr=0, .board.x=0, .board.y=0, .enabled=true},
    [10][1]={.addr=BOARD_ADDR_31, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [11][1]={.addr=BOARD_ADDR_31, .nr=1, .board.x=1, .board.y=0, .enabled=true},
    [10][2]={.addr=BOARD_ADDR_32, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [11][2]={.addr=BOARD_ADDR_32, .nr=1, .board.x=1, .board.y=0, .enabled=true},
    [10][3]={.addr=BOARD_ADDR_33, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [11][3]={.addr=BOARD_ADDR_34, .nr=0, .board.x=0, .board.y=0, .enabled=true},
    [10][4]={.addr=BOARD_ADDR_35, .nr=0, .board.x=0, .board.y=0, .enabled=true}, [11][4]={.addr=BOARD_ADDR_36, .nr=0, .board.x=0, .board.y=0, .enabled=true},
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
#elif PL_MATRIX_CONFIG_IS_12x5
  const MatrixClock_t clockMatrix[MATRIX_NOF_CLOCKS_X][MATRIX_NOF_CLOCKS_Y] = /* information about how the clocks are organized */
  {
    [0][0]={.addr=BOARD_ADDR_00, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [1][0]={.addr=BOARD_ADDR_00, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [2][0]={.addr=BOARD_ADDR_00, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [3][0]={.addr=BOARD_ADDR_00, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [0][1]={.addr=BOARD_ADDR_01, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [1][1]={.addr=BOARD_ADDR_01, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [2][1]={.addr=BOARD_ADDR_01, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [3][1]={.addr=BOARD_ADDR_01, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [0][2]={.addr=BOARD_ADDR_02, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [1][2]={.addr=BOARD_ADDR_02, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [2][2]={.addr=BOARD_ADDR_02, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [3][2]={.addr=BOARD_ADDR_02, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [0][3]={.addr=BOARD_ADDR_03, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [1][3]={.addr=BOARD_ADDR_03, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [2][3]={.addr=BOARD_ADDR_03, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [3][3]={.addr=BOARD_ADDR_03, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [0][4]={.addr=BOARD_ADDR_04, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [1][4]={.addr=BOARD_ADDR_04, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [2][4]={.addr=BOARD_ADDR_04, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [3][4]={.addr=BOARD_ADDR_04, .nr=0, .board.x=3, .board.y=0, .enabled=true},

    [4][0]={.addr=BOARD_ADDR_05, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [5][0]={.addr=BOARD_ADDR_05, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [6][0]={.addr=BOARD_ADDR_05, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [7][0]={.addr=BOARD_ADDR_05, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [4][1]={.addr=BOARD_ADDR_06, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [5][1]={.addr=BOARD_ADDR_06, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [6][1]={.addr=BOARD_ADDR_06, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [7][1]={.addr=BOARD_ADDR_06, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [4][2]={.addr=BOARD_ADDR_07, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [5][2]={.addr=BOARD_ADDR_07, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [6][2]={.addr=BOARD_ADDR_07, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [7][2]={.addr=BOARD_ADDR_07, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [4][3]={.addr=BOARD_ADDR_08, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [5][3]={.addr=BOARD_ADDR_08, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [6][3]={.addr=BOARD_ADDR_08, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [7][3]={.addr=BOARD_ADDR_08, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [4][4]={.addr=BOARD_ADDR_09, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [5][4]={.addr=BOARD_ADDR_09, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [6][4]={.addr=BOARD_ADDR_09, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [7][4]={.addr=BOARD_ADDR_09, .nr=0, .board.x=3, .board.y=0, .enabled=true},

    [8][0]={.addr=BOARD_ADDR_10, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [9][0]={.addr=BOARD_ADDR_10, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [10][0]={.addr=BOARD_ADDR_10, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [11][0]={.addr=BOARD_ADDR_10, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [8][1]={.addr=BOARD_ADDR_11, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [9][1]={.addr=BOARD_ADDR_11, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [10][1]={.addr=BOARD_ADDR_11, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [11][1]={.addr=BOARD_ADDR_11, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [8][2]={.addr=BOARD_ADDR_12, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [9][2]={.addr=BOARD_ADDR_12, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [10][2]={.addr=BOARD_ADDR_12, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [11][2]={.addr=BOARD_ADDR_12, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [8][3]={.addr=BOARD_ADDR_13, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [9][3]={.addr=BOARD_ADDR_13, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [10][3]={.addr=BOARD_ADDR_13, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [11][3]={.addr=BOARD_ADDR_13, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [8][4]={.addr=BOARD_ADDR_14, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [9][4]={.addr=BOARD_ADDR_14, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [10][4]={.addr=BOARD_ADDR_14, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [11][4]={.addr=BOARD_ADDR_14, .nr=0, .board.x=3, .board.y=0, .enabled=true},
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
#elif PL_MATRIX_CONFIG_IS_16x9
  const MatrixClock_t clockMatrix[MATRIX_NOF_CLOCKS_X][MATRIX_NOF_CLOCKS_Y] = /* information about how the clocks are organized */
  {
    [0][0]={.addr=BOARD_ADDR_00, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [1][0]={.addr=BOARD_ADDR_00, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [2][0]={.addr=BOARD_ADDR_00, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [3][0]={.addr=BOARD_ADDR_00, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [0][1]={.addr=BOARD_ADDR_01, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [1][1]={.addr=BOARD_ADDR_01, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [2][1]={.addr=BOARD_ADDR_01, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [3][1]={.addr=BOARD_ADDR_01, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [0][2]={.addr=BOARD_ADDR_02, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [1][2]={.addr=BOARD_ADDR_02, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [2][2]={.addr=BOARD_ADDR_02, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [3][2]={.addr=BOARD_ADDR_02, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [0][3]={.addr=BOARD_ADDR_03, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [1][3]={.addr=BOARD_ADDR_03, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [2][3]={.addr=BOARD_ADDR_03, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [3][3]={.addr=BOARD_ADDR_03, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [0][4]={.addr=BOARD_ADDR_04, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [1][4]={.addr=BOARD_ADDR_04, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [2][4]={.addr=BOARD_ADDR_04, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [3][4]={.addr=BOARD_ADDR_04, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [0][5]={.addr=BOARD_ADDR_05, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [1][5]={.addr=BOARD_ADDR_05, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [2][5]={.addr=BOARD_ADDR_05, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [3][5]={.addr=BOARD_ADDR_05, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [0][6]={.addr=BOARD_ADDR_06, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [1][6]={.addr=BOARD_ADDR_06, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [2][6]={.addr=BOARD_ADDR_06, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [3][6]={.addr=BOARD_ADDR_06, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [0][7]={.addr=BOARD_ADDR_07, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [1][7]={.addr=BOARD_ADDR_07, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [2][7]={.addr=BOARD_ADDR_07, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [3][7]={.addr=BOARD_ADDR_07, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [0][8]={.addr=BOARD_ADDR_08, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [1][8]={.addr=BOARD_ADDR_08, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [2][8]={.addr=BOARD_ADDR_08, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [3][8]={.addr=BOARD_ADDR_08, .nr=0, .board.x=3, .board.y=0, .enabled=true},

    [4][0]={.addr=BOARD_ADDR_09, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [5][0]={.addr=BOARD_ADDR_09, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [6][0]={.addr=BOARD_ADDR_09, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [7][0]={.addr=BOARD_ADDR_09, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [4][1]={.addr=BOARD_ADDR_10, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [5][1]={.addr=BOARD_ADDR_10, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [6][1]={.addr=BOARD_ADDR_10, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [7][1]={.addr=BOARD_ADDR_10, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [4][2]={.addr=BOARD_ADDR_11, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [5][2]={.addr=BOARD_ADDR_11, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [6][2]={.addr=BOARD_ADDR_11, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [7][2]={.addr=BOARD_ADDR_11, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [4][3]={.addr=BOARD_ADDR_12, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [5][3]={.addr=BOARD_ADDR_12, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [6][3]={.addr=BOARD_ADDR_12, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [7][3]={.addr=BOARD_ADDR_12, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [4][4]={.addr=BOARD_ADDR_13, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [5][4]={.addr=BOARD_ADDR_13, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [6][4]={.addr=BOARD_ADDR_13, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [7][4]={.addr=BOARD_ADDR_13, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [4][5]={.addr=BOARD_ADDR_14, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [5][5]={.addr=BOARD_ADDR_14, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [6][5]={.addr=BOARD_ADDR_14, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [7][5]={.addr=BOARD_ADDR_14, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [4][6]={.addr=BOARD_ADDR_15, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [5][6]={.addr=BOARD_ADDR_15, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [6][6]={.addr=BOARD_ADDR_15, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [7][6]={.addr=BOARD_ADDR_15, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [4][7]={.addr=BOARD_ADDR_16, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [5][7]={.addr=BOARD_ADDR_16, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [6][7]={.addr=BOARD_ADDR_16, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [7][7]={.addr=BOARD_ADDR_16, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [4][8]={.addr=BOARD_ADDR_17, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [5][8]={.addr=BOARD_ADDR_17, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [6][8]={.addr=BOARD_ADDR_17, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [7][8]={.addr=BOARD_ADDR_17, .nr=0, .board.x=3, .board.y=0, .enabled=true},

    [8][0]={.addr=BOARD_ADDR_18, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [9][0]={.addr=BOARD_ADDR_18, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [10][0]={.addr=BOARD_ADDR_18, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [11][0]={.addr=BOARD_ADDR_18, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [8][1]={.addr=BOARD_ADDR_19, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [9][1]={.addr=BOARD_ADDR_19, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [10][1]={.addr=BOARD_ADDR_19, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [11][1]={.addr=BOARD_ADDR_19, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [8][2]={.addr=BOARD_ADDR_20, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [9][2]={.addr=BOARD_ADDR_20, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [10][2]={.addr=BOARD_ADDR_20, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [11][2]={.addr=BOARD_ADDR_20, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [8][3]={.addr=BOARD_ADDR_21, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [9][3]={.addr=BOARD_ADDR_21, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [10][3]={.addr=BOARD_ADDR_21, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [11][3]={.addr=BOARD_ADDR_21, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [8][4]={.addr=BOARD_ADDR_22, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [9][4]={.addr=BOARD_ADDR_22, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [10][4]={.addr=BOARD_ADDR_22, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [11][4]={.addr=BOARD_ADDR_22, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [8][5]={.addr=BOARD_ADDR_23, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [9][5]={.addr=BOARD_ADDR_23, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [10][5]={.addr=BOARD_ADDR_23, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [11][5]={.addr=BOARD_ADDR_23, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [8][6]={.addr=BOARD_ADDR_24, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [9][6]={.addr=BOARD_ADDR_24, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [10][6]={.addr=BOARD_ADDR_24, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [11][6]={.addr=BOARD_ADDR_24, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [8][7]={.addr=BOARD_ADDR_25, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [9][7]={.addr=BOARD_ADDR_25, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [10][7]={.addr=BOARD_ADDR_25, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [11][7]={.addr=BOARD_ADDR_25, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [8][8]={.addr=BOARD_ADDR_26, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [9][8]={.addr=BOARD_ADDR_26, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [10][8]={.addr=BOARD_ADDR_26, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [11][8]={.addr=BOARD_ADDR_26, .nr=0, .board.x=3, .board.y=0, .enabled=true},

    [12][0]={.addr=BOARD_ADDR_27, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [13][0]={.addr=BOARD_ADDR_27, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [14][0]={.addr=BOARD_ADDR_27, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [15][0]={.addr=BOARD_ADDR_27, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [12][1]={.addr=BOARD_ADDR_28, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [13][1]={.addr=BOARD_ADDR_28, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [14][1]={.addr=BOARD_ADDR_28, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [15][1]={.addr=BOARD_ADDR_28, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [12][2]={.addr=BOARD_ADDR_29, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [13][2]={.addr=BOARD_ADDR_29, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [14][2]={.addr=BOARD_ADDR_29, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [15][2]={.addr=BOARD_ADDR_29, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [12][3]={.addr=BOARD_ADDR_30, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [13][3]={.addr=BOARD_ADDR_30, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [14][3]={.addr=BOARD_ADDR_30, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [15][3]={.addr=BOARD_ADDR_30, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [12][4]={.addr=BOARD_ADDR_31, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [13][4]={.addr=BOARD_ADDR_31, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [14][4]={.addr=BOARD_ADDR_31, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [15][4]={.addr=BOARD_ADDR_31, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [12][5]={.addr=BOARD_ADDR_32, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [13][5]={.addr=BOARD_ADDR_32, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [14][5]={.addr=BOARD_ADDR_32, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [15][5]={.addr=BOARD_ADDR_32, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [12][6]={.addr=BOARD_ADDR_33, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [13][6]={.addr=BOARD_ADDR_33, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [14][6]={.addr=BOARD_ADDR_33, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [15][6]={.addr=BOARD_ADDR_33, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [12][7]={.addr=BOARD_ADDR_34, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [13][7]={.addr=BOARD_ADDR_34, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [14][7]={.addr=BOARD_ADDR_34, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [15][7]={.addr=BOARD_ADDR_34, .nr=0, .board.x=3, .board.y=0, .enabled=true},
    [12][8]={.addr=BOARD_ADDR_35, .nr=3, .board.x=0, .board.y=0, .enabled=true}, [13][8]={.addr=BOARD_ADDR_35, .nr=2, .board.x=1, .board.y=0, .enabled=true}, [14][8]={.addr=BOARD_ADDR_35, .nr=1, .board.x=2, .board.y=0, .enabled=true}, [15][8]={.addr=BOARD_ADDR_35, .nr=0, .board.x=3, .board.y=0, .enabled=true},
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
#else
  #error "unknown matrix"
#endif

#endif /* PL_CONFIG_IS_MASTER */
