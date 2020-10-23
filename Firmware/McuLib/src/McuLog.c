/*
 * Copyright (c) 2017 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/* Original source is from https://github.com/rxi/log.c
 * McuLib integration and extensions: Copyright (c) 2020 Erich Styger
 */

#include "McuLog.h"
#if McuLog_CONFIG_IS_ENABLED

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "McuTimeDate.h"
#include "McuUtility.h"
#include "McuXFormat.h"
#include "McuRTT.h"
#include "McuShell.h"
#if McuLog_CONFIG_USE_MUTEX
  #include "McuRTOS.h"
#endif
#if McuLog_CONFIG_USE_FILE
  #include "McuFatFS.h"
#endif
#if McuLog_CONFIG_USE_RTT_DATA_LOGGER
  #include "McuRTT.h"
#endif

static struct {
  McuLog_Levels_e level; /* 0 (TRACE) to 5 (FATAL) */
#if McuLog_CONFIG_USE_MUTEX
  SemaphoreHandle_t McuLog_Mutex; /* built-in FreeRTOS mutex used for lock below */
#endif
  log_LockFn lock; /* user mutex for synchronization */
  void *udata;  /* optional data for lock */
  McuShell_ConstStdIOType *consoleIo; /* I/O for console logging */
  bool quiet; /* if console logging is silent/quiet */
#if McuLog_CONFIG_USE_COLOR
  bool color; /* if using color for terminal */
#endif
#if McuLog_CONFIG_USE_FILE
  McuFatFS_FIL *fp; /* file handle */
  McuFatFS_FIL logFile; /* FatFS log file descriptor */
#endif
#if McuLog_CONFIG_USE_RTT_DATA_LOGGER
  bool rttDataLogger;
#endif
} L;

static const char *level_names[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#if McuLog_CONFIG_USE_COLOR
static const char *level_colors[] = { /* color codes for messages */
  McuShell_ANSI_COLOR_TEXT_BRIGHT_BLUE,     /* trace */
  McuShell_ANSI_COLOR_TEXT_BRIGHT_GREEN,    /* debug */
  McuShell_ANSI_COLOR_TEXT_BRIGHT_CYAN,     /* info */
  McuShell_ANSI_COLOR_TEXT_BRIGHT_YELLOW,   /* warn */
  McuShell_ANSI_COLOR_TEXT_BRIGHT_RED,      /* error */
  McuShell_ANSI_COLOR_TEXT_BRIGHT_MAGENTA   /* fatal */
};
#endif

static void lock(void)   {
  if (L.lock) {
    L.lock(L.udata, 1);
  }
}

static void unlock(void) {
  if (L.lock) {
    L.lock(L.udata, 0);
  }
}

void McuLog_set_console(McuShell_ConstStdIOType *io) {
  L.consoleIo = io;
}

void McuLog_set_udata(void *udata) {
  L.udata = udata;
}

void McuLog_set_lock(log_LockFn fn) {
  L.lock = fn;
}

#if McuLog_CONFIG_USE_FILE
void McuLog_set_fp(McuFatFS_FIL *fp) {
  L.fp = fp;
}
#endif

#if McuLog_CONFIG_USE_FILE
int McuLog_open_logfile(const unsigned char *logFileName) {
  McuFatFS_FRESULT fres;

  fres = f_open(&L.logFile, (const TCHAR*)logFileName, FA_WRITE|FA_OPEN_APPEND);
  if (fres != FR_OK) {
    McuLog_set_fp(NULL);
    return -1; /* failed */
  }
  McuLog_set_fp(&L.logFile);
  return 0; /* success */
}
#endif

#if McuLog_CONFIG_USE_FILE
int McuLog_close_logfile(void) {
  McuFatFS_FRESULT fres;

  fres = f_close(L.fp);
  McuLog_set_fp(NULL);
  if (fres != FR_OK) {
    return -1; /* failed */
  }
  return 0; /* success */
}
#endif

void McuLog_set_level(McuLog_Levels_e level) {
  L.level = level;
}

void McuLog_set_quiet(bool enable) {
  L.quiet = enable;
}

#if McuLog_CONFIG_USE_COLOR
void McuLog_set_color(bool enable) {
  L.color = enable;
}
#endif

static void OutputCharFctConsole(void *p, char ch) {
  McuShell_StdIO_OutErr_FctType io = (McuShell_StdIO_OutErr_FctType)p;
  io(ch);
}

#if McuLog_CONFIG_USE_FILE
static void OutputCharFctFile(void *p, char ch) {
  McuFatFS_FIL *fp = (McuFatFS_FIL*)p;

  f_putc(ch, fp);
}
#endif

#if McuLog_CONFIG_USE_RTT_DATA_LOGGER
void McuLog_set_rtt_logger(bool enable) {
  L.rttDataLogger = enable;
}
#endif

#if McuLog_CONFIG_USE_RTT_DATA_LOGGER
static void OutputCharRttLoggerFct(void *p, char ch) {
  McuRTT_Write(McuLog_RTT_DATA_LOGGER_CHANNEL, (const char*)&ch, sizeof(char));
}
#endif

static void OutString(const unsigned char *str, void (*outchar)(void *,char), void *p) {
  while(*str!='\0') {
    outchar(p, *str);
    str++;
  }
}

static void LogHeader(DATEREC *date, TIMEREC *time, McuLog_Levels_e level, bool supportColor, const char *file, int line, void (*outchar)(void *,char), void *param) {
  unsigned char buf[32];

  /* date/time */
  buf[0] = '\0';
#if McuLog_CONFIG_LOG_TIMESTAMP_DATE
  McuTimeDate_AddDateString((unsigned char*)buf, sizeof(buf), date, (unsigned char*)McuTimeDate_CONFIG_DEFAULT_DATE_FORMAT_STR);
  McuUtility_chcat(buf, sizeof(buf), ' ');
#endif
  McuTimeDate_AddTimeString((unsigned char*)buf, sizeof(buf), time, (unsigned char*)McuTimeDate_CONFIG_DEFAULT_TIME_FORMAT_STR);
  McuUtility_chcat(buf, sizeof(buf), ' ');
  OutString(buf, outchar, param);

  /* level */
  buf[0] = '\0';
#if McuLog_CONFIG_USE_COLOR
  if (supportColor && L.color) {
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*)level_colors[level]);
  }
#endif
  McuUtility_strcatPad(buf, sizeof(buf), (unsigned char*)level_names[level], ' ', sizeof("DEBUG ")-1);
#if McuLog_CONFIG_USE_COLOR
  if (supportColor && L.color) {
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*)McuShell_ANSI_CONTROL_RESET);
  }
#endif
  OutString(buf, outchar, param);

#if McuLog_CONFIG_LOG_STRIP_FILENAME_PATH
  /* file name */
  const unsigned char *p;

  p = (const unsigned char*)file;
  if (*p=='.') { /* relative */
    while(*p==' ' || *p=='.') {
      p++; /* skip leading spaces or dots */
    }
  } else { /* scan for separator */
    size_t pos;
    pos = McuUtility_strlen(file)-1; /* end of the string */
    while(pos>1 && !(file[pos-1]=='/' || file[pos-1]=='\\')) {
      pos--;
    }
    p = (const unsigned char*)&file[pos];
  }
  OutString(p, outchar, param);
#else
  OutString(file, outchar, param);
#endif

  /* line number */
  buf[0] = '\0';
  McuUtility_chcat(buf, sizeof(buf), ':');
  McuUtility_strcatNum32u(buf,  sizeof(buf), line);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)": ");
  OutString(buf, outchar, param);
}

void McuLog_log(McuLog_Levels_e level, const char *file, int line, const char *fmt, ...) {
  TIMEREC time;
#if McuLog_CONFIG_LOG_TIMESTAMP_DATE
  DATEREC date;
  #define DATE_PTR  &date
#else
  #define DATE_PTR  NULL
#endif
  va_list list;

  if (level < L.level) {
    return;
  }
  lock(); /* Acquire lock */
#if McuLog_CONFIG_LOG_TIMESTAMP_DATE
  (void)McuTimeDate_GetTimeDate(&time, &date); /* Get current date and time */
#else
  (void)McuTimeDate_GetTime(&time); /* Get current time */
#endif
  if (!L.quiet && L.consoleIo!=NULL) { /* log to console */
    LogHeader(DATE_PTR, &time, level, true, file, line, OutputCharFctConsole, L.consoleIo->stdErr);
    /* open argument list */
    va_start(list, fmt);
    McuXFormat_xvformat(OutputCharFctConsole, L.consoleIo->stdErr, fmt, list);
    va_end(list);
    OutString((unsigned char *)"\n", OutputCharFctConsole, L.consoleIo->stdErr);
  }

#if McuLog_CONFIG_USE_RTT_DATA_LOGGER
  /* log to RTT Data Logger */
  if (L.rttDataLogger) {
    LogHeader(DATE_PTR, &time, level, false, file, line, OutputCharRttLoggerFct, NULL);
    /* open argument list */
    va_start(list, fmt);
    McuXFormat_xvformat(OutputCharRttLoggerFct, NULL, fmt, list);
    va_end(list);
    OutString((unsigned char *)"\n", OutputCharRttLoggerFct, NULL);
  }
#endif

#if McuLog_CONFIG_USE_FILE
  /* Log to file */
  if (L.fp) {
    LogHeader(DATE_PTR, &time, level, false, file, line, OutputCharFctFile, L.fp);
    /* open argument list */
    va_start(list, fmt);
    McuXFormat_xvformat(OutputCharFctFile, L.fp, fmt, list);
    va_end(list);
    OutString((unsigned char *)"\n", OutputCharFctFile, L.fp);
    f_sync(L.fp);
  }
#endif
  /* Release lock */
  unlock();
}

#if McuLog_CONFIG_USE_MUTEX
static void LockUnlockCallback(void *data, int lock) {
  if (lock) {
    (void)xSemaphoreTakeRecursive(L.McuLog_Mutex, portMAX_DELAY);
  } else {
    (void)xSemaphoreGiveRecursive(L.McuLog_Mutex);
  }
}
#endif

#if McuLog_CONFIG_PARSE_COMMAND_ENABLED
static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  McuShell_SendStatusStr((unsigned char*)"McuLog", (unsigned char*)"Log status\r\n", io->stdOut);
#if McuLog_CONFIG_USE_FILE
  McuShell_SendStatusStr((unsigned char*)"  file", L.fp!=NULL?(unsigned char*)"yes\r\n":(unsigned char*)"no\r\n", io->stdOut);
#endif
#if McuLog_CONFIG_USE_RTT_DATA_LOGGER
  McuShell_SendStatusStr((unsigned char*)"  rttlogger", L.rttDataLogger?(unsigned char*)"on\r\n":(unsigned char*)"off\r\n", io->stdOut);
#endif
  McuShell_SendStatusStr((unsigned char*)"  lock", L.lock!=NULL?(unsigned char*)"yes\r\n":(unsigned char*)"no\r\n", io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  level", (unsigned char*)level_names[L.level], io->stdOut);
  McuShell_SendStr((unsigned char*)" (", io->stdOut);
  McuShell_SendNum8u(L.level, io->stdOut);
  McuShell_SendStr((unsigned char*)"), logging for this level and higher\r\n", io->stdOut);
#if McuLog_CONFIG_USE_COLOR
  McuShell_SendStatusStr((unsigned char*)"  color", L.color?(unsigned char*)"on\r\n":(unsigned char*)"off\r\n", io->stdOut);
#endif
  return ERR_OK;
}
#endif /* McuLog_CONFIG_PARSE_COMMAND_ENABLED */

#if McuLog_CONFIG_PARSE_COMMAND_ENABLED
static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"McuLog", (unsigned char*)"Group of McuLog commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  level <level>", (unsigned char*)"Set log level, 0 (TRACE) 1 (DEBUG), 2 (INFO),  3 (WARN), 4 (ERROR), 5 (FATAL)\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  quiet <on|off>", (unsigned char*)"Set quiet mode for console\r\n", io->stdOut);
#if McuLog_CONFIG_USE_COLOR
  McuShell_SendHelpStr((unsigned char*)"  color <on|off>", (unsigned char*)"Set color mode\r\n", io->stdOut);
#endif
#if McuLog_CONFIG_USE_RTT_DATA_LOGGER
  McuShell_SendHelpStr((unsigned char*)"  rttlogger <on|off>", (unsigned char*)"Set rtt data logger mode\r\n", io->stdOut);
#endif
  return ERR_OK;
}
#endif /* McuLog_CONFIG_PARSE_COMMAND_ENABLED */

#if McuLog_CONFIG_PARSE_COMMAND_ENABLED
uint8_t McuLog_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  uint8_t res = ERR_OK;
  const unsigned char *p;

  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP) == 0
    || McuUtility_strcmp((char*)cmd, "McuLog help") == 0)
  {
    *handled = TRUE;
    return PrintHelp(io);
  } else if (   (McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0)
             || (McuUtility_strcmp((char*)cmd, "McuLog status")==0)
            )
  {
    *handled = TRUE;
    res = PrintStatus(io);
  } else if (McuUtility_strcmp((char*)cmd, "McuLog quiet on")==0) {
    *handled = TRUE;
    McuLog_set_quiet(true);
  } else if (McuUtility_strcmp((char*)cmd, "McuLog quiet off")==0) {
    *handled = TRUE;
    McuLog_set_quiet(false);
#if McuLog_CONFIG_USE_COLOR
  } else if (McuUtility_strcmp((char*)cmd, "McuLog color on")==0) {
    *handled = TRUE;
    McuLog_set_color(true);
  } else if (McuUtility_strcmp((char*)cmd, "McuLog color off")==0) {
    *handled = TRUE;
    McuLog_set_color(false);
#endif
    } else if (McuUtility_strncmp((char*)cmd, "McuLog level ", sizeof("McuLog level ")-1)==0) {
    uint8_t level;

    *handled = TRUE;
    p = cmd+sizeof("McuLog level ")-1;
    if (McuUtility_ScanDecimal8uNumber(&p, &level)==ERR_OK && level<=McuLog_FATAL) {
      McuLog_set_level(level);
    }
  }
  return res;
}
#endif /* McuLog_CONFIG_PARSE_COMMAND_ENABLED */


#if McuLog_CONFIG_USE_RTT_DATA_LOGGER
static char McuLog_RttUpBuffer[McuLog_CONFIG_RTT_DATA_LOGGER_BUFFER_SIZE];
#endif

void McuLog_Init(void) {
#if McuLog_CONFIG_USE_MUTEX
  L.McuLog_Mutex = xSemaphoreCreateRecursiveMutex();
  if (L.McuLog_Mutex==NULL) { /* semaphore creation failed */
    for(;;) {} /* error, not enough memory? */
  }
  vQueueAddToRegistry(L.McuLog_Mutex, "McuLog_Mutex");
  McuLog_set_lock(LockUnlockCallback);
#endif
#if McuLog_CONFIG_USE_COLOR
  McuLog_set_color(true);
#endif
#if McuLog_CONFIG_USE_RTT_DATA_LOGGER
  McuLog_set_rtt_logger(true);
#endif
  L.consoleIo = McuShell_GetStdio();
#if McuLog_CONFIG_USE_RTT_DATA_LOGGER
  #if McuLib_CONFIG_SDK_USE_FREERTOS && configUSE_SEGGER_SYSTEM_VIEWER_HOOKS && McuSystemView_CONFIG_RTT_CHANNEL==McuLog_RTT_DATA_LOGGER_CHANNEL
    #error "Both RTT Logger and SystemViewer are using the same channel! Change McuSystemView_CONFIG_RTT_CHANNEL to a different value."
  #endif
  SEGGER_RTT_ConfigUpBuffer(McuLog_RTT_DATA_LOGGER_CHANNEL, "Logger", &McuLog_RttUpBuffer[0], sizeof(McuLog_RttUpBuffer), McuLog_CONFIG_RTT_DATA_LOGGER_CHANNEL_MODE);
#endif
}

void McuLog_Deinit(void) {
#if McuLog_CONFIG_USE_MUTEX
  vSemaphoreDelete(L.McuLog_Mutex);
  L.McuLog_Mutex = NULL;
  McuLog_set_lock(NULL);
#endif
}

#endif /* McuLog_CONFIG_IS_ENABLED */

