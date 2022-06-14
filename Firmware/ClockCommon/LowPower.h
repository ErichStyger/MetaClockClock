/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LOWPOWER_H_
#define LOWPOWER_H_


/* supported power saving modes: */
#define LP_MODE_NONE    (0) /* no power saving at all */
#define LP_MODE_WAIT    (1) /* power saving with WFI */
#define LP_MODE_STOP    (2) /* partial stop mode */

#define LP_MODE_SELECTED   (LP_MODE_NONE)

void LP_EnterWaitMode(void);
void LP_EnterStopMode(void);

void LP_Init(void);

#endif /* LOWPOWER_H_ */
