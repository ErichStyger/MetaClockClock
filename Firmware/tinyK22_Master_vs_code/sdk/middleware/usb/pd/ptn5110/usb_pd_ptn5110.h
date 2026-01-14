/*
 * Copyright 2016 - 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PD_PTN5110_H__
#define __PD_PTN5110_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define PD_INTERFACE_REV2VER1              (0x2010U)
#define MSG_TCPC_MAX_TX_WINDOW_WRITE_BURST (132U)

#define USBPD_ENABLE_PTN5110_A0R1_WORKAROUNDS (0U)
#define USBPD_ENABLE_PTN5110_A0R2_WORKAROUNDS (1U)

#define TCPC_TX_BUF_HEADER_LEN       (2U)
#define TCPC_TRANSMIT_BYTE_COUNT_LEN (1U)
/* For TCPC the registers are not mapped to local memory, we need to use an intermediate buffer uint8_t msg_buf[32]; */
/* When sending TRANSMIT_BUFFER through I2c, the TRANSMIT_BYTE_COUNT and TX_BUF_HEADER_BYTE_0/1 must been sent together
 */
/* with TX_BUF_OBJn_BYTE as per USB-Port Conroller Specification 4.3.5 Writing the TRANSMIT_BUFFER */
/* So define this array to contain all buf content. */
/* When transmiting, 0-reserved, 1-TRANSMIT_BYTE_COUNT, 2-TX_BUF_HEADER_BYTE_0, 3-TX_BUF_HEADER_BYTE_1, */
/* 4~31-TX_BUF_OBJn_BYTE */
/* When receiving, 0-RECEIVE_BYTE_COUNT, 1-RX_BUF_FRAME_TYPE, 2-RX_BUF_HEADER_BYTE_0, 3-RX_BUF_HEADER_BYTE_1, */
/* 4~31-RX_BUF_OBJn_BYTE */
#define TCPC_TRANSMIT_BYTE_COUNT_POS (1U)
#define TCPC_TX_BUF_HEADER_BYTE_POS  (2U)

#define TCPC_TX_BUF_AVAILABLE (0xFFU) /* If TCPC allows message to be sent, set to this state */
/* If a message is transmitting, no other message execept for Hard_Reset can be send through TCPC. Set to this state */
#define TCPC_TX_BUF_UNAVAILABLE (0xEEU)

/* VBus definition */
/* Unit: 25mV, Real value is 800mV as per USBPD R3 V0.91 Table 7-18 Common Source/Sink Electrical Parameters */
/* To pass Ellisys compliance test TD.PD.PC.E2, use 775mv as vSafe0V threshold here. */
#define VBUS_VSAFE0V_MAX_THRESHOLD (31U)
/* Unit: 25mV, Real value is 400mV as per USBPD R3 V0.91 Table 7-18 Common Source/Sink Electrical Parameters */
#define VBUS_VSAFE0V_MIN_THRESHOLD (16U)
/* Unit: 25mV, Real value is 4000mV as per USB3.1 Table 11-2. DC Electrical Characteristics when working as a  upstream
 * connector */
#define VBUS_VSAFE5V_MIN_THRESHOLD (160U)
/* 5.5V */
#define VBUS_VSAFE5V_MAX_THRESHOLD (220U)
/* 3.5V */
#define VBUS_SINK_DISCONNECT_THRESHOLD (140U)
/* 0.6V */
#define VBUS_STOP_DISCHARGE_THRESHOLD (24U)

#define TX_ABORT_MASK (TCPC_ALERT_TRANSMIT_SOP_MESSAGE_FAILED_MASK | TCPC_ALERT_TRANSMIT_SOP_MESSAGE_DISCARDED_MASK)
#define TX_DONE_MASK  (TX_ABORT_MASK | TCPC_ALERT_TRANSMIT_SOP_MESSAGE_SUCCESSFUL_MASK)
#define TX_HARDRESET_MASK \
    (TCPC_ALERT_TRANSMIT_SOP_MESSAGE_FAILED_MASK | TCPC_ALERT_TRANSMIT_SOP_MESSAGE_SUCCESSFUL_MASK)

#define PRODUCT_ID_PTN5110     (0x5110U)
#define DEVICE_ID_PTN5110_A0R1 (0x0000U)
#define DEVICE_ID_PTN5110_A0R2 (0x0001U)
/* DEVICE_ID value 2 is intentionally skipped to align nomenclature */
#define DEVICE_ID_PTN5110_A1R3 (0x0003U)
#define DEVICE_ID_PTN5110_A1R4 (0x0004U)

/* defined - use PDPTN5110_VbusDischarge() below */
#define HAL_SUPPORTS_VBUS_DISCHARGE (1U)

#define TcpcRegMask(REG_NAME, FIELD_NAME) (TCPC_##REG_NAME##_##FIELD_NAME##_MASK)
#define TcpcRegLsb(REG_NAME, FIELD_NAME)  (TCPC_##REG_NAME##_##FIELD_NAME##_LSB)
#define TcpcReadField(VALUE, REG_NAME, FIELD_NAME) \
    ((((uint32_t)VALUE) & (uint32_t)TcpcRegMask(REG_NAME, FIELD_NAME)) >> (uint32_t)TcpcRegLsb(REG_NAME, FIELD_NAME))

/* NOTE: masks should be defined in the device specific header */
typedef enum
{
    kRegModule_Intc           = 1 << 0,
    kRegModule_Mask           = 1 << 1,
    kRegModule_MsgRX          = 1 << 2,
    kRegModule_ConfStdOut     = 1 << 3,
    kRegModule_Control        = 1 << 4,
    kRegModule_MsgTX          = 1 << 5,
    kRegModule_Command        = 1 << 6,
    kRegModule_Capability     = 1 << 7,
    kRegModule_Vbus           = 1 << 8,
    kRegModule_CCStatus       = 1 << 9,
    kRegModule_PowerStatus    = 1 << 10,
    kRegModule_FaultStatus    = 1 << 11,
    kRegModule_ExtendedStatus = 1 << 13,
    kRegModule_AlertExtended  = 1 << 14,
    kRegModule_All            = 0xFFFFU,
} RegModuleMaskTcpc_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#define RoleControlIsToggleSrcFirst(roleControl)                                                   \
    (((roleControl) & ((uint8_t)TCPC_ROLE_CONTROL_DRP_MASK | (uint8_t)TCPC_ROLE_CONTROL_CC2_MASK | \
                       (uint8_t)TCPC_ROLE_CONTROL_CC1_MASK)) ==                                    \
     ((uint8_t)TCPC_ROLE_CONTROL_DRP_MASK | (uint8_t)ROLE_CONTROL_CC2_RP | (uint8_t)ROLE_CONTROL_CC1_RP))

#define RoleControlIsBothRp(roleControl)                                                              \
    (((roleControl) & ((uint8_t)TCPC_ROLE_CONTROL_CC2_MASK | (uint8_t)TCPC_ROLE_CONTROL_CC1_MASK)) == \
     ((uint8_t)ROLE_CONTROL_CC2_RP | (uint8_t)ROLE_CONTROL_CC1_RP))

#define RoleControlIsToggleSnkFirst(roleControl)                                                   \
    (((roleControl) & ((uint8_t)TCPC_ROLE_CONTROL_DRP_MASK | (uint8_t)TCPC_ROLE_CONTROL_CC2_MASK | \
                       (uint8_t)TCPC_ROLE_CONTROL_CC1_MASK)) ==                                    \
     ((uint8_t)TCPC_ROLE_CONTROL_DRP_MASK | (uint8_t)ROLE_CONTROL_CC2_RD | (uint8_t)ROLE_CONTROL_CC1_RD))

#define RoleControlIsBothRd(roleControl)                                                              \
    (((roleControl) & ((uint8_t)TCPC_ROLE_CONTROL_CC2_MASK | (uint8_t)TCPC_ROLE_CONTROL_CC1_MASK)) == \
     ((uint8_t)ROLE_CONTROL_CC2_RD | (uint8_t)ROLE_CONTROL_CC1_RD))

#define REG_ADDR(REG) (ADDR_##REG)

#define Reg_CacheRead(pdInstance, MODULE, REGISTER)         ((pdInstance)->tcpcRegCache.MODULE.REGISTER)
#define Reg_CacheWrite(pdInstance, MODULE, REGISTER, VALUE) ((pdInstance)->tcpcRegCache.MODULE.REGISTER) = (VALUE)

#define Reg_CacheReadByteField(pdInstance, MODULE, REGISTER, MASK) \
    (((uint8_t)((pdInstance)->tcpcRegCache.MODULE.REGISTER)) & ((uint8_t)(MASK)))

#define Reg_CacheReadWordField(pdInstance, MODULE, REGISTER, MASK) \
    (((uint16_t)((pdInstance)->tcpcRegCache.MODULE.REGISTER)) & ((uint16_t)(MASK)))

#define Reg_CacheWriteByteField(pdInstance, MODULE, REGISTER, MASK, UPDATE_VALUE)                      \
    ((pdInstance)->tcpcRegCache.MODULE.REGISTER) =                                                     \
        ((((uint8_t)((pdInstance)->tcpcRegCache.MODULE.REGISTER)) & ((uint8_t)(~((uint8_t)(MASK))))) | \
         (((uint8_t)(UPDATE_VALUE)) & ((uint8_t)(MASK))))

#define Reg_CacheWriteWordField(pdInstance, MODULE, REGISTER, MASK, UPDATE_VALUE)                         \
    ((pdInstance)->tcpcRegCache.MODULE.REGISTER) =                                                        \
        ((((uint16_t)((pdInstance)->tcpcRegCache.MODULE.REGISTER)) & ((uint16_t)(~((uint16_t)(MASK))))) | \
         (((uint16_t)(UPDATE_VALUE)) & ((uint16_t)(MASK))))

#define Reg_BusReadByte(pdInstance, REGISTER) Reg_BusReadByteFun(pdInstance, REG_ADDR(REGISTER))

#define Reg_BusReadWord(pdInstance, REGISTER) Reg_BusReadWordFun(pdInstance, REG_ADDR(REGISTER))

#define Reg_BusReadBlock(pdInstance, REGISTER, length, dst) \
    Reg_BusReadBlockFun(pdInstance, REG_ADDR(REGISTER), (length), (dst))

#define Reg_BusWriteByte(pdInstance, REGISTER, byteData) Reg_BusWriteByteFun(pdInstance, REG_ADDR(REGISTER), byteData)

#define Reg_BusWriteWord(pdInstance, REGISTER, wordData) Reg_BusWriteWordFun(pdInstance, REG_ADDR(REGISTER), wordData)

#define Reg_BusWriteBlock(pdInstance, src, length, REGISTER) \
    Reg_BusWriteBlockFun(pdInstance, REG_ADDR(REGISTER), (length), (src))

#define Reg_BusReadByteField(pdInstance, REGISTER, MASK) (Reg_BusReadByteFun(pdInstance, REG_ADDR(REGISTER)) & (MASK))

#define Reg_BusReadWordField(pdInstance, REGISTER, MASK) (Reg_BusReadWordFun(pdInstance, REG_ADDR(REGISTER)) & (MASK))

#define Reg_CacheAndBusModifyByteField(pdInstance, MODULE, REGISTER, MASK, UPDATE_VALUE) \
    Reg_CacheWriteByteField(pdInstance, MODULE, REGISTER, MASK, UPDATE_VALUE);           \
    Reg_BusWriteByteFun(pdInstance, REG_ADDR(REGISTER), ((pdInstance)->tcpcRegCache.MODULE.REGISTER))

#define Reg_CacheAndBusModifyWordField(pdInstance, MODULE, REGISTER, MASK, UPDATE_VALUE) \
    Reg_CacheWriteWordField(pdInstance, MODULE, REGISTER, MASK, UPDATE_VALUE);           \
    (void)Reg_BusWriteBlock(pdInstance, (uint8_t *)&((pdInstance)->tcpcRegCache.MODULE.REGISTER), 2, REGISTER)

#define Reg_BusModifyByteField(pdInstance, REGISTER, MASK, UPDATE_VALUE) \
    Reg_BusModifyByteFieldFun(pdInstance, REG_ADDR(REGISTER), MASK, UPDATE_VALUE)

#define Reg_BusModifyWordField(pdInstance, REGISTER, MASK, UPDATE_VALUE) \
    Reg_BusModifyWordFieldFun(pdInstance, REG_ADDR(REGISTER), MASK, UPDATE_VALUE)

#define Reg_BusClrBit(pdInstance, REGISTER, field)      \
    Reg_BusWriteByteFun(pdInstance, REG_ADDR(REGISTER), \
                        Reg_BusReadByteFun(pdInstance, REG_ADDR(REGISTER)) & (~(uint8_t)(field)))

#define Reg_BusSetBit(pdInstance, REGISTER, field)      \
    Reg_BusWriteByteFun(pdInstance, REG_ADDR(REGISTER), \
                        Reg_BusReadByteFun(pdInstance, REG_ADDR(REGISTER)) | ((uint8_t)(field)))

/* I2C */
uint8_t Reg_BusReadByteFun(pd_instance_t *pdInstance, uint32_t reg);
void Reg_BusWriteByteFun(pd_instance_t *pdInstance, uint32_t reg, uint8_t byteData);
uint16_t Reg_BusReadWordFun(pd_instance_t *pdInstance, uint32_t reg);
void Reg_BusWriteWordFun(pd_instance_t *pdInstance, uint32_t reg, uint16_t wordData);
pd_status_t Reg_BusReadBlockFun(pd_instance_t *pdInstance, uint32_t reg, uint32_t length, uint8_t *dst);
pd_status_t Reg_BusWriteBlockFun(pd_instance_t *pdInstance, uint32_t reg, uint32_t length, uint8_t *src);
void Reg_BusModifyByteFieldFun(pd_instance_t *pdInstance, uint32_t reg, uint16_t mask, uint8_t updateVal);
void Reg_BusModifyWordFieldFun(pd_instance_t *pdInstance, uint32_t reg, uint16_t mask, uint16_t updateVal);

/* Connect */
void PDPTN5110_ConnectControlAutoDischarge(pd_instance_t *pdInstance, uint8_t enable);
void PDPTN5110_ConnectGetCC(pd_instance_t *pdInstance, pd_phy_get_cc_state_t *ccStates);
void PDPTN5110_ConnectSetCCOrientaion(pd_instance_t *pdInstance, pd_cc_type_t usedCC);
void PDPTN5110_ConnectControlDebugAccessory(pd_instance_t *pdInstance, uint8_t enable);
void PDPTN5110_ConnectSyncRoleControl(pd_instance_t *pdInstance);
void PDPTN5110_ConnectAssertRpAttached(pd_instance_t *pdInstance, pd_cc_type_t usedCC, uint8_t srcRp);
void PDPTN5110_ConnectAssertRpUnattached(pd_instance_t *pdInstance, uint8_t srcRp);
void PDPTN5110_ConnectAssertRpDbgAccSrc(pd_instance_t *pdInstance);
void PDPTN5110_ConnectAssertRdDbgAccSnk(pd_instance_t *pdInstance);
void PDPTN5110_ConnectAssertRdAttached(pd_instance_t *pdInstance, pd_cc_type_t usedCC);
void PDPTN5110_ConnectAssertRdUnattached(pd_instance_t *pdInstance);
void PDPTN5110_ConnectAssertDrpUnattached(pd_instance_t *pdInstance, uint8_t startSrc, uint8_t srcRp);
void PDPTN5110_ConnectDbgAccSnkAttached(pd_instance_t *pdInstance);
void PDPTN5110_ConnectDbgAccSrcAttached(pd_instance_t *pdInstance);
void PDPTN5110_ConnectAudioAccessoryAttached(pd_instance_t *pdInstance, pd_cc_type_t usedCC);
void PDPTN5110_ConnectDetachOrDisable(pd_instance_t *pdInstance);
void PDPTN5110_ConnectRawVbusDischarge(pd_instance_t *pdInstance, uint8_t enable);
#if defined(PD_CONFIG_VCONN_SUPPORT) && (PD_CONFIG_VCONN_SUPPORT)
void PDPTN5110_ConnectSwitchVConn(pd_instance_t *pdInstance, uint8_t enable);
void PDPTN5110_ConnectRawVconnDischarge(pd_instance_t *pdInstance, uint8_t enable);
#endif
void PDPTN5110_ConnectDetectAttach(pd_instance_t *pdInstance);
void PDPTN5110_ConnectDetectDetach(pd_instance_t *pdInstance);
void PDPTN5110_ConnectSrcSetTypecCurrent(pd_instance_t *pdInstance, pd_cc_type_t usedCC, uint8_t srcRp);
uint8_t PDPTN5110_ConnectGetTypeCCurrent(pd_instance_t *pdInstance, pd_cc_type_t usedCC);

/* Message */
void PDPTN5110_MsgHalSetRxSopEnable(pd_instance_t *pdInstance, uint8_t sop_mask);
void PDPTN5110_MsgHalDisableMessageRx(pd_instance_t *pdInstance);
void PDPTN5110_MsgHalSetMessageHeaderInfo(pd_instance_t *pdInstance,
                                          uint8_t revision,
                                          uint8_t powerRole,
                                          uint8_t dataRole);
void PDPTN5110_MsgHalSendReset(pd_instance_t *pdInstance, uint8_t hardResetOrCableReset);
pd_status_t PDPTN5110_MsgHalSendControlDataOrChunked(pd_instance_t *pdInstance,
                                                     uint8_t sop,
                                                     uint8_t *buffer,
                                                     uint8_t count);
#if defined(PD_CONFIG_EXTENDED_MSG_SUPPORT) && (PD_CONFIG_EXTENDED_MSG_SUPPORT)
pd_status_t PDPTN5110_MsgHalSendUnchunked(pd_instance_t *pdInstance, uint8_t sop, uint8_t *buffer, uint16_t dataSize);
#endif
void PDPTN5110_MsgHalEnterBist(pd_instance_t *pdInstance, pd_bist_mst_t mode);
void PDPTN5110_MsgHalExitBist(pd_instance_t *pdInstance);
void PDPTN5110_MsgHalSendCompleteCallback(pd_instance_t *pdInstance, pd_status_t sendResult);
void PDPTN5110_MsgHalReceiveCompleteCallback(pd_instance_t *pdInstance, pd_status_t rxState);

/* HAL */
pd_status_t PDPTN5110_HalInit(pd_handle pdHandle);
void PDPTN5110_RegCacheSynC(pd_instance_t *pdInstance, RegModuleMaskTcpc_t mask);
void PDPTN5110_IntcIrqClearAndEnable(pd_instance_t *pdInstance, uint16_t mask);
void PDPTN5110_IntcIrqEnable(pd_instance_t *pdInstance, uint16_t mask);
void PDPTN5110_IntcIrqDisable(pd_instance_t *pdInstance, uint16_t mask);
uint8_t PDPTN5110_VsysIsPresent(pd_instance_t *pdInstance);
uint16_t PDPTN5110_GetVbusVoltage(pd_instance_t *pdInstance);
void PDPTN5110_ControlVbusVoltageAlarm(pd_instance_t *pdInstance, uint8_t enable);
void PDPTN5110_SetVbusVoltageAlarmHiLo(pd_instance_t *pdInstance, uint32_t vbusVoltage);
void PDPTN5110_ControlVbusMonitorAndDetect(pd_instance_t *pdInstance, uint8_t enable);
void PDPTN5110_ControlVbusSourceOrSink(pd_instance_t *pdInstance, pd_ptn5110_ctrl_pin_t *set);
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
void PDPTN5110_SignalFRSwap(pd_instance_t *pdInstance);
void PDPTN5110_EnableFRSwap(pd_instance_t *pdInstance);
#endif
void PDPTN5110_DisableFRSwap(pd_instance_t *pdInstance);
void PDPTN5110_IntcProcessIntAll(pd_instance_t *pdInstance);
#if defined(PD_CONFIG_PHY_LOW_POWER_LEVEL) && (PD_CONFIG_PHY_LOW_POWER_LEVEL)
void PDPTN5110_EnterLowPowerMode(pd_instance_t *pdInstance);
void PDPTN5110_ExitLowPowerMode(pd_instance_t *pdInstance);
#endif

#endif
