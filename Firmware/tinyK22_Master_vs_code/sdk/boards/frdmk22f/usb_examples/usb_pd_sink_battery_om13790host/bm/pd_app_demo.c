/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdarg.h>
#include "usb_pd_config.h"
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
pd_status_t PD_DemoFindPDO(
    pd_app_t *pdAppInstance, pd_rdo_t *rdo, uint32_t requestVoltagemV, uint32_t requestCurrentmA, uint32_t *voltage);
void PD_DemoReset(pd_app_t *pdAppInstance);
void PD_DemoInit(void);
void PD_DemoTaskFun(pd_app_t *pdAppInstance);
void PD_Demo1msIsrProcess(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

static uint32_t PD_DemoStartUpGetBatteryValue(pd_app_t *pdAppInstance)
{
    return 10; /* default start up battery is 10% */
}

static void PD_DemoBatteryChange(pd_app_t *pdAppInstance)
{
    if (pdAppInstance->contractValid)
    {
        if (pdAppInstance->batteryQuantity < 100)
        {
            pdAppInstance->batteryQuantity++;
        }

        /* battery charge faster */
        if (pdAppInstance->sinkRequestVoltage > 5000)
        {
            if (pdAppInstance->batteryQuantity < 100)
            {
                pdAppInstance->batteryQuantity++;
            }
        }
    }
    else
    {
        if (pdAppInstance->batteryQuantity > 0)
        {
            pdAppInstance->batteryQuantity--;
        }
    }
}

void PD_DemoReset(pd_app_t *pdAppInstance)
{
    pdAppInstance->batteryChange = 0;
    pdAppInstance->contractValid = 0;
}

void PD_DemoInit(void)
{
    uint8_t index;

    for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        if (g_PDAppInstanceArray[index] != NULL)
        {
            g_PDAppInstanceArray[index]->batteryQuantity = PD_DemoStartUpGetBatteryValue(g_PDAppInstanceArray[index]);
            PD_DemoReset(g_PDAppInstanceArray[index]);
        }
    }
}

void PD_Demo1msIsrProcess(void)
{
    uint8_t index;
    static volatile uint32_t delay = 0;
    delay++;

    /* 2 s */
    if (delay >= 2000)
    {
        delay = 0;
        for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
        {
            if (g_PDAppInstanceArray[index]->pdHandle != NULL)
            {
                PD_DemoBatteryChange(g_PDAppInstanceArray[index]);
                g_PDAppInstanceArray[index]->batteryChange = 1;
            }
        }
    }
}

pd_status_t PD_DemoFindPDO(
    pd_app_t *pdAppInstance, pd_rdo_t *rdo, uint32_t requestVoltagemV, uint32_t requestCurrentmA, uint32_t *voltage)
{
    uint32_t index;
    pd_source_pdo_t sourcePDO;
    uint8_t findSourceCap = 0;

    if (pdAppInstance->partnerSourceCapNumber == 0)
    {
        return kStatus_PD_Error;
    }

    /* default rdo as 5V - 0.5A or less */
    *voltage                                = 5000;
    rdo->bitFields.objectPosition           = 1;
    rdo->bitFields.giveBack                 = 0;
    rdo->bitFields.capabilityMismatch       = 0;
    rdo->bitFields.usbCommunicationsCapable = 0;
    rdo->bitFields.noUsbSuspend             = 1;
    rdo->bitFields.operateValue             = 500 / PD_PDO_CURRENT_UNIT;
    if (rdo->bitFields.operateValue > pdAppInstance->partnerSourceCaps[0].fixedPDO.maxCurrent)
    {
        rdo->bitFields.operateValue = pdAppInstance->partnerSourceCaps[0].fixedPDO.maxCurrent;
    }
    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;

    for (index = 0; index < pdAppInstance->partnerSourceCapNumber; ++index)
    {
        sourcePDO.PDOValue = pdAppInstance->partnerSourceCaps[index].PDOValue;
        switch (sourcePDO.commonPDO.pdoType)
        {
            case kPDO_Fixed:
            {
                if ((sourcePDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT == requestVoltagemV) &&
                    (sourcePDO.fixedPDO.maxCurrent * PD_PDO_CURRENT_UNIT >= requestCurrentmA))
                {
                    *voltage                      = sourcePDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT;
                    rdo->bitFields.objectPosition = (index + 1);
                    if (requestCurrentmA == 0u)
                    {
                        rdo->bitFields.operateValue = sourcePDO.fixedPDO.maxCurrent;
                    }
                    else
                    {
                        rdo->bitFields.operateValue = requestCurrentmA / PD_PDO_CURRENT_UNIT;
                    }
                    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;
                    findSourceCap                       = 1;
                }
                break;
            }

            case kPDO_Variable:
            {
                if ((sourcePDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT <= requestVoltagemV) &&
                    (sourcePDO.variablePDO.maxVoltage * PD_PDO_VOLTAGE_UNIT >= requestVoltagemV) &&
                    (sourcePDO.variablePDO.maxCurrent * PD_PDO_CURRENT_UNIT >= requestCurrentmA))
                {
                    *voltage                      = sourcePDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT;
                    rdo->bitFields.objectPosition = (index + 1);
                    if (requestCurrentmA == 0u)
                    {
                        rdo->bitFields.operateValue = sourcePDO.variablePDO.maxCurrent;
                    }
                    else
                    {
                        rdo->bitFields.operateValue = requestCurrentmA / PD_PDO_CURRENT_UNIT;
                    }
                    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;
                    findSourceCap                       = 1;
                }
                break;
            }

            case kPDO_Battery:
            {
                if ((sourcePDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT <= requestVoltagemV) &&
                    (sourcePDO.batteryPDO.maxVoltage * PD_PDO_VOLTAGE_UNIT >= requestVoltagemV) &&
                    (sourcePDO.batteryPDO.maxAllowPower * PD_PDO_POWER_UNIT >=
                     (requestVoltagemV * requestCurrentmA / 1000)))
                {
                    *voltage                      = sourcePDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT;
                    rdo->bitFields.objectPosition = (index + 1);
                    if (requestCurrentmA == 0u)
                    {
                        rdo->bitFields.operateValue = sourcePDO.batteryPDO.maxAllowPower;
                    }
                    else
                    {
                        rdo->bitFields.operateValue = (requestVoltagemV * requestCurrentmA) / 1000 / PD_PDO_POWER_UNIT;
                    }
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

    if (findSourceCap)
    {
        return kStatus_PD_Success;
    }
    return kStatus_PD_Error;
}

void PD_DemoTaskFun(pd_app_t *pdAppInstance)
{
    uint8_t index;

    for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        if (g_PDAppInstanceArray[index]->pdHandle != NULL)
        {
            if (g_PDAppInstanceArray[index]->batteryChange == 1)
            {
                g_PDAppInstanceArray[index]->batteryChange = 0;

                PRINTF("port %d battery percent:%d, charge voltage:%dV\r\n", g_PDAppInstanceArray[index]->portNumber,
                       g_PDAppInstanceArray[index]->batteryQuantity,
                       (g_PDAppInstanceArray[index]->contractValid) ?
                           (uint16_t)(g_PDAppInstanceArray[index]->sinkRequestVoltage / 1000) :
                           0u);
            }
        }
    }
}
