/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NEOPIXEL_H_
#define NEOPIXEL_H_

#include <stdint.h>
#include "NeoConfig.h"
#include "McuShell.h"

#define NEO_LANE_START          NEOC_LANE_START  /* starting lane number */
#define NEO_LANE_END            NEOC_LANE_END  /* end lane number */
#define NEO_NOF_LEDS_IN_LANE    NEOC_NOF_LEDS_IN_LANE /* number of LEDS in each lane */
#define NEO_NOF_PIXEL           NEOC_NOF_PIXEL /* total number of pixels */

#define NEO_PIXEL_FIRST         (0) /* index of first pixel */

/*!
 * \brief Parses NeoPixel shell commands.
 * \param cmd Command string to parse or send.
 * \param handled Set to true when the command has been handled.
 * \param io Shell I/O streams used for command output.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NEO_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

typedef uint32_t NEO_PixelIdxT;
typedef uint32_t NEO_PixelColor;

#define NEO_SPLIT_RGB(u32)        (((u32)>>16)&0xff), (((u32)>>8)&0xff), ((u32)&0xff)
#define NEO_COMBINE_RGB(r,g,b)    (((uint32_t)(r)<<16) | ((uint32_t)(g)<<8) | (uint32_t)(b))
#if NEOC_NOF_COLORS==4
  #define NEO_SPLIT_WRGB(u32)          (((u32)>>24)&0xff), (((u32)>>16)&0xff), (((u32)>>8)&0xff), ((u32)&0xff)
  #define NEO_COMBINE_WRGB(w,r,g,b)    (((uint32_t)(w)<<24) | ((uint32_t)(r)<<16) | ((uint32_t)(g)<<8) | (uint32_t)(b))
#endif

/*!
 * \brief Clears one NeoPixel.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NEO_ClearPixel(NEO_PixelIdxT x, NEO_PixelIdxT y);

/*!
 * \brief Clears all NeoPixels.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NEO_ClearAllPixel(void);

/*!
 * \brief Sets all NeoPixels to the same RGB color.
 * \param color Packed RGB color value.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NEO_SetAllPixelColor(uint32_t color);
/*!
 * \brief Sets one NeoPixel using a packed color value.
 * \param lane NeoPixel lane index.
 * \param pos Pixel or step position.
 * \param color Packed RGB color value.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NEO_SetPixelColor(NEO_PixelIdxT lane, NEO_PixelIdxT pos, uint32_t color);
/*!
 * \brief Gets one NeoPixel packed color value.
 * \param lane NeoPixel lane index.
 * \param pos Pixel or step position.
 * \param color Packed RGB color value.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NEO_GetPixelColor(NEO_PixelIdxT lane, NEO_PixelIdxT pos, uint32_t *color);

/*!
 * \brief Sets one NeoPixel using RGB components.
 * \param lane NeoPixel lane index.
 * \param pos Pixel or step position.
 * \param red Red color component.
 * \param green Green color component.
 * \param blue Blue color component.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NEO_SetPixelRGB(NEO_PixelIdxT lane, NEO_PixelIdxT pos, uint8_t red, uint8_t green, uint8_t blue);

/*!
 * \brief Gets one NeoPixel as RGB components.
 * \param lane NeoPixel lane index.
 * \param pos Pixel or step position.
 * \param redP Red color component.
 * \param greenP Green color component.
 * \param blueP Blue color component.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NEO_GetPixelRGB(NEO_PixelIdxT lane, NEO_PixelIdxT pos, uint8_t *redP, uint8_t *greenP, uint8_t *blueP);

/*!
 * \brief ORs RGB component bits into one NeoPixel.
 * \param lane NeoPixel lane index.
 * \param pos Pixel or step position.
 * \param red Red color component.
 * \param green Green color component.
 * \param blue Blue color component.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NEO_OrPixelRGB(NEO_PixelIdxT lane, NEO_PixelIdxT pos, uint8_t red, uint8_t green, uint8_t blue);

/*!
 * \brief XORs RGB component bits into one NeoPixel.
 * \param lane NeoPixel lane index.
 * \param pos Pixel or step position.
 * \param red Red color component.
 * \param green Green color component.
 * \param blue Blue color component.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NEO_XorPixelRGB(NEO_PixelIdxT lane, NEO_PixelIdxT pos, uint8_t red, uint8_t green, uint8_t blue);

/*!
 * \brief Scales a packed RGB color by a brightness percentage.
 * \param rgbColor Packed RGB color value.
 * \param percent Brightness percentage.
 * \return Computed packed color value.
 */
NEO_PixelColor NEO_BrightnessPercentColor(NEO_PixelColor rgbColor, uint8_t percent);
/*!
 * \brief Scales a packed RGB color by an 8-bit brightness factor.
 * \param rgbColor Packed RGB color value.
 * \param factor Brightness scale factor.
 * \return Computed packed color value.
 */
NEO_PixelColor NEO_BrightnessFactorColor(NEO_PixelColor rgbColor, uint8_t factor);

/*!
 * \brief Dims one NeoPixel by a percentage.
 * \param lane NeoPixel lane index.
 * \param pos Pixel or step position.
 * \param percent Brightness percentage.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NEO_DimmPercentPixel(NEO_PixelIdxT lane, NEO_PixelIdxT pos, uint8_t percent);

/*!
 * \brief Transfers the buffered NeoPixel data to the LED hardware.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t NEO_TransferPixels(void);

/*!
 * \brief Applies gamma correction to an 8-bit color component.
 * \param color Packed RGB color value.
 * \return Requested 8-bit value.
 */
uint8_t NEO_GammaCorrect8(uint8_t color);

/*!
 * \brief Applies gamma correction to a packed 24-bit RGB color.
 * \param rgb Packed RGB color value.
 * \return Requested numeric value.
 */
uint32_t NEO_GammaCorrect24(uint32_t rgb);

#if NEOC_NOF_COLORS==4
/*!
 * \brief Sets one RGBW NeoPixel using white, red, green, and blue components.
 * \param lane NeoPixel lane index.
 * \param pos Pixel or step position.
 * \param white White color component.
 * \param red Red color component.
 * \param green Green color component.
 * \param blue Blue color component.
 * \return Error code, typically ERR_OK on success.
 */
  uint8_t NEO_SetPixelWRGB(NEO_PixelIdxT lane, NEO_PixelIdxT pos, uint8_t white, uint8_t red, uint8_t green, uint8_t blue);
/*!
 * \brief Gets one RGBW NeoPixel as white, red, green, and blue components.
 * \param lane NeoPixel lane index.
 * \param pos Pixel or step position.
 * \param whiteP White color component.
 * \param redP Red color component.
 * \param greenP Green color component.
 * \param blueP Blue color component.
 * \return Error code, typically ERR_OK on success.
 */
  uint8_t NEO_GetPixelWRGB(NEO_PixelIdxT lane, NEO_PixelIdxT pos, uint8_t *whiteP, uint8_t *redP, uint8_t *greenP, uint8_t *blueP);
#endif

/*!
 * \brief Initializes NeoPixel buffering and hardware output.
 */
void NEO_Init(void);

#endif /* NEOPIXEL_H_ */
