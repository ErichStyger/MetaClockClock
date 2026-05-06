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
 * \brief Checks whether the selected button is currently pressed.
 * \param btn Button to query.
 * \return True if the condition or operation succeeds, false otherwise.
 */
bool BTN_IsPressed(BTN_Buttons_e btn);

/*!
 * \brief Deinitializes the button module.
 */
void BTN_Deinit(void);

/*!
 * \brief Initializes the button module.
 */
void BTN_Init(void);

#endif /* BUTTONS_H_ */
