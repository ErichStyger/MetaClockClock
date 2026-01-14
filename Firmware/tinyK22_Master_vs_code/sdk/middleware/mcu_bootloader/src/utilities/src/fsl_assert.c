/*
 * Copyright (c) 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_assert.h"

void fsl_assert(void)
{
    //lock_acquire();
    //while (1);
}
#ifndef NDEBUG
#if (defined(__ARMCC_VERSION)) && (__ARMCC_VERSION >= 6010050)
void __aeabi_assert(const char *failedExpr, const char *file, int line)
{
    
}
#endif /*__ARMCC_VERSION*/
#endif /* NDEBUG */

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
