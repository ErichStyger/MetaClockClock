/*
 * Copyright (c) 2023, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_UNIT_TESTS
#include "tests.h"
#include "unity.h"
#include "McuUnity.h"
#include "McuRTOS.h"
#include "McuShell.h"
#include "McuRTT.h"
#include "McuUtility.h"
#include "McuLog.h"
#include "test_sensor.h"
#include "test_leds.h"
#include "test_dns_resolver.h"

static void TestArgFailed(void) {
  TEST_ASSERT_MESSAGE(false, "wrong test_arg value");
}

void Tests_Run(void) {
  int nofFailures;
  uint32_t test_arg = -1;
  int nofBytes;
  unsigned char buf[32];

  nofBytes = McuUnity_RTT_GetArgs(buf, sizeof(buf));
  UNITY_BEGIN();
  if (nofBytes>0) {
    if (McuUtility_strcmp(buf, "led")==0) {
      RUN_TEST(TestLeds_OnOff); 
      RUN_TEST(TestLeds_Toggle);
    } else if (McuUtility_strcmp(buf, "sensor")==0) {
      RUN_TEST(TestSensor_Temperature);
      RUN_TEST(TestSensor_Humidity);
      RUN_TEST(TestSensor_Both);
    } else if (McuUtility_strcmp(buf, "dns")==0) {
      RUN_TEST(TestDnsResolver_Test);
    } else {
      RUN_TEST(TestArgFailed);
     }
  } else {
    RUN_TEST(TestArgFailed);
  }
  nofFailures = UNITY_END();
#if PL_CONFIG_USE_RTT
  McuUnity_Exit_JRun_RTT(nofFailures==0);
#else
  McuUnity_Exit_LinkServer_Log(nofFailures==0);
#endif
}

static void TestTask(void *pv) {
  McuLog_info("starting test task");
  vTaskDelay(pdMS_TO_TICKS(100)); /* give sensor some time */
  Tests_Run();
  vTaskDelete(NULL); /* terminate task */
}

void Tests_Init(void) {
  if (xTaskCreate(
      TestTask,  /* pointer to the task */
      "Test", /* task name for kernel awareness debugging */
      1500/sizeof(StackType_t), /* task stack size */
      (void*)NULL, /* optional task startup argument */
      tskIDLE_PRIORITY,  /* initial priority */
      (TaskHandle_t*)NULL /* optional task handle to create */
    ) != pdPASS)
  {
    McuLog_fatal("Failed creating task");
    for(;;){} /* error! probably out of memory */
  }
}
#endif /* PL_CONFIG_USE_UNIT_TESTS */