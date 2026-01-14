/*
 * Copyright  2016 NXP
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_psp27801.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief Send data to OLED via the DSPI.
 * @param  bufPtr  pointer to data which will be send to OLED.
 * @param  txSize  data-size
 * @return         status flag
 */
static oled_status_t OLED_SendViaDSPI(const uint8_t *bufPtr, uint32_t txSize);

/*!
 * @brief Transpose the image
 * @param image      given image
 * @param transImage new image
 */
static void Transpose(oled_pixel_t transImage, const oled_pixel_t image, uint8_t width, uint8_t height);

/*!
 * @brief Trnasition type right->left.
 * @param  image  given image
 * @param  xCrd   image's x-coordinate
 * @param  yCrd   image's y-coordinate
 * @param  width  image's width
 * @param  height image's height
 * @return        status flag
 */
static oled_status_t RightLeft(const uint8_t *image, uint8_t xCrd, uint8_t yCrd, uint8_t width, uint8_t height);

/*!
 * @brief Transition type left->right.
 * @param  image  given image
 * @param  xCrd   image's x-coordinate
 * @param  yCrd   image's y-coordinate
 * @param  width  image's width
 * @param  height image's height
 * @return        status flag
 */
static oled_status_t LeftRight(const uint8_t *image, uint8_t xCrd, uint8_t yCrd, uint8_t width, uint8_t height);

/*!
 * @brief  Transition type down->top.
 * @param  image  given image
 * @param  xCrd   image's x-coordinate
 * @param  yCrd   image's y-coordinate
 * @param  width  image's width
 * @param  height image's height
 * @return        status flag
 */
static oled_status_t DownTop(const uint8_t *image, uint8_t xCrd, uint8_t yCrd, uint8_t width, uint8_t height);

/*!
 * @brief Transition type Top->down .
 * @param  image  given image
 * @param  xCrd   image's x-coordinate
 * @param  yCrd   image's y-coordinate
 * @param  width  image's width
 * @param  height image's height
 * @return        status flag
 */
static oled_status_t TopDown(const uint8_t *image, uint8_t xCrd, uint8_t yCrd, uint8_t width, uint8_t height);

/*!
 * @brief Set borders for the box-like object to draw
 * @param  xCrd   x-coordinate for the object
 * @param  yCrd   y-coordinate for the object
 * @param  width  object's width
 * @param  height object's height
 */
static void SetBorders(uint8_t xCrd, uint8_t yCrd, uint8_t width, uint8_t height);

/*!
 * @brief Create the text background
 *
 * @return status flag
 */
static oled_status_t CreateTextBackground(void);

/*!
 * @brief Write the character to the buffer.
 * @param charToWrite  character to be written
 * @param chrBuf       given pointer for buffer for the character
 */
static void WriteCharToBuf(uint16_t charToWrite, oled_pixel_t *chrBuf);

/*!
 * @brief Add character to the active text area.
 * @param  xOffset offset for the x-coordinate
 * @param  yOffset offset for the y-coordinate
 * @param  width   desired width
 * @param  height  desired height
 * @return         status flag
 */
static oled_status_t AddCharToTextArea(
    oled_pixel_t chrPtr, uint8_t chrWidth, uint8_t chrHeight, oled_pixel_t copyAddr, uint8_t imgWidth);

/*!
 * @brief Allocate memory for the desired image/character
 * @param area desired area dimensions
 */
static void *AllocateDynamicArea(uint32_t area);

/*!
 * @brief Deallocate current area
 * @param area pointer to current area
 */
static oled_status_t DestroyDynamicArea(void *ptr);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t isFontInitialized = 0;
static const uint8_t *selectedFont;
static uint16_t selectedFont_color;
static uint16_t selectedFont_firstChar;
static uint16_t selectedFont_height;
static oled_dynamic_area_t oled_dynamic_area       = {0};
static oled_text_properties_t oled_text_properties = {.alignParam = OLED_TEXT_ALIGN_CENTER, .background = NULL};
static uint8_t screenBuf[OLED_GRAM_SIZE];
static uint8_t currentChar_width  = 0;
static uint8_t currentChar_height = 0;
static uint16_t colorMask         = GUI_COLOR_WHITE;

/*******************************************************************************
 * Code
 ******************************************************************************/

void OLED_Hardware_Init(void)
{
    gpio_pin_config_t OLED_CS_Config  = {kGPIO_DigitalOutput, 1};
    gpio_pin_config_t OLED_RST_Config = {kGPIO_DigitalOutput, 0};
    gpio_pin_config_t OLED_DC_Config  = {kGPIO_DigitalOutput, 0};
    gpio_pin_config_t OLED_PWR_Config = {kGPIO_DigitalOutput, 1};

    GPIO_PinInit(BOARD_OLED_CS_GPIO, BOARD_OLED_CS_PIN, &OLED_CS_Config);
    GPIO_PinInit(BOARD_OLED_RST_GPIO, BOARD_OLED_RST_PIN, &OLED_RST_Config);
    GPIO_PinInit(BOARD_OLED_DC_GPIO, BOARD_OLED_DC_PIN, &OLED_DC_Config);
    GPIO_PinInit(BOARD_OLED_PWR_GPIO, BOARD_OLED_PWR_PIN, &OLED_PWR_Config);

    dspi_master_config_t masterConfig;

    /*Master config*/
    masterConfig.whichCtar                                = kDSPI_Ctar0;
    masterConfig.ctarConfig.baudRate                      = BOARD_OLED_DSPI_MASTER_TRANSFER_BAUDRATE;
    masterConfig.ctarConfig.bitsPerFrame                  = 8U;
    masterConfig.ctarConfig.cpol                          = kDSPI_ClockPolarityActiveLow;
    masterConfig.ctarConfig.cpha                          = kDSPI_ClockPhaseSecondEdge;
    masterConfig.ctarConfig.direction                     = kDSPI_MsbFirst;
    masterConfig.ctarConfig.pcsToSckDelayInNanoSec        = 64;
    masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec    = 0;
    masterConfig.ctarConfig.betweenTransferDelayInNanoSec = 0;

    masterConfig.whichPcs                   = kDSPI_Pcs0;
    masterConfig.pcsActiveHighOrLow         = kDSPI_PcsActiveLow;
    masterConfig.enableContinuousSCK        = false;
    masterConfig.enableRxFifoOverWrite      = false;
    masterConfig.enableModifiedTimingFormat = false;
    masterConfig.samplePoint                = kDSPI_SckToSin0Clock;

    DSPI_MasterInit(BOARD_OLED_DSPI_MASTER_BASEADDR, &masterConfig, BOARD_OLED_DSPI_MASTER_CLK_FREQ);
}

static oled_status_t OLED_SendViaDSPI(const uint8_t *txBuffer, uint32_t txSize)
{
    oled_status_t status = OLED_STATUS_SUCCESS;
    status_t dspiStatus  = kStatus_Success;
    dspi_transfer_t masterXfer;

    /* Start master transfer */
    masterXfer.txData      = (uint8_t *)txBuffer;
    masterXfer.rxData      = NULL;
    masterXfer.dataSize    = txSize;
    masterXfer.configFlags = kDSPI_MasterCtar0;
    dspiStatus             = DSPI_MasterTransferBlocking(BOARD_OLED_DSPI_MASTER_BASEADDR, &masterXfer);

    if (kStatus_Success != dspiStatus)
    {
        status = OLED_STATUS_PROTOCOL_ERROR;
    }
    else
    {
        status = OLED_STATUS_SUCCESS;
    }
    return status;
}

oled_status_t OLED_SendCmd(uint32_t cmd, uint8_t isFirst)
{
    uint8_t txSize = 1;
    uint8_t txBuf[4];

    memcpy((void *)txBuf, (void *)&cmd, txSize);

    if (isFirst)
    {
        GPIO_PortClear(BOARD_OLED_DC_GPIO, 1U << BOARD_OLED_DC_PIN);
    }
    else
    {
        GPIO_PortSet(BOARD_OLED_DC_GPIO, 1U << BOARD_OLED_DC_PIN);
    }

    GPIO_PortClear(BOARD_OLED_CS_GPIO, 1U << BOARD_OLED_CS_PIN);
    oled_status_t spiStatus = OLED_SendViaDSPI(txBuf, txSize);

    if (OLED_STATUS_SUCCESS == spiStatus)
    {
        GPIO_PortSet(BOARD_OLED_CS_GPIO, 1U << BOARD_OLED_CS_PIN);
        return OLED_STATUS_SUCCESS;
    }
    else
    {
        GPIO_PortSet(BOARD_OLED_CS_GPIO, 1U << BOARD_OLED_CS_PIN);
        return OLED_STATUS_PROTOCOL_ERROR;
    }
}

oled_status_t OLED_Init(void)
{
    /* Power on the OLED for initialization */
    GPIO_PortClear(BOARD_OLED_PWR_GPIO, 1U << BOARD_OLED_PWR_PIN);
    Timer_Delay_ms(2);
    GPIO_PortClear(BOARD_OLED_RST_GPIO, 1U << BOARD_OLED_RST_PIN);
    Timer_Delay_ms(2);
    GPIO_PortSet(BOARD_OLED_RST_GPIO, 1U << BOARD_OLED_RST_PIN);
    Timer_Delay_ms(2);
    GPIO_PortSet(BOARD_OLED_PWR_GPIO, 1U << BOARD_OLED_PWR_PIN);

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_SET_CMD_LOCK, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_UNLOCK, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_SET_CMD_LOCK, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_ACC_TO_CMD_YES, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_DISPLAYOFF, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_SET_OSC_FREQ_AND_CLOCKDIV, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0xF1, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_SET_MUX_RATIO, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0x5F, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_SET_REMAP, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_REMAP_SETTINGS, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_SET_COLUMN, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0x00, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0x5F, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_SET_ROW, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0x00, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0x5F, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_STARTLINE, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0x80, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_DISPLAYOFFSET, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0x60, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_PRECHARGE, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0x32, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_VCOMH, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0x05, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_NORMALDISPLAY, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_CONTRASTABC, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0x8A, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0x51, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0x8A, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_CONTRASTMASTER, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0xCF, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_SETVSL, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0xA0, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0xB5, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0x55, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_PRECHARGE2, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0x01, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_DISPLAYON, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    return OLED_STATUS_SUCCESS;
}

oled_status_t OLED_Deinit(void)
{
    /* Deinit the DSPI */
    DSPI_Deinit(BOARD_OLED_DSPI_MASTER_BASEADDR);
    return OLED_STATUS_SUCCESS;
}

oled_status_t OLED_SendData(const uint8_t *dataToSend, uint32_t dataSize)
{
    oled_status_t spiStatus;

    /* Check if the default color is white */
    if ((GUI_COLOR_WHITE != colorMask))
    {
        uint16_t *arrayPtr = (uint16_t *)dataToSend;

        for (uint32_t i = 0; i < dataSize / 2; i++)
        {
            arrayPtr[i] &= colorMask;
        }
    }
    /* Send command to OLED */
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_WRITERAM, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    /* Set DC pin and CS pin to enable OLED transfer */
    GPIO_PortSet(BOARD_OLED_DC_GPIO, 1U << BOARD_OLED_DC_PIN);
    GPIO_PortClear(BOARD_OLED_CS_GPIO, 1U << BOARD_OLED_CS_PIN);

    spiStatus = OLED_SendViaDSPI(dataToSend, dataSize);

    if (OLED_STATUS_SUCCESS != spiStatus)
    {
        GPIO_PortSet(BOARD_OLED_CS_GPIO, 1U << BOARD_OLED_CS_PIN);
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    GPIO_PortSet(BOARD_OLED_CS_GPIO, 1U << BOARD_OLED_CS_PIN);
    return OLED_STATUS_SUCCESS;
}

oled_status_t OLED_DrawBox(uint16_t xCrd, uint16_t yCrd, uint16_t width, uint16_t height, uint16_t color)
{
    oled_status_t status = OLED_STATUS_SUCCESS;

    oled_dynamic_area_t boxArea;

    boxArea.xCrd   = xCrd;
    boxArea.yCrd   = yCrd;
    boxArea.width  = width;
    boxArea.height = height;

    uint32_t boxSize = width * height;

    OLED_SetDynamicArea(&boxArea);

    /* Helper pointer */
    uint8_t *boxBuf = (uint8_t *)oled_dynamic_area.areaBuffer;

    if (NULL == boxBuf)
    {
        return OLED_STATUS_ERROR;
    }

    /* check the bounds */
    if (AreCoordsNotValid(xCrd, yCrd, width, height))
    {
        status = OLED_STATUS_INIT_ERROR;
    }

    else
    {
        /* Fill the buffer with color */
        for (uint16_t i = 0; i < boxSize; i++)
        {
            boxBuf[2 * i]     = color >> 8;
            boxBuf[2 * i + 1] = color;
        }

        /* Adjust for the offset */
        OLED_AdjustColumnOffset(xCrd);
        OLED_AdjustRowOffset(yCrd);

        if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_SET_COLUMN, FIRST_BYTE))
        {
            return OLED_STATUS_PROTOCOL_ERROR;
        }
        if (OLED_STATUS_SUCCESS != OLED_SendCmd(xCrd, OTHER_BYTE))
        {
            return OLED_STATUS_PROTOCOL_ERROR;
        }
        if (OLED_STATUS_SUCCESS != OLED_SendCmd(xCrd + (width - 1), OTHER_BYTE))
        {
            return OLED_STATUS_PROTOCOL_ERROR;
        }

        if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_SET_ROW, FIRST_BYTE))
        {
            return OLED_STATUS_PROTOCOL_ERROR;
        }
        if (OLED_STATUS_SUCCESS != OLED_SendCmd(yCrd, OTHER_BYTE))
        {
            return OLED_STATUS_PROTOCOL_ERROR;
        }
        if (OLED_STATUS_SUCCESS != OLED_SendCmd(yCrd + (height - 1), OTHER_BYTE))
        {
            return OLED_STATUS_PROTOCOL_ERROR;
        }

        /* Fill the GRAM */
        if (OLED_STATUS_SUCCESS != OLED_SendData((uint8_t *)boxBuf, boxSize * OLED_BYTES_PER_PIXEL))
        {
            return OLED_STATUS_PROTOCOL_ERROR;
        }
        /* Destory the dynamic area */
        OLED_DestroyDynamicArea();
        status = OLED_STATUS_SUCCESS;
    }

    return status;
}

oled_status_t OLED_FillScreen(uint16_t color)
{
    /* Fill the screen buffer with color */
    for (uint16_t i = 0; i < (OLED_SCREEN_WIDTH * OLED_SCREEN_HEIGHT); i++)
    {
        screenBuf[2 * i]     = color >> 8;
        screenBuf[2 * i + 1] = color;
    }

    /* Set the locations */
    SetBorders(0, 0, OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT);

    /* Fill GRAM */
    if (OLED_STATUS_SUCCESS !=
        OLED_SendData((uint8_t *)screenBuf, OLED_SCREEN_WIDTH * OLED_SCREEN_HEIGHT * OLED_BYTES_PER_PIXEL))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    return OLED_STATUS_SUCCESS;
}

oled_status_t OLED_DrawPixel(int16_t xCrd, int16_t yCrd, uint16_t color)
{
    /* Check the bounds */
    if (AreCoordsNotValid(xCrd, yCrd, 1, 1))
    {
        return OLED_STATUS_INIT_ERROR;
    }
    else
    {
        /* Set directions */
        SetBorders(xCrd, yCrd, OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT);
        /* swap bytes */
        uint16_t dot = color;
        OLED_SwapMe(dot);

        /* Fill the GRAM */
        if (OLED_STATUS_SUCCESS != OLED_SendData((uint8_t *)&dot, 2))
        {
            return OLED_STATUS_PROTOCOL_ERROR;
        }
        return OLED_STATUS_SUCCESS;
    }
}

oled_status_t OLED_DrawScreen(
    const uint8_t *image, uint8_t xCrd, uint8_t yCrd, uint8_t width, uint8_t height, oled_transition_t transition)
{
    oled_status_t status = OLED_STATUS_SUCCESS;

    if (AreCoordsNotValid(xCrd, yCrd, width, height))
    {
        return OLED_STATUS_INIT_ERROR;
    }

    switch (transition)
    {
        case kOLED_Transition_None:
        {
            /* Set the locations */
            SetBorders(xCrd, yCrd, width, height);

            /* Fill the GRAM */
            if (OLED_STATUS_SUCCESS != OLED_SendData((const uint8_t *)image, width * height * OLED_BYTES_PER_PIXEL))
            {
                return OLED_STATUS_PROTOCOL_ERROR;
            }
            break;
        }

        case kOLED_Transition_Top_Down:
        {
            TopDown(image, xCrd, yCrd, width, height);
            break;
        }

        case kOLED_Transition_Down_Top:
        {
            DownTop(image, xCrd, yCrd, width, height);
            break;
        }

        case kOLED_Transition_Left_Right:
        {
            LeftRight(image, xCrd, yCrd, width, height);
            break;
        }

        case kOLED_Transition_Right_Left:
        {
            RightLeft(image, xCrd, yCrd, width, height);
            break;
        }

        default:
        {
        }
    }

    return status;
}

oled_status_t OLED_SetFont(const uint8_t *newFont, uint16_t newColor)
{
    selectedFont = newFont;

    selectedFont_firstChar = newFont[2] | ((uint16_t)newFont[3] << 8);
    selectedFont_height    = newFont[6];
    selectedFont_color     = newColor;

    OLED_SwapMe(selectedFont_color);

    isFontInitialized = 1;
    return OLED_STATUS_SUCCESS;
}

oled_status_t OLED_SetDynamicArea(oled_dynamic_area_t *dynamic_area)
{
    if (NULL == oled_dynamic_area.areaBuffer)
    {
        oled_dynamic_area.areaBuffer = (oled_pixel_t)AllocateDynamicArea(dynamic_area->width * dynamic_area->height);
    }
    else if ((dynamic_area->height != oled_dynamic_area.height) || (dynamic_area->width != oled_dynamic_area.width))
    {
        OLED_DestroyDynamicArea();
        oled_dynamic_area.areaBuffer = (oled_pixel_t)AllocateDynamicArea(dynamic_area->width * dynamic_area->height);
    }

    if (!oled_dynamic_area.areaBuffer)
    {
        return OLED_STATUS_ERROR;
    }

    oled_dynamic_area.xCrd   = dynamic_area->xCrd;
    oled_dynamic_area.yCrd   = dynamic_area->yCrd;
    oled_dynamic_area.width  = dynamic_area->width;
    oled_dynamic_area.height = dynamic_area->height;

    return OLED_STATUS_SUCCESS;
}

void OLED_DestroyDynamicArea()
{
    if (NULL != oled_dynamic_area.areaBuffer)
    {
        DestroyDynamicArea(oled_dynamic_area.areaBuffer);
        oled_dynamic_area.areaBuffer = NULL;
    }
}

void OLED_SetTextProperties(oled_text_properties_t *textProperties)
{
    oled_text_properties.font       = textProperties->font;
    oled_text_properties.fontColor  = textProperties->fontColor;
    oled_text_properties.alignParam = textProperties->alignParam;
    oled_text_properties.background = textProperties->background;

    OLED_SetFont(oled_text_properties.font, oled_text_properties.fontColor);
}

uint8_t OLED_GetTextWidth(const uint8_t *text)
{
    uint8_t chrCnt     = 0;
    uint8_t text_width = 0;

    while (0 != text[chrCnt])
    {
        text_width += *(selectedFont + 8 + (uint16_t)((text[chrCnt++] - selectedFont_firstChar) << 2));
        text_width++;
    }

    text_width--;
    return text_width;
}

uint8_t OLED_CharCount(uint8_t width, const uint8_t *font, const uint8_t *text, uint8_t length)
{
    uint8_t chrCnt     = 0;
    uint8_t text_width = 0;
    uint16_t firstChar;

    firstChar = font[2] | ((uint16_t)font[3] << 8);

    while (chrCnt < length)
    {
        text_width += *(font + 8 + (uint16_t)((text[chrCnt++] - firstChar) << 2));
        if (text_width > width)
        {
            chrCnt--;
            break;
        }

        text_width++;
    }

    return chrCnt;
}

oled_status_t OLED_AddText(const uint8_t *text)
{
    uint16_t chrCnt       = 0;
    oled_pixel_t chrBuf   = NULL;
    uint8_t currentChar_x = 0, currentChar_y = 0;
    uint8_t text_height = 0, text_width = 0;

    text_width  = OLED_GetTextWidth(text);
    text_height = selectedFont_height;

    if ((oled_dynamic_area.width < text_width) || (oled_dynamic_area.height < text_height))
    {
        oled_dynamic_area_t textArea = {0};

        textArea.width  = text_width;
        textArea.height = text_height;
        OLED_SetDynamicArea(&textArea);
    }

    currentChar_y = (oled_dynamic_area.height - text_height) >> 1;

    switch (oled_text_properties.alignParam & OLED_TEXT_HALIGN_MASK)
    {
        case OLED_TEXT_ALIGN_LEFT:
        {
            currentChar_x = 0;
            break;
        }

        case OLED_TEXT_ALIGN_RIGHT:
        {
            currentChar_x = (oled_dynamic_area.width - text_width);
            break;
        }

        case OLED_TEXT_ALIGN_CENTER:
        {
            currentChar_x += (oled_dynamic_area.width - text_width) >> 1;
            break;
        }

        default:
        {
            break;
        }
    }

    if (CreateTextBackground() != OLED_STATUS_SUCCESS)
    {
        return OLED_STATUS_ERROR;
    }

    chrCnt = 0;
    while (0 != text[chrCnt])
    {
        WriteCharToBuf(text[chrCnt++], &chrBuf);

        if (NULL == chrBuf)
        {
            return OLED_STATUS_INIT_ERROR;
        }

        else
        {
            if (((currentChar_x + currentChar_width) > oled_dynamic_area.width) ||
                ((currentChar_y + currentChar_height) > oled_dynamic_area.height))
            {
                DestroyDynamicArea(chrBuf);
                chrBuf = NULL;
                return OLED_STATUS_ERROR;
            }

            /* Copy data */
            oled_pixel_t copyAddr =
                oled_dynamic_area.areaBuffer + (currentChar_y * oled_dynamic_area.width + currentChar_x);
            AddCharToTextArea(chrBuf, currentChar_width, currentChar_height, copyAddr, oled_dynamic_area.width);

            currentChar_x += (currentChar_width + 1);
            currentChar_y += 0;

            DestroyDynamicArea(chrBuf);
            chrBuf = NULL;
        }
    }

    return OLED_STATUS_SUCCESS;
}

oled_status_t OLED_DrawText(const uint8_t *text)
{
    if (NULL == text)
    {
        return OLED_STATUS_ERROR;
    }

    OLED_AddText(text);

    /* Set the locations */
    SetBorders(oled_dynamic_area.xCrd, oled_dynamic_area.yCrd, oled_dynamic_area.width, oled_dynamic_area.height);

    /* Fill the GRAM */
    if (OLED_STATUS_SUCCESS != OLED_SendData((const uint8_t *)oled_dynamic_area.areaBuffer,
                                             oled_dynamic_area.width * oled_dynamic_area.height * OLED_BYTES_PER_PIXEL))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    return OLED_STATUS_SUCCESS;
}

void OLED_GetImageDimensions(uint8_t *width, uint8_t *height, const uint8_t *image)
{
    *height = image[2] + (image[3] << 8);
    *width  = image[4] + (image[5] << 8);
}

oled_status_t OLED_AddImage(const uint8_t *image)
{
    oled_status_t status = OLED_STATUS_SUCCESS;

    /* Check the bounds */
    if (AreCoordsNotValid(oled_dynamic_area.xCrd, oled_dynamic_area.yCrd, oled_dynamic_area.width,
                          oled_dynamic_area.height))
    {
        status = OLED_STATUS_INIT_ERROR;
    }
    else
    {
        OLED_Swap((oled_pixel_t)oled_dynamic_area.areaBuffer, GuiDriver_SkipHeader(image),
                  oled_dynamic_area.width * oled_dynamic_area.height);
    }

    return status;
}

oled_status_t OLED_DrawImage(const uint8_t *image)
{
    OLED_AddImage(image);

    /* Set the locations */
    SetBorders(oled_dynamic_area.xCrd, oled_dynamic_area.yCrd, oled_dynamic_area.width, oled_dynamic_area.height);

    /* Fill the GRAM */
    if (OLED_STATUS_SUCCESS != OLED_SendData((const uint8_t *)oled_dynamic_area.areaBuffer,
                                             oled_dynamic_area.width * oled_dynamic_area.height * OLED_BYTES_PER_PIXEL))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    return OLED_STATUS_SUCCESS;
}

oled_status_t OLED_DimScreenON(void)
{
    for (int i = 0; i < 16; i++)
    {
        if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_CONTRASTMASTER, FIRST_BYTE))
        {
            return OLED_STATUS_PROTOCOL_ERROR;
        }
        if (OLED_STATUS_SUCCESS != OLED_SendCmd(0xC0 | (0xF - i), OTHER_BYTE))
        {
            return OLED_STATUS_PROTOCOL_ERROR;
        }
        Timer_Delay_ms(20);
    }
    return OLED_STATUS_SUCCESS;
}

oled_status_t OLED_DimScreenOFF(void)
{
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_CONTRASTMASTER, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(0xC0 | 0xF, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    return OLED_STATUS_SUCCESS;
}

void OLED_Swap(oled_pixel_t imgDst, const uint8_t *imgSrc, uint16_t imgSize)
{
    for (int var = 0; var < imgSize; var++)
    {
        *imgDst = *imgSrc << 8;
        imgSrc++;
        *imgDst |= *imgSrc;
        imgDst++;
        imgSrc++;
    }
}

/* Intern functions */
static void SetBorders(uint8_t xCrd, uint8_t yCrd, uint8_t width, uint8_t height)
{
    oled_status_t oledStatus;

    /* Adjust for the offset */
    OLED_AdjustColumnOffset(xCrd);
    OLED_AdjustRowOffset(yCrd);

    while (1)
    {
        oledStatus = OLED_STATUS_SUCCESS;

        oledStatus |= OLED_SendCmd(OLED_CMD_SET_COLUMN, FIRST_BYTE);
        oledStatus |= OLED_SendCmd(xCrd, OTHER_BYTE);
        oledStatus |= OLED_SendCmd(xCrd + (width - 1), OTHER_BYTE);
        oledStatus |= OLED_SendCmd(OLED_CMD_SET_ROW, FIRST_BYTE);
        oledStatus |= OLED_SendCmd(yCrd, OTHER_BYTE);
        oledStatus |= OLED_SendCmd(yCrd + (height - 1), OTHER_BYTE);

        if (OLED_STATUS_SUCCESS == oledStatus)
        {
            break;
        }
    }
}

static void WriteCharToBuf(uint16_t charToWrite, oled_pixel_t *chrBuf)
{
    uint8_t foo             = 0, mask;
    const uint8_t *pChTable = selectedFont + 8 + (uint16_t)((charToWrite - selectedFont_firstChar) << 2);
    currentChar_width = *pChTable, currentChar_height = selectedFont_height;
    uint32_t offset          = (uint32_t)pChTable[1] | ((uint32_t)pChTable[2] << 8) | ((uint32_t)pChTable[3] << 16);
    const uint8_t *pChBitMap = selectedFont + offset;

    if (0 == isFontInitialized)
    {
        /* Set default font */
        OLED_SetFont(guiFont_10x10_Regular, 0xFFFF);
    }

    /* Allocate space for char image */
    *chrBuf = (oled_pixel_t)AllocateDynamicArea(currentChar_height * currentChar_width);

    if (NULL == *chrBuf)
    {
        return;
    }

    for (uint8_t yCnt = 0; yCnt < currentChar_height; yCnt++)
    {
        mask = 0;

        for (uint8_t xCnt = 0; xCnt < currentChar_width; xCnt++)
        {
            if (0 == mask)
            {
                mask = 1;
                foo  = *pChBitMap++;
            }

            if (0 != (foo & mask))
            {
                *(*chrBuf + yCnt * currentChar_width + xCnt) = selectedFont_color;
            }

            else
            {
                *(*chrBuf + yCnt * currentChar_width + xCnt) = 0;
            }

            mask <<= 1;
        }
    }
}

static void Transpose(oled_pixel_t transImage, const oled_pixel_t image, uint8_t width, uint8_t height)
{
    for (uint8_t i = 0; i < height; i++)
    {
        for (uint8_t j = 0; j < width; j++)
        {
            transImage[j * height + i] = image[i * width + j];
        }
    }
}

static oled_status_t TopDown(const uint8_t *image, uint8_t xCrd, uint8_t yCrd, uint8_t width, uint8_t height)
{
    uint16_t transStep   = OLED_TRANSITION_STEP;
    uint16_t partImgSize = width * transStep;
    uint8_t *partImgPtr  = (uint8_t *)image + (height - transStep) * (width * OLED_BYTES_PER_PIXEL);

    /* Set locations */
    while (1)
    {
        SetBorders(xCrd, yCrd, width, height);

        if (partImgSize > width * height)
        {
            if (OLED_STATUS_SUCCESS != OLED_SendData((const uint8_t *)image, width * height * OLED_BYTES_PER_PIXEL))
            {
                return OLED_STATUS_PROTOCOL_ERROR;
            }
            break;
        }
        else
        {
            if (OLED_STATUS_SUCCESS != OLED_SendData((const uint8_t *)partImgPtr, partImgSize * OLED_BYTES_PER_PIXEL))
            {
                return OLED_STATUS_PROTOCOL_ERROR;
            }
        }

        /* Update variables */
        partImgPtr -= (width * transStep) * OLED_BYTES_PER_PIXEL;
        partImgSize += (width * transStep);
        transStep++;
    }

    return OLED_STATUS_SUCCESS;
}

static oled_status_t DownTop(const uint8_t *image, uint8_t xCrd, uint8_t yCrd, uint8_t width, uint8_t height)
{
    uint16_t transStep   = OLED_TRANSITION_STEP;
    uint16_t partImgSize = width * transStep;
    oled_status_t status = OLED_STATUS_SUCCESS;
    uint8_t *partImgPtr  = (uint8_t *)image;
    uint8_t yCrd_moving  = (yCrd + height) - 1;

    /* Set locations */
    while (1)
    {
        if ((partImgSize > OLED_SCREEN_SIZE) || (yCrd_moving < yCrd))
        {
            /* Draw full image */
            SetBorders(xCrd, yCrd, width, height);
            if (OLED_STATUS_SUCCESS != OLED_SendData((const uint8_t *)image, width * height * OLED_BYTES_PER_PIXEL))
            {
                return OLED_STATUS_PROTOCOL_ERROR;
            }
            break;
        }

        else
        {
            SetBorders(xCrd, yCrd_moving, width, (yCrd + height) - yCrd_moving);
            if (OLED_STATUS_SUCCESS != OLED_SendData((const uint8_t *)partImgPtr, partImgSize * OLED_BYTES_PER_PIXEL))
            {
                return OLED_STATUS_PROTOCOL_ERROR;
            }
        }

        /* Update variables */
        yCrd_moving -= transStep;
        partImgSize += (width * transStep);
        transStep++;
    }

    return status;
}

static oled_status_t LeftRight(const uint8_t *image, uint8_t xCrd, uint8_t yCrd, uint8_t width, uint8_t height)
{
    oled_status_t status               = OLED_STATUS_SUCCESS;
    oled_dynamic_area_t transImageArea = {.xCrd = 0, .yCrd = 0, .width = 96, .height = 96};

    OLED_SetDynamicArea(&transImageArea);

    /* Helper pointer */
    oled_pixel_t transImage = (oled_pixel_t)oled_dynamic_area.areaBuffer;

    if (NULL == transImage)
    {
        return OLED_STATUS_INIT_ERROR;
    }

    Transpose((oled_pixel_t)transImage, (oled_pixel_t)image, width, height);

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_SET_REMAP, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_REMAP_SETTINGS | REMAP_VERTICAL_INCREMENT, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    uint16_t transStep   = OLED_TRANSITION_STEP;
    uint16_t partImgSize = height * transStep;
    uint8_t *partImgPtr  = (uint8_t *)transImage + (width - transStep) * (height * OLED_BYTES_PER_PIXEL);

    /* Set locations */
    while (1)
    {
        SetBorders(xCrd, yCrd, width, height);

        if (partImgSize > width * height)
        {
            if (OLED_STATUS_SUCCESS !=
                OLED_SendData((const uint8_t *)transImage, width * height * OLED_BYTES_PER_PIXEL))
            {
                return OLED_STATUS_PROTOCOL_ERROR;
            }
            break;
        }
        else
        {
            if (OLED_STATUS_SUCCESS != OLED_SendData((const uint8_t *)partImgPtr, partImgSize * OLED_BYTES_PER_PIXEL))
            {
                return OLED_STATUS_PROTOCOL_ERROR;
            }
        }

        if (OLED_STATUS_SUCCESS == status)
        {
            /* Update variables */
            partImgPtr -= (transStep * height) * OLED_BYTES_PER_PIXEL;
            partImgSize += (transStep * height);
            transStep++;
        }
    }

    if (OLED_STATUS_SUCCESS == status)
    {
        if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_SET_REMAP, FIRST_BYTE))
        {
            return OLED_STATUS_PROTOCOL_ERROR;
        }
        if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_REMAP_SETTINGS, OTHER_BYTE))
        {
            return OLED_STATUS_PROTOCOL_ERROR;
        }
    }

    OLED_DestroyDynamicArea();
    return status;
}

static oled_status_t RightLeft(const uint8_t *image, uint8_t xCrd, uint8_t yCrd, uint8_t width, uint8_t height)
{
    oled_dynamic_area_t transImageArea = {.xCrd = 0, .yCrd = 0, .width = 96, .height = 96};

    OLED_SetDynamicArea(&transImageArea);

    /* Helper pointer */
    oled_pixel_t transImage = oled_dynamic_area.areaBuffer;

    if (NULL == transImage)
    {
        return OLED_STATUS_INIT_ERROR;
    }

    Transpose((oled_pixel_t)transImage, (oled_pixel_t)image, width, height);

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_SET_REMAP, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_REMAP_SETTINGS | REMAP_VERTICAL_INCREMENT, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }

    uint16_t transStep   = OLED_TRANSITION_STEP;
    uint16_t partImgSize = height * transStep;
    uint8_t *partImgPtr  = (uint8_t *)transImage;
    uint8_t xCrd_moving  = (xCrd + width) - 1;

    /* Set locations */
    while (1)
    {
        if ((partImgSize > width * height) || (xCrd_moving < xCrd))
        {
            SetBorders(xCrd, yCrd, width, height);
            if (OLED_STATUS_SUCCESS !=
                OLED_SendData((const uint8_t *)transImage, height * width * OLED_BYTES_PER_PIXEL))
            {
                return OLED_STATUS_PROTOCOL_ERROR;
            }
            break;
        }

        else
        {
            SetBorders(xCrd_moving, yCrd, (xCrd + width) - xCrd_moving, height);
            if (OLED_STATUS_SUCCESS != OLED_SendData((const uint8_t *)partImgPtr, partImgSize * OLED_BYTES_PER_PIXEL))
            {
                return OLED_STATUS_PROTOCOL_ERROR;
            }
        }

        /* Update variables*/
        xCrd_moving -= transStep;
        partImgSize += (height * transStep);
        transStep++;
    }

    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_CMD_SET_REMAP, FIRST_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    if (OLED_STATUS_SUCCESS != OLED_SendCmd(OLED_REMAP_SETTINGS, OTHER_BYTE))
    {
        return OLED_STATUS_PROTOCOL_ERROR;
    }
    /* Destory the dynamic area */
    OLED_DestroyDynamicArea();
    return OLED_STATUS_SUCCESS;
}

static oled_status_t CreateTextBackground(void)
{
    uint8_t xCrd = oled_dynamic_area.xCrd, yCrd = oled_dynamic_area.yCrd, width = oled_dynamic_area.width,
            height            = oled_dynamic_area.height;
    oled_pixel_t imgBuf       = oled_dynamic_area.areaBuffer, copyAddr;
    const uint8_t *background = oled_text_properties.background;

    /* Copy data */
    if ((NULL == imgBuf) || ((xCrd + width) > OLED_SCREEN_WIDTH) || ((yCrd + height) > OLED_SCREEN_HEIGHT))
    {
        return OLED_STATUS_INIT_ERROR;
    }

    if (NULL == background)
    {
        for (uint8_t i = 0; i < height; i++)
        {
            memset((void *)imgBuf, 0, width * OLED_BYTES_PER_PIXEL);
            imgBuf += width;
        }
    }

    else
    {
        copyAddr = (oled_pixel_t)(GuiDriver_SkipHeader(background)) + (yCrd * OLED_SCREEN_WIDTH + xCrd);
        for (uint8_t i = 0; i < height; i++)
        {
            OLED_Swap((oled_pixel_t)imgBuf, (const uint8_t *)copyAddr, width);
            imgBuf += width;
            copyAddr += OLED_SCREEN_WIDTH;
        }
    }

    return OLED_STATUS_SUCCESS;
}

static oled_status_t AddCharToTextArea(
    oled_pixel_t chrPtr, uint8_t chrWidth, uint8_t chrHeight, oled_pixel_t copyAddr, uint8_t imgWidth)
{
    if (NULL == copyAddr)
    {
        return OLED_STATUS_INIT_ERROR;
    }

    for (uint8_t i = 0; i < chrHeight; i++)
    {
        for (uint8_t j = 0; j < chrWidth; j++)
        {
            if (0 != chrPtr[j])
            {
                copyAddr[j] = chrPtr[j];
            }
        }
        copyAddr += imgWidth;
        chrPtr += chrWidth;
    }
    return OLED_STATUS_SUCCESS;
}

static void *AllocateDynamicArea(uint32_t area)
{
    void *ptr = malloc(area * OLED_BYTES_PER_PIXEL);
    if (NULL == ptr)
    {
        return NULL;
    }
    return ptr;
}

static oled_status_t DestroyDynamicArea(void *ptr)
{
    if (NULL == ptr)
    {
        return OLED_STATUS_INIT_ERROR;
    }
    else
    {
        free(ptr);
        return OLED_STATUS_SUCCESS;
    }
}
