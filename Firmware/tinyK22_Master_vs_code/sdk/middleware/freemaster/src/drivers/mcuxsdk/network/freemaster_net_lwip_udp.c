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
 * FreeMASTER Communication Driver - Network LWIP UDP driver
 */

#include "freemaster.h"
#include "freemaster_private.h"

/* Compile this code only if the NET driver is selected in freemaster_cfg.h. */
#if defined(FMSTR_NET_DRV) && (FMSTR_MK_IDSTR(FMSTR_NET_DRV) == FMSTR_NET_LWIP_UDP_ID)

#include "freemaster_net_lwip_udp.h"

#if FMSTR_DISABLE == 0

#include "freemaster_protocol.h"
#include "freemaster_net.h"

#include "lwip/api.h"
#include "lwip/sys.h"

/******************************************************************************
 * Adapter configuration
 ******************************************************************************/
#if (defined(FMSTR_SHORT_INTR) && FMSTR_SHORT_INTR) || (defined(FMSTR_LONG_INTR) && FMSTR_LONG_INTR)
#error The FreeMASTER network UDP lwip driver does not support interrupt mode.
#endif

/* santiy check of lwip options */
#if FMSTR_NET_AUTODISCOVERY != 0 && LWIP_NETBUF_RECVINFO == 0
#error The LWIP_NETBUF_RECVINFO (lwipopts.h) must be set to 1 when UDP transport used and FMSTR_NET_AUTODISCOVERY is enabled!
#endif

/******************************************************************************
 * Local functions
 ******************************************************************************/
static FMSTR_BOOL _FMSTR_NetLwipUdpInit(void);
static void _FMSTR_NetLwipUdpPoll(void);
static FMSTR_S32 _FMSTR_NetLwipUdpRecv(FMSTR_BPTR msgBuff,
                                       FMSTR_SIZE msgMaxSize,
                                       FMSTR_NET_ADDR *recvAddr,
                                       FMSTR_BOOL *isBroadcast);
static FMSTR_S32 _FMSTR_NetLwipUdpSend(FMSTR_NET_ADDR *sendAddr, FMSTR_BPTR msgBuff, FMSTR_SIZE msgSize);
static void _FMSTR_NetLwipUdpClose(FMSTR_NET_ADDR *addr);
static void _FMSTR_NetLwipUdpGetCaps(FMSTR_NET_IF_CAPS *caps);

static void _FMSTR_NetAddrToFmstr(FMSTR_NET_ADDR *fmstrAddr, struct netbuf *buf);
static void _FMSTR_NetAddrFromFmstr(struct netbuf *buf, FMSTR_NET_ADDR *fmstrAddr);

/******************************************************************************
 * Local variables
 ******************************************************************************/

static struct netconn *fmstrUdpConn = NULL;

/******************************************************************************
 * Driver interface
 ******************************************************************************/
/* Interface of this network UDP driver */
const FMSTR_NET_DRV_INTF FMSTR_NET_LWIP_UDP = {
    .Init    = _FMSTR_NetLwipUdpInit,
    .Poll    = _FMSTR_NetLwipUdpPoll,
    .Recv    = _FMSTR_NetLwipUdpRecv,
    .Send    = _FMSTR_NetLwipUdpSend,
    .Close   = _FMSTR_NetLwipUdpClose,
    .GetCaps = _FMSTR_NetLwipUdpGetCaps,
};

/******************************************************************************
 * Implementation
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_NetLwipUdpInit(void)
{
    err_t err;

    /* Create new netconn connection */
    fmstrUdpConn = netconn_new(NETCONN_UDP);
    if (fmstrUdpConn == NULL)
    {
        return FMSTR_FALSE;
    }

    /* Bind address and port */
    err = netconn_bind(fmstrUdpConn, IP_ADDR_ANY, FMSTR_NET_PORT);
    if (err != ERR_OK)
    {
        /* Coverity: Intentionally ignore return code, going to return FALSE anyway */
        /* coverity[misra_c_2012_directive_4_7_violation:FALSE] */
        (void)netconn_delete(fmstrUdpConn);
        fmstrUdpConn = NULL;
        return FMSTR_FALSE;
    }

    /* Ignore coverity signed/unsigned warning in 3rd party code */
#if FMSTR_NET_BLOCKING_TIMEOUT == 0
    /* coverity[misra_c_2012_rule_10_1_violation:FALSE] */
    /* coverity[misra_c_2012_rule_10_4_violation:FALSE] */
    /* coverity[misra_c_2012_rule_10_8_violation:FALSE] */
    netconn_set_nonblocking(fmstrUdpConn, true);
#else
    /* coverity[misra_c_2012_rule_10_1_violation:FALSE] */
    /* coverity[misra_c_2012_rule_10_4_violation:FALSE] */
    /* coverity[misra_c_2012_rule_10_8_violation:FALSE] */
    netconn_set_nonblocking(fmstrUdpConn, false);
    netconn_set_recvtimeout(fmstrUdpConn, FMSTR_NET_BLOCKING_TIMEOUT);
#endif

    return FMSTR_TRUE;
}

static void _FMSTR_NetLwipUdpPoll(void)
{
}

static FMSTR_S32 _FMSTR_NetLwipUdpRecv(FMSTR_BPTR msgBuff,
                                       FMSTR_SIZE msgMaxSize,
                                       FMSTR_NET_ADDR *recvAddr,
                                       FMSTR_BOOL *isBroadcast)
{
    err_t err;
    int recvSize = 0;
    struct netbuf *buf = NULL;

    FMSTR_ASSERT(msgBuff != NULL);
    FMSTR_ASSERT(recvAddr != NULL);
    FMSTR_ASSERT(isBroadcast != NULL);

    *isBroadcast = FMSTR_FALSE;

    if (fmstrUdpConn == NULL)
    {
        return -1;
    }

    /* Receive data */
    err = netconn_recv(fmstrUdpConn, &buf);
    if (err == ERR_WOULDBLOCK)
    {
        return 0;
    }
    if (err != ERR_OK || buf == NULL)
    {
        return -1;
    }

    /* Copy received data. Ignore coverity type-cast warning in 3rd party code */
    /* coverity[misra_c_2012_rule_10_3_violation:FALSE] */
    recvSize = netbuf_copy(buf, msgBuff, msgMaxSize);

    /* Copy address of receiver */
    _FMSTR_NetAddrToFmstr(recvAddr, buf);

    netbuf_delete(buf);

    return (FMSTR_S32)recvSize;
}

static FMSTR_S32 _FMSTR_NetLwipUdpSend(FMSTR_NET_ADDR *sendAddr, FMSTR_BPTR msgBuff, FMSTR_SIZE msgSize)
{
    FMSTR_S32 sentSize = 0;
    struct netbuf *buf;
    err_t err;

    /* Sanity checks */
    FMSTR_ASSERT(msgBuff != NULL);
    FMSTR_ASSERT(sendAddr != NULL);
    FMSTR_ASSERT(msgSize <= 0x600U);

    buf = netbuf_new();

    if (fmstrUdpConn == NULL)
    {
        sentSize = -1;
        goto ERROR;
    }

    /* Reference the message into the netbuf. Ignore coverity type-cast warning in 3rd party code */
    err = netbuf_ref(buf, msgBuff, (u16_t)msgSize);
    if (err != ERR_OK)
    {
        sentSize = -1;
        goto ERROR;
    }

    /* Copy destination address */
    _FMSTR_NetAddrFromFmstr(buf, sendAddr);

    /* Send data */
    err = netconn_send(fmstrUdpConn, buf);
    if (err != ERR_OK)
    {
        sentSize = -1;
        goto ERROR;
    }

    sentSize = (FMSTR_S32)buf->p->tot_len;

ERROR:
    netbuf_delete(buf);

    return sentSize;
}

static void _FMSTR_NetLwipUdpClose(FMSTR_NET_ADDR *addr)
{
}

static void _FMSTR_NetLwipUdpGetCaps(FMSTR_NET_IF_CAPS *caps)
{
    FMSTR_ASSERT(caps != NULL);

    caps->flags |= FMSTR_NET_IF_CAPS_FLAG_UDP;
}

static void _FMSTR_NetAddrToFmstr(FMSTR_NET_ADDR *fmstrAddr, struct netbuf *buf)
{
    FMSTR_ASSERT(buf != NULL);
    FMSTR_ASSERT(fmstrAddr != NULL);

    /* IP address type */
    fmstrAddr->type = FMSTR_NET_ADDR_TYPE_V4;

    /* Port */
    fmstrAddr->port = buf->port;

    /* IP address */
    if (fmstrAddr->type == FMSTR_NET_ADDR_TYPE_V4)
    {
        FMSTR_MemCpy(fmstrAddr->addr.v4, &buf->addr, 4);
    }

    // TODO: add support for IPv6 address
}

static void _FMSTR_NetAddrFromFmstr(struct netbuf *buf, FMSTR_NET_ADDR *fmstrAddr)
{
    FMSTR_ASSERT(fmstrAddr != NULL);
    FMSTR_ASSERT(buf != NULL);

    /* Port */
    buf->port = fmstrAddr->port;

    if (fmstrAddr->type == FMSTR_NET_ADDR_TYPE_V4)
    {
        /* IP address */
        FMSTR_MemCpy(&buf->addr, fmstrAddr->addr.v4, 4);
    }

    // TODO: add support for IPv6 address
}

#endif /* (!(FMSTR_DISABLE)) */
#endif /* defined(FMSTR_NET_DRV) && (FMSTR_MK_IDSTR(FMSTR_NET_DRV) == FMSTR_NET_MCUX_LWIP_UPD_ID) */
