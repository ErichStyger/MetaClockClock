/*
 * Copyright 2015 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_phy.h"
#include "usb_pd_interface.h"
#include "usb_pd_ptn5110.h"
#include "usb_pd_ptn5110_register.h"
#include "usb_pd_timer.h"
#include "usb_pd_spec.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void PDPTN5110_MsgHalClearAnyPendingMessage(pd_instance_t *pdInstance);
static void PDPTN5110_MsgHalRcvdControlDataOrChunked(pd_instance_t *pdInstance,
                                                     uint8_t *dest,
                                                     pd_phy_rx_result_t *rxResult);
#if defined(PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
static void PDPTN5110_MsgHalRcvdExtMsgUnChunked(pd_instance_t *pdInstance, uint8_t *dst, pd_phy_rx_result_t *rxResult);
#endif
static void PDPTN5110_MsgHalRxReceiveBuffer(pd_instance_t *pdInstance, uint8_t *dest, pd_phy_rx_result_t *rxResult);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Write transmit register to send message.
 *
 * This function is used to writes the content and configuration of the message to be transmitted into
 * the TRANSMIT_BUFFER before start transmitting.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit
 * @param retryCounter    Retry counter. Please refer to the enumeration Retry_Counter_t.
 * @param transmitSop     Transmit SOP* message. Please refer to the enumeration Received_SOP_Message_t
 *
 * @return None.
 */
static inline void PDPTN5110_MsgHalTxStartTransmit(pd_instance_t *pdInstance,
                                                   Retry_Counter_t retryCounter,
                                                   uint8_t transmitSop)
{
    Reg_BusWriteByte(pdInstance, transmit, (uint8_t)retryCounter | transmitSop);
}

/*!
 * @brief Detect SOP* message type.
 *
 * This function is used to notify the TCPC of the message type and/or signaling types to be detected.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit
 * @param sop_mask        SOP mask. Please refer to the enumeration start_of_packet_t.
 *
 * @return None.
 */
void PDPTN5110_MsgHalSetRxSopEnable(pd_instance_t *pdInstance, uint8_t sop_mask)
{
    if (sop_mask != Reg_CacheRead(pdInstance, MSG_RX, receive_detect))
    {
        /* always enable Hard Reset reception */
        Reg_CacheAndBusModifyByteField(pdInstance, MSG_RX, receive_detect, POR_receive_detect_mask,
                                       (sop_mask | TCPC_RECEIVE_DETECT_ENABLE_HARD_RESET_MASK));
    }
}

/*!
 * @brief Force clearing any pending message.
 *
 * This function is used to force clearing any pending messages or free the extended message buffer.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit
 *
 * @return None.
 */
static void PDPTN5110_MsgHalClearAnyPendingMessage(pd_instance_t *pdInstance)
{
    uint8_t i         = 3;
    uint16_t ClrValue = TCPC_ALERT_BEGINNING_SOP_MESSAGE_STATUS_MASK | TCPC_ALERT_RECEIVE_SOP_MESSAGE_STATUS_MASK;

    while (i-- > 0U)
    {
        /* Clear again - we may have 2nd message pending */
        (void)Reg_BusWriteBlock(pdInstance, (uint8_t *)&ClrValue, 2, alert);
    }
}

/*!
 * @brief Disable receiving any messages (Reset the protcol layer hardware).
 *
 * This function is used to disable receiving any messages.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit
 *
 * @return None.
 */
void PDPTN5110_MsgHalDisableMessageRx(pd_instance_t *pdInstance)
{
    /* reset config to POR */
    Reg_CacheAndBusModifyByteField(pdInstance, MSG_RX, receive_detect, POR_receive_detect_mask, POR_receive_detect);

    /* Disable Bist Test Mode */
    PDPTN5110_MsgHalExitBist(pdInstance);

    /* Clear any pending Message */
    PDPTN5110_MsgHalClearAnyPendingMessage(pdInstance);
}

/*!
 * @brief Set message header information.
 *
 * This function is used to set message header information used to response GoodCRC.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit
 * @param revision        USB PD specification revision. The value can be one of the following values.
 *                        PD_SPEC_REVISION_10 (0x00)
 *                        PD_SPEC_REVISION_20 (0x01)
 *                        PD_SPEC_REVISION_30 (0x02)
 * @param powerRole       The current power role. Please refer to the enumeration pd_power_role_t.
 * @param dataRole        The current data role. Please refer to the enumeration pd_data_role_t.
 *
 * @return None.
 */
void PDPTN5110_MsgHalSetMessageHeaderInfo(pd_instance_t *pdInstance,
                                          uint8_t revision,
                                          uint8_t powerRole,
                                          uint8_t dataRole)
{
    /* As we are not a cable plug, set Cable Plug bit-4 to 0 */
    /* In order to keep compatible with PD2.0 and PD3.0, set PD revision to PD2.0 here. For more information, please
       refer to chapter 6.2.1.1.5 Specification Revision in the USB PD specification. */
    Reg_BusWriteByte(
        pdInstance, message_header_info,
        ((dataRole << TCPC_MESSAGE_HEADER_INFO_DATA_ROLE_LSB) & TCPC_MESSAGE_HEADER_INFO_DATA_ROLE_MASK) |
            (((uint8_t)(PD_SPEC_REVISION_20) << TCPC_MESSAGE_HEADER_INFO_USB_PD_SPECIFICATION_REVISION_LSB) &
             TCPC_MESSAGE_HEADER_INFO_USB_PD_SPECIFICATION_REVISION_MASK) |
            ((powerRole << TCPC_MESSAGE_HEADER_INFO_POWER_ROLE_LSB) & TCPC_MESSAGE_HEADER_INFO_POWER_ROLE_MASK));

    /* Avoid compiler warning */
    (void)revision;
}

/*!
 * @brief Transmit hard reset or cable reset.
 *
 * This function is used to send hard reset or hard reset.
 * This function should be called when policy engine initiates hard reset or cable reset.
 *
 * @param pdInstance            The pd handle. It equals the value returned from PD_InstanceInit.
 * @param hardResetOrCableReset 0 - hard reset, 1 - cable reset.
 *
 * @return None.
 */
void PDPTN5110_MsgHalSendReset(pd_instance_t *pdInstance, uint8_t hardResetOrCableReset)
{
    /* Need to update Transmit SOP* message field to Hard Reset/Cable Reset type and Retry Counter should be set to 0 */
    /* TCPM must writes the content and configuration of the message to be transmitted into the TRANSMIT_BUFFER before
       start transmitting */
    PDPTN5110_MsgHalTxStartTransmit(pdInstance, NO_MESSAGE_RETRY_IS_REQUIRED, 5U + hardResetOrCableReset);
}

/*!
 * @brief Send control message, data message or chunked extended message.
 *
 * This function is used to send control message, data message or chunked extended message.
 * This function is called by the function PDPTN5110_Send.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param sop             The sending sop. Please refer to the enumeration start_of_packet_t
 * @param buffer          The sending buffer. The format of buffer - 0-NULL, 1-I2C_WRITE_BYTE_COUNT, 2-MsgHeaderLowByte,
 *                        3-MsgHeaderHighByte, 4-Byte0, 5-Byte1, ..., N-ByteN.
 * @param count           The length of buffer. It is from MsgHeaderLow to ByteN.
 *
 * @return kStatus_PD_Error or kStatus_PD_Success.
 */
pd_status_t PDPTN5110_MsgHalSendControlDataOrChunked(pd_instance_t *pdInstance,
                                                     uint8_t sop,
                                                     uint8_t *buffer,
                                                     uint8_t count)
{
    /* include msg header and dataObj */
    if (count > 30U)
    {
        count = 30U;
    }
    buffer[TCPC_TRANSMIT_BYTE_COUNT_POS] = count;

    Reg_BusWriteByte(pdInstance, command, (uint8_t)TCPC_RESETTRASMITBUFFER);

    /* Write byte_count/tx_buf_header/tx_buf_obj_byte in one transaction, as per USB TCPC specification 4.3.5 */
    /* Writing the TRANSMIT_BUFFER */
    if (Reg_BusWriteBlock(pdInstance, (uint8_t *)(buffer + TCPC_TRANSMIT_BYTE_COUNT_POS),
                          (uint32_t)count + TCPC_TRANSMIT_BYTE_COUNT_LEN, transmit_byte_count) != kStatus_PD_Success)
    {
        return kStatus_PD_Error;
    }

    PDPTN5110_MsgHalTxStartTransmit(pdInstance,
                                    (pdInstance->revision == PD_SPEC_REVISION_20) ?
                                        AUTOMATICALLY_RETRY_MESSAGE_TRANSMISSION_THREE_TIMES :
                                        AUTOMATICALLY_RETRY_MESSAGE_TRANSMISSION_TWICE,
                                    sop);

    return kStatus_PD_Success;
}

#if defined(PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
/*!
 * @brief Send an unchunked extended message.
 *
 * This function is used to send an unchunked extended message.
 * This function is called by the function PDPTN5110_Send.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param sop             The sending sop. Please refer to the enumeration start_of_packet_t
 * @param buffer          The sending buffer. The format of buffer - 0-NULL, 1-I2C_WRITE_BYTE_COUNT, 2-MsgHeaderLowByte,
 *                        3-MsgHeaderHighByte, 4-Byte0, 5-Byte1, ..., N-ByteN.
 * @param dataSize        The length of buffer. It is from MsgHeaderLow to ByteN.
 *
 * @return kStatus_PD_Error or kStatus_PD_Success.
 */
pd_status_t PDPTN5110_MsgHalSendUnchunked(pd_instance_t *pdInstance, uint8_t sop, uint8_t *buffer, uint16_t dataSize)
{
    uint8_t i;
    int16_t dataRemain;
    uint16_t txCount;
    uint8_t savedByte;
    uint8_t *i2cWriteBuf = (uint8_t *)buffer + TCPC_TRANSMIT_BYTE_COUNT_POS;

    Reg_BusWriteByte(pdInstance, command, (uint8_t)TCPC_RESETTRASMITBUFFER);

    if (Reg_CacheRead(pdInstance, GLOBAL, pd_interface_rev) >= PD_INTERFACE_REV2VER1)
    {
        /* dataSize include msg header and dataObj */
        if (dataSize > 264U)
        {
            dataSize = 264U;
        }
        /* 1st half of data */
        i2cWriteBuf[0] =
            (dataSize > MSG_TCPC_MAX_TX_WINDOW_WRITE_BURST) ? MSG_TCPC_MAX_TX_WINDOW_WRITE_BURST : (uint8_t)dataSize;
        if (Reg_BusWriteBlock(pdInstance, i2cWriteBuf, (uint32_t)i2cWriteBuf[0] + TCPC_TRANSMIT_BYTE_COUNT_LEN,
                              transmit_byte_count) != kStatus_PD_Success)
        {
            return kStatus_PD_Error;
        }

        /* Second half of data */
        if (dataSize > MSG_TCPC_MAX_TX_WINDOW_WRITE_BURST)
        {
            i2cWriteBuf += MSG_TCPC_MAX_TX_WINDOW_WRITE_BURST;
            savedByte      = i2cWriteBuf[0];
            i2cWriteBuf[0] = (uint8_t)(dataSize - MSG_TCPC_MAX_TX_WINDOW_WRITE_BURST);
            (void)Reg_BusWriteBlock(pdInstance, i2cWriteBuf, (uint32_t)i2cWriteBuf[0] + TCPC_TRANSMIT_BYTE_COUNT_LEN,
                                    transmit_byte_count);
            i2cWriteBuf[0] = savedByte;
        }
    }
    else /* Legacy vendor defined interface */
    {
        if (dataSize < 6U) /* dataSize include msg header and dataObj */
        {
            dataSize = 6U;
        }

        txCount    = dataSize;
        dataRemain = (int16_t)dataSize;
        if (txCount > 30U)
        {
            txCount = 30U;
        }

        /* Write byte_count/tx_buf_header/tx_buf_obj_byte in one transaction, as per USB TCPC specification 4.3.5 */
        /* Writing the TRANSMIT_BUFFER */
        i2cWriteBuf[0] = (uint8_t)txCount;
        if (Reg_BusWriteBlock(pdInstance, i2cWriteBuf, (uint32_t)txCount + TCPC_TRANSMIT_BYTE_COUNT_LEN,
                              transmit_byte_count) != kStatus_PD_Success)
        {
            return kStatus_PD_Error;
        }

        dataRemain -= (int16_t)txCount;
        /* Need to use TCPC extended tx buffer */
        if (dataRemain > 0)
        {
            i2cWriteBuf += 30U;
            i = 1;
            while (dataRemain > 0)
            {
                txCount = (uint16_t)dataRemain;
                if (txCount > 30U)
                {
                    txCount = 30U;
                }

                /* set ext_msg_index */
                Reg_BusWriteByte(pdInstance, ptn5110_ext_msg_index, i);
                /* copy data (30 bytes) to tcpc tx buffer */
                (void)Reg_BusWriteBlock(pdInstance, i2cWriteBuf, txCount, tx_buf_header);
                dataRemain -= (int16_t)txCount;
                i2cWriteBuf += txCount;
                i++;
            }
        }

        /* Msg header + ext_msg header + ext_data */
        (void)Reg_BusWriteBlock(pdInstance, (uint8_t *)&dataSize, 2, ptn5110_ext_transmit_byte_count);
    }

    PDPTN5110_MsgHalTxStartTransmit(pdInstance,
                                    (pdInstance->revision == PD_SPEC_REVISION_20) ?
                                        AUTOMATICALLY_RETRY_MESSAGE_TRANSMISSION_THREE_TIMES :
                                        AUTOMATICALLY_RETRY_MESSAGE_TRANSMISSION_TWICE,
                                    sop);

    return kStatus_PD_Success;
}
#endif

/*!
 * @brief Receive the control messgae, data message or chunked extended message in the receive buffer.
 *
 * This function is used to receive the control messgae, data message or chunked extended message in the receive buffer.
 * This function is called by the function PDPTN5110_MsgHalRxReceiveBuffer.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param dest            The receive buffer. The format of dest - 0-readable_byte_count, 1-rx_buf_frame_type,
 *                        2-MsgHeaderLowByte, 3-MsgHeaderHighByte, 4-Byte0, 5-Byte1, ..., N-ByteN.
 * @param rxResult        It is out parameter. Please refer to the enumeration pd_phy_rx_result_t.
 *
 * @return None.
 */
static void PDPTN5110_MsgHalRcvdControlDataOrChunked(pd_instance_t *pdInstance,
                                                     uint8_t *dest,
                                                     pd_phy_rx_result_t *rxResult)
{
    uint8_t bytes = dest[0];

    if (bytes > 31U) /* 1 byte frame type + 2 byte message header + 0(min)~7(max) data object(4 byte) */
    {
        bytes = 31U;
    }

    if (bytes > 3U) /* if not data object, exit */
    {
        if (Reg_CacheRead(pdInstance, GLOBAL, pd_interface_rev) >= PD_INTERFACE_REV2VER1)
        {
            /* Reset and re-read from start - TODO - avoid this by doing byte read to READABLE_BYTE_COUNT */
            Reg_BusWriteByte(pdInstance, command, (uint8_t)TCPC_RESETRECEIVEBUFFER);
            (void)Reg_BusReadBlock(pdInstance, receive_byte_count, (uint32_t)bytes + 1U, dest);
        }
        else
        {
            (void)Reg_BusReadBlock(pdInstance, rx_buf_obj1, (uint32_t)bytes - 3U, dest + 4U);
        }
    }
    if (bytes >= 1U)
    {
        rxResult->rxLength       = (uint16_t)bytes - 1U;
        rxResult->rxResultStatus = kStatus_PD_Success;
    }
    else
    {
        rxResult->rxResultStatus = kStatus_PD_Error;
    }
}

#if defined(PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
/*!
 * @brief Receive the unchunked extended message in the receive buffer.
 *
 * This function is used to receive the unchunked extended message in the receive buffer.
 * This function is called by the function PDPTN5110_MsgHalRxReceiveBuffer.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param dst             The receive buffer. The format of dst - 0-readable_byte_count, 1-rx_buf_frame_type,
 *                        2-MsgHeaderLowByte, 3-MsgHeaderHighByte, 4-Byte0, 5-Byte1, ..., N-ByteN.
 * @param rxResult        It is out parameter. Please refer to the enumeration pd_phy_rx_result_t.
 *
 * @return None.
 */
static void PDPTN5110_MsgHalRcvdExtMsgUnChunked(pd_instance_t *pdInstance, uint8_t *dst, pd_phy_rx_result_t *rxResult)
{
    if (Reg_CacheRead(pdInstance, GLOBAL, pd_interface_rev) >= PD_INTERFACE_REV2VER1)
    {
        /* Allow for MSG_HAL_RCVD_EXT_MSG_UNCHUNKED_PAD_BYTES to not match the TCPC2 requirements if
         * another platform is added.*/
        uint16_t total_bytes_received = 0;
        uint8_t rx_buf_count          = dst[0];
        uint8_t tmp_buf[2];

        /* Move to start of buffer */
        Reg_BusWriteByte(pdInstance, command, (uint8_t)TCPC_RESETRECEIVEBUFFER);
        /* Returned data format is {readable_byte_count, frame type, data0, data1 ... dataN} */
        /* Need to store data0 at dst[0], so read buffer from */
        (void)Reg_BusReadBlock(pdInstance, receive_byte_count, (uint32_t)rx_buf_count + 1U, dst);
        total_bytes_received += (uint16_t)rx_buf_count - 1U;

        if ((Reg_CacheRead(pdInstance, INTERRUPT, alert) & TCPC_ALERT_BEGINNING_SOP_MESSAGE_STATUS_MASK) != 0U)
        {
            uint16_t ClrValue = TCPC_ALERT_BEGINNING_SOP_MESSAGE_STATUS_MASK;

            /* Move pointer to next location */
            dst += rx_buf_count - 1U; /* Number of data bytes, remove the 1 byte count for frame type */

            /* Save the bytes that are overwritten by {readable byte count, frame type} */
            tmp_buf[0] = dst[0]; /* save last two bytes of previous buffer read */
            tmp_buf[1] = dst[1]; /* which will get overwritten by readable_byte_count and frame_type of next read */

            /* Move TCPC window to next region in buffer */
            (void)Reg_BusWriteBlock(pdInstance, (uint8_t *)&ClrValue, 2, alert);

            /* Re-read count */
            (void)Reg_BusReadBlock(pdInstance, receive_byte_count, 1, &rx_buf_count);
            total_bytes_received += (uint16_t)rx_buf_count - 1U;

            /* Note: I am assuming that there is no need to limit
             * read length to 132 bytes */
            /* Should have finished reading ext message, but just in case
             * Assume that receive_byte_count will be zero once ext message is completely read */
            (void)Reg_BusReadBlock(pdInstance, receive_byte_count, (uint32_t)rx_buf_count + 1U, dst);

            dst[0] = tmp_buf[0]; /* restore overwritten bytes */
            dst[1] = tmp_buf[1];
        }

        rxResult->rxLength       = total_bytes_received;
        rxResult->rxResultStatus = kStatus_PD_Success;
    }
    else
    {
        /* MEAD vendor defined method does not use padding. */
        uint8_t i;
        int16_t data_received;
        int16_t data_remain;

        dst += 2;
        i                  = dst[2] & (uint8_t)PD_MSG_EXT_HEADER_DATA_SIZE_MASK;
        data_received      = (int16_t)i;
        rxResult->rxLength = (uint16_t)data_received + 2U + 2U; /* add ext header and normal header */
        dst += (2 + 28);                                        /* head and first packet has been buffer */

        /* %%% FIXME: These registers are different per-chip, needs Hal layer %%% */
        if (data_received > 26)
        {
            /* need to use TCPC extended rx buffer */
            data_remain = data_received - 26;
            i           = 1;
            while (data_remain > 0)
            {
                /* set ext_msg_index */
                Reg_BusWriteByte(pdInstance, ptn5110_ext_msg_index, i);
                /* copy data (30 bytes) from tcpc rx buffer */
                (void)Reg_BusReadBlock(pdInstance, rx_buf_header, 30, dst);
                data_remain -= 30;
                dst += 30;
                i++;
            }
            Reg_BusWriteByte(pdInstance, ptn5110_ext_msg_index, 0);
        }
        rxResult->rxResultStatus = kStatus_PD_Success;
    }
}
#endif

/*!
 * @brief Receive message in the receive buffer.
 *
 * This function is used to receive message in the receive buffer.
 * This function is called by the function PDPTN5110_MsgHalReceiveCompleteCallback.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param dest            The receive buffer. The format of dest - 0-readable_byte_count, 1-rx_buf_frame_type,
 *                        2-MsgHeaderLowByte, 3-MsgHeaderHighByte, 4-Byte0, 5-Byte1, ..., N-ByteN.
 * @param rxResult        It is out parameter. Please refer to the enumeration pd_phy_rx_result_t.
 *
 * @return None.
 */
static void PDPTN5110_MsgHalRxReceiveBuffer(pd_instance_t *pdInstance, uint8_t *dest, pd_phy_rx_result_t *rxResult)
{
#if defined(PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
    pd_msg_header_t msgHeader;
    pd_extended_msg_header_t extendMsgHeader;
#endif

    /* Reading the RECEIVE_BUFFER from TCPC will include 0-RECEIVE_BYTE_COUNT, 1-RX_BUF_FRAME_TYPE, */
    /* 2-RX_BUF_HEADER_BYTE_0, 3-RX_BUF_HEADER_BYTE_1 */
    (void)Reg_BusReadBlock(pdInstance, receive_byte_count, 4, dest);

    if (dest[0] < 3U) /* receive_byte_count < 3 */
    {
        rxResult->rxSop          = kPD_MsgSOPInvalid;
        rxResult->rxLength       = 0U;
        rxResult->rxResultStatus = kStatus_PD_Error;
        return;
    }
    else
    {
        rxResult->rxSop = (start_of_packet_t)dest[1]; /* rx_buf_frame_type */
    }

    PDPTN5110_MsgHalRcvdControlDataOrChunked(pdInstance, dest, rxResult);

#if defined(PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
    msgHeader.msgHeaderVal = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)dest + 2));
    if (msgHeader.bitFields.extended != 0U)
    {
        extendMsgHeader.extendedMsgHeaderVal = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)dest + 4));
        if (extendMsgHeader.bitFields.chunked == 0U)
        {
            PDPTN5110_MsgHalRcvdExtMsgUnChunked(pdInstance, dest, rxResult);
        }
    }
#endif
}

/*!
 * @brief Enter the HW bist mode.
 *
 * This function is used to enter the hardware bist mode.
 * TCPC will send a test frame, continuous pattern or setup the hardware to receive bist.
 * @note This will also enable the tx_done interrupt for TX mode.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param mode            Please refer to the enumeration pd_bist_mst_t.
 *                        kBIST_TestData - TCPC will setup the hardware to receive bist.
 *                        kBIST_CarrierMode2 - TCPC will send a test frame, continuous pattern.
 *
 * @return None.
 */
void PDPTN5110_MsgHalEnterBist(pd_instance_t *pdInstance, pd_bist_mst_t mode)
{
    /* TCPC supports only two mode: kBIST_CarrierMode2, kBIST_TestData */
    if (mode == kBIST_TestData)
    {
        Reg_CacheAndBusModifyByteField(pdInstance, CONTROL, tcpc_control, TCPC_TCPC_CONTROL_BIST_TEST_MODE_MASK,
                                       TCPC_TCPC_CONTROL_BIST_TEST_MODE_MASK);
    }
    else if (mode == kBIST_CarrierMode2)
    {
        PDPTN5110_MsgHalTxStartTransmit(pdInstance, NO_MESSAGE_RETRY_IS_REQUIRED,
                                        (uint8_t)TRANSMIT_TRANSMIT_SOP_MESSAGE_TRANSMIT_BIST_CARRIER_MODE_2);
    }
    else
    {
        /* No action required. */
    }
}

/*!
 * @brief Exit the HW bist mode.
 *
 * This function is used to exit the hardware bist mode.
 * @note TCPC shall exit the BIST Carrier Mode 2 no later than tBISTContMode max(30 ~ 60ms).
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 *
 * @return None.
 */
void PDPTN5110_MsgHalExitBist(pd_instance_t *pdInstance)
{
    if (Reg_CacheReadByteField(pdInstance, CONTROL, tcpc_control, TCPC_TCPC_CONTROL_BIST_TEST_MODE_MASK) != 0U)
    {
        Reg_CacheAndBusModifyByteField(pdInstance, CONTROL, tcpc_control, TCPC_TCPC_CONTROL_BIST_TEST_MODE_MASK, 0);
    }
}

/*!
 * @brief Callback for message sent.
 *
 * This function is used to notify message has been sent.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param sendResult      Please refer to the enumerarion pd_status_t.
 *
 * @return None.
 */
void PDPTN5110_MsgHalSendCompleteCallback(pd_instance_t *pdInstance, pd_status_t sendResult)
{
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    if (pdInstance->sendingState == 1U)
    {
        OSA_EXIT_CRITICAL();
        /* notify pd stack */
        PD_Notify(pdInstance, PD_PHY_EVENT_SEND_COMPLETE, &sendResult);
    }
    else
    {
        OSA_EXIT_CRITICAL();
    }
}

/*!
 * @brief Callback for message received.
 *
 * This function is used to receive message and notify message has been received.
 *
 * @param pdInstance      The pd handle. It equals the value returned from PD_InstanceInit.
 * @param rxState         Please refer to the enumerarion pd_status_t.
 *
 * @return None.
 */
void PDPTN5110_MsgHalReceiveCompleteCallback(pd_instance_t *pdInstance, pd_status_t rxState)
{
    pd_phy_rx_result_t rxResult;
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    if (pdInstance->receiveState == 1U)
    {
        OSA_EXIT_CRITICAL();

        if (rxState == kStatus_PD_Success)
        {
            PDPTN5110_MsgHalRxReceiveBuffer(pdInstance, (uint8_t *)pdInstance->receivingDataBuffer, &rxResult);
            if (rxResult.rxResultStatus != kStatus_PD_Success)
            {
                return;
            }
        }
        else
        {
            return;
        }
        PD_Notify(pdInstance, PD_PHY_EVENT_RECEIVE_COMPLETE, &rxResult);
    }
    else
    {
        OSA_EXIT_CRITICAL();
        if (rxState == kStatus_PD_Success)
        {
            /* execute here indicating that the data in the local buffer hasn't been proccessed yet.
               There is a possible senario that if partner contiguously sends message, PHY will save two messages to
               the RX buffer and assert #Alert pin twice, so pend a message here. */
            pdInstance->pendingMsg = 1;
            PDPTN5110_MsgHalRxReceiveBuffer(pdInstance, (uint8_t *)&(pdInstance->receivingDataCache[0]),
                                            (pd_phy_rx_result_t *)&(pdInstance->cacheRxResult));
        }
    }
}
