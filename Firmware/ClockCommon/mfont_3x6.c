/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "mfont.h"
#if PL_CONFIG_USE_FONT && MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_3x6 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_3x6
#include "matrix.h"
#include "matrixposition.h"
#include "matrixhand.h"

typedef struct MClockChar3x6_t {
  MClock_t digit[MFONT_SIZE_Y_3x6][MFONT_SIZE_X_3x6]; /* a digit is built by 6 (vertical) and 3 (horizontal) clocks */
  uint8_t width;
} MClockChar3x6_t;

/* larger digits (3x6) */
static const MClockChar3x6_t clockDigits3x6[10] = {
    [0].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [5][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [5][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [5][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [0].width = 3,
    [1].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true}}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=224, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][1]={.hands={{.angle=  0, .enabled=true}, {.angle=180, .enabled=true }}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [5][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [5][1]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [5][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [1].width = 3,
    [2].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][1]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [5][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [5][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [5][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [2].width = 3,
    [3].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [5][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [5][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [5][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [3].width = 3,
    [4].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [5][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [5][1]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [5][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [4].width = 3,
    [5].digit = {
        [0][0]={.hands={{.angle= 90, .enabled=true },{.angle=180, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [5][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [5][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [5][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [5].width = 3,
    [6].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [2][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][1]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [5][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [5][1]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
        [5][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [6].width = 3,
    [7].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle=270, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [1][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][1]={.hands={{.angle=225, .enabled=true },{.angle=  0, .enabled=true }}},
        [2][2]={.hands={{.angle=225, .enabled=true },{.angle=  0, .enabled=true }}},
        [3][0]={.hands={{.angle=180, .enabled=true },{.angle= 45, .enabled=true }}},
        [3][1]={.hands={{.angle=180, .enabled=true },{.angle= 45, .enabled=true }}},
        [3][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][1]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][2]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [5][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [5][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [5][2]={.hands={{.angle=225, .enabled=false },{.angle=225, .enabled=false }}},
    },
    [7].width = 3,
    [8].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=135, .enabled=true }}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=225, .enabled=true }}},
        [3][0]={.hands={{.angle=180, .enabled=true },{.angle= 45, .enabled=true }}},
        [3][1]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [3][2]={.hands={{.angle=305, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [5][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true}}},
        [5][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true}}},
        [5][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true}}},
    },
    [8].width = 3,
    [9].digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [0][2]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [1][1]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [1][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][1]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [2][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [3][1]={.hands={{.angle=270, .enabled=true },{.angle=180, .enabled=true }}},
        [3][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=180, .enabled=true },{.angle= 90, .enabled=true }}},
        [4][1]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][2]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [5][0]={.hands={{.angle=  0, .enabled=true },{.angle= 90, .enabled=true }}},
        [5][1]={.hands={{.angle=270, .enabled=true },{.angle= 90, .enabled=true }}},
        [5][2]={.hands={{.angle=270, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    [9].width = 3,
};


static const MClockChar3x6_t clockCharSpace3x6 =
{ /* <space> */
    .digit = {
        [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [5][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    },
    .width = 1,
};

static const MClockChar3x6_t clockCharColon3x6 =
{ /* : */
    .digit = {
        [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
        [1][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [3][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [4][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [5][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    },
    .width = 1,
};

static const MClockChar3x6_t clockCharExclamation3x6 =
{ /* ! */
    .digit = {
        [0][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [1][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [2][0]={.hands={{.angle=  0, .enabled=true },{.angle=180, .enabled=true }}},
        [3][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
        [4][0]={.hands={{.angle=180, .enabled=true },{.angle=180, .enabled=true }}},
        [5][0]={.hands={{.angle=  0, .enabled=true },{.angle=  0, .enabled=true }}},
    },
    .width = 1,
};

static const MClockChar3x6_t *GetCharacterDesc(char ch) {
  const MClockChar3x6_t *desc = NULL;

  if (ch>='0' && ch<='9') {
    desc = &clockDigits3x6[ch-'0'];
  } else {
    switch(ch) {
      case ' ': desc = &clockCharSpace3x6; break;
      case ':': desc = &clockCharColon3x6; break;
      case '!': desc = &clockCharExclamation3x6; break;
      default:
        desc = NULL;
        break;
    }
  }
  return desc;
}

uint8_t MFONT_GetCharWidth3x6(char ch) {
  const MClockChar3x6_t *desc = GetCharacterDesc(ch);
  return desc->width;
}

static void DrawChar3x6(const MClockChar3x6_t *ch, uint8_t xPos, uint8_t yPos) {
  for(int y=0; y<MFONT_SIZE_Y_3x6; y++) { /* every clock row */
    for(int x=0; x<ch->width; x++) { /* every clock column */
      MPOS_SetAngleZ0Z1(xPos+x, yPos+y, ch->digit[y][x].hands[0].angle, ch->digit[y][x].hands[1].angle);
      MPOS_SetMoveModeZ0Z1(xPos+x, yPos+y, STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
    #if PL_MATRIX_CONFIG_IS_RGB
      MHAND_HandEnable(xPos+x, yPos+y, 0, ch->digit[y][x].hands[0].enabled);
      MHAND_HandEnable(xPos+x, yPos+y, 1, ch->digit[y][x].hands[1].enabled);
    #endif
    }
  }
}

void MFONT_PrintString3x6(const unsigned char *str, int xPos, int yPos) {
  const MClockChar3x6_t *desc;

  while(*str!='\0') {
    desc = GetCharacterDesc(*str);
    if (desc!=NULL && xPos<=MATRIX_NOF_STEPPERS_X-desc->width && yPos<=MATRIX_NOF_STEPPERS_Y-MFONT_SIZE_Y_3x6) {
      DrawChar3x6(desc, xPos, yPos);
    }
    xPos += desc->width;
    str++;
  }
}

#endif /* PL_CONFIG_USE_FONT && MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_3x6 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_3x6 */
