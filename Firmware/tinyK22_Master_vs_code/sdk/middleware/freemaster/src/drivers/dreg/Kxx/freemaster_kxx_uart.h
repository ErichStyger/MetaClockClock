/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2018-2019, 2024 NXP
 *
 * License: NXP LA_OPT_Online Code Hosting NXP_Software_License
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may
 * only be used strictly in accordance with the applicable license terms.
 * By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that
 * you have read, and that you agree to comply with and are bound by,
 * such license terms.  If you do not agree to be bound by the applicable
 * license terms, then you may not retain, install, activate or otherwise
 * use the software.
 *
 * FreeMASTER Communication Driver - UART low-level driver
 */

#ifndef __FREEMASTER_SERIAL_DREG_KXX_UART_H
#define __FREEMASTER_SERIAL_DREG_KXX_UART_H

/******************************************************************************
* Required header files include check
******************************************************************************/
#ifndef __FREEMASTER_H
#error Please include the freemaster.h master header file before the freemaster_serial_uart.h
#endif

/******************************************************************************
* Adapter configuration
******************************************************************************/

#ifdef __cplusplus
  extern "C" {
#endif

/******************************************************************************
* Types definition
******************************************************************************/

/** UART - Register Layout Typedef */
typedef struct {
    FMSTR_U8 BDH;                                /**< UART Baud Rate Registers: High, offset: 0x0 */
    FMSTR_U8 BDL;                                /**< UART Baud Rate Registers: Low, offset: 0x1 */
    FMSTR_U8 C1;                                 /**< UART Control Register 1, offset: 0x2 */
    FMSTR_U8 C2;                                 /**< UART Control Register 2, offset: 0x3 */
    FMSTR_U8 S1;                                 /**< UART Status Register 1, offset: 0x4 */
    FMSTR_U8 S2;                                 /**< UART Status Register 2, offset: 0x5 */
    FMSTR_U8 C3;                                 /**< UART Control Register 3, offset: 0x6 */
    FMSTR_U8 D;                                  /**< UART Data Register, offset: 0x7 */
    FMSTR_U8 MA1;                                /**< UART Match Address Registers 1, offset: 0x8 */
    FMSTR_U8 MA2;                                /**< UART Match Address Registers 2, offset: 0x9 */
    FMSTR_U8 C4;                                 /**< UART Control Register 4, offset: 0xA */
    FMSTR_U8 C5;                                 /**< UART Control Register 5, offset: 0xB */
    FMSTR_U8 ED;                                 /**< UART Extended Data Register, offset: 0xC */
    FMSTR_U8 MODEM;                              /**< UART Modem Register, offset: 0xD */
    FMSTR_U8 IR;                                 /**< UART Infrared Register, offset: 0xE */
    FMSTR_U8 RESERVED_0;
    FMSTR_U8 PFIFO;                              /**< UART FIFO Parameters, offset: 0x10 */
} FMSTR_UART_Type;

/*! C1_RSRC - Receiver Source Select
 *  0b0..Selects internal loop back mode. The receiver input is internally connected to transmitter output.
 *  0b1..Single wire UART mode where the receiver input is connected to the transmit pin input signal.
 */
#define FMSTR_UART_C1_RSRC_MASK                        (0x20U)

/*! C1_LOOPS - Loop Mode Select
 *  0b0..Normal operation.
 *  0b1..Loop mode where transmitter output is internally connected to receiver input. The receiver input is determined by RSRC.
 */
#define FMSTR_UART_C1_LOOPS_MASK                       (0x80U)

/*! C2_RE - Receiver Enable
 *  0b0..Receiver off.
 *  0b1..Receiver on.
 */
#define FMSTR_UART_C2_RE_MASK                          (0x4U)

/*! C2_TE - Transmitter Enable
 *  0b0..Transmitter off.
 *  0b1..Transmitter on.
 */
#define FMSTR_UART_C2_TE_MASK                          (0x8U)

/*! C2_RIE - Receiver Full Interrupt or DMA Transfer Enable
 *  0b0..RDRF interrupt and DMA transfer requests disabled.
 *  0b1..RDRF interrupt or DMA transfer requests enabled.
 */
#define FMSTR_UART_C2_RIE_MASK                         (0x20U)

/*! C2_TCIE - Transmission Complete Interrupt or DMA Transfer Enable
 *  0b0..TC interrupt and DMA transfer requests disabled.
 *  0b1..TC interrupt or DMA transfer requests enabled.
 */
#define FMSTR_UART_C2_TCIE_MASK                        (0x40U)

/*! C2_TIE - Transmitter Interrupt or DMA Transfer Enable.
 *  0b0..TDRE interrupt and DMA transfer requests disabled.
 *  0b1..TDRE interrupt or DMA transfer requests enabled.
 */
#define FMSTR_UART_C2_TIE_MASK                         (0x80U)

/*! S1_RDRF - Receive Data Register Full Flag
 *  0b0..The number of datawords in the receive buffer is less than the number indicated by RXWATER.
 *  0b1..The number of datawords in the receive buffer is equal to or greater than the number indicated by RXWATER at some point in time since this flag was last cleared.
 */
#define FMSTR_UART_S1_RDRF_MASK                        (0x20U)

/*! C3_TXDIR - Transmitter Pin Data Direction in Single-Wire mode
 *  0b0..TXD pin is an input in single wire mode.
 *  0b1..TXD pin is an output in single wire mode.
 */
#define FMSTR_UART_C3_TXDIR_MASK                       (0x20U)

/*! S1_TC - Transmit Complete Flag
 *  0b0..Transmitter active (sending data, a preamble, or a break).
 *  0b1..Transmitter idle (transmission activity complete).
 */
#define FMSTR_UART_S1_TC_MASK                          (0x40U)

/*! S1_TDRE - Transmit Data Register Empty Flag
 *  0b0..The amount of data in the transmit buffer is greater than the value indicated by TWFIFO[TXWATER].
 *  0b1..The amount of data in the transmit buffer is less than or equal to the value indicated by TWFIFO[TXWATER] at some point in time since the flag has been cleared.
 */
#define FMSTR_UART_S1_TDRE_MASK                        (0x80U)

/*! PFIFO_TXFE - Transmit FIFO Enable
 *  0b0..Transmit FIFO is not enabled. Buffer is depth 1. (Legacy support).
 *  0b1..Transmit FIFO is enabled. Buffer is depth indicated by TXFIFOSIZE.
 */
#define FMSTR_UART_PFIFO_TXFE_MASK                     (0x80U)

/*! PFIFO_RXFE - Receive FIFO Enable
 *  0b0..Receive FIFO is not enabled. Buffer is depth 1. (Legacy support)
 *  0b1..Receive FIFO is enabled. Buffer is depth indicted by RXFIFOSIZE.
 */
#define FMSTR_UART_PFIFO_RXFE_MASK                     (0x8U)


/******************************************************************************
* inline functions
******************************************************************************/

/******************************************************************************
* Global API functions
******************************************************************************/

void FMSTR_SerialSetBaseAddress(void *base);
void FMSTR_SerialIsr(void);

#ifdef __cplusplus
  }
#endif

#endif /* __FREEMASTER_SERIAL_DREG_KXX_UART_H */

