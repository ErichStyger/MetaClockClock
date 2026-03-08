/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "mfont.h"
#if MFONT_4x5_AVAILABLE
#include "matrix.h"
#include "matrixposition.h"
#include "matrixhand.h"

typedef struct MClockChar4x5_t {
  MClock_t digit[MFONT_SIZE_Y_4x5][MFONT_SIZE_X_4x5]; /* a digit is built by 4 (vertical) and 5 (horizontal) clocks */
  uint8_t width;
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
  [0].width = 4,
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
  [1].width = 4,
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
  [2].width = 4,
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
  [3].width = 4,
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
  [4].width = 4,
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
  [5].width = 4,
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
  [6].width = 4,
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
  [7].width = 4,
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
  [8].width = 4,
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
  [9].width = 4,
};

static const MClockChar4x5_t clockCharSpace4x5 =  /* ' ' */
{ .digit = {
    [0][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [1][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [2][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [3][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
    [4][0]={.hands={{.angle=225, .enabled=false},{.angle=225, .enabled=false}}},
  },
  .width = 1,
};

static const MClockChar4x5_t *GetCharacterDesc(char ch) {
  const MClockChar4x5_t *desc = NULL;

  if (ch>='0' && ch<='9') {
    desc = &clockDigits4x5[ch-'0'];
  } else {
    switch(ch) {
      case ' ': desc = &clockCharSpace4x5; break;
      default: desc = NULL; break;
    }
  }
  return desc;
}

uint8_t MFONT_GetCharWidth4x5(char ch) {
  const MClockChar4x5_t *desc = GetCharacterDesc(ch);
  return desc->width;
}

void DrawChar4x5(const MClockChar4x5_t *ch, uint8_t xPos, uint8_t yPos) {
  for(int y=0; y<MFONT_SIZE_Y_4x5; y++) { /* every clock row */
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

void MFONT_PrintString4x5(const unsigned char *str, int xPos, int yPos) {
  const MClockChar4x5_t *desc;

  while(*str!='\0') {
    desc = GetCharacterDesc(*str);
    if (xPos<=MATRIX_NOF_STEPPERS_X-desc->width && yPos<=MATRIX_NOF_STEPPERS_Y-MFONT_SIZE_Y_4x5) {
      DrawChar4x5(desc, xPos, yPos);
    }
    xPos += desc->width;
    str++;
  }
}

#endif /* MFONT_4x5_AVAILABLE */
