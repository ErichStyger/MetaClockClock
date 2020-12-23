/*
 * matrixposition.h
 *
 * Author: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 *
 * Interface to set motor position in the matrix
 */

#ifndef HANDS_H_
#define HANDS_H_

#include <stdint.h>

#define MPOS_ANGLE_HIDE   (225)  /* special angle position to 'hide' */

/* set the angle of a single hand */
void MPOS_SetAngle(uint8_t x, uint8_t y, uint8_t z, int16_t angle);

/* set the angle of two hands: z==0 and z==1 */
void MPOS_SetAngleZ0Z1(uint8_t x, uint8_t y, int16_t z0Angle, int16_t z1Angle);

/* set the angle of all clocks for z==0 and z==1 */
void MPOS_SetAngleZ0Z1All(int16_t z0Angle, int16_t z1Angle);

/* set the angle for all motors */
void MPOS_SetAngleAll(int16_t angle);

#endif /* HANDS_H_ */