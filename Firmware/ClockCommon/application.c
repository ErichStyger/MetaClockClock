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
#define DEMO_MODE   (1)  /* demo mode after power-on */

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
#if 0 &&  PL_CONFIG_USE_LED_PIXEL
  MATRIX_SetLEDPixelColor(0, 0, 0, 0, 0, 0x10);
  MATRIX_SetLEDPixelColor(1, 0, 0, 0, 0x10, 0);
  MATRIX_SetLEDPixelColor(2, 0, 0, 0x10, 0, 0);
  MATRIX_SetLEDPixelColor(3, 0, 0, 0, 0x10, 0x10);
  MATRIX_SetLEDPixelColor(4, 0, 0, 0x10, 0, 0x10);
  MATRIX_SetLEDPixelColor(5, 0, 0, 0x5, 0x5, 0x10);
  MATRIX_SetLEDPixelColor(6, 0, 0, 0, 0x5, 0x5);
  MATRIX_SetLEDPixelColor(7, 0, 0, 0x5, 0, 0x10);
  NEO_TransferPixels();
#endif
  for(;;) {
#if PL_CONFIG_USE_NEO_PIXEL_HW
    res = xSemaphoreTake(semNeoUpdate, portMAX_DELAY); /* block until we get a request to update */
#if 1
    if (res==pdTRUE) { /* received the signal */
      NEO_ClearAllPixel();
   #if PL_CONFIG_USE_LED_RING
      MATRIX_IlluminateHands();
   #endif
      NEO_TransferPixels();
    }
#endif
#endif
    vTaskDelay(pdMS_TO_TICKS(20)); /* can wait 20 ms until a next update is needed */
  } /* for */
}
#endif /* PL_CONFIG_USE_NEO_PIXEL_HW */

#if configUSE_HEAP_SCHEME==5
  static __attribute__ ((used,section(".noinit.$SRAM_LOWER.Heap5"))) uint8_t heap_sram_lower[8*1024]; /* placed in in no_init section inside SRAM_LOWER */
  static __attribute__ ((used,section(".noinit_Heap5"))) uint8_t heap_sram_upper[1*1024]; /* placed in in no_init section inside SRAM_UPPER */

  static HeapRegion_t xHeapRegions[] =
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
void __assertion_failed(char *_Expr)  {
  McuLog_fatal(_Expr);
  McuLog_fatal("Assert failed!");
  __asm volatile("bkpt #0");
  for(;;) {
    __asm("nop");
  }
}
