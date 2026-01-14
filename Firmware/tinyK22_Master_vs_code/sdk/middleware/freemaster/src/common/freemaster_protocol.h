/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2018-2021, 2024 NXP
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
 * FreeMASTER Communication Driver - protocol declarations
 */

#ifndef __FREEMASTER_PROTOCOL_H
#define __FREEMASTER_PROTOCOL_H

/*-------------------------------------
  command message - standard commands
  -------------------------------------*/
#define FMSTR_CMD_GETCONFIG     0x20U
#define FMSTR_CMD_READMEM       0x21U
#define FMSTR_CMD_READMEM_BA    0x22U
#define FMSTR_CMD_WRITEMEM      0x23U
#define FMSTR_CMD_SETREC        0x24U /* Setup the recorder */
#define FMSTR_CMD_GETREC        0x25U /* Get the recorder status/data etc. */
#define FMSTR_CMD_SETSCOPE      0x26U /* Configure Scope */
#define FMSTR_CMD_READSCOPE     0x27U /* Read the scope data */
#define FMSTR_CMD_PIPE          0x28U
#define FMSTR_CMD_GETTSAINFO    0x29U
#define FMSTR_CMD_GETSTRLEN     0x2AU
#define FMSTR_CMD_AUTH1         0x2CU
#define FMSTR_CMD_AUTH2         0x2DU
#define FMSTR_CMD_URESRWI       0x2EU /* User Resource Read/Write/IOctl */
#define FMSTR_CMD_GETPIPE       0x2FU /* Get information about pipe */
#define FMSTR_CMD_SENDAPPCMD    0x30U /* send the application command */
#define FMSTR_CMD_GETAPPCMDSTS  0x31U /* get the application command status */
#define FMSTR_CMD_GETAPPCMDDATA 0x32U /* get the application command data */
#define FMSTR_CMD_FEATLOCK      0x33U /* Lock or unlock feature in a multi-session configuration */

/* special transport-specific commands */
#define FMSTR_CANSPC_PING       0xC0U
#define FMSTR_NET_PING          0xC0U
#define FMSTR_NET_DISCOVERY     0xC1U

/*-------------------------------------
  response message - status byte
  -------------------------------------*/

/* flags in response codes */
#define FMSTR_STSF_ERROR  0x80U /* FLAG: error answer (no response data) */
#define FMSTR_STSF_VARLEN 0x40U /* FLAG: variable-length answer (length byte) */
#define FMSTR_STSF_EVENT  0x20U /* FLAG: reserved */

#define FMSTR_STS_INVALID 0xFFU /* Invalid status code - just for internal use, not used in communication */

/* confirmation codes */
#define FMSTR_STS_OK 0x00U /* operation finished successfully */

/* error codes */
#define FMSTR_STC_INVCMD        0x81U /* unknown command code */
#define FMSTR_STC_CMDCSERR      0x82U /* command checksum error */
#define FMSTR_STC_CMDTOOLONG    0x83U /* command is too long */
#define FMSTR_STC_RSPBUFFOVF    0x84U /* the response would not fit into transmit buffer */
#define FMSTR_STC_INVBUFF       0x85U /* invalid buffer length or operation */
#define FMSTR_STC_INVSIZE       0x86U /* invalid size specified */
#define FMSTR_STC_SERVBUSY      0x87U /* service is busy */
#define FMSTR_STC_NOTINIT       0x88U /* service is not initialized */
#define FMSTR_STC_EACCESS       0x89U /* access is denied */
#define FMSTR_STC_SFIOERR       0x8AU /* Error in SFIO frame */
#define FMSTR_STC_SFIOUNMATCH   0x8BU /* Even/odd mismatch in SFIO transaction */
#define FMSTR_STC_PIPEERR       0x8CU /* Pipe error */
#define FMSTR_STC_FASTRECERR    0x8DU /* Feature not implemented in Fast Recorder */
#define FMSTR_STC_CANTGLERR     0x8EU /* CAN fragmentation (toggle bit) error */
#define FMSTR_STC_CANMSGERR     0x8FU /* CAN message format error */
#define FMSTR_STC_FLOATDISABLED 0x90U /* Floating point triggering is not enabled */
#define FMSTR_STC_EAUTH         0x91U /* Password authentication is required */
#define FMSTR_STC_EPASS         0x92U /* Password invalid */
#define FMSTR_STC_URESERR       0x93U /* User resources error */
#define FMSTR_STC_INSTERR       0x94U /* Bad instance index (e.g. recorder) */
#define FMSTR_STC_INVOPCODE     0x95U /* Invalid operation code (e.g. recorder) */
#define FMSTR_STC_EINSTANCE     0x96U /* Invalid instance number (e.g. recorder) */
#define FMSTR_STC_INVCONF       0x97U /* Invalid configuration */
#define FMSTR_STC_DEBUGTX_TEST  0xA9U /* Artificial +@W test frame sent when DEBUG_TX is enabled */

/******************************************************************************
 * Protocol constants
 *******************************************************************************/

#define FMSTR_PRCTL_VER  0x04  /* Protocol version */
#define FMSTR_SOB        0x2bU /* '+' - start of message*/
#define FMSTR_DESCR_SIZE 25U   /* length board desription string */

/* TSA-global flags  */
#define FMSTR_TSA_INFO_VERSION_MASK 0x0fU /* TSA version  */
#define FMSTR_TSA_INFO_SIZE_MASK    0x30U /* Address information */
#define FMSTR_TSA_INFO_ADRSIZE_16   0x00U /* Address information: 16 bit entries */
#define FMSTR_TSA_INFO_ADRSIZE_32   0x10U /* Address information: 32 bit entries */
#define FMSTR_TSA_INFO_ADRSIZE_64   0x20U /* Address information: 64 bit entries */
#define FMSTR_TSA_INFO_HV2BA        0x80U /* TSA HawkV2 byte-addressing mode */

/* Write command flags  */
#define FMSTR_WRMFLAG_WITHMASK 0x01U /* Used when Write memory command writes with Mask. */
#define FMSTR_WRMFLAG_WRFLASH  0x80U /* Used when Wrie memory command knowingly writing to flash. */

/******************************************************************************
 * CAN Protocol constants
 *******************************************************************************/

/* control byte (the first byte in each CAN message) */
#define FMSTR_CANCTL_TGL      0x80U /* toggle bit, first message clear, then toggles */
#define FMSTR_CANCTL_M2S      0x40U /* master to slave direction */
#define FMSTR_CANCTL_FST      0x20U /* first CAN message of FreeMASTER packet */
#define FMSTR_CANCTL_LST      0x10U /* last CAN message of FreeMASTER packet */
#define FMSTR_CANCTL_SPC      0x08U /* special command (in data[1], handled by CAN sublayer (no FM protocol decode) */
#define FMSTR_CANCTL_LEN_MASK 0x07U /* number of data bytes after the CTL byte (0..7) */

/******************************************************************************
 * Feature constants
 *******************************************************************************/

/* Feature ID */
#define FMSTR_FEATURE_SCOPE     0x01
#define FMSTR_FEATURE_REC       0x02
#define FMSTR_FEATURE_APPCMD    0x03
#define FMSTR_FEATURE_FLASHPROG 0x04
#define FMSTR_FEATURE_SFIO      0x05
#define FMSTR_FEATURE_PIPE      0x06

#endif /* __FREEMASTER_PROTOCOL_H */
