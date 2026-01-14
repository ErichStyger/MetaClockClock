/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * FreeMASTER Example Application - Demo code configuration file
 */

#ifndef __FREEMASTER_DEMO_H
#define __FREEMASTER_DEMO_H

/* These options are used in freemaster_example.c to configure features of FreeMASTER example application */

#define FMSTR_DEMO_ENOUGH_ROM 1 /* Platform has enough ROM to show most of the FreeMASTER features */
#define FMSTR_DEMO_LARGE_ROM \
    1 /* Platform has large ROM enough to store the extended data structures used in FreeMASTER demo */
#define FMSTR_DEMO_SUPPORT_I64 1 /* Support for long long type */
#define FMSTR_DEMO_SUPPORT_FLT 1 /* Support for float type */
#define FMSTR_DEMO_SUPPORT_DBL 1 /* Support for double type */

#endif /* __FREEMASTER_DEMO_H */
