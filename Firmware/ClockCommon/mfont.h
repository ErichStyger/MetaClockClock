/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MFONT_H_
#define MFONT_H_

#include "platform.h"
#include "McuShell.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
  MFONT_SIZE_2x3,
  MFONT_SIZE_3x5,
  MFONT_SIZE_4x5,
  MFONT_SIZE_3x6,
  MFONT_SIZE_ERROR, /* sentinel, used for errors */
} MFONT_Size_e;

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

#define MFONT_SIZE_X_2x3   2  /* number of columns needed */
#define MFONT_SIZE_Y_2x3   3  /* number of rows needed */

#define MFONT_SIZE_X_3x5   3  /* number of columns needed */
#define MFONT_SIZE_Y_3x5   5  /* number of rows needed */

#define MFONT_SIZE_X_4x5   4  /* number of columns needed */
#define MFONT_SIZE_Y_4x5   5  /* number of rows needed */

#define MFONT_SIZE_X_3x6   3  /* number of columns needed */
#define MFONT_SIZE_Y_3x6   6  /* number of rows needed */

/* special character support */
#define MFONT_CHAR_DEGREE  '~' /* placeholder, need to use a 8bit ASCII code */
#define MFONT_STR_DEGREE   "~" /* placeholder, need to use a 8bit ASCII code */

/* print the font name into a string */
void MFONT_FontToStr(MFONT_Size_e font, unsigned char *buf, size_t bufSize);

/* determine the size of a text based on font */
void MFONT_GetFontTextSize(const unsigned char *text, MFONT_Size_e font, int *xSize, int *ySize);

/* parse a font name string and return its handle */
void MFONT_ParseFontName(const unsigned char **p, MFONT_Size_e *font);

/* print a string with a 3x6 font at a position */
void MFONT_PrintString3x6(const unsigned char *str, int xPos, int yPos);

/* print a string using a font at a position */
void MFONT_PrintString(const unsigned char *str, int xPos, int yPos, MFONT_Size_e font);

/*!
 * \brief Move all hands to the 'clear' or disabled position (at 225 degrees) with LED off
 */
void MFONT_PositionAllToClear(void);

uint8_t MFONT_ShowFramedText(uint8_t x, uint8_t y, const unsigned char *text, MFONT_Size_e font, bool withBorder, bool wait);

uint8_t MFONT_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

#endif /* MFONT_H_ */
