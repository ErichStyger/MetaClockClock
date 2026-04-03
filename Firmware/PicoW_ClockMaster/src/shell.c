/*
 * Copyright (c) 2019-2024, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_SHELL
#include "shell.h"
#include "McuShell.h"
#include "McuRTOS.h"
#if PL_CONFIG_USE_RTT
  #include "McuRTT.h"
#endif
#if McuLib_CONFIG_CPU_IS_ARM_CORTEX_M
  #include "McuArmTools.h"
#endif
#include "McuShellUart.h"
#include "McuTimeDate.h"
#include "McuLog.h"
#include "McuUtility.h"
#if PL_CONFIG_USE_BUTTONS
  #include "buttons.h"
#endif
#if PL_CONFIG_USE_LEDS
  #include "leds.h"
#endif
#if PL_CONFIG_USE_BLINKY
  #include "blinky.h"
#endif
#if PL_CONFIG_USE_USB_CDC
  #include "pico/stdlib.h"
#endif
#if PL_CONFIG_USE_WIFI
  #include "wifi.h"
#endif
#if PL_CONFIG_USE_PICO_W
  #include "PicoWiFi.h"
#endif
#if PL_CONFIG_USE_PING
  #include "ping_shell.h"
#endif
#if PL_CONFIG_USE_UDP_SERVER
  #include "udp_server_shell.h"
#endif
#if PL_CONFIG_USE_UDP_CLIENT
  #include "udp_client.h"
#endif
#if PL_CONFIG_USE_MQTT_CLIENT
  #include "mqtt_client.h"
#endif
#if PL_CONFIG_USE_NTP_CLIENT
  #include "ntp_client.h"
#endif
#if PL_CONFIG_USE_SENSOR
  #include "sensor.h"
#endif
#if PL_CONFIG_USE_SENSOR && PL_CONFIG_USE_SHT31
  #include "McuSHT31.h"
#elif PL_CONFIG_USE_SENSOR && PL_CONFIG_USE_SHT40
  #include "McuSHT40.h"
#endif
#if PL_CONFIG_USE_NVMC
  #include "nvmc.h"
#endif
#if McuFlash_CONFIG_IS_ENABLED
  #include "McuFlash.h"
#endif
#if PL_CONFIG_USE_RS485
  #include "rs485.h"
  #include "McuUart485.h"
#endif
#if PL_HAS_RADIO
  #include "RNet/McuRNet.h"
  #include "RNet/RStdIO.h"
  #include "RNet_App.h"
#endif
#if PL_CONFIG_USE_ROBO_REMOTE
  #include "robot.h"
#endif
#if PL_CONFIG_USE_CHALLENGE
  #include "challenge.h"
#endif
#if McuLib_CONFIG_CPU_IS_ESP32
  #include "driver/uart.h"
  #include "driver/gpio.h"
#endif
#if PL_CONFIG_USE_MININI
  #include "minIni/McuMinINI.h"
#endif
#if PL_CONFIG_USE_TUD_CDC
  #include "McuShellCdcDevice.h"
#endif
#if PL_CONFIG_USE_WATCHDOG
  #include "McuWatchdog.h"
#endif
#if PL_CONFIG_USE_BALBOA
  #include "balboa.h"
#endif
#include "application.h"

/* table of shell parsers */
static const McuShell_ParseCommandCallback CmdParserTable[] =
{
  McuShell_ParseCommand,
  McuRTOS_ParseCommand,
#if McuLog_CONFIG_IS_ENABLED
  McuLog_ParseCommand,
#endif
  McuTimeDate_ParseCommand,
#if PL_CONFIG_USE_BUTTONS
  BTN_ParseCommand,
#endif
#if McuLib_CONFIG_CPU_IS_ARM_CORTEX_M
  McuArmTools_ParseCommand,
#endif
#if PL_CONFIG_USE_WIFI
  WiFi_ParseCommand,
#endif
#if PL_CONFIG_USE_UDP_CLIENT
  UdpClient_ParseCommand,
#endif
#if PL_CONFIG_USE_UDP_SERVER
  UdpServer_ParseCommand,
#endif
#if PL_CONFIG_USE_PING
  PING_ParseCommand,
#endif
#if PL_CONFIG_USE_SENSOR && PL_CONFIG_USE_SHT31
  McuSHT31_ParseCommand,
#elif PL_CONFIG_USE_SENSOR && PL_CONFIG_USE_SHT40
  McuSHT40_ParseCommand,
#endif
#if PL_CONFIG_USE_RS485 && McuUart485_CONFIG_USE_RS_485
  McuUart485_ParseCommand,
#endif
#if PL_CONFIG_USE_RS485 && PL_CONFIG_USE_RS485_SHELL && !McuUart485_CONFIG_USE_RAW
  RS485_ParseCommand,
#endif
#if PL_HAS_RADIO
  McuRNet_ParseCommand,
  RNETA_ParseCommand,
#endif
#if PL_CONFIG_USE_LEDS
  Leds_ParseCommand,
#endif
#if PL_CONFIG_USE_BLINKY
  Blinky_ParseCommand,
#endif
#if PL_CONFIG_USE_ROBO_REMOTE
  ROBOT_ParseCommand,
#endif
#if PL_CONFIG_USE_CHALLENGE
  Challenge_ParseCommand,
#endif
#if McuFlash_CONFIG_IS_ENABLED
  McuFlash_ParseCommand,
#endif
#if PL_CONFIG_USE_MININI
  McuMinINI_ParseCommand,
  ini_ParseCommand,
#endif
#if PL_CONFIG_USE_MQTT_CLIENT
  MqttClient_ParseCommand,
#endif
#if PL_CONFIG_USE_NTP_CLIENT
  NtpClient_ParseCommand,
#endif
#if PL_CONFIG_USE_TUD_CDC
  McuShellCdcDevice_ParseCommand,
#endif
#if PL_CONFIG_USE_BALBOA
  Balboa_ParseCommand,
#endif
  NULL /* Sentinel */
};

typedef struct SHELL_IODesc {
  McuShell_ConstStdIOType *stdio;
  unsigned char *buf;
  size_t bufSize;
} SHELL_IODesc;

#if McuLib_CONFIG_CPU_IS_RPxxxx && PL_CONFIG_USE_USB_CDC

static void cdc_StdIOReadChar(uint8_t *c) {
  int res;

  res = getchar_timeout_us(500);
  if (res==-1) { /* no character present */
    *c = '\0';
  } else {
    *c = (uint8_t)res; /* return character */
  }
}

bool cdc_StdIOKeyPressed(void) {
  return true; /* hack, don't know if there is any other way? */
}

void cdc_StdIOSendChar(uint8_t ch) {
  putchar_raw(ch);
}

/* default standard I/O struct */
static McuShell_ConstStdIOType cdc_stdio = {
    .stdIn = (McuShell_StdIO_In_FctType)cdc_StdIOReadChar,
    .stdOut = (McuShell_StdIO_OutErr_FctType)cdc_StdIOSendChar,
    .stdErr = (McuShell_StdIO_OutErr_FctType)cdc_StdIOSendChar,
    .keyPressed = cdc_StdIOKeyPressed, /* if input is not empty */
  #if McuShell_CONFIG_ECHO_ENABLED
    .echoEnabled = false,
  #endif
  };
static uint8_t cdc_DefaultShellBuffer[McuShell_DEFAULT_SHELL_BUFFER_SIZE]; /* default buffer which can be used by the application */
#endif


#if McuLib_CONFIG_CPU_IS_ESP32

#define SHELL_ESP32_UART_DEVICE   (UART_NUM_0) /* Uart for bootloader and connection to robot */

static SemaphoreHandle_t SHELL_stdioMutex; /* mutex to protect access to ESP32 standard I/O */

static void Uart_SendString(const unsigned char *str) {
  size_t len;
  int written;

  len = strlen((const char*)str);
  written = uart_write_bytes(SHELL_ESP32_UART_DEVICE, (const char*)str, len);
  if (written!=len) {
    McuLog_error("failed sending uart bytes");
  }
}

static void Uart_SendChar(unsigned char ch) {
  uart_write_bytes(SHELL_ESP32_UART_DEVICE, &ch, 1);
}

static void Uart_ReadChar(uint8_t *c) {
  unsigned char ch = '\0';
  int len = 0;

  if (xSemaphoreTakeRecursive(SHELL_stdioMutex, portMAX_DELAY)==pdPASS) { /* take mutex */
    len = uart_read_bytes(SHELL_ESP32_UART_DEVICE, &ch, 1, 0);
    (void)xSemaphoreGiveRecursive(SHELL_stdioMutex); /* give back mutex */
  }
  if (len==0) {
    *c = '\0';
  } else {
    *c = ch;
  }
}

static bool Uart_CharPresent(void) {
  size_t size=0;

  if (xSemaphoreTakeRecursive(SHELL_stdioMutex, portMAX_DELAY)==pdPASS) { /* take mutex */
    uart_get_buffered_data_len(SHELL_ESP32_UART_DEVICE, &size);
    (void)xSemaphoreGiveRecursive(SHELL_stdioMutex); /* give back mutex */
  }
  return size!=0;
}

static McuShell_ConstStdIOType Uart_stdio = {
    .stdIn = (McuShell_StdIO_In_FctType)Uart_ReadChar,
    .stdOut = (McuShell_StdIO_OutErr_FctType)Uart_SendChar,
    .stdErr = (McuShell_StdIO_OutErr_FctType)Uart_SendChar,
    .keyPressed = Uart_CharPresent, /* if input is not empty */
  #if McuShell_CONFIG_ECHO_ENABLED
   .echoEnabled = true,
  #endif
  };

static uint8_t Uart_DefaultShellBuffer[McuShell_DEFAULT_SHELL_BUFFER_SIZE]; /* default buffer which can be used by the application */
#endif /* McuLib_CONFIG_CPU_IS_ESP */

static const SHELL_IODesc ios[] =
{
#if McuLib_CONFIG_CPU_IS_ESP32
  {&Uart_stdio,  Uart_DefaultShellBuffer,  sizeof(Uart_DefaultShellBuffer)},
#endif
#if PL_CONFIG_USE_SHELL_UART
  {&McuShellUart_stdio,  McuShellUart_DefaultShellBuffer,  sizeof(McuShellUart_DefaultShellBuffer)},
#endif
#if PL_CONFIG_USE_USB_CDC
  {&cdc_stdio,  cdc_DefaultShellBuffer,  sizeof(cdc_DefaultShellBuffer)},
#endif
#if PL_CONFIG_USE_SHELL_CDC
  {&McuShellCdcDevice_stdio,  McuShellCdcDevice_DefaultShellBuffer,  sizeof(McuShellCdcDevice_DefaultShellBuffer)},
#endif
#if PL_CONFIG_USE_RTT
  {&McuRTT_stdio,  McuRTT_DefaultShellBuffer,  sizeof(McuRTT_DefaultShellBuffer)},
#endif
#if PL_HAS_RADIO && RNET_CONFIG_REMOTE_STDIO
  {&RSTDIO_stdio, RSTDIO_DefaultShellBuffer, sizeof(RSTDIO_DefaultShellBuffer)},
#endif
};

void SHELL_SendChar(unsigned char ch) {
  for(int i=0;i<sizeof(ios)/sizeof(ios[0]);i++) {
    McuShell_SendCh(ch, ios[i].stdio->stdOut);
  }
}

uint8_t SHELL_ParseCommand(unsigned char *cmd) {
  return McuShell_ParseWithCommandTable(cmd, McuShell_GetStdio(), CmdParserTable);
}

void SHELL_SendString(const unsigned char *str) {
#if McuLib_CONFIG_CPU_IS_ESP32
  /* need to improve write speed, as writing character by character is too slow */
  Uart_SendString(str);
#else
  for(int i=0;i<sizeof(ios)/sizeof(ios[0]);i++) {
    McuShell_SendStr(str, ios[i].stdio->stdOut);
  }
#endif
}

void SHELL_SendStringToIO(const unsigned char *str, McuShell_ConstStdIOType *io) {
#if McuLib_CONFIG_CPU_IS_ESP32
  if (io->stdOut == Uart_SendChar) { /* ESP32 UART? */
    /* if out channel is ESP32 UART: speed it up by sending whole buffer */
    Uart_SendString(str);
  } else { /* send it char by char */
    McuShell_SendStr(str, io->stdOut);
  }
#else
  McuShell_SendStr(str, io->stdOut);
#endif
}

uint8_t SHELL_ParseCommandIO(const unsigned char *command, McuShell_ConstStdIOType *io, bool silent) {
  if (io==NULL) { /* use a default */
#if McuLib_CONFIG_CPU_IS_ESP32
    io = &Uart_stdio;
#elif PL_CONFIG_USE_SHELL_UART
    io = &McuShellUart_stdio;
#elif PL_CONFIG_USE_USB_CDC
    io = &cdc_stdio;
#elif PL_CONFIG_USE_RTT
    io = &McuRTT_stdio;
#else
  #error "no shell std IO?"
#endif
  }
  return McuShell_ParseWithCommandTableExt(command, io, CmdParserTable, silent);
}

#if McuLib_CONFIG_CPU_IS_ESP32
/* ----------------- buffer handling for shell messages sent to ESP32 */
static unsigned char *esp_io_buf; /* pointer to buffer */
static size_t esp_io_buf_size; /* size of buffer */

static void esp_io_buf_SendChar(unsigned char ch) {
  McuUtility_chcat(esp_io_buf, esp_io_buf_size, ch);
}

static void esp_io_buf_ReadChar(uint8_t *c) {
  *c = '\0';
}

static bool esp_io_buf_CharPresent(void) {
  return false;
}

static McuShell_ConstStdIOType esp_stdio = {
  .stdIn = (McuShell_StdIO_In_FctType)esp_io_buf_ReadChar,
  .stdOut = (McuShell_StdIO_OutErr_FctType)esp_io_buf_SendChar,
  .stdErr = (McuShell_StdIO_OutErr_FctType)esp_io_buf_SendChar,
  .keyPressed = esp_io_buf_CharPresent, /* if input is not empty */
#if McuShell_CONFIG_ECHO_ENABLED
  .echoEnabled = false, /* echo enabled for idf.py monitor */
#endif
};

void SHELL_SendToESPAndGetResponse(const unsigned char *msg, unsigned char *response, size_t responseSize) {
  esp_io_buf = response;
  esp_io_buf_size = responseSize;
  esp_io_buf[0] = '\0'; /* initialize buffer */
  McuLog_info("Sending to ESP Shell: %s", msg);
  McuShell_ParseWithCommandTableExt(msg, &esp_stdio, CmdParserTable, true); /* send to ESP32 shell */
  if (response[0]=='\0') { /* empty response? add a default */
    McuUtility_strcpy(response, responseSize, (unsigned char*)"OK"); /* default response */
  }
}
/* ----------------------------------------------------------------------*/
void SHELL_SendToRobotAndGetResponse(const unsigned char *send, unsigned char *response, size_t responseSize) {
  unsigned char buffer[128]; /* buffer for sending command to robot */

  /* build a frame around the message: that way the robot is able to recognize it */
  McuUtility_strcpy(buffer, sizeof(buffer), (unsigned char*)"@robot:cmd ");
  McuUtility_strcat(buffer, sizeof(buffer), send);
  McuUtility_strcat(buffer, sizeof(buffer), (unsigned char*)"!\r\n");
  SHELL_SendString(buffer); /* send to UART, which is read by the robot */
  /* get response */
#if 1
  /* Important: this consumes directly all characters coming from the robot. That way the ESP32 shell does not get it.
   * A mutex is used to block the shell from getting the UART stream.
   */
  #define TIMEOUT_MS  (500) /* stop if we don't get new input after this timeout */
  int timeoutMs = TIMEOUT_MS;

  *response = '\0';
  if (xSemaphoreTakeRecursive(SHELL_stdioMutex, portMAX_DELAY)==pdPASS) { /* take mutex */
    while (true) { /* breaks after timeout */
      if (!Uart_stdio.keyPressed()) { /* no input: wait for timeout */
        timeoutMs -= 50;
        if (timeoutMs<=0) {
          break; /* timeout */
        }
        vTaskDelay(pdMS_TO_TICKS(50));
      } else { /* character available */
        unsigned char ch;
        Uart_stdio.stdIn(&ch);
        if (ch!='\r') { /* filter out '\r' in "\r\n" */
          McuUtility_chcat(response, responseSize, ch);
        }
        timeoutMs = TIMEOUT_MS; /* reset timeout */
      } /* if */
    } /* while */
    (void)xSemaphoreGiveRecursive(SHELL_stdioMutex); /* give back mutex */
  }
  if (*response=='\0') { /* if response is empty, send back at least an acknowledgment */
    McuUtility_strcpy(response, responseSize, (unsigned char*)"OK"); /* default response */
  }
#else
  McuUtility_strcpy(response, responseSize, (unsigned char*)"OK"); /* default response */
#endif
}
#endif /* McuLib_CONFIG_CPU_IS_ESP32 */
/* ----------------------------------------------------------------------*/

static void ShellTask(void *pvParameters) {
  int i;

  (void)pvParameters; /* not used */
  McuLog_trace("started shell task");
  /* initialize buffers */
  for(i=0;i<sizeof(ios)/sizeof(ios[0]);i++) {
    ios[i].buf[0] = '\0'; /* initialize I/O buffers */
  }
  vTaskDelay(pdMS_TO_TICKS(500)); /* give some time to other tasks to start up */
  McuShell_PrintPrompt(McuShell_GetStdio()); /* print prompt */
  for(;;) {
    /* process all I/Os */
    for(i=0;i<sizeof(ios)/sizeof(ios[0]);i++) {
      (void)McuShell_ReadAndParseWithCommandTable(ios[i].buf, ios[i].bufSize, ios[i].stdio, CmdParserTable);
    }
  #if PL_HAS_RADIO && RNET_CONFIG_REMOTE_STDIO
    #if PL_CONFIG_USE_SHELL_CDC
    RSTDIO_Print(McuShellCdcDevice_GetStdio()); /* dispatch incoming messages */
    #elif PL_CONFIG_USE_RTT
    RSTDIO_Print(McuRTT_GetStdio()); /* dispatch incoming messages */
    #endif
  #endif
  #if PL_CONFIG_USE_WATCHDOG
    McuWatchdog_DelayAndReport(McuWatchdog_REPORT_ID_TASK_SHELL, 1, 5);
  #else
    vTaskDelay(pdMS_TO_TICKS(20));
  #endif
  } /* for */
}

#if McuLib_CONFIG_CPU_IS_ESP32
static void InitUart(void) {
#define ESP32_UART_BUF_SIZE  512
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 0,
  };

  /* Configure UART parameters */
  uart_param_config(SHELL_ESP32_UART_DEVICE, &uart_config);
  uart_set_pin(SHELL_ESP32_UART_DEVICE, GPIO_NUM_1, GPIO_NUM_3, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  /* Install UART driver (we don't need an event queue here) */
  uart_driver_install(SHELL_ESP32_UART_DEVICE, ESP32_UART_BUF_SIZE*2, ESP32_UART_BUF_SIZE*2, 0, NULL, 0);
  uart_set_mode(SHELL_ESP32_UART_DEVICE, UART_MODE_UART);
}
#endif/* McuLib_CONFIG_CPU_IS_ESP32 */

static void ConfigureLogger(void) {
#if McuLog_CONFIG_IS_ENABLED
	#if McuLib_CONFIG_CPU_IS_ESP32
  	McuLog_set_console(&Uart_stdio, 0);
  #elif PL_CONFIG_USE_RTT && PL_CONFIG_USE_SHELL_UART && McuLog_CONFIG_NOF_CONSOLE_LOGGER==2 /* both */
    McuLog_set_console(McuRTT_GetStdio(), 0);
    McuLog_set_console(&McuShellUart_stdio, 1);
  #elif PL_CONFIG_USE_RTT && PL_CONFIG_USE_USB_CDC && McuLog_CONFIG_NOF_CONSOLE_LOGGER==2 /* both */
    McuLog_set_console(McuRTT_GetStdio(), 0);
    McuLog_set_console(&cdc_stdio, 1);
  #elif PL_CONFIG_USE_RTT && PL_CONFIG_USE_TUD_CDC && McuLog_CONFIG_NOF_CONSOLE_LOGGER==2 /* both */
    McuLog_set_console(McuRTT_GetStdio(), 0);
    McuLog_set_console(McuShellCdcDevice_GetStdio(), 1);
  #elif PL_CONFIG_USE_RTT /* only RTT */
    McuLog_set_console(McuRTT_GetStdio(), 0);
  #elif PL_CONFIG_USE_SHELL_UART /* only UART */
    McuLog_set_console(&McuShellUart_stdio, 0);
  #elif PL_CONFIG_USE_TUD_CDC
    McuLog_set_console(McuShellCdcDevice_GetStdio(), 0);
  #endif
#endif
}

void SHELL_Init(void) {
  BaseType_t res;

#if McuLib_CONFIG_CPU_IS_ESP32
  InitUart();

  SHELL_stdioMutex = xSemaphoreCreateRecursiveMutex();
  if (SHELL_stdioMutex==NULL) { /* creation failed? */
    McuLog_fatal("Failed creating mutex");
    for(;;);
  }
  vQueueAddToRegistry(SHELL_stdioMutex, "ShellStdIoMutex");
#endif

#if McuLib_CONFIG_CPU_IS_ESP32
  McuShell_SetStdio(&Uart_stdio);
#elif McuLib_CONFIG_CPU_IS_RPxxxx && PL_CONFIG_USE_USB_CDC
  McuShell_SetStdio(&cdc_stdio); /* send to USB CDC */
#elif McuLib_CONFIG_CPU_IS_RPxxxx && PL_CONFIG_USE_TUD_CDC
  McuShell_SetStdio(McuShellCdcDevice_GetStdio()); /* send to USB CDC */
#elif PL_CONFIG_USE_RTT
  McuShell_SetStdio(McuRTT_GetStdio()); /* use RTT as the default */
#else
  #error "need a standard I/O defined"
#endif
  ConfigureLogger();
  res = xTaskCreate(ShellTask, "ShellTask", 4*1024/sizeof(StackType_t), NULL, tskIDLE_PRIORITY+4, NULL);
  if (res!=pdPASS) {
    McuLog_fatal("creating ShellTask task failed!");  // GCOVR_EXCL_LINE
    for(;;) {}                                        // GCOVR_EXCL_LINE
  }
}

void SHELL_Deinit(void) {     // GCOVR_EXCL_LINE
  McuShell_SetStdio(NULL);    // GCOVR_EXCL_LINE
}                             // GCOVR_EXCL_LINE

#endif /* PL_CONFIG_USE_SHELL */
