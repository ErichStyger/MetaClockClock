/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2018-2021, 2024-2025 NXP
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
 * FreeMASTER Communication Driver - protocol implementation
 */

#include "freemaster.h"
#include "freemaster_private.h"
#include "freemaster_protocol.h"
#include "freemaster_utils.h"

#if FMSTR_DISABLE == 0

/* When using sessions, calculate number of instances of each lockable feature as U8 type */
#if FMSTR_SESSION_COUNT > 1

#if FMSTR_USE_SCOPE > 0
#define _FMSTR_SC ((FMSTR_SIZE)(FMSTR_USE_SCOPE))
#else
#define _FMSTR_SC 0U
#endif

#if FMSTR_USE_RECORDER > 0
#define _FMSTR_RC ((FMSTR_SIZE)(FMSTR_USE_RECORDER))
#else
#define _FMSTR_RC 0U
#endif

#if FMSTR_USE_PIPES > 0
#define _FMSTR_PC ((FMSTR_SIZE)(FMSTR_USE_PIPES))
#else
#define _FMSTR_PC 0U
#endif

#if FMSTR_USE_APPCMD > 0
#define _FMSTR_AC 1U /* All app commands make up one instance, if enabled */
#else
#define _FMSTR_AC 0U
#endif

#define _FMSTR_FPG 1U /* Flash programmer is a lockable software feature. Always enabled */

/* Macros to convert instance to an index in one large fature locking array */

#define FMSTR_FEATURE_SCOPE_INSTANCE(n)     ((FMSTR_SIZE)(n))
#define FMSTR_FEATURE_REC_INSTANCE(n)       ((FMSTR_SIZE)(_FMSTR_SC + (FMSTR_U8)(n)))
#define FMSTR_FEATURE_APPCMD_INSTANCE(n)    ((FMSTR_SIZE)(_FMSTR_SC + _FMSTR_RC + 0U))
#define FMSTR_FEATURE_FLASHPROG_INSTANCE(n) ((FMSTR_SIZE)(_FMSTR_SC + _FMSTR_RC + _FMSTR_AC + 0U))
#define FMSTR_FEATURE_PIPE_INSTANCE(n)      ((FMSTR_SIZE)(_FMSTR_SC + _FMSTR_RC + _FMSTR_AC + _FMSTR_FPG + (FMSTR_U8)(n)))
#define FMSTR_FEATURE_LOCK_COUNT            ((FMSTR_SIZE)(_FMSTR_SC + _FMSTR_RC + _FMSTR_AC + _FMSTR_FPG + _FMSTR_PC))

typedef struct
{
    FMSTR_SESSION *owner; /* Session which locks feature */
    FMSTR_S32 lockCount;  /* Locked count */
} FMSTR_FEATURE_LOCK;
#endif

/******************************************************************************
 * Local Functions
 ******************************************************************************/

/* The functions are local, but not declared "static" to prevent linker warnings
   when features are excluded from protocol. */
const FMSTR_CHAR *_FMSTR_GetAccessPassword(FMSTR_U8 requiredAccess);
FMSTR_BPTR _FMSTR_GetBoardConfig(FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus);
FMSTR_BPTR _FMSTR_ReadMem(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus);
FMSTR_BPTR _FMSTR_ReadMemBaseAddress(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus);
FMSTR_BPTR _FMSTR_WriteMem(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus);

#if FMSTR_CFG_F1_RESTRICTED_ACCESS != 0
FMSTR_BPTR _FMSTR_AuthenticationStep1(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus);
FMSTR_BPTR _FMSTR_AuthenticationStep2(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus);
#endif

static FMSTR_SESSION *_FMSTR_FindSession(void *identification, FMSTR_BOOL create);

#if FMSTR_SESSION_COUNT > 1
static FMSTR_BPTR _FMSTR_FeatureLock(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus);
#endif
/******************************************************************************
 * Local variables
 ******************************************************************************/

/* Protocol sessions */
static FMSTR_SESSION fmstr_sessions[FMSTR_SESSION_COUNT];

#if FMSTR_SESSION_COUNT > 1
/* Features */
static FMSTR_FEATURE_LOCK fmstr_features[FMSTR_FEATURE_LOCK_COUNT];
#endif

#if FMSTR_CFG_F1_RESTRICTED_ACCESS != 0
/* To save stack usage, we have the following SHA context variables static, although
 * they could be local in Auth2 function. */
static FMSTR_SHA1_CTX fmstr_sha1Ctx;
/* Currently granted access (one of FMSTR_RESTRICTED_ACCESS_xxx). */
static FMSTR_U8 fmstr_dfltGrantedAccess;
#endif /* FMSTR_CFG_F1_RESTRICTED_ACCESS */

#if FMSTR_DEBUG_TX > 0
/* This warning is for you not to forget to disable the DEBUG_TX mode once the
 * communication line works fine. You can ignore this warning if it is okay
 * that the application will be periodically sending test frames to the world. */
#warning FMSTR_DEBUG_TX is enabled. Test frames will be transmitted periodically.
/* When communication debugging mode is requested, this global variable is used to
   turn the debugging off once a valid connection is detected */
FMSTR_BOOL fmstr_doDebugTx;
#endif

/******************************************************************************
 *
 * @brief    FreeMASTER driver initialization
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_Init(void)
{
    FMSTR_BOOL ok = FMSTR_TRUE;

    FMSTR_MemSet(&fmstr_sessions, 0, sizeof(fmstr_sessions));

#if FMSTR_SESSION_COUNT > 1
    FMSTR_MemSet(&fmstr_features, 0, sizeof(fmstr_features));
#endif

#if FMSTR_DEBUG_LEVEL >= 1
    FMSTR_DEBUG_PRINTF("FMSTR Init begins\n");
#endif

    /* Check the transport interface validity */
    FMSTR_ASSERT_RETURN(FMSTR_TRANSPORT.Init != NULL, FMSTR_FALSE);
    FMSTR_ASSERT_RETURN(FMSTR_TRANSPORT.Poll != NULL, FMSTR_FALSE);
    FMSTR_ASSERT_RETURN(FMSTR_TRANSPORT.SendResponse != NULL, FMSTR_FALSE);

#if FMSTR_USE_TSA > 0
    /* initialize TSA */
    if (FMSTR_InitTsa() == FMSTR_FALSE)
    {
        ok = FMSTR_FALSE;
    }
#endif

#if FMSTR_USE_SCOPE > 0
    /* initialize Scope */
    if (FMSTR_InitScope() == FMSTR_FALSE)
    {
        ok = FMSTR_FALSE;
    }
#endif

#if FMSTR_USE_RECORDER > 0
    /* initialize Recorder */
    if (FMSTR_InitRec() == FMSTR_FALSE)
    {
        ok = FMSTR_FALSE;
    }
#endif

#if FMSTR_USE_APPCMD > 0
    /* initialize application commands */
    if (FMSTR_InitAppCmds() == FMSTR_FALSE)
    {
        ok = FMSTR_FALSE;
    }
#endif

#if FMSTR_USE_PIPES > 0
    /* initialize PIPES interface */
    if (FMSTR_InitPipes() == FMSTR_FALSE)
    {
        ok = FMSTR_FALSE;
    }
#endif

    /* initialize communication layer and start listening for commands */
    if (FMSTR_TRANSPORT.Init() == FMSTR_FALSE)
    {
        ok = FMSTR_FALSE;
    }

#if FMSTR_DEBUG_TX > 0
    /* start in the debugging mode, we will be sending test frames periodically */
    fmstr_doDebugTx = FMSTR_TRUE;
#endif

#if FMSTR_CFG_F1_RESTRICTED_ACCESS != 0
    fmstr_dfltGrantedAccess = FMSTR_RESTRICTED_ACCESS_NO;
    /* set the access level to the highest "open" level (the one without a password) */
    for (FMSTR_U8 access = FMSTR_RESTRICTED_ACCESS_R; access <= FMSTR_RESTRICTED_ACCESS_RWF; access++)
    {
        if (_FMSTR_GetAccessPassword(access) != NULL)
        {
            break; /* this level (and all levels above) are protected by a password */
        }
        else
        {
            fmstr_dfltGrantedAccess = access;
        }
    }
#endif

#if FMSTR_DEBUG_LEVEL >= 1
    FMSTR_DEBUG_PRINTF("FMSTR Init finished, ok=%d\n", (int)ok);
#endif

    FMSTR_ASSERT(ok != FMSTR_FALSE);
    return ok;
}

/*******************************************************************************
 *
 * @brief    API: Main "Polling" call from the application main loop
 *
 * This function either handles all the communication (polling-only mode =
 * FMSTR_POLL_DRIVEN) or decodes messages received on the background by SCI interrupt
 * (short-interrupt mode = FMSTR_SHORT_INTR).
 *
 *
 *******************************************************************************/

void FMSTR_Poll(void)
{
    /* Increase entropy in each poll round */
#if FMSTR_CFG_F1_RESTRICTED_ACCESS != 0
    static FMSTR_U8 e = 0x7U;
    FMSTR_Randomize(e);
    e += 13U;
#endif

    FMSTR_TRANSPORT.Poll();
}

/******************************************************************************
 *
 * @brief    Finalize transmit buffer before transmitting
 *
 * @param    length - response length
 *
 *
 * This Function takes the data already prepared in the transmit buffer
 * (inlcuding the status byte). It computes the check sum and kicks on tx.
 *
 ******************************************************************************/

void FMSTR_SendResponse(FMSTR_BPTR response, FMSTR_SIZE length, FMSTR_U8 statusCode, FMSTR_SESSION *session)
{
    /* Allow NULL session only if sessions count is 1, use the first (single) session as default. */
    if (session == NULL)
    {
#if FMSTR_SESSION_COUNT == 1
        session = &fmstr_sessions[0];
#else
        /* You must specify session when multiple sessions are supported */
        FMSTR_ASSERT(FMSTR_FALSE);
        return;
#endif
    }

#if FMSTR_DEBUG_LEVEL >= 3
    FMSTR_DEBUG_PRINTF("FMSTR SendResponse Status: 0x%x, Len: 0x%x\n", statusCode, length);
#endif

    FMSTR_TRANSPORT.SendResponse(response, length, statusCode, session->identification);
}

/******************************************************************************
 *
 * @brief    Decodes the FreeMASTER protocol and calls appropriate handlers
 *
 * @param    msgBuffIO - message in/out buffer
 * @param    identification - transport identification
 *
 * @return   TRUE if frame was valid and any output was generated to IO buffer
 *
 * This Function decodes given message and invokes proper command handler
 * which fills in the response. The response transmission is initiated
 * in this call as well.
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_ProtocolDecoder(FMSTR_BPTR msgBuffIO, FMSTR_SIZE msgSize, FMSTR_U8 cmdCode, void *identification)
{
    FMSTR_BPTR responseEnd       = msgBuffIO;
    FMSTR_U8 statusCode          = FMSTR_STS_INVALID;
    FMSTR_SESSION *activeSession = NULL;

#if FMSTR_DEBUG_LEVEL >= 3
    FMSTR_DEBUG_PRINTF("FMSTR ProtocolDecoder Cmd: 0x%x, Len: 0x%x\n", cmdCode, msgSize);
#endif

    /* Find session (or free) and create it for this identification  */
    activeSession = _FMSTR_FindSession(identification, FMSTR_TRUE);

    FMSTR_ASSERT_RETURN(activeSession != NULL, FMSTR_FALSE);

    /* process command   */
    switch (cmdCode)
    {
        /* retrieve board configuration value*/
        case FMSTR_CMD_GETCONFIG:
            responseEnd = _FMSTR_GetBoardConfig(msgBuffIO, &statusCode);
            break;

#if FMSTR_USE_READMEM > 0
        /* read a block of memory */
        case FMSTR_CMD_READMEM:
            responseEnd = _FMSTR_ReadMem(activeSession, msgBuffIO, &statusCode);
            break;

#ifdef FMSTR_PLATFORM_BASE_ADDRESS
        /* read a block of memory with base address*/
        case FMSTR_CMD_READMEM_BA:
            responseEnd = _FMSTR_ReadMemBaseAddress(activeSession, msgBuffIO, &statusCode);
            break;
#endif

#endif /* FMSTR_USE_READMEM */

#if FMSTR_USE_WRITEMEM > 0
        /* write a block of memory */
        case FMSTR_CMD_WRITEMEM:
            responseEnd = _FMSTR_WriteMem(activeSession, msgBuffIO, &statusCode);
            break;
#endif /* FMSTR_USE_WRITEMEM */

#if FMSTR_CFG_F1_RESTRICTED_ACCESS != 0
        /* Initiate password authentication, request authentication challenge */
        case FMSTR_CMD_AUTH1:
            responseEnd = _FMSTR_AuthenticationStep1(activeSession, msgBuffIO, &statusCode);
            break;

        /* Initiate password authentication, request authentication challenge */
        case FMSTR_CMD_AUTH2:
            responseEnd = _FMSTR_AuthenticationStep2(activeSession, msgBuffIO, &statusCode);
            break;
#endif

#if FMSTR_USE_PIPES > 0
        case FMSTR_CMD_GETPIPE:
            responseEnd = FMSTR_GetPipe(activeSession, msgBuffIO, msgSize, &statusCode);
            break;
        case FMSTR_CMD_PIPE:
            responseEnd = FMSTR_PipeFrame(activeSession, msgBuffIO, msgSize, &statusCode);
            break;
#endif /* FMSTR_USE_PIPES */

#if FMSTR_USE_TSA > 0
        /* get TSA table (force EX instead of non-EX) */
        case FMSTR_CMD_GETTSAINFO:
            responseEnd = FMSTR_GetTsaInfo(msgBuffIO, &statusCode);
            break;

        case FMSTR_CMD_GETSTRLEN:
            responseEnd = FMSTR_GetStringLen(msgBuffIO, &statusCode);
            break;
#if FMSTR_USE_URES > 0
        case FMSTR_CMD_URESRWI:
            responseEnd = FMSTR_UresControl(msgBuffIO, msgSize, &statusCode);
            break;
#endif /* FMSTR_USE_URES */
#endif /* FMSTR_USE_TSA */

#if FMSTR_USE_APPCMD > 0
        /* accept the application command */
        case FMSTR_CMD_SENDAPPCMD:
            responseEnd = FMSTR_StoreAppCmd(activeSession, msgBuffIO, msgSize, &statusCode);
            break;

        /* get the application command status */
        case FMSTR_CMD_GETAPPCMDSTS:
            responseEnd = FMSTR_GetAppCmdStatus(activeSession, msgBuffIO, &statusCode);
            break;

        /* get the application command data */
        case FMSTR_CMD_GETAPPCMDDATA:
            responseEnd = FMSTR_GetAppCmdRespData(activeSession, msgBuffIO, &statusCode);
            break;
#endif /* FMSTR_USE_APPCMD */

#if FMSTR_USE_RECORDER > 0
        /* get recorder data */
        case FMSTR_CMD_GETREC:
            responseEnd = FMSTR_GetRecCmd(activeSession, msgBuffIO, &statusCode);
            break;

        /* setup recorder */
        case FMSTR_CMD_SETREC:
            responseEnd = FMSTR_SetRecCmd(activeSession, msgBuffIO, msgSize, &statusCode);
            break;
#endif /* FMSTR_USE_RECORDER */

#if FMSTR_USE_SCOPE > 0
        /* prepare scope variables */
        case FMSTR_CMD_SETSCOPE:
            responseEnd = FMSTR_SetScope(activeSession, msgBuffIO, msgSize, &statusCode);
            break;

        case FMSTR_CMD_READSCOPE:
            responseEnd = FMSTR_ReadScope(activeSession, msgBuffIO, &statusCode, FMSTR_COMM_BUFFER_SIZE);
            break;
#endif /* FMSTR_USE_SCOPE */

#if FMSTR_SESSION_COUNT > 1
        /* Lock or unlock feature in a multi-session configuration */
        case FMSTR_CMD_FEATLOCK:
            responseEnd = _FMSTR_FeatureLock(activeSession, msgBuffIO, &statusCode);
            break;
#endif /* FMSTR_SESSION_COUNT > 1 */

        /* unknown command */
        default:
            statusCode = FMSTR_STC_INVCMD;
            break;
    }

    /* anything to send back? */
    if (statusCode != FMSTR_STS_INVALID)
    {
        /*lint -e{946,960} subtracting pointers is appropriate here */
        FMSTR_INDEX ptrDiff = (FMSTR_INDEX)(responseEnd - msgBuffIO);

        /* Coverity: Intentional signed/unsigned cast. The 'responseEnd' value is always >= msgBuffIO 
           and within the buffer size limit, so the result will not wrap and will be positive. 
           No risk of misinterpretation. */
        /* coverity[cert_int31_c_violation:FALSE] */
        FMSTR_SIZE respSize = (FMSTR_SIZE)ptrDiff;
        
        /* Non-variable length error responses are forced to 0 length */
        if ((statusCode & (FMSTR_STSF_VARLEN | FMSTR_STSF_ERROR)) == FMSTR_STSF_ERROR)
        {
            respSize = 0;
        }

#if FMSTR_DEBUG_TX > 0
        /* the first sane frame received from PC Host ends test frame sending */
        fmstr_doDebugTx = FMSTR_FALSE;
#endif
        FMSTR_SendResponse(msgBuffIO, respSize, statusCode, activeSession);
        return FMSTR_TRUE;
    }
    else
    {
        /* nothing sent out */
        return FMSTR_FALSE;
    }
}

/******************************************************************************
 *
 * @brief    Sending debug test frame
 *
 * @param    msgBuffIO - outut frame buffer
 * @param    session - transport session
 *
 * @return   True if successful (always in current implementation)
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_SendTestFrame(FMSTR_BPTR msgBuffIO, FMSTR_SESSION *session)
{
    FMSTR_SendResponse(msgBuffIO, 0, FMSTR_STC_DEBUGTX_TEST, session);
    return FMSTR_TRUE;
}

/******************************************************************************
 *
 * @brief    Handling GETINFO or GETINFO_BRIEF
 *
 * @param    msgBuffIO - original command (in) and response buffer (out)
 * @param    retStatus - response status
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response data payload is finished
 *
 ******************************************************************************/

FMSTR_BPTR _FMSTR_GetBoardConfig(FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus)
{
    static const FMSTR_CHAR *const fmstr_cfgParamNames[] = {
        "MTU", "VS", "NM", "DS", "BD", "F1", "BA", "RC", "SC", "PV", "PC",
    };

    FMSTR_BPTR response = msgBuffIO;
    FMSTR_U8 respCode   = FMSTR_STS_INVALID;
    FMSTR_CHAR str[4];
    FMSTR_INDEX ix = 0;

    /* Get the IX from incomming buffer */
    msgBuffIO = FMSTR_IndexFromBuffer(&ix, msgBuffIO);

    if (ix == 0)
    {
        FMSTR_INDEX i;
        /* Get the configuration name from incoming buffer */
        msgBuffIO = FMSTR_StringFromBuffer(msgBuffIO, str, sizeof(str));

        /* Try to find the Index of the config item */
        for (i = 0; i < (FMSTR_INDEX)FMSTR_COUNTOF(fmstr_cfgParamNames); i++)
        {
            if (FMSTR_StrCmp(fmstr_cfgParamNames[i], str) == 0)
            {
                ix = i + 1;
                break;
            }
        }
    }

    /* Name not found or index invalid */
    if (ix <= 0)
    {
        respCode = FMSTR_STC_EACCESS;
        goto FMSTR_GetConfig_exit;
    }
    
    respCode = FMSTR_STS_OK | FMSTR_STSF_VARLEN;
    response = FMSTR_StringCopyToBuffer(response, fmstr_cfgParamNames[ix - 1]);

    /* The ix must corresponded to table fmstr_cfgParamNames incremented by one */
    switch (ix)
    {
        case 1: /* MTU */
            response = FMSTR_SizeToBuffer(response, FMSTR_COMM_BUFFER_SIZE);
            break;
        case 2: /* VS */
            response = FMSTR_StringCopyToBuffer(response, FMSTR_VERSION_STR);
            break;
        case 3: /* NM */
            response = FMSTR_StringCopyToBuffer(response, FMSTR_APPLICATION_STR);
            break;
        case 4: /* DS */
            response = FMSTR_StringCopyToBuffer(response, FMSTR_DESCRIPTION_STR);
            break;
        case 5: /* BD */
            response = FMSTR_StringCopyToBuffer(response, FMSTR_BUILDTIME_STR);
            break;
        case 6: /* F1 */
            /* Coverity: Intentional use of invariant macro expression. */
            /* coverity[misra_c_2012_rule_14_3_violation:FALSE] */
            /* coverity[misra_c_2012_rule_10_4_violation:FALSE] */
            response = FMSTR_ValueToBuffer8(response, FMSTR_CFG_F1);
            break;
#ifdef FMSTR_PLATFORM_BASE_ADDRESS
        case 7: /* BA */
            response = FMSTR_SizeToBuffer(response, FMSTR_PLATFORM_BASE_ADDRESS);
            break;
#endif
        case 8: /* RC */
            response = FMSTR_ValueToBuffer8(response, FMSTR_USE_RECORDER);
            break;
        case 9: /* SC */
            response = FMSTR_ValueToBuffer8(response, FMSTR_USE_SCOPE);
            break;
        case 10: /* PV */
            response = FMSTR_ValueToBuffer8(response, FMSTR_PRCTL_VER);
            break;
        case 11: /* PC */
            response = FMSTR_ValueToBuffer8(response, FMSTR_USE_PIPES);
            break;
        default:
            respCode = FMSTR_STC_EACCESS;
            break;
    }

FMSTR_GetConfig_exit:
    *retStatus = respCode;

    return response;
}

/* Helper call to obtain password required for certain access level. */

const FMSTR_CHAR *_FMSTR_GetAccessPassword(FMSTR_U8 requiredAccess)
{
    const FMSTR_CHAR *password = NULL;
    FMSTR_UNUSED(requiredAccess);

#if defined(FMSTR_RESTRICTED_ACCESS_R_PASSWORD)
    if (requiredAccess >= FMSTR_RESTRICTED_ACCESS_R)
    {
        password = (FMSTR_CHAR *)FMSTR_RESTRICTED_ACCESS_R_PASSWORD;
    }
#endif

#if defined(FMSTR_RESTRICTED_ACCESS_RW_PASSWORD)
    if (requiredAccess >= FMSTR_RESTRICTED_ACCESS_RW)
    {
        password = (FMSTR_CHAR *)FMSTR_RESTRICTED_ACCESS_RW_PASSWORD;
    }
#endif

#if defined(FMSTR_RESTRICTED_ACCESS_RWF_PASSWORD)
    if (requiredAccess >= FMSTR_RESTRICTED_ACCESS_RWF)
    {
        password = (FMSTR_CHAR *)FMSTR_RESTRICTED_ACCESS_RWF_PASSWORD;
    }
#endif

    /* password remains NULL if requested level and any lower level are unprotected */
    return password;
}

/******************************************************************************
 *
 * @brief    Handling FMSTR_CMD_AUTH1 command
 *
 * @param    session - transport session
 * @param    msgBuffIO - original command (in) and response buffer (out)
 * @param    retStatus - response status
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response data payload is finished
 *
 ******************************************************************************/

#if FMSTR_CFG_F1_RESTRICTED_ACCESS != 0

FMSTR_BPTR _FMSTR_AuthenticationStep1(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus)
{
    FMSTR_BPTR response = msgBuffIO;
    FMSTR_U8 reqAccess  = 0;

    FMSTR_ASSERT(session != NULL);

    /* Get the Required access from incomming buffer */
    msgBuffIO = FMSTR_ValueFromBuffer8(&reqAccess, msgBuffIO);

    /* success */
    *retStatus = FMSTR_STS_OK | FMSTR_STSF_VARLEN;

    /* Check if password is needed for requested access. When logging-out, the client sends 0 as requested level. */
    if (reqAccess == 0U || _FMSTR_GetAccessPassword(reqAccess) == NULL)
    {
        /* no password is required, access is granted, we do not require the step 2 */
        session->restr.grantedAccess = reqAccess;

        FMSTR_FreeSession(session);

        return FMSTR_ValueToBuffer8(response, 0U);
    }

    /* ULEB-encoded algorithm ID */
    response = FMSTR_SizeToBuffer(response, FMSTR_AUTHENT_PRTCL_SHA1);

    /* Generate random salt and store it to local buffer for later use with FMSTR_AuthenticationM2 handler. */
    (void)FMSTR_RandomNumbersToBuffer(session->restr.authSalt, sizeof(session->restr.authSalt));
    return FMSTR_CopyToBuffer(response, session->restr.authSalt, sizeof(session->restr.authSalt));
}

#endif /* FMSTR_CFG_F1_RESTRICTED_ACCESS */

/******************************************************************************
 *
 * @brief    Handling FMSTR_CMD_AUTH2 command
 *
 * @param    session - transport session
 * @param    msgBuffIO - original command (in) and response buffer (out)
 * @param    retStatus - response status
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response data payload is finished
 *
 ******************************************************************************/

#if FMSTR_CFG_F1_RESTRICTED_ACCESS != 0

FMSTR_BPTR _FMSTR_AuthenticationStep2(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus)
{
    FMSTR_BPTR response = msgBuffIO;
    FMSTR_U8 reqAccess  = 0;
    FMSTR_U8 access;
    const FMSTR_CHAR *pass;

    /* Get the Required access from incomming buffer */
    msgBuffIO = FMSTR_ValueFromBuffer8(&reqAccess, msgBuffIO);

    /* Get access key provided by client */
    msgBuffIO = FMSTR_CopyFromBuffer(session->restr.accessKey, msgBuffIO, sizeof(session->restr.accessKey));
    FMSTR_UNUSED(msgBuffIO);

    /* Evaluate all levels (from 3..1) */
    for (access = FMSTR_RESTRICTED_ACCESS_RWF; access >= FMSTR_RESTRICTED_ACCESS_R; access--)
    {
        if ((pass = _FMSTR_GetAccessPassword(access)) == NULL)
        {
            break; /* this access level and all lower levels are open (can be granted) */
        }

#if FMSTR_USE_HASHED_PASSWORDS > 0
        /* Password is already hashed */
        FMSTR_MemCpyFrom(session->restr.localKey, (FMSTR_ADDR)pass, sizeof(session->restr.localKey));
#else
        /* Password was provided as a string, compute SHA1 hash now */
        FMSTR_MemSet(session->restr.localKey, 0, sizeof(session->restr.localKey));
        FMSTR_Sha1Init(&fmstr_sha1Ctx);
        FMSTR_Sha1Update(&fmstr_sha1Ctx, (const FMSTR_U8 *)pass, FMSTR_StrLen(pass));
        FMSTR_Sha1Final(&fmstr_sha1Ctx, session->restr.localKey);
#endif

        /* compute correct key as SHA1(salt + SHA1(password) + salt) */
        FMSTR_Sha1Init(&fmstr_sha1Ctx);
        FMSTR_Sha1Update(&fmstr_sha1Ctx, session->restr.authSalt, sizeof(session->restr.authSalt));
        FMSTR_Sha1Update(&fmstr_sha1Ctx, session->restr.localKey, sizeof(session->restr.localKey));
        FMSTR_Sha1Update(&fmstr_sha1Ctx, session->restr.authSalt, sizeof(session->restr.authSalt));
        FMSTR_Sha1Final(&fmstr_sha1Ctx, session->restr.localKey);

        /* is access key valid? (i.e. is client's password valid?) */
        if (FMSTR_MemCmp(session->restr.localKey, session->restr.accessKey, sizeof(session->restr.localKey)) == 0)
        {
            break;
        }
    }

    /* Here, the access is the maximum level which is possible to grant with the password provided */

    /* Do not grant higher level than requested */
    session->restr.grantedAccess = access <= reqAccess ? access : reqAccess;

    /* Return success if at least some access level has been granted. Or when user was only validating the password with
     * level=0. */
    if (access != 0U || reqAccess == 0U)
    {
        *retStatus = FMSTR_STS_OK;
        response   = FMSTR_ValueToBuffer8(response, session->restr.grantedAccess);
    }
    else
    {
        *retStatus = FMSTR_STC_EPASS;
    }

    return response;
}

#endif /* FMSTR_CFG_F1_RESTRICTED_ACCESS */

/******************************************************************************
 *
 * @brief    Handling READMEM command
 *
 * @param    session - transport session
 * @param    msgBuffIO - original command (in) and response buffer (out)
 * @param    retStatus - response status
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response data payload is finished
 *
 ******************************************************************************/

FMSTR_BPTR _FMSTR_ReadMem(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus)
{
    FMSTR_BPTR response = msgBuffIO;
    FMSTR_ADDR addr;
    FMSTR_SIZE size;

#if FMSTR_CFG_F1_RESTRICTED_ACCESS != 0
    if (session->restr.grantedAccess < FMSTR_RESTRICTED_ACCESS_R)
    {
        *retStatus = FMSTR_STC_EAUTH;
        return response;
    }
#else
    FMSTR_UNUSED(session);
#endif

    /* Get the Address from incomming buffer */
    msgBuffIO = FMSTR_AddressFromBuffer(&addr, msgBuffIO);
    /* Get the Size from incomming buffer */
    msgBuffIO = FMSTR_SizeFromBuffer(&size, msgBuffIO);

#if FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY
    if (FMSTR_CheckTsaSpace(addr, size, FMSTR_FALSE) == FMSTR_FALSE)
    {
        *retStatus = FMSTR_STC_EACCESS;
        return response;
    }
#endif

    /* check the response will safely fit into comm buffer */
    if (size > (FMSTR_U16)FMSTR_COMM_BUFFER_SIZE)
    {
        *retStatus = FMSTR_STC_RSPBUFFOVF;
        return response;
    }

    /* success  */
    *retStatus = FMSTR_STS_OK;
    return FMSTR_CopyToBuffer(response, addr, size);
}

/******************************************************************************
 *
 * @brief    Handling READMEM command with base address
 *
 * @param    session - transport session
 * @param    msgBuffIO - original command (in) and response buffer (out)
 * @param    retStatus - response status
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response data payload is finished
 *
 ******************************************************************************/

#ifdef FMSTR_PLATFORM_BASE_ADDRESS

FMSTR_BPTR _FMSTR_ReadMemBaseAddress(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus)
{
    FMSTR_BPTR response = msgBuffIO;
    FMSTR_ADDR addr;
    FMSTR_SIZE size;
    FMSTR_INDEX index;

#if FMSTR_CFG_F1_RESTRICTED_ACCESS != 0
    if (session->restr.grantedAccess < FMSTR_RESTRICTED_ACCESS_R)
    {
        *retStatus = FMSTR_STC_EAUTH;
        return response;
    }
#else
    FMSTR_UNUSED(session);
#endif

    /* Get the Relative address to base from incoming buffer */
    msgBuffIO = FMSTR_IndexFromBuffer(&index, msgBuffIO);
    /* Get the Size from incomming buffer */
    msgBuffIO = FMSTR_SizeFromBuffer(&size, msgBuffIO);

    addr = (FMSTR_ADDR)(FMSTR_PLATFORM_BASE_ADDRESS + index);

#if FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY
    if (FMSTR_CheckTsaSpace(addr, size, FMSTR_FALSE) == FMSTR_FALSE)
    {
        *retStatus = FMSTR_STC_EACCESS;
        return response;
    }
#endif

    /* check the response will safely fit into comm buffer */
    if (size > (FMSTR_U16)FMSTR_COMM_BUFFER_SIZE)
    {
        *retStatus = FMSTR_STC_RSPBUFFOVF;
        return response;
    }

    /* success  */
    *retStatus = FMSTR_STS_OK;
    return FMSTR_CopyToBuffer(response, addr, size);
}
#endif /* FMSTR_PLATFORM_BASE_ADDRESS */

/******************************************************************************
 *
 * @brief    Handling WRITEMEM and WRITEMEM_EX commands
 *
 * @param    session - transport session
 * @param    msgBuffIO - original command (in) and response buffer (out)
 * @param    retStatus - response status
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response data payload is finished
 *
 ******************************************************************************/

FMSTR_BPTR _FMSTR_WriteMem(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus)
{
    FMSTR_BPTR response = msgBuffIO;
    FMSTR_ADDR addr;
    FMSTR_SIZE size;
    FMSTR_U8 respCode, flags;

#if FMSTR_CFG_F1_RESTRICTED_ACCESS != 0
    if (session->restr.grantedAccess < FMSTR_RESTRICTED_ACCESS_RW)
    {
        *retStatus = FMSTR_STC_EAUTH;
        return response;
    }
#else
    FMSTR_UNUSED(session);
#endif

    /* Get the Flags from incomming buffer */
    msgBuffIO = FMSTR_ValueFromBuffer8(&flags, msgBuffIO);
    /* Get the Address from incomming buffer */
    msgBuffIO = FMSTR_AddressFromBuffer(&addr, msgBuffIO);
    /* Get the Size from incomming buffer */
    msgBuffIO = FMSTR_SizeFromBuffer(&size, msgBuffIO);

#if FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY
    if (FMSTR_CheckTsaSpace(addr, size, FMSTR_TRUE) == FMSTR_FALSE)
    {
        respCode = FMSTR_STC_EACCESS;
        goto FMSTR_WriteMem_exit;
    }
#endif

    // TODO Add flash write support FLAG FMSTR_WRMFLAG_WRFLASH

    /*lint -e{534} ignoring function return value */
    if ((flags & FMSTR_WRMFLAG_WITHMASK) != 0U)
    {
        FMSTR_CopyFromBufferWithMask(addr, msgBuffIO, size);
    }
    else
    {
        msgBuffIO = FMSTR_CopyFromBuffer(addr, msgBuffIO, size);
        FMSTR_UNUSED(msgBuffIO);
    }

    respCode = FMSTR_STS_OK;

#if FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY
FMSTR_WriteMem_exit:
#endif

    *retStatus = respCode;

    return response;
}

#if FMSTR_SESSION_COUNT > 1
/******************************************************************************
 *
 * @brief    Get feature structure from the static array of features
 *
 * @param    featureType - feature type
 * @param    instance - instance number of the selected feature
 *
 * @return   Selected feature struct
 *
 ******************************************************************************/

static FMSTR_FEATURE_LOCK *_FMSTR_FeatureGet(FMSTR_U8 featureType, FMSTR_U8 instance)
{
    FMSTR_INDEX index = -1;

    switch (featureType)
    {
#if FMSTR_USE_SCOPE > 0
        case FMSTR_FEATURE_SCOPE:
            if (instance < _FMSTR_SC)
            {
                /* Coverity: Intentional constant type cast. */
                /* coverity[misra_c_2012_rule_10_8_violation:FALSE] */
                index = (FMSTR_INDEX)FMSTR_FEATURE_SCOPE_INSTANCE(instance);
            }
            break;
#endif

#if FMSTR_USE_RECORDER > 0
        case FMSTR_FEATURE_REC:
            if (instance < _FMSTR_RC)
            {
                /* coverity[misra_c_2012_rule_10_8_violation:FALSE] */
                index = (FMSTR_INDEX)FMSTR_FEATURE_REC_INSTANCE(instance);
            }
            break;
#endif

#if FMSTR_USE_APPCMD > 0
        case FMSTR_FEATURE_APPCMD:
            if (instance < _FMSTR_AC)
            {
                /* coverity[misra_c_2012_rule_10_8_violation:FALSE] */
                index = (FMSTR_INDEX)FMSTR_FEATURE_APPCMD_INSTANCE(instance);
            }
            break;
#endif

        case FMSTR_FEATURE_FLASHPROG:
            if (instance < _FMSTR_FPG)
            {
                /* coverity[misra_c_2012_rule_10_8_violation:FALSE] */
                index = (FMSTR_INDEX)FMSTR_FEATURE_FLASHPROG_INSTANCE(instance);
            }
            break;

#if FMSTR_USE_PIPES > 0
        case FMSTR_FEATURE_PIPE:
        {
            FMSTR_INDEX pipeIndex = FMSTR_FindPipeIndex(instance);
            if (pipeIndex < (FMSTR_INDEX)_FMSTR_PC)
            {
                /* coverity[misra_c_2012_rule_10_8_violation:FALSE] */
                index = (FMSTR_INDEX)FMSTR_FEATURE_PIPE_INSTANCE(pipeIndex);
            }
        }
        break;
#endif

        default:
            index = -1;
            break;
    }

    /* Coverity: Check max size, always check the index is within array. */
    /* coverity[misra_c_2012_rule_14_3_violation:FALSE] */
    if (index < 0 || index > (FMSTR_INDEX)FMSTR_FEATURE_LOCK_COUNT)
    {
        return NULL;
    }

    return &fmstr_features[index];
}

/******************************************************************************
 *
 * @brief    Lock or unlock feature in a multi-session configuration
 *
 * @param    session - transport session
 * @param    msgBuffIO - original command (in) and response buffer (out)
 * @param    retStatus - response status
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response data payload is finished
 *
 ******************************************************************************/

static FMSTR_BPTR _FMSTR_FeatureLock(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus)
{
    FMSTR_BPTR response         = msgBuffIO;
    FMSTR_FEATURE_LOCK *feature = NULL;
    FMSTR_U8 featType           = 0;
    FMSTR_U8 instance           = 0;
    FMSTR_U8 lock               = 0;

#if FMSTR_CFG_F1_RESTRICTED_ACCESS != 0
    if (session->restr.grantedAccess < FMSTR_RESTRICTED_ACCESS_R)
    {
        *retStatus = FMSTR_STC_EAUTH;
        return response;
    }
#endif

    /* Get the Feature Type from incomming buffer */
    msgBuffIO = FMSTR_ValueFromBuffer8(&featType, msgBuffIO);
    /* Get the Instance from incomming buffer */
    msgBuffIO = FMSTR_ValueFromBuffer8(&instance, msgBuffIO);
    /* Get the Lock/Unlock from incomming buffer */
    msgBuffIO = FMSTR_ValueFromBuffer8(&lock, msgBuffIO);

    /* Get feature struct */
    feature = _FMSTR_FeatureGet(featType, instance);
    if (feature == NULL)
    {
        /* Invalid instance */
        *retStatus = FMSTR_STC_EINSTANCE;
        return response;
    }

    /* Check locked session */
    if (feature->lockCount > 0 && feature->owner != session)
    {
        /* Feature is locked by another session */
        *retStatus = FMSTR_STC_SERVBUSY;
        return response;
    }

    /* Lock */
    if (lock != 0U)
    {
        feature->lockCount++;
        feature->owner = session;
    }
    /* Unlock */
    else
    {
        if (feature->lockCount > 0)
        {
            feature->lockCount--;
        }
        if (feature->lockCount == 0)
        {
            feature->owner = NULL;
        }
    }

    *retStatus = FMSTR_STS_OK;

    return response;
}

/******************************************************************************
 *
 * @brief    Unlock all features, which is owned by session
 *
 * @param    session - transport session
 *
 ******************************************************************************/

static void _FMSTR_UnlockFeatures(FMSTR_SESSION *session)
{
    FMSTR_FEATURE_LOCK *feature = NULL;
    FMSTR_U8 i;

    for (i = 0; i < FMSTR_FEATURE_LOCK_COUNT; i++)
    {
        feature = &fmstr_features[i];
        if (feature->owner == session)
        {
            feature->lockCount = 0;
            feature->owner     = NULL;
        }
    }
}

/******************************************************************************
 *
 * @brief    Check if feature is locked for the owner (session)
 *
 * @param    session - transport session
 * @param    featureType - feature type
 * @param    instance - instance number of the selected feature
 *
 * @return   locked state
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_IsFeatureOwned(FMSTR_SESSION *session, FMSTR_U8 featureType, FMSTR_U8 instance)
{
    FMSTR_FEATURE_LOCK *feature = NULL;

    /* Get feature struct */
    feature = _FMSTR_FeatureGet(featureType, instance);
    if (feature == NULL)
    {
        return FMSTR_FALSE;
    }

    if (feature->owner == session && feature->lockCount > 0)
    {
        return FMSTR_TRUE;
    }

    return FMSTR_FALSE;
}
#endif /* FMSTR_SESSION_COUNT > 1 */

/******************************************************************************
 *
 * @brief    Free transport session
 *
 * @param    identification - transport identification
 *
 ******************************************************************************/

void FMSTR_FreeSession(void *identification)
{
    FMSTR_SESSION *ses = _FMSTR_FindSession(identification, FMSTR_FALSE);
    if (ses != NULL && ses->identification == identification)
    {
#if FMSTR_SESSION_COUNT > 1
        /* Unlock all features, which is owned by session */
        _FMSTR_UnlockFeatures(ses);
#endif

        FMSTR_MemSet(ses, 0, sizeof(FMSTR_SESSION));
    }
}

/******************************************************************************
 *
 * @brief    Find transport session by transport identification
 *
 * @param    identification - transport identification
 * @param    create - when true, creates session with transport identification
 *
 * @return   Found/created session.
 *
 ******************************************************************************/

static FMSTR_SESSION *_FMSTR_FindSession(void *identification, FMSTR_BOOL create)
{
    FMSTR_SESSION *ses;
    FMSTR_SESSION *freeSession = NULL;
    FMSTR_INDEX i;

    for (i = 0; i < FMSTR_SESSION_COUNT; i++)
    {
        ses = &fmstr_sessions[i];

        /* Find session by identification */
        if (ses->identification == identification)
        {
            return ses;
        }

        /* Looking for the first free session */
        /* Coverity: Intentional, may be always true when FMSTR_SESSION_COUNT==1. */
        /* coverity[misra_c_2012_rule_14_3_violation:FALSE] */
        if (freeSession == NULL && ses->identification == NULL)
        {
            freeSession = ses;
        }
    }

    /* Set identification to session */
    if (freeSession != NULL && create != FMSTR_FALSE)
    {
        freeSession->identification = identification;

#if FMSTR_CFG_F1_RESTRICTED_ACCESS != 0
        /* Initialize the access protection variables */
        FMSTR_MemSet(freeSession->restr.authSalt, 0xaa, sizeof(freeSession->restr.authSalt));
        freeSession->restr.grantedAccess = fmstr_dfltGrantedAccess;
#endif
    }

    return freeSession;
}

#else /* !FMSTR_DISABLE */

/******************************************************************************
 *
 * @brief    FreeMASTER driver initialization is disabled
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_Init(void)
{
    return FMSTR_FALSE;
}

/*******************************************************************************
 *
 * @brief    API: Main "Polling" call from the application main loop
 *
 *******************************************************************************/

void FMSTR_Poll(void)
{
}

#endif /* !FMSTR_DISABLE */
