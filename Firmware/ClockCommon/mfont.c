/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_FONT
#include "mfont.h"
#include "McuUtility.h"
#include "matrix.h"
#include "matrixposition.h"
#include "matrixhand.h"

void MFONT_DrawBitmap(const MClock_t *map, size_t width, size_t height, uint8_t xPos, uint8_t yPos, bool doDimming, uint32_t color) {
  int x, y;

  x = y = 0;
  for(int i=0; i<width*height; i++) {
    MPOS_SetAngleZ0Z1(xPos+x, yPos+y, map[i].hands[0].angle, map[i].hands[1].angle);
    MPOS_SetMoveModeZ0Z1(xPos+x, yPos+y, STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  #if PL_MATRIX_CONFIG_IS_RGB
    #if PL_CONFIG_USE_LED_DIMMING
      if (doDimming) {
        MATRIX_StartHandDimming(xPos+x, yPos+y, 0, map[i].hands[0].enabled?0xff:0);
        MATRIX_StartHandDimming(xPos+x, yPos+y, 1, map[i].hands[1].enabled?0xff:0);
      }
    #else
      (void)doDimming; /* not used */
    #endif
    MHAND_HandEnable(xPos+x, yPos+y, 0, map[i].hands[0].enabled);
    MHAND_SetHandColor(xPos+x, yPos+y, 0, color);
    MHAND_HandEnable(xPos+x, yPos+y, 1, map[i].hands[1].enabled);
    MHAND_SetHandColor(xPos+x, yPos+y, 1, color);
  #endif
  #if PL_CONFIG_USE_EXTENDED_HANDS
    MHAND_2ndHandEnable(xPos+x, yPos+y, 0, map[i].hands[0].enabled2nd);
    MHAND_2ndHandEnable(xPos+x, yPos+y, 1, map[i].hands[1].enabled2nd);
  #endif
    x++; /* next in list */
    if (x==width) { /* next line */
      x = 0;
      y++;
    }
  }
}

void MFONT_PrintString(const unsigned char *str, int xPos, int yPos, MFONT_Size_e font, uint32_t color) {
#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_2x3 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_2x3
  if (font==MFONT_SIZE_2x3) {
    MFONT_PrintString2x3(str, xPos, yPos, color);
    return;
  }
#endif
#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_3x5 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_3x5
  if (font==MFONT_SIZE_3x5) {
    MFONT_PrintString3x5(str, xPos, yPos, color);
    return;
  }
#endif
#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_3x6 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_3x6
  if (font==MFONT_SIZE_3x6) {
    MFONT_PrintString3x6(str, xPos, yPos, color);
    return;
  }
#endif
#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_4x5 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_4x5
  if (font==MFONT_SIZE_4x5) {
    MFONT_PrintString4x5(str, xPos, yPos, color);
    return;
  }
#endif
}

void MFONT_GetFontTextSize(const unsigned char *text, MFONT_Size_e font, int *xSize, int *ySize) {
  switch(font) {
#if MFONT_2x3_AVAILABLE
    case MFONT_SIZE_2x3:
      *ySize = MFONT_SIZE_Y_2x3;
      break;
#endif
#if MFONT_3x5_AVAILABLE
    case MFONT_SIZE_3x5:
      *ySize = MFONT_SIZE_Y_3x5;
      break;
#endif
#if MFONT_3x6_AVAILABLE
    case MFONT_SIZE_3x6:
      *ySize = MFONT_SIZE_Y_3x6;
      break;
#endif
#if MFONT_4x5_AVAILABLE
    case MFONT_SIZE_4x5:
      *ySize = MFONT_SIZE_Y_4x5;
      break;
#endif
    default: /* error case */
      *ySize = 0;
      break;
  } /* switch */
  int width = 0 ;
  while(*text!='\0') {
    switch(font) {
#if MFONT_2x3_AVAILABLE
      case MFONT_SIZE_2x3:
        width += MFONT_GetCharWidth2x3(*text);
        break;
#endif
#if MFONT_3x5_AVAILABLE
      case MFONT_SIZE_3x5:
        width += MFONT_GetCharWidth3x5(*text);
        break;
#endif
#if MFONT_3x6_AVAILABLE
      case MFONT_SIZE_3x6:
        width += MFONT_GetCharWidth3x6(*text);
        break;
#endif
#if MFONT_4x5_AVAILABLE
      case MFONT_SIZE_4x5:
        width += MFONT_GetCharWidth4x5(*text);
        break;
#endif
      default: /* error case */
        break;
    } /* switch */
    text++;
  }
  *xSize = width;
}

void MFONT_PositionAllToClear(void) {
  MPOS_SetAngleAll(MPOS_ANGLE_HIDE); /* move to 'hide' position all by default */
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(false); /* turn all off, they will be turned on while writing the font */
#endif
}

#if PL_CONFIG_IS_MASTER
uint8_t MFONT_ShowFramedText(uint8_t x, uint8_t y, const unsigned char *text, MFONT_Size_e font, bool withBorder, uint32_t color, bool wait) {
  int xSize, ySize;
  int xPos, yPos;

  MATRIX_SetMoveDelayAll(2);
  MPOS_SetAngleAll(MPOS_ANGLE_HIDE); /* move to 'hide' position all by default */
#if PL_MATRIX_CONFIG_IS_RGB
  MHAND_HandEnableAll(false); /* turn all off, they will be turned on while writing the font */
#endif
  MFONT_GetFontTextSize(text, font, &xSize, &ySize);
  if (withBorder && xSize<=MATRIX_NOF_STEPPERS_X-1 && ySize<=MATRIX_NOF_STEPPERS_Y-1) { /* only if enough space for border */
    MATRIX_DrawRectangle(0, 0, MATRIX_NOF_STEPPERS_X, MATRIX_NOF_STEPPERS_Y);
  }
  /* calculate text start position */
  xPos = (MATRIX_NOF_STEPPERS_X-xSize)/2;
  if (xPos<0) {
    xPos = 0;
  }
  yPos = (MATRIX_NOF_STEPPERS_Y-ySize)/2;
  if (yPos<0) {
    yPos = 0;
  }
  MFONT_PrintString(text, xPos, yPos, font, color);
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
#if MATRIX_NOF_STEPPERS_X>=MFONT_SIZE_X_3x6 && MATRIX_NOF_STEPPERS_Y>=MFONT_SIZE_Y_3x6
  McuShell_SendStr((unsigned char*)"3x6 ", io->stdOut);
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
  McuShell_SendHelpStr((unsigned char*)"  clear all", (unsigned char*)"Clear all the text area\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  text <f> <x> <y> <t> <c>", (unsigned char*)"Write text with font (e.g. 2x3) at position with color\r\n", io->stdOut);
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
  } else if (McuUtility_strcmp((char*)cmd, "mfont clear all")==0) {
    *handled = true;
    MFONT_PositionAllToClear();
    return MATRIX_SendToRemoteQueueExecuteAndWait(true);
  } else if (McuUtility_strncmp((char*)cmd, "mfont text ", sizeof("mfont text ")-1)==0) {
    const unsigned char *p;
    uint8_t xPos, yPos;
    MFONT_Size_e font;
    uint32_t color;

    *handled = TRUE;
    p = cmd + sizeof("mfont text ")-1;
    MFONT_ParseFontName(&p, &font);
    if (font==MFONT_SIZE_ERROR) {
      return ERR_FAILED;
    }
    if (McuUtility_xatoi(&p, (int32_t*)&color)!=ERR_OK) {
      return ERR_FAILED;
    }
    if (   McuUtility_ScanDecimal8uNumber(&p, &xPos)==ERR_OK && xPos<MATRIX_NOF_STEPPERS_X
        && McuUtility_ScanDecimal8uNumber(&p, &yPos)==ERR_OK && yPos<MATRIX_NOF_STEPPERS_Y
        )
    {
      uint8_t buf[32];

      McuUtility_SkipSpaces(&p);
      if (McuUtility_ReadEscapedName(p, buf, sizeof(buf), NULL, NULL, NULL)!=ERR_OK) {
        return ERR_FAILED;
      }
      MATRIX_SetMoveDelayZ0Z1All(2, 2);
      MFONT_PrintString(buf, xPos, yPos, font, color);
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
    case MFONT_SIZE_3x6: McuUtility_strcpy(buf, bufSize, (unsigned char*)"3x6"); break;
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
  } else if (McuUtility_strncmp((char*)*p, "3x6", sizeof("3x6")-1)==0) {
    *font = MFONT_SIZE_3x6;
    *p += sizeof("3x6")-1;
  } else if (McuUtility_strncmp((char*)*p, "4x5", sizeof("4x5")-1)==0) {
    *font = MFONT_SIZE_4x5;
    *p += sizeof("4x5")-1;
  } else {
    *font = MFONT_SIZE_ERROR;
  }
}

#endif /* PL_CONFIG_USE_FONT */
