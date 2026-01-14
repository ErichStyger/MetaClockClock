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
 * FreeMASTER Communication Driver - Oscilloscope implementation
 */

#include "freemaster.h"
#include "freemaster_private.h"
#include "freemaster_protocol.h"
#include "freemaster_utils.h"

#if FMSTR_USE_SCOPE > 0 && FMSTR_DISABLE == 0

/********************************************************
 *  local macros definition
 ********************************************************/
/* Define Protocol operations*/
#define FMSTR_SCOPE_PRTCLSET_OP_CFGMEM 0x01 /* Set number of recorder variables */
#define FMSTR_SCOPE_PRTCLSET_OP_CFGVAR 0x02 /* Setup address and size of one scope variable */

/********************************************************
 *  local types definition
 ********************************************************/

/* Scope instance definition */
typedef struct
{
    FMSTR_U8 varCnt;                          /* number of active scope variables */
    FMSTR_ADDR varAddr[FMSTR_MAX_SCOPE_VARS]; /* addresses of scope variables */
    FMSTR_U8 varSize[FMSTR_MAX_SCOPE_VARS];   /* sizes of scope variables */
} FMSTR_SCOPE;

/********************************************************
 *  local static functions declarations
 ********************************************************/
static FMSTR_U8 _FMSTR_SetScope_CFGMEM(FMSTR_BPTR msgBuffIO, FMSTR_SCOPE *scope);
static FMSTR_U8 _FMSTR_SetScope_CFGVAR(FMSTR_BPTR msgBuffIO, FMSTR_SCOPE *scope, FMSTR_U8 opLen);

/***********************************
 *  local variables
 ***********************************/
static FMSTR_SCOPE fmstr_scopeCfg[FMSTR_USE_SCOPE]; /* Container of all scopes configurations*/

/******************************************************************************
 *
 * @brief    Scope Initialization
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_InitScope(void)
{
    FMSTR_MemSet(fmstr_scopeCfg, 0, sizeof(fmstr_scopeCfg));
    return FMSTR_TRUE;
}

/******************************************************************************
 *
 * @brief    Handling FMSTR_CMD_SETSCOPE Memory configuration command
 *
 * @param    msgBuffIO   - original command (in) and response buffer (out)
 * @param    scope       - pointer to scope configuration
 *
 * @return   status of operation usable in protocol
 *
 ******************************************************************************/

static FMSTR_U8 _FMSTR_SetScope_CFGMEM(FMSTR_BPTR msgBuffIO, FMSTR_SCOPE *scope)
{
    FMSTR_U8 varCnt;

    /* Get the active variables count of Scope Instance */
    (void)FMSTR_ValueFromBuffer8(&varCnt, msgBuffIO);

    if (varCnt > (FMSTR_U8)FMSTR_MAX_SCOPE_VARS)
    {
        return FMSTR_STC_INVSIZE;
    }

    /* Initialize the scope configuration */
    FMSTR_MemSet(scope, 0, sizeof(*scope));
    scope->varCnt = varCnt;

    return FMSTR_STS_OK;
}

/******************************************************************************
 *
 * @brief    API: Set up the recorder variable configuration (internal version)
 *
 * @param    recIx - index of recorder
 * @param    recCfg - pointer to recorder configuration
 *
 *
 ******************************************************************************/

static FMSTR_U8 _FMSTR_SetScope_CFGVAR(FMSTR_BPTR msgBuffIO, FMSTR_SCOPE *scope, FMSTR_U8 opLen)
{
    FMSTR_BPTR msgBuffIOStart = msgBuffIO;
    FMSTR_U8 varIx;
    FMSTR_ADDR addr;
    FMSTR_U8 size;

    /* Get the variable index */
    msgBuffIO = FMSTR_ValueFromBuffer8(&varIx, msgBuffIO);
    /* Get the variable address */
    msgBuffIO = FMSTR_AddressFromBuffer(&addr, msgBuffIO);
    /* Get the variable size */
    msgBuffIO = FMSTR_ValueFromBuffer8(&size, msgBuffIO);

    /* Decoded ULEBs should match the expected op_data length */
    if (msgBuffIO != (msgBuffIOStart + opLen))
    {
        return FMSTR_STC_INVSIZE;
    }

    /* Check the variable index  */
    if (varIx >= scope->varCnt)
    {
        return FMSTR_STC_INVBUFF;
    }

    /* Valid numeric variable sizes only */
    if ((size != 1U) && (size != 2U) && (size != 4U) && (size != 8U))
    {
        return FMSTR_STC_INVSIZE;
    }

    /* Check the TSA safety */
#if FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY
    if (FMSTR_CheckTsaSpace(addr, size, FMSTR_FALSE) == FMSTR_FALSE)
    {
        return FMSTR_STC_EACCESS;
    }
#endif /* FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY */

    /* Store the variable configuration */
    scope->varAddr[varIx] = addr;
    scope->varSize[varIx] = size;

    return FMSTR_STS_OK;
}

/******************************************************************************
 *
 * @brief    Handling SETUPSCOPE command
 *
 * @param    session - transport session
 * @param    msgBuffIO - original command (in) and response buffer (out)
 * @param    inputLen - Count of received bytes in input buffer
 * @param    retStatus   - pointer to return status variable
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response output finished (except checksum)
 *
 ******************************************************************************/

FMSTR_BPTR FMSTR_SetScope(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_SIZE inputLen, FMSTR_U8 *retStatus)
{
    FMSTR_SCOPE *scope;
    FMSTR_BPTR response   = msgBuffIO;
    FMSTR_U8 responseCode = FMSTR_STS_OK;
    FMSTR_U8 scopeIndex;

    /* Need at least one byte with oscilloscope index */
    if (inputLen < 1U)
    {
        *retStatus = FMSTR_STC_INVSIZE;
        return response;
    }

    /* Get oscilloscope index */
    msgBuffIO = FMSTR_ValueFromBuffer8(&scopeIndex, msgBuffIO);
    inputLen--;

    if (scopeIndex >= (FMSTR_U8)FMSTR_USE_SCOPE)
    {
        *retStatus = FMSTR_STC_INSTERR;
        return response;
    }

#if FMSTR_SESSION_COUNT > 1
    /* Is feature locked by me */
    if (FMSTR_IsFeatureOwned(session, FMSTR_FEATURE_SCOPE, scopeIndex) == FMSTR_FALSE)
    {
        *retStatus = FMSTR_STC_SERVBUSY;
        return response;
    }
#else
    FMSTR_UNUSED(session);
#endif

    scope = &fmstr_scopeCfg[scopeIndex];

    while (inputLen >= 2U && (responseCode == FMSTR_STS_OK))
    {
        FMSTR_U8 opCode, opLen;

        /* Get Operation Code and data length */
        msgBuffIO = FMSTR_ValueFromBuffer8(&opCode, msgBuffIO);
        msgBuffIO = FMSTR_ValueFromBuffer8(&opLen, msgBuffIO);
        inputLen -= 2U;

        if (((FMSTR_SIZE)opLen) > inputLen)
        {
            *retStatus = FMSTR_STC_INVSIZE;
            return response;
        }

        switch (opCode)
        {
            /* Configure scope memory */
            case FMSTR_SCOPE_PRTCLSET_OP_CFGMEM:
                responseCode = _FMSTR_SetScope_CFGMEM(msgBuffIO, scope);
                break;

            /* Configure variable */
            case FMSTR_SCOPE_PRTCLSET_OP_CFGVAR:
                responseCode = _FMSTR_SetScope_CFGVAR(msgBuffIO, scope, opLen);
                break;

            default:
                responseCode = FMSTR_STC_INVCMD;
                break;
        }

        inputLen -= opLen;
        msgBuffIO += opLen;
    }

    *retStatus = responseCode;
    return response;
}

/******************************************************************************
 *
 * @brief    Handling READSCOPE command
 *
 * @param    session - transport session
 * @param    msgBuffIO - original command (in) and response buffer (out)
 * @param    retStatus   - pointer to return status variable
 * @param    maxOutSize - Maximal size of output data
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response output finished (except checksum)
 *
 ******************************************************************************/

FMSTR_BPTR FMSTR_ReadScope(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus, FMSTR_SIZE maxOutSize)
{
    FMSTR_U8 i;
    FMSTR_U8 scopeIndex;
    FMSTR_SCOPE *scope;
    FMSTR_BPTR msgBuffIOStart = msgBuffIO;

    /* Get recerder index */
    (void)FMSTR_ValueFromBuffer8(&scopeIndex, msgBuffIO);

    /* Check the index of scope if it fits to configuration */
    if (scopeIndex >= (FMSTR_U8)FMSTR_USE_SCOPE)
    {
        *retStatus = FMSTR_STC_INSTERR;
        return msgBuffIO;
    }

#if FMSTR_SESSION_COUNT > 1
    /* Is feature locked by me */
    if (FMSTR_IsFeatureOwned(session, FMSTR_FEATURE_SCOPE, scopeIndex) == FMSTR_FALSE)
    {
        *retStatus = FMSTR_STC_SERVBUSY;
        return msgBuffIO;
    }
#else
    FMSTR_UNUSED(session);
#endif

    /* Get the scope */
    scope = &fmstr_scopeCfg[scopeIndex];

    /* Check if there are defined some variables */
    if (scope->varCnt == 0U)
    {
        *retStatus = FMSTR_STC_NOTINIT;
        return msgBuffIO;
    }

    /* Copy all variables into the output buffer */
    for (i = 0U; i < scope->varCnt; i++)
    {
        /* Check the size of output buffer */
        if (maxOutSize < scope->varSize[i])
        {
            *retStatus = FMSTR_STC_INVSIZE;
            return msgBuffIOStart;
        }
        maxOutSize -= scope->varSize[i];

        /* Copy variable */
        msgBuffIO = FMSTR_CopyToBuffer(msgBuffIO, scope->varAddr[i], scope->varSize[i]);
    }

    *retStatus = FMSTR_STS_OK;

    /* return end position */
    return msgBuffIO;
}

#endif /* (FMSTR_USE_SCOPE) && !(FMSTR_DISABLE) */
