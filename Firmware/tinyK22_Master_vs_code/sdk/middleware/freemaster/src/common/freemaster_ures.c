/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2018-2020, 2024 NXP
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
 * FreeMASTER Communication Driver - User Resources
 */

#include "freemaster.h"
#include "freemaster_private.h"
#include "freemaster_protocol.h"
#include "freemaster_ures.h"

#if FMSTR_USE_TSA > 0 && FMSTR_DISABLE == 0 && FMSTR_USE_URES > 0

static FMSTR_BPTR _FMSTR_UresControlRead(
    FMSTR_ADDR resourceId, FMSTR_BPTR msgBuffIO, FMSTR_SIZE msgSize, FMSTR_BPTR response, FMSTR_U8 *retStatus);
static FMSTR_BPTR _FMSTR_UresControlWrite(
    FMSTR_ADDR resourceId, FMSTR_BPTR msgBuffIO, FMSTR_SIZE msgSize, FMSTR_BPTR response, FMSTR_U8 *retStatus);
static FMSTR_BPTR _FMSTR_UresControlIoctl(
    FMSTR_ADDR resourceId, FMSTR_BPTR msgBuffIO, FMSTR_SIZE msgSize, FMSTR_BPTR response, FMSTR_U8 *retStatus);

static FMSTR_LP_TSA_ENTRY _FMSTR_FindUresInTsaInternal(FMSTR_ADDR resourceId);

/******************************************************************************
 *
 * @brief    Get resource TSA entry - helper function
 *
 * @return   Address of the TSA entry typed to FMSTR_LP_TSA_ENTRY private type
 *
 ******************************************************************************/

static FMSTR_LP_TSA_ENTRY _FMSTR_FindUresInTsaInternal(FMSTR_ADDR resourceId)
{
    FMSTR_ADDR pteAddr = FMSTR_FindUresInTsa(resourceId);
    return (FMSTR_LP_TSA_ENTRY)FMSTR_CAST_ADDR_TO_PTR(pteAddr);
}

/******************************************************************************
 *
 * @brief    Handling User Resources
 *
 * @param    msgBuffIO - original command (in) and response buffer (out)
 * @param    msgSize   - size of data in buffer
 * @param    retStatus - response status
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response output finished (except checksum)
 *
 ******************************************************************************/

FMSTR_BPTR FMSTR_UresControl(FMSTR_BPTR msgBuffIO, FMSTR_SIZE msgSize, FMSTR_U8 *retStatus)
{
    volatile FMSTR_BPTR response = msgBuffIO;
    FMSTR_URES_OP_CODE opCode;
    FMSTR_ADDR resourceId;
    FMSTR_INDEX hdrDiff;
    FMSTR_SIZE hdrSize;

    FMSTR_ASSERT(msgBuffIO != NULL);
    FMSTR_ASSERT(retStatus != NULL);

    /* need at least operation code and resource ID */
    if (msgSize < 2U)
    {
        /* return status  */
        *retStatus = FMSTR_STC_URESERR;
        return response;
    }

    /* Get operation code */
    msgBuffIO = FMSTR_ValueFromBuffer8(&opCode, msgBuffIO);

    /* Get reource ID */
    msgBuffIO = FMSTR_AddressFromBuffer(&resourceId, msgBuffIO);

    /* Where have we stopped while processing? */
    hdrDiff = (FMSTR_INDEX)(msgBuffIO - response);
    hdrSize = (FMSTR_SIZE)hdrDiff;

    /* Handle command by operation code */
    switch (opCode)
    {
        case FMSTR_URES_OP_READ:
            response = _FMSTR_UresControlRead(resourceId, msgBuffIO, msgSize - hdrSize, response, retStatus);
            break;
        case FMSTR_URES_OP_WRITE:
            response = _FMSTR_UresControlWrite(resourceId, msgBuffIO, msgSize - hdrSize, response, retStatus);
            break;
        case FMSTR_URES_OP_IOCTL:
            response = _FMSTR_UresControlIoctl(resourceId, msgBuffIO, msgSize - hdrSize, response, retStatus);
            break;
        default:
            *retStatus = FMSTR_STC_URESERR;
            break;
    }

    return response;
}

/******************************************************************************
 *
 * @brief    Handling User Resources - read
 *
 * @param    resourceId - resource ID in TSA table
 * @param    msgBuffIO - message input buffer
 * @param    msgSize   - size of data in buffer
 * @param    response - message output buffer
 * @param    retStatus - response status
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response output finished (except checksum)
 *
 ******************************************************************************/

static FMSTR_BPTR _FMSTR_UresControlRead(
    FMSTR_ADDR resourceId, FMSTR_BPTR msgBuffIO, FMSTR_SIZE msgSize, FMSTR_BPTR response, FMSTR_U8 *retStatus)
{
    FMSTR_BOOL ret;
    FMSTR_ADDR readOffset;
    FMSTR_SIZE readLen = 0;
    FMSTR_LP_TSA_ENTRY ures;
    FMSTR_URES_HANDLER_FUNC uresFunc;
    FMSTR_RWI_BUFF uresBuffRWI;

    FMSTR_ASSERT(resourceId != NULL);
    FMSTR_ASSERT(msgBuffIO != NULL);
    FMSTR_ASSERT(response != NULL);
    FMSTR_ASSERT(retStatus != NULL);

    /* need at least read offset and read len */
    if (msgSize < 2U)
    {
        /* return status */
        *retStatus = FMSTR_STC_URESERR;
        return response;
    }

    /* Get read offset */
    msgBuffIO = FMSTR_AddressFromBuffer(&readOffset, msgBuffIO);

    /* Get read length */
    msgBuffIO = FMSTR_SizeFromBuffer(&readLen, msgBuffIO);

    /* Find resource in TSA table */
    ures = _FMSTR_FindUresInTsaInternal(resourceId);
    if (ures == NULL)
    {
        /* User resource not found in TSA table */
        /* return status */
        *retStatus = FMSTR_STC_URESERR;
        return response;
    }

    /* Red/write/ioctl buffer */
    uresBuffRWI.buff       = response;
    uresBuffRWI.sizeIn     = 0;
    uresBuffRWI.sizeOut    = 0;
    uresBuffRWI.sizeOutMax = readLen;
    uresBuffRWI.offset     = readOffset;
    uresBuffRWI.ioctlCode  = 0;

    /* Check max size */
    if (uresBuffRWI.sizeOutMax > (FMSTR_U16)FMSTR_COMM_BUFFER_SIZE)
    {
        uresBuffRWI.sizeOutMax = (FMSTR_U16)FMSTR_COMM_BUFFER_SIZE;
    }

    /* User function */
    uresFunc = (FMSTR_URES_HANDLER_FUNC)ures->addr.p;
    if (uresFunc == NULL)
    {
        /* User resource function not exists */
        /* return status */
        *retStatus = FMSTR_STC_URESERR;
        return response;
    }

    /* Call user function */
    ret = uresFunc(FMSTR_URES_OP_READ, &uresBuffRWI, (void *)ures->info.p);
    if (ret == FMSTR_FALSE)
    {
        /* User resource function error */
        /* return status */
        *retStatus = FMSTR_STC_URESERR;
        return response;
    }

    /* Move pointer in response buffer */
    response += uresBuffRWI.sizeOut;

    /* success  */
    *retStatus = FMSTR_STS_OK | FMSTR_STSF_VARLEN;
    return response;
}

/******************************************************************************
 *
 * @brief    Handling User Resources - write
 *
 * @param    resourceId - resource ID in TSA table
 * @param    msgBuffIO - message input buffer
 * @param    msgSize   - size of data in buffer
 * @param    response - message output buffer
 * @param    retStatus - response status
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response output finished (except checksum)
 *
 ******************************************************************************/

static FMSTR_BPTR _FMSTR_UresControlWrite(
    FMSTR_ADDR resourceId, FMSTR_BPTR msgBuffIO, FMSTR_SIZE msgSize, FMSTR_BPTR response, FMSTR_U8 *retStatus)
{
    FMSTR_BOOL ret;
    FMSTR_ADDR writeOffset;
    FMSTR_SIZE writeLen = 0;
    FMSTR_LP_TSA_ENTRY ures;
    FMSTR_URES_HANDLER_FUNC uresFunc;
    FMSTR_RWI_BUFF uresBuffRWI;

    FMSTR_ASSERT(resourceId != NULL);
    FMSTR_ASSERT(msgBuffIO != NULL);
    FMSTR_ASSERT(response != NULL);
    FMSTR_ASSERT(retStatus != NULL);

    /* need at least write offset and write len */
    if (msgSize < 2U)
    {
        /* return status */
        *retStatus = FMSTR_STC_URESERR;
        return response;
    }

    /* Get write offset */
    msgBuffIO = FMSTR_AddressFromBuffer(&writeOffset, msgBuffIO);

    /* Get write length */
    msgBuffIO = FMSTR_SizeFromBuffer(&writeLen, msgBuffIO);

    /* Find resource in TSA table */
    ures = _FMSTR_FindUresInTsaInternal(resourceId);
    if (ures == NULL)
    {
        /* User resource not found in TSA table */
        /* return status */
        *retStatus = FMSTR_STC_URESERR;
        return response;
    }

    /* Red/write/ioctl buffer */
    uresBuffRWI.buff       = msgBuffIO;
    uresBuffRWI.sizeIn     = writeLen;
    uresBuffRWI.sizeOut    = 0;
    uresBuffRWI.sizeOutMax = 0;
    uresBuffRWI.offset     = writeOffset;
    uresBuffRWI.ioctlCode  = 0;

    /* User function */
    uresFunc = (FMSTR_URES_HANDLER_FUNC)ures->addr.p;
    if (uresFunc == NULL)
    {
        /* User resource function not exists */
        /* return status */
        *retStatus = FMSTR_STC_URESERR;
        return response;
    }

    /* Call user function */
    ret = uresFunc(FMSTR_URES_OP_WRITE, &uresBuffRWI, (void *)ures->info.p);
    if (ret == FMSTR_FALSE)
    {
        /* User resource function error */
        /* return status */
        *retStatus = FMSTR_STC_URESERR;
        return response;
    }

    /* Successful wrote data size */
    response = FMSTR_SizeToBuffer(response, uresBuffRWI.sizeConsumed);

    /* success  */
    *retStatus = FMSTR_STS_OK | FMSTR_STSF_VARLEN;
    return response;
}

static FMSTR_BPTR _FMSTR_UresControlIoctl(
    FMSTR_ADDR resourceId, FMSTR_BPTR msgBuffIO, FMSTR_SIZE msgSize, FMSTR_BPTR response, FMSTR_U8 *retStatus)
{
    FMSTR_BOOL ret;
    FMSTR_URES_IOCTL_CODE ioctlCode;
    FMSTR_LP_TSA_ENTRY ures;
    FMSTR_URES_HANDLER_FUNC uresFunc;
    FMSTR_RWI_BUFF uresBuffRWI;
    FMSTR_INDEX hdrSize;
    FMSTR_SIZE sizeIn = 0;
    FMSTR_SIZE i;

    FMSTR_ASSERT(resourceId != NULL);
    FMSTR_ASSERT(msgBuffIO != NULL);
    FMSTR_ASSERT(response != NULL);
    FMSTR_ASSERT(retStatus != NULL);

    /* need at least write offset and write len */
    if (msgSize < 2U)
    {
        /* return status */
        *retStatus = FMSTR_STC_URESERR;
        return response;
    }

    /* Get IOCTL code */
    msgBuffIO = FMSTR_SizeFromBuffer(&ioctlCode, msgBuffIO);

    /* Where have we stopped while processing? */
    hdrSize = msgBuffIO - response;

    /* Find resource in TSA table */
    ures = _FMSTR_FindUresInTsaInternal(resourceId);
    if (ures == NULL)
    {
        /* User resource not found in TSA table */
        /* return status */
        *retStatus = FMSTR_STC_URESERR;
        return response;
    }

    /* move the reminder of the IO buffer to the response */
    sizeIn = 0U;
    for (i = (FMSTR_SIZE)hdrSize; i < msgSize; i++)
    {
        response[sizeIn++] = *msgBuffIO++;
    }

    /* Read/write/ioctl buffer */
    uresBuffRWI.buff       = response;
    uresBuffRWI.sizeIn     = sizeIn;
    uresBuffRWI.sizeOut    = 0U;
    uresBuffRWI.sizeOutMax = FMSTR_COMM_BUFFER_SIZE;
    uresBuffRWI.offset     = NULL;
    uresBuffRWI.ioctlCode  = ioctlCode;

    /* User function */
    uresFunc = (FMSTR_URES_HANDLER_FUNC)ures->addr.p;
    if (uresFunc == NULL)
    {
        /* User resource function not exists */
        /* return status */
        *retStatus = FMSTR_STC_URESERR;
        return response;
    }

    /* Call user function */
    ret = uresFunc(FMSTR_URES_OP_IOCTL, &uresBuffRWI, (void *)ures->info.p);
    if (ret == FMSTR_FALSE)
    {
        /* User resource function error */
        /* return status */
        *retStatus = FMSTR_STC_URESERR;
        return response;
    }

    /* Move pointer in response buffer */
    response += uresBuffRWI.sizeOut;

    /* success  */
    *retStatus = FMSTR_STS_OK | FMSTR_STSF_VARLEN;
    return response;
}

#endif /* (FMSTR_USE_TSA) && (!(FMSTR_DISABLE)) */
