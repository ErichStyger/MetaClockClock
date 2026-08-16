/*!
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * \file
 * \brief Interface for the HTTPD server.
 */

#ifndef __HTTPD_SERVER__H_
#define __HTTPD_SERVER__H_

#ifdef __cplusplus
extern "C" {
#endif

void HttpdServer_TaskSuspend(void);

void HttpdServer_TaskResume(void);

void HttpdServer_Init(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __HTTPD_SERVER__H_ */