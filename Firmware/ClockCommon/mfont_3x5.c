/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "mfont.h"
#if MFONT_3x5_AVAILABLE
#include "matrix.h"
#include "matrixposition.h"
#include "matrixhand.h"

typedef struct MClockChar3x5_t {
  MClock_t digit[MFONT_SIZE_Y_3x5][MFONT_SIZE_X_3x5]; /* a digit is built by 5 (vertical) and 3 (horizontal) clocks */
  uint8_t width;
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
  [0].width = 3,
  [1].digit = {
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
  },
  [1].width = 3,
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
  [2].width = 3,
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
  [3].width = 3,
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
  [4].width = 3,
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
  [5].width = 3,
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
  [6].width = 3,
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
  [7].width = 3,
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
  [8].width = 3,
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
  [9].width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
    [4][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  },
  .width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
    [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [4][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  },
  .width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
  },
  .width = 3,
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
    [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][1]={.hands={{.angle=225, .enabled=true },{.angle= 45, .enabled=true }}},
    [2][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [3][0]={.hands={{.angle= 45, .enabled=true },{.angle=180, .enabled=true }}},
    [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
    [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
    [4][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
  },
  .width = 3,
};

static const MClockChar3x5_t clockCharDegree3x5 =
{ /* ° */
  .digit = {
    [0][0]={.hands={{.angle=200, .enabled=true },{.angle= 70, .enabled=true }}},
    [0][1]={.hands={{.angle=290, .enabled=true },{.angle=160, .enabled=true }}},
    [1][0]={.hands={{.angle=340, .enabled=true },{.angle=110, .enabled=true }}},
    [1][1]={.hands={{.angle=250, .enabled=true },{.angle= 20, .enabled=true }}},
    [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [3][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  },
  .width = 2,
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
  },
  .width = 3,
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
  },
  .width = 3,
};

static const MClockChar3x5_t clockCharDot3x5 =
{ /* . */
  .digit = {
    [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [4][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=false}}},
  },
  .width = 1,
};

static const MClockChar3x5_t clockCharColon3x5 =
{ /* : */
  .digit = {
    [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=false}}},
    [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [4][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=false}}},
  },
  .width = 1,
};

static const MClockChar3x5_t clockCharExclamation3x5 =
{ /* J */
  .digit = {
    [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true}}},
    [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
    [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
    [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [4][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
  },
  .width = 1,
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
    [4][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  },
  .width = 3,
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
  },
  .width = 3,
};

static const MClockChar3x5_t clockCharSpace3x5 =
{ /* <space> */
  .digit = {
    [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  },
  .width = 1,
};

static const MClockChar3x5_t *GetCharacterDesc(char ch) {
  const MClockChar3x5_t *desc = NULL;

  if (ch>='0' && ch<='9') {
    desc = &clockDigits3x5[ch-'0'];
  } else {
    switch(ch) {
      case ' ': desc = &clockCharSpace3x5; break;
      case 'a':
      case 'A': desc = &clockCharA3x5; break;
      case 'b':
      case 'B': desc = &clockCharB3x5; break;
      case 'c':
      case 'C': desc = &clockCharC3x5; break;
      case 'd':
      case 'D': desc = &clockCharD3x5; break;
      case 'e':
      case 'E': desc = &clockCharE3x5; break;
      case 'f':
      case 'F': desc = &clockCharF3x5; break;
      case 'g':
      case 'G': desc = &clockCharG3x5; break;
      case 'h':
      case 'H': desc = &clockCharH3x5; break;
      case 'i':
      case 'I': desc = &clockCharI3x5; break;
      case 'j':
      case 'J': desc = &clockCharJ3x5; break;
      case 'k':
      case 'K': desc = &clockCharK3x5; break;
      case 'l':
      case 'L': desc = &clockCharL3x5; break;
      case 'm':
      case 'M': desc = &clockCharM3x5; break;
      case 'n':
      case 'N': desc = &clockCharN3x5; break;
      case 'o':
      case 'O': desc = &clockDigits3x5[0]; break;
      case 'p':
      case 'P': desc = &clockCharP3x5; break;
      case 'q':
      case 'Q': desc = &clockCharQ3x5; break;
      case 'r':
      case 'R': desc = &clockCharR3x5; break;
      case 's':
      case 'S': desc = &clockCharS3x5; break;
      case 't':
      case 'T': desc = &clockCharT3x5; break;
      case 'u':
      case 'U': desc = &clockCharU3x5; break;
      case 'v':
      case 'V': desc = &clockCharV3x5; break;
      case 'w':
      case 'W': desc = &clockCharW3x5; break;
      case 'x':
      case 'X': desc = &clockCharX3x5; break;
      case 'y':
      case 'Y': desc = &clockCharY3x5; break;
      case 'z':
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
  return desc;
}

uint8_t MFONT_GetCharWidth3x5(char ch) {
  const MClockChar3x5_t *desc = GetCharacterDesc(ch);
  return desc->width;
}

static void DrawChar3x5(const MClockChar3x5_t *ch, uint8_t xPos, uint8_t yPos) {
  for(int y=0; y<MFONT_SIZE_Y_3x5; y++) { /* every clock row */
    for(int x=0; x<ch->width; x++) { /* every clock column */
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

void MFONT_PrintString3x5(const unsigned char *str, int xPos, int yPos, uint32_t color) {
  const MClockChar3x5_t *desc;

  while(*str!='\0') {
    desc = GetCharacterDesc(*str);
    if (xPos<=MATRIX_NOF_STEPPERS_X-desc->width && yPos<=MATRIX_NOF_STEPPERS_Y-MFONT_SIZE_Y_3x5) {
      DrawChar3x5(desc, xPos, yPos);
    }
    str++; /* got to next char */
    if (str[0]=='\\' && str[1]=='n') { /* newline?`*/
      xPos = 0;
      yPos += MFONT_SIZE_Y_2x3;
      str += 2; /* skip "\n" */
    } else {
      xPos += desc->width;
    }
  }
}

#endif /* MFONT_3x5_AVAILABLE */
