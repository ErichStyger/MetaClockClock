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
pd_status_t PD_DemoFindPDO(
    pd_app_t *pdAppInstance, pd_rdo_t *rdo, uint32_t requestVoltagemV, uint32_t requestCurrentmA, uint32_t *voltage);
void PD_DemoTaskFun(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

static void USB_PDDemoRequest5V(pd_app_t *pdAppInstance)
{
    pdAppInstance->sinkRequestVoltage                                = 5000U;
    pdAppInstance->sinkRequestRDO.bitFields.objectPosition           = 1U;
    pdAppInstance->sinkRequestRDO.bitFields.giveBack                 = 0U;
    pdAppInstance->sinkRequestRDO.bitFields.capabilityMismatch       = 0U;
    pdAppInstance->sinkRequestRDO.bitFields.usbCommunicationsCapable = 0U;
    pdAppInstance->sinkRequestRDO.bitFields.noUsbSuspend             = 1U;
    pdAppInstance->sinkRequestRDO.bitFields.operateValue             = 270U; /* 2.7A */
    pdAppInstance->sinkRequestRDO.bitFields.maxOrMinOperateValue     = 270U;
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

static void PD_DpmPrintCableInfo(pd_app_t *pdAppInstance)
{
    pd_cable_plug_info_t cablePlugInfo;

    PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_CABLE_INFO, &cablePlugInfo);
    if (cablePlugInfo.cableType == kCableType_Invalid)
    {
        PRINTF("no cable plug ");
        PRINTF("(only Vconn source can get the cable info, swap as vconn source and try again)\r\n");
    }
    else
    {
        uint8_t current;
        uint8_t voltage = 0xFFu;
        PRINTF("VDM ver: %d.0", (cablePlugInfo.vdmVersion + 1U));
        PRINTF(", cable type:");
        if (cablePlugInfo.cableType == kCableType_PassiveCable)
        {
            PRINTF("passive");
            current = cablePlugInfo.passiveCableVDOvdm20.bitFields.vbusCurrentHandlingCapability;
            if (cablePlugInfo.vdmVersion > 0U)
            {
                voltage = cablePlugInfo.passiveCableVDOvdm20.bitFields.maxVbusVoltage;
            }
        }
        else
        {
            PRINTF("active");
            current = cablePlugInfo.activeCableVDOvdm20.bitFields.vbusCurrentHandlingCapability;
            if (cablePlugInfo.vdmVersion > 0U)
            {
                voltage = cablePlugInfo.activeCableVDOvdm20.bitFields.maxVbusVoltage;
            }
        }

        if (current == 0x02u)
        {
            current = 5U; /* 5A */
        }
        else
        {
            current = 3U; /* 3A */
        }

        if (voltage != 0xFFu)
        {
            switch (voltage)
            {
                case 0x00u:
                    voltage = 20U;
                    break;
                case 0x01u:
                    voltage = 30U;
                    break;
                case 0x02u:
                    voltage = 40U;
                    break;
                case 0x03u:
                    voltage = 50U;
                    break;
                default:
                    voltage = 20U;
                    break;
            }
        }
        PRINTF(" cable,");
        PRINTF(" current:%dA", current);
        if (voltage != 0xFFu)
        {
            PRINTF(", max voltage:%dV\r\n", voltage);
        }
        else
        {
            PRINTF("\r\n");
        }
    }
}

static void USB_PDDemoPrintMenu(pd_app_t *pdAppInstance)
{
    uint8_t powerRole;
    uint8_t dataRole;

    PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &powerRole);
    PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_DATA_ROLE, &dataRole);

#if (defined PD_DEMO_PORTS_COUNT) && (PD_DEMO_PORTS_COUNT > 1)
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
        PRINTF("a. source power change\r\n");
        PRINTF("b. goto min\r\n");
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
        PRINTF("c. fast role swap\r\n");
#endif
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
    PRINTF("h. vconn swap\r\n");
    PRINTF("i. get partner sink capabilities\r\n");
    if (dataRole == kPD_DataRoleDFP)
    {
        PRINTF("j. standard structured VDM test (only DFP can send enter mode)\r\n");
        PRINTF("k. exit mode (only DFP)\r\n");
    }
    PRINTF("l. send attention\r\n");
    PRINTF("m. test vendor structured VDM\r\n");
    PRINTF("n. test unstructured VDM\r\n");
#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
    PRINTF("o. get source extended capabilities\r\n");
    PRINTF("p. get status\r\n");
    PRINTF("q. alert\r\n");
    PRINTF("r. get battery capabilities\r\n");
    PRINTF("s. get battery status\r\n");
    PRINTF("t. get manufacturer info\r\n");
#endif
    PRINTF("u. get cable info\r\n");
    PRINTF("v. cable reset  (not supported yet)\r\n");
#if (defined PD_CONFIG_PHY_LOW_POWER_LEVEL) && (PD_CONFIG_PHY_LOW_POWER_LEVEL)
    PRINTF("z. enable low power, soc enter low power automatically when PD is idle\r\n");
#endif
}

static void USB_PDDemoProcessMenu(pd_app_t *pdAppInstance, char ch)
{
    uint8_t powerRole;
    uint8_t dataRole;
    uint32_t commandValid;
    void *commandParam;
    pd_unstructured_vdm_command_param_t unstructuredVDMCommandParam;
#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
    pd_command_data_param_t extCommandParam;
#endif

#if (defined PD_CONFIG_PHY_LOW_POWER_LEVEL) && (PD_CONFIG_PHY_LOW_POWER_LEVEL)
    pdAppInstance->lowPowerEnable = 0;
#endif
    PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &powerRole);
    /* function codes */
    if ((ch >= '0') && (ch <= '9'))
    {
        commandValid = 0U;
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
        commandValid = 0U;

        switch (ch)
        {
            case 'a':
                if (powerRole == kPD_PowerRoleSource)
                {
                    PRINTF("a. source power change\r\n");
                    commandValid = PD_DPM_CONTROL_POWER_NEGOTIATION;
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
                if (powerRole == kPD_PowerRoleSource)
                {
                    PRINTF("b. goto min\r\n");
                    if (!(pdAppInstance->sinkRequestRDO.bitFields.giveBack))
                    {
                        PRINTF("warning: the GiveBack flag is not set\r\n");
                    }
                    commandValid = PD_DPM_CONTROL_GOTO_MIN;
                    commandParam = NULL;
                }
                else
                {
                    PRINTF("b. request 5V\r\n");
                    USB_PDDemoRequest5V(pdAppInstance);
                    commandValid = PD_DPM_CONTROL_REQUEST;
                    commandParam = &pdAppInstance->sinkRequestRDO;
                }
                break;

            case 'c':
                if (powerRole == kPD_PowerRoleSource)
                {
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
                    PRINTF("c. fast role swap\r\n");
                    commandValid = PD_DPM_FAST_ROLE_SWAP;
                    commandParam = NULL;
#endif
                }
                else
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
                PRINTF("warning: hard reset will occur if in alternate mode (menu 'i')\r\n");
                commandValid = PD_DPM_CONTROL_DR_SWAP;
                commandParam = NULL;
                break;

            case 'h':
                PRINTF("h. vconn swap\r\n");
                commandValid = PD_DPM_CONTROL_VCONN_SWAP;
                commandParam = NULL;
                break;

            case 'i':
                PRINTF("i. get partner sink capabilities\r\n");
                commandValid = PD_DPM_CONTROL_GET_PARTNER_SINK_CAPABILITIES;
                commandParam = NULL;
                break;

            case 'j':
                if (dataRole == kPD_DataRoleDFP)
                {
                    PRINTF("j. standard structured VDM test (only DFP can send enter mode)\r\n");
                    PRINTF("(1) discovery identity\r\n");
                    commandValid                                    = PD_DPM_CONTROL_DISCOVERY_IDENTITY;
                    pdAppInstance->structuredVDMCommandParam.vdmSop = pdAppInstance->msgSop;
                    commandParam                                    = &pdAppInstance->structuredVDMCommandParam;
                }
                break;

            case 'k':
                if (dataRole == kPD_DataRoleDFP)
                {
                    PRINTF("k. exit mode (only DFP)\r\n");
                    commandValid                                                        = PD_DPM_CONTROL_EXIT_MODE;
                    pdAppInstance->structuredVDMCommandParam.vdmSop                     = pdAppInstance->msgSop;
                    pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.objPos = 1U;
                    pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.SVID = pdAppInstance->partnerSVIDs[0];
                    commandParam = &pdAppInstance->structuredVDMCommandParam;
                }
                break;

            case 'l':
                PRINTF("l. send attention\r\n");
                commandValid                                                        = PD_DPM_CONTROL_SEND_ATTENTION;
                pdAppInstance->structuredVDMCommandParam.vdmSop                     = pdAppInstance->msgSop;
                pdAppInstance->structuredVDMCommandParam.vdoCount                   = 0U;
                pdAppInstance->structuredVDMCommandParam.vdoData                    = NULL;
                pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.objPos = 1;
                pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.SVID   = PD_VENDOR_VID;
                commandParam = &pdAppInstance->structuredVDMCommandParam;
                break;

            case 'm':
                PRINTF("m. test vendor structured VDM\r\n");
                commandValid                                                      = PD_DPM_SEND_VENDOR_STRUCTURED_VDM;
                pdAppInstance->structuredVDMCommandParam.vdmSop                   = pdAppInstance->msgSop;
                pdAppInstance->structuredVDMCommandParam.vdoCount                 = 0U;
                pdAppInstance->structuredVDMCommandParam.vdoData                  = NULL;
                pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.SVID = PD_VENDOR_VID;
                pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.command = 16U;
                pdAppInstance->structuredVDMCommandParam.vendorVDMNeedResponse       = 1U;
                commandParam = &pdAppInstance->structuredVDMCommandParam;
                break;

            case 'n':
            {
                PRINTF("n. test unstructured VDM\r\n");
                commandValid                                                  = PD_DPM_SEND_UNSTRUCTURED_VDM;
                pdAppInstance->unstructuredVDMCommandHeader.bitFields.SVID    = PD_VENDOR_VID;
                pdAppInstance->unstructuredVDMCommandHeader.bitFields.vdmType = 0U;
                unstructuredVDMCommandParam.vdmSop                            = pdAppInstance->msgSop;
                unstructuredVDMCommandParam.vdmHeaderAndVDOsCount             = 1U;
                unstructuredVDMCommandParam.vdmHeaderAndVDOsData =
                    (uint32_t *)&pdAppInstance->unstructuredVDMCommandHeader;
                commandParam = &unstructuredVDMCommandParam;
                break;
            }

#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
            case 'o':
            {
                PRINTF("o. get source extended capabilities\r\n");
                commandValid = PD_DPM_GET_SRC_EXT_CAP;
                commandParam = NULL;
                break;
            }

            case 'p':
            {
                PRINTF("p. get status\r\n");
                commandValid = PD_DPM_GET_STATUS;
                commandParam = NULL;
                break;
            }

            case 'q':
            {
                PRINTF("q. alert\r\n");
                commandValid                                   = PD_DPM_ALERT;
                pdAppInstance->selfAlert.bitFields.typeOfAlert = 0x02u; /* battery status change */
                commandParam                                   = &pdAppInstance->selfAlert;
                break;
            }

            case 'r':
            {
                PRINTF("r. get battery capabilities\r\n");
                commandValid                          = PD_DPM_GET_BATTERY_CAP;
                pdAppInstance->getBatteryCapDataBlock = 0x01u; /* get battery 1 cap */
                commandParam                          = &pdAppInstance->getBatteryCapDataBlock;
                break;
            }

            case 's':
            {
                PRINTF("s. get battery status\r\n");
                commandValid                          = PD_DPM_GET_BATTERY_STATUS;
                pdAppInstance->getBatteryCapDataBlock = 0x01u; /* get battery 1 cap */
                commandParam                          = &pdAppInstance->getBatteryCapDataBlock;
                break;
            }

            case 't':
            {
                PRINTF("t. get manufacturer info\r\n");
                pdAppInstance->commonData[0] = 1;
                pdAppInstance->commonData[1] = 0; /* battery zero */
                extCommandParam.dataBuffer   = &pdAppInstance->commonData[0];
                extCommandParam.dataLength   = 2;
                extCommandParam.sop          = pdAppInstance->msgSop;
                commandValid                 = PD_DPM_GET_MANUFACTURER_INFO;
                commandParam                 = &extCommandParam;
                break;
            }
#endif
            case 'u':
                PRINTF("u. get cable info\r\n");
                PD_DpmPrintCableInfo(pdAppInstance);
                break;

#if (defined PD_CONFIG_PHY_LOW_POWER_LEVEL) && (PD_CONFIG_PHY_LOW_POWER_LEVEL)
            case 'z':
                PRINTF("z. enable low power, soc enter low power automatically when PD is idle\r\n");
                pdAppInstance->lowPowerEnable = 1;
                break;
#endif

#if 0
        case 'v':
            PRINTF("u. cable reset\r\n");
            commandValid = PD_DPM_CONTROL_CABLE_RESET;
            commandParam = NULL;
            break;
#endif

            default:
                break;
        }

        if (commandValid)
        {
#if (defined PD_DEMO_PORTS_COUNT) && (PD_DEMO_PORTS_COUNT > 1)
            PRINTF("(tested port%d)\r\n", pdAppInstance->portNumber);
#endif
            if (PD_Command(pdAppInstance->pdHandle, commandValid, commandParam) != kStatus_PD_Success)
            {
                PRINTF("command fail\r\n");
            }
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
            else
            {
                if (commandValid == PD_DPM_FAST_ROLE_SWAP)
                {
                    PD_PowerBoardReset(pdAppInstance->portNumber);
                    PD_PowerSnkDrawTypeCVbus(&g_PDAppInstancePort1, kCurrent_StdUSB, kVbusPower_InFRSwap);
                }
            }
#endif
        }
    }
    else
    {
    }
}

void PD_DemoInit(void)
{
    volatile uint8_t discardValue;

    g_DemoGlobal.powerRequestSWState = kDEMO_SWIdle;
    g_DemoGlobal.powerChangeSWState  = kDEMO_SWIdle;
    g_DemoGlobal.processPort         = 0x00u;

    /* clear debug console input cache */
    DbgConsole_TryGetchar((char *)&discardValue);
    /* fix arm gcc warning */
    discardValue = discardValue;
}

#if (defined BOARD_PD_SW_INPUT_SUPPORT) && (BOARD_PD_SW_INPUT_SUPPORT)
void PD_Demo1msIsrProcessSW(pd_app_t *pdAppInstance)
{
    uint8_t pinState;

    if (g_DemoGlobal.powerRequestSWState == kDEMO_SWIsrTrigger)
    {
        /* 10ms as short press, 700ms as long press */
        HAL_GpioGetInput((hal_gpio_handle_t)(&pdAppInstance->gpioPowerRequestSwHandle[0]), &pinState);
        if (pinState == 0)
        {
            g_DemoGlobal.powerRequestSWTime++;
            if (g_DemoGlobal.powerRequestSWTime > 700)
            {
                g_DemoGlobal.powerRequestSWState = kDEMO_SWLongPress;
            }
        }
        else
        {
            if (g_DemoGlobal.powerRequestSWTime > 10)
            {
                g_DemoGlobal.powerRequestSWState = kDEMO_SWShortPress;
            }
            else
            {
                g_DemoGlobal.powerRequestSWState = kDEMO_SWIdle;
            }
        }
    }
    else if (g_DemoGlobal.powerRequestSWState == kDEMO_SWProcessed)
    {
        HAL_GpioGetInput((hal_gpio_handle_t)(&pdAppInstance->gpioPowerRequestSwHandle[0]), &pinState);
        if (pinState == 1)
        {
            g_DemoGlobal.powerRequestSWState = kDEMO_SWIdle;
        }
    }
    else if (g_DemoGlobal.powerRequestSWState == kDEMO_SWIdle)
    {
        HAL_GpioGetInput((hal_gpio_handle_t)(&pdAppInstance->gpioPowerRequestSwHandle[0]), &pinState);
        if (pinState == 0)
        {
            g_DemoGlobal.powerRequestSWTime  = 0u;
            g_DemoGlobal.powerRequestSWState = kDEMO_SWIsrTrigger;
        }
    }
    else
    {
    }

    if (g_DemoGlobal.powerChangeSWState == kDEMO_SWIsrTrigger)
    {
        /* 10ms as short press, 700ms as long press */
        HAL_GpioGetInput((hal_gpio_handle_t)(&pdAppInstance->gpioPowerChangeSwHandle[0]), &pinState);
        if (pinState == 0)
        {
            g_DemoGlobal.powerChangeSWTime++;
            if (g_DemoGlobal.powerChangeSWTime > 700)
            {
                g_DemoGlobal.powerChangeSWState = kDEMO_SWLongPress;
            }
        }
        else
        {
            if (g_DemoGlobal.powerChangeSWTime > 10)
            {
                g_DemoGlobal.powerChangeSWState = kDEMO_SWShortPress;
            }
            else
            {
                g_DemoGlobal.powerChangeSWState = kDEMO_SWIdle;
            }
        }
    }
    else if (g_DemoGlobal.powerChangeSWState == kDEMO_SWProcessed)
    {
        HAL_GpioGetInput((hal_gpio_handle_t)(&pdAppInstance->gpioPowerChangeSwHandle[0]), &pinState);
        if (pinState == 1)
        {
            g_DemoGlobal.powerChangeSWState = kDEMO_SWIdle;
        }
    }
    else if (g_DemoGlobal.powerChangeSWState == kDEMO_SWIdle)
    {
        HAL_GpioGetInput((hal_gpio_handle_t)(&pdAppInstance->gpioPowerChangeSwHandle[0]), &pinState);
        if (pinState == 0)
        {
            g_DemoGlobal.powerChangeSWTime  = 0u;
            g_DemoGlobal.powerChangeSWState = kDEMO_SWIsrTrigger;
        }
    }
    else
    {
    }
}

void USB_PDDemoProcessRequstSW(pd_app_t *pdAppInstance, uint8_t SWState)
{
    uint8_t powerRole;

    PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &powerRole);

    if (SWState == kDEMO_SWShortPress)
    {
        if (powerRole == kPD_PowerRoleSink)
        {
            PRINTF("request 5V\r\n");
            USB_PDDemoRequest5V(pdAppInstance);
            if (PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_REQUEST, &pdAppInstance->sinkRequestRDO) !=
                kStatus_PD_Success)
            {
                PRINTF("request call fail\r\n");
            }
        }
    }
    else if (SWState == kDEMO_SWLongPress)
    {
        if (powerRole == kPD_PowerRoleSink)
        {
            PRINTF("request high voltage\r\n");
            if (USB_PDDemoRequestHighVoltage(pdAppInstance))
            {
                if (PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_REQUEST, &pdAppInstance->sinkRequestRDO) !=
                    kStatus_PD_Success)
                {
                    PRINTF("request call fail\r\n");
                }
            }
            else
            {
                PRINTF("partner don't support");
            }
        }
    }
    else
    {
    }
}

void USB_PDDemoProcessPowerChangeSW(pd_app_t *pdAppInstance, uint8_t SWState)
{
    if (SWState == kDEMO_SWShortPress)
    {
        PRINTF("request power role swap\r\n");
        if (PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_PR_SWAP, NULL) != kStatus_PD_Success)
        {
            PRINTF("request call fail\r\n");
        }
    }
    else if (SWState == kDEMO_SWLongPress)
    {
        PRINTF("hard reset\r\n");
        if (PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_HARD_RESET, NULL) != kStatus_PD_Success)
        {
            PRINTF("request call fail\r\n");
        }
    }
    else
    {
    }
}
#endif

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
                    rdo->bitFields.operateValue   = (requestVoltagemV * requestCurrentmA) / 1000 / PD_PDO_POWER_UNIT;
                    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;
                    findSourceCap                       = 1U;
                }
                break;
            }

            default:
                break;
        }

        if (0U != findSourceCap)
        {
            break;
        }
    }

    if (0U != findSourceCap)
    {
        return kStatus_PD_Success;
    }
    return kStatus_PD_Error;
}

void PD_DemoTaskFun(void)
{
    pd_app_t *pdAppInstance = NULL;
    char uartData           = 0U;
    uint8_t connectState    = kTYPEC_ConnectNone;
    uint8_t hasEvent        = 0U;

    /* If input to select port */
    if (DbgConsole_TryGetchar(&uartData) == kStatus_Success)
    {
#if (defined PD_DEMO_PORTS_COUNT) && (PD_DEMO_PORTS_COUNT > 1)
        uint8_t testPort = 0;
        if (uartData == '1')
        {
#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE)
            testPort = 1;
#endif
        }
        else if (uartData == '2')
        {
#if (defined PD_DEMO_PORT2_ENABLE) && (PD_DEMO_PORT2_ENABLE)
            testPort = 2;
#endif
        }
        else if (uartData == '3')
        {
#if (defined PD_DEMO_PORT3_ENABLE) && (PD_DEMO_PORT3_ENABLE)
            testPort = 3;
#endif
        }
        else if (uartData == '4')
        {
#if (defined PD_DEMO_PORT4_ENABLE) && (PD_DEMO_PORT4_ENABLE)
            testPort = 4;
#endif
        }
        else
        {
        }

        if (testPort != 0)
        {
            uartData = 0;
            PRINTF("test port %d\r\n", testPort);
            g_DemoGlobal.processPort = testPort;
        }
#endif
    }

#if (defined PD_DEMO_PORTS_COUNT) && (PD_DEMO_PORTS_COUNT == 1)
#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE)
    g_DemoGlobal.processPort = 1U;
#endif
#if (defined PD_DEMO_PORT2_ENABLE) && (PD_DEMO_PORT2_ENABLE)
    g_DemoGlobal.processPort = 2U;
#endif
#if (defined PD_DEMO_PORT3_ENABLE) && (PD_DEMO_PORT3_ENABLE)
    g_DemoGlobal.processPort = 3U;
#endif
#if (defined PD_DEMO_PORT4_ENABLE) && (PD_DEMO_PORT4_ENABLE)
    g_DemoGlobal.processPort = 4U;
#endif
#endif

    /* there are valid SW or inputs for command test */
    hasEvent = 0;
#if (defined BOARD_PD_SW_INPUT_SUPPORT) && (BOARD_PD_SW_INPUT_SUPPORT)
    if ((g_DemoGlobal.powerRequestSWState == kDEMO_SWShortPress) ||
        (g_DemoGlobal.powerRequestSWState == kDEMO_SWLongPress))
    {
        hasEvent = 1U;
    }
    else if ((g_DemoGlobal.powerChangeSWState == kDEMO_SWShortPress) ||
             (g_DemoGlobal.powerChangeSWState == kDEMO_SWLongPress))
    {
        hasEvent = 1U;
    }
    else
    {
    }
#endif
    if (uartData != 0x00u)
    {
        hasEvent = 1U;
    }

/* if there is no port selected, remid user to select port. */
#if (defined PD_DEMO_PORTS_COUNT) && (PD_DEMO_PORTS_COUNT > 1)
    if (g_DemoGlobal.processPort == 0x00u)
    {
        if (hasEvent)
        {
            g_DemoGlobal.powerRequestSWState = kDEMO_SWProcessed;
            g_DemoGlobal.powerChangeSWState  = kDEMO_SWProcessed;
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

    /* process the SW or inputs command */
    if ((g_DemoGlobal.processPort) > 0 && (g_DemoGlobal.processPort < 5U))
    {
        uint8_t index;

        for (index = 0U; index < PD_DEMO_PORTS_COUNT; ++index)
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

    if (hasEvent)
    {
        /* get connect state */
        PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_TYPEC_CONNECT_STATE, &connectState);
        if (connectState == kTYPEC_ConnectNone)
        {
            PRINTF("port don't connect!!!\r\n");
        }
    }

#if (defined BOARD_PD_SW_INPUT_SUPPORT) && (BOARD_PD_SW_INPUT_SUPPORT)
    if ((g_DemoGlobal.powerRequestSWState == kDEMO_SWShortPress) ||
        (g_DemoGlobal.powerRequestSWState == kDEMO_SWLongPress))
    {
        USB_PDDemoProcessRequstSW(pdAppInstance, g_DemoGlobal.powerRequestSWState);
        g_DemoGlobal.powerRequestSWState = kDEMO_SWProcessed;
    }
    else if ((g_DemoGlobal.powerChangeSWState == kDEMO_SWShortPress) ||
             (g_DemoGlobal.powerChangeSWState == kDEMO_SWLongPress))
    {
        USB_PDDemoProcessPowerChangeSW(pdAppInstance, g_DemoGlobal.powerChangeSWState);
        g_DemoGlobal.powerChangeSWState = kDEMO_SWProcessed;
    }
    else
    {
    }
#endif

    if (uartData != 0U)
    {
        USB_PDDemoProcessMenu(pdAppInstance, uartData);
    }
}
