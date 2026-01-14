/*
 * Copyright 2016 - 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
/*${standard_header_anchor}*/
#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_i2c.h"
#include "pd_app.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pd_power_interface.h"
#include "board.h"
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
#include "usb_pd_auto_policy.h"
#endif
#include "pd_board_config.h"
#include "usb_pd_alt_mode.h"
#include "usb_pd_alt_mode_dp.h"
#include "pd_crossbar_cbtl.h"
#include "pd_dp_hpd_driver.h"
#include "pd_ptn36502.h"
#include "pd_dp_board_chip.h"
#include "pd_power_nx20p3483.h"
#include "fsl_adapter_timer.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if (PD_DEMO_PORTS_COUNT > PD_CONFIG_MAX_PORT)
#error "please increase the instance count"
#endif

/* The follow MACROs are for passing compliance test, it is not actual product logical */
#define PD_SOURCE_POWER (18U) /* W */
#ifndef PD_TIMER_INSTANCE
#define PD_TIMER_INSTANCE (0U)
#endif

#if ((!defined(__DSC__)) || (!defined(__CW__)))
#define SYSTICK_CTRL_REG          (*((volatile uint32_t *)0xe000e010U))
#define SYSTICK_LOAD_REG          (*((volatile uint32_t *)0xe000e014U))
#define SYSTICK_CURRENT_VALUE_REG (*((volatile uint32_t *)0xe000e018U))
#define SYSTICK_ENABLE_BIT        (1UL << 0UL)
#define SYSTICK_CLK_BIT           (1UL << 2UL)
#define SYSTICK_VALUE_MASK        (0x00FFFFFFu)
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void PD_BMEnterCritical(uint32_t *sr);
void PD_BMExitCritical(uint32_t sr);
uint32_t PD_DemoSoftTimer_msGet(void);
uint32_t PD_DemoSoftTimer_getInterval(uint32_t startTime);
void PD_WaitUs(uint32_t us);
uint32_t HW_I2CGetFreq(uint8_t i2cInstance);
void HW_I2CReleaseBus(void);
void PD_DemoInit(void);
void PD_DemoTaskFun(void);
void BOARD_InitHardware(void);
uint32_t HW_TimerGetFreq(void);
void HW_TimerCallback(void *callbackParam);
pd_status_t PD_DpmAppCommandCallback(void *callbackParam, uint32_t event, void *param);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static pd_source_pdo_t s_PortSourceCaps[] = {
    {
        /* PDO1: fixed supply: dual-role power; Externally Powered; no USB communication; dual-role data; 5V; 3A */
        .fixedPDO.dualRoleData    = 1U,
        .fixedPDO.dualRolePower   = 1U,
        .fixedPDO.externalPowered = 1U,
        .fixedPDO.fixedSupply     = kPDO_Fixed,
        .fixedPDO.maxCurrent      = (3U * 100U),
        .fixedPDO.peakCurrent     = 0U,
#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
        .fixedPDO.unchunkedSupported = 1U,
#endif
        .fixedPDO.usbCommunicationsCapable = 0U,
        .fixedPDO.usbSuspendSupported      = 0U,
        .fixedPDO.voltage                  = (5U * 1000U / 50U),
    },
    {
        /* PDO2: fixed Supply: 9V - 2A */
        .fixedPDO.fixedSupply = kPDO_Fixed,
        .fixedPDO.maxCurrent  = (2U * 100U),
        .fixedPDO.voltage     = (9U * 1000U / 50U),
    },
};

static pd_sink_pdo_t s_PortSinkcaps[] = {{
                                             /* PDO1: fixed:5.0V, 3A */
                                             .fixedPDO.fixedSupply     = kPDO_Fixed,
                                             .fixedPDO.dualRoleData    = 1U,
                                             .fixedPDO.dualRolePower   = 1U,
                                             .fixedPDO.externalPowered = 1U,
#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
#if (defined PD_CONFIG_COMPLIANCE_TEST_ENABLE) && (PD_CONFIG_COMPLIANCE_TEST_ENABLE)
                                             .fixedPDO.frSwapRequiredCurrent = 0U,
#else
                                             .fixedPDO.frSwapRequiredCurrent = kFRSwap_CurrentDefaultUSB,
#endif
#endif
                                             .fixedPDO.higherCapability         = 1U,
                                             .fixedPDO.usbCommunicationsCapable = 0U,
                                             .fixedPDO.voltage                  = (5U * 1000U / 50U),
                                             .fixedPDO.operateCurrent           = (3U * 100U),
                                         },
                                         {
                                             /* PDO2: fixed: 9V, 2A */
                                             .fixedPDO.fixedSupply    = kPDO_Fixed,
                                             .fixedPDO.voltage        = (9U * 1000U / 50U),
                                             .fixedPDO.operateCurrent = (2U * 100U),
                                         }};

const static pd_dp_peripheral_interface_t s_boardChipInterface = {PD_DpBoardChipsInit, PD_DpBoardChipsDeinit,
                                                                  PD_DpBoardChipsControl};

#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
static pd_auto_policy_t s_autoPolicyForAllPorts = {
    0,                          /* autoRequestPRSwapAsSource */
    0,                          /* autoRequestPRSwapAsSink */
    kAutoRequestProcess_Accept, /* autoAcceptPRSwapAsSource */
    kAutoRequestProcess_Accept, /* autoAcceptPRSwapAsSink */
    kPD_DataRoleNone,           /* autoRequestDRSwap */
    kAutoRequestProcess_Accept, /* autoAcceptDRSwapToDFP */
    kAutoRequestProcess_Reject, /* autoAcceptDRSwapToUFP */
    kPD_VconnNone,              /* autoRequestVConnSwap */
    kAutoRequestProcess_Accept, /* autoAcceptVconnSwapToOn */
    kAutoRequestProcess_Accept, /* autoAcceptVconnSwapToOff */
    1,                          /* autoSinkNegotiation */
};
#endif

#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE > 0)
/* displayport config */
const static pd_cbtl_crossbar_config_t s_CrossbarConfig1 = {
    PD_PORT1_SHIELD2_ORIENT_PORT,  /* orientControlPort */
    PD_PORT1_SHIELD2_ORIENT_PIN,   /* orientControlPin */
    PD_PORT1_SHIELD2_XSD_PORT,     /* xsdnControlPort */
    PD_PORT1_SHIELD2_XSD_PIN,      /* xsdnControlPin */
    PD_PORT1_SHIELD2_DP4LANE_PORT, /* dp4laneControlPort */
    PD_PORT1_SHIELD2_DP4LANE_PIN,  /* dp4laneControlPin */
};
const static pd_hpd_driver_config_t s_HpdDriverConfig1 = {
    PD_PORT1_SHIELD2_HPD_PORT, /* hpd port */
    PD_PORT1_SHIELD2_HPD_PIN,  /* hpd pin */
};

static pd_dp_redriver_config_t s_Ptn36502Config1 = {
    kInterface_i2c0 + BOARD_PD_I2C_INDEX, /* I2C master */
    PD_PTN36502_SLAVE_ADDRESS,            /* slave address */
    0u,                                   /* Source clock */
    (void *)HW_I2CReleaseBus,             /* I2C release bus */
};

const static pd_dp_peripheral_config_t s_dpBoardConfig1 = {
    (void *)&s_CrossbarConfig1,
    (void *)&s_HpdDriverConfig1,
    (void *)&s_Ptn36502Config1,
};

const static pd_dp_mode_obj_t s_DPCaps[] = {
    {
        .bitFields.portCap              = kDPPortCap_DFPD,
        .bitFields.signalSupport        = kDPSignal_DP,
        .bitFields.receptacleIndication = kReceptacle_TypeCReceptacle,
        .bitFields.usb2SignalNotUsed    = kUSB2_Required,
        .bitFields.DFPDPinSupport       = kPinAssign_C | kPinAssign_D | kPinAssign_E,
        .bitFields.UFPDPinSupport       = kPinAssign_DeSelect,
    },
};

static pd_alt_mode_dp_config_t s_DPConfig1 = {
    .peripheralConfig    = &s_dpBoardConfig1,
    .peripheralInterface = &s_boardChipInterface,
    .modesList           = (uint32_t *)&s_DPCaps[0], /* mode list */
    .modesCount          = 1,                        /* mode count */
    .dpStatusConfig =
        {
            .bitFields.multiFunctionPreferred = 0U,
        },
    .dpConfigurationsConfig =
        {
            .bitFields.configureUFPUPin = kPinAssign_C | kPinAssign_D | kPinAssign_E,
        },
};

const static pd_vdm_identity_data_t s_DPDiscoverIdentityVDO = {
    .idHeaderVDO =
        {
            .bitFields.usbVendorID                     = PD_VENDOR_VID,
            .bitFields.reserved                        = 0,
            .bitFields.connectorType                   = kConnectorType_UsbTypecReceptacle,
            .bitFields.productTypeDFP                  = kProductTypeDfp_Undefined,
            .bitFields.modalOperateSupport             = kModalOperation_Supported,
            .bitFields.productTypeUFPOrCablePlug       = kProductTypeUfp_Undefined,
            .bitFields.usbCommunicationCapableAsDevice = kUsbCommunicationCapableAsDevice_Unsupported,
            .bitFields.usbCommunicationCapableAsHost   = kUsbCommunicationCapableAsHost_Unsupported,
        },
    .certStatVDO = PD_CERT_STAT_VDO,
    .bcdDevice   = PD_CONFIG_BCD_DEVICE,
    .pid         = PD_CONFIG_PID,
};

/* alt mode config */
const static pd_alt_mode_module_t s_AltModeModules1[] = {{0xFF01, &s_DPConfig1}};
const static pd_alt_mode_config_t s_AltModeConfig1    = {
    .altModeRole         = kDataConfig_DFP,
    .modules             = &s_AltModeModules1[0],
    .moduleCount         = 1,
    .identityData        = (uint32_t *)&s_DPDiscoverIdentityVDO,
    .identityObjectCount = 3,
};

const static pd_power_port_config_t s_Port1PowerConfig = {
    (uint32_t *)&s_PortSourceCaps[0],                   /* source caps */
    (uint32_t *)&s_PortSinkcaps[0],                     /* self sink caps */
    sizeof(s_PortSourceCaps) / sizeof(pd_source_pdo_t), /* source cap count */
    sizeof(s_PortSinkcaps) / sizeof(pd_sink_pdo_t),     /* sink cap count */
    kPowerConfig_DRPToggling,                           /* typec role */
    PD_DEMO_TYPEC_CURRENT,                              /* source: Rp current level */
    kTypecTry_None,                                     /* drp try function */
    kDataConfig_DRD,                                    /* data function */
    1,                                                  /* support vconn */
    0,                                                  /* reserved */
    (void *)&s_AltModeConfig1,                          /* alt mode config */
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
    (void *)&s_autoPolicyForAllPorts, /* auto policy */
#else
    NULL, /* auto policy */
#endif
    NULL,
};

static pd_phy_config_t s_Port1PhyConfig = {
    .i2cInstance   = kInterface_i2c0 + BOARD_PD_I2C_INDEX,
    .slaveAddress  = 0x52u,
    .i2cSrcClock   = 0u,
    .i2cReleaseBus = HW_I2CReleaseBus,
    .alertPort     = PD_PORT1_PHY_INTERRUPT_PORT,
    .alertPin      = PD_PORT1_PHY_INTERRUPT_PIN,
    .alertPriority = PD_PORT1_PHY_INTERRUPT_PRIORITY,
};

pd_instance_config_t g_Port1PDConfig = {
    kDeviceType_NormalPowerPort, /* normal power port */
    kPD_PhyPTN5110,
    (void *)&s_Port1PhyConfig,
    (void *)&s_Port1PowerConfig,
};
#endif

pd_instance_config_t *g_PortsConfigArray[] = {
#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE)
    &g_Port1PDConfig,
#endif
};

#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE > 0)
pd_app_t g_PDAppInstancePort1;
#endif
#if (defined PD_DEMO_PORT2_ENABLE) && (PD_DEMO_PORT2_ENABLE > 0)
pd_app_t g_PDAppInstancePort2;
#endif
#if (defined PD_DEMO_PORT3_ENABLE) && (PD_DEMO_PORT3_ENABLE > 0)
pd_app_t g_PDAppInstancePort3;
#endif
#if (defined PD_DEMO_PORT4_ENABLE) && (PD_DEMO_PORT4_ENABLE > 0)
pd_app_t g_PDAppInstancePort4;
#endif

pd_app_t *g_PDAppInstanceArray[] = {
#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE > 0)
    &g_PDAppInstancePort1,
#endif
#if (defined PD_DEMO_PORT2_ENABLE) && (PD_DEMO_PORT2_ENABLE > 0)
    &g_PDAppInstancePort2,
#endif
#if (defined PD_DEMO_PORT3_ENABLE) && (PD_DEMO_PORT3_ENABLE > 0)
    &g_PDAppInstancePort3,
#endif
#if (defined PD_DEMO_PORT4_ENABLE) && (PD_DEMO_PORT4_ENABLE > 0)
    &g_PDAppInstancePort4,
#endif
};

pd_power_handle_callback_t callbackFunctions = {
    PD_PowerSrcTurnOnDefaultVbus,  PD_PowerSrcTurnOnRequestVbus,  PD_PowerSrcTurnOffVbus,
    PD_PowerSrcGotoMinReducePower, PD_PowerSnkDrawTypeCVbus,      PD_PowerSnkDrawRequestVbus,
    PD_PowerSnkStopDrawVbus,       PD_PowerSnkGotoMinReducePower, PD_PowerControlVconn,
};

pd_demo_global_t g_DemoGlobal;
volatile uint32_t g_SoftTimerCount;
TIMER_HANDLE_DEFINE(g_PDTimerHandle);
/*******************************************************************************
 * Code
 ******************************************************************************/

static void PD_GpioInit(pd_app_t *pdAppInstance)
{
    pd_demo_io_init_t portsDemoPinConfigArray[] = {
#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE)
        {
            .extraEnSrcPort = PD_PORT1_EXTRA_SRC_PORT,
            .extraEnSrcPin  = PD_PORT1_EXTRA_SRC_PIN,
        },
#endif
    };
    pd_demo_io_init_t *demoGpioPinConfig = &portsDemoPinConfigArray[pdAppInstance->portNumber - 1U];
    hal_gpio_pin_config_t config;

    config.direction = kHAL_GpioDirectionOut;
    config.port      = demoGpioPinConfig->extraEnSrcPort;
    config.pin       = demoGpioPinConfig->extraEnSrcPin;
    config.level     = 1;
    HAL_GpioInit((hal_gpio_handle_t *)(&pdAppInstance->gpioExtraSrcHandle[0]), &config);
}

#if ((!defined(__DSC__)) || (!defined(__CW__)))
static void PD_SystickInit(void)
{
    /* For FreeRTOS, don't need this function, FreeRTOS will initialize systick by self */
    SYSTICK_LOAD_REG          = SYSTICK_VALUE_MASK;
    SYSTICK_CURRENT_VALUE_REG = 0UL;
    SYSTICK_CTRL_REG          = (SYSTICK_CLK_BIT | SYSTICK_ENABLE_BIT);
}
#endif

void PD_WaitUs(uint32_t us)
{
#if (defined(__DSC__) && defined(__CW__))
    int32_t sysTickMaxValue   = 1000;
    int32_t startSystickCount = HAL_TimerGetCurrentTimerCount((hal_timer_handle_t)&g_PDTimerHandle[0]);
    int32_t waitCount         = us;
    int32_t currentCount;

    do
    {
        currentCount = HAL_TimerGetCurrentTimerCount((hal_timer_handle_t)&g_PDTimerHandle[0]);
        /* systick is increased */
        currentCount = currentCount - startSystickCount;
        if (currentCount < 0)
        {
            currentCount = (sysTickMaxValue + currentCount);
        }
    } while (currentCount < waitCount);
#else
    int32_t sysTickMaxValue   = SYSTICK_LOAD_REG;
    int32_t startSystickCount = ((SYSTICK_CURRENT_VALUE_REG)&SYSTICK_VALUE_MASK);
    /* systick's clock source is core clock */
    int32_t waitCount = SystemCoreClock / 1000000 * us;
    int32_t currentCount;

    do
    {
        currentCount = ((SYSTICK_CURRENT_VALUE_REG)&SYSTICK_VALUE_MASK);
        /* systick is decreased */
        currentCount = startSystickCount - currentCount;
        if (currentCount < 0)
        {
            currentCount = (sysTickMaxValue + currentCount);
        }
    } while (currentCount < waitCount);
#endif
}

void PD_BMEnterCritical(uint32_t *sr)
{
    *sr = DisableGlobalIRQ();
}

void PD_BMExitCritical(uint32_t sr)
{
    EnableGlobalIRQ(sr);
}

static pd_status_t PD_DpmConnectCallback(void *callbackParam, uint32_t event, void *param)
{
    pd_status_t status      = kStatus_PD_Error;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    switch (event)
    {
        case PD_DISCONNECTED:
            PD_PowerSrcTurnOffVbus(pdAppInstance, kVbusPower_Stable);
            PRINTF("port %d disconnect\r\n", pdAppInstance->portNumber);
            status = kStatus_PD_Success;
            break;

        case PD_CONNECT_ROLE_CHANGE:
        case PD_CONNECTED:
        {
            uint8_t getInfo;

            PD_NX20PExitDeadBatteryMode(pdAppInstance->portNumber);
            pdAppInstance->partnerSourceCapNumber = 0;
            pdAppInstance->partnerSinkCapNumber   = 0;
            pdAppInstance->portShieldVersion      = 0;
            PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &getInfo);

            PRINTF((event == PD_CONNECTED) ? "port %d connected," : "port %d connect change,",
                   pdAppInstance->portNumber);
            PRINTF(" power role:%s,", (getInfo == kPD_PowerRoleSource) ? "Source" : "Sink");
            PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_DATA_ROLE, &getInfo);
            PRINTF(" data role:%s,", (getInfo == kPD_DataRoleDFP) ? "DFP" : "UFP");
            PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_VCONN_ROLE, &getInfo);
            PRINTF(" vconn source:%s\r\n", (getInfo == kPD_IsVconnSource) ? "yes" : "no");
            PRINTF("Please input '0' to show test menu\r\n");
            status = kStatus_PD_Success;
            break;
        }

        default:
            /* no action */
            break;
    }

    return status;
}

static pd_status_t PD_StackDemoAppCallback(void *callbackParam, uint32_t event, void *param)
{
    pd_status_t status      = kStatus_PD_Error;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    switch (event)
    {
        case PD_FUNCTION_DISABLED:
            /* need hard or software reset */
            status = kStatus_PD_Success;
            break;

        case PD_CONNECTED:
        case PD_CONNECT_ROLE_CHANGE:
        case PD_DISCONNECTED:
            status = PD_DpmConnectCallback(callbackParam, event, param);
            break;

        case PD_DPM_ALTMODE_DP_DFP_MODE_CONFIGURED:
            PRINTF("enter and configure DP\r\n");
            break;

        case PD_DPM_ALTMODE_DP_DFP_MODE_UNCONFIGURED:
            PRINTF("exit DP\r\n");
            break;

        case PD_DPM_OVP_OCP_FAULT:
            PD_NX20PClearInt(pdAppInstance->portNumber);
            break;

        case PD_DPM_VBUS_ALARM:
            /* Users need to take care of this. Some corrective actions may be taken, such as disconnect
               or debounce time to turn off the power switch. It is up to the system power management. */
            break;

        default:
            status = PD_DpmAppCommandCallback(callbackParam, event, param);
            break;
    }
    return status;
}

static void PD_AppInit(void)
{
    uint8_t index;
    pd_app_t *pdAppInstance;
    pd_app_t *pdAppInstanceArray[] = {
#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE > 0)
        &g_PDAppInstancePort1,
#else
        NULL,
#endif
#if (defined PD_DEMO_PORT2_ENABLE) && (PD_DEMO_PORT2_ENABLE > 0)
        &g_PDAppInstancePort2,
#else
        NULL,
#endif
#if (defined PD_DEMO_PORT3_ENABLE) && (PD_DEMO_PORT3_ENABLE > 0)
        &g_PDAppInstancePort3,
#else
        NULL,
#endif
#if (defined PD_DEMO_PORT4_ENABLE) && (PD_DEMO_PORT4_ENABLE > 0)
        &g_PDAppInstancePort4,
#else
        NULL,
#endif
    };

    g_SoftTimerCount = 0U;
    for (index = 0U; index < 4U; ++index)
    {
        if (pdAppInstanceArray[index] != NULL)
        {
            pdAppInstanceArray[index]->portNumber = (index + 1);
        }
    }

    for (index = 0U; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        pdAppInstance                = g_PDAppInstanceArray[index];
        pdAppInstance->pdHandle      = NULL;
        pdAppInstance->pdConfigParam = g_PortsConfigArray[index];
        ((pd_phy_config_t *)pdAppInstance->pdConfigParam->phyConfig)->i2cSrcClock =
            HW_I2CGetFreq(((pd_phy_config_t *)pdAppInstance->pdConfigParam->phyConfig)->i2cInstance);

        if (PD_InstanceInit(&pdAppInstance->pdHandle, PD_StackDemoAppCallback, &callbackFunctions, pdAppInstance,
                            g_PortsConfigArray[index]) != kStatus_PD_Success)
        {
            PRINTF("pd port %d init fail\r\n", pdAppInstance->portNumber);
        }
        else
        {
            PD_GpioInit(pdAppInstance);
            PD_PowerBoardControlInit(pdAppInstance->portNumber, pdAppInstance->pdHandle,
                                     (hal_gpio_handle_t)(&pdAppInstance->gpioExtraSrcHandle[0]));

            pdAppInstance->msgSop                 = kPD_MsgSOP;
            pdAppInstance->partnerSourceCapNumber = 0;
            pdAppInstance->partnerSinkCapNumber   = 0;
            pdAppInstance->reqestResponse         = kCommandResult_Accept;
            /* self ext cap */
            pdAppInstance->selfExtCap.vid       = PD_VENDOR_VID;
            pdAppInstance->selfExtCap.pid       = PD_CONFIG_PID;
            pdAppInstance->selfExtCap.xid       = PD_CONFIG_XID;
            pdAppInstance->selfExtCap.fwVersion = PD_CONFIG_FW_VER;
            pdAppInstance->selfExtCap.hwVersion = PD_CONFIG_HW_VER;
            pdAppInstance->selfExtCap.batteries = 0;
            pdAppInstance->selfExtCap.sourcePDP = PD_SOURCE_POWER;
            /* self battery */
            pdAppInstance->selfBatteryCap.batteryDesignCap         = 10;
            pdAppInstance->selfBatteryCap.batteryLastFullChargeCap = 10;
            pdAppInstance->selfBatteryCap.batteryType              = 1; /* invalid battery reference */
            pdAppInstance->selfBatteryCap.pid                      = PD_CONFIG_PID;
            pdAppInstance->selfBatteryCap.vid                      = PD_VENDOR_VID;
            pdAppInstance->selfBatteryStatus.batterInfo            = 0;
            /* no battery */
            pdAppInstance->selfBatteryStatus.batterInfoBitFields.invalidBatteryRef = 1;
            pdAppInstance->selfBatteryStatus.batteryPC                             = 10;
            /* self status */
            pdAppInstance->selfStatus.eventFlags          = 0;
            pdAppInstance->selfStatus.internalTemp        = 0;
            pdAppInstance->selfStatus.presentBatteryInput = 0;
            pdAppInstance->selfStatus.presentInput        = 0;
            pdAppInstance->selfStatus.temperatureStatus   = 0;
            pdAppInstance->selfStatus.powerStatus         = 0;
            /* evaluate result */
            pdAppInstance->prSwapAccept    = 1;
            pdAppInstance->drSwapAccept    = 1;
            pdAppInstance->vconnSwapAccept = 1;
            PRINTF("pd port %d init success\r\n", pdAppInstance->portNumber);
        }
    }
}

#if (defined(__DSC__) && defined(__CW__))
static void HW_GpioPDPHYPortsIntCallback(void *callbackParam)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    PD_PTN5110IsrFunction(pdAppInstance->pdHandle);
}
#endif

/* ms */
uint32_t PD_DemoSoftTimer_msGet(void)
{
    return g_SoftTimerCount;
}

/* ms */
uint32_t PD_DemoSoftTimer_getInterval(uint32_t startTime)
{
    if (g_SoftTimerCount >= startTime)
    {
        return (g_SoftTimerCount - startTime);
    }
    else
    {
        return (0xFFFFFFFFu - (startTime - g_SoftTimerCount));
    }
}

static void PD_AppTimerInit(void)
{
    hal_timer_config_t halTimerConfig;
    halTimerConfig.timeout     = 1000;
    halTimerConfig.srcClock_Hz = HW_TimerGetFreq();
    halTimerConfig.instance    = PD_TIMER_INSTANCE;
    HAL_TimerInit((hal_timer_handle_t)&g_PDTimerHandle[0], &halTimerConfig);

    HAL_TimerInstallCallback((hal_timer_handle_t)&g_PDTimerHandle[0], HW_TimerCallback, NULL);
    HAL_TimerEnable((hal_timer_handle_t)&g_PDTimerHandle[0]);
}

void HW_TimerCallback(void *callbackParam)
{
    /* Callback into timer service */
    uint8_t index;
    for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        PD_TimerIsrFunction(g_PDAppInstanceArray[index]->pdHandle);
#if (defined(__DSC__) && defined(__CW__))
        /* DSC doesn't support low level interrupt, workaround this to poll
         * alert pin and external power pin state in the 1ms timer IRQ */
        HW_GpioPDPHYPortsIntCallback(g_PDAppInstanceArray[index]);
#endif
    }
    PD_DpBoardChip1msIsr();
    g_SoftTimerCount++;
}

int main(void)
{
#if (!defined PD_CONFIG_COMMON_TASK) && (PD_CONFIG_COMMON_TASK != 0)
    uint8_t index;
#endif
    BOARD_InitHardware();

#if ((!defined(__DSC__)) || (!defined(__CW__)))
    PD_SystickInit();
#endif
    /* @TEST_ANCHOR */
    PD_AppTimerInit();
    PD_AppInit();
    PD_DemoInit();

    while (1)
    {
#if (defined PD_CONFIG_COMMON_TASK) && (PD_CONFIG_COMMON_TASK)
        PD_Task();
#else
        for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
        {
            PD_InstanceTask(g_PDAppInstanceArray[index]->pdHandle);
        }
#endif
        PD_AltModeTask();
        PD_DpBoardChipTask();
        PD_DemoTaskFun();
    }
}
