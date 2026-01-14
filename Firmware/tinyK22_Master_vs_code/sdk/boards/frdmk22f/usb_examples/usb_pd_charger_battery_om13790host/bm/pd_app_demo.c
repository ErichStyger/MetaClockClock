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
void PD_AppPortInit(pd_app_t *arg);
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
        if (pdAppInstance->batteryQuantity > 5U)
        {
            pdAppInstance->batteryQuantity--;
        }

        /* battery consume faster */
        if (pdAppInstance->partnerRequestRDO.bitFields.objectPosition == 2U)
        {
            if (pdAppInstance->batteryQuantity > 5U)
            {
                pdAppInstance->batteryQuantity--;
            }
        }
    }
    else
    {
        if (pdAppInstance->batteryQuantity < 100U)
        {
            pdAppInstance->batteryQuantity++;
        }

        /* battery charge faster */
        if (pdAppInstance->sinkRequestVoltage == PD_DEMO_BATTERY_CHARGE_REQUEST_VOLTAGE)
        {
            if (pdAppInstance->batteryQuantity < 100U)
            {
                pdAppInstance->batteryQuantity++;
            }
        }
    }
}

void PD_DemoReset(pd_app_t *pdAppInstance)
{
    if (pdAppInstance->batteryQuantity > 30U)
    {
        ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount = 2U;
    }
    else
    {
        ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount = 1U;
    }
    pdAppInstance->partnerRequestRDO.bitFields.objectPosition = 0U; /* invalid */
    pdAppInstance->contractValid                              = 0U;
    pdAppInstance->demoState                                  = kDemoState_Start;
    pdAppInstance->partnerSourceCapNumber                     = 0U;
}

void PD_DemoInit(void)
{
    uint8_t index;

    for (index = 0U; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        g_PDAppInstanceArray[index]->batteryQuantity = PD_DemoStartUpGetBatteryValue(g_PDAppInstanceArray[index]);
        g_PDAppInstanceArray[index]->batteryChange   = 0U;
        PD_DemoReset(g_PDAppInstanceArray[index]);
    }

    return;
}

void PD_Demo1msIsrProcess(void)
{
    uint8_t index;
    static volatile uint32_t delay = 0U;
    delay++;

    /* 1.2 s */
    if (delay >= 1200U)
    {
        delay = 0U;
        for (index = 0U; index < PD_DEMO_PORTS_COUNT; ++index)
        {
            if (!g_PDAppInstanceArray[index]->contractValid)
            {
                continue;
            }
            if (g_PDAppInstanceArray[index]->pdHandle != NULL)
            {
                PD_DemoBatteryChange(g_PDAppInstanceArray[index]);
                g_PDAppInstanceArray[index]->batteryChange = 1U;

                if (g_PDAppInstanceArray[index]->retrySwapDelay > 0U)
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
    uint8_t findSourceCap = 0U;

    if (pdAppInstance->partnerSourceCapNumber == 0U)
    {
        return kStatus_PD_Error;
    }

    /* default rdo as 5V - 0.9A */
    *voltage                                = 5000U;
    rdo->bitFields.objectPosition           = 1U;
    rdo->bitFields.giveBack                 = 0U;
    rdo->bitFields.capabilityMismatch       = 0U;
    rdo->bitFields.usbCommunicationsCapable = 0U;
    rdo->bitFields.noUsbSuspend             = 1U;
    rdo->bitFields.operateValue             = 500U / PD_PDO_CURRENT_UNIT;
    rdo->bitFields.maxOrMinOperateValue     = rdo->bitFields.operateValue;
    if (pdAppInstance->batteryQuantity < 100U)
    {
        requestVoltage = PD_DEMO_BATTERY_CHARGE_REQUEST_VOLTAGE;
        requestCurrent = PD_DEMO_BATTERY_CHARGE_REQUEST_CURRENT;
    }
    else
    {
        requestVoltage = PD_DEMO_BATTERY_FULL_REQUEST_VOLTAGE;
        requestCurrent = PD_DEMO_BATTERY_FULL_REQUEST_CURRENT;
    }

    for (index = 0U; index < pdAppInstance->partnerSourceCapNumber; ++index)
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
                    rdo->bitFields.objectPosition       = (index + 1U);
                    rdo->bitFields.operateValue         = requestCurrent / PD_PDO_CURRENT_UNIT;
                    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;
                    findSourceCap                       = 1U;
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
                    rdo->bitFields.objectPosition       = (index + 1U);
                    rdo->bitFields.operateValue         = requestCurrent / PD_PDO_CURRENT_UNIT;
                    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;
                    findSourceCap                       = 1U;
                }
                break;
            }

            case kPDO_Battery:
            {
                if ((sourcePDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT <= requestVoltage) &&
                    (sourcePDO.batteryPDO.maxVoltage * PD_PDO_VOLTAGE_UNIT >= requestVoltage) &&
                    (sourcePDO.batteryPDO.maxAllowPower * PD_PDO_POWER_UNIT >=
                     (requestVoltage * requestCurrent / 1000U)))
                {
                    *voltage                            = sourcePDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT;
                    rdo->bitFields.objectPosition       = (index + 1U);
                    rdo->bitFields.operateValue         = (requestVoltage * requestCurrent) / 1000U / PD_PDO_POWER_UNIT;
                    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;
                    findSourceCap                       = 1U;
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
    uint32_t sr;

    PD_PowerSrcTurnOffVbus(pdAppInstance, kVbusPower_Stable);
    sr                                    = DisableGlobalIRQ();
    pdAppInstance->partnerSourceCapNumber = 0U;
    pdAppInstance->contractValid          = 0U;
    pdAppInstance->demoState              = kDemoState_Start;
    pdTmpHandle                           = pdAppInstance->pdHandle;
    pdAppInstance->pdHandle               = NULL;
    PD_InstanceDeinit(pdTmpHandle);
    PD_AppPortInit(pdAppInstance);
    EnableGlobalIRQ(sr);
}

void PD_DemoTaskFun()
{
    pd_app_t *pdAppInstance;
    pd_status_t status = kStatus_PD_Error;
    uint8_t index;

    for (index = 0U; index < PD_DEMO_PORTS_COUNT; ++index)
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

        if (pdAppInstance->batteryChange == 1U)
        {
            uint8_t chargeVoltage;
            pdAppInstance->batteryChange = 0U;

            if (pdAppInstance->selfPowerRole == kPD_PowerRoleSource)
            {
                chargeVoltage = (pdAppInstance->partnerRequestRDO.bitFields.objectPosition == 2U ? 9U : 5U);
                if (pdAppInstance->batteryQuantity <= 5U)
                {
                    chargeVoltage = 0U;
                }
            }
            else
            {
                chargeVoltage = ((pdAppInstance->sinkRequestVoltage == PD_DEMO_BATTERY_FULL_REQUEST_VOLTAGE) ? 5U : 9U);
            }
            PRINTF("port %d battery percent:%d, charge voltage:%dV\r\n", pdAppInstance->portNumber,
                   (uint16_t)pdAppInstance->batteryQuantity, chargeVoltage);
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

        if (pdAppInstance->batteryQuantity > 30U)
        {
            if (((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount == 1U)
            {
                ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount = 2U;
                PRINTF("change source caps as high power(5V/9V)\r\n");
            }
        }
        else
        {
            if (((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount == 2U)
            {
                ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount = 1U;
            }
        }

        /* reference to readme */
        if (pdAppInstance->runningPowerRole == kPD_PowerRoleSource)
        {
            pdAppInstance->prSwapAccept = 1U;

            switch (pdAppInstance->demoState)
            {
                case kDemoState_Idle:
                    if (pdAppInstance->batteryQuantity > 5U)
                    {
                        ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->drpTryFunction =
                            kTypecTry_None;
                    }

                    if ((pdAppInstance->batteryQuantity <= 30U) &&
                        (pdAppInstance->partnerRequestRDO.bitFields.objectPosition == 2U))
                    {
                        ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount = 1U;
                        PRINTF("change source caps as low power(5V)\r\n");
                        pdAppInstance->partnerRequestRDO.bitFields.objectPosition = 0U; /* invalid */
                        status = PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_POWER_NEGOTIATION, NULL);
                        if (status != kStatus_PD_Success)
                        {
                            PRINTF("error\r\n");
                        }
                    }
                    else if (pdAppInstance->batteryQuantity <= 20U)
                    {
                        pdAppInstance->demoState = kDemoState_NeedChangeAsSink;
                    }
                    else
                    {
                    }
                    break;

                case kDemoState_TryChangeAsSink:
                    PRINTF("try to swap as sink\r\n");
                    pdAppInstance->trySwap    = 1U;
                    pdAppInstance->retryCount = 2U;
                    pdAppInstance->demoState  = kDemoState_GetPartnerSrcCap;
                    break;

                case kDemoState_NeedChangeAsSink:
                    PRINTF("need change as sink because battery is low\r\n");
                    pdAppInstance->trySwap    = 0U;
                    pdAppInstance->retryCount = 2U;
                    if (pdAppInstance->partnerSourceCapNumber > 0U)
                    {
                        pdAppInstance->retryCount = 2U;
                        pdAppInstance->demoState  = kDemoState_SwapAsSink;
                    }
                    else
                    {
                        pdAppInstance->demoState = kDemoState_GetPartnerSrcCap;
                    }
                    break;

                case kDemoState_GetPartnerSrcCap:
                    if (pdAppInstance->retryCount > 0U)
                    {
                        PRINTF("get partner source caps\r\n");
                        /* get partner source cap */
                        pdAppInstance->commandWait   = 1U;
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
                    if (pdAppInstance->commandWait == 0U)
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
                                        pdAppInstance->retryCount = 2U;
                                        pdAppInstance->demoState  = kDemoState_SwapAsSink;
                                    }
                                    else
                                    {
                                        pdAppInstance->demoState = kDemoState_Idle;
                                    }
                                }
                                else
                                {
                                    pdAppInstance->retryCount = 2U;
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
                    if (pdAppInstance->retryCount > 0U)
                    {
                        PRINTF("try pr swap\r\n");
                        pdAppInstance->commandWait   = 1U;
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
                    if (pdAppInstance->commandWait == 0U)
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
                    if (pdAppInstance->partnerRequestRDO.bitFields.objectPosition != 1U)
                    {
                        PRINTF("source reduce power\r\n");
                        ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sourceCapCount = 1U;
                        PRINTF("change source caps as low power(5V)\r\n");
                        pdAppInstance->partnerRequestRDO.bitFields.objectPosition = 0; /* invalid */
                        status = PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_POWER_NEGOTIATION, NULL);
                        if (status != kStatus_PD_Success)
                        {
                            PRINTF("error\r\n");
                        }
                    }

                    pdAppInstance->retrySwapDelay = 5U * 1000U; /* 5s */
                    pdAppInstance->demoState      = kDemoState_SwapSinkFail;
                    break;

                case kDemoState_SwapSinkFail:
                    if (pdAppInstance->batteryQuantity <= 5U)
                    {
                        ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->drpTryFunction =
                            kTypecTry_Snk;
                        PD_DemoPDReset(pdAppInstance);
                        pdAppInstance->demoState = kDemoState_Start;
                    }
                    else
                    {
                        if (pdAppInstance->retrySwapDelay == 0U)
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
            if (pdAppInstance->batteryQuantity > 30U)
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
                    if (pdAppInstance->batteryQuantity >= 100U)
                    {
                        /* rquest low power */
                        if (pdAppInstance->sinkRequestVoltage == PD_DEMO_BATTERY_CHARGE_REQUEST_VOLTAGE)
                        {
                            pdAppInstance->retryCount = 2U;
                            pdAppInstance->demoState  = kDemoState_RequestLowPower;
                        }
                    }
                    break;

                case kDemoState_RequestLowPower:
                    if (pdAppInstance->retryCount > 0U)
                    {
                        /* default rdo as 5V - 0.9A */
                        pdAppInstance->sinkRequestVoltage                                = 5000U;
                        pdAppInstance->sinkRequestRDO.bitFields.objectPosition           = 1U;
                        pdAppInstance->sinkRequestRDO.bitFields.giveBack                 = 0U;
                        pdAppInstance->sinkRequestRDO.bitFields.capabilityMismatch       = 0U;
                        pdAppInstance->sinkRequestRDO.bitFields.usbCommunicationsCapable = 0U;
                        pdAppInstance->sinkRequestRDO.bitFields.noUsbSuspend             = 1U;
                        pdAppInstance->sinkRequestRDO.bitFields.operateValue             = 500U / PD_PDO_CURRENT_UNIT;
                        pdAppInstance->sinkRequestRDO.bitFields.maxOrMinOperateValue =
                            pdAppInstance->sinkRequestRDO.bitFields.operateValue;
                        /* get partner source cap */
                        pdAppInstance->commandWait   = 1U;
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
                    if (pdAppInstance->commandWait == 0U)
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
