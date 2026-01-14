/*
 * Copyright 2015 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_phy.h"
#include "usb_pd_timer.h"
#include "usb_pd_interface.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define PD_MSG_HEADER_MASK_FOR_SOP            (0xFFFFFFFFu)
#define PD_MSG_HEADER_MASK_FOR_SOPP_CABLE     (0xFFDFFFFFu)
#define PD_MSG_HEADER_MASK_FOR_SOPP_NON_CABLE (0xFEDFFFFFu)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void PD_MsgResetMsgId(pd_instance_t *pdInstance, start_of_packet_t sop);
static void PD_MsgResetAllMsgId(pd_instance_t *pdInstance);
static inline void PD_MsgIncrMsgId(pd_instance_t *pdInstance, start_of_packet_t sop);
static pd_status_t PD_MsgPrevSendCheck(pd_instance_t *pdInstance);
static void PD_MsgCopy(void *dst, void *src, uint32_t size);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

static void PD_MsgResetMsgId(pd_instance_t *pdInstance, start_of_packet_t sop)
{
    pdInstance->msgId[sop]              = 0;
    pdInstance->rcvMsgId[sop]           = 0;
    pdInstance->firstMsgAfterReset[sop] = 1;
}

static void PD_MsgResetAllMsgId(pd_instance_t *pdInstance)
{
    (void)memset(pdInstance->msgId, 0, sizeof(pdInstance->msgId));
    (void)memset(pdInstance->rcvMsgId, 0, sizeof(pdInstance->rcvMsgId));
    (void)memset(pdInstance->firstMsgAfterReset, 1, sizeof(pdInstance->firstMsgAfterReset));
}

static inline void PD_MsgIncrMsgId(pd_instance_t *pdInstance, start_of_packet_t sop)
{
    pdInstance->msgId[sop] = (uint8_t)(pdInstance->msgId[sop] + 1U) & (uint8_t)0x07U;
}

void PD_MsgReset(pd_instance_t *pdInstance)
{
    PD_MsgResetAllMsgId(pdInstance);
    (void)PD_PhyControl(pdInstance, PD_PHY_RESET_MSG_FUNCTION, NULL);
    pdInstance->hardResetReceived = 0;
    pdInstance->sendingState      = 0;
    pdInstance->receiveState      = 0;
    pdInstance->pendingMsg        = 0;
    pdInstance->receivedData      = NULL;
}

void PD_MsgSendDone(pd_instance_t *pdInstance, pd_status_t result)
{
    if (pdInstance->sendingState == 1U)
    {
        PD_MsgIncrMsgId(pdInstance, pdInstance->sendingSop);
        pdInstance->sendingState  = 2;
        pdInstance->sendingResult = result;
        PD_StackSetEvent(pdInstance, PD_TASK_EVENT_SEND_DONE);
    }
}

static pd_status_t PD_MsgPrevSendCheck(pd_instance_t *pdInstance)
{
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    if ((pdInstance->hardResetReceived != 0U) || (pdInstance->sendingState == 1U))
    {
        OSA_EXIT_CRITICAL();
        return kStatus_PD_Error;
    }

    pdInstance->sendingState = 1;
    OSA_EXIT_CRITICAL();
    return kStatus_PD_Success;
}

static void PD_MsgCopy(void *dst, void *src, uint32_t size)
{
    if (dst == src)
    {
        return;
    }
    else
    {
        (void)memcpy(dst, src, size);
    }
}

/*!
 * @brief Send control, data and extended message.
 *
 * This function is used to send control message, data message or extended message.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param sop             The sending sop. Please refer to the enumeration start_of_packet_t.
 * @param msgType         Message type. Please refer to the enumeration message_type_t.
 * @param dataLength      The length of buffer. It is from MsgHeaderLow to ByteN.
 * @param dataBuffer      The sending buffer. The pointer of dataBuffer points to Byte0.
 *                        The format of buffer. 0-NULL, 1-I2C_WRITE_BYTE_COUNT, 2-MsgHeaderLowByte,
 *                        3-MsgHeaderHighByte, 4-Byte0, 5-Byte1, ..., N-ByteN.
 *
 * @return kStatus_PD_Error or kStatus_PD_Success.
 */
pd_status_t PD_MsgSend(
    pd_instance_t *pdInstance, start_of_packet_t sop, message_type_t msgType, uint32_t dataLength, uint8_t *dataBuffer)
{
    pd_msg_header_t msgHeader;
    if (PD_MsgPrevSendCheck(pdInstance) != kStatus_PD_Success)
    {
        return kStatus_PD_Error;
    }

    if (msgType == kPD_MsgSoftReset)
    {
        PD_MsgResetMsgId(pdInstance, sop);
    }

    /* Set message header */
    pdInstance->sendingSop                       = sop;
    msgHeader.msgHeaderVal                       = 0;
    msgHeader.bitFields.portPowerRoleOrCablePlug = (uint16_t)pdInstance->curPowerRole;
    msgHeader.bitFields.portDataRole             = (uint16_t)pdInstance->curDataRole;
    msgHeader.bitFields.specRevision             = pdInstance->revision;
    if (dataLength < 2U)
    {
        pdInstance->sendingResult = kStatus_PD_Error;
        return pdInstance->sendingResult;
    }
    msgHeader.bitFields.NumOfDataObjs            = (uint16_t)((dataLength - 2U) >> 2U); /* control, data, chunked */
    msgHeader.bitFields.messageID                = (uint16_t)pdInstance->msgId[sop];
    msgHeader.bitFields.messageType              = (uint16_t)msgType & PD_MSG_TYPE_VALUE_MASK;
    if (((uint8_t)msgType & PD_MSG_EXT_TYPE_MASK) == PD_MSG_EXT_TYPE_MASK)
    {
        msgHeader.bitFields.extended = 1;
        if (pdInstance->unchunkedFeature != 0U)
        {
            msgHeader.bitFields.NumOfDataObjs = 0; /* unchunked */
        }
    }
    else
    {
        msgHeader.bitFields.extended = 0;
    }
    pdInstance->sendingData[0] = (uint32_t)msgHeader.msgHeaderVal << 16U;

    if ((sop != kPD_MsgSOP) && (pdInstance->pdConfig->deviceType != (uint8_t)kDeviceType_Cable))
    {
        /* Clear bit5(Port Data Role) and bit8(Cable Plug. Message originated from a DFP or UFP) */
        pdInstance->sendingData[0] &= PD_MSG_HEADER_MASK_FOR_SOPP_NON_CABLE;
    }
    /* Copy message data */
    PD_MsgCopy(&pdInstance->sendingData[1], dataBuffer, dataLength - 2U);

    PD_MsgReceive(pdInstance);
    pdInstance->sendingResult = kStatus_PD_Error;
    if (pdInstance->phyInterface->pdPhySend(pdInstance, (uint8_t)sop, (uint8_t *)&pdInstance->sendingData[0],
                                            dataLength) == kStatus_PD_Success)
    {
        /* Wait for the result to be sent. */
        (void)PD_PsmTimerWait(pdInstance, timrMsgSendWaitResultTimer, T_WAIT_SEND_RESULT, PD_TASK_EVENT_SEND_DONE);
        (void)OSA_EventClear(pdInstance->taskEventHandle, PD_TASK_EVENT_SEND_DONE);
    }

    pdInstance->sendingState = 0;

    return pdInstance->sendingResult;
}

/*!
 * @brief Send extended message.
 *
 * This function is used to send extended message.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param sop             The sending sop. Please refer to the enumeration start_of_packet_t.
 * @param extMsgType      Message type. Please refer to the enumeration message_type_t.
 * @param dataLength      The length of buffer. It is from Byte0 to ByteN.
 * @param dataBuffer      The sending buffer. The pointer of dataBuffer points to Byte0.
 *                        The format of buffer. 0-NULL, 1-I2C_WRITE_BYTE_COUNT, 2-MsgHeaderLowByte,
 *                        3-MsgHeaderHighByte, 4-ExtHeaderLowByte, 5-ExtHeaderHighByte, 6-Byte0, 7-Byte1, ..., N-ByteN.
 * @param dataSize        The field of extended message header. This value should be 0 when requestChunk equals to 1;
 * @param requestChunk    The field of extended message header. This value should be 0 when sending unckunked message.
 * @param chunkNumber     The field of extended message header. This value should be 0 when sending unckunked message.
 *
 * @return kStatus_PD_Error or kStatus_PD_Success.
 */
#if (defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
pd_status_t PD_MsgSendExtendedMsg(pd_instance_t *pdInstance,
                                  start_of_packet_t sop,
                                  message_type_t extMsgType,
                                  uint32_t dataLength,
                                  uint8_t *dataBuffer,
                                  uint16_t dataSize,
                                  uint8_t requestChunk,
                                  uint8_t chunkNumber)
{
    uint32_t index;
    uint8_t *dst;
    uint32_t sendlength;
    pd_extended_msg_header_t extMsgHeader;

    /* set extended msg header */
    extMsgHeader.extendedMsgHeaderVal = 0;
    if (pdInstance->unchunkedFeature != 0U)
    {
        extMsgHeader.bitFields.chunked = 0;
    }
    else
    {
        extMsgHeader.bitFields.chunked = 1;
    }
    extMsgHeader.bitFields.dataSize     = dataSize;
    extMsgHeader.bitFields.requestChunk = requestChunk;
    extMsgHeader.bitFields.chunkNumber  = chunkNumber;
    pdInstance->sendingData[1]          = extMsgHeader.extendedMsgHeaderVal;

    /* copy msg data */
    dst = (uint8_t *)pdInstance->sendingData + 6;
    PD_MsgCopy(dst, dataBuffer, dataLength);

    /* pad zero */
    if (pdInstance->unchunkedFeature == 0U)
    {
        sendlength = ((dataLength + 2U + 3U) >> 2U);                 /* add extended msg header */
        sendlength = (sendlength << 2U) + 2U;                        /* add msg header */
        for (index = dataLength; index < (sendlength - 4U); ++index) /* subtract extended msg header */
        {
            dst[index] = 0U;
        }
    }
    else
    {
        sendlength = dataLength + 4U; /* add msg header, extended msg header */
    }

    return PD_MsgSend(pdInstance, sop, extMsgType, sendlength, (uint8_t *)&pdInstance->sendingData[1]);
}
#endif

#if ((defined PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE) && (PD_CONFIG_VENDOR_DEFINED_MESSAGE_ENABLE)) || \
    (defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)) ||        \
    (defined(PD_CONFIG_SRC_AUTO_DISCOVER_CABLE_PLUG) && (PD_CONFIG_SRC_AUTO_DISCOVER_CABLE_PLUG))
pd_status_t PD_MsgSendStructuredVDM(pd_instance_t *pdInstance,
                                    start_of_packet_t sop,
                                    pd_structured_vdm_header_t reponseVdmHeader,
                                    uint8_t count,
                                    uint32_t *vdos)
{
    /* set structured vdm header */
    pdInstance->sendingData[1] = reponseVdmHeader.structuredVdmHeaderVal;
    /* copy msg data */
    PD_MsgCopy(&pdInstance->sendingData[2], vdos, (uint32_t)count * 4U);

    return PD_MsgSend(pdInstance, sop, kPD_MsgVendorDefined, ((uint32_t)count + 1U) * 4U + 2U,
                      (uint8_t *)&pdInstance->sendingData[1]);
}
#endif

void PD_MsgReceivedHardReset(pd_instance_t *pdInstance)
{
    pdInstance->hardResetReceived = 1;
    PD_MsgResetAllMsgId(pdInstance);
    PD_StackSetEvent(pdInstance, PD_TASK_EVENT_RECEIVED_HARD_RESET);
}

/*!
 * @brief Transmit hard reset or cable reset.
 *
 * This function should be called when policy engine initiates hard reset or cable reset.
 *
 * @param pdInstance            The pd handle. It equals the value returned from PD_InstanceInit.
 * @param hardResetOrCableReset 0 - hard reset, 1 - cable reset.
 *
 * @return None.
 */
void PD_MsgSendHardOrCableReset(pd_instance_t *pdInstance, uint8_t hardResetOrCableReset)
{
    /* Buffer will now be used for TX, Hard Reset is allowed to be sent while the TCPC is still processing a previous */
    /* transmission */
    pdInstance->sendingState = 1;

    if (hardResetOrCableReset == 0U)
    {
        (void)PD_PhyControl(pdInstance, PD_PHY_SEND_HARD_RESET, NULL);
    }
    else
    {
        (void)PD_PhyControl(pdInstance, PD_PHY_SEND_CABLE_RESET, NULL);
    }

    (void)PD_PsmTimerWait(pdInstance, tMsgHardResetCompleteTimer, T_HARD_RESET_COMPLETE, PD_TASK_EVENT_SEND_DONE);

    (void)OSA_EventClear(pdInstance->taskEventHandle, PD_TASK_EVENT_SEND_DONE);
    pdInstance->sendingState = 0;
    if (hardResetOrCableReset == 0U) /* Hard Reset */
    {
        PD_MsgResetAllMsgId(pdInstance);
    }
    else /* Cable Reset */
    {
        /* Reset all sop' sop'' msgid  after sending cable reset */
        PD_MsgResetMsgId(pdInstance, kPD_MsgSOPp);
        PD_MsgResetMsgId(pdInstance, kPD_MsgSOPpp);
        PD_MsgResetMsgId(pdInstance, kPD_MsgSOPDbg);
        PD_MsgResetMsgId(pdInstance, kPD_MsgSOPpDbg);
    }
}

void PD_MsgReceived(pd_instance_t *pdInstance, pd_phy_rx_result_t *rxResult)
{
    pd_msg_header_t msgHeader;

    if (pdInstance->receiveState == 1U)
    {
        msgHeader.msgHeaderVal =
            USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)pdInstance->receivingDataBuffer + 2U));

        if ((msgHeader.bitFields.messageType == (uint8_t)kPD_MsgSoftReset) && (msgHeader.bitFields.extended == 0U) &&
            (msgHeader.bitFields.NumOfDataObjs == 0U))
        {
            /* SoftReset Message always has a MessageID value of zero */
            PD_MsgResetMsgId(pdInstance, ((rxResult->rxSop == kPD_MsgSOP) ? kPD_MsgSOP : kPD_MsgSOPp));
        }
        else if ((msgHeader.bitFields.messageID != pdInstance->rcvMsgId[rxResult->rxSop]) ||
                 (pdInstance->firstMsgAfterReset[rxResult->rxSop] != 0U))
        {
            /* Drop the duplicate messages */
            /* 1.When the first good packet is received after a reset, the receiver shall store a copy of the received
             */
            /* MessageID value */
            /* 2.If MessageID value in the received Message is different than the stored value, the receiver shall
               return a GoodCRC */
            /* Message with the new MessageID value, store a copy of the new MessageID value and process the Message. */
            pdInstance->firstMsgAfterReset[rxResult->rxSop] = 0;
            pdInstance->rcvMsgId[rxResult->rxSop]           = (uint8_t)msgHeader.bitFields.messageID;
        }
        else
        {
            /* For subsequent Messages, if MessageID value in a received Message is the same as the stored value, */
            /* the receiver shall return a GoodCRC Message with that MessageID value and drop the Message */
            /* (this is a retry of an already received Message). */
            pdInstance->receiveState = 0;
            PD_MsgReceive(pdInstance);
            return;
        }

        pdInstance->receiveState   = 2;
        pdInstance->receivedSop    = rxResult->rxSop;
        pdInstance->receivedLength = rxResult->rxLength;
        pdInstance->receiveResult  = rxResult->rxResultStatus;
        PD_StackSetEvent(pdInstance, PD_TASK_EVENT_PD_MSG);
    }
}

void PD_MsgStopReceive(pd_instance_t *pdInstance)
{
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    pdInstance->enableReceive = 0;
    if (pdInstance->receiveState == 1U)
    {
        pdInstance->receiveState = 0;
        OSA_EXIT_CRITICAL();
    }
    else
    {
        OSA_EXIT_CRITICAL();
    }
}

void PD_MsgStartReceive(pd_instance_t *pdInstance)
{
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    pdInstance->enableReceive = 1;
    OSA_EXIT_CRITICAL();
    PD_MsgReceive(pdInstance);
}

void PD_MsgReceive(pd_instance_t *pdInstance)
{
    if (pdInstance->enableReceive == 0U)
    {
        return;
    }

    if (pdInstance->receiveState == 0U)
    {
#if (defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
        if ((pdInstance->receivingDataBuffer == NULL) ||
            ((pdInstance->unchunkedFeature != 0U) &&
             (pdInstance->receivingDataBuffer == (uint32_t *)&pdInstance->receivingChunkedData[0])))
#else
        if (pdInstance->receivingDataBuffer == NULL)
#endif
        {
#if (defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
            if (pdInstance->unchunkedFeature != 0U)
            {
                pdInstance->receivingDataBuffer = (uint32_t *)&pdInstance->receivingData[0];
            }
            else
            {
                pdInstance->receivingDataBuffer = (uint32_t *)&pdInstance->receivingChunkedData[0];
            }
#else
            pdInstance->receivingDataBuffer = (uint32_t *)&pdInstance->receivingData[0];
#endif
        }
        pdInstance->receiveState = 1;
        (void)pdInstance->phyInterface->pdPhyReceive(pdInstance, pdInstance->pendingSOP,
                                                     (uint8_t *)&pdInstance->receivingDataBuffer[0]);
    }
}

uint8_t PD_MsgGetReceiveResult(pd_instance_t *pdInstance)
{
    uint8_t retVal = 0;
#if (defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
    uint32_t receiveLength;
    uint8_t *destBuffer;
    uint8_t *sourceBuffer;
    uint8_t needCopy = 0;
    pd_msg_header_t msgHeader;
    pd_extended_msg_header_t extHeader;
#endif

    if (pdInstance->receiveState == 2U)
    {
        pdInstance->receiveState = 0;
        if (pdInstance->receiveResult == kStatus_PD_Success)
        {
#if (defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
            sourceBuffer = (uint8_t *)pdInstance->receivingDataBuffer;
            sourceBuffer += 2U;
            msgHeader.msgHeaderVal = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(sourceBuffer);
            if (msgHeader.bitFields.extended != 0U)
            {
                sourceBuffer += 2U;
                extHeader.extendedMsgHeaderVal = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(sourceBuffer);

                /* chunked ext msg need copy, because it may be larger than 28 bytes and are chunked */
                if (pdInstance->receivingDataBuffer == (uint32_t *)&pdInstance->receivingChunkedData[0])
                {
                    needCopy = 1;

                    /* copy msg header */
                    pdInstance->receivingData[0] =
                        USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)pdInstance->receivingDataBuffer));
                    /* skip count, frame type, msg header */
                    destBuffer   = (uint8_t *)&pdInstance->receivingData[0] + 4U;
                    sourceBuffer = (uint8_t *)pdInstance->receivingDataBuffer + 4U;
                    /* copy ext header */
                    destBuffer[0] = sourceBuffer[0];
                    destBuffer[1] = sourceBuffer[1];
                    /* skip ext header */
                    destBuffer += 2;
                    sourceBuffer += 2;
                }
                else
                {
                    needCopy = 0;
                }

                receiveLength = pdInstance->receivedLength; /* receivelength is from msgHeader to ByteN */

                if (extHeader.bitFields.chunked == 0U)
                {
                    /* unchunked ext msg */
                    if (receiveLength > 264U)
                    {
                        receiveLength = 264U;
                    }
                }
                else
                {
                    /* chunked ext msg */
                    if (receiveLength > 30U)
                    {
                        receiveLength = 30U;
                    }

                    if (needCopy != 0U)
                    {
                        destBuffer += (extHeader.bitFields.chunkNumber * 26U);
                    }
                }
            }
            else
#endif
            {
                /* control or data msg */
            }

#if (defined PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
            if (needCopy != 0U)
            {
                /* reduce the normal header and ext header */
                /* copy the remaining bytes except msgHeader and extHeader */
                PD_MsgCopy(destBuffer, sourceBuffer, receiveLength - 4U);
                pdInstance->receivedData = (uint32_t *)&(pdInstance->receivingData[0]);
            }
            else
#endif
            {
                pdInstance->receivedData = pdInstance->receivingDataBuffer;
            }
        }

        retVal = (pdInstance->receiveResult == kStatus_PD_Success) ? 1U : 0U;
    }

    return retVal;
}

uint8_t PD_MsgRecvPending(pd_instance_t *pdInstance)
{
    return (pdInstance->receiveState == 2U) ? 1U : 0U;
}

#if defined(PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE) && (PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE)
/* return 0 wait, 1 start */
uint8_t PD_MsgSrcStartCommand(pd_instance_t *pdInstance)
{
    if (pdInstance->revision >= PD_SPEC_REVISION_30)
    {
        if (pdInstance->commandSrcOwner == 0U)
        {
            typec_current_val_t rpVal = kCurrent_1A5;
            uint16_t timrTime         = T_SINK_TX;

            pdInstance->commandSrcOwner = 1;
            (void)PD_PhyControl(pdInstance, PD_PHY_SRC_SET_TYPEC_CURRENT_CAP, &rpVal);

            do
            {
                timrTime =
                    PD_PsmTimerWait(pdInstance, tSinkTxTimer, timrTime,
                                    ((uint32_t)PD_TASK_EVENT_RECEIVED_HARD_RESET | (uint32_t)PD_TASK_EVENT_PD_MSG));

                if (timrTime != 0U)
                {
#if defined(PD_CONFIG_CABLE_COMMUNICATION_ENABLE) && (PD_CONFIG_CABLE_COMMUNICATION_ENABLE)
                    uint32_t eventSet = 0U;
                    (void)OSA_EventGet(pdInstance->taskEventHandle, PD_TASK_EVENT_ALL, &eventSet);

                    if (((PD_TASK_EVENT_PD_MSG & eventSet) != 0U) && (pdInstance->receivedSop != kPD_MsgSOP))
                    {
                        /* discard sop' sop'' message */
                        (void)OSA_EventClear(pdInstance->taskEventHandle, PD_TASK_EVENT_PD_MSG);
                        pdInstance->receiveState = 0U;
                        PD_MsgReceive(pdInstance);
                        continue;
                    }
#endif
                    return 0;
                }
                else
                {
                    break;
                }
            } while (true);
        }
    }

    return 1;
}

void PD_MsgSrcEndCommand(pd_instance_t *pdInstance)
{
    if ((pdInstance->commandSrcOwner != 0U) && (pdInstance->revision >= PD_SPEC_REVISION_30))
    {
        pdInstance->commandSrcOwner = 0U;
        if (pdInstance->curPowerRole == kPD_PowerRoleSource)
        {
            typec_current_val_t rpVal = kCurrent_3A;
            (void)PD_PhyControl(pdInstance, PD_PHY_SRC_SET_TYPEC_CURRENT_CAP, &rpVal);
        }
    }
}

uint8_t PD_MsgSnkCheckStartCommand(pd_instance_t *pdInstance, pd_command_t command)
{
    if (pdInstance->revision >= PD_SPEC_REVISION_30)
    {
        /* soft_reset for protocol error and hard_reset don't care Rp */
        if ((command != PD_DPM_CONTROL_HARD_RESET) && (command != PD_DPM_CONTROL_SOFT_RESET))
        {
            typec_current_val_t rpVal = kCurrent_3A;

            (void)PD_PhyControl(pdInstance, PD_PHY_GET_TYPEC_CURRENT_CAP, &rpVal);
            if (rpVal != kCurrent_3A)
            {
                return 0;
            }
        }
    }

    return 1;
}
#endif /* PD_CONFIG_PD3_AMS_COLLISION_AVOID_ENABLE */
