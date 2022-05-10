/*
 * Copyright (c) 2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_SHIFT_REGS
#include "shiftreg.h"
#include "spireg.h"
#include "McuWait.h"
#include "McuRTOS.h"
#include "McuLog.h"
#include "McuUtility.h"
#include "McuShell.h"
#include "stepper.h"

/* GPIO handles for latching shift registers */
static McuGPIO_Handle_t MotorLatch; /* to latch the motor signals, this pin does a active high pulse */
#if 0 /* not used */
static McuGPIO_Handle_t SensorLatch;  /* to latch the sensor signals, this pin does a active low pulse */
#endif
static SemaphoreHandle_t ShiftRegMutex = NULL; /* Mutex to protect SPI bus access */
static uint8_t MotorBitsByteToSend[SHIFTREG_NOF_MOTORBIT_BYTES]; /* array of bits stored to be sent */
static uint8_t prevMotorBitsByte[SHIFTREG_NOF_MOTORBIT_BYTES]; /* backup of what has been sent */

void ShiftReg_WriteMotorBits(const uint8_t *data, size_t nofBytes) {
  if (xSemaphoreTake(ShiftRegMutex, portMAX_DELAY)==pdTRUE) {
    SpiReg_WriteData(data, nofBytes);
    /* latch: idle state is LOW, make a HIGH pulse: */
    McuGPIO_SetHigh(MotorLatch);
    McuWait_Wait10Cycles();
    McuGPIO_SetLow(MotorLatch);
    xSemaphoreGive(ShiftRegMutex);
  }
}

#if 0 /* not used */
void ShiftReg_ReadSensorBits(uint8_t *data, size_t dataSize) {
  if (xSemaphoreTake(ShiftRegMutex, portMAX_DELAY)==pdTRUE) {
    /* latch: LOW pulse latches the data */
    McuGPIO_SetLow(SensorLatch);
    McuWait_Wait10Cycles();
    McuGPIO_SetHigh(SensorLatch); /* now read the data */

    SpiReg_ReadData(data, dataSize);
    xSemaphoreGive(ShiftRegMutex);
  }
}
#endif

void ShiftReg_StoreMotorBits(uint32_t motorIdx, const bool w[SHIFTREG_NOF_MOTOR_BITS]) { /* \TODO change to use 3bits instead of 4 */
  uint8_t data, clrMask;
  size_t byteIdx;

  /* transform truth table for motor wires/connections into bit sequence */
  if (motorIdx&1) { /* which nibble? */
    data = (w[0]<<3) + (w[1]<<2) + (w[2]<<1) + w[3];
    clrMask = 0xF0;
  } else {
    data = (w[0]<<7) + (w[1]<<6) + (w[2]<<5) + (w[3]<<4);
    clrMask = 0x0F;
  }
  /* One driver board:
   * MotorBitsByteToSend: [3]  [2]  [1]  [0]
   * MotorIdx             6,7  4,5  2,3  0,1
   *
   * Two driver board:
   * MotorBitsByteToSend: [7]  [6]  [5]  [4]  -> [3]   [2]   [1]  [0]
   * MotorIdx             6,7  4,5  2,3  0,1    14,15 12,13 10,11 8,9
   *
   * Three driver board:
   * MotorBitsByteToSend: [11] [10] [9]  [8]  -> [7]   [6]   [5]  [4]  -> [3]   [2]   [1]   [0]
   * MotorIdx             6,7  4,5  2,3  0,1    14,15 12,13 10,11 8,9    22,23 20,21 18,19 16,17
   */
  byteIdx = (SHIFTREG_NOF_MOTORBIT_BYTES-1) - ((motorIdx/8)*4) - ((7-(motorIdx%8))/2);
  MotorBitsByteToSend[byteIdx] &= clrMask; /* clear bits */
  MotorBitsByteToSend[byteIdx] |= data; /* store nibble */
}

void ShiftReg_SendStoredMotorBits(void) {
  ShiftReg_WriteMotorBits(MotorBitsByteToSend, sizeof(MotorBitsByteToSend));
  memcpy(prevMotorBitsByte, MotorBitsByteToSend, sizeof(MotorBitsByteToSend));
}

void ShiftReg_SendStoredMotorBitsIfChanged(void) {
  if (memcmp(MotorBitsByteToSend, prevMotorBitsByte, sizeof(MotorBitsByteToSend))!=0) { /* has it changed? */
    ShiftReg_WriteMotorBits(MotorBitsByteToSend, sizeof(MotorBitsByteToSend)); /* yes, send new data */
    memcpy(prevMotorBitsByte, MotorBitsByteToSend, sizeof(MotorBitsByteToSend));
  }
}

static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  unsigned char buf[64];

  McuShell_SendStatusStr((unsigned char*)"shiftreg", (unsigned char*)"ShiftReg status\r\n", io->stdOut);
  McuGPIO_GetPinStatusString(MotorLatch, buf, sizeof(buf));
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  motor", buf, io->stdOut);
#if 0 /* not used */
  McuGPIO_GetPinStatusString(SensorLatch, buf, sizeof(buf));
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
  McuShell_SendStatusStr((unsigned char*)"  sensor", buf, io->stdOut);
#endif
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"shiftreg", (unsigned char*)"Group of ShiftReg commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  set <motor> <val>", (unsigned char*)"Set motor value\r\n", io->stdOut);
#if 0 /* not used */
  McuShell_SendHelpStr((unsigned char*)"  readsensor", (unsigned char*)"Read sensor bits\r\n", io->stdOut);
#endif
  return ERR_OK;
}

uint8_t ShiftReg_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  const unsigned char *p;
  int32_t val, motorIdx;

  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "shiftreg help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "shiftreg status")==0)) {
    *handled = TRUE;
    return PrintStatus(io);
  } else if (McuUtility_strncmp((char*)cmd, "shiftreg set ", sizeof("shiftreg set ")-1)==0) {
    *handled = TRUE;
    p = cmd + sizeof("shiftreg set ")-1;
    if (McuUtility_xatoi(&p, &motorIdx)==ERR_OK && motorIdx<SHIFTREG_CONFIG_NOF_MOTORS) {
      if (McuUtility_xatoi(&p, &val)==ERR_OK && val<=0x3) {
        bool w[SHIFTREG_NOF_MOTOR_BITS];

        w[0] = (val&(1<<0))!=0;
        w[1] = (val&(1<<1))!=0;
        w[2] = (val&(1<<2))!=0;
        ShiftReg_StoreMotorBits(motorIdx, w);
        ShiftReg_SendStoredMotorBits();
        return ERR_OK;
      }
    }
    return ERR_FAILED;
#if 0 /* not used */
  } else if (McuUtility_strcmp((char*)cmd, "shiftreg readsensor")==0) {
    uint8_t sensorBytes[SHIFTREG_NOF_SENSORBIT_BYTES];
    uint8_t buf[16];

    *handled = TRUE;
    ShiftReg_ReadSensorBits(sensorBytes, sizeof(sensorBytes));
    for(int i=0;i<sizeof(sensorBytes); i++) {
      McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"0x");
      McuUtility_strcatNum8Hex(buf, sizeof(buf), sensorBytes[i]);
      McuUtility_chcat(buf, sizeof(buf), ' ');
      McuShell_SendStr(buf, io->stdOut);
    }
    McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
    return ERR_OK;
#endif
  }
  return ERR_OK;
}

void ShiftReg_Deinit(void) {
  MotorLatch = McuGPIO_DeinitGPIO(MotorLatch);
  vQueueDelete(ShiftRegMutex);
  ShiftRegMutex = NULL;
}

void ShiftReg_Init(void) {
  McuGPIO_Config_t config;

  SHIFTREG_CONFIG_LATCH_ENABLE_CLOCKS();
  McuGPIO_GetDefaultConfig(&config);
  config.isHighOnInit = false; /* high pulse latches the data */
  config.isInput = false;
  config.hw.gpio = SHIFTREG_CONFIG_MOTORLATCH_GPIO;
  config.hw.port = SHIFTREG_CONFIG_MOTORLATCH_PORT;
  config.hw.pin = SHIFTREG_CONFIG_MOTORLATCH_PIN;
  MotorLatch = McuGPIO_InitGPIO(&config);

#if 0 /* not used */
  McuGPIO_GetDefaultConfig(&config);
  config.isHighOnInit = true; /* low pulse latches the data */
  config.isInput = false;
  config.hw.gpio = SHIFTREG_CONFIG_SENSORLATCH_GPIO;
  config.hw.port = SHIFTREG_CONFIG_SENSORLATCH_PORT;
  config.hw.pin = SHIFTREG_CONFIG_SENSORLATCH_PIN;
  SensorLatch = McuGPIO_InitGPIO(&config);
#endif
  ShiftRegMutex = xSemaphoreCreateRecursiveMutex();
  if (ShiftRegMutex==NULL) { /* semaphore creation failed */
    McuLog_fatal("failed creating ShiftReg mutex");
    for(;;) {} /* error, not enough memory? */
  }
  vQueueAddToRegistry(ShiftRegMutex, "ShiftRegMutex");
}

#endif /* PL_CONFIG_USE_SHIFT_REGS */
