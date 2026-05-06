/*
 * Copyright (c) 20251, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ESP_TIME_H_
#define ESP_TIME_H_

#include "McuTimeDate.h"
#include <stdint.h>

uint8_t EspTime_GetTimeDate(TIMEREC *time, DATEREC *date);

/*!
 * \brief Deinitializes the module.
 */
void EspTime_Deinit(void);
/*!
 * \brief Initializes the module.
 */
void EspTime_Init(void);

#endif /* ESP_TIME_H_ */
