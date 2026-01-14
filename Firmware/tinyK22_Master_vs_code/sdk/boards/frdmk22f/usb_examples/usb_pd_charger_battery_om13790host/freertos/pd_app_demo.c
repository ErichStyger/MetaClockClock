/*
 * Copyright 2016 - 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_pd.h"
#include "usb_pd_i2c.h"
#include "pd_app.h"
#include "fsl_debug_console.h"
#include "pd_power_interface.h"
#include "pd_board_config.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void PD_AppPortInit(void *arg);
void PD_PortTask(void *arg);
void PD_DemoReset(pd_app_t *pdAppInstance);
void PD_DemoInit(void);
void PD_Demo1msIsrProcess(void);
pd_status_t PD_DemoFindPDO(pd_app_t *pdAppInstance, pd_rdo_t *rdo, uint32_t *voltage);
void PD_DemoTaskFun();
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

static uint32_t PD_DemoStartUpGetBatteryValue(pd_app_t *pdAppInstance)
{
    return 30; /* default start up battery is 5% */
}

static void PD_DemoBatteryChange(pd_app_t *pdAppInstance)
{
    if (pdAppInstance->selfPowerRole == kPD_PowerRoleSource)
    {
        if (pdAppInstance->batteryQuantity > 5)
        {
            pdAppInstance->batteryQuantity--;
        }

        /* battery consume faster */
        if (pdAppInstance->partnerRequestRDO.bitFields.objectPosition == 2)
        {
            if (pdAppInstance->batteryQuantity > 5)
            {
                pdAppInstance->batteryQuantity--;
            }
        }
    }
    else
    {
        if (pdAppInstance->batteryQuantity < 100)
        {
            pdAppInstance->batteryQuantity++;
        }

        /* battery charge faster */
        if (pdAppInstance->sinkRequestVoltage == PD_DEMO_BATTERY_CHARGE_REQUEST_VOLTAGE)
        {
            if (pdAppInstance->batteryQuantity < 100)
            {
                pdAppInstance->batteryQuantity++;
            }
        }
    }
}

void PD_DemoReset(pd_app_t *pdAppInstance)
{
    if (pdAppInstance->batteryQuantity > 30)
    {
        ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount = 2;
    }
    else
    {
        ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount = 1;
    }
    pdAppInstance->partnerRequestRDO.bitFields.objectPosition = 0; /* invalid */
    pdAppInstance->contractValid                              = 0;
    pdAppInstance->demoState                                  = kDemoState_Start;
    pdAppInstance->partnerSourceCapNumber                     = 0;
}

void PD_DemoInit(void)
{
    uint8_t index;

    for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        g_PDAppInstanceArray[index]->batteryQuantity = PD_DemoStartUpGetBatteryValue(g_PDAppInstanceArray[index]);
        g_PDAppInstanceArray[index]->batteryChange   = 0;
        PD_DemoReset(g_PDAppInstanceArray[index]);
    }

    return;
}

void PD_Demo1msIsrProcess(void)
{
    uint8_t index;
    static volatile uint32_t delay = 0;
    delay++;

    /* 1.2 s */
    if (delay >= 1200)
    {
        delay = 0;
        for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
        {
            if (!g_PDAppInstanceArray[index]->contractValid)
            {
                continue;
            }
            if (g_PDAppInstanceArray[index]->pdHandle != NULL)
            {
                PD_DemoBatteryChange(g_PDAppInstanceArray[index]);
                g_PDAppInstanceArray[index]->batteryChange = 1;

                if (g_PDAppInstanceArray[index]->retrySwapDelay > 0)
                {
                    g_PDAppInstanceArray[index]->retrySwapDelay--;
                }
            }
        }
    }
}

pd_status_t PD_DemoFindPDO(pd_app_t *pdAppInstance, pd_rdo_t *rdo, uint32_t *voltage)
{
    uint32_t index;
    pd_source_pdo_t sourcePDO;
    uint32_t requestVoltage;
    uint32_t requestCurrent;
    uint8_t findSourceCap = 0;

    if (pdAppInstance->partnerSourceCapNumber == 0)
    {
        return kStatus_PD_Error;
    }

    /* default rdo as 5V - 0.9A */
    *voltage                                = 5000;
    rdo->bitFields.objectPosition           = 1;
    rdo->bitFields.giveBack                 = 0;
    rdo->bitFields.capabilityMismatch       = 0;
    rdo->bitFields.usbCommunicationsCapable = 0;
    rdo->bitFields.noUsbSuspend             = 1;
    rdo->bitFields.operateValue             = 500 / PD_PDO_CURRENT_UNIT;
    rdo->bitFields.maxOrMinOperateValue     = rdo->bitFields.operateValue;
    if (pdAppInstance->batteryQuantity < 100)
    {
        requestVoltage = PD_DEMO_BATTERY_CHARGE_REQUEST_VOLTAGE;
        requestCurrent = PD_DEMO_BATTERY_CHARGE_REQUEST_CURRENT;
    }
    else
    {
        requestVoltage = PD_DEMO_BATTERY_FULL_REQUEST_VOLTAGE;
        requestCurrent = PD_DEMO_BATTERY_FULL_REQUEST_CURRENT;
    }

    for (index = 0; index < pdAppInstance->partnerSourceCapNumber; ++index)
    {
        sourcePDO.PDOValue = pdAppInstance->partnerSourceCaps[index].PDOValue;
        switch (sourcePDO.commonPDO.pdoType)
        {
            case kPDO_Fixed:
            {
                if ((sourcePDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT == requestVoltage) &&
                    (sourcePDO.fixedPDO.maxCurrent * PD_PDO_CURRENT_UNIT >= requestCurrent))
                {
                    *voltage                            = sourcePDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT;
                    rdo->bitFields.objectPosition       = (index + 1);
                    rdo->bitFields.operateValue         = requestCurrent / PD_PDO_CURRENT_UNIT;
                    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;
                    findSourceCap                       = 1;
                }
                break;
            }

            case kPDO_Variable:
            {
                if ((sourcePDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT <= requestVoltage) &&
                    (sourcePDO.variablePDO.maxVoltage * PD_PDO_VOLTAGE_UNIT >= requestVoltage) &&
                    (sourcePDO.variablePDO.maxCurrent * PD_PDO_CURRENT_UNIT >= requestCurrent))
                {
                    *voltage                            = sourcePDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT;
                    rdo->bitFields.objectPosition       = (index + 1);
                    rdo->bitFields.operateValue         = requestCurrent / PD_PDO_CURRENT_UNIT;
                    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;
                    findSourceCap                       = 1;
                }
                break;
            }

            case kPDO_Battery:
            {
                if ((sourcePDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT <= requestVoltage) &&
                    (sourcePDO.batteryPDO.maxVoltage * PD_PDO_VOLTAGE_UNIT >= requestVoltage) &&
                    (sourcePDO.batteryPDO.maxAllowPower * PD_PDO_POWER_UNIT >=
                     (requestVoltage * requestCurrent / 1000)))
                {
                    *voltage                            = sourcePDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT;
                    rdo->bitFields.objectPosition       = (index + 1);
                    rdo->bitFields.operateValue         = (requestVoltage * requestCurrent) / 1000 / PD_PDO_POWER_UNIT;
                    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;
                    findSourceCap                       = 1;
                }
                break;
            }

            default:
                break;
        }

        if (findSourceCap)
        {
            break;
        }
    }

    return kStatus_PD_Success;
}

static void PD_DemoPDReset(pd_app_t *pdAppInstance)
{
    pd_handle pdTmpHandle;
    uint32_t sr = 0u;

    PD_PowerSrcTurnOffVbus(pdAppInstance, kVbusPower_Stable);
#if defined(__GIC_PRIO_BITS)
    if ((__get_CPSR() & CPSR_M_Msk) == 0x13)
#else
    if (__get_IPSR())
#endif
    {
        sr = portSET_INTERRUPT_MASK_FROM_ISR();
    }
    else
    {
        portENTER_CRITICAL();
    }

    pdAppInstance->partnerSourceCapNumber = 0;
    pdAppInstance->contractValid          = 0;
    pdAppInstance->demoState              = kDemoState_Start;
    pdTmpHandle                           = pdAppInstance->pdHandle;
    vTaskDelete(pdAppInstance->pdTaskHandler);
    pdAppInstance->pdHandle = NULL;
    PD_InstanceDeinit(pdTmpHandle);
    PD_AppPortInit(pdAppInstance);

    if (xTaskCreate(PD_PortTask, "port", (1024u + 512u) / sizeof(portSTACK_TYPE), pdAppInstance, 5,
                    &pdAppInstance->pdTaskHandler) != pdPASS)
    {
        PRINTF("create pd task error\r\n");
    }

#if defined(__GIC_PRIO_BITS)
    if ((__get_CPSR() & CPSR_M_Msk) == 0x13)
#else
    if (__get_IPSR())
#endif
    {
        portCLEAR_INTERRUPT_MASK_FROM_ISR(sr);
    }
    else
    {
        portEXIT_CRITICAL();
    }
}

void PD_DemoTaskFun()
{
    pd_app_t *pdAppInstance;
    pd_status_t status = kStatus_PD_Error;
    uint8_t index;

    for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        if (g_PDAppInstanceArray[index]->pdHandle == NULL)
        {
            continue;
        }
        pdAppInstance = g_PDAppInstanceArray[index];

        if (!pdAppInstance->contractValid)
        {
            pdAppInstance->demoState = kDemoState_Start;
            continue;
        }

        if (pdAppInstance->batteryChange == 1)
        {
            pdAppInstance->batteryChange = 0;

            uint8_t chargeVoltage;
            if (pdAppInstance->selfPowerRole == kPD_PowerRoleSource)
            {
                chargeVoltage = (pdAppInstance->partnerRequestRDO.bitFields.objectPosition == 2 ? 9 : 5);
                if (pdAppInstance->batteryQuantity <= 5)
                {
                    chargeVoltage = 0;
                }
            }
            else
            {
                chargeVoltage = ((pdAppInstance->sinkRequestVoltage == PD_DEMO_BATTERY_FULL_REQUEST_VOLTAGE) ? 5 : 9);
            }
            PRINTF("port %d battery percent:%d, charge voltage:%dV\r\n", pdAppInstance->portNumber,
                   pdAppInstance->batteryQuantity, chargeVoltage);
        }

        if (pdAppInstance->runningPowerRole != pdAppInstance->selfPowerRole)
        {
            pdAppInstance->demoState        = kDemoState_Start;
            pdAppInstance->runningPowerRole = pdAppInstance->selfPowerRole;
            if (pdAppInstance->runningPowerRole == kPD_PowerRoleSource)
            {
                pdAppInstance->prSwapAccept = 1u;
            }
            else
            {
                pdAppInstance->prSwapAccept = 0u;
            }
        }

        if (pdAppInstance->demoState == kDemoState_Start)
        {
            if (pdAppInstance->runningPowerRole == kPD_PowerRoleSource)
            {
                pdAppInstance->demoState = kDemoState_TryChangeAsSink;
            }
            else
            {
                pdAppInstance->demoState = kDemoState_Idle;
            }
        }

        if (pdAppInstance->batteryQuantity > 30)
        {
            if (((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount == 1)
            {
                ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount = 2;
                PRINTF("change source caps as high power(5V/9V)\r\n");
            }
        }
        else
        {
            if (((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount == 2)
            {
                ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount = 1;
            }
        }

        /* reference to readme */
        if (pdAppInstance->runningPowerRole == kPD_PowerRoleSource)
        {
            pdAppInstance->prSwapAccept = 1;

            switch (pdAppInstance->demoState)
            {
                case kDemoState_Idle:
                    if (pdAppInstance->batteryQuantity > 5)
                    {
                        ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->drpTryFunction =
                            kTypecTry_None;
                    }

                    if ((pdAppInstance->batteryQuantity <= 30) &&
                        (pdAppInstance->partnerRequestRDO.bitFields.objectPosition == 2))
                    {
                        ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount = 1;
                        PRINTF("change source caps as low power(5V)\r\n");
                        pdAppInstance->partnerRequestRDO.bitFields.objectPosition = 0; /* invalid */
                        status = PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_POWER_NEGOTIATION, NULL);
                        if (status != kStatus_PD_Success)
                        {
                            PRINTF("error\r\n");
                        }
                    }
                    else if (pdAppInstance->batteryQuantity <= 20)
                    {
                        pdAppInstance->demoState = kDemoState_NeedChangeAsSink;
                    }
                    else
                    {
                    }
                    break;

                case kDemoState_TryChangeAsSink:
                    PRINTF("try to swap as sink\r\n");
                    pdAppInstance->trySwap    = 1;
                    pdAppInstance->retryCount = 2;
                    pdAppInstance->demoState  = kDemoState_GetPartnerSrcCap;
                    break;

                case kDemoState_NeedChangeAsSink:
                    PRINTF("need change as sink because battery is low\r\n");
                    pdAppInstance->trySwap    = 0;
                    pdAppInstance->retryCount = 2;
                    if (pdAppInstance->partnerSourceCapNumber > 0)
                    {
                        pdAppInstance->retryCount = 2;
                        pdAppInstance->demoState  = kDemoState_SwapAsSink;
                    }
                    else
                    {
                        pdAppInstance->demoState = kDemoState_GetPartnerSrcCap;
                    }
                    break;

                case kDemoState_GetPartnerSrcCap:
                    if (pdAppInstance->retryCount > 0)
                    {
                        PRINTF("get partner source caps\r\n");
                        /* get partner source cap */
                        pdAppInstance->commandWait   = 1;
                        pdAppInstance->commandResult = kCommandResult_Error;
                        status =
                            PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_GET_PARTNER_SOURCE_CAPABILITIES, NULL);
                        if (status == kStatus_PD_Success)
                        {
                            pdAppInstance->demoState = kDemoState_WaitPartnerSrcCap;
                        }
                        else
                        {
                            PRINTF("error\r\n");
                        }
                        pdAppInstance->retryCount--;
                    }
                    else
                    {
                        PRINTF("retry fail\r\n");
                        if (pdAppInstance->trySwap)
                        {
                            pdAppInstance->demoState = kDemoState_Idle;
                        }
                        else
                        {
                            pdAppInstance->demoState = kDemoState_SrcReducePower;
                        }
                    }
                    break;

                case kDemoState_WaitPartnerSrcCap:
                    if (pdAppInstance->commandWait == 0)
                    {
                        if (pdAppInstance->commandResult == kCommandResult_Success)
                        {
                            pd_rdo_t rdo;
                            uint32_t voltage;
                            PRINTF("receive source capabilities\r\n");

                            if (PD_DemoFindPDO(pdAppInstance, &rdo, &voltage) == kStatus_PD_Success)
                            {
                                if (pdAppInstance->trySwap)
                                {
                                    if (pdAppInstance->partnerSourceCaps[0].fixedPDO.externalPowered)
                                    {
                                        pdAppInstance->retryCount = 2;
                                        pdAppInstance->demoState  = kDemoState_SwapAsSink;
                                    }
                                    else
                                    {
                                        pdAppInstance->demoState = kDemoState_Idle;
                                    }
                                }
                                else
                                {
                                    pdAppInstance->retryCount = 2;
                                    pdAppInstance->demoState  = kDemoState_SwapAsSink;
                                }
                            }
                            else
                            {
                                PRINTF("source cap cannot satisfy reqeust\r\n");
                                if (pdAppInstance->trySwap)
                                {
                                    pdAppInstance->demoState = kDemoState_Idle;
                                }
                                else
                                {
                                    pdAppInstance->demoState = kDemoState_SrcReducePower;
                                }
                            }
                        }
                        else if (pdAppInstance->commandResult == kCommandResult_NotSupported)
                        {
                            PRINTF("partner don't support src cap\r\n");
                            if (pdAppInstance->trySwap)
                            {
                                pdAppInstance->demoState = kDemoState_Idle;
                            }
                            else
                            {
                                pdAppInstance->demoState = kDemoState_SrcReducePower;
                            }
                            PRINTF("partner don't support source function\r\n");
                        }
                        else
                        {
                            pdAppInstance->demoState = kDemoState_GetPartnerSrcCap;
                        }
                    }
                    break;

                case kDemoState_SwapAsSink:
                    if (pdAppInstance->retryCount > 0)
                    {
                        PRINTF("try pr swap\r\n");
                        pdAppInstance->commandWait   = 1;
                        pdAppInstance->commandResult = kCommandResult_Error;
                        status = PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_PR_SWAP, NULL);
                        if (status == kStatus_PD_Success)
                        {
                            pdAppInstance->demoState = kDemoState_WaitPRSwap;
                        }
                        else
                        {
                            PRINTF("error\r\n");
                        }
                        pdAppInstance->retryCount--;
                    }
                    else
                    {
                        PRINTF("retry fail\r\n");
                        if (pdAppInstance->trySwap)
                        {
                            pdAppInstance->demoState = kDemoState_Idle;
                        }
                        else
                        {
                            pdAppInstance->demoState = kDemoState_SrcReducePower;
                        }
                    }
                    break;

                case kDemoState_WaitPRSwap:
                    if (pdAppInstance->commandWait == 0)
                    {
                        if (pdAppInstance->commandResult == kCommandResult_Success)
                        {
                            /* code cannot execute here, will go to the sink code */
                        }
                        else if ((pdAppInstance->commandResult == kCommandResult_NotSupported) ||
                                 (pdAppInstance->commandResult == kCommandResult_Reject))
                        {
                            PRINTF("swap reject\r\n");
                            if (pdAppInstance->trySwap)
                            {
                                pdAppInstance->demoState = kDemoState_Idle;
                            }
                            else
                            {
                                pdAppInstance->demoState = kDemoState_SrcReducePower;
                            }
                        }
                        else
                        {
                            pdAppInstance->demoState = kDemoState_SwapAsSink; /* retry */
                        }
                    }
                    break;

                case kDemoState_SrcReducePower:
                    if (pdAppInstance->partnerRequestRDO.bitFields.objectPosition != 1)
                    {
                        PRINTF("source reduce power\r\n");
                        ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount = 1;
                        PRINTF("change source caps as low power(5V)\r\n");
                        pdAppInstance->partnerRequestRDO.bitFields.objectPosition = 0; /* invalid */
                        status = PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_POWER_NEGOTIATION, NULL);
                        if (status != kStatus_PD_Success)
                        {
                            PRINTF("error\r\n");
                        }
                    }

                    pdAppInstance->retrySwapDelay = 5 * 1000; /* 5s */
                    pdAppInstance->demoState      = kDemoState_SwapSinkFail;
                    break;

                case kDemoState_SwapSinkFail:
                    if (pdAppInstance->batteryQuantity <= 5)
                    {
                        ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->drpTryFunction =
                            kTypecTry_Snk;
                        PD_DemoPDReset(pdAppInstance);
                        pdAppInstance->demoState = kDemoState_Start;
                    }
                    else
                    {
                        if (pdAppInstance->retrySwapDelay == 0)
                        {
                            PRINTF("try swap again\r\n");
                            pdAppInstance->demoState = kDemoState_NeedChangeAsSink;
                        }
                    }
                    break;

                case kDemoState_NoPower:
                    PD_DemoPDReset(pdAppInstance);
                    break;

                default:
                    break;
            }
        }
        else
        {
            if (pdAppInstance->batteryQuantity > 30)
            {
                pdAppInstance->prSwapAccept = 1u;
            }
            else
            {
                pdAppInstance->prSwapAccept = 0u;
            }

            switch (pdAppInstance->demoState)
            {
                case kDemoState_Idle:
                    if (pdAppInstance->batteryQuantity >= 100)
                    {
                        /* rquest low power */
                        if (pdAppInstance->sinkRequestVoltage == PD_DEMO_BATTERY_CHARGE_REQUEST_VOLTAGE)
                        {
                            pdAppInstance->retryCount = 2;
                            pdAppInstance->demoState  = kDemoState_RequestLowPower;
                        }
                    }
                    break;

                case kDemoState_RequestLowPower:
                    if (pdAppInstance->retryCount > 0)
                    {
                        /* default rdo as 5V - 0.9A */
                        pdAppInstance->sinkRequestVoltage                                = 5000;
                        pdAppInstance->sinkRequestRDO.bitFields.objectPosition           = 1;
                        pdAppInstance->sinkRequestRDO.bitFields.giveBack                 = 0;
                        pdAppInstance->sinkRequestRDO.bitFields.capabilityMismatch       = 0;
                        pdAppInstance->sinkRequestRDO.bitFields.usbCommunicationsCapable = 0;
                        pdAppInstance->sinkRequestRDO.bitFields.noUsbSuspend             = 1;
                        pdAppInstance->sinkRequestRDO.bitFields.operateValue             = 500 / PD_PDO_CURRENT_UNIT;
                        pdAppInstance->sinkRequestRDO.bitFields.maxOrMinOperateValue =
                            pdAppInstance->sinkRequestRDO.bitFields.operateValue;
                        /* get partner source cap */
                        pdAppInstance->commandWait   = 1;
                        pdAppInstance->commandResult = kCommandResult_Error;
                        status =
                            PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_REQUEST, &pdAppInstance->sinkRequestRDO);
                        if (status == kStatus_PD_Success)
                        {
                            pdAppInstance->demoState = kDemoState_WaitRequestLowPower;
                        }
                        else
                        {
                            PRINTF("error\r\n");
                        }
                    }
                    else
                    {
                        PRINTF("retry fail\r\n");
                        pdAppInstance->demoState = kDemoState_Idle;
                    }
                    break;

                case kDemoState_WaitRequestLowPower:
                    if (pdAppInstance->commandWait == 0)
                    {
                        if (pdAppInstance->commandResult == kCommandResult_Success)
                        {
                            pdAppInstance->demoState = kDemoState_Idle;
                        }
                        else
                        {
                            pdAppInstance->demoState = kDemoState_RequestLowPower; /* retry */
                        }
                    }
                    break;

                default:
                    break;
            }
        }
    }
}
