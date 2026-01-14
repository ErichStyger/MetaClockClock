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
void PD_DemoInit(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

static void USB_PDDemoRequest5V(pd_app_t *pdAppInstance)
{
    pdAppInstance->sinkRequestVoltage                                = 5000;
    pdAppInstance->sinkRequestRDO.bitFields.objectPosition           = 1;
    pdAppInstance->sinkRequestRDO.bitFields.giveBack                 = 0;
    pdAppInstance->sinkRequestRDO.bitFields.capabilityMismatch       = 0;
    pdAppInstance->sinkRequestRDO.bitFields.usbCommunicationsCapable = 0;
    pdAppInstance->sinkRequestRDO.bitFields.noUsbSuspend             = 1;
    pdAppInstance->sinkRequestRDO.bitFields.operateValue             = 270; /* 2.7A */
    pdAppInstance->sinkRequestRDO.bitFields.maxOrMinOperateValue     = 270;
}

void PD_DemoInit(void)
{
    volatile uint8_t discardValue;

    g_DemoGlobal.processPort = 0x00u;

    /* clear debug console input cache */
    DbgConsole_TryGetchar((char *)&discardValue);
    /* fix arm gcc warning */
    discardValue = discardValue;

    /* demo event */
    g_DemoGlobal.demoEvent = xEventGroupCreate();
    if (NULL == g_DemoGlobal.demoEvent)
    {
        PRINTF("create event fail\r\n");
    }
}

pd_status_t PD_DemoFindPDO(
    pd_app_t *pdAppInstance, pd_rdo_t *rdo, uint32_t requestVoltagemV, uint32_t requestCurrentmA, uint32_t *voltage)
{
    uint32_t index;
    pd_source_pdo_t sourcePDO;
    uint8_t findSourceCap = 0U;

    if (pdAppInstance->partnerSourceCapNumber == 0U)
    {
        return kStatus_PD_Error;
    }

    /* default rdo as 5V - 0.5A or less */
    *voltage                                = 5000U;
    rdo->bitFields.objectPosition           = 1U;
    rdo->bitFields.giveBack                 = 0U;
    rdo->bitFields.capabilityMismatch       = 0U;
    rdo->bitFields.usbCommunicationsCapable = 0U;
    rdo->bitFields.noUsbSuspend             = 1U;
    rdo->bitFields.operateValue             = 500U / PD_PDO_CURRENT_UNIT;
#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
    rdo->bitFields.unchunkedSupported = 1;
#endif
    if (rdo->bitFields.operateValue > pdAppInstance->partnerSourceCaps[0].fixedPDO.maxCurrent)
    {
        rdo->bitFields.operateValue = pdAppInstance->partnerSourceCaps[0].fixedPDO.maxCurrent;
    }
    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;

    for (index = 0U; index < pdAppInstance->partnerSourceCapNumber; ++index)
    {
        sourcePDO.PDOValue = pdAppInstance->partnerSourceCaps[index].PDOValue;
        switch (sourcePDO.commonPDO.pdoType)
        {
            case kPDO_Fixed:
            {
                if ((sourcePDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT == requestVoltagemV) &&
                    (sourcePDO.fixedPDO.maxCurrent * PD_PDO_CURRENT_UNIT >= requestCurrentmA))
                {
                    *voltage                            = sourcePDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT;
                    rdo->bitFields.objectPosition       = (index + 1U);
                    rdo->bitFields.operateValue         = requestCurrentmA / PD_PDO_CURRENT_UNIT;
                    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;
                    findSourceCap                       = 1U;
                }
                break;
            }

            case kPDO_Variable:
            {
                if ((sourcePDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT <= requestVoltagemV) &&
                    (sourcePDO.variablePDO.maxVoltage * PD_PDO_VOLTAGE_UNIT >= requestVoltagemV) &&
                    (sourcePDO.variablePDO.maxCurrent * PD_PDO_CURRENT_UNIT >= requestCurrentmA))
                {
                    *voltage                            = sourcePDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT;
                    rdo->bitFields.objectPosition       = (index + 1U);
                    rdo->bitFields.operateValue         = requestCurrentmA / PD_PDO_CURRENT_UNIT;
                    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;
                    findSourceCap                       = 1U;
                }
                break;
            }

            case kPDO_Battery:
            {
                if ((sourcePDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT <= requestVoltagemV) &&
                    (sourcePDO.batteryPDO.maxVoltage * PD_PDO_VOLTAGE_UNIT >= requestVoltagemV) &&
                    (sourcePDO.batteryPDO.maxAllowPower * PD_PDO_POWER_UNIT >=
                     (requestVoltagemV * requestCurrentmA / 1000U)))
                {
                    *voltage                      = sourcePDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT;
                    rdo->bitFields.objectPosition = (index + 1U);
                    rdo->bitFields.operateValue   = (requestVoltagemV * requestCurrentmA) / 1000U / PD_PDO_POWER_UNIT;
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

    if (findSourceCap)
    {
        return kStatus_PD_Success;
    }
    return kStatus_PD_Error;
}

static uint8_t USB_PDDemoRequestHighVoltage(pd_app_t *pdAppInstance)
{
    pd_rdo_t rdo;
    uint8_t snkCapIndex;
    uint32_t voltage;
    pd_sink_pdo_t pdo;

    for (snkCapIndex = (((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sinkCapCount - 1U);
         snkCapIndex > 0U; --snkCapIndex)
    {
        pdo.PDOValue = ((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sinkCaps[snkCapIndex];
        if (PD_DemoFindPDO(pdAppInstance, &rdo, pdo.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT, 0u, &voltage) ==
            kStatus_PD_Success)
        {
            break;
        }
    }

    if (snkCapIndex > 0U)
    {
        pdAppInstance->sinkRequestVoltage    = voltage;
        pdAppInstance->sinkRequestRDO.rdoVal = rdo.rdoVal;
        return 1U;
    }
    else
    {
        return 0U;
    }
}

static void USB_PDDemoPrintMenu(pd_app_t *pdAppInstance)
{
    uint8_t powerRole;
    uint8_t dataRole;

    PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &powerRole);
    PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_DATA_ROLE, &dataRole);

#if (defined PD_DEMO_PORTS_COUNT) && (PD_DEMO_PORTS_COUNT > 1U)
    PRINTF(
        "Please input\""
#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE)
        "1"
#if (PD_DEMO_PORT2_ENABLE) || (PD_DEMO_PORT3_ENABLE) || (PD_DEMO_PORT4_ENABLE)
        ","
#endif
#endif
#if (defined PD_DEMO_PORT2_ENABLE) && (PD_DEMO_PORT2_ENABLE)
        "2"
#if (PD_DEMO_PORT3_ENABLE) || (PD_DEMO_PORT4_ENABLE)
        ","
#endif
#endif
#if (defined PD_DEMO_PORT3_ENABLE) && (PD_DEMO_PORT3_ENABLE)
        "3"
#if (PD_DEMO_PORT4_ENABLE)
        ","
#endif
#endif
#if (defined PD_DEMO_PORT4_ENABLE) && (PD_DEMO_PORT4_ENABLE)
        "4"
#endif
        "\"to select the tested port if want to test another port at any time.\r\n");
#endif
    if (powerRole == kPD_PowerRoleSource)
    {
        /* source role */
        PRINTF("The menu is as follow for source:\r\n");
        PRINTF("0. print menu\r\n");
        PRINTF("a. get partner sink capabilities\r\n");
    }
    else
    {
        /* sink role */
        PRINTF("The menu is as follow for sink:\r\n");
        PRINTF("0. print menu\r\n");
        PRINTF("a. get partner source capabilities\r\n");
        PRINTF("b. request 5V\r\n");
        if (((pd_power_port_config_t *)pdAppInstance->pdConfigParam->portConfig)->sinkCapCount > 1)
        {
            PRINTF("c. request high voltage\r\n");
        }
    }

    PRINTF("d. hard reset\r\n");
    PRINTF("e. soft reset\r\n");
    PRINTF("f. power role swap\r\n");
    PRINTF("g. data role swap\r\n");
}

static void USB_PDDemoProcessMenu(pd_app_t *pdAppInstance, char ch)
{
    uint8_t powerRole;
    uint8_t dataRole;
    uint32_t commandValid;
    void *commandParam;

    PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &powerRole);
    /* function codes */
    if ((ch >= '0') && (ch <= '9'))
    {
        commandValid = 0;
        switch (ch)
        {
            case '0':
                USB_PDDemoPrintMenu(pdAppInstance);
                break;

            default:
                break;
        }
    }
    else if ((ch >= 'a') && (ch <= 'z'))
    {
        PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_DATA_ROLE, &dataRole);
        commandValid = 0;

        switch (ch)
        {
            case 'a':
                if (powerRole == kPD_PowerRoleSource)
                {
                    PRINTF("a. get partner sink capabilities\r\n");
                    commandValid = PD_DPM_CONTROL_GET_PARTNER_SINK_CAPABILITIES;
                    commandParam = NULL;
                }
                else
                {
                    PRINTF("a. get partner source capabilities\r\n");
                    commandValid = PD_DPM_CONTROL_GET_PARTNER_SOURCE_CAPABILITIES;
                    commandParam = NULL;
                }
                break;

            case 'b':
                if (powerRole == kPD_PowerRoleSink)
                {
                    PRINTF("b. request 5V\r\n");
                    USB_PDDemoRequest5V(pdAppInstance);
                    commandValid = PD_DPM_CONTROL_REQUEST;
                    commandParam = &pdAppInstance->sinkRequestRDO;
                }
                break;

            case 'c':
                if (powerRole == kPD_PowerRoleSink)
                {
                    PRINTF("c. request high voltage\r\n");
                    if (USB_PDDemoRequestHighVoltage(pdAppInstance))
                    {
                        commandValid = PD_DPM_CONTROL_REQUEST;
                        commandParam = &pdAppInstance->sinkRequestRDO;
                    }
                    else
                    {
                        PRINTF("partner don't support");
                    }
                }
                break;

            case 'd':
                PRINTF("d. hard reset\r\n");
                commandValid = PD_DPM_CONTROL_HARD_RESET;
                commandParam = NULL;
                break;

            case 'e':
                PRINTF("e. soft reset\r\n");
                commandValid = PD_DPM_CONTROL_SOFT_RESET;
                commandParam = &pdAppInstance->msgSop;
                break;

            case 'f':
                PRINTF("f. power role swap\r\n");
                commandValid = PD_DPM_CONTROL_PR_SWAP;
                commandParam = NULL;
                break;

            case 'g':
                PRINTF("g. data role swap\r\n");
                commandValid = PD_DPM_CONTROL_DR_SWAP;
                commandParam = NULL;
                break;

            default:
                break;
        }

        if (commandValid)
        {
#if (defined PD_DEMO_PORTS_COUNT) && (PD_DEMO_PORTS_COUNT > 1U)
            PRINTF("(tested port%d)\r\n", pdAppInstance->portNumber);
#endif
            if (PD_Command(pdAppInstance->pdHandle, commandValid, commandParam) != kStatus_PD_Success)
            {
                PRINTF("command fail\r\n");
            }
        }
    }
    else
    {
    }
}

void pd_DemoConsoleReadFun(void *arg)
{
    while (1)
    {
        g_DemoGlobal.consoleInputChar = GETCHAR();
        xEventGroupSetBits(g_DemoGlobal.demoEvent, PD_DEMO_INPUT_EVENT_BIT);
    }
}

void PD_DemoTaskFun(void)
{
    pd_app_t *pdAppInstance = NULL;
    uint32_t eventBits;
    uint8_t connectState = kTYPEC_ConnectNone;

    eventBits = xEventGroupWaitBits(g_DemoGlobal.demoEvent, 0xFFu, 1, 0, portMAX_DELAY);

/* If input to select port */
#if (defined PD_DEMO_PORTS_COUNT) && (PD_DEMO_PORTS_COUNT > 1U)
    if (eventBits & PD_DEMO_INPUT_EVENT_BIT)
    {
        uint8_t testPort = 0U;
        if (g_DemoGlobal.consoleInputChar == '1')
        {
#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE)
            testPort = 1U;
#endif
        }
        else if (g_DemoGlobal.consoleInputChar == '2')
        {
#if (defined PD_DEMO_PORT2_ENABLE) && (PD_DEMO_PORT2_ENABLE)
            testPort = 2U;
#endif
        }
        else if (g_DemoGlobal.consoleInputChar == '3')
        {
#if (defined PD_DEMO_PORT3_ENABLE) && (PD_DEMO_PORT3_ENABLE)
            testPort = 3U;
#endif
        }
        else if (g_DemoGlobal.consoleInputChar == '4')
        {
#if (defined PD_DEMO_PORT4_ENABLE) && (PD_DEMO_PORT4_ENABLE)
            testPort = 4U;
#endif
        }
        else
        {
        }

        if (testPort != 0U)
        {
            PRINTF("test port %d\r\n", testPort);
            g_DemoGlobal.processPort = testPort;
        }
    }
#else

#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE)
    g_DemoGlobal.processPort = 1;
#endif
#if (defined PD_DEMO_PORT2_ENABLE) && (PD_DEMO_PORT2_ENABLE)
    g_DemoGlobal.processPort = 2;
#endif
#if (defined PD_DEMO_PORT3_ENABLE) && (PD_DEMO_PORT3_ENABLE)
    g_DemoGlobal.processPort = 3;
#endif
#if (defined PD_DEMO_PORT4_ENABLE) && (PD_DEMO_PORT4_ENABLE)
    g_DemoGlobal.processPort = 4;
#endif
#endif

/* if there is no port selected, remid user to select port. */
#if (defined PD_DEMO_PORTS_COUNT) && (PD_DEMO_PORTS_COUNT > 1)
    if (g_DemoGlobal.processPort == 0x00u)
    {
        if (eventBits & PD_DEMO_INPUT_EVENT_BIT)
        {
            g_DemoGlobal.consoleInputChar = 0x00u;

            PRINTF(
                "please set the tested port by input number \""
#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE)
                "1"
#if (PD_DEMO_PORT2_ENABLE) || (PD_DEMO_PORT3_ENABLE) || (PD_DEMO_PORT4_ENABLE)
                ","
#endif
#endif
#if (defined PD_DEMO_PORT2_ENABLE) && (PD_DEMO_PORT2_ENABLE)
                "2"
#if (PD_DEMO_PORT3_ENABLE) || (PD_DEMO_PORT4_ENABLE)
                ","
#endif
#endif
#if (defined PD_DEMO_PORT3_ENABLE) && (PD_DEMO_PORT3_ENABLE)
                "3"
#if (PD_DEMO_PORT4_ENABLE)
                ","
#endif
#endif
#if (defined PD_DEMO_PORT4_ENABLE) && (PD_DEMO_PORT4_ENABLE)
                "4"
#endif
                "\"\r\n");
        }
        return;
    }
#endif

    /* process inputs command */
    if ((g_DemoGlobal.processPort) > 0 && (g_DemoGlobal.processPort < 5))
    {
        for (uint8_t index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
        {
            if (g_PDAppInstanceArray[index]->portNumber == g_DemoGlobal.processPort)
            {
                pdAppInstance = g_PDAppInstanceArray[index];
                break;
            }
        }
    }
    if ((pdAppInstance == NULL) || (pdAppInstance->pdHandle == NULL))
    {
        return;
    }

    if (eventBits & PD_DEMO_INPUT_EVENT_BIT)
    {
        /* get connect state */
        PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_TYPEC_CONNECT_STATE, &connectState);
        if (connectState == kTYPEC_ConnectNone)
        {
            PRINTF("port don't connect!!!\r\n");
        }
        USB_PDDemoProcessMenu(pdAppInstance, g_DemoGlobal.consoleInputChar);
    }
}
