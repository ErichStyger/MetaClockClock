/*
 * Copyright (c) 2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SHIFTREG_CONFIG_H_
#define SHIFTREG_CONFIG_H_

#include "platform.h"

#ifndef SHIFTREG_CONFIG_NOF_DRIVER_BOARDS
  #define SHIFTREG_CONFIG_NOF_DRIVER_BOARDS      (PL_CONFIG_NOF_DRIVER_BOARDS)
    /*!< number of motor driver boards, connected by shift registers */
#endif

#define SHIFTREG_CONFIG_NOF_MOTORS_PER_DRIVER_BOARD   (16) /* there are 16 motors per PCB */
  /*!< number of motors on each motor driver board */

#define SHIFTREG_CONFIG_NOF_MOTORS    (SHIFTREG_CONFIG_NOF_DRIVER_BOARDS*SHIFTREG_CONFIG_NOF_MOTORS_PER_DRIVER_BOARD)
  /*!< number of motors in the system */

/*! \brief GPIO pin settings on master board: */
#define SHIFTREG_CONFIG_MOTORLATCH_GPIO     GPIOD
#define SHIFTREG_CONFIG_MOTORLATCH_PORT     PORTD
#define SHIFTREG_CONFIG_MOTORLATCH_PIN      4U  /* STEPREG_LATCH on PTD4 */

#if 0 /* not used */
#define SHIFTREG_CONFIG_SENSORLATCH_GPIO    GPIOD
#define SHIFTREG_CONFIG_SENSORLATCH_PORT    PORTD
#define SHIFTREG_CONFIG_SENSORLATCH_PIN     0U
#endif

#define SHIFTREG_CONFIG_LATCH_ENABLE_CLOCKS()     CLOCK_EnableClock(kCLOCK_PortD)


#endif /* SHIFTREG_CONFIG_H_ */
