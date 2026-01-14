/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PD_TIMER_H__
#define __PD_TIMER_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define mSec(MSEC) (MSEC)

typedef enum tTimer
{
    _tStartTimer           = 0,
    tSenderResponseTimer   = 0,
    tSourceCapabilityTimer = 1,
    tSinkWaitCapTimer      = 2,
    tSinkRequestTimer      = 3,
    tDrSwapWaitTimer       = 4,
    tPrSwapWaitTimer       = 5,
    tPSTransitionTimer     = 6,
    tPSSourceOffTimer      = 7,
    tPSSourceOnTimer       = 8,
    tNoResponseTimer = 9, /* The NoResponseTimer is used by the Policy Engine in a Source or Sink to determine that its
                            Port Partner is not responding after a Hard Reset. */
    tSwapSourceStartTimer = 10, /* pr swap */
    tPSHardResetTimer     = 11, /* source hard_reset */
    tVconnOnTimer         = 12, /* vconn swap */
    tVDMResponseTimer     = 13,
    tVDMModeEntryTimer    = 14,
    tVDMModeExitTimer     = 15,
    tSinkTxTimer          = 16,

    tSrcRecoverTimer            = 17, /* hard_reset tSrcRecover */
    timrPsmRdyEvalDelayTimer    = 18,
    tMsgHardResetCompleteTimer  = 19, /* MSG */
    tSinkPPSPeriodicTimer       = 20,
    tSourcePPSCommTimer         = 21,
    _tMaxPSMTimer               = tMsgHardResetCompleteTimer,  /* The maximum PSM timer */
    tAMETimeoutTimer            = 22,                          /* T_AME_TIMEOUT */
    _tMaxDpmTimer               = tAMETimeoutTimer,            /* The maximum port timer */
    tPDDebounceTimer            = 23,                          /* TypeC */
    tPDDebounce2Timer           = 24,                          /* TypeC */
    tCCDebounceTimer            = 25,                          /* TypeC */
    tCCDebounceTimer1           = 26,                          /* TypeC */
    tCCDebounceTimer2           = 27,                          /* TypeC */
    tDRPToggleTimer             = 28,                          /* TypeC */
    tTypeCVbusMinDischargeTimer = 29,                          /* TypeC */
    tTypeCVbusMaxDischargeTimer = 30,                          /* TypeC */
    _tMaxTypeCTimer             = tTypeCVbusMaxDischargeTimer, /* The maximum TypeC timer */
    tVDMBusyTimer               = 31,
    tDelayTimer                 = 32, /* tSrcRecover, T_SRC_TRANSITION (DPM), T_ERROR_RECOVERY (CONNECT), */
    timrCCFilter                = 33,

    tDRPTryWaitTimer             = 34,
    tDRPTryTimer                 = 35,
    tTryTimeoutTimer             = 36,
    tBISTContModeTimer           = 37,
    tFRSwapSignalTimer           = 38,
    tVBUSONTimer                 = 39,
    timrChunkSenderResponseTimer = 40,
    timrChunkSenderRequestTimer  = 41,
    timrMsgSendWaitResultTimer   = 42,
    timrFRSwapWaitPowerStable    = 43,
    tDiscoverIdentityTimer       = 44,
    tDpmComandRetryTimer         = 45,
    tTimerCount                  = 46,
    _tEnd                        = tTimerCount - 1,

/* don't implement yet */
#if 0
    tUFPDebounceTimer = 35,
    tUSBSuspendTimer = 36, /* USB */
    tUSBTimer = 37,        /* USB */
    tHciSpDebounceTimer = 38,
    tHostifWakeupTimer = 39,
#endif
} tTimer_t;

#define PD_MAX_TIMER_COUNT (46U)

#define T_FRSWAP_WAIT_POWER_STABLE (50U)
#define T_WAIT_SEND_RESULT         mSec(20)
#define T_PPS_TIMEOUT              mSec(15000) /* 15s */
#define T_PPS_REQUEST              mSec(10000) /* 10s */
#define T_HARD_RESET_COMPLETE      mSec(5)     /* 5 maximum */
#define T_UFP_CONNECT_DEBOUNCE     mSec(15)    /* 15 maximum */
#define T_SWAP_SOURCE_START        mSec(25)    /* 20 minimum, so we choose 25 to be safe. */
#define T_SRC_TRANSITION           mSec(30)    /* 25 to 35, so we choose 30 to be safe. */
#define T_SENDER_RESPONSE          mSec(26)    /* 24 to 30, so we choose 26 to be safe. */
#define T_VDM_SENDER_RESPONSE      mSec(27)    /* 24 to 30, so we choose 27 to be safe. */
#define T_CHUNK_SENDER_RESPONSE    mSec(25)    /* 24 to 30, so we choose 25 to be safe. */
#define T_CHUNK_SENDER_REQUEST     mSec(25)    /* 24 to 30, so we choose 25 to be safe. */
#define T_PS_HARD_RESET            mSec(26)    /* 25 to 35, so we choose 26 to be safe. */
#define T_SAFE0V_MAX               mSec(650)   /* 0 ~ 650 */
#define T_ERROR_RECOVERY           mSec(30)    /* 25 minimum, so we choose 30 to be safe. */
#define T_BIST_CONT_MODE           mSec(45)    /* 30 to 60, so we choose 45 to be safe. */
#define T_DISCOVER_IDENTITY        mSec(45)    /* 40 to 50, so we choose 45 to be safe. */
#define T_VDM_WAIT_MODE_ENTRY      mSec(45)    /* 40 to 50, so we choose 45 to be safe. */
#define T_VDM_WAIT_MODE_EXIT       mSec(45)    /* 40 to 50, so we choose 45 to be safe. */
#define T_VDM_BUSY                 mSec(55)    /* 50 minumum */
#define T_VCONN_SOURCE_ON          mSec(100)   /* 100 maximum */
#define T_VCONN_PA_SOURCE_ON       mSec(100)   /* 100 maximum */
#define T_SINK_REQUEST             mSec(100)   /* 100 to no maximum */
#define T_DRSWAP_WAIT              mSec(100)   /* 100 to no maximum */
#define T_DRSWAP_WAIT_ALT_MODE     mSec(200)   /* 200, wait exit mode then do dr swap */
#define T_PRSWAP_WAIT              mSec(100)   /* 100 to no maximum */
#define T_PSM_RDY_EVAL_DELAY       mSec(120)   /* Arbitary - not a USBPD timer */
#if (defined PD_CONFIG_TRY_SRC_SUPPORT) && (PD_CONFIG_TRY_SRC_SUPPORT)
#define T_SINK_WAIT_CAP mSec(311) /* 310 to 620 */
#else
#define T_SINK_WAIT_CAP mSec(400) /* 310 to 620 */
#endif
#define T_PS_SOURCE_ON           mSec(400)   /* 390 to 480 */
#define T_PS_TRANSITION          mSec(500)   /* 450 to 550 */
#define T_SWAP_RECOVER           mSec(500)   /* 500 to 1000 */
#define T_SRC_RECOVER            mSec(700)   /* 660 to 1000 */
#define T_PS_SOURCE_OFF          mSec(800)   /* 750 to 920 */
#define T_AME_TIMEOUT            mSec(900)   /* 1000 maximum */
#define T_HARD_RESET_MAX         mSec(2000)  /* tSafe0V (max) + tSrcRecover (max) + tSrcTurnOn (max) */
#define T_NO_RESPONSE            mSec(5000)  /* 4500 to 5500 */
#define T_HOSTIF_WAKEUP          mSec(150)   /* 4500 to 5500 */
#define T_USBFET2_OVERLAP        mSec(30)    /* */
#define T_USBFET_TRANSITION      mSec(1)     /* */
#define T_AUTO_PR_SWAP_TEST      mSec(45000) /* */
#define T_PSM_SRC_RDY_EVAL_DELAY mSec(100)   /* 100 to no maximum */
#define T_PSM_SNK_RDY_EVAL_DELAY mSec(115)   /* 100 to no maximum */

#define T_VCONN_STABLE    mSec(50)  /* 50ms minimum */
#define T_SEND_SOURCE_CAP mSec(150) /* 100 to 200 */
#define T_SRC_NEW_POWER   mSec(80)  /* 285 ms max */

#define T_MIN_VBUS_DISCHARGE mSec(14)  /* corrosponds to vbus_present debounce time (tccDebounce) */
#define T_MAX_VBUS_DISCHARGE mSec(650) /* tVBUSOFF */
#define T_MAX_VBUS_ON        mSec(275) /* tVBUSON */

#define T_SINK_TX           mSec(18) /* 20ms max */
#define T_DPM_COMMAND_RETRY mSec(5)

#define T_CC_FILTER_MAX mSec(1)

/*******************************************************************************
 * API
 ******************************************************************************/

void PD_TimerInit(pd_handle pdHandle);
uint8_t PD_TimerCheckInvalidOrTimeOut(pd_handle pdHandle, tTimer_t timrName);
uint8_t PD_TimerCheckValidTimeOut(pd_handle pdHandle, tTimer_t timrName);
uint8_t PD_TimerCheckStarted(pd_handle pdHandle, tTimer_t timrName);
pd_status_t PD_TimerStart(pd_handle pdHandle, tTimer_t timrName, uint16_t timrTime);
pd_status_t PD_TimerClear(pd_handle pdHandle, tTimer_t timrName);
void PD_TimerCancelAllTimers(pd_handle pdHandle, tTimer_t timrBegin, tTimer_t timrEnd);
uint8_t PD_TimerBusy(pd_handle pdHandle);

#endif
