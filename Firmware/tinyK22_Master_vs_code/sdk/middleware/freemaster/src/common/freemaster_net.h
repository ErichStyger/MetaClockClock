/*
 * Copyright 2021, 2024 NXP
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
 * FreeMASTER Communication Driver - Network communication
 */

#ifndef __FREEMASTER_NET_H
#define __FREEMASTER_NET_H

/******************************************************************************
* Required header files include check
******************************************************************************/
#ifndef __FREEMASTER_H
#error Please include the freemaster.h master header file before the freemaster_net.h
#endif

/******************************************************************************
* Serial transport related configuration checks
******************************************************************************/

/* transport MUST be defined in configuration */
#if !defined(FMSTR_NET_DRV)
    #error No FreeMASTER communication network driver interface is enabled. Please choose the driver (FMSTR_NET_DRV) or set FMSTR_DISABLE option to 1.
#endif

/* Network timeout definition check */
#ifndef FMSTR_NET_BLOCKING_TIMEOUT
#define FMSTR_NET_BLOCKING_TIMEOUT 10
#endif

/* Auto discovery definition check */
#ifndef FMSTR_NET_AUTODISCOVERY
#define FMSTR_NET_AUTODISCOVERY 0
#endif

#ifdef __cplusplus
  extern "C" {
#endif

/******************************************************************************
* Types definition
******************************************************************************/

#define FMSTR_NET_DISCOVERY_VERSION     1U
#define FMSTR_NET_IF_CAPS_FLAG_UDP      0x01UL
#define FMSTR_NET_IF_CAPS_FLAG_TCP      0x02UL
#define FMSTR_NET_IF_CAPS_FLAG_RTT      0x04UL

typedef struct FMSTR_NET_IF_CAPS_S
{
    FMSTR_U32 flags;
} FMSTR_NET_IF_CAPS;

typedef enum
{
    FMSTR_NET_PROTOCOL_UDP = 1,
    FMSTR_NET_PROTOCOL_TCP = 2,
} FMSTR_NET_PROTOCOL;

typedef enum
{
  FMSTR_NET_ADDR_TYPE_V4,
  FMSTR_NET_ADDR_TYPE_V6,
} FMSTR_NET_ADDR_TYPE;

typedef struct FMSTR_NET_ADDR_S
{
    FMSTR_NET_ADDR_TYPE type;
    FMSTR_U16 port;
    union
    {
        FMSTR_BCHR v4[4];
        FMSTR_BCHR v6[16];
    } addr;
} FMSTR_NET_ADDR;

/* Network Driver interface between the Network layer and hardware driver */
typedef struct FMSTR_NET_DRV_INTF_S
{
    FMSTR_BOOL (*Init)(void);   /* Initialization */
    void (*Poll)(void);         /* General poll call */
    FMSTR_S32 (*Recv)(FMSTR_BPTR msgBuff, FMSTR_SIZE msgMaxSize, FMSTR_NET_ADDR * recvAddr, FMSTR_BOOL * isBroadcast);  /* Receive data */
    FMSTR_S32 (*Send)(FMSTR_NET_ADDR * sendAddr, FMSTR_BPTR msgBuff, FMSTR_SIZE msgSize);     /* Send data */
    void (*Close)(FMSTR_NET_ADDR * addr); /* Close active session */
    void (*GetCaps)(FMSTR_NET_IF_CAPS * caps);
} FMSTR_NET_DRV_INTF;

/******************************************************************************
* Functions definitions
******************************************************************************/

/* If defined in freemaster_cfg.h, make sure the NETWORK driver interface is also declared properly. */
#ifdef FMSTR_NET_DRV
extern const FMSTR_NET_DRV_INTF FMSTR_NET_DRV;
#endif

#ifdef __cplusplus
  }
#endif

#endif /* __FREEMASTER_NET_H */
