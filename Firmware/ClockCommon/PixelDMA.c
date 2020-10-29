/*
 * Copyright (c) 2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"

#if PL_CONFIG_USE_NEO_PIXEL_HW
#include <stdint.h>
#include <stddef.h>
#include "fsl_ftm.h"
#include "fsl_edma.h"
#include "fsl_dmamux.h"
#include "fsl_gpio.h"
#include "PixelDMA.h"
#include "McuTimeout.h"
#include "McuWait.h"
#include "FTM_Macros.h"
#include "DMA_Macros.h"
#include "DMAMUX_Macros.h"

/* WS2812(S) timing:
 * 0.35us for a 0
 * 0.90us for a 1
 * 1.25us Period
 * See https://mcuoneclipse.com/2015/08/05/tutorial-adafruit-ws2812b-neopixels-with-the-freescale-frdm-k64f-board-part-5-dma/
 * It uses the 8bit port of f the DMA_GPIO_PORT for the data, so 8bits in parallel.
 * */
#if PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN64 || PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN128
  /* values for a 36 MHz FTM0 clock and a 96 MHz core clock! */
  #define FTM_CH0_TICKS 0x0A /* go high, start bit */
  #define FTM_CH1_TICKS 0x1A /* data bit, go low for 0 after 0.35us, stay high for 1 */
  #define FTM_CH2_TICKS 0x33 /* end of 1 bit, go low after 0.9us */
  #define FTM_OVL_TICKS 0x3C /* end of 1.25 us cycle for a 0 or 1 bit */
#elif PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_MASTER_K22FN512
  /* values for a 60 MHz FTM0 clock and a 120 core clock! */
  #define FTM_CH0_TICKS 0x10 /* go high, start bit */
  #define FTM_CH1_TICKS 0x25 /* data bit, go low for 0 after 0.35us, stay high for 1 */
  #define FTM_CH2_TICKS 0x45 /* end of 1 bit, go low after 0.9us */
  #define FTM_OVL_TICKS 0x4A /* end of 1.25 us cycle for a 0 or 1 bit */
#endif

#define DMA_BASE_PTR      (DMA0) /*((DMA_MemMapPtr)0x40008000u)*/
#define DMAMUX_BASE_PTR   (DMAMUX)
#define FTM0_BASE_PTR     (FTM0)

/* GPIO port used */
#if PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN64 || PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_CLOCK_K02FN128
  #define DMA_GPIO_PORT     (GPIOC)
#elif PL_CONFIG_BOARD_ID==PL_CONFIG_BOARD_ID_MASTER_K22FN512
  #define DMA_GPIO_PORT     (GPIOD)
#endif

void FTM0_IRQHandler(void) {
  if ((FTM_PDD_GetOverflowInterruptFlag(FTM0_BASE_PTR)) != 0U) { /* Is the overflow interrupt flag pending? */
    FTM_PDD_ClearOverflowInterruptFlag(FTM0_BASE_PTR); /* Clear flag */
  }
  if (FTM_PDD_GetChannelInterruptFlag(FTM0_BASE_PTR, 0)) {
    FTM_PDD_ClearChannelInterruptFlag(FTM0_BASE_PTR, 0);
//    GPIOD_PDOR ^= (1<<1);
  }
  if (FTM_PDD_GetChannelInterruptFlag(FTM0_BASE_PTR, 1)) {
    FTM_PDD_ClearChannelInterruptFlag(FTM0_BASE_PTR, 1);
//    GPIOD_PDOR ^= (1<<2);
  }
  if (FTM_PDD_GetChannelInterruptFlag(FTM0_BASE_PTR, 2)) {
    FTM_PDD_ClearChannelInterruptFlag(FTM0_BASE_PTR, 2);
//    GPIOD_PDOR ^= (1<<3);
  }
  __DSB();
}

static void InitTimer(void) {
  CLOCK_EnableClock(kCLOCK_Ftm0);

  FTM_PDD_WriteStatusControlReg(FTM0_BASE_PTR, 0); /* init timer status and control register */

  FTM_PDD_SelectPrescalerSource(FTM0_BASE_PTR, FTM_PDD_DISABLED); /* disable timer */
  FTM_PDD_SetPrescaler(FTM0_BASE_PTR, FTM_PDD_DIVIDE_1);
  FTM_PDD_InitializeCounter(FTM0_BASE_PTR); /* reset timer counter */
  FTM_PDD_WriteModuloReg(FTM0_BASE_PTR, FTM_OVL_TICKS); /* set overflow to 1.25 us */

  FTM_PDD_WriteChannelControlReg(FTM0_BASE_PTR, 0, FTM_CnSC_MSA_MASK); /* channel control register: (MSB MSA) set to 01 (Output compare) */
  FTM_PDD_WriteChannelControlReg(FTM0_BASE_PTR, 1, FTM_CnSC_MSA_MASK); /* channel control register: (MSB MSA) set to 01 (Output compare) */
  FTM_PDD_WriteChannelControlReg(FTM0_BASE_PTR, 2, FTM_CnSC_MSA_MASK); /* channel control register: (MSB MSA) set to 01 (Output compare) */

  FTM_PDD_WriteChannelValueReg(FTM0_BASE_PTR, 0, FTM_CH0_TICKS); /* channel 0 match at 0.4 us */
  FTM_PDD_WriteChannelValueReg(FTM0_BASE_PTR, 1, FTM_CH1_TICKS); /* channel 1 match at 0.8 us */
  FTM_PDD_WriteChannelValueReg(FTM0_BASE_PTR, 2, FTM_CH2_TICKS); /* channel 2 match at 1.25 us */
#if 0 /* not needed */
  FTM_PDD_EnableOverflowInterrupt(FTM0_BASE_PTR);
#endif
  /* both the DMA and the Interrupt enable flag needs to be turned on to generate DMA transfer requests */
  FTM_PDD_EnableChannelInterrupt(FTM0_BASE_PTR, 0);
  FTM_PDD_EnableChannelInterrupt(FTM0_BASE_PTR, 1);
  FTM_PDD_EnableChannelInterrupt(FTM0_BASE_PTR, 2);
  /* make sure they have the highest priority */
  NVIC_SetPriority(FTM0_IRQn, 0);
}

static void StartTimer(void) {
  FTM_PDD_InitializeCounter(FTM0_BASE_PTR); /* reset timer counter */
  FTM_PDD_EnableChannelInterrupt(FTM0_BASE_PTR, 0);
  FTM_PDD_EnableChannelInterrupt(FTM0_BASE_PTR, 1);
  FTM_PDD_EnableChannelInterrupt(FTM0_BASE_PTR, 2);
  FTM_PDD_SelectPrescalerSource(FTM0_BASE_PTR, FTM_PDD_SYSTEM); /* enable timer so I can reset the value below */
}

static void StopTimer(void) {
  FTM_PDD_SelectPrescalerSource(FTM0_BASE_PTR, FTM_PDD_DISABLED); /* disable timer */
  FTM_PDD_DisableChannelInterrupt(FTM0_BASE_PTR, 0);
  FTM_PDD_DisableChannelInterrupt(FTM0_BASE_PTR, 1);
  FTM_PDD_DisableChannelInterrupt(FTM0_BASE_PTR, 2);
  FTM_PDD_ClearChannelInterruptFlag(FTM0_BASE_PTR, 0);
  FTM_PDD_ClearChannelInterruptFlag(FTM0_BASE_PTR, 1);
  FTM_PDD_ClearChannelInterruptFlag(FTM0_BASE_PTR, 2);
}

static void InitDMA(void) {
  /* init the TCDs */
  for (int i = 0; i<3; i++) {
    DMA_BASE_PTR->TCD[i].SADDR = 0;
    DMA_BASE_PTR->TCD[i].SOFF = 0;
    DMA_BASE_PTR->TCD[i].ATTR = 0;
    DMA_BASE_PTR->TCD[i].NBYTES_MLNO = 0;
    DMA_BASE_PTR->TCD[i].SLAST = 0;
    DMA_BASE_PTR->TCD[i].DADDR = 0;
    DMA_BASE_PTR->TCD[i].DOFF = 0;
    DMA_BASE_PTR->TCD[i].CITER_ELINKNO = 1;
    DMA_BASE_PTR->TCD[i].DLAST_SGA = 0;
    DMA_BASE_PTR->TCD[i].CSR = 0;
    DMA_BASE_PTR->TCD[i].BITER_ELINKNO = 1;
  }
  /* enable minor loop mapping. CITER and BITER will set the number of requests (which are the number of bytes) */
  DMA_PDD_EnableMinorLoopMapping(DMA_BASE_PTR, PDD_ENABLE);
  /* one byte for each transfer request: */
  DMA_PDD_SetByteCount32(DMA_BASE_PTR, DMA_PDD_CHANNEL_0, 1); /* set number of bytes for each transfer request */
  DMA_PDD_SetByteCount32(DMA_BASE_PTR, DMA_PDD_CHANNEL_1, 1); /* set number of bytes for each transfer request  */
  DMA_PDD_SetByteCount32(DMA_BASE_PTR, DMA_PDD_CHANNEL_2, 1); /* set number of bytes for each transfer request  */

  /* setup address modulo: we are not using it as we stream out the data once and then latch it */
  DMA_PDD_SetSourceAddressModulo(DMA_BASE_PTR, DMA_PDD_CHANNEL_0, DMA_PDD_CIRCULAR_BUFFER_DISABLED); /* circular buffer disabled */
  DMA_PDD_SetSourceAddressModulo(DMA_BASE_PTR, DMA_PDD_CHANNEL_1, DMA_PDD_CIRCULAR_BUFFER_DISABLED); /* circular buffer disabled */
  DMA_PDD_SetSourceAddressModulo(DMA_BASE_PTR, DMA_PDD_CHANNEL_2, DMA_PDD_CIRCULAR_BUFFER_DISABLED); /* circular buffer disabled */
  /* the 'set all bits' and 'clear all bits' DMA events will use a single value, so no address increment.
   * But for the data we will increment the source address counter
   */
  DMA_PDD_SetSourceAddressOffset(DMA_BASE_PTR, DMA_PDD_CHANNEL_0, 0); /* zero address increment */
  DMA_PDD_SetSourceAddressOffset(DMA_BASE_PTR, DMA_PDD_CHANNEL_1, 1); /* increment by 1 */
  DMA_PDD_SetSourceAddressOffset(DMA_BASE_PTR, DMA_PDD_CHANNEL_2, 0); /* zero address increment */
  /* we transfer one byte every time */
  DMA_PDD_SetSourceDataTransferSize(DMA_BASE_PTR, DMA_PDD_CHANNEL_0, DMA_PDD_8_BIT); /* Transfer size from source is 8bit */
  DMA_PDD_SetSourceDataTransferSize(DMA_BASE_PTR, DMA_PDD_CHANNEL_1, DMA_PDD_8_BIT); /* Transfer size from source is 8bit */
  DMA_PDD_SetSourceDataTransferSize(DMA_BASE_PTR, DMA_PDD_CHANNEL_2, DMA_PDD_8_BIT); /* Transfer size from source is 8bit */
  /* set up destination address:
   * PSOR (Port Set Output Register) will use 0xff to set the bits
   * PDOR (Port Data Output Register) will use the data
   * PDCR (Port Data Clear Register) will use 0xff to clear the bits
   */
  CLOCK_EnableClock(kCLOCK_PortD);
  DMA_PDD_SetDestinationAddress(DMA_BASE_PTR, DMA_PDD_CHANNEL_0, (uint32_t)&DMA_GPIO_PORT->PSOR); /* set destination address: address of PTD Set Output register */
  DMA_PDD_SetDestinationAddress(DMA_BASE_PTR, DMA_PDD_CHANNEL_1, (uint32_t)&DMA_GPIO_PORT->PDOR); /* set destination address: address of PTD Data Output register */
  DMA_PDD_SetDestinationAddress(DMA_BASE_PTR, DMA_PDD_CHANNEL_2, (uint32_t)&DMA_GPIO_PORT->PCOR); /* set destination address: address of PTD Clear Output register */
  /* no destination address buffer module: we will stream data only once */
  DMA_PDD_SetDestinationAddressModulo(DMA_BASE_PTR, DMA_PDD_CHANNEL_0, DMA_PDD_CIRCULAR_BUFFER_DISABLED); /* no circular buffer */
  DMA_PDD_SetDestinationAddressModulo(DMA_BASE_PTR, DMA_PDD_CHANNEL_1, DMA_PDD_CIRCULAR_BUFFER_DISABLED); /* no circular buffer */
  DMA_PDD_SetDestinationAddressModulo(DMA_BASE_PTR, DMA_PDD_CHANNEL_2, DMA_PDD_CIRCULAR_BUFFER_DISABLED); /* no circular buffer */
  /* no destination address increments needed */

  DMA_PDD_SetDestinationAddressOffset(DMA_BASE_PTR, DMA_PDD_CHANNEL_0, 0);
  DMA_PDD_SetDestinationAddressOffset(DMA_BASE_PTR, DMA_PDD_CHANNEL_1, 0);
  DMA_PDD_SetDestinationAddressOffset(DMA_BASE_PTR, DMA_PDD_CHANNEL_2, 0);
  /* we are transferring 1 byte of data */
  DMA_PDD_SetDestinationDataTransferSize(DMA_BASE_PTR, DMA_PDD_CHANNEL_0, DMA_PDD_8_BIT); /* Transfer to destination size is 16bit */
  DMA_PDD_SetDestinationDataTransferSize(DMA_BASE_PTR, DMA_PDD_CHANNEL_1, DMA_PDD_8_BIT); /* Transfer to destination size is 16bit */
  DMA_PDD_SetDestinationDataTransferSize(DMA_BASE_PTR, DMA_PDD_CHANNEL_2, DMA_PDD_8_BIT); /* Transfer to destination size is 16bit */
  /* at the and of the DMA sequence, disable DMA */
  DMA_PDD_EnableRequestAutoDisable(DMA_BASE_PTR, DMA_PDD_CHANNEL_0, PDD_ENABLE); /* disable DMA request at the end of the sequence */
  DMA_PDD_EnableRequestAutoDisable(DMA_BASE_PTR, DMA_PDD_CHANNEL_1, PDD_ENABLE); /* disable DMA request at the end of the sequence */
  DMA_PDD_EnableRequestAutoDisable(DMA_BASE_PTR, DMA_PDD_CHANNEL_2, PDD_ENABLE); /* disable DMA request at the end of the sequence */

  /* do the DMA Muxing */
  DMAMUX_Init(DMAMUX_BASE_PTR);
  DMAMUX_PDD_EnableChannel(DMAMUX_BASE_PTR, 0, PDD_DISABLE);
  DMAMUX_PDD_EnableChannel(DMAMUX_BASE_PTR, 1, PDD_DISABLE);
  DMAMUX_PDD_EnableChannel(DMAMUX_BASE_PTR, 2, PDD_DISABLE);
  DMAMUX_PDD_EnableTrigger(DMAMUX_BASE_PTR, 0, PDD_DISABLE);
  DMAMUX_PDD_EnableTrigger(DMAMUX_BASE_PTR, 1, PDD_DISABLE);
  DMAMUX_PDD_EnableTrigger(DMAMUX_BASE_PTR, 2, PDD_DISABLE);

  DMAMUX_PDD_SetChannelSource(DMAMUX_BASE_PTR, 0, 20); /* kDmaRequestMux0FTM0Channel0 */
  DMAMUX_PDD_SetChannelSource(DMAMUX_BASE_PTR, 1, 21); /* kDmaRequestMux0FTM0Channel1 */
  DMAMUX_PDD_SetChannelSource(DMAMUX_BASE_PTR, 2, 22); /* kDmaRequestMux0FTM0Channel2 */

  DMAMUX_PDD_EnableChannel(DMAMUX_BASE_PTR, 0, PDD_ENABLE);
  DMAMUX_PDD_EnableChannel(DMAMUX_BASE_PTR, 1, PDD_ENABLE);
  DMAMUX_PDD_EnableChannel(DMAMUX_BASE_PTR, 2, PDD_ENABLE);
#if 0
  DMA_PDD_EnableErrorInterrupt(DMAMUX_BASE_PTR, 0);
  DMA_PDD_EnableErrorInterrupt(DMAMUX_BASE_PTR, 1);
  DMA_PDD_EnableErrorInterrupt(DMAMUX_BASE_PTR, 2);

  DMA_PDD_EnableTransferCompleteInterrupt(DMAMUX_BASE_PTR, 0, PDD_ENABLE);
  DMA_PDD_EnableTransferCompleteInterrupt(DMAMUX_BASE_PTR, 1, PDD_ENABLE);
  DMA_PDD_EnableTransferCompleteInterrupt(DMAMUX_BASE_PTR, 2, PDD_ENABLE);
#endif
}

uint8_t PIXDMA_Transfer(uint32_t dataAddress, size_t nofBytes) {
  static const uint8_t OneValue = 0xFF; /* value to clear or set the port bits */
  McuTimeout_CounterHandle handle;
  bool isTimeout;
  uint32_t done0, done1, done2;

  /* clear any pending done flags for DMA channels */
  DMA_PDD_ClearDoneFlag(DMA_BASE_PTR, DMA_PDD_CHANNEL_0);
  DMA_PDD_ClearDoneFlag(DMA_BASE_PTR, DMA_PDD_CHANNEL_1);
  DMA_PDD_ClearDoneFlag(DMA_BASE_PTR, DMA_PDD_CHANNEL_2);

  /* set DMA source addresses */
  DMA_PDD_SetSourceAddress(DMA_BASE_PTR, DMA_PDD_CHANNEL_0, (uint32_t)&OneValue); /* set source address */
  DMA_PDD_SetSourceAddress(DMA_BASE_PTR, DMA_PDD_CHANNEL_1, dataAddress); /* set source address */
  DMA_PDD_SetSourceAddress(DMA_BASE_PTR, DMA_PDD_CHANNEL_2, (uint32_t)&OneValue); /* set source address */

  /* set byte count addresses */
  DMA_PDD_SetCurrentMajorLoopCount15(DMA_BASE_PTR, DMA_PDD_CHANNEL_0, nofBytes);
  DMA_PDD_SetCurrentMajorLoopCount15(DMA_BASE_PTR, DMA_PDD_CHANNEL_1, nofBytes);
  DMA_PDD_SetCurrentMajorLoopCount15(DMA_BASE_PTR, DMA_PDD_CHANNEL_2, nofBytes);

  DMA_PDD_SetBeginningMajorLoopCount15(DMA_BASE_PTR, DMA_PDD_CHANNEL_0, nofBytes);
  DMA_PDD_SetBeginningMajorLoopCount15(DMA_BASE_PTR, DMA_PDD_CHANNEL_1, nofBytes);
  DMA_PDD_SetBeginningMajorLoopCount15(DMA_BASE_PTR, DMA_PDD_CHANNEL_2, nofBytes);

  /* reset TPM counter */
  FTM_PDD_InitializeCounter(FTM0_BASE_PTR); /* reset timer counter */
  FTM_PDD_ClearChannelFlags(FTM0_BASE_PTR, 0x00);
  FTM_PDD_ClearOverflowInterruptFlag(FTM0_BASE_PTR);
  /* re-enable DMA Muxing: it will disabled at the end of the transfer */
  DMAMUX_PDD_EnableChannel(DMAMUX_BASE_PTR, 0, PDD_ENABLE);
  DMAMUX_PDD_EnableChannel(DMAMUX_BASE_PTR, 1, PDD_ENABLE);
  DMAMUX_PDD_EnableChannel(DMAMUX_BASE_PTR, 2, PDD_ENABLE);
  /* enable DMA peripheral requests */
  DMA_PDD_EnablePeripheralRequest(DMA_BASE_PTR, DMA_PDD_CHANNEL_2, PDD_ENABLE); /* enable request from peripheral */
  DMA_PDD_EnablePeripheralRequest(DMA_BASE_PTR, DMA_PDD_CHANNEL_1, PDD_ENABLE); /* enable request from peripheral */
  DMA_PDD_EnablePeripheralRequest(DMA_BASE_PTR, DMA_PDD_CHANNEL_0, PDD_ENABLE); /* enable request from peripheral */
  /* clear timer flags and status so it starts from a clean starting point */
  FTM_PDD_ClearChannelFlags(FTM0_BASE_PTR, 0x00);
  FTM_PDD_ClearOverflowInterruptFlag(FTM0_BASE_PTR);
  /* enable FTM DMA */
#if 0
  FTM_PDD_WriteStatusControlReg(FTM0_BASE_PTR, FTM_PDD_ReadStatusControlReg(FTM0_BASE_PTR)|FTM_SC_DMA_MASK);
#endif

  /* enable DMA request in the FTM timer  channels */
  FTM_PDD_EnableChannelDma(FTM0_BASE_PTR, 0);
  FTM_PDD_EnableChannelDma(FTM0_BASE_PTR, 1);
  FTM_PDD_EnableChannelDma(FTM0_BASE_PTR, 2);

#if 0 /* debug mode: triggering manual DMA requests to verify functionality */
  int cntr = 0;
  for (;;) {
    if (cntr==10) {
      DMA_PDD_StartTransfer(DMA_BASE_PTR, DMA_PDD_ONE_CHANNEL, 0);
    }
    done0 = DMA_PDD_GetDoneFlag(DMA_BASE_PTR, DMA_PDD_CHANNEL_0);
    if (done0) {
      DMA_PDD_ClearDoneFlag(DMA_BASE_PTR, 0);
      DMA_PDD_StartTransfer(DMA_BASE_PTR, DMA_PDD_ONE_CHANNEL, 1);
    }
    done1 = DMA_PDD_GetDoneFlag(DMA_BASE_PTR, DMA_PDD_CHANNEL_1);
    if (done1) {
      DMA_PDD_ClearDoneFlag(DMA_BASE_PTR, 1);
      DMA_PDD_StartTransfer(DMA_BASE_PTR, DMA_PDD_ONE_CHANNEL, 2);
    }
    done2 = DMA_PDD_GetDoneFlag(DMA_BASE_PTR, DMA_PDD_CHANNEL_2);
    if (done2) {
      DMA_PDD_ClearDoneFlag(DMA_BASE_PTR, 2);
      cntr = 0;
    }
    McuWait_WaitOSms(10); /* give back some time */
    cntr++;
  }
#endif

  /* start the TPM timer */
  StartTimer();

  isTimeout = FALSE;
  handle = McuTimeout_GetCounter(100/McuTimeout_TICK_PERIOD_MS);
  for(;;) {
    /* wait until transfer is complete */
    if (McuTimeout_CounterExpired(handle)) {
      isTimeout = TRUE;
      break; /* leave loop */
    }
    done0 = DMA_PDD_GetDoneFlag(DMA_BASE_PTR, DMA_PDD_CHANNEL_0);
    done1 = DMA_PDD_GetDoneFlag(DMA_BASE_PTR, DMA_PDD_CHANNEL_1);
    done2 = DMA_PDD_GetDoneFlag(DMA_BASE_PTR, DMA_PDD_CHANNEL_2);
    if (done0 && done1 && done2) {
      break; /* done! */
    }
    vTaskDelay(pdMS_TO_TICKS(1)); /* give back some time */
  }
  StopTimer(); /* stop TPM */
  McuTimeout_LeaveCounter(handle);
  McuWait_Waitus(50); /* latch, low for at least 50 us (40x1.25us) */

  /* disable DMA-Muxing: necessary, otherwise DMA events on FTM0 channel 0 might be still latched.
   * Will enable muxing for next transfer */
  DMAMUX_PDD_EnableChannel(DMAMUX_BASE_PTR, 0, PDD_DISABLE);
  DMAMUX_PDD_EnableChannel(DMAMUX_BASE_PTR, 1, PDD_DISABLE);
  DMAMUX_PDD_EnableChannel(DMAMUX_BASE_PTR, 2, PDD_DISABLE);
#if 0
  /* disable peripheral DMA */
  FTM_PDD_WriteStatusControlReg(FTM0_BASE_PTR, FTM_PDD_ReadStatusControlReg(FTM0_BASE_PTR)&(~FTM_SC_DMA_MASK));
#endif
  FTM_PDD_DisableChannelDma(FTM0_BASE_PTR, 0);
  FTM_PDD_DisableChannelDma(FTM0_BASE_PTR, 1);
  FTM_PDD_DisableChannelDma(FTM0_BASE_PTR, 2);
  if (isTimeout) {
    return ERR_BUSY;
  }
  return ERR_OK;
}

void PIXDMA_Init(void) {
  InitTimer(); /* timer setup */
  InitDMA();
}
#endif /* PL_CONFIG_USE_NEO_PIXEL_HW */
