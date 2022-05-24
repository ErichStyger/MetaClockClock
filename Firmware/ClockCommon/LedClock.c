/*
 * Copyright (c) 2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_LED_CLOCK
#include <stdbool.h>
#include <stdint.h>
#include "LedClock.h"
#include "NeoPixel.h"
#include "McuTimeDate.h"
#include "matrixpixel.h"

void LedDisp_Clear(void) {
  MPIXEL_ClearAll();
}

int LedDisp_GetWidth(void) {
  return MPIXEL_NOF_X;
}

int LedDisp_GetHeight(void) {
  return MPIXEL_NOF_Y;
}

static void LedDisp_PutPixel(NEO_PixelIdxT x, NEO_PixelIdxT y, NEO_PixelColor color) {
  MPIXEL_SetColor(x, y, 0, NEO_SPLIT_RGB(color));
}

#define LED_CLOCK_CONFIG_USE_SMALL_FONT   (1)

static bool LedClock_doMirror = false;
static NEO_PixelColor LedClock_colorDigits = 0x10;
static NEO_PixelColor LedClock_colorDots = 0x1000;

#if LED_CLOCK_CONFIG_USE_SMALL_FONT
static const uint32_t Numbers8x4_Small[10] = { /* digits 0-9, data byte with two nibbles (rows) */
    /* from MSB to LSB: 4bits for each row:
     * 0     1     2     3     4     5     6     7     8     9
     * 0000  0000  0000  0000  0000  0000  0000  0000  0000  0000
     * 0111  0001  0111  0111  0001  0111  0111  0111  0111  0111
     * 0101  0011  0101  0001  0101  0100  0100  0001  0101  0101
     * 0101  0101  0001  0001  0101  0100  0100  0001  0101  0101
     * 0101  0001  0010  0111  0111  0011  0111  0010  0111  0111
     * 0101  0001  0100  0001  0001  0001  0101  0010  0101  0001
     * 0111  0001  0111  0111  0001  0111  0111  0010  0111  0111
     * 0000  0000  0000  0000  0000  0000  0000  0000  0000  0000
     */
  /* 0 */ 0b00000111010101010101010101110000,
  /* 1 */ 0b00000001001101010001000100010000,
  /* 2 */ 0b00000111010100010010010001110000,
  /* 3 */ 0b00000111000100010111000101110000,
  /* 4 */ 0b00000001010101010111000100010000,
  /* 5 */ 0b00000111010001000011000101110000,
  /* 6 */ 0b00000111010001000111010101110000,
  /* 7 */ 0b00000111000100010010001000100000,
  /* 8 */ 0b00000111010101010111010101110000,
  /* 9 */ 0b00000111010101010111000101110000
};
#else
static const uint32_t Numbers8x4[10] = { /* digits 0-9, data byte with two nibbles (rows) */
    /* from MSB to LSB: 4bits for each row, e.g. for '1':
     *  0010
     *  0110
     *  1010
     *  0010
     *  0010
     *  0010
     *  0010
     *  0010
     */
  /* 0 */ 0b01101001100110011001100110010110,
  /* 1 */ 0b00100110101000100010001000100010,
  /* 2 */ 0b01101001000100100100100010001111,
  /* 3 */ 0b01101001000101100010000110010110,
  /* 4 */ 0b10011001100111110001000100010001,
  /* 5 */ 0b11111000100001100001000110010110,
  /* 6 */ 0b01101001100010001111100110010110,
  /* 7 */ 0b11110001000100100010010001001000,
  /* 8 */ 0b01101001100101101001100110010110,
  /* 9 */ 0b01101001100101110001000110010110
};
#endif

static void LedClock_SetChar8x4Pixels(char ch, NEO_PixelIdxT x0, NEO_PixelIdxT y0, NEO_PixelColor color) {
  uint32_t data;
  int i;

  /* x/y (0/0) is top left corner, with x increasing to the right and y increasing to the bottom */
  if (ch>='0' && ch<='9') { /* check if valid digit */
#if LED_CLOCK_CONFIG_USE_SMALL_FONT
    data = Numbers8x4_Small[(unsigned int)(ch-'0')];
#else
    data = Numbers8x4[(unsigned int)(ch-'0')];
#endif
    for(i=0;i<32;i++) { /* 32bits for each digit */
      if (LedClock_doMirror) {
        if (data&(1<<31)) { /* MSB set? */
          LedDisp_PutPixel(x0+(i%4), y0+7-(i/4), color);
        } else {
          LedDisp_PutPixel(x0+(i%4), y0+7-(i/4), 0);
        }
      } else {
        if (data&(1<<31)) { /* MSB set? */
          LedDisp_PutPixel(x0+(i%4), y0+(i/4), color);
        } else {
          LedDisp_PutPixel(x0+(i%4), y0+(i/4), 0);
        }
      }
      data <<= 1; /* next bit */
    } /* for */
  } /* if number */
}

static void LedClock_PutClockPixels(TIMEREC *time, DATEREC *date, NEO_PixelColor colorDigits, NEO_PixelColor colorDots, bool showHH, bool showMM, bool showSS) {
  int pos;

  colorDigits = NEO_GammaCorrect24(colorDigits);
  colorDots = NEO_GammaCorrect24(colorDots);
  LedDisp_Clear();
  if (LedDisp_GetWidth()==16) {
    /* write hhss */
    pos = 0;
    if (!showHH) {
      pos += 4;
    }
    if (!showMM) {
      pos += 4;
    }
    if (!showSS) {
      pos += 4;
    }
    if (showHH) {
      LedClock_SetChar8x4Pixels('0'+(time->Hour/10), pos, 0, colorDigits);
      pos+=4;
      LedClock_SetChar8x4Pixels('0'+(time->Hour%10), pos, 0, colorDigits);
      pos+=4;
    }
    if (showMM) {
      LedClock_SetChar8x4Pixels('0'+(time->Min/10),  pos, 0, colorDigits);
      pos+=4;
      LedClock_SetChar8x4Pixels('0'+(time->Min%10), pos, 0, colorDigits);
      pos+=4;
    }
    if (showSS) {
      LedClock_SetChar8x4Pixels('0'+(time->Sec/10), pos, 0, colorDigits);
      pos+=4;
      LedClock_SetChar8x4Pixels('0'+(time->Sec%10), pos, 0, colorDigits);
      pos+=4;
    }
  } else if (LedDisp_GetWidth()==24) {
    /* write hour:min:sec with two dots */
    pos = 0;
    if (!showHH) {
      pos += 4;
    }
    if (!showMM) {
      pos += 4;
    }
    if (!showSS) {
      pos += 4;
    }
    if (showHH) {
      LedClock_SetChar8x4Pixels('0'+(time->Hour/10), pos, 0, colorDigits);
      pos+=4;
      LedClock_SetChar8x4Pixels('0'+(time->Hour%10), pos, 0, colorDigits);
      pos+=4;
    }
    if (showMM) {
      LedClock_SetChar8x4Pixels('0'+(time->Min/10),  pos, 0, colorDigits);
      if (showHH) {
        LedDisp_PutPixel(pos, 3, colorDots);
        LedDisp_PutPixel(pos, 5, colorDots);
      }
      pos+=4;
      LedClock_SetChar8x4Pixels('0'+(time->Min%10), pos, 0, colorDigits);
      pos+=4;
    }
    if (showSS) {
      LedClock_SetChar8x4Pixels('0'+(time->Sec/10), pos, 0, colorDigits);
      if (showMM) {
        LedDisp_PutPixel(pos, 3, colorDots);
        LedDisp_PutPixel(pos, 5, colorDots);
      }
      pos+=4;
      LedClock_SetChar8x4Pixels('0'+(time->Sec%10), pos, 0, colorDigits);
      pos+=4;
    }
  } else if (LedDisp_GetHeight()==24) { /* portrait mode */
    pos = 0;
    if (!showHH) {
      pos += 4;
    }
    if (!showMM) {
      pos += 4;
    }
    if (!showSS) {
      pos += 4;
    }
    if (showHH) {
      LedClock_SetChar8x4Pixels('0'+(time->Hour/10), 0, pos, colorDigits);
      LedClock_SetChar8x4Pixels('0'+(time->Hour%10), 4, pos, colorDigits);
    }
    if (showMM) {
      pos += 8;
      LedClock_SetChar8x4Pixels('0'+(time->Min/10), 0, pos, colorDigits);
      LedClock_SetChar8x4Pixels('0'+(time->Min%10), 4, pos, colorDigits);
      if (showHH) {
        LedDisp_PutPixel(3, pos-1, colorDots);
        LedDisp_PutPixel(3, pos, colorDots);
        LedDisp_PutPixel(5, pos-1, colorDots);
        LedDisp_PutPixel(5, pos, colorDots);
      }
    }
    if (showSS) {
      pos += 8;
      LedClock_SetChar8x4Pixels('0'+(time->Sec/10), 0, pos, colorDigits);
      LedClock_SetChar8x4Pixels('0'+(time->Sec%10), 4, pos, colorDigits);
      if (showMM) {
        LedDisp_PutPixel(3, pos-1, colorDots);
        LedDisp_PutPixel(3, pos, colorDots);
        LedDisp_PutPixel(5, pos-1, colorDots);
        LedDisp_PutPixel(5, pos, colorDots);
      }
    }
  }
}

void LedClock_ShowTimeDate(TIMEREC *time, DATEREC *date) {
  LedClock_PutClockPixels(time, date, LedClock_colorDigits, LedClock_colorDots, true, true, false);
}

void LedClock_Deinit(void) {
}

void LedClock_Init(void) {
}

#endif /* PL_CONFIG_USE_LED_CLOCK */
