/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2018-2020, 2024-2025 NXP
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
 * FreeMASTER Communication Driver - Application Commands handling
 */

#include "freemaster.h"
#include "freemaster_private.h"
#include "freemaster_protocol.h"

#if FMSTR_USE_APPCMD > 0 && FMSTR_DISABLE == 0

/***********************************
 *  local variables
 ***********************************/

static FMSTR_APPCMD_CODE fmstr_appCmd;                             /* app.cmd code (to application) */
static FMSTR_APPCMD_DATA fmstr_appCmdBuff[FMSTR_APPCMD_BUFF_SIZE]; /* app.cmd data buffer  */
static FMSTR_SIZE fmstr_appCmdLen;                                 /* app.cmd data length */

static FMSTR_APPCMD_RESULT fmstr_appCmdResult; /* app.cmd result code (from application) */
static FMSTR_SIZE8 fmstr_appCmdResultDataLen;

#if FMSTR_MAX_APPCMD_CALLS > 0
static FMSTR_APPCMD_CODE fmstr_appCmdCallId[FMSTR_MAX_APPCMD_CALLS];   /* registered callback commands */
static FMSTR_PAPPCMDFUNC fmstr_appCmdCallFunc[FMSTR_MAX_APPCMD_CALLS]; /* registered callback handlers */
#endif

/***********************************
 *  local functions
 ***********************************/

static FMSTR_INDEX FMSTR_FindAppCmdCallIndex(FMSTR_APPCMD_CODE appCmdCode);

/******************************************************************************
 *
 * @brief    Initialize app.cmds interface
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_InitAppCmds(void)
{
#if FMSTR_MAX_APPCMD_CALLS > 0
    FMSTR_INDEX i;

    for (i = 0; i < FMSTR_MAX_APPCMD_CALLS; i++)
    {
        fmstr_appCmdCallId[i]   = FMSTR_APPCMDRESULT_NOCMD;
        fmstr_appCmdCallFunc[i] = NULL;
    }
#endif

    fmstr_appCmd       = (FMSTR_APPCMD_CODE)FMSTR_APPCMDRESULT_NOCMD;
    fmstr_appCmdResult = (FMSTR_APPCMD_RESULT)FMSTR_APPCMDRESULT_NOCMD;

    return FMSTR_TRUE;
}

/******************************************************************************
 *
 * @brief    Handling SANDAPPCMD command
 *
 * @param    session - transport session
 * @param    msgBuffIO - original command (in) and response buffer (out)
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response output finished (except checksum)
 *
 ******************************************************************************/

FMSTR_BPTR FMSTR_StoreAppCmd(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_SIZE msgSize, FMSTR_U8 *retStatus)
{
    FMSTR_BPTR response = msgBuffIO;
    FMSTR_SIZE argsLen  = 0;
    FMSTR_U8 code;

    FMSTR_ASSERT(msgBuffIO != NULL);
    FMSTR_ASSERT(retStatus != NULL);

#if FMSTR_SESSION_COUNT > 1
    /* Is feature locked by me */
    if (FMSTR_IsFeatureOwned(session, FMSTR_FEATURE_APPCMD, 0) == FMSTR_FALSE)
    {
        *retStatus = FMSTR_STC_SERVBUSY;
        return response;
    }
#else
    FMSTR_UNUSED(session);
#endif

    /* the previous command not yet processed */
    if (fmstr_appCmd != ((FMSTR_APPCMD_CODE)FMSTR_APPCMDRESULT_NOCMD))
    {
        *retStatus = FMSTR_STC_SERVBUSY;
        return response;
    }

    /* sanity check */
    if (msgSize == 0U)
    {
        *retStatus = FMSTR_STC_INVSIZE;
        return response;
    }

    /* args len is datalen minus one */
    argsLen = msgSize - 1U;

    /* does the application command fit to buffer ? */
    if (argsLen > (FMSTR_SIZE)FMSTR_APPCMD_BUFF_SIZE)
    {
        *retStatus = FMSTR_STC_INVBUFF;
        return response;
    }

    /* Fetch command code */
    msgBuffIO = FMSTR_ValueFromBuffer8(&code, msgBuffIO);

    /* store command data into dedicated buffer */
    fmstr_appCmd    = code;
    fmstr_appCmdLen = argsLen;

    /* data copy */
    if (argsLen != 0U)
    {
        FMSTR_ADDR appCmdBuffAddr = (FMSTR_ADDR)fmstr_appCmdBuff;

        /*lint -e{534} ignoring return value */
        msgBuffIO = FMSTR_CopyFromBuffer(appCmdBuffAddr, msgBuffIO, (FMSTR_SIZE8)argsLen);
        FMSTR_UNUSED(msgBuffIO);
    }

    /* mark command as "running" (without any response data) */
    fmstr_appCmdResult        = (FMSTR_APPCMD_RESULT)FMSTR_APPCMDRESULT_RUNNING;
    fmstr_appCmdResultDataLen = 0U;

    /* success  */
    *retStatus = FMSTR_STS_OK;
    return response;
}

/******************************************************************************
 *
 * @brief    Handling GETAPPCMDSTS command
 *
 * @param    session - transport session
 * @param    msgBuffIO - original command (in) and response buffer (out)
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response output finished (except checksum)
 *
 * @note The callback-registered commands are processed at the moment the PC
 *       tries to get the result for the first time. At this moment, we are
 *       sure the PC already got the command delivery packet acknowledged.
 *
 ******************************************************************************/

FMSTR_BPTR FMSTR_GetAppCmdStatus(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus)
{
#if FMSTR_MAX_APPCMD_CALLS > 0
    FMSTR_PAPPCMDFUNC func = NULL;
    FMSTR_INDEX index;

    FMSTR_ASSERT(msgBuffIO != NULL);
    FMSTR_ASSERT(retStatus != NULL);

#if FMSTR_SESSION_COUNT > 1
    /* Is feature locked by me */
    if (FMSTR_IsFeatureOwned(session, FMSTR_FEATURE_APPCMD, 0) == FMSTR_FALSE)
    {
        *retStatus = FMSTR_STC_SERVBUSY;
        return msgBuffIO;
    }
#else
    FMSTR_UNUSED(session);
#endif

    /* time to execute the command's callback */
    if ((index = FMSTR_FindAppCmdCallIndex(fmstr_appCmd)) >= 0)
    {
        func = fmstr_appCmdCallFunc[index];
    }

    /* valid callback function found? */
    if (func != NULL)
    {
        /* do execute callback, return value is app.cmd result code */
        fmstr_appCmdResult = func(fmstr_appCmd, fmstr_appCmdBuff, fmstr_appCmdLen);

        /* nothing more to do with this command (i.e. command acknowledged) */
        fmstr_appCmd = FMSTR_APPCMDRESULT_NOCMD;
    }
#endif

    /* success  */
    *retStatus = FMSTR_STS_OK;
    return FMSTR_ValueToBuffer8(msgBuffIO, (FMSTR_U8)fmstr_appCmdResult);
}

/******************************************************************************
 *
 * @brief    Handling GETAPPCMDDATA command
 *
 * @param    session - transport session
 * @param    msgBuffIO - original command (in) and response buffer (out)
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response output finished (except checksum)
 *
 ******************************************************************************/

FMSTR_BPTR FMSTR_GetAppCmdRespData(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus)
{
    FMSTR_BPTR response = msgBuffIO;
    FMSTR_SIZE dataLen;
    FMSTR_SIZE dataOffset;

    FMSTR_ASSERT(msgBuffIO != NULL);
    FMSTR_ASSERT(retStatus != NULL);

#if FMSTR_SESSION_COUNT > 1
    /* Is feature locked by me */
    if (FMSTR_IsFeatureOwned(session, FMSTR_FEATURE_APPCMD, 0) == FMSTR_FALSE)
    {
        *retStatus = FMSTR_STC_SERVBUSY;
        return response;
    }
#else
    FMSTR_UNUSED(session);
#endif

    /* the previous command not yet processed */
    if (fmstr_appCmd != ((FMSTR_APPCMD_CODE)FMSTR_APPCMDRESULT_NOCMD))
    {
        *retStatus = FMSTR_STC_SERVBUSY;
        return response;
    }

    /* Get the data len from incoming buffer */
    msgBuffIO = FMSTR_SizeFromBuffer(&dataLen, msgBuffIO);

    /* Get the data offset from incoming buffer */
    msgBuffIO = FMSTR_SizeFromBuffer(&dataOffset, msgBuffIO);

    /* the response would not fit into comm buffer */
    if (dataLen > (FMSTR_U16)FMSTR_COMM_BUFFER_SIZE)
    {
        *retStatus = FMSTR_STC_RSPBUFFOVF;
        return response;
    }

    /* Coverity: Potential sum wrap would be detected by the compare operation. */
    /* coverity[cert_int30_c_violation:FALSE] */
    if((dataOffset + dataLen) <= dataOffset)
    {
        *retStatus = FMSTR_STC_INVSIZE;
        return response;
    }

    /* prevent data to be fetched outside the app.cmd response data */
    if ((FMSTR_SIZE)(dataOffset + dataLen) > (FMSTR_SIZE)fmstr_appCmdResultDataLen)
    {
        *retStatus = FMSTR_STC_INVSIZE;
        return response;
    }

    /* copy to buffer */
    {
        FMSTR_ADDR appCmdBuffAddr = (FMSTR_ADDR)(fmstr_appCmdBuff + dataOffset);
        response = FMSTR_CopyToBuffer(response, appCmdBuffAddr, (FMSTR_SIZE8)dataLen);
    }

    /* success  */
    *retStatus = FMSTR_STS_OK | FMSTR_STSF_VARLEN;
    return response;
}

/******************************************************************************
 *
 * @brief    Find index of registered app.cmd callback
 *
 * @param    appCmdCode - App. command ID
 *
 * @return   Index of function pointer in our local tables
 *
 ******************************************************************************/

static FMSTR_INDEX FMSTR_FindAppCmdCallIndex(FMSTR_APPCMD_CODE appCmdCode)
{
#if FMSTR_MAX_APPCMD_CALLS > 0
    FMSTR_INDEX i;

    for (i = 0; i < FMSTR_MAX_APPCMD_CALLS; i++)
    {
        if (fmstr_appCmdCallId[i] == appCmdCode)
        {
            return i;
        }
    }
#else
    /*lint -esym(528, FMSTR_FindAppCmdCallIndex) this function is
      not referenced when APPCMD_CALLS are not used */
    FMSTR_UNUSED(appCmdCode);
#endif

    return -1;
}

/******************************************************************************
 *
 * @brief    API: Mark the application command is processed by the application
 *
 * @param    resultCode - the result code which is returned to a host
 *
 ******************************************************************************/

void FMSTR_AppCmdAck(FMSTR_APPCMD_RESULT resultCode)
{
    fmstr_appCmdResult        = resultCode;
    fmstr_appCmdResultDataLen = 0U;

    /* waiting for a new command to come */
    fmstr_appCmd = (FMSTR_APPCMD_CODE)FMSTR_APPCMDRESULT_NOCMD;
}

/******************************************************************************
 *
 * @brief    API: Mark the application command is processed by the application
 *
 * @param    pResultDataAddr - address of data we want to return to the PC
 * @param    resultDataLen - length of return data
 *
 ******************************************************************************/

void FMSTR_AppCmdSetResponseData(FMSTR_ADDR resultDataAddr, FMSTR_SIZE resultDataLen)
{
    /* any data supplied by user? */
    if (FMSTR_ADDR_VALID(resultDataAddr) != FMSTR_FALSE)
    {
        /* command response shall fit into SIZE8 */
        FMSTR_ASSERT(((FMSTR_U32)FMSTR_APPCMD_BUFF_SIZE) <= 0xffUL);

        /* response data length is trimmed if response data would not fit into buffer */
        if(resultDataLen > (FMSTR_SIZE)FMSTR_APPCMD_BUFF_SIZE)
        {
            resultDataLen = (FMSTR_SIZE)FMSTR_APPCMD_BUFF_SIZE;
        }

        /* remember the stored data size */
        fmstr_appCmdResultDataLen = (FMSTR_SIZE8)resultDataLen;
        if (fmstr_appCmdResultDataLen > 0U)
        {
            FMSTR_ADDR appCmdBuffAddr = (FMSTR_ADDR)fmstr_appCmdBuff;
            FMSTR_MemCpyFrom(appCmdBuffAddr, resultDataAddr, fmstr_appCmdResultDataLen);
        }
    }
    else
    {
        /* no data being returned at all (same effect as pure FMSTR_AppCmdAck) */
        fmstr_appCmdResultDataLen = 0U;
    }
}

/******************************************************************************
 *
 * @brief    API: Fetch the application command code if one is ready for processing
 *
 * @return   A command code stored in the application cmd buffer.
 *           The return value is FMSTR_APPCMDRESULT_NOCMD if there is no
 *           new command since the last call to FMSTR_AppCmdAck
 *
 ******************************************************************************/

FMSTR_APPCMD_CODE FMSTR_GetAppCmd(void)
{
#if FMSTR_MAX_APPCMD_CALLS > 0
    /* the user can never see the callback-registered commands */
    if (FMSTR_FindAppCmdCallIndex(fmstr_appCmd) >= 0)
    {
        return FMSTR_APPCMDRESULT_NOCMD;
    }
#endif

    /* otherwise, return the appcomand pending */
    return fmstr_appCmd;
}

/******************************************************************************
 *
 * @brief    API: Get a pointer to application command data
 *
 * @param    dataLen - A pointer to variable which receives the data length
 *
 * @return   Pointer to the "application command" data
 *
 ******************************************************************************/

FMSTR_APPCMD_PDATA FMSTR_GetAppCmdData(FMSTR_SIZE *dataLen)
{
    /* no command, no data */
    if (fmstr_appCmd == ((FMSTR_APPCMD_CODE)FMSTR_APPCMDRESULT_NOCMD))
    {
        return NULL;
    }

#if FMSTR_MAX_APPCMD_CALLS > 0
    /* the user never sees the callback-registered commands */
    if (FMSTR_FindAppCmdCallIndex(fmstr_appCmd) >= 0)
    {
        return NULL;
    }
#endif

    /* caller want to know the data length */
    if (dataLen != NULL)
    {
        *dataLen = fmstr_appCmdLen;
    }

    /* data are saved in out buffer */
    return fmstr_appCmdLen > 0U ? fmstr_appCmdBuff : (FMSTR_APPCMD_PDATA)NULL;
}

/******************************************************************************
 *
 * @brief    API: Register or unregister app.cmd callback handler
 *
 * @param    appCmdCode   - App.command ID
 * @param    callbackFunc - Pointer to handler function (NULL to unregister)
 *
 * @return   Non-zero if successfull, zero if maximum callbacks already set
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_RegisterAppCmdCall(FMSTR_APPCMD_CODE appCmdCode, FMSTR_PAPPCMDFUNC callbackFunc)
{
#if FMSTR_MAX_APPCMD_CALLS > 0

    FMSTR_INDEX index;

    /* keep "void" ID as reserved */
    if (appCmdCode == FMSTR_APPCMDRESULT_NOCMD)
    {
        return FMSTR_FALSE;
    }

    /* get index of app.cmd ID (if already set) */
    index = FMSTR_FindAppCmdCallIndex(appCmdCode);

    /* when not found, get a free slot (only if registering new callback) */
    if ((index < 0) && (callbackFunc != NULL))
    {
        index = FMSTR_FindAppCmdCallIndex(FMSTR_APPCMDRESULT_NOCMD);
    }

    /* failed? */
    if (index < 0)
    {
        return FMSTR_FALSE;
    }

    /* register handler */
    fmstr_appCmdCallFunc[index] = callbackFunc;
    fmstr_appCmdCallId[index]   = (FMSTR_APPCMD_CODE)(callbackFunc != NULL ? appCmdCode : FMSTR_APPCMDRESULT_NOCMD);

    return FMSTR_TRUE;

#else
    FMSTR_UNUSED(callbackFunc);
    FMSTR_UNUSED(appCmdCode);

    /* app.cmd callback not implemented */
    return FMSTR_FALSE;

#endif
}

#else /* FMSTR_USE_APPCMD && (!FMSTR_DISABLE) */

/* void Application Command API functions */

void FMSTR_AppCmdAck(FMSTR_APPCMD_RESULT resultCode)
{
    FMSTR_UNUSED(resultCode);
}

void FMSTR_AppCmdSetResponseData(FMSTR_ADDR resultDataAddr, FMSTR_SIZE resultDataLen)
{
    FMSTR_UNUSED(resultDataAddr);
    FMSTR_UNUSED(resultDataLen);
}

FMSTR_APPCMD_CODE FMSTR_GetAppCmd(void)
{
    return (FMSTR_APPCMD_CODE)FMSTR_APPCMDRESULT_NOCMD;
}

FMSTR_APPCMD_PDATA FMSTR_GetAppCmdData(FMSTR_SIZE *dataLen)
{
    FMSTR_UNUSED(dataLen);
    return NULL;
}

FMSTR_BOOL FMSTR_RegisterAppCmdCall(FMSTR_APPCMD_CODE appCmdCode, FMSTR_PAPPCMDFUNC callbackFunc)
{
    FMSTR_UNUSED(appCmdCode);
    FMSTR_UNUSED(callbackFunc);
    return FMSTR_FALSE;
}

/*lint -efile(766, freemaster_protocol.h) include file is not used in this case */

#endif /* FMSTR_USE_APPCMD && (!FMSTR_DISABLE) */
