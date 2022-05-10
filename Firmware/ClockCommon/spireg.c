/*
 * Copyright (c) 2022, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform.h"
#if PL_CONFIG_USE_SHIFT_REGS
#include "spireg.h" /* own interface */
#include "fsl_dspi.h" /* SDK interface */

#define DSPI_MASTER_BASEADDR				SPI1
#define DSPI_MASTER_CLK_SRC					DSPI1_CLK_SRC
#define DSPI_MASTER_CLK_FREQ				CLOCK_GetFreq(DSPI1_CLK_SRC)

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

void SpiReg_DeInit(void) {
  DSPI_Deinit(DSPI_MASTER_BASEADDR);
}

void SpiReg_Init(void) {
  dspi_master_config_t masterConfig;

  DSPI_MasterGetDefaultConfig(&masterConfig);
  masterConfig.ctarConfig.baudRate = 1000000;
  masterConfig.ctarConfig.bitsPerFrame = 8;
  /* data get sampled on the raising edge */
  masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveHigh; /*!< CPOL=0. Active-high DSPI clock (idles low) */
  masterConfig.ctarConfig.cpha = kDSPI_ClockPhaseFirstEdge;  /*!< CPHA=0. Data is captured on the leading edge of the SCK and changed on the following edge.*/
  DSPI_MasterInit(DSPI_MASTER_BASEADDR, &masterConfig, DSPI_MASTER_CLK_FREQ);
}

#endif /* PL_CONFIG_USE_SHIFT_REGS */
