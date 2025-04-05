/*
 * Copyright (c) 2021, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MAIN_SNTP_TIME_H_
#define MAIN_SNTP_TIME_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Get the time from the sntp server
 * \return 0 if time has been obtained, negative value in case of error.
 */
int SNTP_ObtainTime(void);

/*! \brief module initialization */
void SNTP_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* MAIN_SNTP_TIME_H_ */
