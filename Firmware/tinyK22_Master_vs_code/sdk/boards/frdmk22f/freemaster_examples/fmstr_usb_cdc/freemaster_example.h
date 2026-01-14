/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * FreeMASTER Communication Driver - Example Application Declarations
 */

#ifndef __FMSTR_EXAMPLE_H
#define __FMSTR_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Functions definitions
 ******************************************************************************/

// Prototypes of example functions
void FMSTR_Example_Init(void);
void FMSTR_Example_Init_Ex(FMSTR_BOOL callFmstrInit);
void FMSTR_Example_Poll(void);
void FMSTR_Example_Poll_Ex(FMSTR_BOOL callFmstrPoll);

#ifdef __cplusplus
}
#endif

#endif /* __FMSTR_EXAMPLE_H */
