/*
 * platform_id.h
 *
 * Author: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_ID_H_
#define PLATFORM_ID_H_

/* list of available board IDs: */
#define PL_CONFIG_BOARD_ID_CLOCK_LPC845_2X2             (1)  /* Board V0.1: initial version ('world clock') with 2x2 arrangement */
#define PL_CONFIG_BOARD_ID_CLOCK_LPC845_1X4             (2)  /* Board V1.0: second version with 1x4 arrangement, different pin assignment and better hall sensors */
#define PL_CONFIG_BOARD_ID_MASTER_K22FN512              (3)  /* Board V1.0 RS-485 Master 16-Mar-2020, board with tinyK22 and sensors */
#define PL_CONFIG_BOARD_ID_CLOCK_K02FN64                (4)  /* Board V1.1, dual clock with K02FN64 */
#define PL_CONFIG_BOARD_ID_CLOCK_K02FN128               (5)  /* Board V1.4, dual clock with K02FN128 */
#define PL_CONFIG_BOARD_ID_MASTER_LPC845_BRK            (6)  /* Board LPC-845-BRK breadboard as master */
#define PL_CONFIG_BOARD_ID_SMARTWALL_K02FN64            (7)  /* SmArtWall Board V1.0 with 4 (or 8) motors using K02FN64 */
#define PL_CONFIG_BOARD_ID_SMARTWALL_K02FN128           (8)  /* SmArtWall Board V1.0 with 4 (or 8) motors using K02FN12 */
#define PL_CONFIG_BOARD_ID_PIXELUNIT_MASTER_K22FN512    (9)  /* PixelUnit Master with tinyK22 */
/* \todo update naming above to PixelUnit */


/* \todo provide better way to provide master configurations */
#define PL_MATRIX_ID_CLOCK_8x3          (1)  /* first small 'meta-clock, 6 LPC845 boards building a 8x3 matrix */
#define PL_MATRIX_ID_CLOCK_12x5_60B     (2)  /* McuOneEclipse '60 billion lights' configuration: LPC845 boards (4 clocks), total 60 clocks, with RGB ring controlled by tinyK22 */
#define PL_MATRIX_ID_CLOCK_12x5_MOD     (3)  /* 60 'modular' clock configuration */
#define PL_MATRIX_ID_CLOCK_12x9_ALEXIS  (4)  /* 16x9 matrix by Alexis */
#define PL_MATRIX_ID_SMARTWALL_8x5      (5)  /* Sm(A)rtwall with 5 boards with 8 pixels each stacked up */


#endif /* PLATFORM_ID_H_ */
