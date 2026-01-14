/*
 * Author:     Brad Conte (brad AT bradconte.com)
 * Copyright:
 * Disclaimer: This code is presented "as is" without any guarantees.
 * Details:    Implementation of the SHA1 hashing algorithm.
               Algorithm specification can be found here:
               http://csrc.nist.gov/publications/fips/fips180-2/fips180-2withchangenotice.pdf
               This implementation uses little endian byte order.
 * Origin: https://github.com/B-Con/crypto-algorithms
 *
 * Copyright 2018-2020, 2024-2025 NXP
 * This code is based on original implementation by Brad Conte. Extended to
 * support big-endian platforms for FreeMASTER driver needs.
 *
 */

#include "freemaster.h"
#include "freemaster_private.h"

#if FMSTR_DISABLE == 0

/*****************************************************************************
 * Local Functions
 ******************************************************************************/

/* Coverity: Note that some CERT-C rules are intentionally violated in code below, marked with 
   coverity comment. Typically for shifting, casting and wrapping SHA-1 state data. This is 
   an inherent concept of SHA-1 algorithm. */

#define ROTLEFT(a, b) (((a) << (b)) | ((a) >> (32 - (b))))

static void FMSTR_Sha1Transform(FMSTR_SHA1_CTX *ctx, const FMSTR_U8 *data)
{
    // save 352 bytes on stack by making these static, we never run more instances than one anyway
    static FMSTR_U32 a, b, c, d, e, i, j, t, m[80];

    for (i = 0; i < 16U; i++)
    {
        j = i * 4U;

        a = ((FMSTR_U32)data[j]) << 24;
        /* coverity[cert_int30_c_violation:FALSE] */
        a += ((FMSTR_U32)data[j + 1U]) << 16;
        /* coverity[cert_int30_c_violation:FALSE] */
        a += ((FMSTR_U32)data[j + 2U]) << 8;
        /* coverity[cert_int30_c_violation:FALSE] */
        a += data[j + 3U];
        m[i] = a;
    }

    for (; i < 80U; i++)
    {
        m[i] = (m[i - 3U] ^ m[i - 8U] ^ m[i - 14U] ^ m[i - 16U]);
        m[i] = (m[i] << 1) | (m[i] >> 31);
    }

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];

    for (i = 0; i < 20U; i++)
    {
        /* coverity[cert_int30_c_violation:FALSE] */
        t = ROTLEFT(a, 5) + ((b & c) ^ (~b & d)) + e + ctx->k[0] + m[i];
        e = d;
        d = c;
        c = ROTLEFT(b, 30);
        b = a;
        a = t;
    }

    for (; i < 40U; i++)
    {
        /* coverity[cert_int30_c_violation:FALSE] */
        t = ROTLEFT(a, 5) + (b ^ c ^ d) + e + ctx->k[1] + m[i];
        e = d;
        d = c;
        c = ROTLEFT(b, 30);
        b = a;
        a = t;
    }

    for (; i < 60U; i++)
    {
        /* coverity[cert_int30_c_violation:FALSE] */
        t = ROTLEFT(a, 5) + ((b & c) ^ (b & d) ^ (c & d)) + e + ctx->k[2] + m[i];
        e = d;
        d = c;
        c = ROTLEFT(b, 30);
        b = a;
        a = t;
    }

    for (; i < 80U; i++)
    {
        /* coverity[cert_int30_c_violation:FALSE] */
        t = ROTLEFT(a, 5) + (b ^ c ^ d) + e + ctx->k[3] + m[i];
        e = d;
        d = c;
        c = ROTLEFT(b, 30);
        b = a;
        a = t;
    }

    /* coverity[cert_int30_c_violation:FALSE] */
    ctx->state[0] += a;
    /* coverity[cert_int30_c_violation:FALSE] */
    ctx->state[1] += b;
    /* coverity[cert_int30_c_violation:FALSE] */
    ctx->state[2] += c;
    /* coverity[cert_int30_c_violation:FALSE] */
    ctx->state[3] += d;
    /* coverity[cert_int30_c_violation:FALSE] */
    ctx->state[4] += e;
}

/*****************************************************************************
 * API Functions
 ******************************************************************************/

void FMSTR_Sha1Init(FMSTR_SHA1_CTX *ctx)
{
    ctx->datalen = 0U;
    ctx->bitlen  = 0U;

    ctx->state[0] = 0x67452301UL;
    ctx->state[1] = 0xEFCDAB89UL;
    ctx->state[2] = 0x98BADCFEUL;
    ctx->state[3] = 0x10325476UL;
    ctx->state[4] = 0xc3d2e1f0UL;

    ctx->k[0] = 0x5a827999UL;
    ctx->k[1] = 0x6ed9eba1UL;
    ctx->k[2] = 0x8f1bbcdcUL;
    ctx->k[3] = 0xca62c1d6UL;
}

void FMSTR_Sha1Update(FMSTR_SHA1_CTX *ctx, const FMSTR_U8 *data, FMSTR_SIZE len)
{
    FMSTR_SIZE i;

    for (i = 0U; i < len; i++)
    {
        ctx->data[ctx->datalen] = data[i];

        /* coverity[cert_int30_c_violation:FALSE] */
        ctx->datalen++;
        if (ctx->datalen == 64U)
        {
            FMSTR_Sha1Transform(ctx, ctx->data);
            ctx->bitlen += 512U;
            ctx->datalen = 0U;
        }
    }
}

void FMSTR_Sha1Final(FMSTR_SHA1_CTX *ctx, FMSTR_U8 *hash)
{
    FMSTR_U32 i;

    i = ctx->datalen;

    // Pad whatever data is left in the buffer.
    if (ctx->datalen < 56U)
    {
        /* coverity[cert_int30_c_violation:FALSE] */
        ctx->data[i++] = 0x80U;
        while (i < 56U)
        {
            ctx->data[i++] = 0x00U;
        }
    }
    else
    {
        /* coverity[cert_int30_c_violation:FALSE] */
        ctx->data[i++] = 0x80U;
        while (i < 64U)
        {
            ctx->data[i++] = 0x00U;
        }
        FMSTR_Sha1Transform(ctx, ctx->data);
        FMSTR_MemSet(ctx->data, 0U, 56U);
    }

    // Append to the padding the total message's length in bits and transform.
    i = ctx->datalen * 8U;
    /* coverity[cert_int30_c_violation:FALSE] */
    ctx->bitlen += (FMSTR_U64)i;
    /* coverity[cert_int31_c_violation:FALSE] */
    ctx->data[63] = (FMSTR_U8)(ctx->bitlen);
    /* coverity[cert_int31_c_violation:FALSE] */
    ctx->data[62] = (FMSTR_U8)(ctx->bitlen >> 8);
    /* coverity[cert_int31_c_violation:FALSE] */
    ctx->data[61] = (FMSTR_U8)(ctx->bitlen >> 16);
    /* coverity[cert_int31_c_violation:FALSE] */
    ctx->data[60] = (FMSTR_U8)(ctx->bitlen >> 24);
    /* coverity[cert_int31_c_violation:FALSE] */
    ctx->data[59] = (FMSTR_U8)(ctx->bitlen >> 32);
    /* coverity[cert_int31_c_violation:FALSE] */
    ctx->data[58] = (FMSTR_U8)(ctx->bitlen >> 40);
    /* coverity[cert_int31_c_violation:FALSE] */
    ctx->data[57] = (FMSTR_U8)(ctx->bitlen >> 48);
    /* coverity[cert_int31_c_violation:FALSE] */
    ctx->data[56] = (FMSTR_U8)(ctx->bitlen >> 56);
    
    FMSTR_Sha1Transform(ctx, ctx->data);

#if FMSTR_PLATFORM_BIG_ENDIAN > 0
    /* copy state to output buffer */
    for (i = 0U; i < 5U; i++)
    {
        *(FMSTR_U32 *)(hash + i * 4U) = ctx->state[i];
    }
#else
    /* copy while reversing byte order */
    for (i = 0U; i < 4U; i++)
    {
        FMSTR_SIZE shift = (FMSTR_SIZE)(24U - i * 8U);
        hash[i]          = (FMSTR_U8)((ctx->state[0] >> shift) & 0xffU);
        hash[i + 4U]     = (FMSTR_U8)((ctx->state[1] >> shift) & 0xffU);
        hash[i + 8U]     = (FMSTR_U8)((ctx->state[2] >> shift) & 0xffU);
        hash[i + 12U]    = (FMSTR_U8)((ctx->state[3] >> shift) & 0xffU);
        hash[i + 16U]    = (FMSTR_U8)((ctx->state[4] >> shift) & 0xffU);
    }
#endif
}

#endif /* !FMSTR_DISABLE */
