/*
 * Copyright (c) 2016 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * All rights are reserved.
 * Proprietary and confidential.
 * Unauthorized copying of this file, via any medium is strictly prohibited.
 * Any use is subject to an appropriate license granted by Fingerprint Cards AB.
 *
 * This file implements a limited version of the vsnprintf.
 *
 * For currently supported flags, width, precision, length and specifier see
 * the test cases in the test_fpc_snprintf.c file. Feel free to implement missing
 * features.
 *
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include "fpc_snprintf.h"
#include <stdbool.h>

#define FLAG_MINUS (1<<0)
#define FLAG_PLUS  (1<<1)
#define FLAG_SPACE (1<<2)
#define FLAG_HASH  (1<<3)
#define FLAG_ZERO  (1<<4)

typedef enum {
    SPECIFIER_UNKNOWN,
    SPECIFIER_SIGNED_INT,
    SPECIFIER_UNSIGNED_INT,
    SPECIFIER_UNSIGNED_HEX_LOWER,
    SPECIFIER_UNSIGNED_HEX_UPPER,
    SPECIFIER_STRING,
    SPECIFIER_CHARACTER,
    SPECIFIER_POINTER,
    SPECIFIER_PERCENTAGE,
    SPECIFIER_FLOAT,
    SPECIFIER_NOTHING,
} specifier_t;

typedef enum {
    LENGTH_LONG, LENGTH_LONG_LONG, LENGTH_SHORT, LENGTH_CHAR,
} length_t;

#define WIDTH_DEFAULT      (0)
#define PRECISION_DEFAULT (-1)
#define PRECISION_DEFAULT_FLOAT (6)

//%[flags][width][.precision][length]specifier
typedef struct {
    int flags;
    int width;
    int precision;
    length_t length;
    specifier_t specifier;
} format_specifier;

typedef struct {
    char *buffer;
    size_t required;
    size_t size;
} format_output;

static const char* parse_flags(const char *format, format_specifier *fspec)
{
    while (1) {
        switch (*format) {
        case '-':
            fspec->flags |= FLAG_MINUS;
            break;
        case '+':
            fspec->flags |= FLAG_PLUS;
            break;
        case ' ':
            fspec->flags |= FLAG_SPACE;
            break;
        case '#':
            fspec->flags |= FLAG_HASH;
            break;
        case '0':
            fspec->flags |= FLAG_ZERO;
            break;
        default:
            return format;
        }
        format++;
    }
}

static int fpc_isdigit(char s)
{
    if (('0' <= s) && ('9' >= s)) {
        return 1;
    } else {
        return 0;
    }
}

#define PARSE_WIDTH_PRECISION(format, result, args) \
{ \
    if (*format == '*') { \
        result = va_arg(args, int); \
        format++; \
    } else { \
        result = 0; \
        while (fpc_isdigit(*format)) { \
            result *= 10; \
            result += *format - '0'; \
            format++; \
        } \
    } \
}

#define PARSE_WIDTH(format, fpsec, args) \
{ \
    fspec.width = WIDTH_DEFAULT; \
    PARSE_WIDTH_PRECISION(format, fspec.width, args)\
}

#define PARSE_PRECISION(format, fspec, args) \
{ \
    fspec.precision = PRECISION_DEFAULT; \
    if (*format == '.') { \
        format++; \
        PARSE_WIDTH_PRECISION(format, fspec.precision, args)\
    } \
}

static const char* parse_length(const char *format, format_specifier *fspec)
{
    switch (*format) {
    case 'l':
        fspec->length = LENGTH_LONG;
        format++;
        if (*format == 'l') {
            fspec->length = LENGTH_LONG_LONG;
            format++;
        }
        break;
    case 'h':
        fspec->length = LENGTH_SHORT;
        format++;
        if (*format == 'h') {
            fspec->length = LENGTH_CHAR;
            format++;
        }
        break;
    case 'j':
    case 'z':
    case 't':
    case 'L':
        // Currently not handled, just skip;
        format++;
        break;
    }
    return format;
}

static const char* parse_specifier(const char *format, format_specifier *fspec)
{
    switch (*format) {
    case 'd':
    case 'i':
        fspec->specifier = SPECIFIER_SIGNED_INT;
        break;
    case 'u':
        fspec->specifier = SPECIFIER_UNSIGNED_INT;
        break;
    case 'x':
        fspec->specifier = SPECIFIER_UNSIGNED_HEX_LOWER;
        break;
    case 'X':
        fspec->specifier = SPECIFIER_UNSIGNED_HEX_UPPER;
        break;
    case 's':
        fspec->specifier = SPECIFIER_STRING;
        break;
    case 'c':
        fspec->specifier = SPECIFIER_CHARACTER;
        break;
    case 'p':
        fspec->specifier = SPECIFIER_POINTER;
        if (sizeof(void *) == sizeof(long long)) {
            fspec->length = LENGTH_LONG_LONG;
        }
        break;
    case 'f':
    case 'F':
    case 'e':
    case 'E':
    case 'g':
    case 'G':
    case 'A':
    case 'a':
        fspec->specifier = SPECIFIER_FLOAT;
        break;
    case 'n':
        fspec->specifier = SPECIFIER_NOTHING;
        break;
    case '%':
        fspec->specifier = SPECIFIER_PERCENTAGE;
        break;
    default:
        fspec->specifier = SPECIFIER_UNKNOWN;
        break;
    }
    return ++format;
}

static void append_character(format_output *output, char character)
{
    if (output->size > output->required) {
        if (output->size > (output->required + 1)) {
            *output->buffer++ = character;
            *output->buffer = '\0';
        } else {
            *output->buffer = '\0';
        }
    }
    output->required++;
}

static int get_number_of_digits(unsigned int base, unsigned long long number,
                                unsigned long long *highest)
{
    int digits = 1;
    *highest = base;
    if (number < base) {
        *highest = 1;
    } else {
        digits++;
        while (number > (*highest * base - 1)) {
            unsigned long long highest_last = *highest;
            *highest *= base;
            if (highest_last > *highest) {
                *highest = highest_last;
                break;
            }
            digits++;
        }
    }
    return digits;
}

static void append_number(format_output *output, unsigned long long number,
                          unsigned long long highest, unsigned int base, bool upper)
{
    static const char *digit_to_char_lower = "0123456789abcdef";
    static const char *digit_to_char_upper = "0123456789ABCDEF";
    const char *digit_to_char = upper ? digit_to_char_upper : digit_to_char_lower;
    do {
        unsigned long long digit = number / highest;
        append_character(output, digit_to_char[digit]);
        number -= highest * digit;
        highest /= (unsigned long long)base;
    } while (highest);
}

static void print_number(format_output *output, unsigned int base, format_specifier *fspec,
        unsigned long long number)
{
    int digits;
    unsigned long long highest;

    if (fspec->specifier == SPECIFIER_SIGNED_INT) {
        if (((long long) number) < 0) {
            append_character(output, '-');
            number = 0 - number;
        }
    }

    digits = get_number_of_digits(base, number, &highest);

    int minlen = 1;
    char padding = ' ';
    if (fspec->width != WIDTH_DEFAULT) {
        minlen = fspec->width;
    }

    if (fspec->flags & FLAG_ZERO) {
        padding = '0';
    }

    if (fspec->precision != PRECISION_DEFAULT) {
        minlen = fspec->precision;
        padding = '0';
    }

    for (int i = 0; i < (minlen - digits); i++) {
        append_character(output, padding);
    }

    if ((minlen > 0) || (number != 0)) {
        append_number(output, number, highest, base,
                      fspec->specifier != SPECIFIER_UNSIGNED_HEX_LOWER);
    }
}

static double deci_pow(int pow)
{

    double result = 1.0;
    if (pow >= 0) {
        for (int i = 0; i < pow; i++) {
            result *= 10;
        }
    } else {
        for (int i = 0; i < -pow; i++) {
            result /= 10;
        }
    }

    return result;
}

static double round_away_from_zero(int precision, double number)
{
    /* According to http://pubs.opengroup.org/onlinepubs/009695399/functions/fprintf.html:
     * "The low-order digit shall be rounded in an implementation-defined manner."
     *
     * This implementation rounds away from zero. According to some other sources out there
     * the rounding should be "bankers rounding" which is also the rounding used in linux
     * snprintf. This is however trickier to implement and will probably not make any
     * practical difference in code that uses this.
     */
    double rounding = 0.5f * deci_pow(-precision);

    if (number > 0) {
        number += rounding;
    } else {
        number -= rounding;
    }
    return number;
}

static void print_float(format_output *output,format_specifier *fspec, double number)
{
    /* Currently we don't support huge numbers, long long is at least 64 bits
     * according to C99, so we assume that. A IEEE754 double have 52 bits of fraction
     * so we cap the max supported conservative to fit inside these bits to avoid overflow
     * when casting to long long.
     */
    if (number > 0x7ffffffffffff000 || number < -0x7ffffffffffff000) {
        append_character(output, 'E');
        append_character(output, 'r');
        append_character(output, 'r');
        return;
    }
    bool negative = false;
    if (number < 0) {
        number = -number;
        negative = true;
    }
    number = round_away_from_zero(fspec->precision, number);

    /* This will overflow before double does which is a current
     * limitation to this implementation. I.e. we can't print numbers
     * larger that max long long
     */
    long long integer = (long long) number;
    double fraction = number - (double)integer;

    int padding_length = fspec->width;

    if (negative) {
        append_character(output, '-');
        padding_length--;
    }

    unsigned long long highest;
    int integer_digits = get_number_of_digits(10, (unsigned long long)integer, &highest);
    padding_length -= integer_digits;

    if (fspec->precision > 0) {
        padding_length -= fspec->precision + 1; // Number of decimals and the '.'
    }

    char padding = ' ';
    if (fspec->flags & FLAG_ZERO) {
        padding = '0';
    }

    for(int i = 0; i < padding_length; i++) {
        append_character(output, padding);
    }

    append_number(output, (unsigned long long)integer, highest, 10, true);

    if (fspec->precision) {
        append_character(output, '.');

        fraction = fraction * deci_pow(fspec->precision);
        long long fraction_integer = (long long) fraction;
        int fraction_digits = get_number_of_digits(10, (unsigned long long)fraction_integer,
                                                   &highest);

        padding_length = fspec->precision - fraction_digits;

        for(int i = 0; i < padding_length; i++) {
            append_character(output, '0');
        }
        append_number(output, (unsigned long long)fraction_integer, highest, 10, true);
    }
}

static void print_string(format_output* output, format_specifier *fspec, const char* string)
{
    if (fspec->width) {
        int len = 0;
        while (*(string + len) != '\0') {
            len++;
        }
        int padding = fspec->width - len;
        while (padding-- > 0) {
            append_character(output, ' ');
        }
    }
    while (*string != '\0') {
        append_character(output, *string++);
    }
}

static void print_pointer(format_output* output, format_specifier *fspec, void* pointer)
{
    append_character(output, '0');
    append_character(output, 'x');
    fspec->specifier = SPECIFIER_UNSIGNED_HEX_LOWER;
    print_number(output, 16, fspec, (uintptr_t) pointer);
}

int fpc_vsnprintf(char *buffer, size_t size, const char *format, va_list args)
{
    if (size > 0) {
        *buffer = '\0';
    }
    format_output output = { .buffer = buffer, .size = size, .required = 0 };
    char cur = *format++;
    while (cur != '\0') {
        if (cur == '%') {
            format_specifier fspec = {0, WIDTH_DEFAULT, PRECISION_DEFAULT, LENGTH_LONG, SPECIFIER_UNKNOWN};
            format = parse_flags(format, &fspec);
            PARSE_WIDTH(format, fspec, args);
            PARSE_PRECISION(format, fspec, args);
            format = parse_length(format, &fspec);
            format = parse_specifier(format, &fspec);

            switch (fspec.specifier) {
            case (SPECIFIER_UNSIGNED_INT):
            case (SPECIFIER_SIGNED_INT): {
                if (fspec.length == LENGTH_LONG_LONG) {
                    long long number = va_arg(args, long long);
                    print_number(&output, 10, &fspec, (unsigned long long)number);
                } else {
                    int number = va_arg(args, int);
                    print_number(&output, 10, &fspec, (unsigned long long)number);
                }
                break;
            }
            case (SPECIFIER_UNSIGNED_HEX_LOWER):
            case (SPECIFIER_UNSIGNED_HEX_UPPER): {
                if (fspec.length == LENGTH_LONG_LONG) {
                    unsigned long long number = va_arg(args, unsigned long long);
                    print_number(&output, 16, &fspec, number);
                } else {
                    unsigned int number = va_arg(args, unsigned int);
                    print_number(&output, 16, &fspec, number);
                }
                break;
            }
            case (SPECIFIER_STRING): {
                char *string = va_arg(args, char *);
                print_string(&output, &fspec, string);
                break;
            }
            case (SPECIFIER_CHARACTER): {
                int character = va_arg(args, int);
                append_character(&output, (char) character);
                break;
            }
            case (SPECIFIER_POINTER): {
                void *pointer = va_arg(args, void *);
                print_pointer(&output, &fspec, pointer);
                break;
            }
            case (SPECIFIER_PERCENTAGE): {
                append_character(&output, '%');
                break;
            }
            case (SPECIFIER_FLOAT): {
                double number = va_arg(args, double);
                if (fspec.precision == PRECISION_DEFAULT) {
                    fspec.precision = PRECISION_DEFAULT_FLOAT;
                }
                print_float(&output, &fspec, number);
                break;
            }
            case (SPECIFIER_NOTHING): {
                int* printed_so_far = va_arg(args, int*);
                *printed_so_far = (int)output.required;
                break;
            }
            default:
                append_character(&output, '?');
                break;
            }
        } else {
            append_character(&output, cur);
        }
        cur = *format++;
    }

    return (int)output.required;
}

int fpc_snprintf(char *buffer, size_t size, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int ret = fpc_vsnprintf(buffer, size, format, args);
    va_end(args);
    return ret;
}
