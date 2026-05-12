/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MFONT_H_
#define MFONT_H_

#include "platform.h"
#include "McuShell.h"
#include "matrix.h"
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

#define MFONT_2x3_AVAILABLE     (PL_CONFIG_USE_FONT && MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_2x3 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_2x3)
#define MFONT_3x5_AVAILABLE     (PL_CONFIG_USE_FONT && MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_3x5 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_3x5)
#define MFONT_4x5_AVAILABLE     (PL_CONFIG_USE_FONT && MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_4x5 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_4x5)
#define MFONT_3x6_AVAILABLE     (PL_CONFIG_USE_FONT && MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_3x6 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_3x6)

/* special character support */
#define MFONT_CHAR_DEGREE  '~' /* placeholder, need to use a 8bit ASCII code */
#define MFONT_STR_DEGREE   "~" /* placeholder, need to use a 8bit ASCII code */

/*!
 * \brief Draws a bitmap of clock-hand states into the matrix.
 * \param map Clock-hand bitmap to draw.
 * \param width Bitmap width in matrix cells.
 * \param height Bitmap height in matrix cells.
 * \param xPos X start position for text or bitmap output.
 * \param yPos Y start position for text or bitmap output.
 * \param doDimming True to use dimming while drawing.
 * \param color Color to be used for drawing.
 */
void MFONT_DrawBitmap(const MClock_t *map, size_t width, size_t height, uint8_t xPos, uint8_t yPos, bool doDimming, uint32_t color);

/*!
 * \brief Converts a font selector into its text name.
 * \param font Font size selector.
 * \param buf Data or text buffer.
 * \param bufSize Size of the buffer in bytes.
 */
void MFONT_FontToStr(MFONT_Size_e font, unsigned char *buf, size_t bufSize);

/*!
 * \brief Calculates the rendered size of text for the selected font.
 * \param text Null-terminated text string.
 * \param font Font size selector.
 * \param xSize Pointer receiving the rendered width.
 * \param ySize Pointer receiving the rendered height.
 */
void MFONT_GetFontTextSize(const unsigned char *text, MFONT_Size_e font, int *xSize, int *ySize);

/*!
 * \brief Parses a font name and returns the matching font selector.
 * \param p Pointer to the parser cursor.
 * \param font Font size selector.
 */
void MFONT_ParseFontName(const unsigned char **p, MFONT_Size_e *font);

/*!
 * \brief Gets the rendered character width for the 2x3 font.
 * \param ch Character to measure.
 * \return Character width in matrix cells.
 * \return Requested 8-bit value.
 */
uint8_t MFONT_GetCharWidth2x3(char ch);

/*!
 * \brief Gets the rendered character width for the 3x5 font.
 * \param ch Character to measure.
 * \return Character width in matrix cells.
 * \return Requested 8-bit value.
 */
uint8_t MFONT_GetCharWidth3x5(char ch);

/*!
 * \brief Gets the rendered character width for the 3x6 font.
 * \param ch Character to measure.
 * \return Character width in matrix cells.
 * \return Requested 8-bit value.
 */
uint8_t MFONT_GetCharWidth3x6(char ch);

/*!
 * \brief Gets the rendered character width for the 4x5 font.
 * \param ch Character to measure.
 * \return Character width in matrix cells.
 * \return Requested 8-bit value.
 */
uint8_t MFONT_GetCharWidth4x5(char ch);

/*!
 * \brief Draw a character with 2x3 font.
 * \param ch Character to print.
 * \param xPos X start position for text or bitmap output.
 * \param yPos Y start position for text or bitmap output.
 * \param color Color to be used for drawing.
 */
void MFONT_DrawChar2x3(char ch, uint8_t *xPos, uint8_t *yPos, uint32_t color);

/*!
 * \brief Draw a character with 3x5 font.
 * \param ch Character to print.
 * \param xPos X start position for text or bitmap output.
 * \param yPos Y start position for text or bitmap output.
 * \param color Color to be used for drawing.
 */
void MFONT_DrawChar3x5(char ch, uint8_t *xPos, uint8_t *yPos, uint32_t color);

/*!
 * \brief Draw a character with 3x6 font.
 * \param ch Character to print.
 * \param xPos X start position for text or bitmap output.
 * \param yPos Y start position for text or bitmap output.
 * \param color Color to be used for drawing.
 */
void MFONT_DrawChar3x6(char ch, uint8_t *xPos, uint8_t *yPos, uint32_t color);

/*!
 * \brief Draw a character with 4x5 font.
 * \param ch Character to print.
 * \param xPos X start position for text or bitmap output.
 * \param yPos Y start position for text or bitmap output.
 * \param color Color to be used for drawing.
 */
void MFONT_DrawChar4x5(char ch, uint8_t *xPos, uint8_t *yPos, uint32_t color);

/*!
 * \brief Prints a string into the matrix using the selected font.
 * \param str Null-terminated string.
 * \param xPos X start position for text or bitmap output.
 * \param yPos Y start position for text or bitmap output.
 * \param font Font size selector.
 * \param color Color to be used for drawing.
 */
void MFONT_PrintString(const unsigned char *str, int xPos, int yPos, MFONT_Size_e font, uint32_t color);

/*!
 * \brief Moves all matrix hands to the clear-font position.
 */
void MFONT_PositionAllToClear(void);

/*!
 * \brief Shows framed text on the matrix.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param text Null-terminated text string.
 * \param font Font size selector.
 * \param withBorder True to draw a border around the text.
 * \param wait True to wait for completion.
 * \param color Color to be used for drawing.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t MFONT_ShowFramedText(uint8_t x, uint8_t y, const unsigned char *text, MFONT_Size_e font, bool withBorder, uint32_t color, bool wait);

/*!
 * \brief Parses font shell commands.
 * \param cmd Command string to parse or send.
 * \param handled Set to true when the command has been handled.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t MFONT_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

#endif /* MFONT_H_ */
