/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_FONT && MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_3x5 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_3x5
#include <stdint.h>
#include <stdbool.h>
#include "McuUtility.h"
#include "mfont.h"
#include "matrix.h"
#include "matrixposition.h"
#include "matrixhand.h"

typedef struct MClockChar3x5_t {
  MClock_t digit[MFONT_SIZE_Y_3x5][MFONT_SIZE_X_3x5]; /* a digit is built by 5 (vertical) and 3 (horizontal) clocks */
} MClockChar3x5_t;

/* larger digits (3x5) */
static const MClockChar3x5_t clockDigits3x5[10] = {
    [0].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [1].digit = {
#if 0
      /* old version with using 2 rows
           ^
          /|
           |
           |
           |
      */
        [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][1]={.hands={{.angle=225, .enabled=true },{.angle=180, .enabled=true }}},
        [0][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][0]={.hands={{.angle=225, .enabled=true },{.angle= 45, .enabled=true }}},
        [1][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
#elif 1 /* new version using 3 rows */
        [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][2]={.hands={{.angle=225, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][1]={.hands={{.angle=225, .enabled=true },{.angle= 45, .enabled=true }}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true}}},
        [2][0]={.hands={{.angle= 45, .enabled=true },{.angle= 45, .enabled=true }}},
        [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][2]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true}}},
#else /* new version using 3 rows, but looks like a '7' */
        [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][1]={.hands={{.angle=225, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=45 , .enabled=true },{.angle= 45, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true}}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][2]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true}}},
#endif
    },
    [2].digit = {
        [0][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
    },
    [3].digit = {
        [0][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
    #if PL_CONFIG_USE_EXTENDED_HANDS
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
    #else
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
    #endif
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [4].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][2]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
    #if PL_CONFIG_USE_EXTENDED_HANDS
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
    #else
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
    #endif
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][2]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [5].digit = {
        [0][0]={.hands={{.angle= 90, .enabled=true },{.angle=180, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [6].digit = {
        [0][0]={.hands={{.angle= 90, .enabled=true },{.angle=180, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [2][0]={.hands={{.angle= 90, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
      #else
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
      #endif
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [7].digit = {
        [0][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][2]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [8].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [2][0]={.hands={{.angle= 90, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
      #else
        [2][0]={.hands={{.angle= 90, .enabled=true },{.angle=180, .enabled=true }}},
      #endif
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
      #else
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
      #endif
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [9].digit = {
        [0][0]={.hands={{.angle= 90, .enabled=true },{.angle=180, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
#if PL_CONFIG_USE_EXTENDED_HANDS
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
#else
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
#endif
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
};

static const MClockChar3x5_t clockCharA3x5 =
{ /* A */
    .digit = {
        [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][1]={.hands={{.angle=225, .enabled=true },{.angle=135, .enabled=true }}},
        [0][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][0]={.hands={{.angle= 45, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=315, .enabled=true },{.angle=180, .enabled=true }}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [2][0]={.hands={{.angle= 90, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
      #else
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
      #endif
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
      #else
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
      #endif
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][2]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    }
};

static const MClockChar3x5_t clockCharB3x5 =
{ /* B */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle=135, .enabled=true }}},
        [0][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=315, .enabled=true },{.angle=225, .enabled=true }}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [2][0]={.hands={{.angle= 90, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
      #else
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
      #endif
        [2][1]={.hands={{.angle= 45, .enabled=true },{.angle=135, .enabled=true }}},
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=315, .enabled=true },{.angle=225, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 45, .enabled=true}}},
        [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    }
};

static const MClockChar3x5_t clockCharC3x5 =
{ /* C */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
    }
};

static const MClockChar3x5_t clockCharD3x5 =
{ /* D */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
        [0][1]={.hands={{.angle=270, .enabled= true},{.angle=135, .enabled= true}}},
        [0][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=315, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=225, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 45, .enabled=true }}},
        [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    }
};

static const MClockChar3x5_t clockCharE3x5 =
{ /* E */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
#if PL_CONFIG_USE_EXTENDED_HANDS
        [2][0]={.hands={{.angle= 90, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
#else
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
#endif
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true}}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
    }
};

static const MClockChar3x5_t clockCharF3x5 =
{ /* F */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
#if PL_CONFIG_USE_EXTENDED_HANDS
        [2][0]={.hands={{.angle= 90, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
#else
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
#endif
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true}}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    }
};

static const MClockChar3x5_t clockCharG3x5 =
{ /* G */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true}}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    }
};

static const MClockChar3x5_t clockCharH3x5 =
{ /* H */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][2]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [2][0]={.hands={{.angle= 90, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
      #else
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
      #endif
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
      #else
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
      #endif
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][2]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    }
};

static const MClockChar3x5_t clockCharI3x5 =
{ /* I */
    .digit = {
        [0][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [0][1]={.hands={{.angle=180, .enabled=true },{.angle=270, .enabled=true , .enabled2nd=true}}},
      #else
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true}}},
      #endif
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
        [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [4][1]={.hands={{.angle=  0, .enabled=true },{.angle=270, .enabled=true , .enabled2nd=true}}},
      #else
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true}}},
      #endif
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
    }
};

static const MClockChar3x5_t clockCharJ3x5 =
{ /* J */
    .digit = {
        [0][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [0][1]={.hands={{.angle=180, .enabled=true },{.angle=270, .enabled=true , .enabled2nd=true}}},
      #else
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true}}},
      #endif
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
        [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=315, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true}}},
        [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    }
};

static const MClockChar3x5_t clockCharK3x5 =
{ /* K */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][2]={.hands={{.angle=225, .enabled=true },{.angle=225, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=true },{.angle= 45, .enabled=true }}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [2][0]={.hands={{.angle= 45, .enabled=true },{.angle=  0, .enabled=true , .enabled2nd=true}}},
      #else
        [2][0]={.hands={{.angle= 45, .enabled=true },{.angle=135, .enabled=true}}},
      #endif
        [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=315, .enabled=true },{.angle=135, .enabled=true}}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][2]={.hands={{.angle=315, .enabled=true },{.angle=315, .enabled=true }}},
    }
};

static const MClockChar3x5_t clockCharL3x5 =
{ /* L */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
    }
};

static const MClockChar3x5_t clockCharM3x5 =
{ /* M */
    .digit = {
    [0][0]={.hands={{.angle=180, .enabled=true },{.angle=135, .enabled=true }}},
    [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [0][2]={.hands={{.angle=225, .enabled=true },{.angle=180, .enabled= true}}},
    [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
    [1][1]={.hands={{.angle=315, .enabled=true },{.angle= 45, .enabled=true }}},
    [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
    [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
    [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
    [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
    [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
    [4][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [4][2]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    }
};

static const MClockChar3x5_t clockCharN3x5 =
{ /* N */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][2]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [1][0]={.hands={{.angle=135, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
      #else
        [1][0]={.hands={{.angle=180, .enabled=true },{.angle=  0, .enabled=true }}},
      #endif
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle=315, .enabled=true },{.angle=135, .enabled=true }}},
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true}}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [3][2]={.hands={{.angle=315, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
      #else
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true}}},
      #endif
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][2]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    }
};

static const MClockChar3x5_t clockCharP3x5 =
{ /* P */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
#if PL_CONFIG_USE_EXTENDED_HANDS
        [2][0]={.hands={{.angle= 90, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
#else
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
#endif
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true}}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    }
};

static const MClockChar3x5_t clockCharQ3x5 =
{ /* Q */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=135, .enabled=true },{.angle=135, .enabled=true }}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=315, .enabled=true }}},
    }
};

static const MClockChar3x5_t clockCharR3x5 =
{ /* R */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
#if PL_CONFIG_USE_EXTENDED_HANDS
        [2][0]={.hands={{.angle= 90, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
#else
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
#endif
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=315, .enabled=true },{.angle=135, .enabled=true }}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][2]={.hands={{.angle=315, .enabled=true },{.angle=315, .enabled=true }}},
    }
};

static const MClockChar3x5_t clockCharS3x5 =
{ /* S */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=135, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=315, .enabled=true },{.angle=135, .enabled=true}}},
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=315, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    }
};

static const MClockChar3x5_t clockCharT3x5 =
{ /* T */
    .digit = {
        [0][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [0][1]={.hands={{.angle=180, .enabled=true },{.angle=270, .enabled=true , .enabled2nd=true}}},
      #else
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true}}},
      #endif
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
        [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false }}},
        [4][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true}}},
        [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    }
};

static const MClockChar3x5_t clockCharU3x5 =
{ /* U */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][2]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][2]={.hands={{.angle=180, .enabled=true },{.angle=  0, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=180, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    }
};

static const MClockChar3x5_t clockCharV3x5 =
{ /* V */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][2]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][2]={.hands={{.angle=180, .enabled=true },{.angle=  0, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=135, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=225, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=315, .enabled=true },{.angle= 45, .enabled=true }}},
        [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    }
};

static const MClockChar3x5_t clockCharW3x5 =
{ /* W */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][2]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][2]={.hands={{.angle=180, .enabled=true },{.angle=  0, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=true },{.angle=135, .enabled=true }}},
        [3][2]={.hands={{.angle=180, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 45, .enabled=true }}},
        [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][2]={.hands={{.angle=315, .enabled=true },{.angle=  0, .enabled=true }}},
    }
};

static const MClockChar3x5_t clockCharX3x5 =
{ /* X */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][2]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=135, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=225, .enabled=true }}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [2][1]={.hands={{.angle=315, .enabled=true },{.angle=45, .enabled=true, .enabled2nd=true}}},
      #else
        [2][1]={.hands={{.angle=225, .enabled=true },{.angle=135, .enabled=true }}},
      #endif
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle= 45, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=315, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][2]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    }
};

static const MClockChar3x5_t clockCharY3x5 =
{ /* Y */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][2]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=135, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=225, .enabled=true }}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=325, .enabled=true },{.angle= 45, .enabled=true }}},
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][2]={.hands={{.angle=315, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    }
};

static const MClockChar3x5_t clockCharZ3x5 =
{ /* Z */
    .digit = {
        [0][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=225, .enabled=true }}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false }}},
        [2][1]={.hands={{.angle=225, .enabled=true },{.angle= 45, .enabled=true }}},
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle= 45, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
    }
};

static const MClockChar3x5_t clockCharDegree3x5 =
{ /* ° */
    .digit = {
        [0][0]={.hands={{.angle=200, .enabled=true },{.angle= 70, .enabled=true }}},
        [0][1]={.hands={{.angle=290, .enabled=true },{.angle=160, .enabled=true }}},
        [0][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][0]={.hands={{.angle=340, .enabled=true },{.angle=110, .enabled=true }}},
        [1][1]={.hands={{.angle=250, .enabled=true },{.angle= 20, .enabled=true }}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    }
};

static const MClockChar3x5_t clockCharPlus3x5 =
{ /* + */
    .digit = {
        [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][1]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true}}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    }
};

static const MClockChar3x5_t clockCharMinus3x5 =
{ /* - */
    .digit = {
        [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true}}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    }
};

static const MClockChar3x5_t clockCharDot3x5 =
{ /* . */
    .digit = {
        [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=false}}},
        [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    }
};

static const MClockChar3x5_t clockCharColon3x5 =
{ /* : */
    .digit = {
        [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=false}}},
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=false}}},
        [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    }
};

static const MClockChar3x5_t clockCharExclamation3x5 =
{ /* J */
    .digit = {
        [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][1]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true}}},
        [0][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true}}},
        [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    }
};

static const MClockChar3x5_t clockCharQuestion3x5 =
{ /* ! */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [1][1]={.hands={{.angle=225, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=270, .enabled=true }}},
        [2][0]={.hands={{.angle= 45, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true}}},
        [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    }
};

static const MClockChar3x5_t clockCharPercent3x5 =
{
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [0][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [1][2]={.hands={{.angle=225, .enabled=true },{.angle=225, .enabled=true }}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=225, .enabled=true },{.angle= 45, .enabled=true }}},
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle= 45, .enabled=true },{.angle= 45, .enabled=true }}},
        [3][1]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    }
};

static const MClockChar3x5_t clockCharSpace3x5 =
{ /* <space> */
    .digit = {
        [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    }
};


static void DrawChar3x5(const MClockChar3x5_t *ch, uint8_t xPos, uint8_t yPos) {
  for(int y=0; y<MFONT_SIZE_Y_3x5; y++) { /* every clock row */
    for(int x=0; x<MFONT_SIZE_X_3x5; x++) { /* every clock column */
      MPOS_SetAngleZ0Z1(xPos+x, yPos+y, ch->digit[y][x].hands[0].angle, ch->digit[y][x].hands[1].angle);
      MPOS_SetMoveModeZ0Z1(xPos+x, yPos+y, STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
    #if PL_MATRIX_CONFIG_IS_RGB
      MHAND_HandEnable(xPos+x, yPos+y, 0, ch->digit[y][x].hands[0].enabled);
      MHAND_HandEnable(xPos+x, yPos+y, 1, ch->digit[y][x].hands[1].enabled);
    #endif
    #if PL_CONFIG_USE_EXTENDED_HANDS
      MHAND_2ndHandEnable(xPos+x, yPos+y, 0, ch->digit[y][x].hands[0].enabled2nd);
      MHAND_2ndHandEnable(xPos+x, yPos+y, 1, ch->digit[y][x].hands[1].enabled2nd);
    #endif
    }
  }
}

void MFONT_PrintString3x5(const unsigned char *str, int xPos, int yPos) {
  const MClockChar3x5_t *desc;

  while(*str!='\0') {
    desc = NULL;
    if (*str>='0' && *str<='9') {
      desc = &clockDigits3x5[*str-'0'];
    } else {
      switch(*str) {
        case ' ': desc = &clockCharSpace3x5; break;
        case 'A': desc = &clockCharA3x5; break;
        case 'B': desc = &clockCharB3x5; break;
        case 'C': desc = &clockCharC3x5; break;
        case 'D': desc = &clockCharD3x5; break;
        case 'E': desc = &clockCharE3x5; break;
        case 'F': desc = &clockCharF3x5; break;
        case 'G': desc = &clockCharG3x5; break;
        case 'H': desc = &clockCharH3x5; break;
        case 'I': desc = &clockCharI3x5; break;
        case 'J': desc = &clockCharJ3x5; break;
        case 'K': desc = &clockCharK3x5; break;
        case 'L': desc = &clockCharL3x5; break;
        case 'M': desc = &clockCharM3x5; break;
        case 'N': desc = &clockCharN3x5; break;
        case 'O': desc = &clockDigits3x5[0]; break;
        case 'P': desc = &clockCharP3x5; break;
        case 'Q': desc = &clockCharQ3x5; break;
        case 'R': desc = &clockCharR3x5; break;
        case 'S': desc = &clockCharS3x5; break;
        case 'T': desc = &clockCharT3x5; break;
        case 'U': desc = &clockCharU3x5; break;
        case 'V': desc = &clockCharV3x5; break;
        case 'W': desc = &clockCharW3x5; break;
        case 'X': desc = &clockCharX3x5; break;
        case 'Y': desc = &clockCharY3x5; break;
        case 'Z': desc = &clockCharZ3x5; break;
        case 176: /* '°' */
        case MFONT_CHAR_DEGREE: desc = &clockCharDegree3x5; break;
        case '%': desc = &clockCharPercent3x5; break;
        case '-': desc = &clockCharMinus3x5; break;
        case '+': desc = &clockCharPlus3x5; break;
        case '.': desc = &clockCharDot3x5; break;
        case ':': desc = &clockCharColon3x5; break;
        case '!': desc = &clockCharExclamation3x5; break;
        case '?': desc = &clockCharQuestion3x5; break;
        default: desc = NULL; break;
      }
    }
    if (desc!=NULL && xPos<=MATRIX_NOF_STEPPERS_X-MFONT_SIZE_X_3x5 && yPos<=MATRIX_NOF_STEPPERS_Y-MFONT_SIZE_Y_3x5) {
      DrawChar3x5(desc, xPos, yPos);
    }
    xPos += MFONT_SIZE_X_3x5;
    str++;
  }
}

#endif /* PL_CONFIG_USE_FONT && MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_3x5 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_3x5 */
