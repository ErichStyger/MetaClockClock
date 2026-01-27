/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_FONT && MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_2x3 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_2x3
#include "mfont.h"
#include "matrix.h"
#include "matrixposition.h"
#include "matrixhand.h"

typedef struct MClockChar2x3_t {
  MClock_t digit[MFONT_SIZE_Y_2x3][MFONT_SIZE_X_2x3]; /* a digit is built by 3 (vertical) and 2 (horizontal) clocks */
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

static const MClockChar2x3_t clockCharDegree2x3 = /* ° */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
    [0][1]={.hands={{.angle=270, .enabled= true},{.angle=180, .enabled= true}}},
    [1][0]={.hands={{.angle=  0, .enabled= true},{.angle= 90, .enabled= true}}},
    [1][1]={.hands={{.angle=270, .enabled= true},{.angle=  0, .enabled= true}}},
    [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  }
};

static const MClockChar2x3_t clockCharPercent2x3 = /* % */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
    [0][1]={.hands={{.angle=225, .enabled= true},{.angle=225, .enabled= true}}},
    [1][0]={.hands={{.angle= 45, .enabled= true},{.angle= 45, .enabled= true}}},
    [1][1]={.hands={{.angle=270, .enabled= true},{.angle=  0, .enabled= true}}},
    [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  }
};

static const MClockChar2x3_t clockCharA2x3 = /* A */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
    [0][1]={.hands={{.angle=270, .enabled= true},{.angle=180, .enabled= true}}},
    [1][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
    [1][1]={.hands={{.angle=270, .enabled= true},{.angle=180, .enabled= true}}},
    [2][0]={.hands={{.angle=  0, .enabled= true},{.angle=  0, .enabled= true}}},
    [2][1]={.hands={{.angle=  0, .enabled= true},{.angle=  0, .enabled= true}}},
  }
};

static const MClockChar2x3_t clockCharB2x3 = /* B */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
    [0][1]={.hands={{.angle=270, .enabled= true},{.angle=180, .enabled= true}}},
    [1][0]={.hands={{.angle=  0, .enabled= true},{.angle=180, .enabled= true}}},
    [1][1]={.hands={{.angle=270, .enabled= true},{.angle=  0, .enabled= true}}},
    [2][0]={.hands={{.angle=  0, .enabled= true},{.angle= 90, .enabled= true}}},
    [2][1]={.hands={{.angle=270, .enabled= true},{.angle=  0, .enabled= true}}},
  }
};

static const MClockChar2x3_t clockCharb2x3 = /* b */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle=180, .enabled= true}}},
    [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][0]={.hands={{.angle=  0, .enabled= true},{.angle=180, .enabled= true}}},
    [1][1]={.hands={{.angle=270, .enabled= true},{.angle=180, .enabled= true}}},
    [2][0]={.hands={{.angle=  0, .enabled= true},{.angle= 90, .enabled= true}}},
    [2][1]={.hands={{.angle=270, .enabled= true},{.angle=  0, .enabled= true}}},
  }
};

static const MClockChar2x3_t clockCharC2x3 = /* C */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
    [0][1]={.hands={{.angle=270, .enabled= true},{.angle=270, .enabled= true}}},
    [1][0]={.hands={{.angle=  0, .enabled= true},{.angle=180, .enabled= true}}},
    [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle=  0, .enabled= true},{.angle= 90, .enabled= true}}},
    [2][1]={.hands={{.angle=270, .enabled= true},{.angle=270, .enabled= true}}},
  }
};

static const MClockChar2x3_t clockCharc2x3 = /* c */
{ .digit = {
    [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
    [1][1]={.hands={{.angle=270, .enabled= true},{.angle=270, .enabled= true}}},
    [2][0]={.hands={{.angle=  0, .enabled= true},{.angle= 90, .enabled= true}}},
    [2][1]={.hands={{.angle=270, .enabled= true},{.angle=270, .enabled= true}}},
  }
};

static const MClockChar2x3_t clockChard2x3 = /* d */
{ .digit = {
    [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [0][1]={.hands={{.angle=180, .enabled= true},{.angle=180, .enabled= true}}},
    [1][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
    [1][1]={.hands={{.angle=  0, .enabled= true},{.angle=180, .enabled= true}}},
    [2][0]={.hands={{.angle=  0, .enabled= true},{.angle= 90, .enabled= true}}},
    [2][1]={.hands={{.angle=270, .enabled= true},{.angle=  0, .enabled= true}}},
  }
};

static const MClockChar2x3_t clockCharD2x3 = /* D */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle=120, .enabled= true}}},
    [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][0]={.hands={{.angle=  0, .enabled= true},{.angle=180, .enabled= true}}},
    [1][1]={.hands={{.angle=330, .enabled= true},{.angle=210, .enabled= true}}},
    [2][0]={.hands={{.angle=  0, .enabled= true},{.angle= 60, .enabled= true}}},
    [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  }
};

static const MClockChar2x3_t clockCharE2x3 = /* E */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
    [0][1]={.hands={{.angle=270, .enabled= true},{.angle=270, .enabled= true}}},
    [1][0]={.hands={{.angle=  0, .enabled= true},{.angle= 90, .enabled= true}}},
    [1][1]={.hands={{.angle=270, .enabled= true},{.angle=270, .enabled= true}}},
    [2][0]={.hands={{.angle=  0, .enabled= true},{.angle= 90, .enabled= true}}},
    [2][1]={.hands={{.angle=270, .enabled= true},{.angle=270, .enabled= true}}},
  }
};

static const MClockChar2x3_t clockCharF2x3 = /* F */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
    [0][1]={.hands={{.angle=270, .enabled= true},{.angle=270, .enabled= true}}},
    [1][0]={.hands={{.angle=  0, .enabled= true},{.angle= 90, .enabled= true}}},
    [1][1]={.hands={{.angle=270, .enabled= true},{.angle=270, .enabled= true}}},
    [2][0]={.hands={{.angle=  0, .enabled= true},{.angle=  0, .enabled= true}}},
    [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  }
};

static const MClockChar2x3_t clockCharG2x3 = /* G */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
    [0][1]={.hands={{.angle=270, .enabled= true},{.angle=270, .enabled= true}}},
    [1][0]={.hands={{.angle=180, .enabled= true},{.angle=  0, .enabled= true}}},
    [1][1]={.hands={{.angle=270, .enabled= true},{.angle=180, .enabled= true}}},
    [2][0]={.hands={{.angle=  0, .enabled= true},{.angle= 90, .enabled= true}}},
    [2][1]={.hands={{.angle=270, .enabled= true},{.angle=  0, .enabled= true}}},
  }
};

static const MClockChar2x3_t clockCharH2x3 = /* H */
{ .digit = {
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

static const MClockChar2x3_t clockCharI2x3 = /* I */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle=180, .enabled= true}}},
    [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][0]={.hands={{.angle=180, .enabled= true},{.angle=  0, .enabled= true}}},
    [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle=  0, .enabled= true},{.angle=  0, .enabled= true}}},
    [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  }
};

static const MClockChar2x3_t clockCharJ2x3 = /* J */
{ .digit = {
    [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [0][1]={.hands={{.angle=180, .enabled= true},{.angle=180, .enabled= true}}},
    [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][1]={.hands={{.angle=  0, .enabled= true},{.angle=180, .enabled= true}}},
    [2][0]={.hands={{.angle=315, .enabled= true},{.angle= 90, .enabled= true}}},
    [2][1]={.hands={{.angle=270, .enabled= true},{.angle=  0, .enabled= true}}},
  }
};

static const MClockChar2x3_t clockCharK2x3 = /* K */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle=180, .enabled= true}}},
    [0][1]={.hands={{.angle=225, .enabled= true},{.angle=225, .enabled= true}}},
    [1][0]={.hands={{.angle= 45, .enabled= true},{.angle=135, .enabled= true}}},
    [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle=  0, .enabled= true},{.angle=  0, .enabled= true}}},
    [2][1]={.hands={{.angle=315, .enabled= true},{.angle=315, .enabled= true}}},
  }
};

static const MClockChar2x3_t clockCharL2x3 = /* L */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle=180, .enabled= true}}},
    [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][0]={.hands={{.angle=  0, .enabled= true},{.angle=180, .enabled= true}}},
    [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle=  0, .enabled= true},{.angle= 90, .enabled= true}}},
    [2][1]={.hands={{.angle=270, .enabled= true},{.angle=270, .enabled= true}}},
  }
};

static const MClockChar2x3_t clockCharM2x3 = /* M */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled=true },{.angle=135, .enabled=true }}},
    [0][1]={.hands={{.angle=225, .enabled=true },{.angle=180, .enabled=true }}},
    [1][0]={.hands={{.angle=180, .enabled=true },{.angle=  0, .enabled=true }}},
    [1][1]={.hands={{.angle=180, .enabled=true },{.angle=  0, .enabled=true }}},
    [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
  }
};

static const MClockChar2x3_t clockCharN2x3 = /* N */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled=true },{.angle=135, .enabled=true }}},
    [0][1]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
    [1][0]={.hands={{.angle=180, .enabled=true },{.angle=  0, .enabled=true }}},
    [1][1]={.hands={{.angle=315, .enabled=true },{.angle=180, .enabled=true }}},
    [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
  }
};

static const MClockChar2x3_t clockCharP2x3 = /* P */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
    [0][1]={.hands={{.angle=270, .enabled= true},{.angle=180, .enabled= true}}},
    [1][0]={.hands={{.angle=180, .enabled= true},{.angle=  0, .enabled= true}}},
    [1][1]={.hands={{.angle=270, .enabled= true},{.angle=  0, .enabled= true}}},
    [2][0]={.hands={{.angle=  0, .enabled= true},{.angle=  0, .enabled= true}}},
    [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  }
};

static const MClockChar2x3_t clockCharQ2x3 = /* Q */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
    [0][1]={.hands={{.angle=270, .enabled= true},{.angle=180, .enabled= true}}},
    [1][0]={.hands={{.angle=180, .enabled= true},{.angle=  0, .enabled= true}}},
    [1][1]={.hands={{.angle=180, .enabled= true},{.angle=  0, .enabled= true}}},
    [2][0]={.hands={{.angle=  0, .enabled= true},{.angle= 90, .enabled= true}}},
    [2][1]={.hands={{.angle=270, .enabled= true},{.angle=315, .enabled= true}}},
  }
};

static const MClockChar2x3_t clockCharR2x3 = /* R */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
    [0][1]={.hands={{.angle=270, .enabled= true},{.angle=180, .enabled= true}}},
    [1][0]={.hands={{.angle=180, .enabled= true},{.angle=  0, .enabled= true}}},
    [1][1]={.hands={{.angle=270, .enabled= true},{.angle=  0, .enabled= true}}},
    [2][0]={.hands={{.angle=  0, .enabled= true},{.angle=  0, .enabled= true}}},
    [2][1]={.hands={{.angle=315, .enabled= true},{.angle=315, .enabled= true}}},
  }
};

static const MClockChar2x3_t clockCharS2x3 = /* S */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled= true},{.angle= 90, .enabled= true}}},
    [0][1]={.hands={{.angle=270, .enabled= true},{.angle=270, .enabled= true}}},
    [1][0]={.hands={{.angle=  0, .enabled= true},{.angle= 90, .enabled= true}}},
    [1][1]={.hands={{.angle=270, .enabled= true},{.angle=180, .enabled= true}}},
    [2][0]={.hands={{.angle= 90, .enabled= true},{.angle= 90, .enabled= true}}},
    [2][1]={.hands={{.angle=270, .enabled= true},{.angle=  0, .enabled= true}}},
  }
};

static const MClockChar2x3_t clockChart2x3 = /* t */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
    [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
    [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
    [2][1]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
  }
};

static const MClockChar2x3_t clockCharU2x3 = /* U */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
    [0][1]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
    [1][0]={.hands={{.angle=180, .enabled=true },{.angle=  0, .enabled=true }}},
    [1][1]={.hands={{.angle=180, .enabled=true },{.angle=  0, .enabled=true }}},
    [2][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
    [2][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
  }
};

static const MClockChar2x3_t clockCharV2x3 = /* V */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
    [0][1]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
    [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=135, .enabled=true }}},
    [1][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
    [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][1]={.hands={{.angle=315, .enabled=true },{.angle=  0, .enabled=true }}},
  }
};

static const MClockChar2x3_t clockCharW2x3 = /* W */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
    [0][1]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
    [1][0]={.hands={{.angle=180, .enabled=true },{.angle=  0, .enabled=true }}},
    [1][1]={.hands={{.angle=180, .enabled=true },{.angle=  0, .enabled=true }}},
    [2][0]={.hands={{.angle=  0, .enabled=true },{.angle= 45, .enabled=true }}},
    [2][1]={.hands={{.angle=315, .enabled=true },{.angle=  0, .enabled=true }}},
  }
};

/* X */
static const MClockChar2x3_t clockCharx2x3 = /* x */
{ .digit = {
    [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][0]={.hands={{.angle=135, .enabled=true },{.angle=135, .enabled=true }}},
    [1][1]={.hands={{.angle=225, .enabled=true },{.angle=225, .enabled=true }}},
    [2][0]={.hands={{.angle= 45, .enabled=true },{.angle= 45, .enabled=true }}},
    [2][1]={.hands={{.angle=315, .enabled=true },{.angle=315, .enabled=true }}},
  }
};

static const MClockChar2x3_t clockCharY2x3 = /* Y */
{ .digit = {
    [0][0]={.hands={{.angle=135, .enabled=true },{.angle=135, .enabled=true }}},
    [0][1]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
    [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][1]={.hands={{.angle=315, .enabled=true },{.angle=180, .enabled=true }}},
    [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
  }
};

static const MClockChar2x3_t clockCharZ2x3 = /* Z */
{ .digit = {
    [0][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
    [0][1]={.hands={{.angle=270, .enabled=true },{.angle=210, .enabled=true }}},
    [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle= 30, .enabled=true },{.angle= 90, .enabled=true }}},
    [2][1]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
  }
};

static const MClockChar2x3_t clockCharSpace2x3 =  /* <space> */
{ .digit = {
    [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  }
};

static const MClockChar2x3_t clockCharComma2x3 =  /* ',' */
{ .digit = {
    [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle=225, .enabled=true },{.angle=225, .enabled=true }}},
    [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  }
};

static const MClockChar2x3_t clockCharDot2x3 =  /* '.' */
{ .digit = {
    [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  }
};

static const MClockChar2x3_t clockCharColon2x3 =  /* ':' */
{ .digit = {
    [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  }
};

static const MClockChar2x3_t clockCharSemicolon2x3 =  /* ';' */
{ .digit = {
    [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle=225, .enabled=true },{.angle=225, .enabled=true }}},
    [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  }
};

static const MClockChar2x3_t clockCharExclamation2x3 =  /* '!' */
{ .digit = {
    [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
    [0][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    [1][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    [2][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  }
};

static void DrawChar2x3(const MClockChar2x3_t *ch, uint8_t xPos, uint8_t yPos) {
  for(int y=0; y<MFONT_SIZE_Y_2x3; y++) { /* every clock row */
    for(int x=0; x<MFONT_SIZE_X_2x3; x++) { /* every clock column */
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

void MFONT_PrintString2x3(const unsigned char *str, int xPos, int yPos) {
  const MClockChar2x3_t *desc;

  while(*str!='\0') {
    desc = NULL;
    if (*str>='0' && *str<='9') {
      desc = &clockDigits2x3[*str-'0'];
    } else {
      switch(*str) {
        case 'A': desc = &clockCharA2x3; break;
        case 'b': desc = &clockCharb2x3; break;
        case 'B': desc = &clockCharB2x3; break;
        case 'c': desc = &clockCharc2x3; break;
        case 'C': desc = &clockCharC2x3; break;
        case 'D': desc = &clockCharD2x3; break;
        case 'd': desc = &clockChard2x3; break;
        case 'E': desc = &clockCharE2x3; break;
        case 'F': desc = &clockCharF2x3; break;
        case 'G': desc = &clockCharG2x3; break;
        case 'H': desc = &clockCharH2x3; break;
        case 'I': desc = &clockCharI2x3; break;
        case 'J': desc = &clockCharJ2x3; break;
        case 'K': desc = &clockCharK2x3; break;
        case 'L': desc = &clockCharL2x3; break;
        case 'M': desc = &clockCharM2x3; break;
        case 'N': desc = &clockCharN2x3; break;
        case 'O': desc = &clockDigits2x3[0]; break;
        case 'P': desc = &clockCharP2x3; break;
        case 'Q': desc = &clockCharQ2x3; break;
        case 'R': desc = &clockCharR2x3; break;
        case 'S': desc = &clockCharS2x3; break;
        case 'T':
        case 't': desc = &clockChart2x3; break;
        case 'U': desc = &clockCharU2x3; break;
        case 'V': desc = &clockCharV2x3; break;
        case 'W': desc = &clockCharW2x3; break;
        case 'X':
        case 'x': desc = &clockCharx2x3; break;
        case 'Y': desc = &clockCharY2x3; break;
        case 'Z': desc = &clockCharZ2x3; break;
        case 176: /* '°' */
        case MFONT_CHAR_DEGREE: desc = &clockCharDegree2x3; break;
        case '%': desc = &clockCharPercent2x3; break;
        case ',': desc = &clockCharComma2x3; break;
        case '.': desc = &clockCharDot2x3; break;
        case ':': desc = &clockCharColon2x3; break;
        case ';': desc = &clockCharSemicolon2x3; break;
        case '!': desc = &clockCharExclamation2x3; break;

        case ' ':
        default:
          desc = &clockCharSpace2x3; break;
          break;
      }
    }
    if (desc!=NULL && xPos<=MATRIX_NOF_STEPPERS_X-MFONT_SIZE_X_2x3 && yPos<=MATRIX_NOF_STEPPERS_Y-MFONT_SIZE_Y_2x3) {
      DrawChar2x3(desc, xPos, yPos);
    }
    xPos += MFONT_SIZE_X_2x3;
    str++;
  }
}

#endif /* PL_CONFIG_USE_FONT && MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_2x3 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_2x3 */
