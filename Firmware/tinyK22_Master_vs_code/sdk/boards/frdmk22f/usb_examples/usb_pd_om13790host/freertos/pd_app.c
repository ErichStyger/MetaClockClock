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
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "pd_app.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pd_power_interface.h"
#include "board.h"
#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
#include "usb_pd_auto_policy.h"
#endif
#include "pd_board_config.h"
#include "pd_power_nx20p3483.h"
#include "fsl_adapter_timer.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if (PD_DEMO_PORTS_COUNT > PD_CONFIG_MAX_PORT)
#error "please increase the instance count"
#endif

#if (defined BOARD_PD_USB3_CROSS_SUPPORT) && (BOARD_PD_USB3_CROSS_SUPPORT)
/* Register indexes */
#define REG_USB_US_TXRX_CONTROL   0x04U
#define REG_USB_DS_TXRX_CONTROL   0x05U
#define REG_DP_LANE0_TXRX_CONTROL 0x07U
#define REG_DP_LANE1_TXRX_CONTROL 0x08U
#define REG_DP_LANE2_TXRX_CONTROL 0x09U
#define REG_DP_LANE3_TXRX_CONTROL 0x0AU
#define REG_MODE_CONTROL1         0x0BU
/* reg mode control 1 */
#define REG_MODE_CONTROL1_OPERATIONAL_MODE_SHIFT (0x00u)
#define REG_MODE_CONTROL1_OPERATIONAL_MODE_MASK  (0x07u)
#define REG_MODE_CONTROL1_DATA_ROLE_SHIFT        (0x06u)
#define REG_MODE_CONTROL1_DATA_ROLE_MASK         (0xC0u)
#define REG_MODE_CONTROL1_ORIENT_SHIFT           (0x05u)
#define REG_MODE_CONTROL1_ORIENT_MASK            (0x20u)

#define PTN36502_POWER_SAVE_MODE_VALUE       (0U)
#define PTN36502_USB_MODE_VALUE              (1U)
#define REG_SET_FIELD(regVal, mask, bitsVal) (regVal) = (((regVal) & (~(mask))) | (bitsVal))
#define ptn36502_I2cWriteRegister(i2cHandle, slaveAddress, registerAdd, data) \
    PD_I2cWriteBlocking(i2cHandle, slaveAddress, registerAdd, 1U, (data), 1U);
#endif

/* The follow MACROs are for passing compliance test, it is not actual product logical */
#define PD_SOURCE_POWER (18U) /* W */
#ifndef PD_TIMER_INSTANCE
#define PD_TIMER_INSTANCE (0U)
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
uint32_t PD_DemoSoftTimer_msGet(void);
uint32_t PD_DemoSoftTimer_getInterval(uint32_t startTime);
void PD_PTN36502Init(pd_app_t *pdAppInstance);
uint32_t HW_I2CGetFreq(uint8_t i2cInstance);
void HW_I2CReleaseBus(void);
void PD_DemoInit(void);
void PD_DemoTaskFun(void);
void BOARD_InitHardware(void);
#if (defined BOARD_PD_SW_INPUT_SUPPORT) && (BOARD_PD_SW_INPUT_SUPPORT)
void PD_Demo1msIsrProcessSW(pd_app_t *pdAppInstance);
#endif
uint32_t HW_TimerGetFreq(void);
void HW_TimerCallback(void *callbackParam);
#if defined(PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT) && (PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT)
void PD_PortsGpioExternalPowerDetectIntCallback(void *callbackParam);
#endif
pd_status_t PD_DpmAppCommandCallback(void *callbackParam, uint32_t event, void *param);
void pd_DemoConsoleReadFun(void *arg);
#if (defined PD_CONFIG_PHY_LOW_POWER_LEVEL) && (PD_CONFIG_PHY_LOW_POWER_LEVEL)
void HW_LPTimerInit(void);
#endif

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
        .fixedPDO.voltage     = (9 * 1000U / 50U),
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

#if (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
#if (defined PD_TEST_ENABLE_AUTO_POLICY)
#if (PD_TEST_ENABLE_AUTO_POLICY == 1U)
static pd_auto_policy_t s_autoPolicy1 = {
    0,                          /* autoRequestPRSwapAsSource */
    0,                          /* autoRequestPRSwapAsSink */
    kAutoRequestProcess_Accept, /* autoAcceptPRSwapAsSource */
    kAutoRequestProcess_Accept, /* autoAcceptPRSwapAsSink */
    kPD_DataRoleNone,           /* autoRequestDRSwap */
    kAutoRequestProcess_Accept, /* autoAcceptDRSwapToDFP */
    kAutoRequestProcess_Accept, /* autoAcceptDRSwapToUFP */
    kPD_VconnNone,              /* autoRequestVConnSwap */
    kAutoRequestProcess_Accept, /* autoAcceptVconnSwapToOn */
    kAutoRequestProcess_Accept, /* autoAcceptVconnSwapToOff */
    0,                          /* autoSinkNegotiation */
};
#endif

#if (PD_TEST_ENABLE_AUTO_POLICY == 2U)
static pd_auto_policy_t s_autoPolicy2 = {
    0,                          /* autoRequestPRSwapAsSource */
    1,                          /* autoRequestPRSwapAsSink */
    kAutoRequestProcess_Reject, /* autoAcceptPRSwapAsSource */
    kAutoRequestProcess_Accept, /* autoAcceptPRSwapAsSink */
    kPD_DataRoleNone,           /* autoRequestDRSwap */
    kAutoRequestProcess_Accept, /* autoAcceptDRSwapToDFP */
    kAutoRequestProcess_Reject, /* autoAcceptDRSwapToUFP */
    kPD_VconnNone,              /* autoRequestVConnSwap */
    kAutoRequestProcess_Accept, /* autoAcceptVconnSwapToOn */
    kAutoRequestProcess_Reject, /* autoAcceptVconnSwapToOff */
    1,                          /* autoSinkNegotiation */
};
#endif

#if (PD_TEST_ENABLE_AUTO_POLICY == 3U)
static pd_auto_policy_t s_autoPolicy3 = {
    0,                          /* autoRequestPRSwapAsSource */
    0,                          /* autoRequestPRSwapAsSink */
    kAutoRequestProcess_Accept, /* autoAcceptPRSwapAsSource */
    kAutoRequestProcess_Reject, /* autoAcceptPRSwapAsSink */
    kPD_DataRoleDFP,            /* autoRequestDRSwap */
    kAutoRequestProcess_Reject, /* autoAcceptDRSwapToDFP */
    kAutoRequestProcess_Accept, /* autoAcceptDRSwapToUFP */
    kPD_VconnNone,              /* autoRequestVConnSwap */
    kAutoRequestProcess_Reject, /* autoAcceptVconnSwapToOn */
    kAutoRequestProcess_Accept, /* autoAcceptVconnSwapToOff */
    0,                          /* autoSinkNegotiation */
};
#endif

#if (PD_TEST_ENABLE_AUTO_POLICY == 4U)
static pd_auto_policy_t s_autoPolicy4 = {
    0,                          /* autoRequestPRSwapAsSource */
    0,                          /* autoRequestPRSwapAsSink */
    kAutoRequestProcess_Reject, /* autoAcceptPRSwapAsSource */
    kAutoRequestProcess_Reject, /* autoAcceptPRSwapAsSink */
    kPD_DataRoleNone,           /* autoRequestDRSwap */
    kAutoRequestProcess_Reject, /* autoAcceptDRSwapToDFP */
    kAutoRequestProcess_Reject, /* autoAcceptDRSwapToUFP */
    kPD_IsVconnSource,          /* autoRequestVConnSwap */
    kAutoRequestProcess_Reject, /* autoAcceptVconnSwapToOn */
    kAutoRequestProcess_Reject, /* autoAcceptVconnSwapToOff */
    0,                          /* autoSinkNegotiation */
};
#endif
#endif
#endif

#if (defined PD_COMPLIANCE_TEST_DRP) && (PD_COMPLIANCE_TEST_DRP)
static pd_power_port_config_t s_Port1PowerConfig = {
    (uint32_t *)&s_PortSourceCaps[0],                   /* source caps */
    (uint32_t *)&s_PortSinkcaps[0],                     /* self sink caps */
    sizeof(s_PortSourceCaps) / sizeof(pd_source_pdo_t), /* source cap count */
    sizeof(s_PortSinkcaps) / sizeof(pd_sink_pdo_t),     /* sink cap count */
    kPowerConfig_DRPToggling,                           /* typec role */
    PD_DEMO_TYPEC_CURRENT,                              /* source: Rp current level */
    kTypecTry_None,                                     /* drp try function */
    kDataConfig_DRD,                                    /* data function */
    1U,                                                 /* support vconn */
    0U,                                                 /* reserved */
    NULL,                                               /* alt mode config */
#if (defined PD_TEST_ENABLE_AUTO_POLICY) && (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
#if (PD_TEST_ENABLE_AUTO_POLICY == 1)
    (void *)&s_autoPolicy1,
#elif (PD_TEST_ENABLE_AUTO_POLICY == 2)
    (void *)&s_autoPolicy2,
#elif (PD_TEST_ENABLE_AUTO_POLICY == 3)
    (void *)&s_autoPolicy3,
#elif (PD_TEST_ENABLE_AUTO_POLICY == 4)
    (void *)&s_autoPolicy4,
#else
    NULL,
#endif
#else
    NULL,
#endif
    NULL,
};

#endif

#if (defined PD_COMPLIANCE_TEST_DRP_TRY_SNK) && (PD_COMPLIANCE_TEST_DRP_TRY_SNK)
static pd_power_port_config_t s_Port1PowerConfig = {
    (uint32_t *)&s_PortSourceCaps[0],                   /* source caps */
    (uint32_t *)&s_PortSinkcaps[0],                     /* self sink caps */
    sizeof(s_PortSourceCaps) / sizeof(pd_source_pdo_t), /* source cap count */
    sizeof(s_PortSinkcaps) / sizeof(pd_sink_pdo_t),     /* sink cap count */
    kPowerConfig_DRPToggling,                           /* typec role */
    PD_DEMO_TYPEC_CURRENT,                              /* source: Rp current level */
    kTypecTry_Snk,                                      /* drp try function */
    kDataConfig_DRD,                                    /* data function */
    1,                                                  /* support vconn */
    0,                                                  /* reserved */
    NULL,
#if (defined PD_TEST_ENABLE_AUTO_POLICY) && (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
#if (PD_TEST_ENABLE_AUTO_POLICY == 1)
    (void *)&s_autoPolicy1,
#elif (PD_TEST_ENABLE_AUTO_POLICY == 2)
    (void *)&s_autoPolicy2,
#elif (PD_TEST_ENABLE_AUTO_POLICY == 3)
    (void *)&s_autoPolicy3,
#elif (PD_TEST_ENABLE_AUTO_POLICY == 4)
    (void *)&s_autoPolicy4,
#else
    NULL,
#endif
#else
    NULL,
#endif
    NULL,
};
#endif

#if (defined PD_COMPLIANCE_TEST_DRP_TRY_SRC) && (PD_COMPLIANCE_TEST_DRP_TRY_SRC)
static pd_power_port_config_t s_Port1PowerConfig = {
    (uint32_t *)&s_PortSourceCaps[0],                   /* source caps */
    (uint32_t *)&s_PortSinkcaps[0],                     /* self sink caps */
    sizeof(s_PortSourceCaps) / sizeof(pd_source_pdo_t), /* source cap count */
    sizeof(s_PortSinkcaps) / sizeof(pd_sink_pdo_t),     /* sink cap count */
    kPowerConfig_DRPToggling,                           /* typec role */
    PD_DEMO_TYPEC_CURRENT,                              /* source: Rp current level */
    kTypecTry_Src,                                      /* drp try function */
    kDataConfig_DRD,                                    /* data function */
    1,                                                  /* support vconn */
    0,                                                  /* reserved */
    NULL,
#if (defined PD_TEST_ENABLE_AUTO_POLICY) && (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
#if (PD_TEST_ENABLE_AUTO_POLICY == 1)
    (void *)&s_autoPolicy1,
#elif (PD_TEST_ENABLE_AUTO_POLICY == 2)
    (void *)&s_autoPolicy2,
#elif (PD_TEST_ENABLE_AUTO_POLICY == 3)
    (void *)&s_autoPolicy3,
#elif (PD_TEST_ENABLE_AUTO_POLICY == 4)
    (void *)&s_autoPolicy4,
#else
    NULL,
#endif
#else
    NULL,
#endif
    NULL,
};
#endif

#if (defined PD_COMPLIANCE_TEST_CONSUMER_PROVIDER) && (PD_COMPLIANCE_TEST_CONSUMER_PROVIDER)
static pd_power_port_config_t s_Port1PowerConfig = {
    (uint32_t *)&s_PortSourceCaps[0],                   /* source caps */
    (uint32_t *)&s_PortSinkcaps[0],                     /* self sink caps */
    sizeof(s_PortSourceCaps) / sizeof(pd_source_pdo_t), /* source cap count */
    sizeof(s_PortSinkcaps) / sizeof(pd_sink_pdo_t),     /* sink cap count */
    kPowerConfig_SinkDefault,                           /* typec role */
    PD_DEMO_TYPEC_CURRENT,                              /* source: Rp current level */
    kTypecTry_None,                                     /* drp try function */
    kDataConfig_DRD,                                    /* data function */
    1,                                                  /* support vconn */
    0,                                                  /* reserved */
    NULL,
#if (defined PD_TEST_ENABLE_AUTO_POLICY) && (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
#if (PD_TEST_ENABLE_AUTO_POLICY == 1)
    (void *)&s_autoPolicy1,
#elif (PD_TEST_ENABLE_AUTO_POLICY == 2)
    (void *)&s_autoPolicy2,
#elif (PD_TEST_ENABLE_AUTO_POLICY == 3)
    (void *)&s_autoPolicy3,
#elif (PD_TEST_ENABLE_AUTO_POLICY == 4)
    (void *)&s_autoPolicy4,
#else
    NULL,
#endif
#else
    NULL,
#endif
    NULL,
};
#endif

#if (defined PD_COMPLIANCE_TEST_PROVIDER_CONSUMER) && (PD_COMPLIANCE_TEST_PROVIDER_CONSUMER)
static pd_power_port_config_t s_Port1PowerConfig = {
    (uint32_t *)&s_PortSourceCaps[0],                   /* source caps */
    (uint32_t *)&s_PortSinkcaps[0],                     /* self sink caps */
    sizeof(s_PortSourceCaps) / sizeof(pd_source_pdo_t), /* source cap count */
    sizeof(s_PortSinkcaps) / sizeof(pd_sink_pdo_t),     /* sink cap count */
    kPowerConfig_SourceDefault,                         /* typec role */
    PD_DEMO_TYPEC_CURRENT,                              /* source: Rp current level */
    kTypecTry_None,                                     /* drp try function */
    kDataConfig_DRD,                                    /* data function */
    1,                                                  /* support vconn */
    0,                                                  /* reserved */
    NULL,
#if (defined PD_TEST_ENABLE_AUTO_POLICY) && (defined PD_CONFIG_ENABLE_AUTO_POLICY) && (PD_CONFIG_ENABLE_AUTO_POLICY)
#if (PD_TEST_ENABLE_AUTO_POLICY == 1)
    (void *)&s_autoPolicy1,
#elif (PD_TEST_ENABLE_AUTO_POLICY == 2)
    (void *)&s_autoPolicy2,
#elif (PD_TEST_ENABLE_AUTO_POLICY == 3)
    (void *)&s_autoPolicy3,
#elif (PD_TEST_ENABLE_AUTO_POLICY == 4)
    (void *)&s_autoPolicy4,
#else
    NULL,
#endif
#else
    NULL,
#endif
    NULL,
};
#endif

#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE)
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
    &s_Port1PhyConfig,
    &s_Port1PowerConfig,
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
            .dp4lanePort = PD_PORT1_SHIELD2_DP4LANE_PORT,
            .dp4lanePin  = PD_PORT1_SHIELD2_DP4LANE_PIN,

            .extraEnSrcPort = PD_PORT1_EXTRA_SRC_PORT,
            .extraEnSrcPin  = PD_PORT1_EXTRA_SRC_PIN,

            .xsdPort = PD_PORT1_SHIELD2_XSD_PORT,
            .xsdPin  = PD_PORT1_SHIELD2_XSD_PIN,

#if defined(PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT) && (PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT)
            .externalPowerDetectPort = PD_PORT1_SHIELD2_EXTERNAL_POWER_DETECT_PORT,
            .externalPowerDetectPin  = PD_PORT1_SHIELD2_EXTERNAL_POWER_DETECT_PIN,
#endif

            .orientPort = PD_PORT1_SHIELD2_ORIENT_PORT,
            .orientPin  = PD_PORT1_SHIELD2_ORIENT_PIN,

#if (defined BOARD_PD_SW_INPUT_SUPPORT) && (BOARD_PD_SW_INPUT_SUPPORT)
            .powerRequestSwPort = PD_PORT1_POWER_REQUEST_SW_PORT,
            .powerRequestSwPin  = PD_PORT1_POWER_REQUEST_SW_PIN,

            .powerChangeSwPort = PD_PORT1_POWER_CHANGE_SW_PORT,
            .powerChangeSwPin  = PD_PORT1_POWER_CHANGE_SW_PIN,
#endif
        },
#endif
    };
    pd_demo_io_init_t *demoGpioPinConfig = &portsDemoPinConfigArray[pdAppInstance->portNumber - 1];
    hal_gpio_pin_config_t config;

    config.direction = kHAL_GpioDirectionOut;
    config.port      = demoGpioPinConfig->extraEnSrcPort;
    config.pin       = demoGpioPinConfig->extraEnSrcPin;
    config.level     = 1;
    HAL_GpioInit((hal_gpio_handle_t *)(&pdAppInstance->gpioExtraSrcHandle[0]), &config);

    config.port  = demoGpioPinConfig->dp4lanePort;
    config.pin   = demoGpioPinConfig->dp4lanePin;
    config.level = 0;
    HAL_GpioInit((hal_gpio_handle_t *)(&pdAppInstance->gpioDp4LaneHandle[0]), &config);

    config.port = demoGpioPinConfig->xsdPort;
    config.pin  = demoGpioPinConfig->xsdPin;
    HAL_GpioInit((hal_gpio_handle_t *)(&pdAppInstance->gpioXsdHandle[0]), &config);

    config.port = demoGpioPinConfig->orientPort;
    config.pin  = demoGpioPinConfig->orientPin;
    HAL_GpioInit((hal_gpio_handle_t *)(&pdAppInstance->gpioOrientHandle[0]), &config);

    /* the external power */
    config.direction = kHAL_GpioDirectionIn;
    config.port      = demoGpioPinConfig->externalPowerDetectPort;
    config.pin       = demoGpioPinConfig->externalPowerDetectPin;
    config.level     = 0;
    HAL_GpioInit((hal_gpio_handle_t *)(&pdAppInstance->gpioExternalPowerDetectHandle[0]), &config);

#if (defined BOARD_PD_SW_INPUT_SUPPORT) && (BOARD_PD_SW_INPUT_SUPPORT)
    /* power request SW */
    config.port = demoGpioPinConfig->powerRequestSwPort;
    config.pin  = demoGpioPinConfig->powerRequestSwPin;
    HAL_GpioInit((hal_gpio_handle_t *)(&pdAppInstance->gpioPowerRequestSwHandle[0]), &config);

    /* power change SW */
    config.port = demoGpioPinConfig->powerChangeSwPort;
    config.pin  = demoGpioPinConfig->powerChangeSwPin;
    HAL_GpioInit((hal_gpio_handle_t *)(&pdAppInstance->gpioPowerChangeSwHandle[0]), &config);
#endif

    /* the external power detection interrupt gpio */
#if defined(PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT) && (PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT)
    HAL_GpioInstallCallback((hal_gpio_handle_t *)(&pdAppInstance->gpioExternalPowerDetectHandle[0]),
                            PD_PortsGpioExternalPowerDetectIntCallback, (void *)pdAppInstance);
#endif
}

#if defined(PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT) && (PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT)
void PD_PortsGpioExternalPowerDetectIntCallback(void *callbackParam)
{
    uint8_t getInfo;
    uint8_t externalPowerState;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    HAL_GpioGetInput((hal_gpio_handle_t)(&pdAppInstance->gpioExternalPowerDetectHandle[0]), &externalPowerState);
    if (externalPowerState == pdAppInstance->externalPowerState)
    {
        return;
    }
    pdAppInstance->externalPowerState = externalPowerState;
    PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &getInfo);
    if (getInfo == kPD_PowerRoleSource)
    {
        if (0U == externalPowerState)
        {
            if (pdAppInstance->isDoingFrs)
            {
                return;
            }
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
            if (PD_Command(pdAppInstance->pdHandle, PD_DPM_FAST_ROLE_SWAP, NULL) != kStatus_PD_Success)
            {
                return;
            }
            pdAppInstance->isDoingFrs = 1;
            PD_PowerSnkDrawTypeCVbus(pdAppInstance, kCurrent_StdUSB, kVbusPower_InFRSwap);
#endif
        }
        else
        {
            /* A source must immediately send new capabilities */
            if (PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_POWER_NEGOTIATION, NULL) != kStatus_PD_Success)
            {
                return;
            }
        }
    }
}
#endif

#if (defined BOARD_PD_USB3_CROSS_SUPPORT) && (BOARD_PD_USB3_CROSS_SUPPORT)
static void PD_PTN36502SetUSB3Function(pd_app_t *pdAppInstance)
{
    uint8_t modeControl1 = 0x00u;
    uint8_t getInfo;

    if (pdAppInstance->i2cHandle == NULL)
    {
        return;
    }

    PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_TYPEC_ORIENTATION, &getInfo);
    REG_SET_FIELD(modeControl1, (uint8_t)REG_MODE_CONTROL1_ORIENT_MASK,
                  ((0U != getInfo) ? (1U << REG_MODE_CONTROL1_ORIENT_SHIFT) : (0U)));
    PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_DATA_ROLE, &getInfo);
    REG_SET_FIELD(modeControl1, (uint8_t)REG_MODE_CONTROL1_DATA_ROLE_MASK,
                  ((getInfo == kPD_DataRoleDFP) ? (0U << REG_MODE_CONTROL1_DATA_ROLE_SHIFT) :
                                                  (2U << REG_MODE_CONTROL1_DATA_ROLE_SHIFT)));
    /* set as usb3 mode */
    REG_SET_FIELD(modeControl1, (uint8_t)REG_MODE_CONTROL1_OPERATIONAL_MODE_MASK, ((uint8_t)PTN36502_USB_MODE_VALUE));
    ptn36502_I2cWriteRegister(pdAppInstance->i2cHandle, PD_PTN36502_SLAVE_ADDRESS, REG_MODE_CONTROL1, &modeControl1);
}

void PD_PTN36502Init(pd_app_t *pdAppInstance)
{
    uint8_t modeControl1 = 0x00u;
    pd_dp_redriver_config_t *config;

    pd_dp_redriver_config_t portsPtn36502I2CConfigArray[] = {
#if (defined PD_DEMO_PORT1_ENABLE) && (PD_DEMO_PORT1_ENABLE)
        {
            .ptn36502I2CMaster     = kInterface_i2c0 + BOARD_PD_I2C_INDEX,
            .ptn36502SlaveAddress  = PD_PTN36502_SLAVE_ADDRESS,
            .ptn36502I2cSrcClock   = 0u,
            .ptn36502I2cReleaseBus = (void *)HW_I2CReleaseBus,
        },
#endif
    };

    config = &portsPtn36502I2CConfigArray[pdAppInstance->portNumber - 1];

    config->ptn36502I2cSrcClock = HW_I2CGetFreq(config->ptn36502I2CMaster);
    if (PD_I2cInit(&pdAppInstance->i2cHandle, config->ptn36502I2CMaster, config->ptn36502I2cSrcClock,
                   (PD_I2cReleaseBus)config->ptn36502I2cReleaseBus) != kStatus_PD_Success)
    {
        return;
    }

    if (pdAppInstance->i2cHandle == NULL)
    {
        return;
    }

    ptn36502_I2cWriteRegister(pdAppInstance->i2cHandle, config->ptn36502SlaveAddress, REG_USB_US_TXRX_CONTROL,
                              (uint8_t *)"\x51");
    ptn36502_I2cWriteRegister(pdAppInstance->i2cHandle, config->ptn36502SlaveAddress, REG_USB_DS_TXRX_CONTROL,
                              (uint8_t *)"\x51");
    ptn36502_I2cWriteRegister(pdAppInstance->i2cHandle, config->ptn36502SlaveAddress, REG_DP_LANE0_TXRX_CONTROL,
                              (uint8_t *)"\x29");
    ptn36502_I2cWriteRegister(pdAppInstance->i2cHandle, config->ptn36502SlaveAddress, REG_DP_LANE1_TXRX_CONTROL,
                              (uint8_t *)"\x29");
    ptn36502_I2cWriteRegister(pdAppInstance->i2cHandle, config->ptn36502SlaveAddress, REG_DP_LANE2_TXRX_CONTROL,
                              (uint8_t *)"\x29");
    ptn36502_I2cWriteRegister(pdAppInstance->i2cHandle, config->ptn36502SlaveAddress, REG_DP_LANE3_TXRX_CONTROL,
                              (uint8_t *)"\x29");

    /* set as power save state */
    REG_SET_FIELD(modeControl1, (uint8_t)REG_MODE_CONTROL1_OPERATIONAL_MODE_MASK,
                  ((uint8_t)PTN36502_POWER_SAVE_MODE_VALUE));
    ptn36502_I2cWriteRegister(pdAppInstance->i2cHandle, config->ptn36502SlaveAddress, REG_MODE_CONTROL1, &modeControl1);
}
#endif

static pd_status_t PD_DpmConnectCallback(void *callbackParam, uint32_t event, void *param)
{
    pd_status_t status      = kStatus_PD_Error;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    switch (event)
    {
        case PD_DISCONNECTED:
#if defined(PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT) && (PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT)
            HAL_GpioSetTriggerMode((hal_gpio_handle_t)(&pdAppInstance->gpioExternalPowerDetectHandle[0]),
                                   kHAL_GpioInterruptDisable); /* Disable interrupt */
#endif
            PD_PowerSrcTurnOffVbus(pdAppInstance, kVbusPower_Stable);
            pdAppInstance->selfHasEnterAlernateMode = 0;
            PRINTF("port %d disconnect\r\n", pdAppInstance->portNumber);
            status = kStatus_PD_Success;
            break;

        case PD_CONNECT_ROLE_CHANGE:
        case PD_CONNECTED:
        {
            uint8_t getInfo;

#if defined(PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT) && (PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT)
            pdAppInstance->isDoingFrs = 0;
            HAL_GpioGetInput((hal_gpio_handle_t)(&pdAppInstance->gpioExternalPowerDetectHandle[0]),
                             (uint8_t *)&pdAppInstance->externalPowerState);
            HAL_GpioSetTriggerMode((hal_gpio_handle_t)(&pdAppInstance->gpioExternalPowerDetectHandle[0]),
                                   kHAL_GpioInterruptEitherEdge); /* Enable interrupt */
#endif
            pdAppInstance->selfHasEnterAlernateMode = 0U;
            pdAppInstance->partnerSourceCapNumber   = 0U;
            pdAppInstance->partnerSinkCapNumber     = 0U;
            pdAppInstance->portShieldVersion        = 0U;
#if (defined BOARD_PD_USB3_CROSS_SUPPORT) && (BOARD_PD_USB3_CROSS_SUPPORT)
            PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_TYPEC_ORIENTATION, &getInfo);
            /* set CROSS based on the typec orientation */
            HAL_GpioSetOutput((hal_gpio_handle_t)(&pdAppInstance->gpioOrientHandle[0]), ((0U != getInfo) ? 1U : 0U));
            PD_PTN36502SetUSB3Function(pdAppInstance);
#endif
            PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &getInfo);
            if (getInfo == kPD_PowerRoleSink)
            {
                PD_NX20PExitDeadBatteryMode(pdAppInstance->portNumber);
            }
            PRINTF((event == PD_CONNECTED) ? "port %d connected," : "port %d connect change,",
                   pdAppInstance->portNumber);
            PRINTF(" power role:%s,", (getInfo == kPD_PowerRoleSource) ? "Source" : "Sink");
            PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_DATA_ROLE, &getInfo);
            PRINTF(" data role:%s,", (getInfo == kPD_DataRoleDFP) ? "DFP" : "UFP");
            PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_VCONN_ROLE, &getInfo);
            PRINTF(" vconn source:%s\r\n", (getInfo == kPD_IsVconnSource) ? "yes" : "no");
            status = kStatus_PD_Success;
            PRINTF("Please input '0' to show test menu\r\n");
            break;
        }

        default:
            break;
    }

    return status;
}

static pd_status_t PD_DpmDemoAppCallback(void *callbackParam, uint32_t event, void *param)
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

        case PD_DPM_OVP_OCP_FAULT:
            PD_NX20PClearInt(pdAppInstance->portNumber);
            break;

#if defined(PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT) && (PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT)
        case PD_DPM_GET_EXTERNAL_POWER_STATE:
            HAL_GpioGetInput((hal_gpio_handle_t)(&pdAppInstance->gpioExternalPowerDetectHandle[0]), (uint8_t *)param);
            status = kStatus_PD_Success;
            break;
#endif

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

    g_SoftTimerCount = 0;
    for (index = 0; index < 4; ++index)
    {
        if (pdAppInstanceArray[index] != NULL)
        {
            pdAppInstanceArray[index]->portNumber = (index + 1);
        }
    }

    for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        pdAppInstance                = g_PDAppInstanceArray[index];
        pdAppInstance->pdHandle      = NULL;
        pdAppInstance->pdConfigParam = g_PortsConfigArray[index];
        ((pd_phy_config_t *)pdAppInstance->pdConfigParam->phyConfig)->i2cSrcClock =
            HW_I2CGetFreq(((pd_phy_config_t *)pdAppInstance->pdConfigParam->phyConfig)->i2cInstance);

        if (PD_InstanceInit(&pdAppInstance->pdHandle, PD_DpmDemoAppCallback, &callbackFunctions, pdAppInstance,
                            g_PortsConfigArray[index]) != kStatus_PD_Success)
        {
            PRINTF("pd port %d init fail\r\n", pdAppInstance->portNumber);
        }
        else
        {
            PD_GpioInit(pdAppInstance);
            PD_PowerBoardControlInit(pdAppInstance->portNumber, pdAppInstance->pdHandle,
                                     (hal_gpio_handle_t)(&pdAppInstance->gpioExtraSrcHandle[0]),
#if defined(PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT) && (PD_CONFIG_EXTERNAL_POWER_DETECTION_SUPPORT)
                                     (hal_gpio_handle_t)(&pdAppInstance->gpioExternalPowerDetectHandle[0]));
#else
                                     NULL);
#endif
#if (defined BOARD_PD_USB3_CROSS_SUPPORT) && (BOARD_PD_USB3_CROSS_SUPPORT)
            PD_PTN36502Init(pdAppInstance);
#endif

            /* the follow many settings are only for passing compliance, not the actual product logic */
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
            /* self alert */
            pdAppInstance->selfAlert.alertValue = 0u;
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
            /* alternate mode (VDM) */
            pdAppInstance->selfVdmIdentity.idHeaderVDO.vdoValue                      = 0;
            pdAppInstance->selfVdmIdentity.idHeaderVDO.bitFields.modalOperateSupport = 1;
            pdAppInstance->selfVdmIdentity.idHeaderVDO.bitFields.connectorType = kConnectorType_UsbTypecReceptacle;
#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
            pdAppInstance->selfVdmIdentity.idHeaderVDO.bitFields.productTypeDFP = 0; /* Udefined */
#endif
            pdAppInstance->selfVdmIdentity.idHeaderVDO.bitFields.productTypeUFPOrCablePlug = 0; /* PDUSB Undefined */
            pdAppInstance->selfVdmIdentity.idHeaderVDO.bitFields.usbCommunicationCapableAsDevice = 0;
            pdAppInstance->selfVdmIdentity.idHeaderVDO.bitFields.usbCommunicationCapableAsHost   = 0;
            pdAppInstance->selfVdmIdentity.idHeaderVDO.bitFields.usbVendorID                     = PD_VENDOR_VID;
            pdAppInstance->selfVdmIdentity.pid                                                   = PD_CONFIG_PID;
            pdAppInstance->selfVdmIdentity.certStatVDO                                           = PD_CONFIG_XID;
            pdAppInstance->selfVdmIdentity.bcdDevice                                             = PD_CONFIG_BCD_DEVICE;
            pdAppInstance->selfVdmSVIDs = ((uint32_t)PD_VENDOR_VID << 16); /* only one SVID (display port) */
            pdAppInstance->selfVdmModes = PD_CONFIG_APP_MODE;              /* only one Mode */
            /* evaluate result */
            pdAppInstance->prSwapAccept    = 1;
            pdAppInstance->drSwapAccept    = 1;
            pdAppInstance->vconnSwapAccept = 1;
            /* other */
            pdAppInstance->selfHasEnterAlernateMode = 0;
#if (defined PD_CONFIG_PHY_LOW_POWER_LEVEL) && (PD_CONFIG_PHY_LOW_POWER_LEVEL)
            pdAppInstance->lowPowerEnable = 0;
#endif
            PRINTF("pd port %d init success\r\n", pdAppInstance->portNumber);
        }
    }
}

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
    }

#if (defined BOARD_PD_SW_INPUT_SUPPORT) && (BOARD_PD_SW_INPUT_SUPPORT)
    for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        if (g_PDAppInstanceArray[index]->pdHandle != NULL)
        {
            PD_Demo1msIsrProcessSW(g_PDAppInstanceArray[index]);
        }
    }
#endif
    g_SoftTimerCount++;
}

#if (defined PD_CONFIG_PHY_LOW_POWER_LEVEL) && (PD_CONFIG_PHY_LOW_POWER_LEVEL)
/* Return 0 - all ports are in low power state, Non-zero - at least one port is not in low power state. */
uint8_t PD_AppGetStackStatus(void)
{
    uint8_t index;
    uint8_t portState;
    uint8_t retVal = 0;

    for (index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        PD_Control(g_PDAppInstanceArray[index]->pdHandle, PD_CONTROL_GET_PD_LOW_POWER_STATE, &portState);
        if ((portState == 0) || (g_PDAppInstanceArray[index]->lowPowerEnable == 0u))
        {
            /* Not in low power state. */
            retVal++;
        }
    }

    if (retVal == 0)
    {
        /* Wait TX buffer empty */
        if (DbgConsole_Flush() != kStatus_Success)
        {
            /* TX buffer is not empty, cannot enter low power state. */
            retVal = 1;
        }
    }

    /* If the number of active port is greater than or equal to one, the processor will not enter low power state. */
    return retVal;
}
#endif

#if (defined PD_CONFIG_COMMON_TASK) && (PD_CONFIG_COMMON_TASK)
void PD_PortsTask(void *arg)
{
    while (1)
    {
        PD_Task();
    }
}
#else
void PD_PortTask(void *arg)
{
    pd_app_t *pdAppInstance = (pd_app_t *)arg;
    while (1)
    {
        PD_InstanceTask(pdAppInstance->pdHandle);
    }
}
#endif

static void pd_demo_task(void *arg)
{
#if (defined PD_CONFIG_PHY_LOW_POWER_LEVEL) && (PD_CONFIG_PHY_LOW_POWER_LEVEL)
    HW_LPTimerInit();
#endif
    /* @TEST_ANCHOR */
    PD_AppTimerInit();
    PD_AppInit();
    PD_DemoInit();

#if (defined PD_CONFIG_COMMON_TASK) && (PD_CONFIG_COMMON_TASK)
    if (xTaskCreate(PD_PortsTask, "PD", (2048u + 512u) / sizeof(portSTACK_TYPE), NULL, 5, NULL) != pdPASS)
    {
        PRINTF("create task error\r\n");
    }
#else
    for (uint8_t index = 0; index < PD_DEMO_PORTS_COUNT; ++index)
    {
        if (xTaskCreate(PD_PortTask, "port", (2048u + 512u) / sizeof(portSTACK_TYPE), g_PDAppInstanceArray[index], 5,
                        NULL) != pdPASS)
        {
            PRINTF("create task error\r\n");
        }
    }
#endif

    if (xTaskCreate(pd_DemoConsoleReadFun, "console", (512u) / sizeof(portSTACK_TYPE), NULL, 3, NULL) != pdPASS)
    {
        PRINTF("create task error\r\n");
    }

    while (1)
    {
        PD_DemoTaskFun();
    }
}

int main(void)
{
    BOARD_InitHardware();

    if (xTaskCreate(pd_demo_task, "demo", (1024u + 512u) / sizeof(portSTACK_TYPE), NULL, 4, NULL) != pdPASS)
    {
        PRINTF("create demo task error\r\n");
    }

    vTaskStartScheduler();

    while (1)
    {
        ;
    }
}
