/*
 * Copyright (c) 2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "platform.h"
#if PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_VERKEHRSHAUS
#include "vh.h"
#include "McuRTOS.h"
#include "McuUtility.h"
#include "McuLog.h"
#include "NeoPixel.h"

static void test(void) {
  uint8_t r, g, b, w;
  #define MIN_VAL  1  /* miniumum brightness */
  #define MAX_VAL  200 /* maximum brightness */

  (void)NEO_ClearAllPixel();
  for(int i=0; i<1; i++) {
    r = g = b = w = 0;
    for(r=MIN_VAL; r<=MAX_VAL; r++) {
      for(int pos=0; pos<NEO_NOF_PIXEL; pos++) {
        (void)NEO_SetPixelWRGB(0, pos, w, r, g, b);
      }
      (void)NEO_TransferPixels();
      vTaskDelay(pdMS_TO_TICKS(20));
    }
    for(r=MAX_VAL; r>=MIN_VAL; r--) {
      for(int pos=0; pos<NEO_NOF_PIXEL; pos++) {
        (void)NEO_SetPixelWRGB(0, pos, w, r, g, b);
      }
      (void)NEO_TransferPixels();
      vTaskDelay(pdMS_TO_TICKS(20));
    }

    r = g = b = w = 0;
    for(g=MIN_VAL; g<=MAX_VAL; g++) {
      for(int pos=0; pos<NEO_NOF_PIXEL; pos++) {
        (void)NEO_SetPixelWRGB(0, pos, w, r, g, b);
      }
      (void)NEO_TransferPixels();
      vTaskDelay(pdMS_TO_TICKS(20));
    }
    for(g=MAX_VAL; g>=MIN_VAL; g--) {
      for(int pos=0; pos<NEO_NOF_PIXEL; pos++) {
        (void)NEO_SetPixelWRGB(0, pos, w, r, g, b);
      }
      (void)NEO_TransferPixels();
      vTaskDelay(pdMS_TO_TICKS(20));
    }

    r = g = b = w = 0;
    for(b=MIN_VAL; b<=MAX_VAL; b++) {
      for(int pos=0; pos<NEO_NOF_PIXEL; pos++) {
        (void)NEO_SetPixelWRGB(0, pos, w, r, g, b);
      }
      (void)NEO_TransferPixels();
      vTaskDelay(pdMS_TO_TICKS(20));
    }
    for(b=MAX_VAL; b>=MIN_VAL; b--) {
      for(int pos=0; pos<NEO_NOF_PIXEL; pos++) {
        (void)NEO_SetPixelWRGB(0, pos, w, r, g, b);
      }
      (void)NEO_TransferPixels();
      vTaskDelay(pdMS_TO_TICKS(20));
    }

    r = g = b = w = 0;
    for(w=MIN_VAL; w<=MAX_VAL; w++) {
      for(int pos=0; pos<NEO_NOF_PIXEL; pos++) {
        (void)NEO_SetPixelWRGB(0, pos, w, r, g, b);
      }
      (void)NEO_TransferPixels();
      vTaskDelay(pdMS_TO_TICKS(20));
    }
    for(w=MAX_VAL; w>=MIN_VAL; w--) {
      for(int pos=0; pos<NEO_NOF_PIXEL; pos++) {
        (void)NEO_SetPixelWRGB(0, pos, w, r, g, b);
      }
      (void)NEO_TransferPixels();
      vTaskDelay(pdMS_TO_TICKS(20));
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

static void test1(void) {
  uint32_t color;
  int k;

  for(int m=0; m<5; m++) {
    color = (0x5<<m)+m;
    k = 0;
    for(int pos=0; pos<NEO_NOF_PIXEL; pos++) {
      (void)NEO_SetPixelColor(0, pos, color);
      color <<= 1; k++;
      if (k==31) {
        k = 0;
        color = 0x5;
      }
    }
    (void)NEO_TransferPixels();
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

static void random(void) {
  uint8_t r, g, b;

  for(int pos=0; pos<NEO_NOF_PIXEL; pos++) {
    r = McuUtility_random(0, 0x20);
    g = McuUtility_random(0, 0x20);
    b = McuUtility_random(0, 0x20);
    (void)NEO_SetPixelWRGB(0, pos, 0 ,r,g,b);
    (void)NEO_TransferPixels();
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

static void vhTask(void *pv) {
  for(;;) {
    random();
    test1();
    test();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void VH_Init(void) {
  McuUtility_randomSetSeed(0xcafe);
  if (xTaskCreate(
      vhTask,  /* pointer to the task */
      "VerkehrsHaus", /* task name for kernel awareness debugging */
      2500/sizeof(StackType_t), /* task stack size */
      (void*)NULL, /* optional task startup argument */
      tskIDLE_PRIORITY+2,  /* initial priority */
      (TaskHandle_t*)NULL /* optional task handle to create */
    ) != pdPASS)
  {
    McuLog_fatal("Failed creating vh task.");
    for(;;){} /* error! probably out of memory */
  }
}
#endif /* #if PL_MATRIX_CONFIGURATION_ID==PL_MATRIX_ID_VERKEHRSHAUS */
