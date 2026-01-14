/*
 * Copyright (c) 2007-2015 Freescale Semiconductor, Inc.
 * Copyright 2018-2020, 2024-2025 NXP
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
 * FreeMASTER Communication Driver - Pipe I/O
 */

#include "freemaster.h"
#include "freemaster_private.h"
#include "freemaster_protocol.h"
#include "freemaster_utils.h"

#if FMSTR_USE_PIPES > 0 && FMSTR_DISABLE == 0

#if FMSTR_USE_PIPE_PRINTF_VARG > 0
/* Coverity: Intentional use of stdarg when user explicitly requires it. */
/* coverity[misra_c_2012_rule_17_1_violation:FALSE] */
#include <stdarg.h>
#endif

/***********************************
 *  local types
 ***********************************/

/* Flags for command FMSTR_CMD_GETPIPE */
#define FMSTR_PIPE_GETPIPE_FLAG_PORT 0x01U

/* Flags for get pipe info in command FMSTR_CMD_GETPIPE */
#define FMSTR_PIPE_GETINFO_FLAG_ISOPEN 0x01U

#define FMSTR_PIPE_CFGCODE_NAME 0x81U
#define FMSTR_PIPE_CFGCODE_INFO 0x82U

/* runtime flags */
typedef union
{
    FMSTR_FLAGS all;

    struct
    {
        unsigned bExpectOdd : 1; /* now expecting even round */
        unsigned bInComm : 1;    /* in protocol handler now */
        unsigned bLocked : 1;    /* data buffer access is locked */
    } flg;

} FMSTR_PIPE_FLAGS;

typedef union
{
    FMSTR_FLAGS all;

    struct
    {
        unsigned bIsFull : 1; /* buffer is full (wp==rp, but not empty) */
    } flg;

} FMSTR_PIPE_BUFF_FLAGS;

typedef struct
{
    FMSTR_LP_U8 buff;
    FMSTR_PIPE_SIZE size;
    FMSTR_PIPE_SIZE wp;
    FMSTR_PIPE_SIZE rp;
    FMSTR_PIPE_BUFF_FLAGS flags;
} FMSTR_PIPE_BUFF;

typedef struct
{
    /* pipe information */
    const FMSTR_CHAR *name; /* String description of the pipe. */
    FMSTR_U8 type;          /* Type of the usage of pipe */

    /* pipe configuration */
    FMSTR_PIPE_BUFF rx;
    FMSTR_PIPE_BUFF tx;
    FMSTR_PIPE_PORT pipePort;
    FMSTR_PPIPEFUNC pCallback;

    /* runtime information */
    FMSTR_PIPE_FLAGS flags;
    FMSTR_U8 nLastBytesReceived;

    /* residue buffer for 8-bit handling on DSP56F8xx platform */
#if FMSTR_CFG_BUS_WIDTH > 1
    FMSTR_U16 rd8Resid;
    FMSTR_U16 wr8Resid;
#endif

#if FMSTR_USE_PIPE_PRINTF > 0
    FMSTR_CHAR printfBuff[FMSTR_PIPES_PRINTF_BUFF_SIZE];
    FMSTR_SIZE printfBPtr;
#endif

} FMSTR_PIPE;

/* Pipe ITOA formatting flags */
typedef union
{
    FMSTR_FLAGS all;

    struct
    {
        unsigned upperc : 1;     /* uppercase HEX letters */
        unsigned zeroes : 1;     /* prepend with zeroes rather than spaces */
        unsigned showsign : 1;   /* show sign always (even if plus) */
        unsigned negative : 1;   /* sign determined during itoa */
        unsigned signedtype : 1; /* type is signed (used with va_list only) */
        unsigned isstring : 1;   /* formatting %s */

    } flg;

} FMSTR_PIPE_ITOA_FLAGS;

/* Pipe printf context */
typedef struct
{
    FMSTR_PIPE_ITOA_FLAGS flags;
    FMSTR_U8 radix;    /* print radix indicated by format (X, x, b, o, d, i, ...) */
    FMSTR_U8 dtsize;   /* data size indicated by format modifiers (h, hh, l) */
    FMSTR_SIZE alen;  /* print length */
    FMSTR_SIZE stklen; /* size of data argument on stack */

} FMSTR_PIPE_PRINTF_CTX;

typedef FMSTR_BOOL (*FMSTR_PIPE_ITOA_FUNC)(FMSTR_HPIPE pipeHandle, const void *parg, FMSTR_PIPE_PRINTF_CTX *ctx);

/***********************************
 *  local variables
 ***********************************/

static FMSTR_PIPE pcm_pipes[FMSTR_USE_PIPES];

/**********************************************************************
 *  local macros
 **********************************************************************/

#define FMSTR_PIPE_ITOAFMT_BIN  0U
#define FMSTR_PIPE_ITOAFMT_OCT  1U
#define FMSTR_PIPE_ITOAFMT_DEC  2U
#define FMSTR_PIPE_ITOAFMT_HEX  3U
#define FMSTR_PIPE_ITOAFMT_CHAR 4U

#define FMSTR_IS_DIGIT(x) (((x) >= '0') && ((x) <= '9'))

/**********************************************************************
 *  local functions
 **********************************************************************/

static FMSTR_PIPE *_FMSTR_FindPipe(FMSTR_PIPE_PORT pipePort);
static FMSTR_PIPE_SIZE _FMSTR_PipeGetBytesFree(FMSTR_PIPE_BUFF *pipeBuff);
static FMSTR_PIPE_SIZE _FMSTR_PipeGetBytesReady(FMSTR_PIPE_BUFF *pipeBuff);
static void _FMSTR_PipeDiscardBytes(FMSTR_PIPE_BUFF *pipeBuff, FMSTR_SIZE8 countBytes);
static FMSTR_BPTR _FMSTR_PipeReceive(FMSTR_BPTR msgBuffIO, FMSTR_PIPE *pp, FMSTR_SIZE8 msgBuffSize);
static FMSTR_BPTR _FMSTR_PipeTransmit(FMSTR_BPTR msgBuffIO, FMSTR_PIPE *pp, FMSTR_SIZE8 msgBuffSize);
static FMSTR_BOOL _FMSTR_PipeIToAFinalize(FMSTR_HPIPE pipeHandle, FMSTR_PIPE_PRINTF_CTX *pctx);

static FMSTR_BOOL _FMSTR_PipePrintfOne(FMSTR_HPIPE pipeHandle,
                                       const char *format,
                                       void *parg, FMSTR_SIZE argSize,
                                       FMSTR_PIPE_ITOA_FUNC pItoaFunc);

static FMSTR_BOOL _FMSTR_PipePrintfAny(FMSTR_HPIPE pipeHandle, va_list *parg, FMSTR_PIPE_PRINTF_CTX *pctx);
static FMSTR_BOOL _FMSTR_PipePrintfV(FMSTR_HPIPE pipeHandle, const char *format, va_list *parg);
static FMSTR_CHAR _FMSTR_XDigit(FMSTR_U8 digit, FMSTR_BOOL uppercase);

/**********************************************************************
 *  local "inline" functions (may also be static on some platforms)
 **********************************************************************/

FMSTR_INLINE FMSTR_BOOL _FMSTR_PipePrintfFlush(FMSTR_HPIPE pipeHandle);
FMSTR_INLINE FMSTR_BOOL _FMSTR_PipePrintfPutc(FMSTR_HPIPE pipeHandle, char c);

/******************************************************************************
 *
 * @brief  Initialization of pipes layer
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_InitPipes(void)
{
    FMSTR_INDEX i;

    for (i = 0; i < (FMSTR_INDEX)FMSTR_USE_PIPES; i++)
    {
        pcm_pipes[i].pipePort = 0;
    }

    return FMSTR_TRUE;
}

/******************************************************************************
 *
 * @brief  API: Opening the pipe
 *
 ******************************************************************************/

FMSTR_HPIPE FMSTR_PipeOpen(FMSTR_PIPE_PORT pipePort,
                           FMSTR_PPIPEFUNC pipeCallback,
                           FMSTR_ADDR pipeRxBuff,
                           FMSTR_PIPE_SIZE pipeRxSize,
                           FMSTR_ADDR pipeTxBuff,
                           FMSTR_PIPE_SIZE pipeTxSize,
                           FMSTR_U8 type,
                           const FMSTR_CHAR *name)
{
    FMSTR_PIPE *pp    = NULL;
    FMSTR_INDEX ifree = -1;
    FMSTR_INDEX i;

    for (i = 0; i < (FMSTR_INDEX)FMSTR_USE_PIPES; i++)
    {
        pp = &pcm_pipes[i];

        /* find first free pipe */
        if (pp->pipePort == 0U && ifree < 0)
        {
            ifree = i;
        }
        /* look for existing pipe with the same port */
        if (pp->pipePort == pipePort)
        {
            break;
        }
    }

    /* pipe not found */
    if (i >= (FMSTR_INDEX)FMSTR_USE_PIPES)
    {
        /* create new pipe */
        if (ifree >= 0)
        {
            pp = &pcm_pipes[ifree];
        }
        /* no slot for creating pipe */
        else
        {
            return NULL;
        }
    }

    FMSTR_MemSet(pp, 0, sizeof(FMSTR_PIPE));

    /* disable pipe (just in case the interrupt would come now) */
    pp->pipePort = 0;

    /* initialize pipe */
    pp->rx.buff   = pipeRxBuff;
    pp->rx.size   = pipeRxSize;
    pp->tx.buff   = pipeTxBuff;
    pp->tx.size   = pipeTxSize;
    pp->pCallback = pipeCallback;
    pp->name      = name;
    pp->type      = type;

#if FMSTR_USE_PIPE_PRINTF > 0
    pp->printfBPtr = 0;
#endif

    /* activate pipe for the new port */
    pp->pipePort = pipePort;

    return (FMSTR_HPIPE)pp;
}

/******************************************************************************
 *
 * @brief  PIPE API: Close pipe
 *
 ******************************************************************************/

void FMSTR_PipeClose(FMSTR_HPIPE pipeHandle)
{
    FMSTR_PIPE *pp = (FMSTR_PIPE *)pipeHandle;

    /* un-initialize pipe */
    if (pp != NULL)
    {
        pp->pipePort = 0;
    }
}

/******************************************************************************
 *
 * @brief  PIPE API: Write data to a pipe
 *
 ******************************************************************************/

FMSTR_PIPE_SIZE FMSTR_PipeWrite(FMSTR_HPIPE pipeHandle,
                                FMSTR_ADDR pipeData,
                                FMSTR_PIPE_SIZE pipeDataLen,
                                FMSTR_PIPE_SIZE writeGranularity)
{
    FMSTR_PIPE *pp         = (FMSTR_PIPE *)pipeHandle;
    FMSTR_PIPE_BUFF *pbuff = &pp->tx;
    FMSTR_PIPE_SIZE total  = _FMSTR_PipeGetBytesFree(pbuff);
    FMSTR_PIPE_SIZE s;

    /* when invalid address is given, just return number of bytes free */
    if (pipeData != NULL)
    {
        /* only fill the free space */
        if (pipeDataLen > total)
        {
            pipeDataLen = total;
        }

        /* obey granularity */
        if (writeGranularity > 1U)
        {
            pipeDataLen /= writeGranularity;
            pipeDataLen *= writeGranularity;
        }

        /* return value */
        total = pipeDataLen;

        /* valid length only */
        if (pipeDataLen > 0U)
        {
            /* Calculate total bytes available in the rest of buffer */
          
            /* Coverity: Construction of the circular buffer logic ensures that this operation is 
               positive and cannot wrap. No risk of signed/unsigned cast misinterpretation. */
            /* coverity[cert_int31_c_violation:FALSE] */
            s = (FMSTR_PIPE_SIZE)((pbuff->size - pbuff->wp) * FMSTR_CFG_BUS_WIDTH);
            if (s > pipeDataLen)
            {
                s = pipeDataLen;
            }

            /* get the bytes */
            FMSTR_MemCpyFrom(pbuff->buff + pbuff->wp, pipeData, (FMSTR_SIZE8)s);
            pipeData += s / FMSTR_CFG_BUS_WIDTH;

            /* Advance & wrap pointer to start of the buffer if needed */
            
            /* Coverity: Conditions above and construction of the buffer logic ensures 
               that wp cannot wrap around maximum value. Wrapping around a buffer size is 
               intentional in the condition below. */
            /* coverity[cert_int30_c_violation:FALSE] */
            pbuff->wp += s / FMSTR_CFG_BUS_WIDTH;
            if (pbuff->wp >= pbuff->size)
            {
                pbuff->wp = 0;
            }

            /* rest of frame to a (wrapped) beginning of buffer */
            pipeDataLen -= (FMSTR_SIZE8)s;
            if (pipeDataLen > 0U)
            {
                FMSTR_MemCpyFrom(pbuff->buff + pbuff->wp, pipeData, (FMSTR_SIZE8)pipeDataLen);
                pbuff->wp += pipeDataLen / FMSTR_CFG_BUS_WIDTH;
            }

            /* buffer got full? */
            if (pbuff->wp == pbuff->rp)
            {
                pbuff->flags.flg.bIsFull = 1;
            }
        }
    }

    return total;
}

/******************************************************************************
 *
 * @brief  PIPE API: Put zero-terminated string into pipe. Succeeds only
 * if full string fits into the output buffer and return TRUE if so.
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_PipePuts(FMSTR_HPIPE pipeHandle, const char *text)
{
    FMSTR_PIPE *pp            = (FMSTR_PIPE *)pipeHandle;
    FMSTR_PIPE_BUFF *pbuff    = &pp->tx;
    FMSTR_PIPE_SIZE bytesFree = _FMSTR_PipeGetBytesFree(pbuff);
    FMSTR_SIZE strLen = FMSTR_StrLen(text);

    /* Either put full string or nothing. This also prevents wrapping when converting to PIPE_SIZE.  */
    if (strLen > (FMSTR_SIZE)bytesFree)
    {
        return FMSTR_FALSE;
    }

    /* Coverity: Intentional cast of const char* to FMSTR_ADDR. */
    /* coverity[misra_c_2012_rule_11_8_violation:FALSE] */
    return FMSTR_PipeWrite(pipeHandle, (FMSTR_ADDR)text, (FMSTR_PIPE_SIZE)strLen, 0) >=
        (FMSTR_PIPE_SIZE)strLen ? FMSTR_TRUE : FMSTR_FALSE;
}

/******************************************************************************
 *
 * @brief  PRINTF-like formatting functions
 *
 *****************************************************************************/

#if FMSTR_USE_PIPE_PRINTF > 0

static FMSTR_CHAR _FMSTR_XDigit(FMSTR_U8 digit, FMSTR_BOOL uppercase)
{
    FMSTR_CHAR c;

    if (digit < 10U)
    {
        c = '0';
        c += (FMSTR_CHAR)digit;
    }
    else
    {
        if (uppercase != FMSTR_FALSE)
        {
            c = 'A' - 10;
        }
        else
        {
            c = 'a' - 10;
        }

        c += (FMSTR_CHAR)digit;
    }

    return c;
}

/******************************************************************************
 *
 * @brief  Flush pipe's printf formatting buffer into pipe output buffer
 *
 *****************************************************************************/

FMSTR_INLINE FMSTR_BOOL _FMSTR_PipePrintfFlush(FMSTR_HPIPE pipeHandle)
{
    FMSTR_PIPE *pp = (FMSTR_PIPE *)pipeHandle;
    FMSTR_BOOL ok  = FMSTR_TRUE;
    FMSTR_SIZE i;

    if (pp->printfBPtr != 0U)
    {
        FMSTR_SIZE sz = FMSTR_PipeWrite(pipeHandle, (FMSTR_ADDR)pp->printfBuff, (FMSTR_PIPE_SIZE)pp->printfBPtr, 0);

        /* all characters could NOT be printed */
        if (sz < pp->printfBPtr)
        {
            /* Move not sent characters */
            for (i = 0; i < (pp->printfBPtr - sz); i++)
            {
                pp->printfBuff[i] = pp->printfBuff[sz + i];
            }

            pp->printfBPtr -= sz;

            ok = FMSTR_FALSE;
        }
        else
        {
            /* not a cyclic buffer, must start over anyway (also if error) */
            pp->printfBPtr = 0;
        }
    }

    return ok;
}

/******************************************************************************
 *
 * @brief  Put one character into pipe's printf formatting buffer
 *
 *****************************************************************************/

FMSTR_INLINE FMSTR_BOOL _FMSTR_PipePrintfPutc(FMSTR_HPIPE pipeHandle, char c)
{
    FMSTR_PIPE *pp = (FMSTR_PIPE *)pipeHandle;

    /* when buffer is full */
    if (pp->printfBPtr >= FMSTR_PIPES_PRINTF_BUFF_SIZE)
    {
        /* try to flush some bytes */
        if (_FMSTR_PipePrintfFlush(pipeHandle) == FMSTR_FALSE)
        {
            return FMSTR_FALSE;
        }
    }

    /* abort if still full */
    if (pp->printfBPtr >= FMSTR_PIPES_PRINTF_BUFF_SIZE)
    {
        return FMSTR_FALSE;
    }

    pp->printfBuff[pp->printfBPtr++] = c;
    return FMSTR_TRUE;
}

/******************************************************************************
 *
 * @brief  This function finishes the number formatting, adds spacing, signs
 *         and reverses the string (the input is coming in reversed order)
 *
 *****************************************************************************/

static FMSTR_BOOL _FMSTR_PipeIToAFinalize(FMSTR_HPIPE pipeHandle, FMSTR_PIPE_PRINTF_CTX *pctx)
{
    FMSTR_PIPE *pp = (FMSTR_PIPE *)pipeHandle;
    FMSTR_SIZE bptr, minlen, i, bhalf;
    FMSTR_CHAR z, sgn;

    /* buffer pointer into local variable */
    bptr = pp->printfBPtr;

    /* if anything goes wrong, throw our prepared string away */
    pp->printfBPtr = 0;

    /* zero may come as an empty string from itoa procedures */
    if (bptr == 0U)
    {
        pp->printfBuff[0] = '0';
        bptr              = 1;
    }

    /* first strip the zeroes put by itoa */
    while (bptr > 1U && pp->printfBuff[bptr - 1U] == '0')
    {
        bptr--;
    }

    /* determine sign to print */
    if (pctx->flags.flg.negative != 0U)
    {
        sgn = '-';

        /* minus need to be shown always */
        pctx->flags.flg.showsign = 1U;
    }
    else
    {
        /* plus will be shown if flg.showsign was set by caller */
        sgn = '+';
    }

    /* unsigned types can not print sign */
    if (pctx->flags.flg.signedtype == 0U)
    {
        pctx->flags.flg.showsign = 0U;
    }

    /* calculate minimum buffer length needed */
    minlen = bptr;
    if (pctx->flags.flg.showsign != 0U)
    {
        minlen++;
    }

    /* will it fit? */
    if (FMSTR_PIPES_PRINTF_BUFF_SIZE < minlen)
    {
        return FMSTR_FALSE;
    }

    /* required length should never exceed the buffer length */
    if (pctx->alen > FMSTR_PIPES_PRINTF_BUFF_SIZE)
    {
        pctx->alen = FMSTR_PIPES_PRINTF_BUFF_SIZE;
    }

    /* choose prefix character (zero, space or sign-extension OCT/HEX/BIN) */
    if (pctx->flags.flg.zeroes != 0U)
    {
        z = '0';

        /* sign extend? */
        if (pctx->flags.flg.negative != 0U)
        {
            switch (pctx->radix)
            {
                case FMSTR_PIPE_ITOAFMT_BIN:
                    z = '1';
                    break;
                case FMSTR_PIPE_ITOAFMT_OCT:
                    z = '7';
                    break;
                case FMSTR_PIPE_ITOAFMT_HEX:
                    z = (FMSTR_CHAR)(pctx->flags.flg.upperc != 0U ? 'F' : 'f');
                    break;
                default:
                    z = '0';
                    break;
            }
        }

        /* the sign will be in front of added zeroes */
        if (pctx->flags.flg.showsign != 0U)
        {
            pctx->alen--;
        }
    }
    else
    {
        z = ' ';

        /* sign should be in front of the number */
        if (pctx->flags.flg.showsign != 0U)
        {
            pp->printfBuff[bptr++]   = sgn;
            pctx->flags.flg.showsign = 0; /* prevent it to be added again below  */
        }
    }

    /* now fill to required len */
    while (bptr < pctx->alen)
    {
        pp->printfBuff[bptr++] = z;
    }

    /* add the sign if needed */
    if (pctx->flags.flg.showsign != 0U)
    {
        pp->printfBuff[bptr++] = sgn;
    }

    /* buffer contains this number of characters */
    pp->printfBPtr = bptr;

    /* now reverse the string and feed it to pipe */
    bhalf = bptr / 2U;
    bptr--;
    for (i = 0; i < bhalf; i++)
    {
        z                        = pp->printfBuff[i];
        pp->printfBuff[i]        = pp->printfBuff[bptr - i];
        pp->printfBuff[bptr - i] = z;
    }

    return FMSTR_TRUE;
}

/******************************************************************************
 *
 * @brief  This function formats the argument into the temporary printf buffer
 *         It is granted by the caller that the buffer is empty before calling.
 *
 *****************************************************************************/

static FMSTR_BOOL FMSTR_PipeU8ToA(FMSTR_HPIPE pipeHandle, const FMSTR_U8 *parg, FMSTR_PIPE_PRINTF_CTX *pctx)
{
    FMSTR_PIPE *pp = (FMSTR_PIPE *)pipeHandle;
    FMSTR_U8 arg;
    FMSTR_U8 dig;
    FMSTR_INDEX i;

    /* We need 1 byte argument */
    FMSTR_ASSERT_RETURN(pctx != NULL && pctx->stklen >= sizeof(arg), FMSTR_FALSE);
    arg = *parg;
      
    switch (pctx->radix)
    {
        case FMSTR_PIPE_ITOAFMT_CHAR:
            FMSTR_ASSERT_RETURN(pp->printfBPtr <= (FMSTR_PIPES_PRINTF_BUFF_SIZE-1U), FMSTR_FALSE);
            
            /* Coverity: Intentional cast U8 to char. The 'arg' is the character being printed by local app. 
               No risk of misinterpretation. */
            /* coverity[cert_int31_c_violation:FALSE] */
            pp->printfBuff[pp->printfBPtr++] = (FMSTR_CHAR)arg;
            break;

        case FMSTR_PIPE_ITOAFMT_BIN:
            FMSTR_ASSERT_RETURN(pp->printfBPtr <= (FMSTR_PIPES_PRINTF_BUFF_SIZE-8U), FMSTR_FALSE);
            for (i = 0; i < 8; i++)
            {
                if (arg == 0U)
                {
                    break;
                }

                dig = (FMSTR_U8)'0';
                dig += (FMSTR_U8)(arg & 1U);

                pp->printfBuff[pp->printfBPtr++] = (FMSTR_CHAR)dig;
                arg >>= 1;
            }
            break;

        case FMSTR_PIPE_ITOAFMT_OCT:
            FMSTR_ASSERT_RETURN(pp->printfBPtr <= (FMSTR_PIPES_PRINTF_BUFF_SIZE-3U), FMSTR_FALSE);
            for (i = 0; i < 3; i++)
            {
                if (arg == 0U)
                {
                    break;
                }

                dig = (FMSTR_U8)'0';
                dig += (FMSTR_U8)(arg & 7U);

                pp->printfBuff[pp->printfBPtr++] = (FMSTR_CHAR)dig;
                arg >>= 3;
            }
            break;

        case FMSTR_PIPE_ITOAFMT_DEC:
            FMSTR_ASSERT_RETURN(pp->printfBPtr <= (FMSTR_PIPES_PRINTF_BUFF_SIZE-3U), FMSTR_FALSE);
            for (i = 0; i < 3; i++)
            {
                if (arg == 0U)
                {
                    break;
                }

                dig = (FMSTR_U8)'0';
                dig += (FMSTR_U8)(arg % 10U);

                pp->printfBuff[pp->printfBPtr++] = (FMSTR_CHAR)dig;
                arg /= 10U;
            }
            break;

        case FMSTR_PIPE_ITOAFMT_HEX:
        default:
            FMSTR_ASSERT_RETURN(pp->printfBPtr <= (FMSTR_PIPES_PRINTF_BUFF_SIZE-2U), FMSTR_FALSE);
            for (i = 0; i < 2; i++)
            {
                if (arg == 0U)
                {
                    break;
                }

                dig = (FMSTR_U8)(arg & 15U);

                pp->printfBuff[pp->printfBPtr++] =
                    _FMSTR_XDigit((FMSTR_U8)dig, (FMSTR_BOOL)(pctx->flags.flg.upperc != 0U));
                arg >>= 4;
            }
            break;
    }

    return _FMSTR_PipeIToAFinalize(pipeHandle, pctx);
}

/******************************************************************************
 *
 * @brief  This function formats the argument into the temporary printf buffer
 *         It is granted by the caller that the buffer is empty before calling.
 *
 *****************************************************************************/

static FMSTR_BOOL FMSTR_PipeS8ToA(FMSTR_HPIPE pipeHandle, const FMSTR_S8 *parg, FMSTR_PIPE_PRINTF_CTX *pctx)
{
    FMSTR_S8 arg = *parg;

    if (arg < 0)
    {
        pctx->flags.flg.negative = 1U;

        /* if sign will be shown, then negate the number */
        if (pctx->flags.flg.signedtype != 0U)
        {
            arg *= -1;
        }
    }

    /* Coverity: Intentional casting after checks above. */
    /* coverity[misra_c_2012_rule_11_3_violation:FALSE] */
    return FMSTR_PipeU8ToA(pipeHandle, (const FMSTR_U8 *)&arg, pctx);
}

/******************************************************************************
 *
 * @brief  This function formats the argument into the temporary printf buffer
 *         It is granted by the caller that the buffer is empty before calling.
 *
 *****************************************************************************/

static FMSTR_BOOL FMSTR_PipeU16ToA(FMSTR_HPIPE pipeHandle, const FMSTR_U16 *parg, FMSTR_PIPE_PRINTF_CTX *pctx)
{
    FMSTR_PIPE *pp = (FMSTR_PIPE *)pipeHandle;
    FMSTR_U16 arg;
    FMSTR_U8 dig;
    FMSTR_INDEX i;

    /* We need 2 byte argument */
    FMSTR_ASSERT_RETURN(pctx != NULL && pctx->stklen >= sizeof(arg), FMSTR_FALSE);
    arg = *parg;
    
    switch (pctx->radix)
    {
        case FMSTR_PIPE_ITOAFMT_CHAR:
            FMSTR_ASSERT_RETURN(pp->printfBPtr <= (FMSTR_PIPES_PRINTF_BUFF_SIZE-1U), FMSTR_FALSE);
            /* Coverity: Explicit arg testing ensures no risk of misinterpretation of cast. */
            /* coverity[cert_int31_c_violation:FALSE] */
            pp->printfBuff[pp->printfBPtr++] = arg <= 0x00ffU ? (FMSTR_CHAR)arg : (FMSTR_CHAR)'.';
            break;

        case FMSTR_PIPE_ITOAFMT_BIN:
            FMSTR_ASSERT_RETURN(pp->printfBPtr <= (FMSTR_PIPES_PRINTF_BUFF_SIZE-16U), FMSTR_FALSE);
            for (i = 0; i < 16; i++)
            {
                if (arg == 0U)
                {
                    break;
                }

                dig = (FMSTR_U8)'0';
                dig += (FMSTR_U8)(arg & 1U);

                pp->printfBuff[pp->printfBPtr++] = (FMSTR_CHAR)dig;
                arg >>= 1;
            }
            break;

        case FMSTR_PIPE_ITOAFMT_OCT:
            FMSTR_ASSERT_RETURN(pp->printfBPtr <= (FMSTR_PIPES_PRINTF_BUFF_SIZE-6U), FMSTR_FALSE);
            for (i = 0; i < 6; i++)
            {
                if (arg == 0U)
                {
                    break;
                }

                dig = (FMSTR_U8)'0';
                dig += (FMSTR_U8)(arg & 7U);

                pp->printfBuff[pp->printfBPtr++] = (FMSTR_CHAR)dig;
                arg >>= 3;
            }
            break;

        case FMSTR_PIPE_ITOAFMT_DEC:
            FMSTR_ASSERT_RETURN(pp->printfBPtr <= (FMSTR_PIPES_PRINTF_BUFF_SIZE-5U), FMSTR_FALSE);
            for (i = 0; i < 5; i++)
            {
                if (arg == 0U)
                {
                    break;
                }

                dig = (FMSTR_U8)'0';
                dig += (FMSTR_U8)(arg % 10U);

                pp->printfBuff[pp->printfBPtr++] = (FMSTR_CHAR)dig;
                arg /= 10U;
            }
            break;

        case FMSTR_PIPE_ITOAFMT_HEX:
        default:
            FMSTR_ASSERT_RETURN(pp->printfBPtr <= (FMSTR_PIPES_PRINTF_BUFF_SIZE-4U), FMSTR_FALSE);
            for (i = 0; i < 4; i++)
            {
                if (arg == 0U)
                {
                    break;
                }

                dig = (FMSTR_U8)(arg & 15U);

                pp->printfBuff[pp->printfBPtr++] =
                    _FMSTR_XDigit((FMSTR_U8)dig, (FMSTR_BOOL)(pctx->flags.flg.upperc != 0U));
                arg >>= 4;
            }
            break;
    }

    return _FMSTR_PipeIToAFinalize(pipeHandle, pctx);
}

/******************************************************************************
 *
 * @brief  This function formats the argument into the temporary printf buffer
 *         It is granted by the caller that the buffer is empty before calling.
 *
 *****************************************************************************/

static FMSTR_BOOL FMSTR_PipeS16ToA(FMSTR_HPIPE pipeHandle, const FMSTR_S16 *parg, FMSTR_PIPE_PRINTF_CTX *pctx)
{
    FMSTR_S16 arg = *parg;

    if (arg < 0)
    {
        pctx->flags.flg.negative = 1U;

        /* if sign will be shown, then negate the number */
        if (pctx->flags.flg.signedtype != 0U)
        {
            arg *= -1;
        }
    }

    /* Coverity: Intentional casting after checks above. */
    /* coverity[misra_c_2012_rule_11_3_violation:FALSE] */
    return FMSTR_PipeU16ToA(pipeHandle, (const FMSTR_U16 *)&arg, pctx);
}

/******************************************************************************
 *
 * @brief  This function formats the argument into the temporary printf buffer
 *         It is granted by the caller that the buffer is empty before calling.
 *
 *****************************************************************************/

static FMSTR_BOOL FMSTR_PipeU32ToA(FMSTR_HPIPE pipeHandle, const FMSTR_U32 *parg, FMSTR_PIPE_PRINTF_CTX *pctx)
{
    FMSTR_PIPE *pp = (FMSTR_PIPE *)pipeHandle;
    FMSTR_U32 arg;
    FMSTR_U8 dig;
    FMSTR_INDEX i;

    /* We need 2 byte argument */
    FMSTR_ASSERT_RETURN(pctx != NULL && pctx->stklen >= sizeof(arg), FMSTR_FALSE);
    arg = *parg;
    
    switch (pctx->radix)
    {
        case FMSTR_PIPE_ITOAFMT_CHAR:
            FMSTR_ASSERT_RETURN(pp->printfBPtr <= (FMSTR_PIPES_PRINTF_BUFF_SIZE-1U), FMSTR_FALSE);

            /* Coverity: Explicit arg testing ensures no risk of misinterpretation of cast. */
            /* coverity[cert_int31_c_violation:FALSE] */
            pp->printfBuff[pp->printfBPtr++] = arg <= 0xffUL ? (FMSTR_CHAR)arg : (FMSTR_CHAR)'.';
            break;

        case FMSTR_PIPE_ITOAFMT_BIN:
            FMSTR_ASSERT_RETURN(pp->printfBPtr <= (FMSTR_PIPES_PRINTF_BUFF_SIZE-32U), FMSTR_FALSE);
            for (i = 0; i < 32; i++)
            {
                if (arg == 0U)
                {
                    break;
                }

                dig = (FMSTR_U8)'0';
                dig += (FMSTR_U8)(arg & 1U);

                pp->printfBuff[pp->printfBPtr++] = (FMSTR_CHAR)dig;
                arg >>= 1;
            }
            break;

        case FMSTR_PIPE_ITOAFMT_OCT:
            FMSTR_ASSERT_RETURN(pp->printfBPtr <= (FMSTR_PIPES_PRINTF_BUFF_SIZE-11U), FMSTR_FALSE);
            for (i = 0; i < 11; i++)
            {
                if (arg == 0U)
                {
                    break;
                }

                dig = (FMSTR_U8)'0';
                dig += (FMSTR_U8)(arg & 7U);

                pp->printfBuff[pp->printfBPtr++] = (FMSTR_CHAR)dig;
                arg >>= 3;
            }
            break;

        case FMSTR_PIPE_ITOAFMT_DEC:
            FMSTR_ASSERT_RETURN(pp->printfBPtr <= (FMSTR_PIPES_PRINTF_BUFF_SIZE-10U), FMSTR_FALSE);
            for (i = 0; i < 10; i++)
            {
                if (arg == 0U)
                {
                    break;
                }

                dig = (FMSTR_U8)'0';
                dig += (FMSTR_U8)(arg % 10U);

                pp->printfBuff[pp->printfBPtr++] = (FMSTR_CHAR)dig;
                arg /= 10U;
            }
            break;

        case FMSTR_PIPE_ITOAFMT_HEX:
        default:
            FMSTR_ASSERT_RETURN(pp->printfBPtr <= (FMSTR_PIPES_PRINTF_BUFF_SIZE-8U), FMSTR_FALSE);
            for (i = 0; i < 8; i++)
            {
                if (arg == 0U)
                {
                    break;
                }

                dig = (FMSTR_U8)(arg & 15U);

                pp->printfBuff[pp->printfBPtr++] =
                    _FMSTR_XDigit((FMSTR_U8)dig, (FMSTR_BOOL)(pctx->flags.flg.upperc != 0U));
                arg >>= 4;
            }
            break;
    }

    return _FMSTR_PipeIToAFinalize(pipeHandle, pctx);
}

/******************************************************************************
 *
 * @brief  This function formats the argument into the temporary printf buffer
 *         It is granted by the caller that the buffer is empty before calling.
 *
 *****************************************************************************/

static FMSTR_BOOL FMSTR_PipeS32ToA(FMSTR_HPIPE pipeHandle, const FMSTR_S32 *parg, FMSTR_PIPE_PRINTF_CTX *pctx)
{
    FMSTR_S32 arg = *parg;

    if (arg < 0)
    {
        pctx->flags.flg.negative = 1U;

        /* if sign will be shown, then negate the number */
        if (pctx->flags.flg.signedtype != 0U)
        {
            arg *= -1;
        }
    }

    /* Coverity: Intentional casting after checks above. */
    /* coverity[misra_c_2012_rule_11_3_violation:FALSE] */
    return FMSTR_PipeU32ToA(pipeHandle, (const FMSTR_U32 *)&arg, pctx);
}

/******************************************************************************
 *
 * @brief  This function parses the printf format and sets the context
 *         structure properly.
 *
 * @return The function returns the pointer to end of format string handled
 *
 *****************************************************************************/

static const char *FMSTR_PipeParseFormat(const char *format, FMSTR_PIPE_PRINTF_CTX *pctx)
{
    FMSTR_CHAR c;

    pctx->flags.all = 0;

    /* skip percent sign */
    if (*format == '%')
    {
        format++;
    }

    /* show sign always? */
    if (*format == '+')
    {
        pctx->flags.flg.showsign = 1U;
        format++;
    }

    /* prefix with zeroes? */
    if (*format == '0')
    {
        pctx->flags.flg.zeroes = 1U;
        format++;
    }

    /* parse print length */
    pctx->alen = 0;
    while (FMSTR_IS_DIGIT(*format))
    {
        c = *format++;
        c -= '0';
        /* Coverity: Intentional. The 'alen' may wrap here, only causing a misformatted value. 
           The value is checked later during Finalize. Also casting 'c' to unsigned
           size is intentional here as it was already checked to be a digit char. */
        /* coverity[cert_int30_c_violation:FALSE] */
        pctx->alen *= 10U;
        /* coverity[cert_int30_c_violation:FALSE] */
        /* coverity[cert_int31_c_violation:FALSE] */
        pctx->alen += (FMSTR_SIZE)c;
    }

    /* parse dtsize modifier */
    switch (*format)
    {
        /* short modifier (char for hh)*/
        case 'h':
            pctx->dtsize = (FMSTR_U8)sizeof(short);
            format++;

            /* one more 'h' means 'char' */
            if (*format == 'h')
            {
                pctx->dtsize = (FMSTR_U8)sizeof(char);
                format++;
            }
            break;

        case 'l':
            pctx->dtsize = (FMSTR_U8)sizeof(long);
            format++;
            break;

        default:
            /* default data type is 'int' */
            pctx->dtsize = (FMSTR_U8)sizeof(int);
            break;
    }

    /* now finally concluding to format letter */
    switch (*format++)
    {
        /* HEXADECIMAL */
        case 'X':
            pctx->flags.flg.upperc = 1U;
            pctx->radix = FMSTR_PIPE_ITOAFMT_HEX;
            break;

        /* hexadecimal */
        case 'x':
            pctx->radix = FMSTR_PIPE_ITOAFMT_HEX;
            break;

        /* octal */
        case 'o':
            pctx->radix = FMSTR_PIPE_ITOAFMT_OCT;
            break;

        /* binary */
        case 'b':
            pctx->radix = FMSTR_PIPE_ITOAFMT_BIN;
            break;

        /* decimal signed */
        case 'd':
        case 'i':
            pctx->flags.flg.signedtype = 1U;
            pctx->radix = FMSTR_PIPE_ITOAFMT_DEC;
            break;

        /* decimal unsigned */
        case 'u':
            pctx->radix = FMSTR_PIPE_ITOAFMT_DEC;
            break;

        /* character */
        case 'c':
            pctx->radix  = FMSTR_PIPE_ITOAFMT_CHAR;
            pctx->dtsize = (FMSTR_U8)sizeof(char);
            break;

        /* string */
        case 's':
            pctx->flags.flg.isstring = 1U;
            pctx->dtsize = (FMSTR_U8)sizeof(void *);
            break;

        /* unknown */
        default:
            FMSTR_ASSERT(0 == 1);
            pctx->radix = FMSTR_PIPE_ITOAFMT_HEX;
            break;
    }

    return format;
}

/******************************************************************************
 *
 * @brief  Printf with one argument passed by pointer.
 *
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_PipePrintfOne(FMSTR_HPIPE pipeHandle,
                                       const char *format,
                                       void *parg, FMSTR_SIZE argSize,
                                       FMSTR_PIPE_ITOA_FUNC pItoaFunc)
{
    FMSTR_BOOL ok = FMSTR_TRUE;
    FMSTR_PIPE_PRINTF_CTX ctx;

    while (*format != (FMSTR_CHAR)0 && ok != FMSTR_FALSE)
    {
        if (*format == '%')
        {
            format++;

            if (*format == '%')
            {
                ok = _FMSTR_PipePrintfPutc(pipeHandle, '%');
                format++;
                continue;
            }

            /* empty the pipe's temporary buffer */
            ok = _FMSTR_PipePrintfFlush(pipeHandle);

            if (ok != FMSTR_FALSE)
            {
                format = FMSTR_PipeParseFormat(format, &ctx);
                ctx.stklen = argSize;

                if (ctx.flags.flg.isstring != 0U)
                {
                    const char *psz;
                    psz = (const char *)parg;
                    ok  = FMSTR_PipePuts(pipeHandle, psz != NULL ? psz : "NULL");
                }
                else
                {
                    ok = pItoaFunc(pipeHandle, parg, &ctx);
                }
            }
        }
        else
        {
            ok = _FMSTR_PipePrintfPutc(pipeHandle, *format++);
        }
    }

    if (ok != FMSTR_FALSE)
    {
        ok = _FMSTR_PipePrintfFlush(pipeHandle);
    }

    return (FMSTR_BOOL)(ok != FMSTR_FALSE);
}

/******************************************************************************
 *
 * @brief  PIPE API: Format argument into the pipe output stream. The format
 *         follows the standard printf format. The leading '%' is optional.
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_PipePrintfU8(FMSTR_HPIPE pipeHandle, const char *format, FMSTR_U8 arg)
{
    /* Coverity: Intentional cast of static function to a generic function pointer. */
    /* coverity[misra_c_2012_rule_11_1_violation:FALSE] */
    return _FMSTR_PipePrintfOne(pipeHandle, format, &arg, (FMSTR_SIZE)sizeof(arg), (FMSTR_PIPE_ITOA_FUNC)FMSTR_PipeU8ToA);
}

/******************************************************************************
 *
 * @brief  PIPE API: Format argument into the pipe output stream. The format
 *         follows the standard printf format. The leading '%' is optional.
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_PipePrintfS8(FMSTR_HPIPE pipeHandle, const char *format, FMSTR_S8 arg)
{
    /* Coverity: Intentional cast of static function to a generic function pointer. */
    /* coverity[misra_c_2012_rule_11_1_violation:FALSE] */
    return _FMSTR_PipePrintfOne(pipeHandle, format, &arg, (FMSTR_SIZE)sizeof(arg), (FMSTR_PIPE_ITOA_FUNC)FMSTR_PipeS8ToA);
}

/******************************************************************************
 *
 * @brief  PIPE API: Format argument into the pipe output stream. The format
 *         follows the standard printf format. The leading '%' is optional.
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_PipePrintfU16(FMSTR_HPIPE pipeHandle, const char *format, FMSTR_U16 arg)
{
    /* Coverity: Intentional cast of static function to a generic function pointer. */
    /* coverity[misra_c_2012_rule_11_1_violation:FALSE] */
    return _FMSTR_PipePrintfOne(pipeHandle, format, &arg, (FMSTR_SIZE)sizeof(arg), (FMSTR_PIPE_ITOA_FUNC)FMSTR_PipeU16ToA);
}

/******************************************************************************
 *
 * @brief  PIPE API: Format argument into the pipe output stream. The format
 *         follows the standard printf format. The leading '%' is optional.
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_PipePrintfS16(FMSTR_HPIPE pipeHandle, const char *format, FMSTR_S16 arg)
{
    /* Coverity: Intentional cast of static function to a generic function pointer. */
    /* coverity[misra_c_2012_rule_11_1_violation:FALSE] */
    return _FMSTR_PipePrintfOne(pipeHandle, format, &arg, (FMSTR_SIZE)sizeof(arg), (FMSTR_PIPE_ITOA_FUNC)FMSTR_PipeS16ToA);
}

/******************************************************************************
 *
 * @brief  PIPE API: Format argument into the pipe output stream. The format
 *         follows the standard printf format. The leading '%' is optional.
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_PipePrintfU32(FMSTR_HPIPE pipeHandle, const char *format, FMSTR_U32 arg)
{
    /* Coverity: Intentional cast of static function to a generic function pointer. */
    /* coverity[misra_c_2012_rule_11_1_violation:FALSE] */
    return _FMSTR_PipePrintfOne(pipeHandle, format, &arg, (FMSTR_SIZE)sizeof(arg), (FMSTR_PIPE_ITOA_FUNC)FMSTR_PipeU32ToA);
}

/******************************************************************************
 *
 * @brief  PIPE API: Format argument into the pipe output stream. The format
 *         follows the standard printf format. The leading '%' is optional.
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_PipePrintfS32(FMSTR_HPIPE pipeHandle, const char *format, FMSTR_S32 arg)
{
    /* Coverity: Intentional cast of static function to a generic function pointer. */
    /* coverity[misra_c_2012_rule_11_1_violation:FALSE] */
    return _FMSTR_PipePrintfOne(pipeHandle, format, &arg, (FMSTR_SIZE)sizeof(arg), (FMSTR_PIPE_ITOA_FUNC)FMSTR_PipeS32ToA);
}

#if FMSTR_USE_PIPE_PRINTF_VARG > 0

/******************************************************************************
 *
 * @brief  Format va_list argument into the pipe output stream. This function
 *         is called as a part of our printf routine.
 *
 ******************************************************************************/
static FMSTR_BOOL _FMSTR_PipePrintfAny(FMSTR_HPIPE pipeHandle, va_list *parg, FMSTR_PIPE_PRINTF_CTX *pctx)
{
    FMSTR_BOOL ok = FMSTR_FALSE;

    switch (pctx->dtsize)
    {
        case 1:
            if (pctx->flags.flg.signedtype != 0U)
            {
                /* Coverity: Intentional cast from va_arg/int to S8. */
                /* coverity[cert_int31_c_violation:FALSE] */
                /* coverity[misra_c_2012_rule_17_1_violation:FALSE] */
                FMSTR_S8 arg = (FMSTR_S8)va_arg(*parg, int);
                ok           = FMSTR_PipeS8ToA(pipeHandle, &arg, pctx);
            }
            else
            {
                /* Coverity: Intentional cast from va_arg/unsigned int to U8. */
                /* coverity[cert_int31_c_violation:FALSE] */
                /* coverity[misra_c_2012_rule_17_1_violation:FALSE] */
                FMSTR_U8 arg = (FMSTR_U8)va_arg(*parg, unsigned);
                ok           = FMSTR_PipeU8ToA(pipeHandle, &arg, pctx);
            }
            break;

        case 2:
            if (pctx->flags.flg.signedtype != 0U)
            {
                /* Coverity: Intentional cast from va_arg/int to S16. */
                /* coverity[cert_int31_c_violation:FALSE] */
                /* coverity[misra_c_2012_rule_17_1_violation:FALSE] */
                FMSTR_S16 arg = (FMSTR_S16)va_arg(*parg, int);
                ok            = FMSTR_PipeS16ToA(pipeHandle, &arg, pctx);
            }
            else
            {
                /* Coverity: Intentional cast from va_arg/unsigned int to U16. */
                /* coverity[cert_int31_c_violation:FALSE] */
                /* coverity[misra_c_2012_rule_17_1_violation:FALSE] */
                FMSTR_U16 arg = (FMSTR_U16)va_arg(*parg, unsigned);
                ok            = FMSTR_PipeU16ToA(pipeHandle, &arg, pctx);
            }
            break;

        case 4:
            if (pctx->flags.flg.signedtype != 0U)
            {
                /* coverity[misra_c_2012_rule_17_1_violation:FALSE] */
                FMSTR_S32 arg = (FMSTR_S32)va_arg(*parg, long);
                ok            = FMSTR_PipeS32ToA(pipeHandle, &arg, pctx);
            }
            else
            {
                /* coverity[misra_c_2012_rule_17_1_violation:FALSE] */
                FMSTR_U32 arg = (FMSTR_U32)va_arg(*parg, unsigned long);
                ok            = FMSTR_PipeU32ToA(pipeHandle, &arg, pctx);
            }
            break;

        default:
            ok = FMSTR_FALSE;
            break;
    }

    return ok;
}

/******************************************************************************
 *
 * @brief  Printf with va_list arguments prepared.
 *
 *         This function is not declared static (may be reused as global),
 *         but public prototype is not available (not to force user to
 *         have va_list defined.
 *
 ******************************************************************************/

static FMSTR_BOOL _FMSTR_PipePrintfV(FMSTR_HPIPE pipeHandle, const char *format, va_list *parg)
{
    FMSTR_BOOL ok = FMSTR_TRUE;
    FMSTR_PIPE_PRINTF_CTX ctx;

    while (*format != (FMSTR_CHAR)0 && ok != FMSTR_FALSE)
    {
        if (*format == '%')
        {
            format++;

            if (*format == '%')
            {
                ok = _FMSTR_PipePrintfPutc(pipeHandle, '%');
                format++;
            }
            else
            {
                ok = _FMSTR_PipePrintfFlush(pipeHandle);

                if (ok != FMSTR_FALSE)
                {
                    format = FMSTR_PipeParseFormat(format, &ctx);

                    if (ctx.flags.flg.isstring != 0U)
                    {
                        /* coverity[misra_c_2012_rule_17_1_violation:FALSE] */
                        const char *psz = va_arg(*parg, char *);
                        ok = FMSTR_PipePuts(pipeHandle, psz != NULL ? psz : "NULL");
                    }
                    else
                    {
                        /* Size of va_list argument */
                        ctx.stklen = 4;
                        ok = _FMSTR_PipePrintfAny(pipeHandle, parg, &ctx);
                    }
                }
            }
        }
        else
        {
            ok = _FMSTR_PipePrintfPutc(pipeHandle, *format++);
        }
    }

    if (ok != FMSTR_FALSE)
    {
        ok = _FMSTR_PipePrintfFlush(pipeHandle);
    }

    return (FMSTR_BOOL)(ok != FMSTR_FALSE);
}

/******************************************************************************
 *
 * @brief  PIPE API: The printf into the pipe
 *
 ******************************************************************************/

FMSTR_BOOL FMSTR_PipePrintf(FMSTR_HPIPE pipeHandle, const char *format, ...)
{
    FMSTR_BOOL ok;

    /* coverity[misra_c_2012_rule_17_1_violation:FALSE] */
    va_list args;
    /* coverity[misra_c_2012_rule_17_1_violation:FALSE] */
    va_start(args, format);

    /* Coverity: Intentional use of va_list to mimic the printf behavior. 
       Can be disabled by USE_PIPE_PRINTF when unused. */
    /* coverity[cert_exp47_c_violation:FALSE] */
    ok = _FMSTR_PipePrintfV(pipeHandle, format, &args);

    /* coverity[misra_c_2012_rule_17_1_violation:FALSE] */
    va_end(args);

    return ok;
}

#endif /* FMSTR_USE_PIPE_PRINTF_VARG */
#endif /* FMSTR_USE_PIPE_PRINTF */

/******************************************************************************
 *
 * @brief  PIPE API: Read data from a pipe
 *
 ******************************************************************************/

FMSTR_PIPE_SIZE FMSTR_PipeRead(FMSTR_HPIPE pipeHandle,
                               FMSTR_ADDR pipeData,
                               FMSTR_PIPE_SIZE pipeDataLen,
                               FMSTR_PIPE_SIZE readGranularity)
{
    FMSTR_PIPE *pp         = (FMSTR_PIPE *)pipeHandle;
    FMSTR_PIPE_BUFF *pbuff = &pp->rx;
    FMSTR_PIPE_SIZE total  = _FMSTR_PipeGetBytesReady(pbuff);
    FMSTR_PIPE_SIZE s;

    /* when invalid address is given, only return number of bytes available */
    if (FMSTR_ADDR_VALID(pipeData) != FMSTR_FALSE)
    {
        /* round length to bus width */
        pipeDataLen /= FMSTR_CFG_BUS_WIDTH;
        pipeDataLen *= FMSTR_CFG_BUS_WIDTH;

        /* only fetch what we have cached */
        if (pipeDataLen > total)
        {
            pipeDataLen = total;
        }

        /* obey granularity */
        if (readGranularity > 1U)
        {
            pipeDataLen /= readGranularity;
            pipeDataLen *= readGranularity;
        }

        /* return value */
        total = pipeDataLen;

        /* rest of cyclic buffer */
        if (pipeDataLen > 0U)
        {
            /* Calculate total bytes available in the rest of buffer */
          
            /* Coverity: Construction of the circular buffer logic ensures that this operation is 
               positive and cannot wrap. No risk of signed/unsigned cast misinterpretation. */
            /* coverity[cert_int31_c_violation:FALSE] */
            s = (FMSTR_PIPE_SIZE)((pbuff->size - pbuff->rp) * FMSTR_CFG_BUS_WIDTH);
            
            if (s > pipeDataLen)
            {
                s = pipeDataLen;
            }

            /* put bytes */
            FMSTR_MemCpyTo(pipeData, pbuff->buff + pbuff->rp, (FMSTR_SIZE8)s);
            pipeData += s / FMSTR_CFG_BUS_WIDTH;

            /* Advance & wrap pointer to start of the buffer if needed */

            /* Coverity: Conditions above and construction of the buffer logic ensures 
               that rp cannot wrap around maximum value. Wrapping around a buffer size is 
               intentional in the condition below. */
            /* coverity[cert_int30_c_violation:FALSE] */
            pbuff->rp += s / FMSTR_CFG_BUS_WIDTH;
            if (pbuff->rp >= pbuff->size)
            {
                pbuff->rp = 0;
            }

            /* rest of frame to a (wrapped) beggining of buffer */
            pipeDataLen -= (FMSTR_SIZE8)s;
            if (pipeDataLen > 0U)
            {
                FMSTR_MemCpyTo(pipeData, pbuff->buff + pbuff->rp, (FMSTR_SIZE8)pipeDataLen);
                pbuff->rp += pipeDataLen / FMSTR_CFG_BUS_WIDTH;
            }

            /* buffer for sure not full now */
            pbuff->flags.flg.bIsFull = 0;
        }
    }

    return total;
}

/******************************************************************************
 *
 * @brief  Find pipe by port number
 *
 ******************************************************************************/

static FMSTR_PIPE *_FMSTR_FindPipe(FMSTR_PIPE_PORT pipePort)
{
    FMSTR_PIPE *pp;
    FMSTR_INDEX i;

    for (i = 0; i < (FMSTR_INDEX)FMSTR_USE_PIPES; i++)
    {
        pp = &pcm_pipes[i];

        /* look for existing pipe with the same port */
        if (pp->pipePort == pipePort)
        {
            return pp;
        }
    }

    return NULL;
}

/******************************************************************************
 *
 * @brief  Find pipe index by port number
 *
 ******************************************************************************/

FMSTR_INDEX FMSTR_FindPipeIndex(FMSTR_PIPE_PORT pipePort)
{
    FMSTR_PIPE *pp;
    FMSTR_INDEX i;

    for (i = 0; i < (FMSTR_INDEX)FMSTR_USE_PIPES; i++)
    {
        pp = &pcm_pipes[i];

        /* look for existing pipe with the same port */
        if (pp->pipePort == pipePort)
        {
            return i;
        }
    }

    return -1;
}

/******************************************************************************
 *
 * @brief  Get number of bytes free in the buffer
 *
 ******************************************************************************/

static FMSTR_PIPE_SIZE _FMSTR_PipeGetBytesFree(FMSTR_PIPE_BUFF *pipeBuff)
{
    FMSTR_PIPE_SIZE szFree;
    
    if (pipeBuff->flags.flg.bIsFull != 0U)
    {
        szFree = 0;
    }
    else if (pipeBuff->wp < pipeBuff->rp)
    {
        szFree = (FMSTR_PIPE_SIZE)(pipeBuff->rp - pipeBuff->wp);
    }
    else
    {
        /* Coverity: Intentional. Construction of the circular buffer and the 
           conditions above ensure the expression does not wrap. */
        /* coverity[cert_int31_c_violation:FALSE] */
        szFree = (FMSTR_PIPE_SIZE)(pipeBuff->size - pipeBuff->wp + pipeBuff->rp);
    }

    return (FMSTR_PIPE_SIZE)(szFree * FMSTR_CFG_BUS_WIDTH);
}

static FMSTR_PIPE_SIZE _FMSTR_PipeGetBytesReady(FMSTR_PIPE_BUFF *pipeBuff)
{
    FMSTR_PIPE_SIZE szFull;

    if (pipeBuff->flags.flg.bIsFull != 0U)
    {
        szFull = (FMSTR_PIPE_SIZE)pipeBuff->size;
    }
    else if (pipeBuff->wp >= pipeBuff->rp)
    {
        szFull = (FMSTR_PIPE_SIZE)(pipeBuff->wp - pipeBuff->rp);
    }
    else
    {
        /* Coverity: Intentional. Construction of the circular buffer and the 
           conditions above ensure the expression does not wrap. */
        /* coverity[cert_int31_c_violation:FALSE] */
        szFull = (FMSTR_PIPE_SIZE)(pipeBuff->size - pipeBuff->rp + pipeBuff->wp);
    }

    return (FMSTR_PIPE_SIZE)(szFull * FMSTR_CFG_BUS_WIDTH);
}

static void _FMSTR_PipeDiscardBytes(FMSTR_PIPE_BUFF *pipeBuff, FMSTR_SIZE8 countBytes)
{
    FMSTR_PIPE_SIZE total;
    FMSTR_PIPE_SIZE discard;

    total   = _FMSTR_PipeGetBytesReady(pipeBuff);
    discard = (FMSTR_PIPE_SIZE)(countBytes > total ? total : countBytes);
    discard /= FMSTR_CFG_BUS_WIDTH;

    if (discard > 0U)
    {
        /* Coverity: Intentional. By design of the circular buffer, the 'rp' is always smaller than 'size'.
           and there is no risk of misinterpretation during cast operations below. */
        /* coverity[cert_int31_c_violation:FALSE] */
        FMSTR_PIPE_SIZE rest = (FMSTR_PIPE_SIZE)(pipeBuff->size - pipeBuff->rp);
        FMSTR_PIPE_SIZE rp;

        /* will RP wrap? */
        if (rest <= discard)
        {
            /* coverity[cert_int31_c_violation:FALSE] */
            rp = (FMSTR_PIPE_SIZE)(discard - rest);
        }
        else
        {
            /* coverity[cert_int31_c_violation:FALSE] */
            rp = (FMSTR_PIPE_SIZE)(pipeBuff->rp + discard);
        }

        /* buffer is for sure not full */
        pipeBuff->flags.flg.bIsFull = 0;
        pipeBuff->rp                = rp;
    }
}

/* get data from frame into our Rx buffer, we are already sure it fits */

static FMSTR_BPTR _FMSTR_PipeReceive(FMSTR_BPTR msgBuffIO, FMSTR_PIPE *pp, FMSTR_SIZE8 msgBuffSize)
{
    FMSTR_PIPE_BUFF *pbuff = &pp->rx;
    FMSTR_PIPE_SIZE s;

    if (msgBuffSize > 0U)
    {
        /* Calculate total bytes available in the rest of buffer */
      
        /* Coverity: Construction of the circular buffer logic ensures that this operation is 
           positive and cannot wrap. No risk of signed/unsigned cast misinterpretation. */
        /* coverity[cert_int31_c_violation:FALSE] */
        s = (FMSTR_PIPE_SIZE)((pbuff->size - pbuff->wp) * FMSTR_CFG_BUS_WIDTH);
        if (s > (FMSTR_PIPE_SIZE)msgBuffSize)
        {
            s = (FMSTR_PIPE_SIZE)msgBuffSize;
        }

        /* get the bytes */
        msgBuffIO = FMSTR_CopyFromBuffer(pbuff->buff + pbuff->wp, msgBuffIO, (FMSTR_SIZE8)s);

        /* Advance & wrap pointer to start of the buffer if needed */
        
        /* Coverity: Conditions above and construction of the buffer logic ensures 
           that wp cannot wrap around maximum value. Wrapping around a buffer size is 
           intentional in the condition below. */
        /* coverity[cert_int30_c_violation:FALSE] */
        pbuff->wp += s / FMSTR_CFG_BUS_WIDTH;
        if (pbuff->wp >= pbuff->size)
        {
            pbuff->wp = 0;
        }

        /* rest of frame to a (wrapped) beginning of buffer */
        msgBuffSize -= (FMSTR_SIZE8)s;
        if (msgBuffSize > 0U)
        {
            msgBuffIO = FMSTR_CopyFromBuffer(pbuff->buff + pbuff->wp, msgBuffIO, msgBuffSize);
            pbuff->wp += ((FMSTR_PIPE_SIZE)msgBuffSize) / FMSTR_CFG_BUS_WIDTH;
        }

        /* buffer got full? */
        if (pbuff->wp == pbuff->rp)
        {
            pbuff->flags.flg.bIsFull = 1;
        }
    }

    return msgBuffIO;
}

/* put data into the comm buffer, we are already sure it fits, buffer's RP is not modified */

static FMSTR_BPTR _FMSTR_PipeTransmit(FMSTR_BPTR msgBuffIO, FMSTR_PIPE *pp, FMSTR_SIZE8 msgBuffSize)
{
    FMSTR_PIPE_BUFF *pbuff = &pp->tx;
    FMSTR_PIPE_SIZE s, rp = pbuff->rp;

    if (msgBuffSize > 0U)
    {
        /* Calculate total bytes available in the rest of buffer */

        /* Coverity: Construction of the circular buffer logic ensures that this operation is 
           positive and cannot wrap. No risk of signed/unsigned cast misinterpretation. */
        /* coverity[cert_int31_c_violation:FALSE] */
        s = (FMSTR_PIPE_SIZE)((pbuff->size - rp) * FMSTR_CFG_BUS_WIDTH);
        if (s > (FMSTR_PIPE_SIZE)msgBuffSize)
        {
            s = (FMSTR_PIPE_SIZE)msgBuffSize;
        }

        /* put bytes */
        msgBuffIO = FMSTR_CopyToBuffer(msgBuffIO, pbuff->buff + rp, (FMSTR_SIZE8)s);

        /* Advance & wrap pointer to start of the buffer if needed */
        
        /* Coverity: Conditions above and construction of the buffer logic ensures 
           that rp cannot wrap around maximum value. Wrapping around a buffer size is 
           intentional in the condition below. */
        /* coverity[cert_int30_c_violation:FALSE] */
        rp += s / FMSTR_CFG_BUS_WIDTH;
        if (rp >= pbuff->size)
        {
            rp = 0;
        }

        /* rest of frame to a (wrapped) beginning of buffer */
        msgBuffSize -= (FMSTR_SIZE8)s;
        if (msgBuffSize > 0U)
        {
            msgBuffIO = FMSTR_CopyToBuffer(msgBuffIO, pbuff->buff + rp, msgBuffSize);
        }
    }

    return msgBuffIO;
}

/******************************************************************************
 *
 * @brief  Get PIPE info
 *
 * @param    session - transport session
 * @param    msgBuffIO - original command (in) and response buffer (out)
 * @param    msgSize   - size of data in buffer
 * @param    retStatus - response status
 *
 * @param  msgBuffIO - original command (in) and response buffer (out)
 *
 ******************************************************************************/

FMSTR_BPTR FMSTR_GetPipe(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_SIZE msgSize, FMSTR_U8 *retStatus)
{
    FMSTR_BPTR response = msgBuffIO;
    FMSTR_U8 cfgCode, pipeIndex, pipeFlags;
    FMSTR_PIPE *pp;

    FMSTR_UNUSED(session);

    FMSTR_ASSERT(msgBuffIO != NULL);
    FMSTR_ASSERT(retStatus != NULL);

    /* need at least pipe index and cfgCode */
    if (msgSize < 2U)
    {
        /* return status  */
        *retStatus = FMSTR_STC_PIPEERR;
        return response;
    }

    /* get Pipe flags */
    msgBuffIO = FMSTR_ValueFromBuffer8(&pipeFlags, msgBuffIO);

    /* get Pipe index */
    msgBuffIO = FMSTR_ValueFromBuffer8(&pipeIndex, msgBuffIO);

    /* get Pipe cfgCode */
    msgBuffIO = FMSTR_ValueFromBuffer8(&cfgCode, msgBuffIO);

    /* Pipe index is a port number of a pipe */
    if ((pipeFlags & FMSTR_PIPE_GETPIPE_FLAG_PORT) != 0U)
    {
        /* Find pipe by port number */
        pp = _FMSTR_FindPipe(pipeIndex);
        if (pp == NULL)
        {
            /* return status  */
            *retStatus = FMSTR_STC_PIPEERR;
            return response;
        }
    }
    else
    {
        /* Check maximum pipe index */
        if (pipeIndex >= (FMSTR_U8)FMSTR_USE_PIPES)
        {
            /* return status  */
            *retStatus = FMSTR_STC_INSTERR;
            return response;
        }

        /* Get pipe from the list */
        pp = &pcm_pipes[pipeIndex];
    }

    switch (cfgCode)
    {
        case FMSTR_PIPE_CFGCODE_NAME:
            /* Put pipe name */
            if (pp->name != NULL)
            {
                /* Coverity: Intentional cast of const char* to FMSTR_ADDR. */
                /* coverity[misra_c_2012_rule_11_8_violation:FALSE] */
                response = FMSTR_CopyToBuffer(response, (FMSTR_ADDR)pp->name, (FMSTR_SIZE)FMSTR_StrLen(pp->name));
            }
            break;

        case FMSTR_PIPE_CFGCODE_INFO:
        default:
            pipeFlags = 0;

            /* Add flag isOpen, when pipe is opened (has port number) */
            if (pp->pipePort != 0U)
            {
                pipeFlags |= FMSTR_PIPE_GETINFO_FLAG_ISOPEN;
            }

            /* Put pipe port */
            response = FMSTR_ValueToBuffer8(response, pp->pipePort);
            /* Put pipe type */
            response = FMSTR_ValueToBuffer8(response, pp->type);
            /* Put pipe flags */
            response = FMSTR_ValueToBuffer8(response, pipeFlags);
            break;
    }

    /* success  */
    *retStatus = FMSTR_STS_OK | FMSTR_STSF_VARLEN;
    return response;
}

/******************************************************************************
 *
 * @brief  Handling PIPE commands
 *
 * @param    session - transport session
 * @param    msgBuffIO - original command (in) and response buffer (out)
 * @param    msgSize   - size of data in buffer
 * @param    retStatus - response status
 *
 * @param  msgBuffIO - original command (in) and response buffer (out)
 *
 ******************************************************************************/

FMSTR_BPTR FMSTR_PipeFrame(FMSTR_SESSION *session, FMSTR_BPTR msgBuffIO, FMSTR_SIZE msgSize, FMSTR_U8 *retStatus)
{
    FMSTR_BPTR response = msgBuffIO;
    FMSTR_U8 skipLen, pipePort;
    FMSTR_PIPE *pp;

    FMSTR_ASSERT(msgBuffIO != NULL);
    FMSTR_ASSERT(retStatus != NULL);

    /* Need at least port number and tx-discard bytes. Also must keep length to fit size8 type. */
    if (msgSize < 1U || msgSize > 0xFFU)
    {
        /* return status  */
        *retStatus = FMSTR_STC_PIPEERR;
        return response;
    }

    /* get port number and even/odd flag */
    msgBuffIO = FMSTR_ValueFromBuffer8(&pipePort, msgBuffIO);

#if FMSTR_SESSION_COUNT > 1
    /* Is feature locked by me */
    if (FMSTR_IsFeatureOwned(session, FMSTR_FEATURE_PIPE, (FMSTR_U8)(pipePort & 0x7fU)) == FMSTR_FALSE)
    {
        *retStatus = FMSTR_STC_SERVBUSY;
        return response;
    }
#else
    FMSTR_UNUSED(session);
#endif

    /* get pipe by port */
    pp = _FMSTR_FindPipe((FMSTR_PIPE_PORT)(((FMSTR_PIPE_PORT)pipePort) & 0x7fU));

    /* pipe port must exist (i.e. be open) */
    if (pp == NULL)
    {
        /* return status  */
        *retStatus = FMSTR_STC_PIPEERR;
        return response;
    }

    /* data-in are valid only in "matching" request (even to even, odd to odd) */
    if ((pipePort & 0x80U) != 0U)
    {
        if (pp->flags.flg.bExpectOdd == 0U)
        {
            /* Discard invalid frame */
            msgSize = 0U;
        }
        else
        {
            /* Toggle for next time */
            pp->flags.flg.bExpectOdd = 0U;
        }
    }
    else
    {
        if (pp->flags.flg.bExpectOdd != 0U)
        {
            /* Discard invalid frame */
            msgSize = 0U;
        }
        else
        {
            /* Toggle for next time */
            pp->flags.flg.bExpectOdd = 1U;
        }
    }

    /* process received data */
    if (msgSize > 0U)
    {
        /* first byte tells me how many output bytes can be discarded from my
           pipe-transmit buffer (this is how PC acknowledges how many bytes it
           received and saved from the last response) */
        msgBuffIO = FMSTR_ValueFromBuffer8(&skipLen, msgBuffIO);

        /* discard bytes from pipe's transmit buffer */
        if (skipLen > 0U)
        {
            _FMSTR_PipeDiscardBytes(&pp->tx, skipLen);
        }

        /* next come (msgSize-2) bytes to be received */
        if (msgSize > 2U)
        {
            /* how many bytes may I accept? */
            FMSTR_PIPE_SIZE rxFree = _FMSTR_PipeGetBytesFree(&pp->rx);
            /* how many bytes PC want to push? */
            FMSTR_U8 rxToRead = (FMSTR_U8)(msgSize - 2U);

            /* round to bus width */
            rxToRead /= FMSTR_CFG_BUS_WIDTH;
            rxToRead *= FMSTR_CFG_BUS_WIDTH;

            /* get the lower of the two numbers */
            if (rxFree < (FMSTR_PIPE_SIZE)rxToRead)
            {
                rxToRead = (FMSTR_U8)rxFree;
            }

            /* get frame data */
            msgBuffIO = _FMSTR_PipeReceive(msgBuffIO, pp, rxToRead);
            FMSTR_UNUSED(msgBuffIO);

            /* this is the number to be returned to PC to inform it how
               many bytes it may discard in his transmit buffer */
            pp->nLastBytesReceived = rxToRead;
        }
        else
        {
            /* no bytes received */
            pp->nLastBytesReceived = 0;
        }
    }

    /* now call the pipe's handler, it may read or write data */
    if (pp->pCallback != NULL)
    {
        pp->flags.flg.bInComm = 1;
        pp->pCallback((FMSTR_HPIPE)pp);
        pp->flags.flg.bInComm = 0;
    }

    /* now put our output data */
    {
        /* how many bytes are waiting to be sent? */
        FMSTR_PIPE_SIZE txAvail = _FMSTR_PipeGetBytesReady(&pp->tx);
        /* how many bytes can we safely put? */
        FMSTR_SIZE txToSend = (FMSTR_SIZE)FMSTR_COMM_BUFFER_SIZE - 3U;

        /* round to bus width */
        txToSend /= FMSTR_CFG_BUS_WIDTH;
        txToSend *= FMSTR_CFG_BUS_WIDTH;

        /* get the lower of two values */
        if (((FMSTR_SIZE)txAvail) < txToSend)
        {
            txToSend = (FMSTR_SIZE)txAvail;
        }
        
        /* Pipe uses 8-bit lengths only, this sanity check shall never happen, but anyway */
        if(txToSend > 0xffU)
        {
            /* Coverity: Intentional condition which may be evaluated as always false. */
            /* coverity[misra_c_2012_rule_14_3_violation:FALSE] */
            txToSend = 0xffU;
        }

        /* send pipe's transmit data back */
        response = FMSTR_ValueToBuffer8(response, pipePort);

        /* inform PC how many bytes it may discard from its pipe's transmit buffer */
        skipLen  = pp->nLastBytesReceived;
        response = FMSTR_ValueToBuffer8(response, skipLen);

        /* put data */
        if (txToSend != 0U)
        {
            response = _FMSTR_PipeTransmit(response, pp, (FMSTR_SIZE8)txToSend);
        }
    }

    /* success  */
    *retStatus = FMSTR_STS_OK | FMSTR_STSF_VARLEN;
    return response;
}

#else /* FMSTR_USE_PIPES && (!FMSTR_DISABLE) */

/* implement void pipe-API functions */

FMSTR_HPIPE FMSTR_PipeOpen(FMSTR_PIPE_PORT pipePort,
                           FMSTR_PPIPEFUNC pipeCallback,
                           FMSTR_ADDR pipeRxBuff,
                           FMSTR_PIPE_SIZE pipeRxSize,
                           FMSTR_ADDR pipeTxBuff,
                           FMSTR_PIPE_SIZE pipeTxSize,
                           FMSTR_U8 type,
                           const FMSTR_CHAR *name)
{
    FMSTR_UNUSED(pipePort);
    FMSTR_UNUSED(pipeCallback);
    FMSTR_UNUSED(pipeRxBuff);
    FMSTR_UNUSED(pipeRxSize);
    FMSTR_UNUSED(pipeTxBuff);
    FMSTR_UNUSED(pipeTxSize);
    FMSTR_UNUSED(type);
    FMSTR_UNUSED(name);

    return NULL;
}

void FMSTR_PipeClose(FMSTR_HPIPE pipeHandle)
{
    FMSTR_UNUSED(pipeHandle);
}

FMSTR_PIPE_SIZE FMSTR_PipeWrite(FMSTR_HPIPE pipeHandle,
                                FMSTR_ADDR pipeData,
                                FMSTR_PIPE_SIZE pipeDataLen,
                                FMSTR_PIPE_SIZE writeGranularity)
{
    FMSTR_UNUSED(pipeHandle);
    FMSTR_UNUSED(pipeData);
    FMSTR_UNUSED(pipeDataLen);
    FMSTR_UNUSED(writeGranularity);

    return 0U;
}

FMSTR_PIPE_SIZE FMSTR_PipeRead(FMSTR_HPIPE pipeHandle,
                               FMSTR_ADDR pipeData,
                               FMSTR_PIPE_SIZE pipeDataLen,
                               FMSTR_PIPE_SIZE readGranularity)
{
    FMSTR_UNUSED(pipeHandle);
    FMSTR_UNUSED(pipeData);
    FMSTR_UNUSED(pipeDataLen);
    FMSTR_UNUSED(readGranularity);

    return 0U;
}

/*lint -efile(766, freemaster_protocol.h) include file is not used in this case */

#endif /* FMSTR_USE_PIPES  && (!FMSTR_DISABLE) */

#if FMSTR_DISABLE > 0 || FMSTR_USE_PIPES == 0 || FMSTR_USE_PIPE_PRINTF == 0

FMSTR_BOOL FMSTR_PipePrintfU8(FMSTR_HPIPE pipeHandle, const char *format, FMSTR_U8 arg)
{
    FMSTR_UNUSED(pipeHandle);
    FMSTR_UNUSED(format);
    FMSTR_UNUSED(arg);

    return FMSTR_FALSE;
}

FMSTR_BOOL FMSTR_PipePrintfS8(FMSTR_HPIPE pipeHandle, const char *format, FMSTR_S8 arg)
{
    FMSTR_UNUSED(pipeHandle);
    FMSTR_UNUSED(format);
    FMSTR_UNUSED(arg);

    return FMSTR_FALSE;
}

FMSTR_BOOL FMSTR_PipePrintfU16(FMSTR_HPIPE pipeHandle, const char *format, FMSTR_U16 arg)
{
    FMSTR_UNUSED(pipeHandle);
    FMSTR_UNUSED(format);
    FMSTR_UNUSED(arg);

    return FMSTR_FALSE;
}

FMSTR_BOOL FMSTR_PipePrintfS16(FMSTR_HPIPE pipeHandle, const char *format, FMSTR_S16 arg)
{
    FMSTR_UNUSED(pipeHandle);
    FMSTR_UNUSED(format);
    FMSTR_UNUSED(arg);

    return FMSTR_FALSE;
}

FMSTR_BOOL FMSTR_PipePrintfU32(FMSTR_HPIPE pipeHandle, const char *format, FMSTR_U32 arg)
{
    FMSTR_UNUSED(pipeHandle);
    FMSTR_UNUSED(format);
    FMSTR_UNUSED(arg);

    return FMSTR_FALSE;
}

FMSTR_BOOL FMSTR_PipePrintfS32(FMSTR_HPIPE pipeHandle, const char *format, FMSTR_S32 arg)
{
    FMSTR_UNUSED(pipeHandle);
    FMSTR_UNUSED(format);
    FMSTR_UNUSED(arg);

    return FMSTR_FALSE;
}

#endif /* (!(FMSTR_USE_PIPES)) || (!(FMSTR_USE_PIPE_PRINTF)) */

#if FMSTR_DISABLE > 0 || FMSTR_USE_PIPES == 0 || FMSTR_USE_PIPE_PRINTF_VARG == 0

FMSTR_BOOL FMSTR_PipePrintf(FMSTR_HPIPE pipeHandle, const char *format, ...)
{
    FMSTR_UNUSED(pipeHandle);
    FMSTR_UNUSED(format);

    return FMSTR_FALSE;
}

#endif /* (!(FMSTR_USE_PIPES)) || (!(FMSTR_USE_PIPE_PRINTF_VARG)) */
