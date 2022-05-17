/*
 * Copyright (c) 2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_SHIFT_REGS
#include "spireg.h" /* own interface */
#include "fsl_dspi.h" /* SDK interface */
#include "fsl_port.h"

#define DSPI_SPI_MASTER_PORT     (0) /* 0: SPI0 (moving pixels), 1: SP1 (splitflap) */

#if DSPI_SPI_MASTER_PORT==0
  #define DSPI_MASTER_BASEADDR        SPI0
  #define DSPI_MASTER_CLK_SRC         DSPI0_CLK_SRC
  #define DSPI_MASTER_CLK_FREQ        CLOCK_GetFreq(DSPI0_CLK_SRC)
#elif DSPI_SPI_MASTER_PORT==1
  #define DSPI_MASTER_BASEADDR				SPI1
  #define DSPI_MASTER_CLK_SRC					DSPI1_CLK_SRC
  #define DSPI_MASTER_CLK_FREQ				CLOCK_GetFreq(DSPI1_CLK_SRC)
#endif

void SpiReg_WriteData(const uint8_t *data, size_t size) {
  dspi_transfer_t masterTransfer;

  masterTransfer.txData      = (uint8_t *)data;
  masterTransfer.rxData      = NULL;
  masterTransfer.dataSize    = size;
  masterTransfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 | kDSPI_MasterPcsContinuous;
  DSPI_MasterTransferBlocking(DSPI_MASTER_BASEADDR, &masterTransfer);
}

void SpiReg_ReadData(uint8_t *data, size_t size) {
  dspi_transfer_t masterTransfer;

	masterTransfer.txData      = NULL;
	masterTransfer.rxData      = data;
	masterTransfer.dataSize    = size;
	masterTransfer.configFlags = kDSPI_MasterCtar0 | kDSPI_MasterPcs0 | kDSPI_MasterPcsContinuous;
  DSPI_MasterTransferBlocking(DSPI_MASTER_BASEADDR, &masterTransfer);
}

static void SpiReg_Mux(void) {
#if DSPI_SPI_MASTER_PORT==0 /* SPI0: PTD1, PTD2, PTD3 */
  /* PORTD1 (pin 58) is configured as SPI0_SCK */
  PORT_SetPinMux(PORTD, 1U, kPORT_MuxAlt2);

  /* PORTD2 (pin 59) is configured as SPI0_SOUT */
  PORT_SetPinMux(PORTD, 2U, kPORT_MuxAlt2);

  /* PORTD3 (pin 60) is configured as SPI0_SIN */
  PORT_SetPinMux(PORTD, 3U, kPORT_MuxAlt2);
#elif DSPI_SPI_MASTER_PORT==1 /* SPI1: PTD5, PTD6, PTD7 */
  /* PORTD5 (pin 62) is configured as SPI1_SCK */
  PORT_SetPinMux(PORTD, 5U, kPORT_MuxAlt7);

  /* PORTD6 (pin 63) is configured as SPI1_SOUT */
  PORT_SetPinMux(PORTD, 6U, kPORT_MuxAlt7);

  /* PORTD7 (pin 64) is configured as SPI1_SIN */
  PORT_SetPinMux(PORTD, 7U, kPORT_MuxAlt7);
#endif
}

void SpiReg_DeInit(void) {
  DSPI_Deinit(DSPI_MASTER_BASEADDR);
}

void SpiReg_Init(void) {
  dspi_master_config_t masterConfig;

  SpiReg_Mux();
  DSPI_MasterGetDefaultConfig(&masterConfig);
  masterConfig.ctarConfig.baudRate = 1000000;
  masterConfig.ctarConfig.bitsPerFrame = 8;
  /* data get sampled on the raising edge */
  masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveHigh; /*!< CPOL=0. Active-high DSPI clock (idles low) */
  masterConfig.ctarConfig.cpha = kDSPI_ClockPhaseFirstEdge;  /*!< CPHA=0. Data is captured on the leading edge of the SCK and changed on the following edge.*/
  DSPI_MasterInit(DSPI_MASTER_BASEADDR, &masterConfig, DSPI_MASTER_CLK_FREQ);
}

#endif /* PL_CONFIG_USE_SHIFT_REGS */
