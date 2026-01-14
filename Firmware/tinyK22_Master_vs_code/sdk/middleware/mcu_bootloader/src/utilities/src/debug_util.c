/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*====================================================================================================
                                        INCLUDE FILES
==================================================================================================*/
#include <stdarg.h>
#include <stdlib.h>
#include "bootloader_common.h"
#include "fsl_device_registers.h"

/*==================================================================================================
                                     MACROs
==================================================================================================*/

#if (defined(DEBUG) || defined(_DEBUG))
/*==================================================================================================
                                     Prototypes
==================================================================================================*/
static void convert_digit_to_string(uint32_t digit, char *str, uint32_t *length);
static void convert_hexdigit_to_string(uint32_t digit, char *str, uint32_t *length);

#ifdef DEBUG_UART
extern void debug_uart_print(const uint8_t *buffer, uint32_t lengthInBytes);
#endif

/*==================================================================================================
                                     LOCAL FUNCTIONS
==================================================================================================*/
static void convert_digit_to_string(uint32_t digit, char *str, uint32_t *length)
{
    char buffer[11] = { 0 };
    uint32_t index = 0;
    if (digit < 1)
    {
        index = 1;
        buffer[0] = '0';
    }
    else
    {
        while (digit > 0)
        {
            buffer[index++] = (digit % 10) + '0';
            digit /= 10;
        }
    }

    char *digitStr = &buffer[index - 1];
    for (uint32_t i = 0; i < index; i++)
    {
        *str++ = *digitStr--;
    }

    *length = index;
}

static void convert_hexdigit_to_string(uint32_t digit, char *str, uint32_t *length)
{
    char buffer[8];
    uint32_t index = 0;

    for (uint32_t i = 0; i < sizeof(buffer) / sizeof(buffer[0]); i++)
    {
        buffer[i] = '0';
    }

    while (digit > 0)
    {
        uint32_t temp = digit & 0x0F;
        digit /= 16;
        if (temp < 10)
        {
            temp += '0';
        }
        else
        {
            temp = (temp - 10) + 'a';
        }
        buffer[index++] = temp;
    }

    index = 8;

    *str++ = '0';
    *str++ = 'x';
    char *digitStr = &buffer[index - 1];
    for (uint32_t i = 0; i < index; i++)
    {
        *str++ = *digitStr--;
    }

    *length = index + 2;
}

void debug_printf(const char *format, ...)
{
    static char buffer[256];
    char *stringBuffer = (char *)buffer;

    va_list arg;
    va_start(arg, format);

    const char *fmt = format;
    uint32_t digitLength = 0;
    char *printStr = stringBuffer;
    while (*fmt != '\0')
    {
        if (*fmt == '%')
        {
            if ((fmt[1] == 'd') || (fmt[1] == 'u') || (fmt[1] == 'l'))
            {
                uint32_t parameter = va_arg(arg, int);
                convert_digit_to_string(parameter, printStr, &digitLength);
                printStr += digitLength;
                fmt++;
            }
            else if (fmt[1] == 'x')
            {
                uint32_t parameter = va_arg(arg, int);
                convert_hexdigit_to_string(parameter, printStr, &digitLength);
                printStr += digitLength;
                fmt++;
            }
            else if (fmt[1] == 'c')
            {
                char parameter = (char)va_arg(arg, int);
                *printStr++ = parameter;
                fmt++;
            }
            else if (fmt[1] == 's')
            {
                char *str = va_arg(arg, char *);
                while (*str != '\0')
                {
                    *printStr++ = *str++;
                }
                fmt++;
            }
            else
            {
                *printStr++ = *fmt;
            }
        }
        else
        {
            *printStr++ = *fmt;
        }
        fmt++;
    }
    *printStr = '\0';

#ifdef DEBUG_UART
    debug_uart_print((const uint8_t *)stringBuffer, printStr - stringBuffer);
#elif defined(DEBUG_SEMIHOST)
    printf("%s", stringBuffer);
#endif
}

#endif
