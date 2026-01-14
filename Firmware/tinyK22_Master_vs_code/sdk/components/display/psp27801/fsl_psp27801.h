/*
 * Copyright  2016 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_PSP27801_H_
#define _FSL_PSP27801_H_

#include "fsl_common.h"
#include "fsl_dspi.h"
#include "stdlib.h"
#include "fsl_timer.h"
#include "gui_resources.h"
#include "board.h"

/**********************************************************************************************************************
 * Definitions
 *********************************************************************************************************************/
/* OLED APIs related  */
#define OLED_REMAP_SETTINGS                                                                             \
    (REMAP_ORDER_ABC | REMAP_COM_SPLIT_ODD_EVEN_EN | REMAP_COLOR_RGB565 | REMAP_COLUMNS_LEFT_TO_RIGHT | \
     REMAP_SCAN_UP_TO_DOWN | REMAP_HORIZONTAL_INCREMENT)
#define OLED_AdjustRowOffset(y)       y += OLED_ROW_OFFSET
#define OLED_AdjustColumnOffset(x)    x += OLED_COLUMN_OFFSET
#define CheckLimits(x, y, w, h)       (((x + w - 1) > OLED_SCREEN_WIDTH) || ((y + h - 1) > OLED_SCREEN_HEIGHT))
#define AreCoordsValid(x, y, w, h)    (0U == CheckLimits(x, y, w, h))
#define AreCoordsNotValid(x, y, w, h) (0U != CheckLimits(x, y, w, h))

/* OLED GUI related  */
#define BMP_HEADER_BYTE_SIZE         (6)
#define GuiDriver_SkipHeader(imgPtr) ((const uint8_t *)(imgPtr) + BMP_HEADER_BYTE_SIZE)
#define GUI_IMAGE_SIZE               (OLED_SCREEN_WIDTH * OLED_SCREEN_HEIGHT * OLED_BYTES_PER_PIXEL)

/* OLED command type: first byte and other byte */
#define FIRST_BYTE (1)
#define OTHER_BYTE (0)

/* OLED offset value for column and row */
#define OLED_COLUMN_OFFSET (16)
#define OLED_ROW_OFFSET    (0)

/* OLED pixel size */
#define OLED_BYTES_PER_PIXEL (2)

/* OLED width size and height size related*/
#define OLED_SCREEN_WIDTH      (96U)
#define OLED_SCREEN_HEIGHT     (96U)
#define OLED_SCREEN_WIDTH_END  ((OLED_SCREEN_WIDTH - 1) + OLED_COLUMN_OFFSET)
#define OLED_SCREEN_HEIGHT_END ((OLED_SCREEN_HEIGHT - 1) + OLED_ROW_OFFSET)
#define OLED_SCREEN_SIZE       (OLED_SCREEN_WIDTH * OLED_SCREEN_HEIGHT)
#define OLED_GRAM_SIZE         (OLED_SCREEN_WIDTH * OLED_SCREEN_HEIGHT * OLED_BYTES_PER_PIXEL)

/* OLED transition step */
#define OLED_TRANSITION_STEP (1)

/* OLED text related */
#define OLED_TEXT_HALIGN_SHIFT (0)
#define OLED_TEXT_HALIGN_MASK  (0x03 << OLED_TEXT_HALIGN_SHIFT)
#define OLED_TEXT_VALIGN_SHIFT (4)
#define OLED_TEXT_VALIGN_MASK  (0x03 << OLED_TEXT_VALIGN_SHIFT)

/* OLED remap settings */
#define REMAP_HORIZONTAL_INCREMENT   (0)
#define REMAP_VERTICAL_INCREMENT     (1 << 0)
#define REMAP_COLUMNS_LEFT_TO_RIGHT  (0)
#define REMAP_COLUMNS_RIGHT_TO_LEFT  (1 << 1)
#define REMAP_ORDER_ABC              (0)
#define REMAP_ORDER_CBA              (1 << 2)
#define REMAP_SCAN_UP_TO_DOWN        (0)
#define REMAP_SCAN_DOWN_TO_UP        (1 << 4)
#define REMAP_COM_SPLIT_ODD_EVEN_DIS (0)
#define REMAP_COM_SPLIT_ODD_EVEN_EN  (1 << 5)
#define REMAP_COLOR_RGB565           (1 << 6)

/* OLED swap one char */
#define OLED_SwapMe(x) x = ((x & 0xFF00) >> 8) | ((x & 0x00FF) << 8)

/* Set start/end for column/row
 * the 2nd and 3rd byte represent the start and the end address, respectively
 */
#define OLED_CMD_SET_COLUMN (0x15)
#define OLED_CMD_SET_ROW    (0x75)

/* Set OLED scan direction*/
#define OLED_DIRECTION_HORIZONTAL (0)
#define OLED_DIRECTION_VERTICAL   (1)

/* OLED display settings */
#define OLED_CMD_SET_DISPLAY_MODE_ALL_OFF (0xA4)
#define OLED_CMD_SET_DISPLAY_MODE_ALL_ON  (0xA5)
#define OLED_CMD_SET_DISPLAY_MODE_NORMAL  (0xA6)
#define OLED_CMD_SET_DISPLAY_MODE_INVERSE (0xA7)

/*
 * Set lock command
 * the locked OLED driver MCU interface prohibits all commands
 * and memory access, except the 0xFD command
 */
#define OLED_CMD_SET_CMD_LOCK (0xFD)
/* Unlock OLED driver MCU interface for entering command (default upon reset) */
#define OLED_UNLOCK (0x12)
/* Lock OLED driver MCU interface for entering command */
#define OLED_LOCK (0x16)
/* Commands 0xA2, 0xB1, 0xB3, 0xBB, 0xBE, 0xC1 are inaccessible in both lock and unlock state (default upon reset) */
#define OLED_ACC_TO_CMD_NO (0xB0)
/* Commands 0xA2, 0xB1, 0xB3, 0xBB, 0xBE, 0xC1 are accessible in unlock state */
#define OLED_ACC_TO_CMD_YES (0xB1)
/* Command for OLED nop alse 0xE3 */
#define OLED_CMD_NOP (0xD1)

/* Set MUX ratio */
#define OLED_CMD_SET_MUX_RATIO (0xCA)

/* Set re-map color depth */
#define OLED_CMD_SET_REMAP (0xA0)

/* Set horisontal or vertical increment */
#define OLED_ADDR_INC_HOR (0x00)
#define OLED_ADDR_INC_VER (0x01)

/* Column address mapping */
#define OLED_COLUMN_ADDR_REMAP_0_TO_SEG0   (0x00)
#define OLED_COLUMN_ADDR_REMAP_127_TO_SEG0 (0x02)

/* Color sequence order */
#define OLED_COLOR_SEQ_A_B_C (0x00)
#define OLED_COLOR_SEQ_C_B_A (0x04)

/* Scanning order (MR == MUX ratio) */
#define OLED_SCAN_FROM_COM_0_TO_MR (0x00)
#define OLED_SCAN_FROM_COM_MR_TO_0 (0x10)

/* COM splitting to odd and even */
#define OLED_COM_SPLIT_DISABLE (0x00)
#define OLED_COM_SPLIT_ENABLE  (0x20)

/* Screen color depth */
#define OLED_COLOR_DEPTH_256    (0x00)
#define OLED_COLOR_DEPTH_65K    (0x40)
#define OLED_COLOR_DEPTH_262K_1 (0x80)
#define OLED_COLOR_DEPTH_262K_2 (0xC0)

/*
 * OLED reset (phase 1) and pre-charge (phase 2) period in [DCLK] settings
 * these commands are locked by command 0xFD by default
 */
#define OLED_CMD_SET_RESET_AND_PRECHARGE_PERIOD (0xB1)

/* OLED reset period related */
#define OLED_RESET_PERIOD_5  (0x02)
#define OLED_RESET_PERIOD_7  (0x03)
#define OLED_RESET_PERIOD_9  (0x04)
#define OLED_RESET_PERIOD_11 (0x05)
#define OLED_RESET_PERIOD_13 (0x06)
#define OLED_RESET_PERIOD_15 (0x07)
#define OLED_RESET_PERIOD_17 (0x08)
#define OLED_RESET_PERIOD_19 (0x09)
#define OLED_RESET_PERIOD_21 (0x0A)
#define OLED_RESET_PERIOD_23 (0x0B)
#define OLED_RESET_PERIOD_25 (0x0C)
#define OLED_RESET_PERIOD_27 (0x0D)
#define OLED_RESET_PERIOD_29 (0x0E)
#define OLED_RESET_PERIOD_31 (0x0F)

/*OLED pre-charge period related */
#define OLED_PRECHARGE_PERIOD_3  (0x03)
#define OLED_PRECHARGE_PERIOD_4  (0x04)
#define OLED_PRECHARGE_PERIOD_5  (0x05)
#define OLED_PRECHARGE_PERIOD_6  (0x06)
#define OLED_PRECHARGE_PERIOD_7  (0x07)
#define OLED_PRECHARGE_PERIOD_8  (0x08)
#define OLED_PRECHARGE_PERIOD_9  (0x09)
#define OLED_PRECHARGE_PERIOD_10 (0x0A)
#define OLED_PRECHARGE_PERIOD_11 (0x0B)
#define OLED_PRECHARGE_PERIOD_12 (0x0C)
#define OLED_PRECHARGE_PERIOD_13 (0x0D)
#define OLED_PRECHARGE_PERIOD_14 (0x0E)
#define OLED_PRECHARGE_PERIOD_15 (0x0F)

/*
 * Set front clock divider (divset) / oscillator frequency
 * this command is locked by command 0xFD by default
 */
#define OLED_CMD_SET_OSC_FREQ_AND_CLOCKDIV (0xB3)

/* OLED clock divider */
#define OLED_CLOCKDIV_1    (0x00)
#define OLED_CLOCKDIV_2    (0x01)
#define OLED_CLOCKDIV_4    (0x02)
#define OLED_CLOCKDIV_8    (0x03)
#define OLED_CLOCKDIV_16   (0x04)
#define OLED_CLOCKDIV_32   (0x05)
#define OLED_CLOCKDIV_64   (0x06)
#define OLED_CLOCKDIV_128  (0x07)
#define OLED_CLOCKDIV_256  (0x08)
#define OLED_CLOCKDIV_512  (0x09)
#define OLED_CLOCKDIV_1024 (0x0A)

/* oled oscillator frequency, frequency increases as level increases */
#define OLED_OSC_FREQ (0xB0)

/* OLED setting commands*/
#define OLED_CMD_STARTLINE      (0xA1)
#define OLED_CMD_WRITERAM       (0x5C)
#define OLED_CMD_READRAM        (0x5D)
#define OLED_CMD_DISPLAYOFFSET  (0xA2)
#define OLED_CMD_DISPLAYALLOFF  (0xA4)
#define OLED_CMD_DISPLAYALLON   (0xA5)
#define OLED_CMD_NORMALDISPLAY  (0xA6)
#define OLED_CMD_INVERTDISPLAY  (0xA7)
#define OLED_CMD_FUNCTIONSELECT (0xAB)
#define OLED_CMD_DISPLAYOFF     (0xAE)
#define OLED_CMD_DISPLAYON      (0xAF)
#define OLED_CMD_PRECHARGE      (0xB1)
#define OLED_CMD_DISPLAYENHANCE (0xB2)
#define OLED_CMD_SETVSL         (0xB4)
#define OLED_CMD_SETGPIO        (0xB5)
#define OLED_CMD_PRECHARGE2     (0xB6)
#define OLED_CMD_SETGRAY        (0xB8)
#define OLED_CMD_USELUT         (0xB9)
#define OLED_CMD_PRECHARGELEVEL (0xBB)
#define OLED_CMD_VCOMH          (0xBE)
#define OLED_CMD_CONTRASTABC    (0xC1)
#define OLED_CMD_CONTRASTMASTER (0xC7)
#define OLED_CMD_MUXRATIO       (0xCA)
#define OLED_CMD_COMMANDLOCK    (0xFD)
#define OLED_CMD_HORIZSCROLL    (0x96)
#define OLED_CMD_STOPSCROLL     (0x9E)
#define OLED_CMD_STARTSCROLL    (0x9F)

/* OLED image transition styles */
typedef enum
{
    kOLED_Transition_None,       /*!< no image transition, just draw it */
    kOLED_Transition_Top_Down,   /*!< image transition from top to down */
    kOLED_Transition_Down_Top,   /*!< image transition from down to top */
    kOLED_Transition_Left_Right, /*!< image transition from left to right */
    kOLED_Transition_Right_Left  /*!< image transition from right to left */
} oled_transition_t;

/* OLED status flags */
typedef enum
{
    OLED_STATUS_SUCCESS,        /*!< success */
    OLED_STATUS_ERROR,          /*!< fail */
    OLED_STATUS_PROTOCOL_ERROR, /*!< DSPI failure */
    OLED_STATUS_INIT_ERROR,     /*!< initialization error */
    OLED_STATUS_DEINIT_ERROR    /*!< deinitialization error */
} oled_status_t;

/* OLED pixel definition */
typedef uint16_t *oled_pixel_t;

/* OLED text alignment options */
typedef enum
{
    OLED_TEXT_ALIGN_NONE = 0,

    OLED_TEXT_ALIGN_LEFT   = 0x1,
    OLED_TEXT_ALIGN_RIGHT  = 0x2,
    OLED_TEXT_ALIGN_CENTER = 0x3,

    OLED_TEXT_VALIGN_TOP    = 0x10,
    OLED_TEXT_VALIGN_BOTTOM = 0x20,
    OLED_TEXT_VALIGN_CENTER = 0x30
} oled_text_align_t;

/* OLED color options */
typedef enum
{
    OLED_COLOR_BLACK   = 0x0000,
    OLED_COLOR_BLUE_1  = 0x06FF,
    OLED_COLOR_BLUE    = 0x001F,
    OLED_COLOR_RED     = 0xF800,
    OLED_COLOR_GREEN   = 0x07E0,
    OLED_COLOR_CYAN    = 0x07FF,
    OLED_COLOR_MAGENTA = 0xF81F,
    OLED_COLOR_YELLOW  = 0xFFE0,
    OLED_COLOR_GRAY    = 0x528A,
    OLED_COLOR_WHITE   = 0xFFFF

} oled_color_t;

/* OLED text/image dynamic area structure */
typedef struct
{
    uint8_t xCrd;            /*!< desired x-coordinate */
    uint8_t yCrd;            /*!< desired y-coordinate */
    uint8_t width;           /*!< desired text/image width */
    uint8_t height;          /*!< desired text/image height */
    oled_pixel_t areaBuffer; /*!< desired memory buffer */
} oled_dynamic_area_t;

/* OLED text properties */
typedef struct
{
    const uint8_t *font;          /*!< desired font */
    uint16_t fontColor;           /*!< desired font color */
    oled_text_align_t alignParam; /*!< desired font alignment */
    const uint8_t *background;    /*!< desired font background */
} oled_text_properties_t;

/* OLED color definition */
typedef enum
{
    GUI_COLOR_BLACK   = 0x0000,
    GUI_COLOR_BLUE_1  = 0x06FF,
    GUI_COLOR_BLUE    = 0x001F,
    GUI_COLOR_RED     = 0xF800,
    GUI_COLOR_GREEN   = 0x07E0,
    GUI_COLOR_CYAN    = 0x07FF,
    GUI_COLOR_MAGENTA = 0xF81F,
    GUI_COLOR_YELLOW  = 0xFFE0,
    GUI_COLOR_GRAY    = 0x528A,
    GUI_COLOR_WHITE   = 0xFFFF
} guiColor_t;

/**********************************************************************************************************************
 * API
 *********************************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief OLED hardware initialization.
 * Initialize the CS/DC/RST/PWR pins and set DSPI to master mode for transition
 */
void OLED_Hardware_Init(void);

/*!
 * @brief Initialize OLED
 * Initialize the OLED and send some command to configure the OLED inter register
 * @return status flag
 */
oled_status_t OLED_Init(void);

/*!
 * @brief Deinitialize the OLED
 * @return status flag
 */
oled_status_t OLED_Deinit(void);

/*!
 * @brief Return given image dimensions
 * @param width  given image's width
 * @param height given image's height
 * @param image  desired image
 */
void OLED_GetImageDimensions(uint8_t *width, uint8_t *height, const uint8_t *image);

/*!
 * @brief Add image to the main screen buffer.
 * @param  image  desired image.
 * @return  status flag.
 */
oled_status_t OLED_AddImage(const uint8_t *image);

/*!
 * @brief Draw image.
 * @param  image       desired image
 * @param  transition  desired transition for the image to appear
 * @return             status flag
 */
oled_status_t OLED_DrawImage(const uint8_t *image);

/*!
 * @brief Send the command to OLED.
 * @param  cmd     OLED command from the datasheet
 * @param  isFirst designate if this is the first byte in the command
 * @return         status flag
 */
oled_status_t OLED_SendCmd(uint32_t cmd, uint8_t isFirst);

/*!
 * @brief Send data to OLED
 * @param  dataToSend  data to send to OLED
 * @param  dataSize    data-size
 * @return             status flag
 */
oled_status_t OLED_SendData(const uint8_t *dataToSend, uint32_t dataSize);

/*!
 * @brief Draw box on OLED
 * @param  xCrd    x-coordinate for box's uper left corner
 * @param  yCrd    y-coordinate for box's uper left corner
 * @param  width   box's width
 * @param  height  box's height
 * @param  color   color of the box
 * @return         status flag
 */
oled_status_t OLED_DrawBox(uint16_t xCrd, uint16_t yCrd, uint16_t width, uint16_t height, uint16_t color);

/*!
 * @brief Fill the entire screen
 * @param  color   color to fill with
 * @return status  flag
 */
oled_status_t OLED_FillScreen(uint16_t color);

/*!
 * @brief Draw a single pixel
 * @param  xCrd   pixel's x coordinate
 * @param  yCrd   pixel's y coordinate
 * @param  color  pixel's color
 * @return        status flag
 */
oled_status_t OLED_DrawPixel(int16_t xCrd, int16_t yCrd, uint16_t color);

/*!
 * @brief Draw the whole screen
 * @param  image      image to draw
 * @param  xCrd       image x-coordinate
 * @param  yCrd       image y-coordinate
 * @param  width      image width
 * @param  height     image height
 * @param  transition transition style for the new image arrival
 * @return            status flag
 */
oled_status_t OLED_DrawScreen(
    const uint8_t *image, uint8_t xCrd, uint8_t yCrd, uint8_t width, uint8_t height, oled_transition_t transition);

/*!
 * @brief Set the font to use
 * @param newFont   desired font
 * @param newColor  desired color
 * @return          status flag
 */
oled_status_t OLED_SetFont(const uint8_t *newFont, uint16_t newColor);

/*!
 * @brief Add text to the main screen buffer.
 * @param  text  text to add
 * @return       status flag
 */
oled_status_t OLED_AddText(const uint8_t *text);

/*!
 * @brief Write text on OLED to given position
 * @param text desired text
 * @param x    x-coordinate for the given text
 * @param y    y-coordinate for the given text
 */
oled_status_t OLED_DrawText(const uint8_t *text);

/*!
 * @brief Set OLED dynamic area
 * @param dynamic_area data-structure with desired values
 */
oled_status_t OLED_SetDynamicArea(oled_dynamic_area_t *dynamic_area);

/*!
 * @brief Destroy current OLED dynamic area.
 */
void OLED_DestroyDynamicArea(void);

/*!
 * @brief Set OLED text properties
 * @param textProperties data-structure with desired properties
 */
void OLED_SetTextProperties(oled_text_properties_t *textProperties);

/*!
 * @brief Count the characters
 * @param  width  text width
 * @param  font   text font
 * @param  text   given text string
 * @param  length text length
 * @return        character count
 */
uint8_t OLED_CharCount(uint8_t width, const uint8_t *font, const uint8_t *text, uint8_t length);

/*!
 * @brief Get the width of the string to be displayed on the screen.
 * @param  text   desired string
 * @return        required text width in [px]
 */
uint8_t OLED_GetTextWidth(const uint8_t *text);

/*!
 * @brief swap image's bytes per pixel to obtain the correct color format
 * @param imgDst  desired image
 * @param imgSrc  original image
 * @param imgSize image's size
 */
void OLED_Swap(oled_pixel_t imgSrc, const uint8_t *imgDst, uint16_t imgSize);

/*!
 * @brief Dim OLED screen on.
 */
oled_status_t OLED_DimScreenON(void);

/*!
 * @brief Dim OLED screen off.
 */
oled_status_t OLED_DimScreenOFF(void);

#if defined(__cplusplus)
}
#endif

#endif /* _FSL_PSP27801_H_ */
