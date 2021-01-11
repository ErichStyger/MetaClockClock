/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_FONT
#include <stdint.h>
#include <stdbool.h>
#include "McuUtility.h"
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

#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_2x3 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_2x3
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
    [1][1]={.hands={{.angle=270, .enabled= true},{.angle=180, .enabled= true}}},
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
#endif /* MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_2x3 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_2x3 */

#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_3x5 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_3x5
typedef struct MClockChar3x5_t {
  MClock_t digit[5][3]; /* a digit is built by 3 (vertical) and 2 (horizontal) clocks */
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

#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_4x5 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_4x5
typedef struct MClockChar4x5_t {
  MClock_t digit[5][4]; /* a digit is built by 4 (vertical) and 2 (horizontal) clocks */
} MClockChar4x5_t;

/* outline digits (4x5) */
static const MClockChar4x5_t clockDigits4x5[10] = {
    [0].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][3]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][3]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][3]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [3][3]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][3]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [1].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [0][3]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][3]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][3]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][3]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true}}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][3]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [2].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][3]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
        [1][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][3]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [2][3]={.hands={{.angle=  0, .enabled=true },{.angle=225, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][2]={.hands={{.angle=270, .enabled=true },{.angle= 45, .enabled=true }}},
        [3][3]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][3]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [3].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][3]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][3]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [2][3]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [3][3]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][3]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [4].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [0][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [0][3]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][2]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][3]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [2][3]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [3][3]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][2]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][3]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [5].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][3]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][3]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][3]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [3][3]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][3]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [6].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][3]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][3]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][3]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
        [3][3]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][3]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [7].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][3]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][2]={.hands={{.angle=270, .enabled=true },{.angle=225, .enabled=true }}},
        [1][3]={.hands={{.angle=  0, .enabled=true },{.angle=225, .enabled=true }}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle= 45, .enabled=true },{.angle=180, .enabled=true }}},
        [2][2]={.hands={{.angle= 45, .enabled=true },{.angle=180, .enabled=true }}},
        [2][3]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][3]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][3]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    },
    [8].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][3]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][3]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
        [2][3]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [3][3]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][3]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [9].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][3]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle= 90, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][2]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
        [1][3]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [2][3]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [3][3]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][3]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
};
#endif

#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_4x5 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_4x5
static void DrawChar4x5(const MClockChar4x5_t *ch, uint8_t xPos, uint8_t yPos) {
  for(int y=0; y<MFONT_SIZE_Y_4x5; y++) { /* every clock row */
    for(int x=0; x<MFONT_SIZE_X_4x5; x++) { /* every clock column */
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


#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_3x5 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_3x5
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
#endif /* MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5 */

#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_2x3 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_2x3
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
#endif /* MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_2x3 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_2x3 */

#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_3x5 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_3x5
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
        case 176: /* '°' */
        case MFONT_CHAR_DEGREE: desc = &clockCharDegree3x5; break;
        case '%': desc = &clockCharPercent3x5; break;
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
#endif /* MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5 */

#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_4x5 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_4x5
static void PrintString4x5(const unsigned char *str, int xPos, int yPos) {
  const MClockChar4x5_t *desc;

  while(*str!='\0') {
    desc = NULL;
    if (*str>='0' && *str<='9') {
      desc = &clockDigits4x5[*str-'0'];
    } else {
      /* only digits have been implemented */
    }
    if (desc!=NULL && xPos<=MATRIX_NOF_STEPPERS_X-MFONT_SIZE_X_4x5 && yPos<=MATRIX_NOF_STEPPERS_Y-MFONT_SIZE_Y_4x5) {
      DrawChar4x5(desc, xPos, yPos);
    }
    xPos += MFONT_SIZE_X_4x5;
    str++;
  }
}
#endif /* MATRIX_NOF_STEPPERS_X>=12 && MATRIX_NOF_STEPPERS_Y>=5 */

#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_2x3 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_2x3
static void PrintString2x3(const unsigned char *str, int xPos, int yPos) {
  const MClockChar2x3_t *desc;

  while(*str!='\0') {
    desc = NULL;
    if (*str>='0' && *str<='9') {
      desc = &clockDigits2x3[*str-'0'];
    } else {
      switch(*str) {
        case ' ': desc = &clockCharSpace2x3; break;
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
        default: desc = NULL; break;
      }
    }
    if (desc!=NULL && xPos<=MATRIX_NOF_STEPPERS_X-MFONT_SIZE_X_2x3 && yPos<=MATRIX_NOF_STEPPERS_Y-MFONT_SIZE_Y_2x3) {
      DrawChar2x3(desc, xPos, yPos);
    }
    xPos += MFONT_SIZE_X_2x3;
    str++;
  }
}
#endif /* MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_2x3 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_2x3 */

void MFONT_PrintString(const unsigned char *str, int xPos, int yPos, MFONT_Size_e font) {
#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_2x3 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_2x3
  if (font==MFONT_SIZE_2x3) {
    PrintString2x3(str, xPos, yPos);
    return;
  }
#endif
#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_3x5 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_3x5
  if (font==MFONT_SIZE_3x5) {
    PrintString3x5(str, xPos, yPos);
    return;
  }
#endif
#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_4x5 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_4x5
  if (font==MFONT_SIZE_4x5) {
    PrintString4x5(str, xPos, yPos);
    return;
  }
#endif
}

#if PL_CONFIG_IS_MASTER
uint8_t MFONT_ShowFramedText(uint8_t x, uint8_t y, unsigned char *text, MFONT_Size_e font, bool withBorder, bool wait) {
  int xSize, ySize;

  MATRIX_SetMoveDelayAll(2);
  MPOS_SetAngleAll(MPOS_ANGLE_HIDE);
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(false);
#endif
  switch(font) {
    default:
      return ERR_FAILED;
    case MFONT_SIZE_2x3:
      xSize = McuUtility_strlen((char*)text)*MFONT_SIZE_X_2x3;
      ySize = MFONT_SIZE_Y_2x3;
      break;
    case MFONT_SIZE_3x5:
      xSize = McuUtility_strlen((char*)text)*MFONT_SIZE_X_3x5;
      ySize = MFONT_SIZE_Y_3x5;
      break;
    case MFONT_SIZE_4x5:
      xSize = McuUtility_strlen((char*)text)*MFONT_SIZE_X_4x5;
      ySize = MFONT_SIZE_Y_4x5;
      break;
  } /* switch */
  if (withBorder && xSize<=MATRIX_NOF_STEPPERS_X-1 && ySize<=MATRIX_NOF_STEPPERS_Y-1) { /* only if enough space for border */
    MATRIX_DrawRectangle(0, 0, MATRIX_NOF_STEPPERS_X, MATRIX_NOF_STEPPERS_Y);
  }
  /* calculate text start position */
  x = (MATRIX_NOF_STEPPERS_X-xSize)/2;
  if (x<0) {
    x = 0;
  }
  y = (MATRIX_NOF_STEPPERS_Y-ySize)/2;
  if (y<0) {
    y = 0;
  }
  MFONT_PrintString(text, x, y, font);
  return MATRIX_SendToRemoteQueueExecuteAndWait(wait);
}
#endif /* PL_CONFIG_IS_MASTER */

#if PL_CONFIG_USE_SHELL
static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  McuShell_SendStatusStr((unsigned char*)"mfont", (unsigned char*)"Matrix font status\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  fonts", (unsigned char*)"", io->stdOut);
#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_2x3 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_2x3
  McuShell_SendStr((unsigned char*)"2x3 ", io->stdOut);
#endif
#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_3x5 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_3x5
  McuShell_SendStr((unsigned char*)"3x5 ", io->stdOut);
#endif
#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_4x5 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_4x5
  McuShell_SendStr((unsigned char*)"4x5 ", io->stdOut);
#endif
  McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
  return ERR_OK;
}
#endif /* PL_CONFIG_USE_SHELL */

#if PL_CONFIG_USE_SHELL
static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"mfont", (unsigned char*)"Group of matrix font commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
#if PL_CONFIG_IS_MASTER
  McuShell_SendHelpStr((unsigned char*)"  text <f> <xy> <txt>", (unsigned char*)"Write text with font (e.g. 2x3) at position\r\n", io->stdOut);
#endif
  return ERR_OK;
}
#endif /* PL_CONFIG_USE_SHELL */

#if PL_CONFIG_USE_SHELL
uint8_t MFONT_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "mfont help")==0) {
    *handled = true;
    return PrintHelp(io);
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "mfont status")==0)) {
    *handled = true;
    return PrintStatus(io);
#if PL_CONFIG_IS_MASTER
  } else if (McuUtility_strncmp((char*)cmd, "mfont text ", sizeof("mfont text ")-1)==0) {
    const unsigned char *p;
    uint8_t xPos, yPos;
    MFONT_Size_e font;

    *handled = TRUE;
    p = cmd + sizeof("mfont text ")-1;
    MFONT_ParseFontName(&p, &font);
    if (font==MFONT_SIZE_ERROR) {
      return ERR_FAILED;
    }
    if (   McuUtility_ScanDecimal8uNumber(&p, &xPos)==ERR_OK && xPos<MATRIX_NOF_STEPPERS_X
        && McuUtility_ScanDecimal8uNumber(&p, &yPos)==ERR_OK && yPos<MATRIX_NOF_STEPPERS_Y
        )
    {
      uint8_t buf[8];

      McuUtility_SkipSpaces(&p);
      if (McuUtility_ReadEscapedName(p, buf, sizeof(buf), NULL, NULL, NULL)!=ERR_OK) {
        return ERR_FAILED;
      }
      MATRIX_SetMoveDelayZ0Z1All(2, 2);
      MFONT_PrintString(buf, xPos, yPos, font);
      return MATRIX_SendToRemoteQueueExecuteAndWait(true);
    } else {
      return ERR_FAILED;
    }
#endif /* PL_CONFIG_IS_MASTER */
  }
  return ERR_OK;
}
#endif /* PL_CONFIG_USE_SHELL */

void MFONT_FontToStr(MFONT_Size_e font, unsigned char *buf, size_t bufSize) {
  switch(font) {
    case MFONT_SIZE_2x3: McuUtility_strcpy(buf, bufSize, (unsigned char*)"2x3"); break;
    case MFONT_SIZE_3x5: McuUtility_strcpy(buf, bufSize, (unsigned char*)"3x5"); break;
    case MFONT_SIZE_4x5: McuUtility_strcpy(buf, bufSize, (unsigned char*)"4x5"); break;
    case MFONT_SIZE_ERROR:
    default: McuUtility_strcpy(buf, bufSize, (unsigned char*)"ERROR"); break;
  }
}

void MFONT_ParseFontName(const unsigned char **p, MFONT_Size_e *font) {
  if (McuUtility_strncmp((char*)*p, "2x3", sizeof("2x3")-1)==0) {
    *font = MFONT_SIZE_2x3;
    *p += sizeof("2x3")-1;
  } else if (McuUtility_strncmp((char*)*p, "3x5", sizeof("3x5")-1)==0) {
    *font = MFONT_SIZE_3x5;
    *p += sizeof("3x5")-1;
  } else if (McuUtility_strncmp((char*)*p, "4x5", sizeof("4x5")-1)==0) {
    *font = MFONT_SIZE_4x5;
    *p += sizeof("4x5")-1;
  } else {
    *font = MFONT_SIZE_ERROR;
  }
}

#endif /* PL_CONFIG_USE_FONT */
