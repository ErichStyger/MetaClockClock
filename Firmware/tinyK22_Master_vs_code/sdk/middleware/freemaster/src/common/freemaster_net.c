/*
 * Copyright 2021, 2024-2025 NXP
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

#include "freemaster.h"
#include "freemaster_private.h"

/* Compile this code only if the NET transport is selected in freemaster_cfg.h. */
#if (FMSTR_MK_IDSTR(FMSTR_TRANSPORT) == FMSTR_NET_ID) && FMSTR_DISABLE == 0

#include "freemaster_protocol.h"
#include "freemaster_net.h"
#include "freemaster_utils.h"

/* Offset of data 'payload' in network buffer (skip total length, seq.number, cmd, payload length) */
#define FMSTR_NET_PAYLOAD_OFFSET 6U

/* FreeMASTER communication buffer (in/out) plus the total length, sequence number, command, payload len and CRC */
#define FMSTR_NET_FRAME_SIZE (((FMSTR_SIZE)FMSTR_COMM_BUFFER_SIZE) + FMSTR_NET_PAYLOAD_OFFSET + 1U)

typedef struct FMSTR_NET_SESSION_S
{
    FMSTR_U32 id;           /* Session identifier for debugging purposes only.  */
    FMSTR_U32 lastUsed;     /* Age of first usage, the smaller value the older. Zero means unused. */
    FMSTR_NET_ADDR address; /* TCP/UDP address */
    FMSTR_BCHR ioBuffer[FMSTR_NET_FRAME_SIZE]; /* Session's receive buffer */
    FMSTR_SIZE rxDataLen;   /* Number of valid bytes in the receive buffer */
    FMSTR_U8 seqNumber;     /* Last sent sequence number (=repeated last received) */
} FMSTR_NET_SESSION;

/***********************************
 *  local variables
 ***********************************/

/* Network sessions */
static FMSTR_NET_SESSION fmstr_netSessions[FMSTR_SESSION_COUNT];
static FMSTR_NET_SESSION fmstr_adhocSession;
static FMSTR_U32 fmstr_nSessionAgeCount = 0U;
static FMSTR_U32 fmstr_nSessionIdLast = 0U;

/***********************************
 *  local function prototypes
 ***********************************/

/* Interface function - Initialization of serial transport */
static FMSTR_BOOL _FMSTR_NetInit(void);
/* Interface function - Poll function of serial transport */
static void _FMSTR_NetPoll(void);
/* Interface function - Send Response function of serial transport */
static void _FMSTR_NetSendResponse(FMSTR_BPTR pResponse, FMSTR_SIZE nLength, FMSTR_U8 statusCode, void *identification);

/* Session find and use or reuse */
static FMSTR_NET_SESSION *_FMSTR_FindNetSession(FMSTR_NET_ADDR *addr, FMSTR_BOOL create);
/* Close session */
static void _FMSTR_NetCloseSession(FMSTR_NET_SESSION *ses);

static FMSTR_BOOL _FMSTR_NetProcess(void);
static void _FMSTR_NetSendStatus(FMSTR_BCHR nErrCode, FMSTR_NET_SESSION *ses);
static void _FMSTR_NetSendSessionFrame(FMSTR_SIZE nLength, FMSTR_U8 statusCode, FMSTR_NET_SESSION *ses);

#if FMSTR_NET_AUTODISCOVERY != 0
static void _FMSTR_NetSendDiscovery(FMSTR_NET_SESSION *ses);
#endif /* FMSTR_NET_AUTODISCOVERY */

/***********************************
 *  global variables
 ***********************************/

/* Interface of this serial driver */
const FMSTR_TRANSPORT_INTF FMSTR_NET = {
    FMSTR_C99_INIT(Init) _FMSTR_NetInit,
    FMSTR_C99_INIT(Poll) _FMSTR_NetPoll,
    FMSTR_C99_INIT(SendResponse) _FMSTR_NetSendResponse,
};

/******************************************************************************
 *
 * @brief    API: Network communication initialization
 *
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_NetInit(void)
{
    /* Check the interface if it's valid */
    FMSTR_ASSERT_RETURN(FMSTR_NET_DRV.Init != NULL, FMSTR_FALSE);
    FMSTR_ASSERT_RETURN(FMSTR_NET_DRV.Poll != NULL, FMSTR_FALSE);
    FMSTR_ASSERT_RETURN(FMSTR_NET_DRV.Recv != NULL, FMSTR_FALSE);
    FMSTR_ASSERT_RETURN(FMSTR_NET_DRV.Send != NULL, FMSTR_FALSE);
    FMSTR_ASSERT_RETURN(FMSTR_NET_DRV.Close != NULL, FMSTR_FALSE);
    FMSTR_ASSERT_RETURN(FMSTR_NET_DRV.GetCaps != NULL, FMSTR_FALSE);

    FMSTR_MemSet(&fmstr_netSessions, 0, sizeof(fmstr_netSessions));

    /* Call initialization of network driver */
    if (FMSTR_NET_DRV.Init() == FMSTR_FALSE)
    {
        return FMSTR_FALSE;
    }

    return FMSTR_TRUE;
}

/*******************************************************************************
 *
 * @brief    API: Network Transport "Polling" call from the application main loop
 *
 * This function either handles all the NET communication (polling-only mode =
 * FMSTR_POLL_DRIVEN) or decodes messages received on the background by NET interrupt
 * (short-interrupt mode = FMSTR_SHORT_INTR).
 *
 *******************************************************************************/

static void _FMSTR_NetPoll(void)
{
    /* Poll network driver */
    FMSTR_NET_DRV.Poll();

    /* Process the protocol */
    (void)_FMSTR_NetProcess();
}

/******************************************************************************
 *
 * @brief    API: Send protocol response. Called by upper layer.
 *
 * @param    nLength - response length (1 for status + data length)
 * @param    identification - a context, in our case a pointer to NET_SESSION
 * @param    pResponse - pointer to response payload, the buffer is actually the
 *                   one we used during message reception.
 *
 * This Function takes the data already prepared in the transmit buffer
 * It computes the check sum and kicks on TX.
 *
 ******************************************************************************/

static void _FMSTR_NetSendResponse(FMSTR_BPTR pResponse, FMSTR_SIZE nLength, FMSTR_U8 statusCode, void *identification)
{
    FMSTR_NET_SESSION *ses = (FMSTR_NET_SESSION *)identification;
    FMSTR_ASSERT(ses != NULL);

    /* Note that is is okay to assume the caller's buffer is actually our own session buffer at offset 6.
       This is the way all our responses were constructed here. */
    FMSTR_ASSERT(pResponse == &ses->ioBuffer[FMSTR_NET_PAYLOAD_OFFSET]);

    /* Sanity check the response length */
    if ((nLength > (FMSTR_SIZE)(FMSTR_COMM_BUFFER_SIZE)))
    {
        /* The Network driver doesn't support bigger responses than FMSTR_COMM_BUFFER_SIZE bytes, change the response to
         * status error */
        statusCode = FMSTR_STC_RSPBUFFOVF;
        nLength    = 0U;
    }

    /* Send the frame from the session's buffer directly */
    _FMSTR_NetSendSessionFrame(nLength, statusCode, ses);
}

/* Actual frame sending code - send a frame stored in the session's IO buffer */

static void _FMSTR_NetSendSessionFrame(FMSTR_SIZE nLength, FMSTR_U8 statusCode, FMSTR_NET_SESSION *ses)
{
    FMSTR_U8 c;
    FMSTR_SIZE i;
    FMSTR_SIZE todo;
    FMSTR_S32 sent, one;
    FMSTR_BCHR chSum = 0U;
    FMSTR_BPTR pMessageIO;
    FMSTR_BPTR pResponse;

    /* Maximum length we can put to output buffer. */
    FMSTR_ASSERT_RETURN(nLength < (FMSTR_SIZE)FMSTR_COMM_BUFFER_SIZE, /* void */);
    
    pMessageIO = &ses->ioBuffer[0];

    /* Send the message with length, seq, status and other info. */
    todo = nLength + 7U;

    /* Total frame length */
    pMessageIO = FMSTR_ValueToBuffer16BE(pMessageIO, (FMSTR_U16)todo);

    /* Sequence number */
    pMessageIO = FMSTR_ValueToBuffer8(pMessageIO, ses->seqNumber);

    /* Status code */
    pMessageIO = FMSTR_ValueToBuffer8(pMessageIO, (FMSTR_BCHR)statusCode);

    /* Response length */
    pMessageIO = FMSTR_ValueToBuffer16BE(pMessageIO, (FMSTR_U16)nLength);

    /* Initialize CRC algorithms */
    FMSTR_Crc8Init(&chSum);

    /* Checksum CRC8 */
    pResponse = &ses->ioBuffer[3];
    for (i = 0; i < nLength + 3U; i++)
    {
        /* Get from FMSTR buffer */
        pResponse = FMSTR_ValueFromBuffer8(&c, pResponse);

        /* add character to checksum */
        FMSTR_Crc8AddByte(&chSum, c);
    }

    /* Store checksum after the message */
    pResponse = FMSTR_ValueToBuffer8(pResponse, chSum);

    /* Send via network in a busy blocking loop. */
    sent = 0;
    while (todo > 0U)
    {
        one = FMSTR_NET_DRV.Send(&ses->address, &ses->ioBuffer[sent], todo);

        if (one < 0 || (sent+one) > (FMSTR_S32)todo)
        {
#if FMSTR_DEBUG_LEVEL >= 2
            FMSTR_DEBUG_PRINTF("NET Send error, session %lu\n", ses->id);
#endif
            /* Socket error condition */
            _FMSTR_NetCloseSession(ses);
            return;
        }

        sent += one;
        todo -= (FMSTR_SIZE)one;
    }
}

/******************************************************************************
 *
 * @brief    Process network communication
 *
 * @return   This function returns TRUE when at least some data were received
 *           so there is a chance something more will happen if called
 *           immediately again.
 *
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_NetProcess(void)
{
    FMSTR_BOOL isBroadcast = FMSTR_FALSE;
    FMSTR_BCHR chSum, crc;
    FMSTR_S32 received;
    FMSTR_BPTR pMessageIO, pCmdPayload, pCrc;
    FMSTR_S32 messageLen;
    FMSTR_U8 cmdCode;
    FMSTR_U16 cmdLen;
    FMSTR_U16 todo;
    FMSTR_U16 i;
    FMSTR_U8 c;

    FMSTR_NET_SESSION *session = NULL;
    FMSTR_NET_ADDR address;
    FMSTR_MemSet(&address, 0, sizeof(address));

    /* Receive data from any client into an adhoc session */
    fmstr_adhocSession.rxDataLen = 0;
    received = FMSTR_NET_DRV.Recv(fmstr_adhocSession.ioBuffer, sizeof(fmstr_adhocSession.ioBuffer), &address, &isBroadcast);

    if(received == 0)
    {
        /* Communication is idle */
        return FMSTR_FALSE;
    }

    /* A client wants to disconnect or suspicious data length received. */
    if (received < 0 || received > 0xffffL)
    {
        /* Find session if exists (do not create it) */
        session = _FMSTR_FindNetSession(&address, FMSTR_FALSE);
        if (session != NULL)
        {
            _FMSTR_NetCloseSession(session);
        }
        return FMSTR_TRUE;
    }

    if (isBroadcast == FMSTR_FALSE)
    {
        /* Find client session, create it if the address is seen for the first time */
        session = _FMSTR_FindNetSession(&address, FMSTR_TRUE);
        FMSTR_ASSERT_RETURN(session != NULL, FMSTR_FALSE);

        /* Copy received data to session's buffer. Does it fit? */
        /* Coverity: Intentional cast of existing rxDataLen to signed value */
        /* coverity[cert_int31_c_violation:FALSE] */
        messageLen = ((FMSTR_S32)session->rxDataLen) + received;

        if(messageLen > 0 && messageLen <= (FMSTR_S32)sizeof(session->ioBuffer))
        {
            FMSTR_MemCpy(&session->ioBuffer[session->rxDataLen], fmstr_adhocSession.ioBuffer, (FMSTR_SIZE)received);
            session->rxDataLen += (FMSTR_SIZE)received;
            pMessageIO = &session->ioBuffer[0];
        }
        else
        {
            /* Broken session */
#if FMSTR_DEBUG_LEVEL >= 2
            FMSTR_DEBUG_PRINTF("NET Recv fragment too long, session %lu\n", session->id);
#endif
            _FMSTR_NetCloseSession(session);
            return FMSTR_TRUE;
        }
    }
    else
    {
        /* Broadcast is processed immediately in the adhoc session. */
        session = &fmstr_adhocSession;
        FMSTR_MemCpy(&session->address, &address, sizeof(address));
        pMessageIO = fmstr_adhocSession.ioBuffer;
        messageLen = received;
    }

    /* Process the message at pMessageIO[0..messageLen]. The 'session' is always valid here */
    FMSTR_ASSERT(session != NULL);

    /* Need at least total length half-word */
    if (messageLen < 2)
    {
        return FMSTR_TRUE;
    }

    /* Fetch 'todo' total frame length */
    todo = 0U;
    pMessageIO = FMSTR_ValueFromBuffer16BE(&todo, pMessageIO);

    /* Sanity check of total length */
    if (todo > (FMSTR_U16)FMSTR_NET_FRAME_SIZE)
    {
#if FMSTR_DEBUG_LEVEL >= 2
        FMSTR_DEBUG_PRINTF("NET Recv frame length broken, session %lu\n", session->id);
#endif
        _FMSTR_NetCloseSession(session);
        return FMSTR_TRUE;
    }

    /* Not enough data received (yet) in buffer */
    if (messageLen < (FMSTR_S32)todo)
    {
        return FMSTR_TRUE;
    }

    /* Frame is now received completely in session's buffer. Start over next time. */
    session->rxDataLen = 0;

    /* Initialize CRC algorithms */
    chSum = 0;
    FMSTR_Crc8Init(&chSum);

    /* Sequence number */
    pMessageIO = FMSTR_ValueFromBuffer8(&session->seqNumber, pMessageIO);

    /* Pointer to start checking CRC */
    pCrc = pMessageIO;

    /* Command code */
    pMessageIO = FMSTR_ValueFromBuffer8(&cmdCode, pMessageIO);

    /* Command length */
    pMessageIO = FMSTR_ValueFromBuffer16BE(&cmdLen, pMessageIO);

    /* Total length shall be command length + the overhead header bytes + CRC  */
    if (todo != (cmdLen + FMSTR_NET_PAYLOAD_OFFSET + 1U))
    {
#if FMSTR_DEBUG_LEVEL >= 2
        FMSTR_DEBUG_PRINTF("NET Recv data length bad, session %lu\n", session->id);
#endif
        _FMSTR_NetCloseSession(session);
        return FMSTR_TRUE;
    }

    /* Command payload will be processed after CRC check */
    pCmdPayload = pMessageIO;

    /* Skip command length to seek to CRC position */
    pMessageIO = FMSTR_SkipInBuffer(pMessageIO, cmdLen);

    /* CRC */
    pMessageIO = FMSTR_ValueFromBuffer8(&crc, pMessageIO);

    /* Count CRC from the whole command */
    for (i = 0; i < cmdLen + 3U; i++)
    {
        /* Get from FMSTR buffer */
        pCrc = FMSTR_ValueFromBuffer8(&c, pCrc);

        /* add character to checksum */
        FMSTR_Crc8AddByte(&chSum, c);
    }

    /* Checksum */
    if (crc == chSum)
    {
        /* Network PING command is processed here. */
        if (cmdCode == FMSTR_NET_PING)
        {
            _FMSTR_NetSendStatus(FMSTR_STS_OK, session);
        }
#if FMSTR_NET_AUTODISCOVERY != 0
        /* Network auto-discovery command processed by this layer directly. */
        else if (cmdCode == FMSTR_NET_DISCOVERY)
        {
            /* Send discovery */
            _FMSTR_NetSendDiscovery(session);
        }
#endif
        else
        {
            /* Other messages are decoded the standard way */
            (void)FMSTR_ProtocolDecoder(pCmdPayload, cmdLen, cmdCode, session);
        }
    }
    else
    {
        /* Invalid checksum means corrupted protocol. */
#if FMSTR_DEBUG_LEVEL >= 2
        FMSTR_DEBUG_PRINTF("NET Recv checksum bad, session %lu\n", session->id);
#endif
        _FMSTR_NetCloseSession(session);
    }

    return FMSTR_TRUE;
}

#if FMSTR_NET_AUTODISCOVERY != 0
static void _FMSTR_NetSendDiscovery(FMSTR_NET_SESSION *ses)
{
    FMSTR_NET_IF_CAPS caps;
    FMSTR_BPTR pMessageIO;
    FMSTR_SIZE nameLen  = 0;
    FMSTR_U8 protocol = 0;

    /* Get protocol from low-level */
    FMSTR_MemSet(&caps, 0, sizeof(caps));
    FMSTR_NET_DRV.GetCaps(&caps);

    if ((caps.flags & FMSTR_NET_IF_CAPS_FLAG_UDP) != 0U)
    {
        protocol = (FMSTR_U8)FMSTR_NET_PROTOCOL_UDP;
    }
    else if ((caps.flags & FMSTR_NET_IF_CAPS_FLAG_TCP) != 0U)
    {
        protocol = (FMSTR_U8)FMSTR_NET_PROTOCOL_TCP;
    }
    else
    {
        FMSTR_ASSERT(FMSTR_FALSE);
    }

    /* Board name length must fit to a buffer */
    nameLen = FMSTR_StrLen(FMSTR_APPLICATION_STR);
    if (nameLen > (((FMSTR_SIZE)FMSTR_COMM_BUFFER_SIZE) - 4U))
    {
        /* Only send trimmed value */
        nameLen = ((FMSTR_SIZE)FMSTR_COMM_BUFFER_SIZE) - 4U;
    }
    else
    {
        /* Also send terminating zero */
        nameLen += 1U;
    }

    /* Optimize send process by giving space for header right now. */
    pMessageIO = &ses->ioBuffer[FMSTR_NET_PAYLOAD_OFFSET];

    /* Discovery command version */
    pMessageIO = FMSTR_ValueToBuffer8(pMessageIO, FMSTR_NET_DISCOVERY_VERSION);

    /* Protocol (TCP/UDP) */
    pMessageIO = FMSTR_ValueToBuffer8(pMessageIO, protocol);

    /* Length of board name fits into a single byte as checked above */
    /* coverity[cert_int31_c_violation:FALSE] */
    pMessageIO = FMSTR_ValueToBuffer8(pMessageIO, (FMSTR_U8)nameLen);

    /* Coverity: Intentional constants string cast and copy. */
    /* coverity[misra_c_2012_rule_7_4_violation:FALSE] */
    pMessageIO = FMSTR_CopyToBuffer(pMessageIO, (FMSTR_ADDR)(char *)FMSTR_APPLICATION_STR, nameLen);

    /* send response 3 fixed bytes and the name */
    _FMSTR_NetSendSessionFrame((FMSTR_SIZE)nameLen + 3U, FMSTR_STS_OK, ses);
}
#endif /* FMSTR_NET_AUTODISCOVERY */

static void _FMSTR_NetSendStatus(FMSTR_BCHR nErrCode, FMSTR_NET_SESSION *ses)
{
    /* fill & send single-byte response */
    _FMSTR_NetSendSessionFrame(0U, nErrCode, ses);
}

static void _FMSTR_NetCloseSession(FMSTR_NET_SESSION *ses)
{
    FMSTR_ASSERT(ses != NULL);

    /* Only close socket for sessions actually used before.
       Note that adhoc session is never closed this way. */
    if(ses->lastUsed > 0U)
    {
#if FMSTR_DEBUG_LEVEL >= 1
        FMSTR_DEBUG_PRINTF("NET Closing session %lu\n", ses->id);
#endif
        /* Close socket or other network resources bound to the session */
        FMSTR_NET_DRV.Close(&ses->address);

        /* Free upper-layer protocol session */
        FMSTR_FreeSession(ses);
    }

    /* Initialize session data */
    ses->lastUsed = 0;
    ses->rxDataLen = 0;
    ses->id = 0;
}

/* Locate NET session to work with. When create is true, we always force-find a session
   even if we discard some (the oldest). It is not our responsibility to count valid
   sessions. The lower-layer knows more about the session nature and if it needs
   to maintain a context (like TCP) or not (like UDP). */

static FMSTR_NET_SESSION *_FMSTR_FindNetSession(FMSTR_NET_ADDR *addr, FMSTR_BOOL create)
{
    FMSTR_NET_SESSION *ses           = NULL;
    FMSTR_NET_SESSION *freeSession   = NULL;
    FMSTR_NET_SESSION *oldestSession = NULL;
    FMSTR_INDEX i;

    FMSTR_ASSERT(addr != NULL);

    /* Session aging counter will never wrap, but anyway do explicitly here. */
    if(fmstr_nSessionAgeCount >= 0xffffffffUL)
    {
        fmstr_nSessionAgeCount = 0;
    }
    
    /* Same with session Id, do not let it wrap to 0 if it really happens. */
    if(fmstr_nSessionIdLast >= 0xffffffffUL)
    {
        fmstr_nSessionIdLast = 0;
    }
      

    for (i = 0; i < FMSTR_SESSION_COUNT; i++)
    {
        ses = &fmstr_netSessions[i];

        /* Find session by address */
        if (FMSTR_MemCmp(&ses->address, addr, sizeof(FMSTR_NET_ADDR)) == 0)
        {
            /* Found session */
            ses->lastUsed = ++fmstr_nSessionAgeCount;
            return ses;
        }

        /* Find free session. Ignore always-true warning in single-session configs. */
        /* coverity[misra_c_2012_rule_14_3_violation:FALSE] */
        if (freeSession == NULL && ses->lastUsed == 0U)
        {
            freeSession = ses;
        }

        /* Find oldest session */
        /* coverity[misra_c_2012_rule_14_3_violation:FALSE] */
        if (oldestSession == NULL || oldestSession->lastUsed > ses->lastUsed)
        {
            oldestSession = ses;
        }
    }

    /* Not found by address, return a new session */
    ses = (freeSession != NULL ? freeSession : oldestSession);

    /* Coverity: Always make sure the ses is valid before accessing it. */
    /* coverity[misra_c_2012_rule_14_3_violation:FALSE] */
    if (ses != NULL && create != FMSTR_FALSE)
    {
        /* Free the session (only if currently active) */
        _FMSTR_NetCloseSession(ses);

        ses->id = ++fmstr_nSessionIdLast;
        ses->lastUsed = ++fmstr_nSessionAgeCount;

        /* Map the session to the requested address. */
        FMSTR_MemCpy(&ses->address, addr, sizeof(FMSTR_NET_ADDR));

#if FMSTR_DEBUG_LEVEL >= 1
        FMSTR_DEBUG_PRINTF("NET Session %lu initialized (client port %u)\n", ses->id, ses->address.port);
#endif
    }

    return ses;
}

#endif /* (FMSTR_MK_IDSTR(FMSTR_TRANSPORT) == FMSTR_NET_ID) && FMSTR_DISABLE == 0 */
