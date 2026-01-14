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
 * FreeMASTER Communication Driver - Packet-driver BDM communication
 */

#include "freemaster.h"
#include "freemaster_private.h"

/* Compile this code only if the PDBDM transport is selected in freemaster_cfg.h. */
#if (FMSTR_MK_IDSTR(FMSTR_TRANSPORT) == FMSTR_PDBDM_ID) && FMSTR_DISABLE == 0

#if FMSTR_SESSION_COUNT != 1
/* Packet Driven BDM transport only supports a single session */
#warning Please set FMSTR_SESSION_COUNT to 1.
#endif

/* Packet Driven BDM communication does not support any interrupt mode */
#if FMSTR_SHORT_INTR > 0 || FMSTR_LONG_INTR > 0
     #warning "FreeMASTER Packet Driven BDM driver doesn't support any Interrupt mode. Call the FMSTR_Poll() function in the Timer ISR routine instead."
#endif

#if (FMSTR_COMM_BUFFER_SIZE & 0x03)
    #error The communication buffer size (FMSTR_COMM_BUFFER_SIZE) must be aligned to 4.
#endif

#if (FMSTR_COMM_BUFFER_SIZE > 240)
    #error The communication buffer size (FMSTR_COMM_BUFFER_SIZE) must be smaller than 240 bytes.
#endif

#include "freemaster_protocol.h"
#include "freemaster_utils.h"

/******************************************************************************
* Protocol constants
*******************************************************************************/

/* Communication buffer borders */
#define FMSTR_PDBDM_BUFFBORDER_CODE1 0x00EEBE01UL  /* Beginning Magic Number for Packet driven BDM communication buffer, highest byte is reserved for communication buffer size  */
#define FMSTR_PDBDM_BUFFBORDER_CODE2 0xF0EEED01UL  /* Ending Magic Number for Packet driven BDM communication buffer */

/* Packet Driven BDM Communication States */
#define FMSTR_PDBDM_IDLE             0x00U    /* FreeMASTER serial driver is configured to BDM and ready to communicate */
#define FMSTR_PDBDM_RECEIVED_FRAME   0x02U    /* New Frame is ready in communication buffer. */
#define FMSTR_PDBDM_DECODING_FRAME   0x04U    /* Received frame is processed */
#define FMSTR_PDBDM_FRAME_TO_SEND    0x08U    /* FreeMASTER Communication buffer includes frame to be sent back */

/***********************************
*  local variables
***********************************/

/* Packet driven communication buffer */
#if FMSTR_PDBDM_USER_BUFFER != 0
extern FMSTR_PDBDM_COMBUFF _pdbdm;  /* Defined by user (e.g. special section placement needed) */
#else
static FMSTR_PDBDM_COMBUFF _pdbdm;  /* Defined locally */
#endif

/***********************************
*  local function prototypes
***********************************/

/* Interface function - Initialization of packet driven BDM transport */
static FMSTR_BOOL _FMSTR_PdBdmInit(void);
/* Interface function - Poll function of packet driven BDM transport */
static void _FMSTR_PdBdmPoll(void);
/* Interface function - Send Response function of packet driven BDM transport */
static void _FMSTR_PdBdmSendResponse(FMSTR_BPTR pResponse, FMSTR_SIZE nLength, FMSTR_U8 statusCode, void * identification);

/***********************************
*  global variables
***********************************/
/* Interface of this serial driver */
const FMSTR_TRANSPORT_INTF FMSTR_PDBDM =
{
    .Init = _FMSTR_PdBdmInit,
    .Poll = _FMSTR_PdBdmPoll,
    .SendResponse = _FMSTR_PdBdmSendResponse,
};

/******************************************************************************
*
* @brief    Packet Driven communication initialization
*
******************************************************************************/

static FMSTR_BOOL _FMSTR_PdBdmInit(void)
{
    /* Initialize buffer border and size in communication buffer */
    _pdbdm.ctxStart   = (FMSTR_U32)(FMSTR_PDBDM_BUFFBORDER_CODE1 | ((FMSTR_U32)FMSTR_COMM_BUFFER_SIZE << 24));
    _pdbdm.pcktSize   = 0;
    _pdbdm.ctxEnd     = FMSTR_PDBDM_BUFFBORDER_CODE2;

    FMSTR_MemSet(_pdbdm.commBuffer, 0, sizeof(_pdbdm.commBuffer));

    _pdbdm.bdmState   = FMSTR_PDBDM_IDLE;

    return FMSTR_TRUE;
}

/*******************************************************************************
*
* @brief    API: Packet Driven BDM Transport "Polling" call from the application main loop
*
* This function either handles all the PDBDM communication and decodes messages received
*   on the background by BDM interaface.
*
*
*******************************************************************************/

static void _FMSTR_PdBdmPoll(void)
{
    if (_pdbdm.bdmState == FMSTR_PDBDM_RECEIVED_FRAME)
    {
        FMSTR_U8 i;
        FMSTR_U8 crc;
        FMSTR_Crc8Init(&crc);

#if FMSTR_DEBUG_LEVEL >= 2
        FMSTR_DEBUG_PRINTF("PDBDM - Received Frame(cmd: 0x%02X, len: %d).\n", _pdbdm.cmdStatus, _pdbdm.pcktSize);
#endif
        /* Check Packet consistency */
        FMSTR_Crc8AddByte(&crc, _pdbdm.pcktSize);
        FMSTR_Crc8AddByte(&crc, _pdbdm.cmdStatus);

        for(i=0; i<_pdbdm.pcktSize; i++)
        {
            FMSTR_Crc8AddByte(&crc, _pdbdm.commBuffer[i]);
        }

        /* If CRC is valid, do the prtocol decoding, otherwise wait to finish background write*/
        if(crc == _pdbdm.commBuffer[i])
        {
#if FMSTR_DEBUG_LEVEL >= 3
            FMSTR_INDEX j;
            if(_pdbdm.pcktSize)
            {
                FMSTR_DEBUG_PRINTF("PDBDM - Received Frame Data: \n");
                for(j=0;j<_pdbdm.pcktSize;j++)
                {
                    FMSTR_DEBUG_PRINTF("%02X ", _pdbdm.commBuffer[j]);
                }
                FMSTR_DEBUG_PRINTF("\n");
            }
#endif
            /* Packet is potentially received */
            _pdbdm.bdmState = FMSTR_PDBDM_DECODING_FRAME;
            /* Destroy the last CRC in memory */
            _pdbdm.commBuffer[i] = ~crc;
            /* Decode received packet. Use "pdbdm" as a globally unique pointer value as our identifier */
            /* coverity[misra_c_2012_rule_7_4_violation:FALSE] */
            (void)FMSTR_ProtocolDecoder(_pdbdm.commBuffer, _pdbdm.pcktSize, _pdbdm.cmdStatus, (void*)"pdbdm");
        }
#if FMSTR_DEBUG_LEVEL >= 2
        else
        {
            FMSTR_DEBUG_PRINTF("PDBDM - Received Frame has invalid CRC(excepted: 0x%02X, received: 0x%02X).\n", crc, _pdbdm.commBuffer[i]);
        }
#endif
    }
}

/******************************************************************************
*
* @brief    Finalize transmit buffer before transmitting
*
* @param    pResponse - response buffer pointer
* @param    nLength - response length
* @param    statusCode - response packat status code
*
* This Function takes the data already prepared in the transmit buffer and notify
* PDBDM sytem that data are ready.
*
******************************************************************************/

static void _FMSTR_PdBdmSendResponse(FMSTR_BPTR pResponse, FMSTR_SIZE nLength, FMSTR_U8 statusCode, void * identification)
{
    FMSTR_U8   crc;
    FMSTR_SIZE i;
    
    /* Check the response that is valid to provided address? */
    FMSTR_ASSERT(pResponse == _pdbdm.commBuffer);
    FMSTR_ASSERT_RETURN(nLength <= (FMSTR_SIZE)FMSTR_COMM_BUFFER_SIZE, /* void */);

    /* Compute CRC8 data consistency check, including status and packet length */
    FMSTR_Crc8Init(&crc);

    FMSTR_Crc8AddByte(&crc, (FMSTR_U8)nLength);
    FMSTR_Crc8AddByte(&crc, statusCode);
    for(i=0; i<nLength; i++)
    {
        FMSTR_Crc8AddByte(&crc, _pdbdm.commBuffer[i]);
    }

    /* Add the response CRC at the end of data, in case that full communication buffer is used the CRC will be written into */
    _pdbdm.commBuffer[i] = crc;
    _pdbdm.pcktSize = (FMSTR_U8)nLength;
    _pdbdm.cmdStatus = statusCode;

#if FMSTR_DEBUG_LEVEL >= 2
        FMSTR_DEBUG_PRINTF("PDBDM - Prepared response(status: 0x%02X, len: %d).\n", _pdbdm.cmdStatus, _pdbdm.pcktSize);
        if(_pdbdm.pcktSize)
        {
            for(i=0;i<_pdbdm.pcktSize;i++)
            {
                FMSTR_DEBUG_PRINTF("%02X ", _pdbdm.commBuffer[i]);
            }
        }
        FMSTR_DEBUG_PRINTF(" CRC: 0x%02X.\n", crc);
#endif

    /* Frame is ready to send */
    _pdbdm.bdmState = FMSTR_PDBDM_FRAME_TO_SEND;
}

#endif /* (FMSTR_MK_IDSTR(FMSTR_TRANSPORT) == FMSTR_PDBDM_ID) && (!(FMSTR_DISABLE)) */
