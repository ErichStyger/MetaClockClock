/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_IS_MASTER
#include <stdint.h>
#include <stdbool.h>
#include "mfont.h"
#include "matrix.h"
#include "matrixposition.h"
#include "matrixhand.h"

typedef struct MHand_t {
  int16_t angle; /* absolute angle for clock hand position */
  bool enabled;  /* if hand is enabled or not */
#if PL_CONFIG_USE_EXTENDED_HANDS
  bool enabled2nd;
#endif
} MHand_t;

typedef struct MClock_t {
  MHand_t hands[2]; /* each clock has two hands */
} MClock_t;

typedef struct MClockChar2x3_t {
  MClock_t digit[3][2]; /* a digit is built by 3 (vertical) and 2 (horizontal) clocks */
} MClockChar2x3_t;

/* smaller digits with 2x3. Angle of 225 (MPOS_ANGLE_HIDE) is a special one ('disabled') */
static const MClockChar2x3_t clockDigits2x3[10] = {
    [0].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [1].digit = {
        [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][1]={.hands={{.angle=225, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle= 45, .enabled=true },{.angle= 45, .enabled=true }}},
        [1][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [2].digit = {
        [0][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
    },
    [3].digit = {
        [0][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
    #if PL_CONFIG_USE_EXTENDED_HANDS
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true, .enabled2nd=true}}},
    #else
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    #endif
        [2][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [4].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [0][1]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
      #else
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
      #endif
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [5].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [6].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [1][0]={.hands={{.angle=90,.enabled=true},{.angle=0, .enabled=true, .enabled2nd=true}}},
      #else
        [1][0]={.hands={{.angle=0, .enabled=true },{.angle=180, .enabled=true }}},
      #endif
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [7].digit = {
        [0][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [8].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [1][0]={.hands={{.angle= 90, .enabled=true },{.angle=180, .enabled=true, .enabled2nd=true}}},
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true, .enabled2nd=true}}},
      #else
        [1][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
      #endif
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [9].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
      #if PL_CONFIG_USE_EXTENDED_HANDS
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle=0, .enabled=true, .enabled2nd=true}}},
      #else
        [1][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
      #endif
        [2][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    }
};

static const MClockChar2x3_t clockCharDegree2x3 =
{ .digit = { /* ° */
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
    [0][1]={.hands={{.angle=270, .enabled= true},{.angle=180, .enabled= true}}},
    [1][0]={.hands={{.angle=  0, .enabled= true},{.angle= 90, .enabled= true}}},
    [1][1]={.hands={{.angle=270, .enabled= true},{.angle=  0, .enabled= true}}},
    [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  }
};

static const MClockChar2x3_t clockCharPercent2x3 =
{ .digit = { /* % */
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
    [0][1]={.hands={{.angle=225, .enabled= true},{.angle=225, .enabled= true}}},
    [1][0]={.hands={{.angle= 45, .enabled= true},{.angle= 45, .enabled= true}}},
    [1][1]={.hands={{.angle=270, .enabled= true},{.angle=  0, .enabled= true}}},
    [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  }
};

static const MClockChar2x3_t clockCharC2x3 =
{ .digit = { /* C */
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
    [0][1]={.hands={{.angle=270, .enabled= true},{.angle=270, .enabled= true}}},
    [1][0]={.hands={{.angle=  0, .enabled= true},{.angle=180, .enabled= true}}},
    [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle=  0, .enabled= true},{.angle= 90, .enabled= true}}},
    [2][1]={.hands={{.angle=270, .enabled= true},{.angle=270, .enabled= true}}},
  }
};

static const MClockChar2x3_t clockCharL2x3 =
{ .digit = { /* C */
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle=180, .enabled= true}}},
    [0][1]={.hands={{.angle=225, .enabled= true},{.angle=225, .enabled= true}}},
    [1][0]={.hands={{.angle=  0, .enabled= true},{.angle=180, .enabled= true}}},
    [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle=  0, .enabled= true},{.angle= 90, .enabled= true}}},
    [2][1]={.hands={{.angle=270, .enabled= true},{.angle=270, .enabled= true}}},
  }
};

static const MClockChar2x3_t clockCharH2x3 =
{ .digit = { /* H */
    [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
    [0][1]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
#if PL_CONFIG_USE_EXTENDED_HANDS
    [1][0]={.hands={{.angle= 90, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
    [1][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true, .enabled2nd=true}}},
#else
    [1][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
    [1][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
#endif
    [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
  }
};

static const MClockChar2x3_t clockCharSpace2x3 =
{ .digit = { /* <space> */
    [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  }
};

#if MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5
typedef struct MClockChar3x5_t {
  MClock_t digit[5][3]; /* a digit is built by 3 (vertical) and 2 (horizontal) clocks */
} MClockChar3x5_t;

/* larger digits (3x5) */
static const MClockChar3x5_t clockDigits3x5[10] = {
    [0].digit = {
        [0][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
        [0][1]={.hands={{.angle=270, .enabled= true},{.angle= 90, .enabled= true}}},
        [0][2]={.hands={{.angle=270, .enabled= true},{.angle=180, .enabled= true}}},
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
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [0][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
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
#endif

#if MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5
static void DrawChar3x5(const MClockChar3x5_t *ch, uint8_t xPos, uint8_t yPos) {
  for(int y=0; y<5; y++) { /* every clock row */
    for(int x=0; x<3; x++) { /* every clock column */
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
#endif /* MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5 */

static void DrawChar2x3(const MClockChar2x3_t *ch, uint8_t xPos, uint8_t yPos) {
  for(int y=0; y<3; y++) { /* every clock row */
    for(int x=0; x<2; x++) { /* every clock column */
      MPOS_SetAngleZ0Z1(xPos+x, yPos+y, ch->digit[y][x].hands[0].angle, ch->digit[y][x].hands[1].angle);
      MPOS_SetMoveModeZ0Z1(xPos+x, yPos+y, STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
    #if PL_MATRIX_CONFIG_IS_RGB
      /* should pass brightness as parameter! */
      #if 0 /* \todo */ /* does not work yet */
      MATRIX_StartHandDimming(xPos+x, yPos+y, 0, ch->digit[y][x].hands[0].enabled?0xff:0);
      MATRIX_StartHandDimming(xPos+x, yPos+y, 1, ch->digit[y][x].hands[1].enabled?0xff:0);
      #endif
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

#if MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5
static void PrintString3x5(const unsigned char *str, int xPos, int yPos) {
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
        case MFONT_CHAR_DEGREE: desc = &clockCharDegree3x5; break;
        case '%': desc = &clockCharPercent3x5; break;
        default: desc = NULL; break;
      }
    }
    if (desc!=NULL && xPos<=MATRIX_NOF_STEPPERS_X-MFONT_SIZE_X_3x5 && yPos<=MATRIX_NOF_STEPPERS_Y-MFONT_SIZE_Y_3x5) {
      DrawChar3x5(desc, xPos, yPos);
    }
    xPos += 3;
    str++;
  }
}
#endif /* MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5 */

static void PrintString2x3(const unsigned char *str, int xPos, int yPos) {
  const MClockChar2x3_t *desc;

  while(*str!='\0') {
    desc = NULL;
    if (*str>='0' && *str<='9') {
      desc = &clockDigits2x3[*str-'0'];
    } else {
      switch(*str) {
        case ' ': desc = &clockCharSpace2x3; break;
        case 'L': desc = &clockCharL2x3; break;
        case 'C': desc = &clockCharC2x3; break;
        case 'H': desc = &clockCharH2x3; break;
        case MFONT_CHAR_DEGREE: desc = &clockCharDegree2x3; break;
        case '%': desc = &clockCharPercent2x3; break;
        default: desc = NULL; break;
      }
    }
    if (desc!=NULL && xPos<=MATRIX_NOF_STEPPERS_X-MFONT_SIZE_X_2x3 && yPos<=MATRIX_NOF_STEPPERS_Y-MFONT_SIZE_Y_2x3) {
      DrawChar2x3(desc, xPos, yPos);
    }
    xPos += 2;
    str++;
  }
}

void MFONT_PrintString(const unsigned char *str, int xPos, int yPos, MFONT_Size_e size) {
  if (size==MFONT_SIZE_2x3) {
    PrintString2x3(str, xPos, yPos);
  }
#if MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5
  else if (size==MFONT_SIZE_3x5) {
    PrintString3x5(str, xPos, yPos);
  }
#endif
}

#endif /* PL_CONFIG_IS_MASTER */
