/*
 * Copyright (c) 20251, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ESP_TIME_H_
#define ESP_TIME_H_

#include "McuTimeDate.h"
#include <stdint.h>

/*!
 * \brief Gets the current time and date from the ESP time source.
 * \param time Time record to read or display.
 * \param date Date record to read or display.
 * \return Error code, typically ERR_OK on success.
 */
uint8_t EspTime_GetTimeDate(TIMEREC *time, DATEREC *date);

/*!
 * \brief Deinitializes ESP time support.
 */
void EspTime_Deinit(void);
/*!
 * \brief Initializes ESP time support.
 */
void EspTime_Init(void);

#endif /* ESP_TIME_H_ */
