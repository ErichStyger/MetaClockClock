/*!
 * Copyright (c) 2026, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * \file
 * \brief Implemenation of the the HTTPD server.
 */

#include "platform.h"
#if PL_CONFIG_USE_HTTPD_SERVER
#include "pico/cyw43_arch.h"
#include "httpdServer.h"
#include "McuRTOS.h"
#include "McuLog.h"
#include "shell.h"
#include "lwip/ip4_addr.h"
#include "lwip/apps/mdns.h"
#include "lwip/init.h"
#include "lwip/apps/httpd.h"

static TaskHandle_t httpTaskHandle = NULL;

void HttpdServer_TaskSuspend(void) {
  if (httpTaskHandle!=NULL) {
    vTaskSuspend(httpTaskHandle);
  }
};

void HttpdServer_TaskResume(void) {
  if (httpTaskHandle!=NULL) {
    vTaskResume(httpTaskHandle);
  }
}

void httpd_init(void);

static absolute_time_t wifi_connected_time;
static bool led_on = false;
#define HTTP_SHELL_LAST_CMD_SIZE      96
#define HTTP_SHELL_LAST_RESULT_SIZE   1024
#define HTTP_SHELL_HTML_EXPANSION_MAX 6 /* worst-case (&quot;) */
static char httpShellLastCmd[HTTP_SHELL_LAST_CMD_SIZE*HTTP_SHELL_HTML_EXPANSION_MAX] = "";
static char httpShellLastResult[HTTP_SHELL_LAST_RESULT_SIZE] = "No command executed yet.";
static char httpShellLastResultHtml[HTTP_SHELL_LAST_RESULT_SIZE*HTTP_SHELL_HTML_EXPANSION_MAX] = "No command executed yet.";
static bool httpShellLastSuccess = true;

#if LWIP_MDNS_RESPONDER
static void srv_txt(struct mdns_service *service, void *txt_userdata)
{
  err_t res;
  LWIP_UNUSED_ARG(txt_userdata);

  res = mdns_resp_add_service_txtitem(service, "path=/", 6);
  LWIP_ERROR("mdns add service txt failed\n", (res == ERR_OK), return);
}
#endif

// Return some characters from the ascii representation of the mac address
// e.g. 112233445566
// chr_off is index of character in mac to start
// chr_len is length of result
// chr_off=8 and chr_len=4 would return "5566"
// Return number of characters put into destination
static size_t get_mac_ascii(int idx, size_t chr_off, size_t chr_len, char *dest_in) {
  static const char hexchr[16] = "0123456789ABCDEF";
  uint8_t mac[6];
  char *dest = dest_in;
  assert(chr_off + chr_len <= (2 * sizeof(mac)));
  cyw43_hal_get_mac(idx, mac);
  for (; chr_len && (chr_off >> 1) < sizeof(mac); ++chr_off, --chr_len) {
    *dest++ = hexchr[mac[chr_off >> 1] >> (4 * (1 - (chr_off & 1))) & 0xf];
  }
  return dest - dest_in;
}

static const char *cgi_handler_test(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
  if (iNumParams > 0) {
    if (strcmp(pcParam[0], "test") == 0) {
      return "/test.shtml";
    }
  }
  return "/index.shtml";
}

static tCGI cgi_handlers[] = {
  { "/", cgi_handler_test },
  { "/index.shtml", cgi_handler_test },
};

// Note that the buffer size is limited by LWIP_HTTPD_MAX_TAG_INSERT_LEN, so use LWIP_HTTPD_SSI_MULTIPART to return larger amounts of data
u16_t ssi_example_ssi_handler(int iIndex, char *pcInsert, int iInsertLen
#if LWIP_HTTPD_SSI_MULTIPART
  , uint16_t current_tag_part, uint16_t *next_tag_part
#endif
) {
  size_t printed;
  switch (iIndex) {
    case 0: { // "status"
        printed = snprintf(pcInsert, iInsertLen, "Pass");
        break;
    }
    case 1: { // "welcome"
        printed = snprintf(pcInsert, iInsertLen, "Hello from Pico V2");
        break;
    }
    case 2: { // "uptime"
        uint64_t uptime_s = absolute_time_diff_us(wifi_connected_time, get_absolute_time()) / 1e6;
        printed = snprintf(pcInsert, iInsertLen, "%"PRIu64, uptime_s);
        break;
    }
    case 3: { // "ledstate"
        printed = snprintf(pcInsert, iInsertLen, "%s", led_on ? "ON" : "OFF");
        break;
    }
    case 4: { // "ledinv"
        printed = snprintf(pcInsert, iInsertLen, "%s", led_on ? "OFF" : "ON");
        break;
    }
    case 5: { // "shellcmd"
        printed = snprintf(pcInsert, iInsertLen, "%s", httpShellLastCmd);
        break;
    }
    case 6: { // "shstat"
        printed = snprintf(pcInsert, iInsertLen, "%s", httpShellLastSuccess ? "OK" : "ERROR");
        break;
    }
    case 7: { // "shresult"
#if LWIP_HTTPD_SSI_MULTIPART
        size_t textLen = strlen(httpShellLastResultHtml);
        size_t maxChunkLen = iInsertLen>0 ? (size_t)(iInsertLen-1) : 0; /* keep room for terminator */
        size_t offset = (size_t)current_tag_part * maxChunkLen;

        if (maxChunkLen==0 || offset>=textLen) {
          printed = 0;
        } else {
          size_t copyLen = textLen-offset;
          if (copyLen>maxChunkLen) {
            copyLen = maxChunkLen;
            *next_tag_part = current_tag_part + 1;
          }
          memcpy(pcInsert, &httpShellLastResultHtml[offset], copyLen);
          pcInsert[copyLen] = '\0';
          printed = copyLen;
        }
#else
        printed = snprintf(pcInsert, iInsertLen, "%s", httpShellLastResultHtml);
#endif
        break;
    }
#if LWIP_HTTPD_SSI_MULTIPART
    case 8: { /* "table" */
        printed = snprintf(pcInsert, iInsertLen, "<tr><td>This is table row number %d</td></tr>", current_tag_part + 1);
        // Leave "next_tag_part" unchanged to indicate that all data has been returned for this tag
        if (current_tag_part < 9) {
            *next_tag_part = current_tag_part + 1;
        }
        break;
    }
#endif
    default: { // unknown tag
      printed = 0;
      break;
    }
  }
  return (u16_t)printed;
}

// Be aware of LWIP_HTTPD_MAX_TAG_NAME_LEN
static const char *ssi_tags[] = {
    "status",
    "welcome",
    "uptime",
    "ledstate",
    "ledinv",
    "shellcmd",
    "shstat",
    "shresult",
    "table",
};

#if LWIP_HTTPD_SUPPORT_POST
#define LED_STATE_BUFSIZE 4
#define SHELL_CMD_BUFSIZE 96
static void *current_connection;
typedef enum HttpPostHandler_e {
  HTTP_POST_HANDLER_NONE = 0,
  HTTP_POST_HANDLER_LED,
  HTTP_POST_HANDLER_SHELL,
} HttpPostHandler_e;
static HttpPostHandler_e currentPostHandler = HTTP_POST_HANDLER_NONE;

typedef struct HttpShellOutputCtx_t {
  char *buf;
  size_t pos;
  size_t maxLen;
} HttpShellOutputCtx_t;

static HttpShellOutputCtx_t httpShellOutCtx;

static void HttpShell_ReadChar(uint8_t *c) {
  *c = '\0'; /* no input available */
}

static void HttpShell_OutputChar(uint8_t ch) {
  if (httpShellOutCtx.pos + 1 < httpShellOutCtx.maxLen) {
    httpShellOutCtx.buf[httpShellOutCtx.pos++] = (char)ch;
    httpShellOutCtx.buf[httpShellOutCtx.pos] = '\0';
  }
}

static bool HttpShell_KeyPressed(void) {
  return false;
}

static McuShell_ConstStdIOType httpShellStdIO = {
  .stdIn = (McuShell_StdIO_In_FctType)HttpShell_ReadChar,
  .stdOut = (McuShell_StdIO_OutErr_FctType)HttpShell_OutputChar,
  .stdErr = (McuShell_StdIO_OutErr_FctType)HttpShell_OutputChar,
  .keyPressed = HttpShell_KeyPressed,
#if McuShell_CONFIG_ECHO_ENABLED
  .echoEnabled = false,
#endif
};

err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
        u16_t http_request_len, int content_len, char *response_uri,
        u16_t response_uri_len, u8_t *post_auto_wnd) {
  if (memcmp(uri, "/led.cgi", 8) == 0 && current_connection != connection) {
    current_connection = connection;
    currentPostHandler = HTTP_POST_HANDLER_LED;
    snprintf(response_uri, response_uri_len, "/ledfail.shtml");
    *post_auto_wnd = 1;
    return ERR_OK;
  } else if (memcmp(uri, "/shell.cgi", 10) == 0 && current_connection != connection) {
    current_connection = connection;
    currentPostHandler = HTTP_POST_HANDLER_SHELL;
    snprintf(response_uri, response_uri_len, "/index.shtml");
    *post_auto_wnd = 1;
    return ERR_OK;
  }
  return ERR_VAL;
}

// Return a value for a parameter
char *httpd_param_value(struct pbuf *p, const char *param_name, char *value_buf, size_t value_buf_len) {
  size_t param_len = strlen(param_name);
  u16_t param_pos = pbuf_memfind(p, param_name, param_len, 0);
  if (param_pos != 0xFFFF) {
    u16_t param_value_pos = param_pos + param_len;
    u16_t param_value_len = 0;
    u16_t tmp = pbuf_memfind(p, "&", 1, param_value_pos);
    if (tmp != 0xFFFF) {
      param_value_len = tmp - param_value_pos;
    } else {
      param_value_len = p->tot_len - param_value_pos;
    }
    if (param_value_len > 0 && param_value_len < value_buf_len) {
      char *result = (char *)pbuf_get_contiguous(p, value_buf, value_buf_len, param_value_len, param_value_pos);
      if (result) {
        result[param_value_len] = 0;
        return result;
      }
    }
  }
  return NULL;
}

static int HttpShell_HexToNibble(char ch) {
  if (ch>='0' && ch<='9') {
    return ch-'0';
  } else if (ch>='A' && ch<='F') {
    return 10 + (ch-'A');
  } else if (ch>='a' && ch<='f') {
    return 10 + (ch-'a');
  }
  return -1;
}

static void HttpShell_DecodeUrl(const char *src, char *dst, size_t dstSize) {
  size_t srcIdx = 0;
  size_t dstIdx = 0;

  if (dstSize==0) {
    return;
  }
  while (src[srcIdx]!='\0' && dstIdx+1<dstSize) {
    if (src[srcIdx]=='+') {
      dst[dstIdx++] = ' ';
      srcIdx++;
    } else if (src[srcIdx]=='%' && src[srcIdx+1]!='\0' && src[srcIdx+2]!='\0') {
      int hi = HttpShell_HexToNibble(src[srcIdx+1]);
      int lo = HttpShell_HexToNibble(src[srcIdx+2]);

      if (hi>=0 && lo>=0) {
        dst[dstIdx++] = (char)((hi<<4) | lo);
        srcIdx += 3;
      } else {
        dst[dstIdx++] = src[srcIdx++];
      }
    } else {
      dst[dstIdx++] = src[srcIdx++];
    }
  }
  dst[dstIdx] = '\0';
}

static void HttpShell_HtmlEscape(const char *src, char *dst, size_t dstSize) {
  size_t dstIdx = 0;

  if (dstSize==0) {
    return;
  }
  while (*src!='\0' && dstIdx+1<dstSize) {
    const char *entity = NULL;

    switch(*src) {
      case '&': entity = "&amp;"; break;
      case '<': entity = "&lt;"; break;
      case '>': entity = "&gt;"; break;
      case '"': entity = "&quot;"; break;
      case '\'': entity = "&#39;"; break;
      default:
        dst[dstIdx++] = *src;
        src++;
        continue;
    }
    {
      size_t entLen = strlen(entity);
      if (dstIdx + entLen >= dstSize) {
        break;
      }
      memcpy(&dst[dstIdx], entity, entLen);
      dstIdx += entLen;
    }
    src++;
  }
  dst[dstIdx] = '\0';
}

err_t httpd_post_receive_data(void *connection, struct pbuf *p) {
  err_t ret = ERR_VAL;
  LWIP_ASSERT("NULL pbuf", p != NULL);
  if (current_connection == connection && currentPostHandler == HTTP_POST_HANDLER_LED) {
    char buf[LED_STATE_BUFSIZE];
    char *val = httpd_param_value(p, "led_state=", buf, sizeof(buf));
    if (val) {
        led_on = (strcmp(val, "ON") == 0) ? true : false;
        cyw43_gpio_set(&cyw43_state, 0, led_on);
        ret = ERR_OK;
    }
  } else if (current_connection == connection && currentPostHandler == HTTP_POST_HANDLER_SHELL) {
    char encodedCmd[HTTP_SHELL_LAST_CMD_SIZE];
    char decodedCmd[HTTP_SHELL_LAST_CMD_SIZE];
    char shellOut[sizeof(httpShellLastResult)];
    char *val = httpd_param_value(p, "cmd=", encodedCmd, sizeof(encodedCmd));

    if (val!=NULL) {
      HttpShell_DecodeUrl(val, decodedCmd, sizeof(decodedCmd));
      HttpShell_HtmlEscape(decodedCmd, httpShellLastCmd, sizeof(httpShellLastCmd));
      shellOut[0] = '\0';
      httpShellOutCtx.buf = shellOut;
      httpShellOutCtx.pos = 0;
      httpShellOutCtx.maxLen = sizeof(shellOut);
      httpShellLastSuccess = SHELL_ParseCommandIO((const unsigned char*)decodedCmd, &httpShellStdIO, false)==ERR_OK;
      if (shellOut[0]=='\0') {
        (void)snprintf(httpShellLastResult, sizeof(httpShellLastResult), "%s", httpShellLastSuccess ? "(no output)" : "(command failed)");
      } else {
        (void)snprintf(httpShellLastResult, sizeof(httpShellLastResult), "%s", shellOut);
      }
      HttpShell_HtmlEscape(httpShellLastResult, httpShellLastResultHtml, sizeof(httpShellLastResultHtml));
      ret = ERR_OK;
    } else {
      (void)snprintf(httpShellLastCmd, sizeof(httpShellLastCmd), "%s", "");
      (void)snprintf(httpShellLastResult, sizeof(httpShellLastResult), "%s", "Missing 'cmd' parameter.");
      HttpShell_HtmlEscape(httpShellLastResult, httpShellLastResultHtml, sizeof(httpShellLastResultHtml));
      httpShellLastSuccess = false;
    }
  }
  pbuf_free(p);
  return ret;
}

void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len) {
  snprintf(response_uri, response_uri_len, "/ledfail.shtml");
  if (current_connection == connection && currentPostHandler == HTTP_POST_HANDLER_LED) {
    snprintf(response_uri, response_uri_len, "/ledpass.shtml");
  } else if (current_connection == connection && currentPostHandler == HTTP_POST_HANDLER_SHELL) {
    snprintf(response_uri, response_uri_len, "/index.shtml");
  }
  current_connection = NULL;
  currentPostHandler = HTTP_POST_HANDLER_NONE;
}
#endif /* LWIP_HTTPD_SUPPORT_POST */

static void setupHttpdServer(void) {
#if WIP_MDNS_RESPONDER /* \TODO does not work, gives mutex free error (not used by same task?) */
    // Setup mdns
    cyw43_arch_lwip_begin();
    mdns_resp_init();
    McuLog_info("mdns host name %s.local\n", hostname);
#if LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 2
    mdns_resp_add_netif(&cyw43_state.netif[CYW43_ITF_STA], hostname);
    mdns_resp_add_service(&cyw43_state.netif[CYW43_ITF_STA], "pico_httpd", "_http", DNSSD_PROTO_TCP, 80, srv_txt, NULL);
#else
    mdns_resp_add_netif(&cyw43_state.netif[CYW43_ITF_STA], hostname, 60);
    mdns_resp_add_service(&cyw43_state.netif[CYW43_ITF_STA], "pico_httpd", "_http", DNSSD_PROTO_TCP, 80, 60, srv_txt, NULL);
#endif
    cyw43_arch_lwip_end();
#endif

  cyw43_arch_lwip_begin();
  httpd_init();
  http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));
  http_set_ssi_handler(ssi_example_ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
  cyw43_arch_lwip_end();
}

static void httpdTask(void *pv) {
  vTaskSuspend(NULL);
  setupHttpdServer();
  McuLog_info("Ready, running httpd at %s", ip4addr_ntoa(netif_ip4_addr(netif_list)));
  for(;;) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void HttpdServer_Init(void) {
  if (xTaskCreate(
      httpdTask,  /* pointer to the task */
      "httpd", /* task name for kernel awareness debugging */
      2*1024/sizeof(StackType_t), /* task stack size */
      (void*)NULL, /* optional task startup argument */
      tskIDLE_PRIORITY+2,  /* initial priority */
      &httpTaskHandle /* optional task handle to create */
    ) != pdPASS)
  {
    McuLog_fatal("Failed creating HTTPD task");
    for(;;){} /* error! probably out of memory */
  }
}

#endif /* PL_CONFIG_USE_HTTPD_SERVER */
