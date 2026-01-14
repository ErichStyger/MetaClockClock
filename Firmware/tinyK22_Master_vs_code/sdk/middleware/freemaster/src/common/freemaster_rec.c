/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2018-2021, 2024-2025 NXP
 *
 * License: NXP LA_OPT_Online Code Hosting NXP_Software_License
 *
 * NXP Proprietary. This software is owned or controlled by NXP and may
 * only be used strictly in accordance with the applicable license terms.
 * By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that
 * you have read, and that you agree to comply with and are bound by,
 * such license terms.  If you do not agree to be bound by the applicable
 * license terms, then you may not retain, install, activate or otherwise
 * use the software.
 *
 * FreeMASTER Communication Driver - Recorder implementation
 */

#include "freemaster.h"
#include "freemaster_private.h"
#include "freemaster_protocol.h"
#include "freemaster_utils.h"

#if FMSTR_USE_RECORDER > 0 && FMSTR_DISABLE == 0

#include "freemaster_rec.h"

/********************************************************
 *  local macros definition
 ********************************************************/

/* Define Protocol operations*/
#define FMSTR_REC_PRTCLSET_OP_CFGMEM 0x01U /* Set number of recorder variables, points and post-trigger */
#define FMSTR_REC_PRTCLSET_OP_CFGVAR 0x02U /* Setup address, size, and threshold detection of one recorder variable */
#define FMSTR_REC_PRTCLSET_OP_START  0x03U /* Start recorder if not yet running */
#define FMSTR_REC_PRTCLSET_OP_STOP   0x04U /* Stop recorder immediately */

#define FMSTR_REC_PRTCLGET_OP_DESCR  0x81U /* String description of recorder sampling point etc. */
#define FMSTR_REC_PRTCLGET_OP_LIMITS 0x82U /* Get maximum number of recorder variables, and size of the recorder memory. */
#define FMSTR_REC_PRTCLGET_OP_INFO   0x83U /* Get recorder base address, number of recorded variables, and other info. */
#define FMSTR_REC_PRTCLGET_OP_STATUS 0x84U /* Get current recorder status (running/stopped etc.) */

/* recorder structures alignment */
#ifndef FMSTR_REC_STRUCT_ALIGN
#define FMSTR_REC_STRUCT_ALIGN sizeof(void *)
#endif

/********************************************************
 *  local types definition
 ********************************************************/

/* Forward declaration of needed structures */
struct FMSTR_REC_VAR_DATA_S;

/* trigger threshold level (1,2,4 or 8 bytes) */
typedef union
{
    FMSTR_U8 raw[8];
    FMSTR_U8 u8;
    FMSTR_S8 s8;
    FMSTR_U16 u16;
    FMSTR_S16 s16;
    FMSTR_U32 u32;
    FMSTR_S32 s32;
    FMSTR_U64 u64;
    FMSTR_S64 s64;
#if FMSTR_REC_FLOAT_TRIG > 0
    FMSTR_FLOAT fp;
    FMSTR_DOUBLE dbl;
#endif
} FMSTR_REC_THRESHOLD;

/* recorder runtime flags */
typedef volatile union
{
    FMSTR_FLAGS all; /* all flags in one field */

    struct
    {
        unsigned isConfigured : 1;  /* recorder is configured */
        unsigned hasData : 1;       /* cleared when configured, set when at least one point is sampled */
        unsigned isRunning : 1;     /* recorder is running */
        unsigned isVirginCycle : 1; /* virgin cycle of the circular buffer in-progress */
        unsigned isStopping : 1;    /* trigger activated, sample countdown */
    } flg;

} FMSTR_REC_FLAGS;

/* pointer to FMSTR_REC_VAR_DATA (potentially far on some platforms) */
FMSTR_TYPEDEF_LPTR(struct FMSTR_REC_VAR_DATA_S, FMSTR_LP_REC_VAR_DATA);

/* compare functions prototype */
typedef FMSTR_BOOL (*FMSTR_PCOMPAREFUNC)(FMSTR_LP_REC_VAR_DATA varData);

/* read variable functions prototype */
typedef void (*FMSTR_PREADFUNC)(FMSTR_ADDR destAddr, FMSTR_ADDR srcAddr);

/* Recorder internal description of one variable */
typedef struct FMSTR_REC_VAR_DATA_S
{
    FMSTR_REC_VAR cfg;                /* variable configuration */
    FMSTR_REC_THRESHOLD thresholdVal; /* trigger threshold value if used */
    FMSTR_PCOMPAREFUNC compareFunc;   /* pointer to trigger compare function if used */
    FMSTR_PREADFUNC readFunc;         /* pointer to variable read (value copy) function */

    FMSTR_BOOL trgLastState;          /* last trigger comparison state for edge detection if used */
} FMSTR_REC_VAR_DATA;

/* runtime variables  */
typedef struct
{
	FMSTR_LP_REC_VAR_DATA varDescr; /* table with recorder variables description. */
    FMSTR_ADDR buffAddr;          /* Address of buffer for recorded variables. */
    FMSTR_SIZE buffSize;          /* Size of buffer for recorded variables. */
    FMSTR_SIZE totalSmplsCnt;     /* recorder total samples count */
    FMSTR_SIZE timeDivCtr;        /* recorder "clock" divisor counter */
    FMSTR_SIZE postTrigger;       /* post-trigger value computed from user's pre-trigger */
    FMSTR_SIZE stopRecCountDown;  /* post-trigger countdown counter */
    FMSTR_ADDR writePtr;          /* write pointer in recorder buffer */
    FMSTR_ADDR endBuffPtr;        /* pointer to end of active recorder buffer */
    FMSTR_SIZE pointSize;         /* size of one variable shot */
    FMSTR_SIZE pointVarCount;     /* number of variables recorded (trigger-only vars excluded) */
    FMSTR_REC_FLAGS flags;        /* recorder flags */
    FMSTR_REC_CFG config;         /* original recorder configuration */
} FMSTR_REC;

/* pointer to FMSTR_REC (potentially far on some platforms) */
FMSTR_TYPEDEF_LPTR(FMSTR_REC, FMSTR_LP_REC);

/* Map of FreeMASTER recorder instance in memory */
/*********************************************/
/*********************************************/
/**               FMSTR_REC                 **/
/**  ************************************** **/
/**  *         FMSTR_REC_CFG              * **/
/**  *                                    * **/
/**  *                                    * **/
/**  ************************************** **/
/**                                         **/
/*********************************************/
/**                                         **/
/**       FMSTR_REC_VAR_DATA - arrays       **/
/**                                         **/
/*********************************************/
/**                                         **/
/**                                         **/
/**                                         **/
/**  FreeMASTER recorder samples buffer     **/
/**                                         **/
/**                                         **/
/**                                         **/
/*********************************************/

/********************************************************
 *  local static functions declarations
 ********************************************************/

static FMSTR_REC_BUFF *_FMSTR_GetRecorderBufferByRecIx(FMSTR_INDEX recIndex);
static FMSTR_LP_REC _FMSTR_GetRecorderByRecIx(FMSTR_INDEX recIndex);

static FMSTR_U8 _FMSTR_CheckConfiguration(FMSTR_LP_REC recorder);
static FMSTR_U8 _FMSTR_RecMemCfg(FMSTR_LP_REC recorder, FMSTR_INDEX recIndex, FMSTR_REC_CFG *recCfg);
static FMSTR_U8 _FMSTR_RecVarCfg(FMSTR_LP_REC recorder, FMSTR_INDEX recVarIx, FMSTR_REC_VAR *recVarCfg);

static FMSTR_U8 _FMSTR_StartRec(FMSTR_LP_REC recorder);
static FMSTR_U8 _FMSTR_TriggerRec(FMSTR_LP_REC recorder);
static FMSTR_U8 _FMSTR_AbortRec(FMSTR_LP_REC recorder);

static FMSTR_BPTR _FMSTR_SetRecCmd_CFGMEM(FMSTR_BPTR msgBuffIO,
		                                  FMSTR_SIZE opLen,
		                                  FMSTR_LP_REC recorder,
		                                  FMSTR_INDEX recIndex,
		                                  FMSTR_U8 *retStatus);

static FMSTR_BPTR _FMSTR_SetRecCmd_CFGVAR(FMSTR_BPTR msgBuffIO,
                                          FMSTR_SIZE opLen,
                                          FMSTR_LP_REC recorder,
                                          FMSTR_U8 *retStatus);

static FMSTR_U8 _FMSTR_CalcRecStatus(FMSTR_REC_FLAGS recFlags);

static FMSTR_BOOL _FMSTR_Compare8S(FMSTR_LP_REC_VAR_DATA varData);
static FMSTR_BOOL _FMSTR_Compare8U(FMSTR_LP_REC_VAR_DATA varData);
static FMSTR_BOOL _FMSTR_Compare16S(FMSTR_LP_REC_VAR_DATA varData);
static FMSTR_BOOL _FMSTR_Compare16U(FMSTR_LP_REC_VAR_DATA varData);
static FMSTR_BOOL _FMSTR_Compare32S(FMSTR_LP_REC_VAR_DATA varData);
static FMSTR_BOOL _FMSTR_Compare32U(FMSTR_LP_REC_VAR_DATA varData);
static FMSTR_BOOL _FMSTR_Compare64S(FMSTR_LP_REC_VAR_DATA varData);
static FMSTR_BOOL _FMSTR_Compare64U(FMSTR_LP_REC_VAR_DATA varData);
#if FMSTR_REC_FLOAT_TRIG > 0
static FMSTR_BOOL _FMSTR_CompareFloat(FMSTR_LP_REC_VAR_DATA varData);
static FMSTR_BOOL _FMSTR_CompareDouble(FMSTR_LP_REC_VAR_DATA varData);
#endif

static void _FMSTR_Recorder2(FMSTR_LP_REC recorder);

/********************************************************
 *  static variables
 ********************************************************/

/* Recorder buffers */
static FMSTR_REC_BUFF fmstr_recBuffs[FMSTR_USE_RECORDER];

#if FMSTR_REC_BUFF_SIZE > 0
/* statically allocated recorder buffer */
static FMSTR_U8 fmstr_pOwnRecBuffer[FMSTR_REC_BUFF_SIZE];
#endif

/******************************************************************************
 *
 * @brief    Recorder Initialization
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_InitRec(void)
{
    FMSTR_BOOL ok = FMSTR_TRUE;

#if FMSTR_REC_BUFF_SIZE > 0
    FMSTR_REC_BUFF recBuffCfg;
#endif

    /* Initialize array of recorder buffers descriptors */
    FMSTR_MemSet(fmstr_recBuffs, 0, sizeof(fmstr_recBuffs));

#if FMSTR_REC_BUFF_SIZE > 0
    /* When FMSTR_REC_BUFF_SIZE is defined, create the default buffer */
    recBuffCfg.addr          = FMSTR_CAST_PTR_TO_ADDR(fmstr_pOwnRecBuffer);
    recBuffCfg.size          = FMSTR_REC_BUFF_SIZE;
    recBuffCfg.basePeriod_ns = FMSTR_REC_TIMEBASE;
    
    /* Coverity: Intentional assignment of constant string */
    /* coverity[misra_c_2012_rule_7_4_violation:FALSE] */
    recBuffCfg.name          = "Default Recorder";

    /* and create the recorder #0 automatically */
    ok = FMSTR_RecorderCreate(0, &recBuffCfg);
#endif

    return ok;
}

/******************************************************************************
 *
 * @brief    API: Set up the recorder buffer
 *
 * @param    recIndex - index of recorder to set up
 * @param    recBuffer - user buffer pointer
 * @param    recBuffSize - buffer size
 *
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_RecorderCreate(FMSTR_INDEX recIndex, FMSTR_REC_BUFF *buffCfg)
{
    FMSTR_REC_BUFF *recBuff;
    FMSTR_SIZE alignment;

    /* Check the input parameters */
    if (recIndex >= (FMSTR_INDEX)FMSTR_USE_RECORDER)
    {
        return FMSTR_FALSE;
    }

    FMSTR_ASSERT_RETURN(buffCfg != NULL, FMSTR_FALSE);

    if (buffCfg->addr == NULL)
    {
        return FMSTR_FALSE;
    }

    /* Check for some minimal value of buffer */
    if (buffCfg->size < (sizeof(FMSTR_REC) + 128U))
    {
        return FMSTR_FALSE;
    }

    /* Check that the name is already included */
    if (buffCfg->name == NULL)
    {
        return FMSTR_FALSE;
    }

    if ((recBuff = _FMSTR_GetRecorderBufferByRecIx(recIndex)) == NULL)
    {
        return FMSTR_FALSE;
    }

    /* The buffer should not yet be used */
    if (FMSTR_ADDR_VALID(recBuff->addr) != FMSTR_FALSE)
    {
        return FMSTR_FALSE;
    }

    /* Set up the recorder buffer */
    *recBuff = *buffCfg;

    /* Clean up the buffer */
    FMSTR_MemSet(recBuff->addr, 0, recBuff->size);

    /* Align buffer base address */
    alignment = FMSTR_GetAlignmentCorrection(recBuff->addr, FMSTR_REC_STRUCT_ALIGN);

    /* Final sanity check to prevent size wrapping */
    if(recBuff->size < alignment)
    {
        return FMSTR_FALSE;
    }

    /* All good, go ahead with the aligned memory block */
    recBuff->addr += alignment;
    recBuff->size -= alignment;
    return FMSTR_TRUE;
}

/******************************************************************************
 *
 * @brief    API: Change the recorder time base information.
 *
 * @param    recIndex - index of recorder
 * @param    timeBase_ns - new time base to be assigned to recorder, in nanoseconds
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_RecorderSetTimeBase(FMSTR_INDEX recIndex, FMSTR_U32 timeBase_ns)
{
    FMSTR_REC_BUFF *recBuff;

    FMSTR_ASSERT_RETURN(recIndex < (FMSTR_INDEX)FMSTR_USE_RECORDER, FMSTR_FALSE);

    recBuff = _FMSTR_GetRecorderBufferByRecIx(recIndex);
    FMSTR_ASSERT_RETURN(recBuff != NULL, FMSTR_FALSE);

    recBuff->basePeriod_ns = timeBase_ns;
    return FMSTR_TRUE;
}

/******************************************************************************
 *
 * @brief    API: Set up the recorder configuration
 *
 * @param    recIndex - index of recorder
 * @param    recCfg - pointer to recorder configuration
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_RecorderConfigure(FMSTR_INDEX recIndex, FMSTR_REC_CFG *recCfg)
{
    FMSTR_LP_REC recorder;

    FMSTR_ASSERT_RETURN(recIndex < (FMSTR_INDEX)FMSTR_USE_RECORDER, FMSTR_FALSE);
    FMSTR_ASSERT_RETURN(recCfg != NULL, FMSTR_FALSE);

    if ((recorder = _FMSTR_GetRecorderByRecIx(recIndex)) == NULL)
    {
        return FMSTR_FALSE;
    }

    return (FMSTR_BOOL)((_FMSTR_RecMemCfg(recorder, recIndex, recCfg) == FMSTR_STS_OK) ? FMSTR_TRUE : FMSTR_FALSE);
}

/******************************************************************************
 *
 * @brief    Set up the recorder configuration (internal version)
 *
 * @param    recorder - recorder structure
 * @param    recCfg - recorder configuration
 *
 * @return   FMSTR_STC_xxx status code
 *
 ******************************************************************************/

static FMSTR_U8 _FMSTR_RecMemCfg(FMSTR_LP_REC recorder, FMSTR_INDEX recIndex, FMSTR_REC_CFG *recCfg)
{
    /* Cannot use time division */
#if FMSTR_FASTREC_NO_TIME_DIVISION
    if (recCfg->timeDiv > 0)
    {
        return FMSTR_STC_INVCONF;
    }
#endif

    FMSTR_REC_BUFF *recBuff = _FMSTR_GetRecorderBufferByRecIx(recIndex);
    FMSTR_ADDR dynAddr;
    FMSTR_INDEX spaceLeft;

    /* Assert that recBuff->addr==recorder and that recBuff->size is large enough for recCfg->varCount and recorder
     * structure itself */
    FMSTR_ASSERT(recBuff != NULL);
    FMSTR_ASSERT(recBuff->addr == FMSTR_CAST_PTR_TO_ADDR(recorder));
    FMSTR_ASSERT(recBuff->size >= (FMSTR_SIZE)(sizeof(FMSTR_REC) + sizeof(FMSTR_REC_VAR_DATA) * (FMSTR_SIZE)recCfg->varCount));

    /* Make sure the recorder is stopped first before clearing the whole structure below
      (in case the FMSTR_Recorder is called from ISR, it could be invoked in the middle of clearing) */
    recorder->flags.all = 0U;

    /* Reset the recorder context */
    FMSTR_MemSet(recorder, 0, sizeof(FMSTR_REC));

    /* Copy the config structure */
    recorder->config = *recCfg;

    /* Initialize the variable configuration array (the array follows the recorder structure) */
    dynAddr = FMSTR_CAST_PTR_TO_ADDR(recorder + 1);
    dynAddr += FMSTR_GetAlignmentCorrection(dynAddr, FMSTR_REC_STRUCT_ALIGN);
    recorder->varDescr = (FMSTR_LP_REC_VAR_DATA)FMSTR_CAST_ADDR_TO_PTR(dynAddr);
    FMSTR_MemSet(recorder->varDescr, 0, sizeof(FMSTR_REC_VAR_DATA) * (FMSTR_SIZE)recCfg->varCount);

    /* Data sampling area follows the variable array */
    dynAddr = FMSTR_CAST_PTR_TO_ADDR(recorder->varDescr + recCfg->varCount);
    dynAddr += FMSTR_GetAlignmentCorrection(dynAddr, FMSTR_REC_STRUCT_ALIGN);
    recorder->buffAddr = dynAddr;

    /*...and spans to the end of the recorder memory */
    spaceLeft          = dynAddr - FMSTR_CAST_PTR_TO_ADDR(recorder);
    
    /* Coverity: Intentional. The computations above make sure the expression is positive (total 
       buffer size minus the space left). No risk of misinterpretation of the subtraction result. */
    /* coverity[cert_int31_c_violation:FALSE] */
    recorder->buffSize = recBuff->size - (FMSTR_SIZE)spaceLeft;

    return FMSTR_STS_OK;
}

/******************************************************************************
 *
 * @brief    Handling SETREC Memory configuration command (CFGMEM op_code)
 *
 * @param    msgBuffIO   - original command (in) and response buffer (out)
 * @param    opLen       - data length
 * @param    recorder    - recorder structure
 * @param    retStatus   - pointer to return status variable
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response output finished (except checksum)
 *
 ******************************************************************************/

static FMSTR_BPTR _FMSTR_SetRecCmd_CFGMEM(
    FMSTR_BPTR msgBuffIO, FMSTR_SIZE opLen, FMSTR_LP_REC recorder, FMSTR_INDEX recIndex, FMSTR_U8 *retStatus)
{
    FMSTR_REC_CFG recCfg;
    FMSTR_BPTR tmpMsgBuffIO = msgBuffIO;

    FMSTR_MemSet(&recCfg, 0, sizeof(recCfg));

    /* Get the configuration from input buffer */
    msgBuffIO = FMSTR_ValueFromBuffer8(&recCfg.varCount, msgBuffIO);
    msgBuffIO = FMSTR_SizeFromBuffer(&recCfg.totalSmps, msgBuffIO);
    msgBuffIO = FMSTR_SizeFromBuffer(&recCfg.preTrigger, msgBuffIO);
    msgBuffIO = FMSTR_SizeFromBuffer(&recCfg.timeDiv, msgBuffIO);

    /* Check the size if it's valid */
    if (msgBuffIO != (tmpMsgBuffIO + opLen))
    {
        *retStatus = FMSTR_STC_INVSIZE;
        return msgBuffIO;
    }

    /* Configure the Recorder */
    *retStatus = _FMSTR_RecMemCfg(recorder, recIndex, &recCfg);

    return msgBuffIO;
}

/******************************************************************************
 *
 * @brief    API: Set up the recorder variable configuration
 *
 * @param    recIndex - index of recorder
 * @param    recCfg - pointer to recorder configuration
 *
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_RecorderAddVariable(FMSTR_INDEX recIndex, FMSTR_INDEX recVarIx, FMSTR_REC_VAR *recVarCfg)
{
    FMSTR_LP_REC recorder;

    if ((recorder = _FMSTR_GetRecorderByRecIx(recIndex)) == NULL)
    {
        return FMSTR_FALSE;
    }

    return (FMSTR_BOOL)((_FMSTR_RecVarCfg(recorder, recVarIx, recVarCfg) == FMSTR_STS_OK) ? FMSTR_TRUE : FMSTR_FALSE);
}

/******************************************************************************
 *
 * @brief    Set up the recorder variable configuration (internal version)
 *
 * @param    recIndex - index of recorder
 * @param    recVarIx - index of variable
 * @param    recVarCfg - variable configuration
 *
 * @return   FMSTR_STC_xxx status code
 *
 ******************************************************************************/

static FMSTR_U8 _FMSTR_RecVarCfg(FMSTR_LP_REC recorder, FMSTR_INDEX recVarIx, FMSTR_REC_VAR *recVarCfg)
{
    FMSTR_LP_REC_VAR_DATA varDescr;
    FMSTR_PCOMPAREFUNC compareFunc = NULL;
    FMSTR_PREADFUNC readFunc = NULL;

    FMSTR_ASSERT_RETURN(recVarIx >= 0 && ((FMSTR_SIZE)recVarIx) < recorder->config.varCount, FMSTR_STC_INSTERR);

    /* Cannot use level mode or faling edge mode for recorder variable */
#if FMSTR_FASTREC_RISING_EDGE_TRG_ONLY
    if ((recVarCfg->triggerMode & FMSTR_REC_TRG_F_LEVEL) != 0U || (recVarCfg->triggerMode & FMSTR_REC_TRG_F_BELOW) != 0U)
    {
        return FMSTR_STC_INVCONF;
    }
#endif

    /* Cannot use level mode or rising edge mode for recorder variable */
#if FMSTR_FASTREC_FALLING_EDGE_TRG_ONLY
    if ((recVarCfg->triggerMode & FMSTR_REC_TRG_F_LEVEL) != 0U || (recVarCfg->triggerMode & FMSTR_REC_TRG_F_ABOVE) != 0U)
    {
        return FMSTR_STC_INVCONF;
    }
#endif

    /* Cannot change variable configuration when recorder is already configured (or even running) */
    if (recorder->flags.all != 0U)
    {
        return FMSTR_STC_SERVBUSY;
    }

    /* valid numeric variable sizes only */
    switch (recVarCfg->size)
    {
    case 1:
        readFunc = FMSTR_MemCpySrcAligned_8;
        break;
    case 2:
        readFunc = FMSTR_MemCpySrcAligned_16;
        break;
    case 4:
        readFunc = FMSTR_MemCpySrcAligned_32;
        break;
    case 8:
        readFunc = FMSTR_MemCpySrcAligned_64;
        break;
    default:
        return FMSTR_STC_INVSIZE;
        /* Coverity: Intentional unreachable break for misra only. */
        /* coverity[misra_c_2012_rule_2_1_violation:FALSE] */
        break;
    }

#if FMSTR_USE_TSA > 0 && FMSTR_USE_TSA_SAFETY > 0
    if (FMSTR_CheckTsaSpace(recVarCfg->addr, recVarCfg->size, FMSTR_FALSE) == FMSTR_FALSE)
    {
        return FMSTR_STC_EACCESS;
    }
#endif /* FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY */

    /* Is the variable set up for triggerring? */
    if ((recVarCfg->triggerMode & FMSTR_REC_TRG_TYPE_MASK) != 0U)
    {
        /* Is the threshold also variable? */
#if FMSTR_USE_TSA > 0 && FMSTR_USE_TSA_SAFETY > 0
        if ((recVarCfg->triggerMode & FMSTR_REC_TRG_F_VARTHR) != 0U)
        {
            if (FMSTR_CheckTsaSpace(recVarCfg->trgAddr, recVarCfg->size, FMSTR_FALSE) == FMSTR_FALSE)
            {
                return FMSTR_STC_EACCESS;
            }
        }
#endif /* FMSTR_USE_TSA && FMSTR_USE_TSA_SAFETY */

        /* get compare function */

        if ((recVarCfg->triggerMode & FMSTR_REC_TRG_TYPE_MASK) == FMSTR_REC_TRG_TYPE_FLOAT)
        {
#if FMSTR_REC_FLOAT_TRIG > 0
            switch (recVarCfg->size)
            {
            case 4:
                compareFunc = _FMSTR_CompareFloat;
                break;
            case 8:
                compareFunc = _FMSTR_CompareDouble;
                break;
            default:
                return FMSTR_STC_INVSIZE;
                /* Coverity: Intentional unreachable break for misra only. */
                /* coverity[misra_c_2012_rule_2_1_violation:FALSE] */
                break;
            }
#else
            return FMSTR_STC_FLOATDISABLED;
#endif
        }
        else
        {
            FMSTR_BOOL sign =
                (FMSTR_BOOL)((recVarCfg->triggerMode & FMSTR_REC_TRG_TYPE_MASK) == FMSTR_REC_TRG_TYPE_SINT);

            switch (recVarCfg->size)
            {
            case 1:
                compareFunc = sign != FMSTR_FALSE ? _FMSTR_Compare8S : _FMSTR_Compare8U;
                break;
            case 2:
                compareFunc = sign != FMSTR_FALSE ? _FMSTR_Compare16S : _FMSTR_Compare16U;
                break;
            case 4:
                compareFunc = sign != FMSTR_FALSE ? _FMSTR_Compare32S : _FMSTR_Compare32U;
                break;
            case 8:
                compareFunc = sign != FMSTR_FALSE ? _FMSTR_Compare64S : _FMSTR_Compare64U;
                break;
            default:
                return FMSTR_STC_INVSIZE;
                /* Coverity: Intentional unreachable break for misra only. */
                /* coverity[misra_c_2012_rule_2_1_violation:FALSE] */
                break;
            }
        }
    }

    varDescr = &recorder->varDescr[recVarIx];

    /* Store the variable configuration */
    varDescr->cfg         = *recVarCfg;
    varDescr->compareFunc = compareFunc;
    varDescr->readFunc    = readFunc;

    return FMSTR_STS_OK;
}

/******************************************************************************
 *
 * @brief    Handling SETREC Variable configuration command (CFGVAR op_code)
 *
 * @param    msgBuffIO   - original command (in) and response buffer (out)
 * @param    opLen       - data length
 * @param    retStatus   - pointer to return status variable
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response output finished (except checksum)
 *
 ******************************************************************************/

static FMSTR_BPTR _FMSTR_SetRecCmd_CFGVAR(FMSTR_BPTR msgBuffIO,
                                          FMSTR_SIZE opLen,
                                          FMSTR_LP_REC recorder,
                                          FMSTR_U8 *retStatus)
{
    FMSTR_REC_VAR recVarCfg;
    FMSTR_BPTR tmpMsgBuffIO = msgBuffIO;
    FMSTR_U8 recVarIx       = 0;

    FMSTR_MemSet(&recVarCfg, 0, sizeof(recVarCfg));

    /* Get the recorder variable configuration from input buffer */
    msgBuffIO = FMSTR_ValueFromBuffer8(&recVarIx, msgBuffIO);
    msgBuffIO = FMSTR_AddressFromBuffer(&recVarCfg.addr, msgBuffIO);
    msgBuffIO = FMSTR_ValueFromBuffer8(&recVarCfg.size, msgBuffIO);
    msgBuffIO = FMSTR_ValueFromBuffer8(&recVarCfg.triggerMode, msgBuffIO);

    /* Only valid variable can be configured */
    if (recVarIx >= recorder->config.varCount)
    {
        *retStatus = FMSTR_STC_INSTERR;
        return msgBuffIO;
    }

    /* Sanity check on variable size */
    if (recVarCfg.size > sizeof(FMSTR_REC_THRESHOLD))
    {
        *retStatus = FMSTR_STC_INVSIZE;
        return msgBuffIO;
    }

    /* Variable threshold? */
    if ((recVarCfg.triggerMode & FMSTR_REC_TRG_F_VARTHR) != 0U)
    {
        /* Yes, get the pointer to variable threshold variable */
        msgBuffIO = FMSTR_AddressFromBuffer(&recVarCfg.trgAddr, msgBuffIO);
    }
    else
    {
        /* Constant threshold. Get its address. */
        FMSTR_LP_U8 constThresholdPtr = (FMSTR_LP_U8)recorder->varDescr[recVarIx].thresholdVal.raw;

        switch (recVarCfg.triggerMode & FMSTR_REC_TRG_TYPE_MASK)
        {
            case FMSTR_REC_TRG_TYPE_FLOAT:
#if FMSTR_REC_FLOAT_TRIG > 0
                /* The Float is coded as a raw bytes in ULEB format */
                msgBuffIO = FMSTR_UlebDecode(msgBuffIO, constThresholdPtr, recVarCfg.size);
                break;
#else
                *retStatus = FMSTR_STC_FLOATDISABLED;
                return msgBuffIO;
#endif

            case FMSTR_REC_TRG_TYPE_SINT:
                msgBuffIO = FMSTR_SlebDecode(msgBuffIO, constThresholdPtr, recVarCfg.size);
                break;

            case FMSTR_REC_TRG_TYPE_UINT:
            default:
                msgBuffIO = FMSTR_UlebDecode(msgBuffIO, constThresholdPtr, recVarCfg.size);
                break;
        }

        /* Pointer to constant threshold value */
        recVarCfg.trgAddr = FMSTR_CAST_PTR_TO_ADDR(constThresholdPtr);
    }

    /* Secoded ULEBs should match the expected op_data length */
    if (msgBuffIO != (tmpMsgBuffIO + opLen))
    {
        *retStatus = FMSTR_STC_INVSIZE;
    }
    else
    {
        /* Configure the Recorder variable */
        *retStatus = _FMSTR_RecVarCfg(recorder, (FMSTR_INDEX)recVarIx, &recVarCfg);
    }
    return msgBuffIO;
}

/******************************************************************************
 *
 * @brief    Handling SETREC command
 *
 * @param    session - transport session
 * @param    msgBuffIO   - original command (in) and response buffer (out)
 * @param    inputLen    - data length of input data
 * @param    retStatus   - pointer to return status variable
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response output finished (except checksum)
 *
 ******************************************************************************/

FMSTR_BPTR FMSTR_SetRecCmd(FMSTR_SESSION * session, FMSTR_BPTR msgBuffIO, FMSTR_SIZE inputLen, FMSTR_U8 *retStatus)
{
    FMSTR_LP_REC recorder   = NULL;
    FMSTR_BPTR response   = msgBuffIO;
    FMSTR_U8 responseCode = FMSTR_STS_OK;
    FMSTR_U8 recIndex     = 0;

    /* Need at least one byte with recorder index */
    if (inputLen < 1U)
    {
        *retStatus = FMSTR_STC_INVSIZE;
        return response;
    }

    /* Get recorder index */
    msgBuffIO = FMSTR_ValueFromBuffer8(&recIndex, msgBuffIO);
    inputLen--;

    /* Valid recorder? */
    if ((recorder = _FMSTR_GetRecorderByRecIx((FMSTR_INDEX)recIndex)) == NULL)
    {
        *retStatus = FMSTR_STC_INSTERR;
        return response;
    }

    while (inputLen >= 2U && (responseCode == FMSTR_STS_OK))
    {
        FMSTR_U8 opCode, opLen;

#if FMSTR_SESSION_COUNT > 1
        /* Is feature locked by me */
        if(FMSTR_IsFeatureOwned(session, FMSTR_FEATURE_REC, recIndex) == FMSTR_FALSE)
        {
            *retStatus = FMSTR_STC_SERVBUSY;
            return response;
        }
#else
    FMSTR_UNUSED(session);
#endif

        /* Get Operation Code and data length */
        msgBuffIO = FMSTR_ValueFromBuffer8(&opCode, msgBuffIO);
        msgBuffIO = FMSTR_ValueFromBuffer8(&opLen, msgBuffIO);
        inputLen -= 2U;

        /* data would span beyond the total frame size */
        if (((FMSTR_SIZE)opLen) > inputLen)
        {
            *retStatus = FMSTR_STC_INVSIZE;
            return response;
        }

        switch (opCode)
        {
            /* Set number of recorder variables, recorder points and post-trigger points. */
            case FMSTR_REC_PRTCLSET_OP_CFGMEM:
                msgBuffIO = _FMSTR_SetRecCmd_CFGMEM(msgBuffIO, opLen, recorder, (FMSTR_INDEX)recIndex, &responseCode);
                break;

            /* Configure variable */
            case FMSTR_REC_PRTCLSET_OP_CFGVAR:
                msgBuffIO = _FMSTR_SetRecCmd_CFGVAR(msgBuffIO, opLen, recorder, &responseCode);
                break;

            /* Start Recorder */
            case FMSTR_REC_PRTCLSET_OP_START:
                responseCode = _FMSTR_StartRec(recorder);
                break;

            /* Stop Recorder */
            case FMSTR_REC_PRTCLSET_OP_STOP:
                responseCode = _FMSTR_AbortRec(recorder);
                break;

            default:
                responseCode = FMSTR_STC_INVOPCODE;
                break;
        }

        inputLen -= opLen;
    }

    *retStatus = responseCode;
    return response;
}

/******************************************************************************
 *
 * @brief    Help function to crete recorder status byte
 *
 * @param    recFlags    - recorder flags
 *
 * @return   Recorder status in specified protocol form.
 *
 ******************************************************************************/

static FMSTR_U8 _FMSTR_CalcRecStatus(FMSTR_REC_FLAGS recFlags)
{
    /* Not Configured */
    if (recFlags.flg.isConfigured == 0U)
    {
        return 0x00;
    }

    /* Running */
    if (recFlags.flg.isRunning != 0U)
    {
        return 0x02;
    }

    /* Stopped, no data yet (was never running or was stopped before the first sample taken) */
    if (recFlags.flg.hasData == 0U)
    {
        return 0x01;
    }

    /* Stopped, some data but not all (stopped during virgn cycle) */
    if (recFlags.flg.isVirginCycle != 0U)
    {
        return 0x03;
    }

    /* Stopped, all data ready */
    return 0x04;
}

/******************************************************************************
 *
 * @brief    Handling GETREC command
 *
 * @param    session - transport session
 * @param    msgBuffIO   - original command (in) and response buffer (out)
 * @param    retStatus   - pointer to return status variable
 *
 * @return   As all command handlers, the return value should be the buffer
 *           pointer where the response output finished (except checksum)
 *
 ******************************************************************************/

FMSTR_BPTR FMSTR_GetRecCmd(FMSTR_SESSION * session, FMSTR_BPTR msgBuffIO, FMSTR_U8 *retStatus)
{
    FMSTR_U8 responseCode = (FMSTR_STS_OK | FMSTR_STSF_VARLEN);
    FMSTR_BPTR response   = msgBuffIO;
    FMSTR_LP_REC recorder;
    FMSTR_U8 recIndex;
    FMSTR_U8 cfgCode;

    FMSTR_UNUSED(session);

    /* Get recorder index */
    msgBuffIO = FMSTR_ValueFromBuffer8(&recIndex, msgBuffIO);

    if ((recorder = _FMSTR_GetRecorderByRecIx((FMSTR_INDEX)recIndex)) == NULL)
    {
        *retStatus = FMSTR_STC_INSTERR;
        return response;
    }
    else
    {
        /* Get the command configuration code */
        msgBuffIO = FMSTR_ValueFromBuffer8(&cfgCode, msgBuffIO);

        switch (cfgCode)
        {
            /* Get recorder description */
            case FMSTR_REC_PRTCLGET_OP_DESCR:
            {
                FMSTR_REC_BUFF *recorderBuff;
                FMSTR_SIZE strLen;

                /* Get the recorder raw buffer where is also stored its name */
                if ((recorderBuff = _FMSTR_GetRecorderBufferByRecIx((FMSTR_INDEX)recIndex)) == NULL)
                {
                    responseCode = FMSTR_STC_INVBUFF;
                }
                else
                {
                    strLen   = FMSTR_StrLen(recorderBuff->name);
                    response = FMSTR_CopyToBuffer(response, FMSTR_CAST_PTR_TO_ADDR(recorderBuff->name), strLen);
                }
            }
            break;

            /* Get recorder limits */
            case FMSTR_REC_PRTCLGET_OP_LIMITS:
            {
                FMSTR_REC_BUFF *recorderBuff;

                /* Get the recorder */
                if ((recorderBuff = _FMSTR_GetRecorderBufferByRecIx((FMSTR_INDEX)recIndex)) == NULL)
                {
                    responseCode = FMSTR_STC_INVBUFF;
                }
                else
                {
                    /* Put Raw Size of recorder buffer */
                    response = FMSTR_SizeToBuffer(response, recorderBuff->size);
                    /* Put Base period of the recorder */
                    response = FMSTR_ULebToBuffer(response, recorderBuff->basePeriod_ns);
                    /* Put Size of recorder structure */
                    response = FMSTR_SizeToBuffer(response, sizeof(FMSTR_REC));
                    /* Put Size of recorder variable structure */
                    response = FMSTR_SizeToBuffer(response, sizeof(FMSTR_REC_VAR_DATA));
                }
            }
            break;

            /* Get recorder info or status */
            case FMSTR_REC_PRTCLGET_OP_STATUS:
            case FMSTR_REC_PRTCLGET_OP_INFO:
            {
                FMSTR_U8 recorderStatus;

                /* Make sure the status is up to date */
                recorderStatus = _FMSTR_CheckConfiguration(recorder);
                FMSTR_UNUSED(recorderStatus);

                /* Add current status to buffer */
                recorderStatus = _FMSTR_CalcRecStatus(recorder->flags);
                response       = FMSTR_ValueToBuffer8(response, recorderStatus);

                /* full info response contains additional data */
                if (cfgCode == FMSTR_REC_PRTCLGET_OP_INFO)
                {
                    if (recorderStatus == 0x00U)
                    {
                        /* count of recorded variables */
                        response = FMSTR_ValueToBuffer8(response, 0U);
                        /* base address of recorder buffer */
                        response = FMSTR_AddressToBuffer(response, FMSTR_CAST_PTR_TO_ADDR(NULL));
                        /* size of the one set of the recorder point */
                        response = FMSTR_SizeToBuffer(response, 0U);
                        /* count of currently stored points  */
                        response = FMSTR_SizeToBuffer(response, 0U);
                        /* index of the firt point in circular buffer */
                        response = FMSTR_SizeToBuffer(response, 0U);
                    }
                    else
                    {
                        FMSTR_U32 byteIx       = recorder->writePtr >= recorder->buffAddr ? FMSTR_CalcAddrDelta(recorder->writePtr, recorder->buffAddr) : 0U;
                        FMSTR_SIZE currIx      = recorder->pointSize > 0U ? (FMSTR_SIZE)(byteIx / recorder->pointSize) : 0U;
                        FMSTR_SIZE recFirstPnt = recorder->flags.flg.isVirginCycle != 0U ? 0U : currIx;
                        FMSTR_SIZE recPntCnt   = recorder->flags.flg.isVirginCycle != 0U ? currIx : recorder->totalSmplsCnt;

                        /* count of recorded variables */
                        response = FMSTR_ValueToBuffer8(response, recorder->pointVarCount <= 0xffU ? recorder->pointVarCount : 0xffU);
                        /* base address of recorder buffer */
                        response = FMSTR_AddressToBuffer(response, recorder->buffAddr);
                        /* size of the one set of the recorder point */
                        response = FMSTR_SizeToBuffer(response, recorder->pointSize);
                        /* count of currently stored points  */
                        response = FMSTR_SizeToBuffer(response, recPntCnt);
                        /* index of the first point in circular buffer */
                        response = FMSTR_SizeToBuffer(response, recFirstPnt);
                    }
                }
            }
            break;

            default:
                responseCode = FMSTR_STC_INVOPCODE;
                break;
        }
    }

    *retStatus = responseCode;
    return response;
}

/******************************************************************************
 *
 * @brief    API: Start recorder function
 *
 * @param    recIndex - recorder index
 *
 * @return   FMSTR_TRUE in case of successfuly start, FMSTR_FALSE otherwise.
 *
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_RecorderStart(FMSTR_INDEX recIndex)
{
    FMSTR_LP_REC recorder;

    /* Get the recorder */
    if ((recorder = _FMSTR_GetRecorderByRecIx(recIndex)) == NULL)
    {
        return FMSTR_FALSE;
    }

    return (FMSTR_BOOL)((_FMSTR_StartRec(recorder) == FMSTR_STS_OK) ? FMSTR_TRUE : FMSTR_FALSE);
}

/******************************************************************************
 *
 * @brief    API: Pull the trigger of the recorder
 *
 * This function starts the post-trigger stop countdown
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_RecorderTrigger(FMSTR_INDEX recIndex)
{
    FMSTR_LP_REC recorder;

    /* Get the recorder */
    if ((recorder = _FMSTR_GetRecorderByRecIx(recIndex)) == NULL)
    {
        return FMSTR_FALSE;
    }

    return (FMSTR_BOOL)((_FMSTR_TriggerRec(recorder) == FMSTR_STS_OK) ? FMSTR_TRUE : FMSTR_FALSE);
}

/******************************************************************************
 *
 * @brief    API: Stop recorder immediatelly
 *
 * This function starts the post-trigger stop countdown
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_RecorderAbort(FMSTR_INDEX recIndex)
{
    FMSTR_LP_REC recorder;

    /* Get the recorder */
    if ((recorder = _FMSTR_GetRecorderByRecIx(recIndex)) == NULL)
    {
        return FMSTR_FALSE;
    }

    return (FMSTR_BOOL)((_FMSTR_AbortRec(recorder) == FMSTR_STS_OK) ? FMSTR_TRUE : FMSTR_FALSE);
}

/******************************************************************************
 *
 * @brief    Internal start recorder function
 *
 * @param    recIndex - recorder index
 *
 * @return   Result status of start recorder operation, that could be used for communication
 *
 * This function starts recording (initializes internal recording variables and flags)
 *
 ******************************************************************************/

static FMSTR_U8 _FMSTR_StartRec(FMSTR_LP_REC recorder)
{
    /* must be configured */
    if (recorder->flags.flg.isConfigured == 0U)
    {
        FMSTR_U8 responseCode;

        if ((responseCode = _FMSTR_CheckConfiguration(recorder)) != FMSTR_STS_OK)
        {
            return responseCode;
        }
    }

    /* already running ? */
    if (recorder->flags.flg.isRunning != 0U)
    {
        return FMSTR_STS_OK;
    }

    /* initialize write pointer */
    recorder->writePtr = recorder->buffAddr;

    /* initialize time divisor */
    recorder->timeDivCtr = 0U;

    /* run now in virgin cycle */
    recorder->flags.flg.isRunning     = 1U;
    recorder->flags.flg.hasData       = 0U;
    recorder->flags.flg.isVirginCycle = 1U;
    recorder->flags.flg.isStopping    = 0U;

    return FMSTR_STS_OK;
}

/******************************************************************************
 *
 * @brief    Internal Trigger recorder function
 *
 * @param    recIndex - recorder index
 *
 * @return   Result status of start recorder operation, taht could be used for communication
 *
 * This function stops recording (same as manual trigger)
 *
 ******************************************************************************/

static FMSTR_U8 _FMSTR_TriggerRec(FMSTR_LP_REC recorder)
{
    /* must be configured */
    if (recorder->flags.flg.isConfigured == 0U)
    {
        return FMSTR_STC_NOTINIT;
    }

    /* must be both running... */
    if (recorder->flags.flg.isRunning != 0U)
    {
        /* ...and not yet stopping */
        if(recorder->flags.flg.isStopping == 0U)
        {
            recorder->flags.flg.isStopping = 1U;
            recorder->stopRecCountDown = recorder->postTrigger;
        }
    }

    return FMSTR_STS_OK;
}

/******************************************************************************
 *
 * @brief    Internal Stop recorder function
 *
 * @return   Result status of start recorder operation, taht could be used for communication
 *
 * This function stops recording immediatelly
 *
 ******************************************************************************/

static FMSTR_U8 _FMSTR_AbortRec(FMSTR_LP_REC recorder)
{
    /* must be configured */
    if (recorder->flags.flg.isConfigured == 0U)
    {
        return FMSTR_STC_NOTINIT;
    }

    if (recorder->flags.flg.isRunning != 0U)
    {
        recorder->flags.flg.isRunning  = 0U;
        recorder->flags.flg.isStopping = 0U;
    }

    return FMSTR_STS_OK;
}

/******************************************************************************
 *
 * @brief    Check wether given memory region is inside any recorder buffer
 *
 * @param    addr - address of the memory to be checked
 * @param    size - size of the memory to be checked
 *
 * @return   This function returns non-zero if user space is in recorder buffer
 *
 * This function is called as a part of TSA-checking process when the PC host
 * is requesting memory contents
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_IsInRecBuffer(FMSTR_ADDR addr, FMSTR_SIZE size)
{
    FMSTR_LP_REC recorder;
    FMSTR_INDEX i;

    for (i = 0; i < (FMSTR_INDEX)FMSTR_USE_RECORDER; i++)
    {
        /* Get the recorder */
        if ((recorder = _FMSTR_GetRecorderByRecIx(i)) != NULL)
        {
            /* Coverity: Intentional compare of seamingly unrelated pointers. */
            /* coverity[misra_c_2012_rule_18_3_violation:FALSE] */
            if (addr >= recorder->buffAddr)
            {
                if ((addr + size) <= (recorder->buffAddr + recorder->buffSize))
                {
                    return FMSTR_TRUE;
                }
            }
        }
    }

    return FMSTR_FALSE;
}

/******************************************************************************
 *
 * @brief    Check the configuration of the recorder
 *
 * @return   * @return   Result status of start recorder operation, that could be used for communication
 *
 *
 ******************************************************************************/

static FMSTR_U8 _FMSTR_CheckConfiguration(FMSTR_LP_REC recorder)
{
    FMSTR_SIZE pointSize     = 0U;
    FMSTR_SIZE pointVarCount = 0U;
    FMSTR_SIZE blen          = 0U;
    FMSTR_SIZE totalSmpls    = 0U;
    FMSTR_SIZE8 i;

    if (recorder->flags.flg.isConfigured == 0U)
    {
        /* calculate sum of sizes of all variables */
        FMSTR_SIZE size;

        /* get all addresses and sizes */
        for (i = 0; i < recorder->config.varCount; i++)
        {
            size = recorder->varDescr[i].cfg.size;

            /* variable size must be one of valid sizes */
            if (size != 1U && size != 2U && size != 4U && size != 8U)
            {
                return FMSTR_STC_INVSIZE;
            }

            /* compute total size of one sample snapshot */
            if ((recorder->varDescr[i].cfg.triggerMode & FMSTR_REC_TRG_F_TRGONLY) == 0U)
            {
                /* total var count and point size shall not exceed sane int16 limit (prevent wrap) */
                if(pointVarCount >= 0x8000U || pointSize >= 0x8000U)
                {
                    return FMSTR_STC_INVSIZE;
                }

                pointSize += size;
                pointVarCount++;
            }
        }

        /* no variable configured, or samples too big */
        if (pointSize == 0U || pointSize > recorder->buffSize)
        {
            return FMSTR_STC_INVSIZE;
        }

        /* user wants to use less sample points than maximum available */
        if (recorder->config.totalSmps != 0U)
        {
            /* total recorder buffer length in bytes */
            blen = (FMSTR_SIZE)(recorder->config.totalSmps * pointSize);

            /* recorder memory available? */
            if (blen > recorder->buffSize)
            {
                totalSmpls = 0; /* user wants more than maximu, use the maximum */
            }
            else
            {
                totalSmpls = recorder->config.totalSmps; /* OK, accept user value */
            }
        }

        /* use maximum available memory for samples */
        if (totalSmpls == 0U)
        {
            totalSmpls = recorder->buffSize / pointSize;

            /* total recorder buffer length in bytes */
            blen = (FMSTR_SIZE)(totalSmpls * pointSize);
        }

        /* Use pre-trigger value to calculate post-trigger count */
        if (recorder->config.preTrigger < totalSmpls)
        {
            recorder->postTrigger = (FMSTR_SIZE)(totalSmpls - recorder->config.preTrigger - 1U);
        }
        else
        {
            recorder->postTrigger = (FMSTR_SIZE)(totalSmpls - 1U); /* pre-trigger too high, use maximum possible */
        }

        /* Remember samples total count*/
        recorder->totalSmplsCnt = totalSmpls;

        /* remember the effective end of circular buffer */
        recorder->endBuffPtr = FMSTR_CAST_PTR_TO_ADDR(recorder->buffAddr + (blen / FMSTR_CFG_BUS_WIDTH));

        /* Store variable set size */
        recorder->pointSize     = pointSize;
        recorder->pointVarCount = pointVarCount;

        /* it was not configured before, now everything is okay */
        recorder->flags.all              = 0;
        recorder->flags.flg.isConfigured = 1U;
    }

    return FMSTR_STS_OK;
}

/******************************************************************************
 *
 * @brief    Compare macro used in trigger detection
 *
 * @param    v - original command
 * @param    t - response buffer
 *
 * @return   zero when value is lower than threshold.
 * @return   non-zero when value is greater than or equal as threshold
 *
 ******************************************************************************/

#define CMP(v, t) ((FMSTR_BOOL)(((v) < (t)) ? 0 : 1))

static FMSTR_BOOL _FMSTR_Compare8S(FMSTR_LP_REC_VAR_DATA varData)
{
    return CMP(FMSTR_GetS8(varData->cfg.addr), FMSTR_GetS8(varData->cfg.trgAddr));
}

static FMSTR_BOOL _FMSTR_Compare8U(FMSTR_LP_REC_VAR_DATA varData)
{
    return CMP(FMSTR_GetU8(varData->cfg.addr), FMSTR_GetU8(varData->cfg.trgAddr));
}

static FMSTR_BOOL _FMSTR_Compare16S(FMSTR_LP_REC_VAR_DATA varData)
{
    return CMP(FMSTR_GetS16(varData->cfg.addr), FMSTR_GetS16(varData->cfg.trgAddr));
}

static FMSTR_BOOL _FMSTR_Compare16U(FMSTR_LP_REC_VAR_DATA varData)
{
    return CMP(FMSTR_GetU16(varData->cfg.addr), FMSTR_GetU16(varData->cfg.trgAddr));
}

static FMSTR_BOOL _FMSTR_Compare32S(FMSTR_LP_REC_VAR_DATA varData)
{
    return CMP(FMSTR_GetS32(varData->cfg.addr), FMSTR_GetS32(varData->cfg.trgAddr));
}

static FMSTR_BOOL _FMSTR_Compare32U(FMSTR_LP_REC_VAR_DATA varData)
{
    return CMP(FMSTR_GetU32(varData->cfg.addr), FMSTR_GetU32(varData->cfg.trgAddr));
}

static FMSTR_BOOL _FMSTR_Compare64S(FMSTR_LP_REC_VAR_DATA varData)
{
    return CMP(FMSTR_GetS64(varData->cfg.addr), FMSTR_GetS64(varData->cfg.trgAddr));
}

static FMSTR_BOOL _FMSTR_Compare64U(FMSTR_LP_REC_VAR_DATA varData)
{
    return CMP(FMSTR_GetU64(varData->cfg.addr), FMSTR_GetU64(varData->cfg.trgAddr));
}

#if FMSTR_REC_FLOAT_TRIG > 0
static FMSTR_BOOL _FMSTR_CompareFloat(FMSTR_LP_REC_VAR_DATA varData)
{
    return CMP(FMSTR_GetFloat(varData->cfg.addr), FMSTR_GetFloat(varData->cfg.trgAddr));
}

static FMSTR_BOOL _FMSTR_CompareDouble(FMSTR_LP_REC_VAR_DATA varData)
{
    return CMP(FMSTR_GetDouble(varData->cfg.addr), FMSTR_GetDouble(varData->cfg.trgAddr));
}
#endif

/******************************************************************************
 *
 * @brief    Gets the pointer pointing to recorder data buffer by Recorder Index
 *
 * @param    recIndex - Index of Recorder
 *
 * @return   NULL when the recorder doesn't exists, otherwise the pointer to recorder buffer
 *
 ******************************************************************************/

static FMSTR_REC_BUFF *_FMSTR_GetRecorderBufferByRecIx(FMSTR_INDEX recIndex)
{
    if (recIndex < 0 || recIndex >= (FMSTR_INDEX)FMSTR_USE_RECORDER)
    {
        return NULL;
    }

    return &fmstr_recBuffs[recIndex];
}

/******************************************************************************
 *
 * @brief    Gets the pointer pointing to recorder data by Recorder Index
 *
 * @param    recIndex - Index of Recorder
 *
 * @return   NULL when the recorder doesn't exists, otherwise the pointer to recorder structure
 *
 ******************************************************************************/

static FMSTR_LP_REC _FMSTR_GetRecorderByRecIx(FMSTR_INDEX recIndex)
{
    if (recIndex < 0 || recIndex >= (FMSTR_INDEX)FMSTR_USE_RECORDER)
    {
        return NULL;
    }

    return (FMSTR_LP_REC)FMSTR_CAST_ADDR_TO_PTR(fmstr_recBuffs[recIndex].addr);
}

/******************************************************************************
 *
 * @brief    API: Recorder worker routine - can be called from application's timer ISR
 *
 *
 * This returns quickly if recorder is not running, otherwise it calls quite lengthy
 * recorder routine which does all the recorder work (sampling, triggering)
 *
 ******************************************************************************/

#if defined(FMSTR_PLATFORM_56F8xxx) || defined(FMSTR_PLATFORM_56F8xx)
#pragma interrupt called
#endif

void FMSTR_Recorder(FMSTR_INDEX recIndex)
{
    FMSTR_LP_REC recorder = _FMSTR_GetRecorderByRecIx(recIndex);

    /* recorder invalid */
    if (recorder == NULL)
    {
        return;
    }

    /* or not active */
    if (recorder->flags.flg.isRunning == 0U)
    {
        return;
    }

    /* do the hard work */
    _FMSTR_Recorder2(recorder);
}

/******************************************************************************
 *
 * @brief    Recorder function called when recorder is active
 *
 ******************************************************************************/

#if defined(FMSTR_PLATFORM_56F8xxx) || defined(FMSTR_PLATFORM_56F8xx)
#pragma interrupt called
#endif

static void _FMSTR_Recorder2(FMSTR_LP_REC recorder)
{
    FMSTR_LP_REC_VAR_DATA recVarData;
    FMSTR_PCOMPAREFUNC compareFunc;
    FMSTR_PREADFUNC readFunc;
    FMSTR_SIZE8 triggerMode;
    FMSTR_SIZE8 sz;
    FMSTR_SIZE i;
    FMSTR_BOOL cmp;
    FMSTR_U8 triggerResult;

#if FMSTR_FASTREC_NO_TIME_DIVISION == 0
    /* skip this call ? */
    if (recorder->timeDivCtr > 0U)
    {
        /* maybe next time... */
        recorder->timeDivCtr--;
        return;
    }

    /* re-initialize divider */
    recorder->timeDivCtr = recorder->config.timeDiv;
#endif /* FMSTR_FASTREC_NO_TIME_DIVISION */

    /* variable info data for the next loop processing */
    recVarData  = recorder->varDescr;

    /* take snapshot of variable values (do not use 'continue' here!) */
    for (i = 0U; i < recorder->config.varCount; i++)
    {
        triggerMode = recVarData->cfg.triggerMode;

        /* evaluate trigger condition if any */
        if (recVarData->compareFunc != NULL)
        {
            /* but only if still running */
            if (recorder->flags.flg.isStopping == 0U)
            {
                /* compare trigger threshold */
                compareFunc = recVarData->compareFunc;
                cmp = compareFunc(recVarData);

                /* No trigger checking in virgin cycle */
                if (recorder->flags.flg.isVirginCycle == 0U)
                {
#if FMSTR_FASTREC_RISING_EDGE_TRG_ONLY
                    if (cmp != FMSTR_FALSE && recVarData->trgLastState == FMSTR_FALSE)
                        triggerResult = _FMSTR_TriggerRec(recorder);
#elif FMSTR_FASTREC_FALLING_EDGE_TRG_ONLY
                    if (cmp == FMSTR_FALSE && recVarData->trgLastState != FMSTR_FALSE)
                        triggerResult = _FMSTR_TriggerRec(recorder);
#else
                    /* Check the Above trigger */
                    if (cmp != FMSTR_FALSE && (triggerMode & FMSTR_REC_TRG_F_ABOVE) != 0U)
                    {
                        /* Solve as Edge or Level trigger */
                        if ((triggerMode & FMSTR_REC_TRG_F_LEVEL) != 0U || (recVarData->trgLastState == FMSTR_FALSE))
                        {
                            triggerResult = _FMSTR_TriggerRec(recorder);
                        }
                    }

                    /* Check the Below trigger */
                    if (cmp == FMSTR_FALSE && (triggerMode & FMSTR_REC_TRG_F_BELOW) != 0U)
                    {
                        /* Solve as Edge or Level trigger */
                        if ((triggerMode & FMSTR_REC_TRG_F_LEVEL) != 0U || recVarData->trgLastState != FMSTR_FALSE)
                        {
                            triggerResult = _FMSTR_TriggerRec(recorder);
                        }
                    }
#endif
                }

                /* Store the last comparison */
                recVarData->trgLastState = cmp;
            }
        }

        /* Store the recorder variable to buffer */
        if ((triggerMode & FMSTR_REC_TRG_F_TRGONLY) == 0U)
        {
            sz = recVarData->cfg.size;
            readFunc = recVarData->readFunc;

            /* Use optimized value-copy routine, otherwise use normal copy */
            if (readFunc != NULL)
            {
                readFunc(recorder->writePtr, recVarData->cfg.addr);
            }
            else
            {
                FMSTR_MemCpyFrom(recorder->writePtr, recVarData->cfg.addr, sz);
            }

            sz /= FMSTR_CFG_BUS_WIDTH;
            recorder->writePtr += sz;
        }

        /* advance to next variable (MISRA does not let to do it in the for() statement */
        recVarData++;
    }

    /* We now have at least some data*/
    recorder->flags.flg.hasData = 1U;

    /* wrap around (circular buffer) ? */
    if (recorder->writePtr >= recorder->endBuffPtr)
    {
        recorder->writePtr                = recorder->buffAddr;
        recorder->flags.flg.isVirginCycle = 0U;
    }

    /* in stopping mode ? (note that this bit might have been set just above!) */
    if (recorder->flags.flg.isStopping != 0U)
    {
        /* count down post-trigger samples expired ? */
        if (recorder->stopRecCountDown == 0U)
        {
            /* STOP RECORDER */
            recorder->flags.flg.isRunning = 0U;
            return;
        }

        /* perhaps next time */
        recorder->stopRecCountDown--;
    }

    FMSTR_UNUSED(triggerResult);
}

#else /* FMSTR_USE_RECORDER && (!FMSTR_DISABLE) */

FMSTR_BOOL FMSTR_RecorderCreate(FMSTR_INDEX recIndex, FMSTR_REC_BUFF *buffCfg)
{
    FMSTR_UNUSED(recIndex);
    FMSTR_UNUSED(buffCfg);

    return FMSTR_TRUE;
}

FMSTR_BOOL FMSTR_RecorderSetTimeBase(FMSTR_INDEX recIndex, FMSTR_U32 timeBase_ns)
{
    FMSTR_UNUSED(recIndex);
    FMSTR_UNUSED(timeBase_ns);

    return FMSTR_TRUE;
}

/* use void recorder API functions */
void FMSTR_Recorder(FMSTR_INDEX recIndex)
{
    FMSTR_UNUSED(recIndex);
}

FMSTR_BOOL FMSTR_RecorderTrigger(FMSTR_INDEX recIndex)
{
    FMSTR_UNUSED(recIndex);

    return FMSTR_TRUE;
}

FMSTR_BOOL FMSTR_RecorderAbort(FMSTR_INDEX recIndex)
{
    FMSTR_UNUSED(recIndex);

    return FMSTR_TRUE;
}

FMSTR_BOOL FMSTR_RecorderConfigure(FMSTR_INDEX recIndex, FMSTR_REC_CFG *recCfg)
{
    FMSTR_UNUSED(recIndex);
    FMSTR_UNUSED(recCfg);

    return FMSTR_TRUE;
}

FMSTR_BOOL FMSTR_RecorderAddVariable(FMSTR_INDEX recIndex, FMSTR_INDEX recVarIx, FMSTR_REC_VAR *recVarCfg)
{
    FMSTR_UNUSED(recIndex);
    FMSTR_UNUSED(recVarIx);
    FMSTR_UNUSED(recVarCfg);

    return FMSTR_TRUE;
}

FMSTR_BOOL FMSTR_RecorderStart(FMSTR_INDEX recIndex)
{
    FMSTR_UNUSED(recIndex);
    return FMSTR_TRUE;
}

#endif /* FMSTR_USE_RECORDER && (!FMSTR_DISABLE) */
