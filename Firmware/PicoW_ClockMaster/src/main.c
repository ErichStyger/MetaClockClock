/*
 * Copyright (c) 2023-2025, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "McuLib.h"
#include "McuRTOS.h"
#include "McuWait.h"
#include <stdio.h>
#include "application.h"
#include "McuShell.h"
#include "McuRTT.h"
#include "McuSemihost.h"
#include "McuLog.h"
#if PL_CONFIG_USE_GCOV
  #include "McuCoverage.h"
#endif
#if PL_CONFIG_USE_GPROF
  #include "gprof_support.h"
#endif

int main(void) {
  PL_Init();
  vTaskStartScheduler();
  // GCOVR_EXCL_START
#if PL_CONFIG_USE_GCOV
   McuCoverage_WriteFiles(); /* write coverage data files */
#endif
#if PL_CONFIG_USE_GPROF
  profile_write_data_file();
#endif

#if 0 && McuSemihost_CONFIG_IS_ENABLED
  /* not using any RTOS or queues after scheduler has stopped! */
  McuSemihost_WriteString0("exit with semihosting\n");
  McuWait_Waitms(1); /* give output some time */
  McuSemihost_SysException(McuSemihost_ADP_Stopped_ApplicationExit);
#endif
  McuRTT_WriteString(0, "Stopping JRun with stop command\n");
  McuWait_Waitms(1); /* give output some time */
  McuRTT_WriteString(0, "*STOP*\n"); /* stop JRun */
  for(;;) {
    __asm("nop"); /* do not leave main() */
  }
  // GCOVR_EXCL_STOP
  return 0;
}
