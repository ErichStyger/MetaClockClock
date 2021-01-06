/*
 * pixel.c
 *
 * Copyright (c) 2020: Erich Styger
 * License: PDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_LED_PIXEL
#include "pixel.h"
#include "McuRTOS.h"
#include "McuLog.h"
#if 0
static void PixelTask(void *pv) {
  for(;;) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
#endif

void PIXEL_Init(void) {
#if 0
  if (xTaskCreate(
      PixelTask,  /* pointer to the task */
      "Pixel", /* task name for kernel awareness debugging */
      1024/sizeof(StackType_t), /* task stack size */
      (void*)NULL, /* optional task startup argument */
      tskIDLE_PRIORITY+1,  /* initial priority */
      NULL /* optional task handle to create */
    ) != pdPASS) {
     McuLog_fatal("failed creating Pixel task");
     for(;;){} /* error! probably out of memory */
  }
#endif
}

#endif /* PL_CONFIG_USE_LED_PIXEL */
