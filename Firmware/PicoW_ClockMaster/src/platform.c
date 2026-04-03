/*
 * Copyright (c) 2019-2024, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_PICO_W
  #include "pico/cyw43_arch.h" /* must be first, otherwise conflict with lwIP ERR_OK */
  #include "PicoWiFi.h"
#endif
#include "McuLib.h"
#if PL_CONFIG_USE_WIFI
  #include "wifi.h"
#endif
#if PL_CONFIG_USE_BLINKY
  #include "blinky.h"
#endif
#if PL_CONFIG_USE_USB_CDC
  #include "pico/stdlib.h"
#endif
#if PL_CONFIG_USE_BUTTONS
  #include "McuButton.h"
  #include "McuDebounce.h"
  #include "buttons.h"
  #include "debounce.h"
#endif
#if PL_CONFIG_USE_UDP_CLIENT
  #include "udp_client.h"
#endif
#if PL_CONFIG_USE_UDP_SERVER
  #include "udp_server.h"
#endif
#if PL_CONFIG_USE_SHELL
  #include "shell.h"
#endif
#if PL_CONFIG_USE_NVMC
  #include "McuFlash.h"
  #include "nvmc.h"
#endif
#if PL_CONFIG_USE_MININI
  #include "MinIni/McuMinINI.h"
#endif
#if PL_CONFIG_USE_PING
  #include "ping_shell.h"
#endif
#if PL_CONFIG_USE_NTP_CLIENT
  #include "ntp_client.h"
#endif
#if PL_CONFIG_USE_MQTT_CLIENT
  #include "mqtt_client.h"
#endif
#if PL_CONFIG_USE_TIME_DATE
  #include "McuTimeDate.h"
  #include "timer.h"
#endif
#if PL_CONFIG_USE_ROBO_REMOTE
  #include "robot.h"
#endif
#if PL_CONFIG_USE_CHALLENGE
  #include "challenge.h"
#endif
#include "McuWait.h"
#include "McuGPIO.h"
#include "McuLED.h"
#include "McuUtility.h"
#include "McuLog.h"
#include "McuXFormat.h"
#if McuLib_CONFIG_SDK_USE_FREERTOS
  #include "McuRTOS.h"
#endif
#if PL_CONFIG_USE_USB_CDC
  #include "pico/stdio_usb.h"
#endif
#if PL_CONFIG_USE_TUD_CDC
  #include "McuShellCdcDevice.h"
#endif
#if PL_CONFIG_USE_UART
  #include "pico/stdio_uart.h"
#endif
#if PL_CONFIG_USE_SEMIHOSTING
  #include "pico/stdio_semihosting.h"
#endif
#if PL_CONFIG_USE_SHELL
  #include "shell.h"
#endif
#if PL_CONFIG_USE_RTT
  #include "McuRTT.h"
#endif
#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  #include "McuSystemView.h"
#endif
#if PL_CONFIG_USE_SPI
  #include "McuSPI.h"
#endif
#if PL_HAS_RADIO
  #include "RNet_App.h"
#endif
#if PL_CONFIG_USE_I2C
  #include "McuGenericI2C.h"
  #include "McuGenericSWI2C.h"
  #include "McuI2cLib.h"
#endif
#if PL_CONFIG_USE_OLED
  #include "McuSSD1306.h"
  #include "oled.h"
#endif
#if PL_CONFIG_USE_SENSOR
  #include "sensor.h"
#endif
#if McuUart485_CONFIG_USE_RS_485
  #include "McuUart485.h"
#endif
#if PL_CONFIG_USE_RS485_SHELL
  #include "rs485.h"
#endif
#if PL_CONFIG_USE_UNIT_TESTS
  #include "tests/tests.h"
#endif
#if McuRdimon_CONFIG_IS_ENABLED
  #include "rdimon/McuRdimon.h"
#endif
#if PL_CONFIG_USE_GCOV
  #include "McuCoverage.h"
#endif
#if PL_CONFIG_USE_LEDS
  #include "leds.h"
#endif
#if PL_CONFIG_USE_MCUFLASH
  #include "McuFlash.h"
#endif
#if PL_CONFIG_USE_MININI
  #include "minIni/McuMinINI.h"
#endif
#if McuSemihost_CONFIG_IS_ENABLED
  #include "McuSemihost.h"
#endif
#if PL_CONFIG_USE_GPROF
  #include "gprof_support.h"
#endif
#if PL_CONFIG_USE_WATCHDOG
  #include "McuWatchdog.h"
#endif
#include "McuHardFault.h"
#include "application.h"
#include "hardware/timer.h"
#if PL_CONFIG_USE_BALBOA
  #include "balboa.h"
#endif

#if PL_CONFIG_USE_WATCHDOG
static void PL_InitWatchdogReportTable(void) {
  McuWatchdog_InitReportEntry(McuWatchdog_REPORT_ID_TASK_APP, "App", 1000, 70, 120);
  #if PL_CONFIG_USE_SHELL
    McuWatchdog_InitReportEntry(McuWatchdog_REPORT_ID_TASK_SHELL, "Shell", 1000, 70, 120);
  #endif
  #if PL_CONFIG_USE_WIFI
    McuWatchdog_InitReportEntry(McuWatchdog_REPORT_ID_TASK_WIFI, "WiFi", 1000, 70, 120);
  #endif
  #if PL_CONFIG_USE_BALBOA
    McuWatchdog_InitReportEntry(McuWatchdog_REPORT_ID_TASK_BALBOA, "Balboa", 1000, 70, 120);
  #endif
}
#endif

void PL_Init(void) {
  portDISABLE_ALL_INTERRUPTS();
#if McuLib_CONFIG_SDK_USE_FREERTOS
  McuRTOS_Init();
#endif
#if PL_CONFIG_USE_RTT
  McuRTT_Init();
#endif
#if configUSE_SEGGER_SYSTEM_VIEWER_HOOKS
  /* do this early so we get the initialization events */
  McuSystemView_Init();
  SEGGER_SYSVIEW_Start(); /* do this to get memory events and others */
#endif
#if PL_CONFIG_USE_WATCHDOG
  PL_InitWatchdogReportTable();
  McuWatchdog_EnableTimer(); /* Enable watchdog timer early, to catch any deadlocks during initialization */
  McuWatchdog_Init();
#endif
#if McuRdimon_CONFIG_IS_ENABLED
  McuRdimon_Init();
#endif
#if PL_CONFIG_USE_TUD_CDC
  McuShellCdcDevice_Init();
  McuShellCdcDevice_SetBufferRxCharCallback(McuShellCdcDevice_QueueChar);
#endif
#if PL_CONFIG_USE_GCOV
  McuCoverage_Init();  /* initialize library */
  McuCoverage_Check(); /* testing only */
  //gcov_test(3); /* testing only */
#endif
#if PL_CONFIG_USE_GPROF
  if (profile_file_write_check()!=1) {
    McuLog_fatal("cannot write file with semihosting");
    for(;;) {}
  }
  gprof_init_timer();
  // gprof_test(); /* test only */
#endif
#if PL_CONFIG_USE_SEMIHOSTING
  stdio_semihosting_init();
  stdio_set_translate_crlf(&stdio_semihosting, false);
#endif
#if 1 /* workaround for CMSIS-DAP, see https://github.com/raspberrypi/pico-sdk/issues/1152 */
  timer_hw->dbgpause = 0;
#endif
  McuLib_Init();
  McuHardFault_Init();
  McuWait_Init();
#if PL_CONFIG_USE_PICO_W
  McuWait_Waitms(50); /* need to wait some time, otherwise cyw43_arch_init() might hang. */
#endif
  McuLog_Init();
  McuUtility_Init();
  McuGPIO_Init();
  McuLED_Init();
#if PL_CONFIG_USE_TIME_DATE
  McuTimeDate_Init();
  TMR_Init();
#endif
#if PL_CONFIG_USE_LEDS
  Leds_Init();
#endif
#if PL_CONFIG_USE_MCUFLASH
  McuFlash_Init();
  McuFlash_RegisterMemory((void*)McuMinINI_CONFIG_FLASH_NVM_ADDR_START, McuMinINI_CONFIG_FLASH_NVM_NOF_BLOCKS*McuMinINI_CONFIG_FLASH_NVM_BLOCK_SIZE);
#endif
#if PL_CONFIG_USE_MININI
  McuMinINI_Init();
#endif
#if PL_CONFIG_USE_BLINKY
  Blinky_Init();
#endif
#if PL_CONFIG_USE_BUTTONS
  McuBtn_Init();
  BTN_Init();
  McuDbnc_Init();
  Debounce_Init();
#endif
#if PL_CONFIG_USE_WIFI && McuLib_CONFIG_CPU_IS_ESP32
  ESP_ERROR_CHECK(nvs_flash_init()); /* need to call this before using any WiFi functions */
  ESP32_MacInit();
#endif
#if PL_CONFIG_USE_WIFI
  WiFi_Init();
#endif
#if PL_CONFIG_USE_PICO_W
  PicoWiFi_Init();
#endif

#if PL_CONFIG_USE_UDP_SERVER
  UdpServer_Init();
#endif
#if PL_CONFIG_USE_UDP_CLIENT
  UdpClient_Init();
#endif
#if PL_CONFIG_USE_PING
  PING_Init();
#endif
#if PL_CONFIG_USE_NTP_CLIENT
  NtpClient_Init();
#endif
#if PL_CONFIG_USE_MQTT_CLIENT
  MqttClient_Init();
#endif
#if PL_CONFIG_USE_SPI
  McuSPI_Init();
#endif
#if PL_HAS_RADIO
  RNETA_Init();
#endif
#if PL_CONFIG_USE_SHELL
  McuXFormat_Init();
  McuShell_Init();
  SHELL_Init();
#endif
#if PL_CONFIG_USE_NVMC
  NVMC_Init();
#endif
#if PL_CONFIG_USE_I2C
  #if CONFIG_USE_HW_I2C
    McuI2cLib_Init();
  #else
    McuGenericSWI2C_Init();
  #endif
  McuGenericI2C_Init();
#endif
#if PL_CONFIG_USE_OLED
  OLED_Init();
#endif
#if PL_CONFIG_USE_SENSOR
  Sensor_Init();
#endif
#if PL_CONFIG_USE_NVMC
  NVMC_Init();
#endif
#if PL_CONFIG_USE_MININI
  McuMinINI_Init();
  McuFlash_Init();
  McuFlash_RegisterMemory((const void*)McuMinINI_CONFIG_FLASH_NVM_ADDR_START, 1*McuMinINI_CONFIG_FLASH_NVM_BLOCK_SIZE);
#endif
#if PL_CONFIG_USE_RS485_SHELL
  RS485_Init();
#elif PL_CONFIG_USE_RS485 && McuUart485_CONFIG_USE_RAW
  McuUart485_Init();
#endif
#if PL_CONFIG_USE_ROBO_REMOTE
  ROBOT_Init();
#endif
#if PL_CONFIG_USE_CHALLENGE
  Challenge_Init();
#endif
#if PL_CONFIG_USE_UNIT_TESTS
  Tests_Init();
#endif
#if McuSemihost_CONFIG_IS_ENABLED
  McuSemiHost_Init();
#endif
#if PL_CONFIG_USE_BALBOA
  Balboa_Init();
#endif
  App_Init();
}
