/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * FreeMASTER Communication Driver - User Configuration File - EXAMPLE
 */

#ifndef __FREEMASTER_CFG_H
#define __FREEMASTER_CFG_H

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

// see freemaster.h for list of all supported platforms
#define FMSTR_PLATFORM_CORTEX_M 1    // Cortex-M platform
// #define FMSTR_PLATFORM_56F800E 0  // 56F800E platform
// #define FMSTR_PLATFORM_S12Z  0    // Use with S12Z platform
// #define FMSTR_PLATFORM_PA32  0    // Use with 32bit Power Architecture platforms

//! Enable/Disable FreeMASTER support as a whole
#define FMSTR_DISABLE           0   // To disable all FreeMASTER functionalities

//! Select interrupt or poll-driven serial communication
#define FMSTR_LONG_INTR         0   // Complete message processing in interrupt
#define FMSTR_SHORT_INTR        0   // Queuing done in interrupt
#define FMSTR_POLL_DRIVEN       1   // No interrupt needed, polling only

// List of standard FreeMASTER transports and their low-level drivers. See more options in src/drivers.
// FMSTR_SERIAL   -   Standard serial transport protocol (Used by various types of UART peripherals as USB CDC implementation)
//    FMSTR_SERIAL_MCUX_UART   -   MCUXSDK driver for UART peripheral
//    FMSTR_SERIAL_MCUX_LPUART -   MCUXSDK driver for LPUART peripheral
//    FMSTR_SERIAL_MCUX_USART  -   MCUXSDK driver for USART peripheral
//    FMSTR_SERIAL_MCUX_MINIUSART -MCUXSDK driver for MINIUSART peripheral
//    FMSTR_SERIAL_MCUX_QSCI   -   MCUXSDK driver for DSC QSCI peripheral
//    FMSTR_SERIAL_MCUX_USB    -   MCUXSDK driver for USB peripheral with CDC class
//    FMSTR_SERIAL_56F800E_EONCE - 56F800E-specific driver for JTAG EOnCE communication
// FMSTR_CAN      -   Standard CAN transport protocol (Used by various types of CAN peripherals)
//    FMSTR_CAN_MCUX_FLEXCAN   -   MCUXSDK driver for FlexCAN peripheral
//    FMSTR_CAN_MCUX_MCAN      -   MCUXSDK driver for MCAN peripheral
//    FMSTR_CAN_MCUX_MSCAN     -   MCUXSDK driver for msCAN peripheral
// FMSTR_NET      -   Network communication using TCP or UDP protocol
//    FMSTR_NET_LWIP_TCP       -   TCP using lwIP stack
//    FMSTR_NET_LWIP_UDP       -   UDP using lwIP stack
//    FMSTR_NET_SEGGER_RTT     -   SEGGER RTT using J-Link debugger
// FMSTR_PDBDM    -   Packet Driven BDM (direct memory access via JTAG, SWD or BDM debug probes). No low-level driver used.

//! Select communication interface
#define FMSTR_TRANSPORT         FMSTR_SERIAL           // Use serial transport layer
#define FMSTR_SERIAL_DRV        FMSTR_SERIAL_MCUX_UART // Use serial driver for UART

//! Define communication interface base address or leave undefined for runtime setting
// #undef FMSTR_SERIAL_BASE   // Serial base will be assigned in runtime (for FMSTR_SERIAL transport)
// #undef FMSTR_CAN_BASE      // CAN base will be assigned in runtime (for FMSTR_CAN transoprot)

//! FlexCAN-specific, communication message buffers
#define FMSTR_FLEXCAN_TXMB      0
#define FMSTR_FLEXCAN_RXMB      1

//! Input/output communication buffer size
#define FMSTR_COMM_BUFFER_SIZE  0   // Set to 0 for "automatic"

//! Receive FIFO queue size (use with FMSTR_SHORT_INTR only)
#define FMSTR_COMM_RQUEUE_SIZE  32  // Set to 0 for "default"

//! Support for Application Commands
#define FMSTR_USE_APPCMD        1  // Enable/disable App.Commands support
#define FMSTR_APPCMD_BUFF_SIZE  32  // App.Command data buffer size
#define FMSTR_MAX_APPCMD_CALLS  4   // How many app.cmd callbacks? (0=disable)

//! Oscilloscope support
#define FMSTR_USE_SCOPE         1   // Specify number of supported oscilloscopes
#define FMSTR_MAX_SCOPE_VARS    8   // Specify maximum number of scope variables per one oscilloscope

//! Recorder support
#define FMSTR_USE_RECORDER      1   // Specify number of supported recorders

//! Built-in recorder buffer
#define FMSTR_REC_BUFF_SIZE     1024    // Built-in buffer size of recorder #0. Set to 0 to use runtime settings.

//! Recorder time base, specifies how often the recorder is called in the user app.
#define FMSTR_REC_TIMEBASE      FMSTR_REC_BASE_MILLISEC(0)  // 0 = "unknown"
#define FMSTR_REC_FLOAT_TRIG    1   // Enable/disable floating point triggering

// Target-side address translation (TSA)
#define FMSTR_USE_TSA           1   // Enable TSA functionality
#define FMSTR_USE_TSA_INROM     1   // TSA tables declared as const (put to ROM)
#define FMSTR_USE_TSA_SAFETY    1   // Enable/Disable TSA memory protection
#define FMSTR_USE_TSA_DYNAMIC   1   // Enable/Disable TSA entries to be added also in runtime

// Pipes as data streaming over FreeMASTER protocol
#define FMSTR_USE_PIPES         3   // Specify number of supported pipe objects

// Enable/Disable read/write memory commands
#define FMSTR_USE_READMEM       1   // Enable read memory commands
#define FMSTR_USE_WRITEMEM      1   // Enable write memory commands
#define FMSTR_USE_WRITEMEMMASK  1   // Enable write memory bits commands

// Define password for access levels to protect them. AVOID SHORT PASSWORDS in production version.
// Passwords should be at least 20 characters long to prevent dictionary attacks.
// #define FMSTR_RESTRICTED_ACCESS_R_PASSWORD    "r"   // Read-only access level password. Let undefined when no password is required.
// #define FMSTR_RESTRICTED_ACCESS_RW_PASSWORD   "rw"  // Write access level password. Let undefined to set the same as for read-only access level.
// #define FMSTR_RESTRICTED_ACCESS_RWF_PASSWORD  "rwf" // Flash access level password. Let undefined to set the same as for write access level.

// Storing cleartext passwords in Flash memory is not safe, consider storing their SHA1 hash instead
// Even with this option, the hash must be generated from reasonably complex password to prevent dictionary attack.
#define FMSTR_USE_HASHED_PASSWORDS  0  // When non-zero, the passwords above are specified as a pointer to 20-byte SHA1 hash of password text

#endif /* __FREEMASTER_CFG_H */

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
