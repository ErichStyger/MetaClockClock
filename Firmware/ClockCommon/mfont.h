/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MFONT_H_
#define MFONT_H_

#include "platform.h"
#include "McuShell.h"
#include <stddef.h>

typedef enum {
  MFONT_SIZE_2x3,
  MFONT_SIZE_3x5,
  MFONT_SIZE_4x5,
  MFONT_SIZE_ERROR, /* sentinal, used for errors */
} MFONT_Size_e;

#define MFONT_SIZE_X_2x3   2  /* number of columns needed */
#define MFONT_SIZE_Y_2x3   3  /* number of rows needed */

#define MFONT_SIZE_X_3x5   3  /* number of columns needed */
#define MFONT_SIZE_Y_3x5   5  /* number of rows needed */

#define MFONT_SIZE_X_4x5   4  /* number of columns needed */
#define MFONT_SIZE_Y_4x5   5  /* number of rows needed */

/* special character support */
#define MFONT_CHAR_DEGREE  '~' /* placeholder, need to use a 8bit ASCII code */
#define MFONT_STR_DEGREE   "~" /* placeholder, need to use a 8bit ASCII code */

/* print the font name into a string */
void MFONT_FontToStr(MFONT_Size_e font, unsigned char *buf, size_t bufSize);

/* parse a font name string and return its handle */
void MFONT_ParseFontName(const unsigned char **p, MFONT_Size_e *font);

void MFONT_PrintString(const unsigned char *str, int xPos, int yPos, MFONT_Size_e font);

uint8_t MFONT_ShowFramedText(uint8_t x, uint8_t y, unsigned char *text, MFONT_Size_e font, bool withBorder, bool wait);

uint8_t MFONT_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);


#endif /* MFONT_H_ */
