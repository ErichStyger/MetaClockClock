/*
 * Copyright (c) 2019, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_RS485
#include "rs485.h"
#include "McuGPIO.h"
#include "rs485Uart.h"
#include "McuShell.h"
#include "McuRTOS.h"
#include "McuUtility.h"
#include "McuLog.h"
#include "Shell.h"
#include "nvmc.h"
#if PL_CONFIG_IS_CLIENT && PL_CONFIG_USE_STEPPER
  #include "StepperBoard.h"
#endif
#if PL_CONFIG_USE_WDT
  #include "watchdog.h"
#endif

typedef enum RS485_Response_e {
  RS485_RESPONSE_CONTINUE, /* continue scanning and parsing */
  RS485_RESPONSE_OK, /* ok response */
  RS485_RESPONSE_NOK, /* not ok response */
  RS485_RESPONSE_TIMEOUT, /* timeout */
} RS485_Response_e;

static bool RS485_DoLogging = false; /* if traffic on the bus shall be reported on the shell */

uint8_t RS485_GetAddress(void) {
  return NVMC_GetRS485Addr();
}

static void RS485_SendChar(unsigned char ch) {
  RS485Uart_stdio.stdOut(ch);
}

static void RS485_NullSend(unsigned char ch) {
  /* do nothing */
}

static void RS485_ReadChar(uint8_t *c) {
  *c = '\0'; /* only sending on this channel */
}

static bool RS485_CharPresent(void) {
  return false; /* only sending on this channel */
}

McuShell_ConstStdIOType RS485_stdio = {
    (McuShell_StdIO_In_FctType)RS485_ReadChar, /* stdin */
    (McuShell_StdIO_OutErr_FctType)RS485_SendChar,  /* stdout */
    (McuShell_StdIO_OutErr_FctType)RS485_SendChar,  /* stderr */
    RS485_CharPresent /* if input is not empty */
  };

McuShell_ConstStdIOType RS485_stdioBroadcast = {
    (McuShell_StdIO_In_FctType)RS485_ReadChar, /* stdin */
    (McuShell_StdIO_OutErr_FctType)RS485_NullSend,  /* stdout */
    (McuShell_StdIO_OutErr_FctType)RS485_NullSend,  /* stderr */
    RS485_CharPresent /* if input is not empty */
  };


static void RS485_SendStr(unsigned char *str) {
  while(*str!='\0') {
    RS485_stdio.stdOut(*str++);
  }
}

static uint8_t CalcCRC(const uint8_t *data, uint8_t dataSize, uint8_t start) {
  uint8_t crc, i, x, y;

  crc = start;
  for(x=0;x<dataSize;x++){
    y = data[x];
    for(i=0;i<8;i++) { /* go through all bits of the data byte */
      if((crc&0x01)^(y&0x01)) {
        crc >>= 1;
        crc ^= 0x8c;
      } else {
        crc >>= 1;
      }
      y >>= 1;
    }
  }
  return crc;
}

static uint8_t CalcMsgCrc(const unsigned char *msg) {
  /* header is "@dd ss cc ...", both dd, ss and cc are 8bit HEX values. The CRC (cc) is not included in the CRC */
  uint8_t crc;

  crc = CalcCRC(msg, sizeof("@dd ss ")-1, 0);
  crc = CalcCRC(msg+sizeof("@dd ss cc")-1, strlen((char*)msg+sizeof("@dd ss cc")-1), crc);
  return crc;
}

typedef enum CMD_ParserState_e {
  CMD_PARSER_INIT,
  CMD_PARSER_START_DETECTED, /* start '@' detected */
  CMD_PARSER_SCAN_DST_ADDR, /* scanning destination address */
  CMD_PARSER_SCAN_SRC_ADDR,  /* scan source address */
  CMD_PARSER_SCAN_CRC,       /* scan CRC */
  CMD_PARSER_SCAN_OK_NOK,   /* reading NOK or OK */
} CMD_ParserState_e;

static RS485_Response_e Scan(CMD_ParserState_e *state, unsigned char ch, unsigned char *buf, size_t bufSize, uint8_t fromAddr) {
  /* scan for "@<dstaddr> <srcAddr> <CRC> OK"
   *       or "@<dstaddr> <srcAddr> <CRC> NOK"
   */
  const unsigned char *p;
  uint8_t addr;
  uint8_t exp_crc;
  static uint8_t crc = 0;

  if (ch=='@') { /* a marker? start scanning again */
    *state = CMD_PARSER_INIT;
  }
  for(;;) { /* breaks or returns */
    switch(*state) {
      case CMD_PARSER_INIT:
        buf[0] = '\0'; /* reset buffer */
        if (ch=='@') { /* a marker? start scanning again */
          McuUtility_chcat(buf, bufSize, ch);
          *state = CMD_PARSER_START_DETECTED;
          break; /* continue state machine */
        }
        return RS485_RESPONSE_CONTINUE;

       case CMD_PARSER_START_DETECTED:
        *state = CMD_PARSER_SCAN_DST_ADDR;
        return RS485_RESPONSE_CONTINUE;

       case CMD_PARSER_SCAN_DST_ADDR:
         McuUtility_chcat(buf, bufSize, ch);
         if (ch==' ') {
           p = &buf[sizeof("@")-1];
           if (McuUtility_ScanHex8uNumberNoPrefix(&p, &addr)==ERR_OK && addr==RS485_GetAddress()) {
             *state = CMD_PARSER_SCAN_SRC_ADDR;
             return RS485_RESPONSE_CONTINUE;
           } else {
             *state = CMD_PARSER_INIT;
           }
         } else if (ch=='\n') { /* failed */
           *state = CMD_PARSER_INIT;
         }
         return RS485_RESPONSE_CONTINUE;

       case CMD_PARSER_SCAN_SRC_ADDR:
         McuUtility_chcat(buf, bufSize, ch);
         if (ch==' ') {
           p = &buf[sizeof("@ss ")-1];
           if (McuUtility_ScanHex8uNumberNoPrefix(&p, &addr)==ERR_OK && addr==fromAddr) {
             *state = CMD_PARSER_SCAN_CRC;
             return RS485_RESPONSE_CONTINUE;
           } else {
             *state = CMD_PARSER_INIT;
           }
         } else if (ch=='\n') { /* failed */
           *state = CMD_PARSER_INIT;
         }
         return RS485_RESPONSE_CONTINUE;

       case CMD_PARSER_SCAN_CRC:
         McuUtility_chcat(buf, bufSize, ch);
         if (ch==' ') {
           p = &buf[sizeof("@ss dd ")-1];
           if (McuUtility_ScanHex8uNumberNoPrefix(&p, &crc)==ERR_OK) {
             *state = CMD_PARSER_SCAN_OK_NOK;
             return RS485_RESPONSE_CONTINUE;
           } else {
             *state = CMD_PARSER_INIT;
           }
         } else if (ch=='\n') { /* failed */
           *state = CMD_PARSER_INIT;
         }
         return RS485_RESPONSE_CONTINUE;

       case CMD_PARSER_SCAN_OK_NOK:
          McuUtility_chcat(buf, bufSize, ch);
          if (ch=='\n') {
            p = &buf[sizeof("@ss dd cc ")-1];
            if (McuUtility_strcmp((char*)p, (char*)"OK\n")==0) { /* a match! */
              buf[sizeof("@ss dd cc OK")-1] = '\0'; /* overwrite '\n' as not included in CRC */
              exp_crc = CalcMsgCrc(buf);
              *state = CMD_PARSER_INIT;
              if (crc==exp_crc) {
                return RS485_RESPONSE_OK;
              } else {
                return RS485_RESPONSE_NOK;
              }
            } else if (McuUtility_strcmp((char*)buf, (char*)"NOK\n")==0) { /* a match! */
              buf[sizeof("@ss dd cc NOK")-1] = '\0'; /* overwrite '\n' as not included in CRC */
              exp_crc = CalcMsgCrc(buf);
              *state = CMD_PARSER_INIT;
              if (crc==exp_crc) {
                return RS485_RESPONSE_NOK;
              } else {
                return RS485_RESPONSE_NOK; /* CRC is not ok, and message is NOK too */
              }
            } else if (ch=='\n') { /* failed */
              *state = CMD_PARSER_INIT;
              break; /* continue in state machine */
            }
          }
          return RS485_RESPONSE_CONTINUE;

      default:
        break;
    } /* switch */
    /* get here with a break */
  } /* for */
  return RS485_RESPONSE_CONTINUE;
}

static RS485_Response_e WaitForResponse(int32_t timeoutMs, uint8_t fromAddr) {
  unsigned char buf[24] = ""; /* "@<addr> <fromAddr> OK" or "@<addr> <fromAddr> NOK" */
  unsigned char ch;
  RS485_Response_e resp;
  CMD_ParserState_e state = CMD_PARSER_INIT;

  for(;;) { /* returns */
    ch = RS458Uart_GetResponseQueueChar();
    if (ch!='\0') {
      resp = Scan(&state, ch, buf, sizeof(buf), fromAddr);
      if (resp==RS485_RESPONSE_OK || resp==RS485_RESPONSE_NOK) {
        return resp;
      }
    } else { /* empty input buffer: wait */
      vTaskDelay(pdMS_TO_TICKS(10));
    #if PL_CONFIG_USE_WDT
      WDT_Report(WDT_REPORT_ID_CURR_TASK, 10);
    #endif
      timeoutMs -= 10;
      if (timeoutMs<=0) {
        return RS485_RESPONSE_TIMEOUT;
      }
    }
  } /* for */
  return RS485_RESPONSE_CONTINUE;
}

uint8_t RS485_SendCommand(uint8_t dstAddr, const unsigned char *cmd, int32_t timeoutMs, bool intern, uint32_t nofRetry) {
  /* example: send "@16 1 cmd stepper status" */
  unsigned char buf[McuShell_DEFAULT_SHELL_BUFFER_SIZE];
  uint8_t res = ERR_OK;
  RS485_Response_e resp;
  uint8_t crc, hex;

#if PL_CONFIG_USE_NEO_PIXEL_HW
  if (intern && (dstAddr==RS485_GetAddress() || dstAddr==RS485_BROADCAST_ADDRESS)) {
    SHELL_ParseCommand(cmd, NULL, true); /* parse it for the LED rings */
    if (dstAddr!=RS485_BROADCAST_ADDRESS) { /* only for us */
      return ERR_OK;
    }
  }
#endif
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)("@"));
  McuUtility_strcatNum8Hex(buf, sizeof(buf), dstAddr); /* add destination address */
  McuUtility_chcat(buf, sizeof(buf), ' ');
  McuUtility_strcatNum8Hex(buf, sizeof(buf), RS485_GetAddress()); /* add src address */
  McuUtility_chcat(buf, sizeof(buf), ' ');
  McuUtility_strcatNum8Hex(buf, sizeof(buf), 0); /* dummy crc, will be replaced with real one */
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)(" cmd "));
  McuUtility_strcat(buf, sizeof(buf), cmd);
  /* update crc */
  crc = CalcMsgCrc(buf);
  hex = (char)((crc>>4) & 0x0F);
  buf[sizeof("@dd ss ")-1] = (char)(hex + ((hex <= 9) ? '0' : ('A'-10)));
  hex = (char)(crc & 0x0F);
  buf[sizeof("@dd ss c")-1] = (char)(hex + ((hex <= 9) ? '0' : ('A'-10)));

  for(;;) { /* breaks */
    RS458Uart_ClearResponseQueue(); /* clear up if there is something pending */
    if (RS485_DoLogging) {
      McuLog_trace("Tx: %s", buf);
    }
    RS485_SendStr(buf);
    RS485_SendStr((unsigned char*)"\n");
    if (dstAddr==RS485_BROADCAST_ADDRESS) {
      /* do not wait for a OK/NOK response for broadcast messages. The caller has to check with 'lastError' */
      res = ERR_OK;
      break; /* leave loop */
    } else {
      vTaskDelay(pdMS_TO_TICKS(10)); /* give back some time for a response */
      resp = WaitForResponse(500, dstAddr);
      if (resp==RS485_RESPONSE_OK) {
        res = ERR_OK;
        break; /* fine, leave loop */
      } else if (resp==RS485_RESPONSE_TIMEOUT) { /* board did not respond? */
        res = ERR_BUSOFF; /* retry */
      } else if (resp==RS485_RESPONSE_NOK) { /* not ok, crc error? */
        res = ERR_CRC; /* retry */
      }
    }
    /* NOK or timeout */
    if (nofRetry==0) { /* tried enough */
      res = ERR_FAILED;
      break; /* leave loop */
    }
    nofRetry--; /* try again */
  } /* for */
  return res;
}

static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  uint8_t buf[16];

  McuShell_SendStatusStr((unsigned char*)"rs", (unsigned char*)"RS-485 settings\r\n", io->stdOut);
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"0x");
  McuUtility_strcatNum8Hex(buf, sizeof(buf), RS485_GetAddress());
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  addr", buf, io->stdOut);
  McuShell_SendStatusStr((unsigned char*)"  log", RS485_DoLogging?(unsigned char*)"on\r\n":(unsigned char*)"off\r\n", io->stdOut);
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"rs", (unsigned char*)"Group of RS-458 commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
#if PL_CONFIG_USE_NVMC
  McuShell_SendHelpStr((unsigned char*)"  addr <addr>", (unsigned char*)"Set RS-485 address\r\n", io->stdOut);
#endif
  McuShell_SendHelpStr((unsigned char*)"  send <text>", (unsigned char*)"Send a text to the RS-485 bus\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  sendcmd <addr> <cmd>", (unsigned char*)"Send a shell command to the RS-485 address and check response\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  log on|off", (unsigned char*)"Log RS-485 bus activity to McuLog\r\n", io->stdOut);
  return ERR_OK;
}

uint8_t RS485_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  const unsigned char *p;
  int32_t val;

  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "rs help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "rs status")==0)) {
    *handled = TRUE;
    return PrintStatus(io);
  } else if (McuUtility_strncmp((char*)cmd, "rs log ", sizeof("rs log ")-1)==0) {
    *handled = TRUE;
    p = cmd + sizeof("rs log ")-1;
    if (McuUtility_strcmp((char*)p, "on")==0) {
      RS485_DoLogging = true;
      return ERR_OK;
    } else if (McuUtility_strcmp((char*)p, "off")==0) {
      RS485_DoLogging = false;
      return ERR_OK;
    }
    return ERR_FAILED;
#if PL_CONFIG_USE_NVMC
  } else if (McuUtility_strncmp((char*)cmd, "rs addr ", sizeof("rs addr ")-1)==0) {
    NVMC_Data_t data;
    *handled = true;
    if (NVMC_IsErased()) {
      McuShell_SendStr((unsigned char*)"FLASH is erased, initialize it first!\r\n", io->stdErr);
      return ERR_FAILED;
    }
    p = cmd + sizeof("rs addr ")-1;
    if (McuUtility_xatoi(&p, &val)==ERR_OK && val>=0 && val<=0xff) {
      data = *NVMC_GetDataPtr(); /* struct copy */
      data.addrRS485 = val;
      if (NVMC_WriteConfig(&data)!=ERR_OK) {
        McuShell_SendStr((unsigned char*)"Failed writing configuration!\r\n", io->stdErr);
        return ERR_FAILED;
      }
      return ERR_OK;
    }
    return ERR_FAILED;
#endif
  } else if (McuUtility_strncmp((char*)cmd, "rs send ", sizeof("rs send ")-1)==0) {
    *handled = true;
    RS485_SendStr((unsigned char*)cmd+sizeof("rs send ")-1);
    RS485_SendStr((unsigned char*)("\n"));
  } else if (McuUtility_strncmp((char*)cmd, "rs sendcmd ", sizeof("rs sendcmd ")-1)==0) {
    *handled = true;
    p = cmd + sizeof("rs sendcmd ")-1;
    if (McuUtility_xatoi(&p, &val)==ERR_OK) { /* parse destination address */
      while (*p==' ') { /* skip leading spaces */
        p++;
      }
      return RS485_SendCommand(val, (unsigned char*)p, 10000, true, 0); /* 10 seconds should be enough */
    }
    return ERR_FAILED;
  }
  return ERR_OK;
}

//#if PL_CONFIG_IS_CLIENT
static uint8_t CheckHeader(unsigned char *msg, const unsigned char **startCmd, uint8_t *sourceAddr, uint8_t *destinationAddr) {
  /* format is in the form "@<DST_ADDR> <SRC_ADDR> <CRC> cmd help" */
  const unsigned char *p;
  uint8_t dstAddr, srcAddr;
  uint8_t expected_crc, crc, res;
  unsigned char buf[42];

  /* init with defaults in case of error */
  *sourceAddr = RS485_BROADCAST_ADDRESS;
  *destinationAddr = RS485_BROADCAST_ADDRESS;
  *startCmd = msg;
  if (*msg=='@') {
    p = msg+1; /* skip '@' */
    /* check for "@<DST_ADDR> <SRC_ADDR>" */
    if (   McuUtility_ScanHex8uNumberNoPrefix(&p, &dstAddr)==ERR_OK
        && (dstAddr==RS485_GetAddress() || dstAddr==RS485_BROADCAST_ADDRESS) /* broadcast or matching destination address */
        && McuUtility_ScanHex8uNumberNoPrefix(&p, &srcAddr)==ERR_OK /* get source address */
        )
    {
      *sourceAddr = srcAddr;
      *destinationAddr = dstAddr;
      /* check CRC */
      res = McuUtility_ScanHex8uNumberNoPrefix(&p, &crc);
      if (res!=ERR_OK) {
        return ERR_CRC;
      }
      expected_crc = CalcMsgCrc(msg);
      if (crc!=expected_crc) {
        if (dstAddr!=RS485_BROADCAST_ADDRESS) { /* only send back error if it was not a broadcast */
          McuUtility_strcpy(buf, sizeof(buf), (uint8_t*)"CRC_ERR 0x");
          McuUtility_strcatNum8Hex(buf, sizeof(buf), RS485_GetAddress());
          McuUtility_strcat(buf, sizeof(buf), (uint8_t*)": 0x");
          McuUtility_strcatNum8Hex(buf, sizeof(buf), crc);
          McuUtility_strcat(buf, sizeof(buf), (uint8_t*)" expected 0x");
          McuUtility_strcatNum8Hex(buf, sizeof(buf), expected_crc);
          McuUtility_chcat(buf, sizeof(buf), '\n');
          McuShell_SendStr(buf, RS485_stdio.stdErr);
        }
        return ERR_CRC;
      }
      *startCmd = p;
      return ERR_OK;
    }
  }
  return ERR_FAILED;
}
//#endif

static void RS485Task(void *pv) {
  static uint8_t cmdBuf[McuShell_DEFAULT_SHELL_BUFFER_SIZE]; /* command line from the RS-485 bus */
//#if PL_CONFIG_IS_CLIENT
  const unsigned char *startCmd;
  uint8_t srcAddr, dstAddr;
  uint8_t res, crc;
  uint8_t buf[32];
  unsigned char hex;
  bool reply;
  static uint8_t lastError = ERR_OK;
//#endif

  (void)pv; /* not used */
  McuLog_trace("Starting RS485 Task");
  #if PL_CONFIG_USE_WDT
  WDT_SetTaskHandle(WDT_REPORT_ID_TASK_RS485, xTaskGetCurrentTaskHandle());
#endif
  cmdBuf[0] = '\0';
  for(;;) {
    if (McuShell_ReadCommandLine(cmdBuf, sizeof(cmdBuf), &RS485Uart_stdio)==ERR_OK) {
      reply = false;
      srcAddr = RS485_ILLEGAL_ADDRESS;
      dstAddr = RS485_ILLEGAL_ADDRESS;
      if (cmdBuf[0]=='@' && strlen((char*)cmdBuf)>sizeof("@dd ss cc ")-1) { /* have a valid message? */
        if (RS485_DoLogging) {
          McuLog_trace("Rx: %s", cmdBuf);
        }
        reply = false; /* default */
        res = CheckHeader(cmdBuf, &startCmd, &srcAddr, &dstAddr);
        if (res == ERR_CRC) { /* wrong crc */
          lastError = ERR_CRC;
          reply = true;
        } else if (res==ERR_OK) { /* header was ok */
          res = ERR_FAILED; /* set error case */
          if (McuUtility_strcmp((char*)startCmd, (char*)" cmd lastError")==0) {
            reply = true;
            res = lastError;  /* report back last error */
            lastError = ERR_OK; /* clear error */
        #if PL_CONFIG_IS_CLIENT && PL_CONFIG_USE_STEPPER
          } else if (McuUtility_strcmp((char*)startCmd, (char*)" cmd idle")==0) {
            reply = true;
            if (!STEPBOARD_ItemsInQueue(STEPBOARD_GetBoard()) && STEPBOARD_IsIdle(STEPBOARD_GetBoard())) {
              res = ERR_OK;  /* ERR_OK if board is idle */
            } else {
              res = ERR_FAILED; /* not idle */
            }
        #endif
          } else if (McuUtility_strncmp((char*)startCmd, " cmd ", sizeof(" cmd ")-1)==0) { /* shell command? */
            startCmd += sizeof(" cmd ")-1;
            if (dstAddr==RS485_BROADCAST_ADDRESS) {
              res = SHELL_ParseCommand(startCmd, &RS485_stdioBroadcast, true); /* do not write anything back if broadcast */
            } else {
              res = SHELL_ParseCommand(startCmd, &RS485_stdio, true);
            }
            lastError = res; /* remember error status if we get asked later on */
            reply = true;
          } else if (McuUtility_strcmp((char*)startCmd, (char*)" OK")==0) {
            reply = false;
          } else if (McuUtility_strcmp((char*)startCmd, (char*)" NOK")==0) {
            reply = false;
          }
        }
      } else {
        /* not starting with '@', print it ... */
        SHELL_SendString((unsigned char *)cmdBuf);
        SHELL_SendString((unsigned char*)"\r\n");
      }
      cmdBuf[0] = '\0'; /* reset buffer for next iteration */
      /* send response back to sender */
      if (reply && dstAddr!=RS485_BROADCAST_ADDRESS) { /* normal message, send response. For broadcasts it is up to the caller to check the last error */
        McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"@");
        McuUtility_strcatNum8Hex(buf, sizeof(buf), srcAddr);
        McuUtility_chcat(buf, sizeof(buf), ' ');
        McuUtility_strcatNum8Hex(buf, sizeof(buf), RS485_GetAddress());
        McuUtility_chcat(buf, sizeof(buf), ' ');
        McuUtility_strcatNum8Hex(buf, sizeof(buf), 0); /* dummy crc, will be replaced later */
        if (res==ERR_OK) {
          McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" OK");
        } else {
          McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" NOK");
        }
        crc = CalcMsgCrc(buf);
        hex = (char)((crc>>4) & 0x0F);
        buf[sizeof("@dd ss ")-1] = (char)(hex + ((hex <= 9) ? '0' : ('A'-10)));
        hex = (char)(crc & 0x0F);
        buf[sizeof("@dd ss c")-1] = (char)(hex + ((hex <= 9) ? '0' : ('A'-10)));
        McuUtility_chcat(buf, sizeof(buf), '\n');
        RS485_SendStr(buf);
      }
    }
    if (!RS485Uart_stdio.keyPressed()) { /* if nothing in input queue, give back some CPU time */
      vTaskDelay(pdMS_TO_TICKS(10));
    #if PL_CONFIG_USE_WDT
      WDT_Report(WDT_REPORT_ID_TASK_RS485, 10);
    #endif
    }
  } /* for */
}

void RS485_Deinit(void) {
  RS485Uart_Deinit();
}

void RS485_Init(void) {
  RS485Uart_Init();
  if (xTaskCreate(
      RS485Task,  /* pointer to the task */
      "RS-485", /* task name for kernel awareness debugging */
      1300/sizeof(StackType_t), /* task stack size */
      (void*)NULL, /* optional task startup argument */
      tskIDLE_PRIORITY+4,  /* initial priority */
      (TaskHandle_t*)NULL /* optional task handle to create */
    ) != pdPASS)
  {
    for(;;){} /* error! probably out of memory */
  }
}

#endif /* PL_CONFIG_USE_RS485 */
