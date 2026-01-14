/*
 * Copyright 2019-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * FreeMASTER Communication Driver - User Configuration File
 */

#ifndef __FREEMASTER_CFG_H
#define __FREEMASTER_CFG_H

/* This is a generic configuration file of the FreeMASTER driver. 
 * You need to edit this file before using FreeMASTER in this application.
 * 
 * RECOMMENDED: Use the MCUXpresso ConfigTools to configure FreeMASTER driver graphically and 
 * to generate this file automatically. Refer to 'Middleware' configuration components in 
 * the Peripheral tool. The ConfigTools will also help you to configure the physical communication 
 * interface, device pins and clocks.
 * 
 * EDIT MANUALLY: Alternatively, you may edit this file manually. See options marked as 'TODO:' 
 * below. The FMSTR_TRANSPORT and related low-level communication driver options must be set. 
 * Also ensure that the configuration, device pins and clocks are initialized by the application.
 * 
 * ALWAYS: Call FMSTR_Init() in your application code and call FMSTR_Poll() in the application loop. 
 * 
 * SEE MORE INFORMATION: in fmstr_uart, fmstr_can and other example applications. Reach out to 
 * FreeMASTER community at https://community.nxp.com/t5/FreeMASTER/bd-p/freemaster to get more support.
 * 
 * Remove the warning statement below after this file is edited. 
 */
#warning This freemaster_cfg.h file needs to be edited to configure low-level communication.
#define FMSTR_DISABLE 1 // TODO: set to 0 after this configuration is ready to be used

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#define FMSTR_PLATFORM_56F800E 1 /* 56F800E platform (see freemaster.h for list of all supported platforms) */

// Set the demo application configuration
#define FMSTR_DEMO_ENOUGH_ROM  1 /* Platform has enough ROM to show most of the FreeMASTER features */
#define FMSTR_DEMO_LARGE_ROM   1 /* ROM is large enough to store the extended data structures used in FreeMASTER demo */
#define FMSTR_DEMO_SUPPORT_I64 1 /* support for long long type */
#define FMSTR_DEMO_SUPPORT_FLT 1 /* support for float type */
#define FMSTR_DEMO_SUPPORT_DBL 1 /* support for double type */

// Select interrupt or poll-driven serial communication
#define FMSTR_LONG_INTR   0 // Complete message processing in interrupt
#define FMSTR_SHORT_INTR  0 // Queuing done in interrupt
#define FMSTR_POLL_DRIVEN 1 // No interrupt needed, polling only

// List of standard FreeMASTER transports and their low-level drivers. See more options in src/drivers.
// FMSTR_SERIAL   -   Standard serial transport protocol (Used by various types of UART peripherals as USB CDC implementation)
//    FMSTR_SERIAL_MCUX_QSCI   -   MCUXSDK driver for DSC QSCI peripheral
//    FMSTR_SERIAL_MCUX_USB    -   MCUXSDK driver for USB peripheral with CDC class
//    FMSTR_SERIAL_56F800E_EONCE - 56F800E-specific driver for JTAG EOnCE communication
// FMSTR_CAN      -   Standard CAN transport protocol (Used by various types of CAN peripherals)
//    FMSTR_CAN_MCUX_DSCFLEXCAN -   MCUXSDK driver for DSC FlexCAN peripheral
//    FMSTR_CAN_MCUX_DSCMSCAN   -   MCUXSDK driver for DSC msCAN peripheral

// Select communication interface
#define FMSTR_TRANSPORT  FMSTR_SERIAL // TODO: select FMSTR_SERIAL or FMSTR_CAN transport interface
#define FMSTR_SERIAL_DRV // TODO: when using FMSTR_SERIAL: select Serial low-level communication driver
#define FMSTR_CAN_DRV    // TODO: when using FMSTR_CAN: select CAN low-level communication driver

// Define communication interface base address or leave undefined for runtime setting
#undef FMSTR_SERIAL_BASE // Serial base will be assigned in runtime (for FMSTR_SERIAL transport)
#undef FMSTR_CAN_BASE    // CAN base will be assigned in runtime (for FMSTR_CAN transoprot)

// FlexCAN-specific, communication message buffers
#define FMSTR_FLEXCAN_TXMB 0
#define FMSTR_FLEXCAN_RXMB 1

// SERIAL communication options
#define FMSTR_COMM_RQUEUE_SIZE 32 // Receive FIFO queue size - FMSTR_SHORT_INTR mode

// Input/output communication buffer size
#define FMSTR_COMM_BUFFER_SIZE 0 // Set to 0 for "automatic"

// Support for Application Commands
#define FMSTR_USE_APPCMD       1  // Enable/disable App.Commands support
#define FMSTR_APPCMD_BUFF_SIZE 32 // App.Command data buffer size
#define FMSTR_MAX_APPCMD_CALLS 4  // How many app.cmd callbacks? (0=disable)

// Oscilloscope support
#define FMSTR_USE_SCOPE      1 // Specify number of supported oscilloscopes
#define FMSTR_MAX_SCOPE_VARS 8 // Specify maximum number of scope variables per one oscilloscope

// Recorder support
#define FMSTR_USE_RECORDER 1 // Specify number of supported recorders

// Built-in recorder buffer
#define FMSTR_REC_BUFF_SIZE 1024 // Built-in buffer size of recorder #0. Set to 0 to use runtime settings.

// Recorder time base, specifies how often the recorder is called in the user app.
#define FMSTR_REC_TIMEBASE   FMSTR_REC_BASE_MILLISEC(0) // 0 = "unknown"
#define FMSTR_REC_FLOAT_TRIG 0                          // Enable/disable floating point triggering

// Target-side address translation (TSA)
#define FMSTR_USE_TSA         1 // Enable TSA functionality
#define FMSTR_USE_TSA_INROM   0 // TSA tables declared as const (put to ROM)
#define FMSTR_USE_TSA_SAFETY  1 // Enable/Disable TSA memory protection
#define FMSTR_USE_TSA_DYNAMIC 1 // Enable/Disable TSA entries to be added also in runtime

// Pipes as data streaming over FreeMASTER protocol
#define FMSTR_USE_PIPES 1 // Specify number of supported pipe objects

// Enable/Disable read/write memory commands
#define FMSTR_USE_READMEM      1 // Enable read memory commands
#define FMSTR_USE_WRITEMEM     1 // Enable write memory commands
#define FMSTR_USE_WRITEMEMMASK 1 // Enable write memory bits commands

// Define password for access levels to protect them. AVOID SHORT PASSWORDS in production version.
// Passwords should be at least 20 characters long to prevent dictionary attacks.

#if 0
#define FMSTR_RESTRICTED_ACCESS_R_PASSWORD    "r"   // Read-only access level password.
#define FMSTR_RESTRICTED_ACCESS_RW_PASSWORD   "rw"  // Write access level password. Let undefined to set the same as for read-only access level. 
#define FMSTR_RESTRICTED_ACCESS_RWF_PASSWORD  "rwf" // Flash access level password. Let undefined to set the same as for write access level.
#endif

// Storing cleartext passwords in Flash memory is not safe, consider storing their SHA1 hash instead
// Even with this option, the hash must be generated from reasonably complex password to prevent dictionary attack.
#define FMSTR_USE_HASHED_PASSWORDS 0 // When non-zero, the passwords above are specified as a pointer to 20-byte SHA1 hash of password text

#endif /* __FREEMASTER_CFG_H */

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
