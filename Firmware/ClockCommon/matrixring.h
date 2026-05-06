/*
 * matrixring.h
 *
 * Copyright (c) 2020: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#ifndef MATRIXRING_H_
#define MATRIXRING_H_

#include <stdint.h>
#include <stdbool.h>

#if PL_CONFIG_USE_LED_RING

/*!
 * \brief Sets one LED pixel color in a ring at a matrix coordinate.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param pos Pixel or step position.
 * \param red Red color component.
 * \param green Green color component.
 * \param blue Blue color component.
 */
void MRING_SetRingPixelColor(int32_t x, int32_t y, uint8_t pos, uint8_t red, uint8_t green, uint8_t blue);
/*!
 * \brief Sets the LED ring color at one matrix coordinate.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \param red Red color component.
 * \param green Green color component.
 * \param blue Blue color component.
 */
void MRING_SetRingColor(int32_t x, int32_t y, int32_t z, uint8_t red, uint8_t green, uint8_t blue);
/*!
 * \brief Sets the LED ring color for all rings.
 * \param red Red color component.
 * \param green Green color component.
 * \param blue Blue color component.
 */
void MRING_SetRingColorAll(uint8_t red, uint8_t green, uint8_t blue);

/*!
 * \brief Enables or disables the LED ring at one matrix coordinate.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \param on True to enable or turn on, false to disable or turn off.
 */
void MRING_EnableRing(int32_t x, int32_t y, uint8_t z, bool on);
/*!
 * \brief Enables or disables all LED rings.
 * \param on True to enable or turn on, false to disable or turn off.
 */
void MRING_EnableRingAll(bool on);

#endif

#endif /* MATRIXRING_H_ */
