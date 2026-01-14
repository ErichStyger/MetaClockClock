/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PD_APP_MISC_H__
#define __PD_APP_MISC_H__

extern void PD_BMEnterCritical(uint32_t *sr);
extern void PD_BMExitCritical(uint32_t sr);

#define APP_CRITICAL_ALLOC() uint32_t usbOsaCurrentSr = 0U;
#define APP_ENTER_CRITICAL() PD_BMEnterCritical(&usbOsaCurrentSr)
#define APP_EXIT_CRITICAL() PD_BMExitCritical(usbOsaCurrentSr)

#endif
