/*
 * Copyright (c) 2020, 2021, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdbool.h>
#include "buttons_config.h"

/*!
 * \brief Decides if a button is pressed
 * \param btn Button to check
 * \return true if button is pressed, false otherwise
 */
bool BTN_IsPressed(BTN_Buttons_e btn);

/*!
 * \brief Module de-initialization
 */
void BTN_Deinit(void);

/*!
 * \brief Module initialization
 */
void BTN_Init(void);

#endif /* BUTTONS_H_ */
