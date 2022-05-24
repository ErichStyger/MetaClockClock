/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#include "application.h"
#include "McuRTOS.h"
#if PL_CONFIG_USE_NEO_PIXEL_HW
  #include "NeoPixel.h"
  #include "NeoStepperRing.h"
#endif
#include "matrix.h"
#include "matrixhand.h"
#include "matrixring.h"
#include "matrixpixel.h"
#include "McuLog.h"
#include "McuUtility.h"
#include "StepperBoard.h"

#if PL_CONFIG_USE_NEO_PIXEL_HW
  static SemaphoreHandle_t semNeoUpdate; /* semaphore use to trigger a display update */
#endif

#if PL_CONFIG_USE_SHELL
uint8_t APP_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  return ERR_OK;
}
#endif

#if PL_CONFIG_USE_NEO_PIXEL_HW
#define TEST_MODE   (0)  /* test mode for rings */

#if TEST_MODE
static void TestRing(void) {
  uint8_t cR, cG, cB;
  int i;

  cR = 0x5;
  cG = 0;
  cB = 0;
  i = 0;
  NEO_ClearAllPixel();
  for(;;) {
    for(int lane=NEOC_LANE_START; lane<=NEOC_LANE_END; lane++) {
      NEO_SetPixelRGB(lane, i, cR, cG, cB);
    }
    NEO_TransferPixels();
    i++;
    if (i==NEOSR_NOF_RING_LED) {
      i = 0;
      NEO_ClearAllPixel();
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
#endif

#if PL_CONFIG_USE_NEO_PIXEL_HW
bool APP_RequestUpdateLEDsFromISR(void) {
  BaseType_t xHigherPriorityTaskWoken;

  xSemaphoreGiveFromISR(semNeoUpdate, &xHigherPriorityTaskWoken);
  return xHigherPriorityTaskWoken==pdTRUE;
}

void APP_RequestUpdateLEDs(void) {
  xSemaphoreGive(semNeoUpdate);
}
#endif

static void NeoTask(void *pv) {
  BaseType_t res;

  vTaskDelay(pdMS_TO_TICKS(100));
  McuLog_trace("Starting Neo Task");
  NEO_ClearAllPixel();
  NEO_TransferPixels();
#if PL_CONFIG_USE_LED_RING
  uint32_t color;

  vTaskDelay(pdMS_TO_TICKS(500)); /* wait a bit until to turn on LED hands */
  MHAND_HandEnableAll(true);
  color = MATRIX_GetHandColorAdjusted();
  MHAND_SetHandColorAll(color);
#if PL_CONFIG_USE_EXTENDED_HANDS
  MHAND_2ndHandEnableAll(false);
  MHAND_Set2ndHandColorAll(color);
#endif
  MRING_SetRingColorAll(0x1, 0, 0);
  APP_RequestUpdateLEDs();
#endif
#if 0 && PL_CONFIG_USE_MOTOR_ON_OFF
  vTaskDelay(pdMS_TO_TICKS(McuUtility_random(200, 1500))); /* wait a random time to avoid too much current rush */
  STEPBOARD_MotorSwitchOnOff(STEPBOARD_GetBoard(), true); /* turn on motors */
#endif
#if TEST_MODE
  TestRing();
#endif
#if PL_CONFIG_USE_LED_PIXEL /* initial demo code */
  for(int y=0; y<MPIXEL_NOF_Y; y++) {
    for(int x=0; x<MPIXEL_NOF_X; x++) {
      MPIXEL_SetColor(x, y, 0, x, y, (x+y)/2);
    }
  }
  APP_RequestUpdateLEDs();
#endif
  for(;;) {
#if PL_CONFIG_USE_NEO_PIXEL_HW
    res = xSemaphoreTake(semNeoUpdate, portMAX_DELAY); /* block until we get a request to update */
    if (res==pdTRUE) { /* received the signal */
   #if PL_CONFIG_USE_LED_RING
      NEO_ClearAllPixel();
      MATRIX_IlluminateHands();
   #endif
      NEO_TransferPixels();
    }
#endif
    vTaskDelay(pdMS_TO_TICKS(20)); /* can wait 20 ms until a next update is needed */
  } /* for */
}
#endif /* PL_CONFIG_USE_NEO_PIXEL_HW */

#if configUSE_HEAP_SCHEME==5
#if PL_CONFIG_IS_K02 /* K02 has two 8K memory banks */
  static __attribute__ ((used,section(".noinit.$SRAM_LOWER.Heap5"))) uint8_t heap_sram_lower[8*1024]; /* placed in in no_init section inside SRAM_LOWER */
  static __attribute__ ((used,section(".noinit_Heap5"))) uint8_t heap_sram_upper[3*1024]; /* placed in in no_init section inside SRAM_UPPER */
#elif PL_CONFIG_IS_TINYK22 /* K22FN512 has two 64K memory banks */
  static __attribute__ ((used,section(".noinit.$SRAM_LOWER.Heap5"))) uint8_t heap_sram_lower[64*1024]; /* placed in in no_init section inside SRAM_LOWER */
  static __attribute__ ((used,section(".noinit_Heap5"))) uint8_t heap_sram_upper[16*1024]; /* placed in in no_init section inside SRAM_UPPER */
#endif
  static const HeapRegion_t xHeapRegions[] =
  {
   { &heap_sram_lower[0], sizeof(heap_sram_lower)},
   { &heap_sram_upper[0], sizeof(heap_sram_upper)},
   { NULL, 0 } /* << Terminates the array. */
  };
#endif

void APP_Run(void) {
#if configUSE_HEAP_SCHEME==5
  vPortDefineHeapRegions(xHeapRegions); /* Pass the array into vPortDefineHeapRegions(). Must be called first! */
#endif
#if PL_CONFIG_USE_NEO_PIXEL_HW
  if (xTaskCreate(
      NeoTask,  /* pointer to the task */
      "Neo", /* task name for kernel awareness debugging */
      900/sizeof(StackType_t), /* task stack size */
      (void*)NULL, /* optional task startup argument */
      tskIDLE_PRIORITY+2,  /* initial priority */
      (TaskHandle_t*)NULL /* optional task handle to create */
    ) != pdPASS) {
     for(;;){} /* error! probably out of memory */
  }
  semNeoUpdate = xSemaphoreCreateBinary();
  if (semNeoUpdate==NULL) {
    McuLog_fatal("Failed creating binary semaphore.");
    for(;;){}
  }
  vQueueAddToRegistry(semNeoUpdate, "semNeoUpdate");
  xSemaphoreGive(semNeoUpdate); /* trigger initial update */
#endif
  PL_Init();
  vTaskStartScheduler();
  for(;;) { /* should not get here */ }
}

/* overwrite assertion callback */
#include "McuLog.h"

void __assertion_failed(char *_Expr)  {
  McuLog_fatal(_Expr);
  McuLog_fatal("Assert failed!");
  __asm volatile("bkpt #0");
  for(;;) {
    __asm("nop");
  }
}

void __assert_func(const char *file, int line, const char *func, const char *expr) {
  McuLog_fatal("%s:%d %s() %s", file, line, func, expr);
  McuLog_fatal("Assert failed!");
  __asm volatile("bkpt #0");
  for(;;) {
    __asm("nop");
  }
}
