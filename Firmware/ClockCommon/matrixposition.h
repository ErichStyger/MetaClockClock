/*
 * matrixposition.h
 *
 * Copyright (c) 2020: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 *
 * Interface to set motor position in the matrix
 */

#ifndef HANDS_H_
#define HANDS_H_

#include "platform.h"
#include <stdint.h>
#include "stepper.h"

#define MPOS_ANGLE_HIDE   (225)  /* special angle position to 'hide' */

/* set the angle of a single hand */
/*!
 * \brief Sets the absolute target angle for one hand.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \param angle Angle value in degrees.
 */
void MPOS_SetAngle(uint8_t x, uint8_t y, uint8_t z, int16_t angle);
/*!
 * \brief Sets the absolute target angle for one hand if the coordinate is active.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \param angle Angle value in degrees.
 */
void MPOS_SetAngleChecked(uint8_t x, uint8_t y, uint8_t z, int16_t angle);

#if MATRIX_NOF_STEPPERS_Z==2 /* the special functions below are only available for dual shaft motors */
/* set the angle of two hands: z==0 and z==1 */
/*!
 * \brief Sets absolute target angles for both hands of one clock.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z0Angle Angle value in degrees.
 * \param z1Angle Angle value in degrees.
 */
void MPOS_SetAngleZ0Z1(uint8_t x, uint8_t y, int16_t z0Angle, int16_t z1Angle);
/*!
 * \brief Sets absolute target angles for both hands if the coordinate is active.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z0Angle Angle value in degrees.
 * \param z1Angle Angle value in degrees.
 */
void MPOS_SetAngleZ0Z1Checked(uint8_t x, uint8_t y, int16_t z0Angle, int16_t z1Angle);

/* set the angle of all clocks for z==0 and z==1 */
/*!
 * \brief Sets absolute target angles for both hands on all clocks.
 * \param z0Angle Angle value in degrees.
 * \param z1Angle Angle value in degrees.
 */
void MPOS_SetAngleZ0Z1All(int16_t z0Angle, int16_t z1Angle);
#endif

/* set the angle for all motors */
/*!
 * \brief Sets the absolute target angle for all hands.
 * \param angle Angle value in degrees.
 */
void MPOS_SetAngleAll(int16_t angle);

/*!
 * \brief Sets the move mode for one hand.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \param mode Stepper or clock move mode.
 */
void MPOS_SetMoveMode(uint8_t x, uint8_t y, uint8_t z, STEPPER_MoveMode_e mode);
/*!
 * \brief Sets the move mode for one hand if the coordinate is active.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \param mode Stepper or clock move mode.
 */
void MPOS_SetMoveModeChecked(uint8_t x, uint8_t y, uint8_t z, STEPPER_MoveMode_e mode);
#if MATRIX_NOF_STEPPERS_Z==2 /* the special functions below are only available for dual shaft motors */
/*!
 * \brief Sets move modes for both hands of one clock.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param mode0 Stepper or clock move mode.
 * \param mode1 Stepper or clock move mode.
 */
void MPOS_SetMoveModeZ0Z1(uint8_t x, uint8_t y, STEPPER_MoveMode_e mode0, STEPPER_MoveMode_e mode1);
/*!
 * \brief Sets move modes for both hands if the coordinate is active.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param mode0 Stepper or clock move mode.
 * \param mode1 Stepper or clock move mode.
 */
void MPOS_SetMoveModeZ0Z1Checked(uint8_t x, uint8_t y, STEPPER_MoveMode_e mode0, STEPPER_MoveMode_e mode1);
/*!
 * \brief Sets move modes for both hands on all clocks.
 * \param mode0 Stepper or clock move mode.
 * \param mode1 Stepper or clock move mode.
 */
void MPOS_SetMoveModeZ0Z1All(STEPPER_MoveMode_e mode0, STEPPER_MoveMode_e mode1);
#endif
/*!
 * \brief Sets the move mode for all hands.
 * \param mode Stepper or clock move mode.
 */
void MPOS_SetMoveModeAll(STEPPER_MoveMode_e mode);

#if PL_CONFIG_USE_RELATIVE_MOVES
/*!
 * \brief Adds a relative angle move for one hand.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param z Matrix Z coordinate or hand index.
 * \param angle Angle value in degrees.
 */
void MPOS_RelativeMove(uint8_t x, uint8_t y, uint8_t z, int16_t angle);
#if MATRIX_NOF_STEPPERS_Z==2 /* the special functions below are only available for dual shaft motors */
/*!
 * \brief Adds relative angle moves for both hands of one clock.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param angle0 Angle value in degrees.
 * \param angle1 Angle value in degrees.
 */
void MPOS_RelativeMoveZ0Z1(uint8_t x, uint8_t y, int16_t angle0, int16_t angle1);
/*!
 * \brief Adds checked relative moves for both hands of one clock.
 * \param x Matrix X coordinate.
 * \param y Matrix Y coordinate.
 * \param angleZ0 Angle value in degrees.
 * \param angleZ1 Angle value in degrees.
 */
void MPOS_SetRelativeMoveZ0Z1Checked(int x, int y, int angleZ0, int angleZ1);
#endif
/*!
 * \brief Adds a relative angle move for all hands.
 * \param angle Angle value in degrees.
 */
void MPOS_RelativeMoveAll(int16_t angle);
#endif

#endif /* HANDS_H_ */
