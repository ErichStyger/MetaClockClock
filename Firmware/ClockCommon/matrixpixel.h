/*
 * matrixpixel.h
 *
 * Copyright (c) 2020: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#ifndef MATRIXPIXEL_H_
#define MATRIXPIXEL_H_

#include "platform.h"
#include <stdint.h>

#if PL_CONFIG_USE_LED_PIXEL
  #include "NeoConfig.h"

  #define MPIXEL_NOF_X    (NEOC_NOF_LEDS_IN_ROW)
  #define MPIXEL_NOF_Y    (NEOC_NOF_LEDS_IN_COL)

/*!
 * \brief Sets the RGB color for one matrix pixel.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \param red Red color component.
 * \param green Green color component.
 * \param blue Blue color component.
 */
  void MPIXEL_SetColor(int32_t x, int32_t y, int32_t z, uint8_t red, uint8_t green, uint8_t blue);

/*!
 * \brief Gets the RGB color for one matrix pixel.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \return Requested numeric value.
 */
  uint32_t MPIXEL_GetColor(int32_t x, int32_t y, int32_t z);

/*!
 * \brief Clears all matrix pixel colors.
 */
  void MPIXEL_ClearAll(void);
#endif

#endif /* MATRIXPIXEL_H_ */
