/*
 * Copyright (c) 2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef LOWPOWERCONFIG_H_
#define LOWPOWERCONFIG_H_

/* supported power saving modes: */
#define LP_MODE_NONE    (0) /* no power saving at all */
#define LP_MODE_WAIT    (1) /* power saving with WFI */
#define LP_MODE_STOP    (2) /* partial stop mode */

#ifndef LP_MODE_SELECTED
  #define LP_MODE_SELECTED   (LP_MODE_NONE)
#endif

#endif /* LOWPOWERCONFIG_H_ */
